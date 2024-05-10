/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ability_runtime/js_ability_context.h"

#include <chrono>
#include <cstdint>

#include "ability_manager_client.h"
#include "app_utils.h"
#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "ability_runtime/js_caller_complex.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "mission_info.h"
#include "napi_common_ability.h"
#include "napi_common_start_options.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "open_link_options.h"
#include "open_link/napi_common_open_link_options.h"
#include "start_options.h"
#include "tokenid_kit.h"
#include "uri.h"
#include "want.h"

#ifdef SUPPORT_GRAPHICS
#include "pixel_map_napi.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t INDEX_THREE = 3;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr int32_t TRACE_ATOMIC_SERVICE_ID = 201;
const std::string TRACE_ATOMIC_SERVICE = "StartAtomicService";
constexpr int32_t CALLER_TIME_OUT = 10; // 10s

namespace {
static std::map<ConnectionKey, sptr<JSAbilityConnection>, KeyCompare> g_connects;
std::mutex gConnectsLock_;
int64_t g_serialNumber = 0;
const std::string ATOMIC_SERVICE_PREFIX = "com.atomicservice.";

// This function has to be called from engine thread
void RemoveConnection(int64_t connectId)
{
    std::lock_guard<std::mutex> lock(gConnectsLock_);
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
    [&connectId](const auto &obj) {
        return connectId == obj.first.id;
    });
    if (item != g_connects.end()) {
        TAG_LOGD(AAFwkTag::CONTEXT, "remove connection ability exist.");
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_connects.erase(item);
    } else {
        TAG_LOGD(AAFwkTag::CONTEXT, "remove connection ability not exist");
    }
}

int64_t InsertConnection(sptr<JSAbilityConnection> connection, const AAFwk::Want &want, int32_t accountId = -1)
{
    std::lock_guard<std::mutex> lock(gConnectsLock_);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "connection null");
        return -1;
    }
    int64_t connectId = g_serialNumber;
    ConnectionKey key;
    key.id = g_serialNumber;
    key.want = want;
    key.accountId = accountId;
    connection->SetConnectionId(key.id);
    g_connects.emplace(key, connection);
    if (g_serialNumber < INT32_MAX) {
        g_serialNumber++;
    } else {
        g_serialNumber = 0;
    }
    return connectId;
}

class StartAbilityByCallParameters {
public:
    int err = 0;
    sptr<IRemoteObject> remoteCallee = nullptr;
    std::shared_ptr<CallerCallBack> callerCallBack = nullptr;
    std::mutex mutexlock;
    std::condition_variable condition;
};

void GenerateCallerCallBack(std::shared_ptr<StartAbilityByCallParameters> calls,
    std::shared_ptr<CallerCallBack> callerCallBack)
{
    if (calls == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "calls is nullptr");
        return;
    }
    if (callerCallBack == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "callerCallBack is nullptr");
        return;
    }
    auto callBackDone = [calldata = calls] (const sptr<IRemoteObject> &obj) {
        TAG_LOGD(AAFwkTag::CONTEXT, "OnStartAbilityByCall callBackDone mutexlock");
        std::unique_lock<std::mutex> lock(calldata->mutexlock);
        TAG_LOGD(AAFwkTag::CONTEXT, "OnStartAbilityByCall callBackDone remoteCallee assignment");
        calldata->remoteCallee = obj;
        calldata->condition.notify_all();
        TAG_LOGD(AAFwkTag::CONTEXT, "OnStartAbilityByCall callBackDone is called end");
    };

    auto releaseListen = [](const std::string &str) {
        TAG_LOGI(AAFwkTag::CONTEXT, "OnStartAbilityByCall releaseListen is called %{public}s", str.c_str());
    };

    callerCallBack->SetCallBack(callBackDone);
    callerCallBack->SetOnRelease(releaseListen);
}

void StartAbilityByCallExecuteDone(std::shared_ptr<StartAbilityByCallParameters> calldata)
{
    if (calldata == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "calldata is nullptr.");
        return;
    }
    std::unique_lock<std::mutex> lock(calldata->mutexlock);
    if (calldata->remoteCallee != nullptr) {
        TAG_LOGI(AAFwkTag::CONTEXT, "OnStartAbilityByCall callExecute callee isn`t nullptr");
        return;
    }

    if (calldata->condition.wait_for(lock, std::chrono::seconds(CALLER_TIME_OUT)) == std::cv_status::timeout) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnStartAbilityByCall callExecute waiting callee timeout");
        calldata->err = -1;
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "OnStartAbilityByCall callExecute end");
}

void StartAbilityByCallComplete(napi_env env, NapiAsyncTask& task, std::weak_ptr<AbilityContext> abilityContext,
    std::shared_ptr<StartAbilityByCallParameters> calldata, std::shared_ptr<CallerCallBack> callerCallBack)
{
    if (calldata == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "calldata is nullptr.");
        return;
    }
    if (calldata->err != 0) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnStartAbilityByCall callComplete err is %{public}d", calldata->err);
        task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
        TAG_LOGD(AAFwkTag::CONTEXT, "clear failed call of startup is called.");
        auto context = abilityContext.lock();
        if (context == nullptr || callerCallBack == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "clear failed call of startup input param is nullptr.");
            return;
        }
        context->ClearFailedCallConnection(callerCallBack);
        return;
    }
    auto context = abilityContext.lock();
    if (context == nullptr || callerCallBack == nullptr || calldata->remoteCallee == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnStartAbilityByCall callComplete params error %{public}s is nullptr",
            context == nullptr ? "context" : (calldata->remoteCallee == nullptr ? "remoteCallee" : "callerCallBack"));
        task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
        TAG_LOGD(AAFwkTag::CONTEXT, "OnStartAbilityByCall callComplete end");
        return;
    }
    auto releaseCallAbilityFunc = [abilityContext] (const std::shared_ptr<CallerCallBack> &callback) -> ErrCode {
        auto contextForRelease = abilityContext.lock();
        if (contextForRelease == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "releaseCallAbilityFunction, context is nullptr");
            return -1;
        }
        return contextForRelease->ReleaseCall(callback);
    };
    task.Resolve(env, CreateJsCallerComplex(env, releaseCallAbilityFunc, calldata->remoteCallee, callerCallBack));
    TAG_LOGD(AAFwkTag::CONTEXT, "OnStartAbilityByCall callComplete end");
}
}

void JsAbilityContext::Finalizer(napi_env env, void* data, void* hint)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "called");
    std::unique_ptr<JsAbilityContext>(static_cast<JsAbilityContext*>(data));
}

napi_value JsAbilityContext::StartAbility(napi_env env, napi_callback_info info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnStartAbility);
}

napi_value JsAbilityContext::OpenLink(napi_env env, napi_callback_info info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnOpenLink);
}

napi_value JsAbilityContext::StartAbilityAsCaller(napi_env env, napi_callback_info info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnStartAbilityAsCaller);
}

napi_value JsAbilityContext::StartRecentAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnStartRecentAbility);
}

napi_value JsAbilityContext::StartAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnStartAbilityWithAccount);
}

napi_value JsAbilityContext::StartAbilityByCall(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnStartAbilityByCall);
}

napi_value JsAbilityContext::StartAbilityForResult(napi_env env, napi_callback_info info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnStartAbilityForResult);
}

napi_value JsAbilityContext::StartAbilityForResultWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnStartAbilityForResultWithAccount);
}

napi_value JsAbilityContext::StartServiceExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnStartExtensionAbility);
}

napi_value JsAbilityContext::StartServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnStartExtensionAbilityWithAccount);
}

napi_value JsAbilityContext::StopServiceExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnStopExtensionAbility);
}

napi_value JsAbilityContext::StopServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnStopExtensionAbilityWithAccount);
}

napi_value JsAbilityContext::ConnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnConnectAbility);
}

napi_value JsAbilityContext::ConnectAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnConnectAbilityWithAccount);
}

napi_value JsAbilityContext::DisconnectAbility(napi_env env, napi_callback_info info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnDisconnectAbility);
}

napi_value JsAbilityContext::TerminateSelf(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnTerminateSelf);
}

napi_value JsAbilityContext::TerminateSelfWithResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnTerminateSelfWithResult);
}

napi_value JsAbilityContext::RestoreWindowStage(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnRestoreWindowStage);
}

napi_value JsAbilityContext::RequestDialogService(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnRequestDialogService);
}

napi_value JsAbilityContext::ReportDrawnCompleted(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnReportDrawnCompleted);
}

napi_value JsAbilityContext::IsTerminating(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnIsTerminating);
}

napi_value JsAbilityContext::StartAbilityByType(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnStartAbilityByType);
}

napi_value JsAbilityContext::RequestModalUIExtension(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnRequestModalUIExtension);
}

napi_value JsAbilityContext::ShowAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnShowAbility);
}

napi_value JsAbilityContext::HideAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnHideAbility);
}

napi_value JsAbilityContext::OpenAtomicService(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnOpenAtomicService);
}

napi_value JsAbilityContext::MoveAbilityToBackground(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnMoveAbilityToBackground);
}

void JsAbilityContext::ClearFailedCallConnection(
    const std::weak_ptr<AbilityContext>& abilityContext, const std::shared_ptr<CallerCallBack> &callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "clear failed call of startup is called.");
    auto context = abilityContext.lock();
    if (context == nullptr || callback == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "clear failed call of startup input param is nullptr.");
        return;
    }

    context->ClearFailedCallConnection(callback);
}

napi_value JsAbilityContext::OnStartAbility(napi_env env, NapiCallbackInfo& info, bool isStartRecent)
{
    StartAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, TRACE_ATOMIC_SERVICE, TRACE_ATOMIC_SERVICE_ID);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Not enough arguments");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want);
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = ARGC_ONE;
    TAG_LOGD(AAFwkTag::CONTEXT, "ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        TAG_LOGD(AAFwkTag::CONTEXT, "OnStartAbility start options is used.");
        if (!AppExecFwk::UnwrapStartOptionsWithProcessOption(env, info.argv[INDEX_ONE], startOptions)) {
            ThrowInvalidParamError(env, "Parse param startOptions failed, startOptions must be StartOptions.");
            TAG_LOGE(AAFwkTag::CONTEXT, "unwrap startOptions failed.");
            return CreateJsUndefined(env);
        }
        unwrapArgc++;
    }

    if (isStartRecent) {
        TAG_LOGD(AAFwkTag::CONTEXT, "OnStartRecentAbility is enter");
        want.SetParam(Want::PARAM_RESV_START_RECENT, true);
    }

    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count());
        want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
    }

    auto innerErrorCode = std::make_shared<int>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, want, startOptions, unwrapArgc,
        &observer = freeInstallObserver_, innerErrorCode]() {
        auto context = weak.lock();
        if (!context) {
            TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
            *innerErrorCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }

        *innerErrorCode = (unwrapArgc == ARGC_ONE) ?
            context->StartAbility(want, -1) : context->StartAbility(want, startOptions, -1);
        if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND &&
            *innerErrorCode != 0 && observer != nullptr) {
            std::string bundleName = want.GetElement().GetBundleName();
            std::string abilityName = want.GetElement().GetAbilityName();
            std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
            observer->OnInstallFinished(bundleName, abilityName, startTime, *innerErrorCode);
        }
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*innerErrorCode == 0) {
            TAG_LOGD(AAFwkTag::CONTEXT, "StartAbility success.");
            task.Resolve(env, CreateJsUndefined(env));
        } else {
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
        }
    };

    napi_value lastParam = (info.argc > unwrapArgc) ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        AddFreeInstallObserver(env, want, lastParam, &result);
        NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnStartAbility", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), nullptr, nullptr));
    } else {
        NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnStartAbility", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    }
    return result;
}

static bool CheckUrl(std::string &urlValue)
{
    if (urlValue.empty()) {
        return false;
    }
    Uri uri = Uri(urlValue);
    if (uri.GetScheme().empty() || uri.GetHost().empty()) {
        return false;
    }

    return true;
}

bool JsAbilityContext::CreateOpenLinkTask(const napi_env &env, const napi_value &lastParam,
    AAFwk::Want &want, int &requestCode)
{
    want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> uasyncTask =
    CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);
    RuntimeTask task = [env, asyncTask](int resultCode, const AAFwk::Want& want, bool isInner) {
        TAG_LOGI(AAFwkTag::CONTEXT, "OnOpenLink async callback is begin");
        HandleScope handleScope(env);
        napi_value abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::CONTEXT, "wrap abilityResult error");
            asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
        } else {
            if (isInner) {
                asyncTask->Reject(env, CreateJsErrorByNativeErr(env, resultCode));
            } else {
                asyncTask->ResolveWithNoError(env, abilityResult);
            }
        }
    };
    curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
    requestCode = curRequestCode_;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
        return false;
    } else {
        context->InsertResultCallbackTask(requestCode, std::move(task));
    }
    return true;
}

static bool ParseOpenLinkParams(const napi_env &env, const NapiCallbackInfo &info, std::string &linkValue,
    AAFwk::OpenLinkOptions &openLinkOptions, AAFwk::Want &want)
{
    if (info.argc != ARGC_THREE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "wrong arguments num");
        return false;
    }

    if (!CheckTypeForNapiValue(env, info.argv[ARGC_ZERO], napi_string)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "link must be string");
        return false;
    }
    if (!ConvertFromJsValue(env, info.argv[ARGC_ZERO], linkValue) || !CheckUrl(linkValue)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "link parameter invalid");
        return false;
    }

    if (CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        TAG_LOGD(AAFwkTag::CONTEXT, "OpenLinkOptions is used.");
        if (!AppExecFwk::UnwrapOpenLinkOptions(env, info.argv[INDEX_ONE], openLinkOptions, want)) {
            TAG_LOGE(AAFwkTag::CONTEXT, "openLinkOptions parse failed");
            return false;
        }
    }

    return true;
}

napi_value JsAbilityContext::OnOpenLink(napi_env env, NapiCallbackInfo& info)
{
    StartAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, TRACE_ATOMIC_SERVICE, TRACE_ATOMIC_SERVICE_ID);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::CONTEXT, "OnOpenLink");

    std::string linkValue("");
    AAFwk::OpenLinkOptions openLinkOptions;
    napi_value lastParam = nullptr;
    AAFwk::Want want;
    want.SetParam(AppExecFwk::APP_LINKING_ONLY, false);

    if (!ParseOpenLinkParams(env, info, linkValue, openLinkOptions, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "parse openLink arguments failed");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    TAG_LOGI(AAFwkTag::CONTEXT, "open link:%{public}s.", linkValue.c_str());
    want.SetUri(linkValue);
    int requestCode = -1;
    if (CheckTypeForNapiValue(env, info.argv[INDEX_TWO], napi_function)) {
        TAG_LOGD(AAFwkTag::CONTEXT, "completionHandler is used.");
        lastParam = info.argv[INDEX_TWO];
        CreateOpenLinkTask(env, lastParam, want, requestCode);
    }

    auto innerErrorCode = std::make_shared<int>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, want, innerErrorCode, requestCode]() {
        auto context = weak.lock();
        if (!context) {
            TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
            *innerErrorCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        *innerErrorCode = context->StartAbility(want, requestCode);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*innerErrorCode == 0) {
            TAG_LOGI(AAFwkTag::CONTEXT, "OpenLink success.");
            task.ResolveWithNoError(env, CreateJsUndefined(env));
        } else {
            TAG_LOGI(AAFwkTag::CONTEXT, "OpenLink failed.");
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
        }
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnOpenLink", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsAbilityContext::OnStartAbilityAsCaller(napi_env env, NapiCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want);
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = ARGC_ONE;
    TAG_LOGI(AAFwkTag::CONTEXT, "StartAbilityAsCaller, ability:%{public}s.",
        want.GetElement().GetAbilityName().c_str());
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        TAG_LOGD(AAFwkTag::CONTEXT, "OnStartAbilityAsCaller start options is used.");
        AppExecFwk::UnwrapStartOptions(env, info.argv[INDEX_ONE], startOptions);
        unwrapArgc++;
    }
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, startOptions, unwrapArgc](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto innerErrorCode = (unwrapArgc == ARGC_ONE) ?
                context->StartAbilityAsCaller(want, -1) : context->StartAbilityAsCaller(want, startOptions, -1);
            if (innerErrorCode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
            }
        };

    napi_value lastParam = (info.argc > unwrapArgc) ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnStartAbilityAsCaller",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsAbilityContext::OnStartRecentAbility(napi_env env, NapiCallbackInfo& info)
{
    return OnStartAbility(env, info, true);
}

napi_value JsAbilityContext::OnStartAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc < ARGC_TWO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want);
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = ARGC_ONE;
    TAG_LOGI(AAFwkTag::CONTEXT, "StartAbilityWithAccount, ability:%{public}s",
        want.GetElement().GetAbilityName().c_str());
    int32_t accountId = 0;
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[INDEX_ONE], accountId)) {
        TAG_LOGD(AAFwkTag::CONTEXT, "the second parameter is invalid.");
        ThrowInvalidParamError(env, "Parse param accountId failed, accountId must be number.");
        return CreateJsUndefined(env);
    }
    unwrapArgc++;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_TWO && CheckTypeForNapiValue(env, info.argv[INDEX_TWO], napi_object)) {
        TAG_LOGD(AAFwkTag::CONTEXT, "OnStartAbilityWithAccount start options is used.");
        AppExecFwk::UnwrapStartOptions(env, info.argv[INDEX_TWO], startOptions);
        unwrapArgc++;
    }

    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count());
        want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
    }

    auto innerErrorCode = std::make_shared<int>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute =
        [weak = context_, want, accountId, startOptions, unwrapArgc, innerErrorCode,
            &observer = freeInstallObserver_]() {
        auto context = weak.lock();
        if (!context) {
            TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
            *innerErrorCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }

        *innerErrorCode = (unwrapArgc == INDEX_TWO) ?
            context->StartAbilityWithAccount(want, accountId, -1) : context->StartAbilityWithAccount(
                want, accountId, startOptions, -1);
        if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND &&
            *innerErrorCode != 0 && observer != nullptr) {
            std::string bundleName = want.GetElement().GetBundleName();
            std::string abilityName = want.GetElement().GetAbilityName();
            std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
            observer->OnInstallFinished(bundleName, abilityName, startTime, *innerErrorCode);
        }
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](
        napi_env env, NapiAsyncTask& task, int32_t status) {
            if (*innerErrorCode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
            }
    };
    napi_value lastParam = (info.argc > unwrapArgc) ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        AddFreeInstallObserver(env, want, lastParam, &result);
        NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnStartAbilityWithAccount", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), nullptr, nullptr));
    } else {
        NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnStartAbilityWithAccount", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    }
    return result;
}

bool JsAbilityContext::CheckStartAbilityByCallParams(napi_env env, NapiCallbackInfo& info,
    AAFwk::Want &want, int32_t &userId, napi_value &lastParam)
{
    if (info.argc < ARGC_ONE) {
        ThrowTooFewParametersError(env);
        return false;
    }

    if (!CheckTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_object) ||
        !AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to parse want!");
        ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return false;
    }

    InheritWindowMode(want);

    if (info.argc == ARGC_ONE) {
        return true;
    }
    bool paramOneIsNumber = CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_number);
    bool paramOneIsFunction = CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function);
    if (!paramOneIsNumber && !paramOneIsFunction) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed, input param type invalid");
        ThrowInvalidParamError(env, "Parse second param failed, second param must be callback or number.");
        return false;
    }
    if (paramOneIsNumber && !ConvertFromJsValue(env, info.argv[INDEX_ONE], userId)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to parse accountId!");
        ThrowInvalidParamError(env, "Parse param accountId failed, accountId must be number.");
        return false;
    }
    if (paramOneIsFunction) {
        lastParam = info.argv[INDEX_ONE];
    }

    if (info.argc > ARGC_TWO && CheckTypeForNapiValue(env, info.argv[INDEX_TWO], napi_function)) {
        lastParam = info.argv[INDEX_TWO];
    }
    return true;
}

napi_value JsAbilityContext::OnStartAbilityByCall(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "JsAbilityContext::%{public}s, called", __func__);
    // 1. check params
    napi_value lastParam = nullptr;
    int32_t userId = DEFAULT_INVAL_VALUE;
    AAFwk::Want want;
    if (!CheckStartAbilityByCallParams(env, info, want, userId, lastParam)) {
        return CreateJsUndefined(env);
    }

    // 2. create CallBack function
    std::shared_ptr<StartAbilityByCallParameters> calls = std::make_shared<StartAbilityByCallParameters>();
    auto callExecute = [calldata = calls] () { StartAbilityByCallExecuteDone(calldata); };
    auto callerCallBack = std::make_shared<CallerCallBack>();
    GenerateCallerCallBack(calls, callerCallBack);
    auto callComplete = [weak = context_, calldata = calls, callerCallBack] (
        napi_env env, NapiAsyncTask& task, int32_t status) {
        StartAbilityByCallComplete(env, task, weak, calldata, callerCallBack);
    };

    // 3. StartAbilityByCall
    napi_value retsult = nullptr;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnStartAbilityByCall context is nullptr");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }

    auto ret = context->StartAbilityByCall(want, callerCallBack, userId);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnStartAbilityByCall StartAbility is failed");
        ThrowErrorByNativeErr(env, ret);
        return CreateJsUndefined(env);
    }

    if (calls->remoteCallee == nullptr) {
        TAG_LOGI(AAFwkTag::CONTEXT, "OnStartAbilityByCall async wait execute");
        NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnStartAbilityByCall", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(callExecute), std::move(callComplete), &retsult));
    } else {
        TAG_LOGI(AAFwkTag::CONTEXT, "OnStartAbilityByCall promiss return result execute");
        NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnStartAbilityByCall", env,
            CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(callComplete), &retsult));
    }

    TAG_LOGD(AAFwkTag::CONTEXT, "JsAbilityContext, called end");
    return retsult;
}

napi_value JsAbilityContext::OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnStartAbilityForResult is called");

    if (info.argc == ARGC_ZERO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to parse want!");
        ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return CreateJsUndefined(env);
    }
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = ARGC_ONE;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        TAG_LOGD(AAFwkTag::CONTEXT, "OnStartAbilityForResult begin options is used.");
        AppExecFwk::UnwrapStartOptions(env, info.argv[INDEX_ONE], startOptions);
        unwrapArgc++;
    }

    napi_value lastParam = info.argc > unwrapArgc ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> uasyncTask;
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, lastParam, &result, true);
        uasyncTask = CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, nullptr);
    } else {
        uasyncTask = CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &result);
    }
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);
    RuntimeTask task = [env, asyncTask, element = want.GetElement(), flags = want.GetFlags(), startTime,
        &observer = freeInstallObserver_]
        (int resultCode, const AAFwk::Want& want, bool isInner) {
        TAG_LOGD(AAFwkTag::CONTEXT, "OnStartAbilityForResult async callback is begin");
        HandleScope handleScope(env);
        std::string bundleName = element.GetBundleName();
        std::string abilityName = element.GetAbilityName();
        napi_value abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::CONTEXT, "wrap abilityResult failed");
            isInner = true;
            resultCode = ERR_INVALID_VALUE;
        }
        bool freeInstallEnable = (flags & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND &&
            observer != nullptr;
        if (freeInstallEnable) {
            isInner ? observer->OnInstallFinished(bundleName, abilityName, startTime, resultCode) :
                observer->OnInstallFinished(bundleName, abilityName, startTime, abilityResult);
        } else {
            isInner ? asyncTask->Reject(env, CreateJsErrorByNativeErr(env, resultCode)) :
                asyncTask->Resolve(env, abilityResult);
        }
    };
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
        asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    } else {
        want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
        curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
        (unwrapArgc == ARGC_ONE) ? context->StartAbilityForResult(want, curRequestCode_, std::move(task)) :
            context->StartAbilityForResult(want, startOptions, curRequestCode_, std::move(task));
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "OnStartAbilityForResult is called end");
    return result;
}

napi_value JsAbilityContext::OnStartAbilityForResultWithAccount(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "StartAbilityForResultWithAccount");
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "This application is not system-app, can not use system-api");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    if (info.argc < ARGC_TWO) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to parse want!");
        ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return CreateJsUndefined(env);
    }
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = ARGC_ONE;
    int32_t accountId = 0;
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[INDEX_ONE], accountId)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "the second parameter is invalid.");
        ThrowInvalidParamError(env, "Parse param accountId failed, accountId must be number.");
        return CreateJsUndefined(env);
    }
    unwrapArgc++;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_TWO && CheckTypeForNapiValue(env, info.argv[INDEX_TWO], napi_object)) {
        TAG_LOGD(AAFwkTag::CONTEXT, "OnStartAbilityForResultWithAccount start options is used.");
        AppExecFwk::UnwrapStartOptions(env, info.argv[INDEX_TWO], startOptions);
        unwrapArgc++;
    }
    napi_value lastParam = info.argc > unwrapArgc ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> uasyncTask;
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, lastParam, &result, true);
        uasyncTask = CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, nullptr);
    } else {
        uasyncTask = CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &result);
    }
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);
    RuntimeTask task = [env, asyncTask, element = want.GetElement(), flags = want.GetFlags(), startTime,
        &observer = freeInstallObserver_]
        (int resultCode, const AAFwk::Want& want, bool isInner) {
        TAG_LOGD(AAFwkTag::CONTEXT, "async callback is called");
        std::string bundleName = element.GetBundleName();
        std::string abilityName = element.GetAbilityName();
        HandleScope handleScope(env);
        napi_value abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::CONTEXT, "wrap abilityResult failed");
            isInner = true;
            resultCode = ERR_INVALID_VALUE;
        }
        bool freeInstallEnable = (flags & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND &&
            observer != nullptr;
        if (freeInstallEnable) {
            isInner ? observer->OnInstallFinished(bundleName, abilityName, startTime, resultCode) :
                observer->OnInstallFinished(bundleName, abilityName, startTime, abilityResult);
        } else {
            isInner ? asyncTask->Reject(env, CreateJsErrorByNativeErr(env, resultCode)) :
                asyncTask->Resolve(env, abilityResult);
        }
        TAG_LOGD(AAFwkTag::CONTEXT, "async callback is called end");
    };
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
        asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    } else {
        curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
        (unwrapArgc == INDEX_TWO) ? context->StartAbilityForResultWithAccount(
            want, accountId, curRequestCode_, std::move(task)) : context->StartAbilityForResultWithAccount(
                want, accountId, startOptions, curRequestCode_, std::move(task));
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "OnStartAbilityForResultWithAccount is called end");
    return result;
}

napi_value JsAbilityContext::OnStartExtensionAbility(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "StartExtensionAbility");
    if (info.argc < ARGC_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to parse want!");
        ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto errcode = context->StartServiceExtensionAbility(want);
            if (errcode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errcode));
            }
        };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnStartExtensionAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsAbilityContext::OnStartExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "StartExtensionAbilityWithAccount");
    if (info.argc < ARGC_TWO) {
        TAG_LOGE(AAFwkTag::CONTEXT, "param is too few.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    int32_t accountId = -1;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want) ||
        !OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[INDEX_ONE], accountId)) {
        ThrowInvalidParamError(env, "Parse param accountId failed, accountId must be number.");
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, accountId](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::CONTEXT, "context has been released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto errcode = context->StartServiceExtensionAbility(want, accountId);
            if (errcode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errcode));
            }
        };

    napi_value lastParam = (info.argc > ARGC_TWO) ? info.argv[INDEX_TWO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnStartExtensionAbilityWithAccount",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsAbilityContext::OnStopExtensionAbility(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "StopExtensionAbility");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "param is too few for stop extension ability.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto errcode = context->StopServiceExtensionAbility(want);
            if (errcode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errcode));
            }
        };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAbilityContext::OnStopExtensionAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsAbilityContext::OnStopExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "StartExtensionAbilityWithAccount");
    if (info.argc < ARGC_TWO) {
        TAG_LOGE(AAFwkTag::CONTEXT, "param is too few for stop extension ability with account.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    int32_t accountId = -1;
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want) ||
        !AppExecFwk::UnwrapInt32FromJS2(env, info.argv[INDEX_ONE], accountId)) {
        ThrowInvalidParamError(env,
            "Parse param want or accountId failed, want must be Want and accountId must be number.");
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, accountId](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto errcode = context->StopServiceExtensionAbility(want, accountId);
            if (errcode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errcode));
            }
        };

    napi_value lastParam = (info.argc > ARGC_TWO) ? info.argv[INDEX_TWO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAbilityContext::OnStopExtensionAbilityWithAccount",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsAbilityContext::OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "TerminateSelfWithResult");

    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    int resultCode = 0;
    AAFwk::Want want;
    if (!AppExecFwk::UnWrapAbilityResult(env, info.argv[INDEX_ZERO], resultCode, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "%s Failed to parse ability result!", __func__);
        ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return CreateJsUndefined(env);
    }

    auto abilityContext = context_.lock();
    if (abilityContext != nullptr) {
        abilityContext->SetTerminating(true);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, resultCode](napi_env env, NapiAsyncTask& task, int32_t status) {
            TAG_LOGI(AAFwkTag::CONTEXT, "TerminateSelfWithResult async");
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            auto errorCode = context->TerminateAbilityWithResult(want, resultCode);
            if (errorCode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errorCode));
            }
        };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnTerminateSelfWithResult",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    TAG_LOGD(AAFwkTag::CONTEXT, "OnTerminateSelfWithResult is called end");
    return result;
}

napi_value JsAbilityContext::OnConnectAbility(napi_env env, NapiCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // only support two params
    if (info.argc < ARGC_TWO) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Connect ability failed, not enough params.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // unwrap want
    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want);
    TAG_LOGI(AAFwkTag::CONTEXT, "ConnectAbility, callee:%{public}s.%{public}s",
        want.GetBundle().c_str(),
        want.GetElement().GetAbilityName().c_str());

    // unwarp connection
    sptr<JSAbilityConnection> connection = new JSAbilityConnection(env);
    connection->SetJsConnectionObject(info.argv[INDEX_ONE]);
    int64_t connectId = InsertConnection(connection, want);
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, connection, connectId](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGE(AAFwkTag::CONTEXT, "Connect ability failed, context is released.");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                RemoveConnection(connectId);
                return;
            }
            TAG_LOGD(AAFwkTag::CONTEXT, "ConnectAbility connection:%{public}d", static_cast<int32_t>(connectId));
            auto innerErrorCode = context->ConnectAbility(want, connection);
            int32_t errcode = static_cast<int32_t>(AbilityRuntime::GetJsErrorCodeByNativeError(innerErrorCode));
            if (errcode) {
                connection->CallJsFailed(errcode);
                RemoveConnection(connectId);
            }
            task.Resolve(env, CreateJsUndefined(env));
        };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnConnectAbility",
        env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
    return CreateJsValue(env, connectId);
}

napi_value JsAbilityContext::OnConnectAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    // only support three params
    if (info.argc < ARGC_THREE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "This application is not system-app, can not use system-api");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    // unwrap want
    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want);
    TAG_LOGI(AAFwkTag::CONTEXT, "ConnectAbilityWithAccount, bundlename:%{public}s abilityname:%{public}s",
        want.GetBundle().c_str(),
        want.GetElement().GetAbilityName().c_str());

    int32_t accountId = 0;
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[INDEX_ONE], accountId)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "the second parameter is invalid.");
        ThrowInvalidParamError(env, "Parse param accountId failed, accountId must be number.");
        return CreateJsUndefined(env);
    }

    // unwarp connection
    sptr<JSAbilityConnection> connection = new JSAbilityConnection(env);
    connection->SetJsConnectionObject(info.argv[INDEX_TWO]);
    int64_t connectId = InsertConnection(connection, want, accountId);
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, accountId, connection, connectId](
            napi_env env, NapiAsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    TAG_LOGE(AAFwkTag::CONTEXT, "context is released");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    RemoveConnection(connectId);
                    return;
                }
                TAG_LOGI(AAFwkTag::CONTEXT, "context->ConnectAbilityWithAccount connection:%{public}d",
                    static_cast<int32_t>(connectId));
                auto innerErrorCode = context->ConnectAbilityWithAccount(want, accountId, connection);
                int32_t errcode = static_cast<int32_t>(AbilityRuntime::GetJsErrorCodeByNativeError(innerErrorCode));
                if (errcode) {
                    connection->CallJsFailed(errcode);
                    RemoveConnection(connectId);
                }
                task.Resolve(env, CreateJsUndefined(env));
        };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnConnectAbilityWithAccount",
        env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
    return CreateJsValue(env, connectId);
}

napi_value JsAbilityContext::OnDisconnectAbility(napi_env env, NapiCallbackInfo& info)
{
    std::lock_guard<std::mutex> lock(gConnectsLock_);
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // unwrap want
    AAFwk::Want want;
    // unwrap connectId
    int64_t connectId = -1;
    sptr<JSAbilityConnection> connection = nullptr;
    int32_t accountId = -1;
    napi_get_value_int64(env, info.argv[INDEX_ZERO], &connectId);
    TAG_LOGI(AAFwkTag::CONTEXT, "DisconnectAbility, connection:%{public}d.", static_cast<int32_t>(connectId));
    auto item = std::find_if(g_connects.begin(),
        g_connects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != g_connects.end()) {
        // match id
        want = item->first.want;
        connection = item->second;
        accountId = item->first.accountId;
    } else {
        TAG_LOGI(AAFwkTag::CONTEXT, "not find conn exist.");
    }
    // begin disconnect
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, connection, accountId](
            napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::CONTEXT, "OnDisconnectAbility context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            if (connection == nullptr) {
                TAG_LOGW(AAFwkTag::CONTEXT, "connection nullptr");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            TAG_LOGD(AAFwkTag::CONTEXT, "context->DisconnectAbility");
            context->DisconnectAbility(want, connection, accountId);
            task.Resolve(env, CreateJsUndefined(env));
        };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAbilityContext::OnDisconnectAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsAbilityContext::OnTerminateSelf(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "TerminateSelf");
    auto abilityContext = context_.lock();
    if (abilityContext != nullptr) {
        abilityContext->SetTerminating(true);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_](napi_env env, NapiAsyncTask& task, int32_t status) {
            TAG_LOGI(AAFwkTag::CONTEXT, "TerminateSelf async");
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            auto errcode = context->TerminateSelf();
            if (errcode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errcode));
            }
        };

    napi_value lastParam = (info.argc > ARGC_ZERO) ? info.argv[INDEX_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnTerminateSelf",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsAbilityContext::OnRestoreWindowStage(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "RestoreWindowStage, info.argc = %{public}d", static_cast<int>(info.argc));
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnRestoreWindowStage need one parameters");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    auto context = context_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnRestoreWindowStage context is released");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }
    auto errcode = context->RestoreWindowStage(env, info.argv[INDEX_ZERO]);
    if (errcode != 0) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsError(env, errcode, "RestoreWindowStage failed.");
    }
    return CreateJsUndefined(env);
}

napi_value JsAbilityContext::OnRequestDialogService(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want);
    TAG_LOGI(AAFwkTag::CONTEXT, "RequestDialogService, target:%{public}s.%{public}s.", want.GetBundle().c_str(),
        want.GetElement().GetAbilityName().c_str());

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    auto uasyncTask = CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);
    RequestDialogResultTask task =
        [env, asyncTask](int32_t resultCode, const AAFwk::Want &resultWant) {
        HandleScope handleScope(env);
        napi_value requestResult = JsAbilityContext::WrapRequestDialogResult(env, resultCode, resultWant);
        if (requestResult == nullptr) {
            TAG_LOGW(AAFwkTag::CONTEXT, "wrap requestResult failed");
            asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
        } else {
            asyncTask->Resolve(env, requestResult);
        }
        TAG_LOGD(AAFwkTag::CONTEXT, "OnRequestDialogService async callback is called end");
    };
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::CONTEXT, "context is released, can not requestDialogService");
        asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    } else {
        auto errCode = context->RequestDialogService(env, want, std::move(task));
        if (errCode != ERR_OK) {
            asyncTask->Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(errCode)));
        }
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "OnRequestDialogService is called end");
    return result;
}

napi_value JsAbilityContext::OnIsTerminating(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "IsTerminating");
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnIsTerminating context is nullptr");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }
    return CreateJsValue(env, context->IsTerminating());
}

napi_value JsAbilityContext::OnReportDrawnCompleted(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "called.");
    auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, innerErrorCode]() {
        auto context = weak.lock();
        if (!context) {
            TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
            *innerErrorCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }

        *innerErrorCode = context->ReportDrawnCompleted();
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*innerErrorCode == ERR_OK) {
            task.Resolve(env, CreateJsUndefined(env));
        } else {
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
        }
    };

    napi_value lastParam = info.argv[INDEX_ZERO];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnReportDrawnCompleted",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAbilityContext::WrapRequestDialogResult(napi_env env,
    int32_t resultCode, const AAFwk::Want &want)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to get object");
        return nullptr;
    }

    napi_set_named_property(env, object, "result", CreateJsValue(env, resultCode));
    napi_set_named_property(env, object, "want", AppExecFwk::WrapWant(env, want));
    return object;
}

void JsAbilityContext::InheritWindowMode(AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "called");
#ifdef SUPPORT_GRAPHICS
    // only split mode need inherit
    auto context = context_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context is nullptr.");
        return;
    }
    auto windowMode = context->GetCurrentWindowMode();
    if (AAFwk::AppUtils::GetInstance().IsInheritWindowSplitScreenMode() &&
        (windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY)) {
        want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "window mode is %{public}d", windowMode);
#endif
}

void JsAbilityContext::ConfigurationUpdated(napi_env env, std::shared_ptr<NativeReference> &jsContext,
    const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "called");
    if (jsContext == nullptr || config == nullptr) {
        TAG_LOGI(AAFwkTag::CONTEXT, "jsContext is nullptr.");
        return;
    }

    napi_value object = jsContext->GetNapiValue();
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to get object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, object, "onUpdateConfiguration", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to get onUpdateConfiguration from object");
        return;
    }

    napi_value argv[] = {CreateJsConfiguration(env, *config)};
    napi_call_function(env, object, method, ARGC_ONE, argv, nullptr);
}

void JsAbilityContext::AddFreeInstallObserver(napi_env env, const AAFwk::Want &want, napi_value callback,
    napi_value *result, bool isAbilityResult)
{
    // adapter free install async return install and start result
    TAG_LOGD(AAFwkTag::CONTEXT, "ConvertWindowSize begin.");
    int ret = 0;
    if (freeInstallObserver_ == nullptr) {
        freeInstallObserver_ = new JsFreeInstallObserver(env);
        ret = AAFwk::AbilityManagerClient::GetInstance()->AddFreeInstallObserver(freeInstallObserver_);
    }

    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AddFreeInstallObserver error.");
    } else {
        TAG_LOGI(AAFwkTag::CONTEXT, "AddJsObserverObject");
        std::string bundleName = want.GetElement().GetBundleName();
        std::string abilityName = want.GetElement().GetAbilityName();
        std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
        freeInstallObserver_->AddJsObserverObject(
            bundleName, abilityName, startTime, callback, result, isAbilityResult);
    }
}

napi_value CreateJsAbilityContext(napi_env env, std::shared_ptr<AbilityContext> context)
{
    napi_value object = CreateJsBaseContext(env, context);

    std::unique_ptr<JsAbilityContext> jsContext = std::make_unique<JsAbilityContext>(context);
    napi_wrap(env, object, jsContext.release(), JsAbilityContext::Finalizer, nullptr, nullptr);

    auto abilityInfo = context->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        napi_set_named_property(env, object, "abilityInfo", CreateJsAbilityInfo(env, *abilityInfo));
    }

    auto configuration = context->GetConfiguration();
    if (configuration != nullptr) {
        napi_set_named_property(env, object, "config", CreateJsConfiguration(env, *configuration));
    }

    const char *moduleName = "JsAbilityContext";
    BindNativeFunction(env, object, "startAbility", moduleName, JsAbilityContext::StartAbility);
    BindNativeFunction(env, object, "openLink", moduleName, JsAbilityContext::OpenLink);
    BindNativeFunction(env, object, "startAbilityAsCaller", moduleName, JsAbilityContext::StartAbilityAsCaller);
    BindNativeFunction(env, object, "startAbilityWithAccount", moduleName,
        JsAbilityContext::StartAbilityWithAccount);
    BindNativeFunction(env, object, "startAbilityByCall", moduleName, JsAbilityContext::StartAbilityByCall);
    BindNativeFunction(env, object, "startAbilityForResult", moduleName, JsAbilityContext::StartAbilityForResult);
    BindNativeFunction(env, object, "startAbilityForResultWithAccount", moduleName,
        JsAbilityContext::StartAbilityForResultWithAccount);
    BindNativeFunction(env, object, "startServiceExtensionAbility", moduleName,
        JsAbilityContext::StartServiceExtensionAbility);
    BindNativeFunction(env, object, "startServiceExtensionAbilityWithAccount", moduleName,
        JsAbilityContext::StartServiceExtensionAbilityWithAccount);
    BindNativeFunction(env, object, "stopServiceExtensionAbility", moduleName,
        JsAbilityContext::StopServiceExtensionAbility);
    BindNativeFunction(env, object, "stopServiceExtensionAbilityWithAccount", moduleName,
        JsAbilityContext::StopServiceExtensionAbilityWithAccount);
    BindNativeFunction(env, object, "connectAbility", moduleName, JsAbilityContext::ConnectAbility);
    BindNativeFunction(env, object, "connectServiceExtensionAbility", moduleName, JsAbilityContext::ConnectAbility);
    BindNativeFunction(env, object, "connectAbilityWithAccount", moduleName,
        JsAbilityContext::ConnectAbilityWithAccount);
    BindNativeFunction(env, object, "connectServiceExtensionAbilityWithAccount", moduleName,
        JsAbilityContext::ConnectAbilityWithAccount);
    BindNativeFunction(env, object, "disconnectAbility", moduleName, JsAbilityContext::DisconnectAbility);
    BindNativeFunction(
        env, object, "disconnectServiceExtensionAbility", moduleName, JsAbilityContext::DisconnectAbility);
    BindNativeFunction(env, object, "terminateSelf", moduleName, JsAbilityContext::TerminateSelf);
    BindNativeFunction(env, object, "terminateSelfWithResult", moduleName,
        JsAbilityContext::TerminateSelfWithResult);
    BindNativeFunction(env, object, "restoreWindowStage", moduleName, JsAbilityContext::RestoreWindowStage);
    BindNativeFunction(env, object, "isTerminating", moduleName, JsAbilityContext::IsTerminating);
    BindNativeFunction(env, object, "startRecentAbility", moduleName,
        JsAbilityContext::StartRecentAbility);
    BindNativeFunction(env, object, "requestDialogService", moduleName,
        JsAbilityContext::RequestDialogService);
    BindNativeFunction(env, object, "reportDrawnCompleted", moduleName,
        JsAbilityContext::ReportDrawnCompleted);
    BindNativeFunction(env, object, "setMissionContinueState", moduleName,
        JsAbilityContext::SetMissionContinueState);
    BindNativeFunction(env, object, "startAbilityByType", moduleName,
        JsAbilityContext::StartAbilityByType);
    BindNativeFunction(env, object, "requestModalUIExtension", moduleName,
        JsAbilityContext::RequestModalUIExtension);
    BindNativeFunction(env, object, "showAbility", moduleName,
        JsAbilityContext::ShowAbility);
    BindNativeFunction(env, object, "hideAbility", moduleName,
        JsAbilityContext::HideAbility);
    BindNativeFunction(env, object, "openAtomicService", moduleName,
        JsAbilityContext::OpenAtomicService);
    BindNativeFunction(env, object, "moveAbilityToBackground", moduleName, JsAbilityContext::MoveAbilityToBackground);

#ifdef SUPPORT_GRAPHICS
    BindNativeFunction(env, object, "setMissionLabel", moduleName, JsAbilityContext::SetMissionLabel);
    BindNativeFunction(env, object, "setMissionIcon", moduleName, JsAbilityContext::SetMissionIcon);
#endif
    return object;
}

JSAbilityConnection::JSAbilityConnection(napi_env env) : env_(env) {}

JSAbilityConnection::~JSAbilityConnection()
{
    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "~JSAbilityConnection: failed to get uv loop.");
        return;
    }

    ConnectCallback *cb = new (std::nothrow) ConnectCallback();
    if (cb == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "~JSAbilityConnection: failed to create cb.");
        return;
    }
    cb->jsConnectionObject_ = std::move(jsConnectionObject_);

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "~JSAbilityConnection: failed to create work.");
        delete cb;
        cb = nullptr;
        return;
    }
    work->data = reinterpret_cast<void *>(cb);
    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
    [](uv_work_t *work, int status) {
        if (work == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "~JSAbilityConnection: work is nullptr.");
            return;
        }
        if (work->data == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "~JSAbilityConnection: data is nullptr.");
            delete work;
            work = nullptr;
            return;
        }
        ConnectCallback *cb = reinterpret_cast<ConnectCallback *>(work->data);
        delete cb;
        cb = nullptr;
        delete work;
        work = nullptr;
    });
    if (ret != 0) {
        if (cb != nullptr) {
            delete cb;
            cb = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
}

void JSAbilityConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

void JSAbilityConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "OnAbilityConnectDone, resultCode:%{public}d", resultCode);
    wptr<JSAbilityConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, remoteObject, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSAbilityConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                TAG_LOGE(AAFwkTag::CONTEXT, "connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
        });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSAbilityConnection::OnAbilityConnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAbilityConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "HandleOnAbilityConnectDone, resultCode:%{public}d", resultCode);
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "jsConnectionObject_ nullptr");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to get object");
        return;
    }
    napi_value methodOnConnect = nullptr;
    napi_get_named_property(env_, obj, "onConnect", &methodOnConnect);
    if (methodOnConnect == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to get onConnect from object");
        return;
    }

    // wrap RemoteObject
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(env_, remoteObject);
    napi_value argv[] = { ConvertElement(element), napiRemoteObject };
    napi_call_function(env_, obj, methodOnConnect, ARGC_TWO, argv, nullptr);
    TAG_LOGD(AAFwkTag::CONTEXT, "OnAbilityConnectDone end");
}

void JSAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "OnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    wptr<JSAbilityConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSAbilityConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                TAG_LOGI(AAFwkTag::CONTEXT, "connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSAbilityConnection::OnAbilityDisconnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAbilityConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    std::lock_guard<std::mutex> lock(gConnectsLock_);
    TAG_LOGD(AAFwkTag::CONTEXT, "resultCode:%{public}d", resultCode);
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "jsConnectionObject_ nullptr");
        return;
    }

    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Wrong to get object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onDisconnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Wrong to get onDisconnect from object");
        return;
    }

    // release connect
    TAG_LOGD(AAFwkTag::CONTEXT, "OnAbilityDisconnectDone g_connects.size:%{public}zu", g_connects.size());
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
        [bundleName, abilityName, connectionId = connectionId_] (
            const auto &obj) {
                return (bundleName == obj.first.want.GetBundle()) &&
                    (abilityName == obj.first.want.GetElement().GetAbilityName()) &&
                    connectionId == obj.first.id;
        });
    if (item != g_connects.end()) {
        // match bundlename && abilityname
        g_connects.erase(item);
        TAG_LOGD(AAFwkTag::CONTEXT, "OnAbilityDisconnectDone erase g_connects.size:%{public}zu", g_connects.size());
    }

    napi_value argv[] = { ConvertElement(element) };
    TAG_LOGD(AAFwkTag::CONTEXT, "OnAbilityDisconnectDone CallFunction success");
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
}

void JSAbilityConnection::CallJsFailed(int32_t errorCode)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "CallJsFailed");
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "jsConnectionObject_ nullptr");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to get object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onFailed", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to get onFailed from object");
        return;
    }

    napi_value argv[] = {CreateJsValue(env_, errorCode)};
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
    TAG_LOGD(AAFwkTag::CONTEXT, "CallJsFailed end");
}

napi_value JSAbilityConnection::ConvertElement(const AppExecFwk::ElementName &element)
{
    return AppExecFwk::WrapElementName(env_, element);
}

void JSAbilityConnection::SetJsConnectionObject(napi_value jsConnectionObject)
{
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsConnectionObject, 1, &ref);
    jsConnectionObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
}

void JSAbilityConnection::RemoveConnectionObject()
{
    jsConnectionObject_.reset();
}

napi_value JsAbilityContext::SetMissionContinueState(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnSetMissionContinueState);
}

napi_value JsAbilityContext::OnSetMissionContinueState(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "OnSetMissionContinueState, info.argc = %{public}d", static_cast<int>(info.argc));
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnSetMissionContinueState, Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::ContinueState state;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], state)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnSetMissionContinueState, parse state failed.");
        ThrowInvalidParamError(env, "Parse param state failed, state must be State.");
        return CreateJsUndefined(env);
    }

    if (state <= AAFwk::ContinueState::CONTINUESTATE_UNKNOWN || state >= AAFwk::ContinueState::CONTINUESTATE_MAX) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnSetMissionContinueState, invalid state param, state = %{public}d.", state);
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, state](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            auto errcode = context->SetMissionContinueState(state);
            if (errcode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errcode));
            }
        };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityContext::SetMissionContinueState",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

#ifdef SUPPORT_GRAPHICS
napi_value JsAbilityContext::SetMissionLabel(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnSetMissionLabel);
}

napi_value JsAbilityContext::SetMissionIcon(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnSetMissionIcon);
}

napi_value JsAbilityContext::OnSetMissionLabel(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "SetMissionLabel, info.argc = %{public}d", static_cast<int>(info.argc));
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnSetMissionLabel, Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string label;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], label)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnSetMissionLabel, parse label failed.");
        ThrowInvalidParamError(env, "Parse param label failed, label must be string.");
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, label](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            auto errcode = context->SetMissionLabel(label);
            if (errcode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errcode));
            }
        };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnSetMissionLabel",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsAbilityContext::OnSetMissionIcon(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "SetMissionIcon, info.argc = %{public}d", static_cast<int>(info.argc));
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnSetMissionIcon, Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    auto icon = OHOS::Media::PixelMapNapi::GetPixelMap(env, info.argv[INDEX_ZERO]);
    if (!icon) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnSetMissionIcon, parse icon failed.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, icon](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::CONTEXT, "context is released when set mission icon");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            auto errcode = context->SetMissionIcon(icon);
            if (errcode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errcode));
            }
        };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnSetMissionIcon",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}
#endif

napi_value JsAbilityContext::OnStartAbilityByType(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "call");
    if (info.argc < ARGC_THREE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnStartAbilityByType, Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string type;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], type)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnStartAbilityByType, parse type failed.");
        ThrowInvalidParamError(env, "Parse param type failed, type must be string.");
        return CreateJsUndefined(env);
    }

    AAFwk::WantParams wantParam;
    if (!AppExecFwk::UnwrapWantParams(env, info.argv[INDEX_ONE], wantParam)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnStartAbilityByType, parse wantParam failed.");
        ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return CreateJsUndefined(env);
    }

    std::shared_ptr<JsUIExtensionCallback> callback = std::make_shared<JsUIExtensionCallback>(env);
    callback->SetJsCallbackObject(info.argv[INDEX_TWO]);
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, type, wantParam, callback](napi_env env, NapiAsyncTask& task, int32_t status) mutable {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::CONTEXT, "OnStartAbilityByType context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            auto errcode = context->StartAbilityByType(type, wantParam, callback);
            if (errcode != 0) {
                task.Reject(env, CreateJsErrorByNativeErr(env, errcode));
            } else {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            }
        };

    napi_value lastParam = (info.argc > ARGC_THREE) ? info.argv[INDEX_THREE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnStartAbilityByType",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsAbilityContext::OnRequestModalUIExtension(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "called");

    if (info.argc < ARGC_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[0], want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to parse want!");
        ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto errcode = context->RequestModalUIExtension(want);
            if (errcode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errcode));
            }
        };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnRequestModalUIExtension",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsAbilityContext::OnShowAbility(napi_env env, NapiCallbackInfo& info)
{
    return ChangeAbilityVisibility(env, info, true);
}

napi_value JsAbilityContext::OnHideAbility(napi_env env, NapiCallbackInfo& info)
{
    return ChangeAbilityVisibility(env, info, false);
}

napi_value JsAbilityContext::ChangeAbilityVisibility(napi_env env, NapiCallbackInfo& info, bool isShow)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "called");
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, isShow](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto errCode = context->ChangeAbilityVisibility(isShow);
            if (errCode == 0) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errCode));
            }
        };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityContext::ChangeAbilityVisibility",
        env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsAbilityContext::OnOpenAtomicService(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc == ARGC_ZERO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string appId;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], appId)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Fail to parse appId.");
        ThrowInvalidParamError(env, "Parse param appId failed, appId must be string.");
        return CreateJsUndefined(env);
    }

    Want want;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        TAG_LOGD(AAFwkTag::CONTEXT, "OnOpenAtomicService atomic service options is used.");
        if (!AppExecFwk::UnwrapStartOptionsAndWant(env, info.argv[INDEX_ONE], startOptions, want)) {
            TAG_LOGE(AAFwkTag::CONTEXT, "Fail to parse atomic service options.");
            ThrowInvalidParamError(env, "Parse param startOptions failed, startOptions must be StartOption.");
            return CreateJsUndefined(env);
        }
    }

    std::string bundleName = ATOMIC_SERVICE_PREFIX + appId;
    TAG_LOGD(AAFwkTag::CONTEXT, "bundleName: %{public}s.", bundleName.c_str());
    want.SetBundle(bundleName);
    return OpenAtomicServiceInner(env, info, want, startOptions);
}

napi_value JsAbilityContext::OpenAtomicServiceInner(napi_env env, NapiCallbackInfo& info, Want &want,
    AAFwk::StartOptions &options)
{
    InheritWindowMode(want);
    want.AddFlags(Want::FLAG_INSTALL_ON_DEMAND);
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
    napi_value result = nullptr;
    AddFreeInstallObserver(env, want, nullptr, &result, true);
    RuntimeTask task = [env, element = want.GetElement(), startTime, &observer = freeInstallObserver_](
        int resultCode, const AAFwk::Want& want, bool isInner) {
        TAG_LOGD(AAFwkTag::CONTEXT, "OnOpenAtomicService async callback is begin");
        if (observer == nullptr) {
            TAG_LOGW(AAFwkTag::CONTEXT, "observer is nullptr.");
            return;
        }
        HandleScope handleScope(env);
        std::string bundleName = element.GetBundleName();
        std::string abilityName = element.GetAbilityName();
        napi_value abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::CONTEXT, "wrap abilityResult failed");
            isInner = true;
            resultCode = ERR_INVALID_VALUE;
        }
        if (isInner) {
            observer->OnInstallFinished(bundleName, abilityName, startTime, resultCode);
        } else {
            observer->OnInstallFinished(bundleName, abilityName, startTime, abilityResult);
        }
    };
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context is released");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    } else {
        want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
        curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
        context->OpenAtomicService(want, options, curRequestCode_, std::move(task));
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "OnOpenAtomicService is called end");
    return result;
}

napi_value JsAbilityContext::OnMoveAbilityToBackground(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnMoveAbilityToBackground");
    auto abilityContext = context_.lock();

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_](napi_env env, NapiAsyncTask& task, int32_t status) {
            TAG_LOGD(AAFwkTag::CONTEXT, "OnMoveAbilityToBackground task");
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::CONTEXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            auto errcode = context->MoveUIAbilityToBackground();
            if (errcode == 0) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errcode));
            }
        };

    napi_value lastParam = (info.argc > ARGC_ZERO) ? info.argv[INDEX_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnMoveAbilityToBackground",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
