/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ability_runtime/js_embeddable_ui_ability_context.h"

#include <chrono>
#include <cstdint>

#include "ability_manager_client.h"
#include "event_handler.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "ability_runtime/js_caller_complex.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "ipc_skeleton.h"
#include "mission_info.h"
#include "napi/native_api.h"
#include "napi_common_ability.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "napi_common_start_options.h"
#include "napi_remote_object.h"
#include "start_options.h"
#include "want.h"
#include "tokenid_kit.h"

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

namespace {
int64_t g_serialNumber = 0;
static std::map<ConnectionKey, sptr<JSEmbeddableUIAbilityConnection>, KeyCompare> g_connects;

void RemoveConnection(int64_t connectId)
{
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
    [&connectId](const auto &obj) {
        return connectId == obj.first.id;
    });
    if (item != g_connects.end()) {
        HILOG_DEBUG("remove connection ability exist.");
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_connects.erase(item);
    } else {
        HILOG_DEBUG("remove connection ability not exist");
    }
}
} // namespace

class StartAbilityByCallParameters {
public:
    int err = 0;
    sptr<IRemoteObject> remoteCallee = nullptr;
    std::shared_ptr<CallerCallBack> callerCallBack = nullptr;
    std::mutex mutexlock;
    std::condition_variable condition;
};

void JsEmbeddableUIAbilityContext::Finalizer(napi_env env, void* data, void* hint)
{
    HILOG_DEBUG("JsEmbeddableUIAbilityContext Finalizer is called");
    std::unique_ptr<JsEmbeddableUIAbilityContext>(static_cast<JsEmbeddableUIAbilityContext*>(data));
}

napi_value JsEmbeddableUIAbilityContext::StartAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbility);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityForResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityForResult);
}

napi_value JsEmbeddableUIAbilityContext::ConnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnConnectAbility);
}

napi_value JsEmbeddableUIAbilityContext::DisconnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnDisconnectAbility);
}
//  不支持以返回errcode形式处理
napi_value JsEmbeddableUIAbilityContext::StartAbilityAsCaller(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityAsCaller);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityByCall(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityByCall);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityForResultWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityForResultWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::StartServiceExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartExtensionAbility);
}

napi_value JsEmbeddableUIAbilityContext::StartServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartExtensionAbilityWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::StopServiceExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStopExtensionAbility);
}

napi_value JsEmbeddableUIAbilityContext::StopServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStopExtensionAbilityWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::ConnectAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnConnectAbilityWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::TerminateSelf(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnTerminateSelf);
}

napi_value JsEmbeddableUIAbilityContext::TerminateSelfWithResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnTerminateSelfWithResult);
}

napi_value JsEmbeddableUIAbilityContext::RestoreWindowStage(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnRestoreWindowStage);
}

napi_value JsEmbeddableUIAbilityContext::IsTerminating(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnIsTerminating);
}

napi_value JsEmbeddableUIAbilityContext::StartRecentAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartRecentAbility);
}

napi_value JsEmbeddableUIAbilityContext::RequestDialogService(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnRequestDialogService);
}

napi_value JsEmbeddableUIAbilityContext::ReportDrawnCompleted(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnReportDrawnCompleted);
}

napi_value JsEmbeddableUIAbilityContext::SetMissionContinueState(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnSetMissionContinueState);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityByType(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityByType);
}

void JsEmbeddableUIAbilityContext::ClearFailedCallConnection(
    const std::weak_ptr<AbilityContext>& abilityContext, const std::shared_ptr<CallerCallBack> &callback)
{
    HILOG_DEBUG("clear failed call of startup is called.");
    auto context = abilityContext.lock();
    if (context == nullptr || callback == nullptr) {
        HILOG_ERROR("clear failed call of startup input param is nullptr.");
        return;
    }

    context->ClearFailedCallConnection(callback);
}

napi_value JsEmbeddableUIAbilityContext::CreateJsEmbeddableUIAbilityContext(napi_env env,
    std::shared_ptr<AbilityContext> context)
{
    HILOG_DEBUG("CreateJsEmbeddableUIAbilityContext begin");
    napi_value objValue = CreateJsBaseContext(env, context);

    std::unique_ptr<JsEmbeddableUIAbilityContext> jsContext = std::make_unique<JsEmbeddableUIAbilityContext>(context);
    napi_wrap(env, objValue, jsContext.release(), Finalizer, nullptr, nullptr);

    auto abilityInfo = context->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        napi_set_named_property(env, objValue, "abilityInfo", CreateJsAbilityInfo(env, *abilityInfo));
    }

    auto configuration = context->GetConfiguration();
    if (configuration != nullptr) {
        napi_set_named_property(env, objValue, "config", CreateJsConfiguration(env, *configuration));
    }

    const char *moduleName = "JsEmbeddableUIAbilityContext";
    BindNativeFunction(env, objValue, "startAbility", moduleName, StartAbility);
    BindNativeFunction(env, objValue, "startAbilityForResult", moduleName, StartAbilityForResult);
    BindNativeFunction(env, objValue, "connectServiceExtensionAbility", moduleName, ConnectAbility);
    BindNativeFunction(env, objValue, "disconnectServiceExtensionAbility", moduleName, DisconnectAbility);
    // 返回errcode形式处理
    BindNativeFunction(env, objValue, "startAbilityAsCaller", moduleName, StartAbilityAsCaller);
    BindNativeFunction(env, objValue, "startAbilityWithAccount", moduleName, StartAbilityWithAccount);
    BindNativeFunction(env, objValue, "startAbilityByCall", moduleName, StartAbilityByCall);
    BindNativeFunction(env, objValue, "startAbilityForResultWithAccount", moduleName, StartAbilityForResultWithAccount);
    BindNativeFunction(env, objValue, "startServiceExtensionAbility", moduleName, StartServiceExtensionAbility);
    BindNativeFunction(env, objValue, "startServiceExtensionAbilityWithAccount", moduleName,
        StartServiceExtensionAbilityWithAccount);
    BindNativeFunction(env, objValue, "stopServiceExtensionAbility", moduleName, StopServiceExtensionAbility);
    BindNativeFunction(env, objValue, "stopServiceExtensionAbilityWithAccount", moduleName,
        StopServiceExtensionAbilityWithAccount);
    BindNativeFunction(env, objValue, "connectServiceExtensionAbilityWithAccount", moduleName, ConnectAbilityWithAccount);
    BindNativeFunction(env, objValue, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(env, objValue, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);
    BindNativeFunction(env, objValue, "restoreWindowStage", moduleName, RestoreWindowStage);
    BindNativeFunction(env, objValue, "isTerminating", moduleName, IsTerminating);
    BindNativeFunction(env, objValue, "startRecentAbility", moduleName, StartRecentAbility);
    BindNativeFunction(env, objValue, "requestDialogService", moduleName, RequestDialogService);
    BindNativeFunction(env, objValue, "reportDrawnCompleted", moduleName, ReportDrawnCompleted);
    BindNativeFunction(env, objValue, "setMissionContinueState", moduleName, SetMissionContinueState);
    BindNativeFunction(env, objValue, "startAbilityByType", moduleName, StartAbilityByType);

#ifdef SUPPORT_GRAPHICS
    BindNativeFunction(env, objValue, "setMissionLabel", moduleName, SetMissionLabel);
    BindNativeFunction(env, objValue, "setMissionIcon", moduleName, SetMissionIcon);
#endif
    return objValue;
}


napi_value JsEmbeddableUIAbilityContext::WrapRequestDialogResult(napi_env env,
    int32_t resultCode, const AAFwk::Want &want)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    napi_set_named_property(env, object, "result", CreateJsValue(env, resultCode));
    napi_set_named_property(env, object, "want", AppExecFwk::WrapWant(env, want));
    return object;
}

void JsEmbeddableUIAbilityContext::InheritWindowMode(AAFwk::Want &want)
{
    HILOG_INFO("InheritWindowMode");
#ifdef SUPPORT_GRAPHICS
    // only split mode need inherit
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_ERROR("context is nullptr.");
        return;
    }
    auto windowMode = context->GetCurrentWindowMode();
    if (windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }
    HILOG_DEBUG("window mode is %{public}d", windowMode);
#endif
}

void JsEmbeddableUIAbilityContext::AddFreeInstallObserver(napi_env env, const AAFwk::Want &want, napi_value callback,
    bool isAbilityResult)
{
    // adapter free install async return install and start result
    HILOG_DEBUG("AddFreeInstallObserver begin.");
    int ret = 0;
    if (freeInstallObserver_ == nullptr) {
        freeInstallObserver_ = new JsFreeInstallObserver(env);
        ret = AAFwk::AbilityManagerClient::GetInstance()->AddFreeInstallObserver(freeInstallObserver_);
    }

    if (ret != ERR_OK) {
        HILOG_ERROR("AddFreeInstallObserver error.");
    } else {
        HILOG_INFO("AddJsObserverObject");
        // build a callback observer with last param
        std::string bundleName = want.GetElement().GetBundleName();
        std::string abilityName = want.GetElement().GetAbilityName();
        std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
        freeInstallObserver_->AddJsObserverObject(
            bundleName, abilityName, startTime, callback, isAbilityResult);
    }
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbility(napi_env env, NapiCallbackInfo& info, bool isStartRecent)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough arguments");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want);
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = 1;
    HILOG_INFO("StartAbility, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        HILOG_DEBUG("JsEmbeddableUIAbilityContext::OnStartAbility start options is used.");
        AppExecFwk::UnwrapStartOptions(env, info.argv[INDEX_ONE], startOptions);
        unwrapArgc++;
    }

    if (isStartRecent) {
        HILOG_DEBUG("JsEmbeddableUIAbilityContext::OnStartRecentAbility is enter");
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
        if (context == nullptr) {
            HILOG_WARN("context is released");
            *innerErrorCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }

        *innerErrorCode = (unwrapArgc == 1) ?
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
            HILOG_DEBUG("StartAbility success.");
            task.Resolve(env, CreateJsUndefined(env));
        } else {
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
        }
    };

    napi_value lastParam = (info.argc > unwrapArgc) ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        AddFreeInstallObserver(env, want, lastParam);
        NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnStartAbility", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), nullptr, &result));
    } else {
        NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnStartAbility", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    }
    return result;
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough arguments");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        HILOG_ERROR("Failed to parse want!");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = 1;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        HILOG_DEBUG("JsEmbeddableUIAbilityContext::OnStartAbilityForResult begin options is used.");
        AppExecFwk::UnwrapStartOptions(env, info.argv[INDEX_ONE], startOptions);
        unwrapArgc++;
    }

    napi_value lastParam = info.argc > unwrapArgc ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count());
        want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, lastParam, true);
    }
    std::unique_ptr<NapiAsyncTask> uasyncTask =
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);
    RuntimeTask task = [env, asyncTask, &observer = freeInstallObserver_](int resultCode, const AAFwk::Want& want,
        bool isInner) {
        HILOG_DEBUG("OnStartAbilityForResult async callback is begin");
        napi_value abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            HILOG_WARN("wrap abilityResult error");
            asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
        } else {
            if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND &&
                resultCode != 0 && observer != nullptr) {
                std::string bundleName = want.GetElement().GetBundleName();
                std::string abilityName = want.GetElement().GetAbilityName();
                std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
                observer->OnInstallFinished(bundleName, abilityName, startTime,
                    static_cast<int>(GetJsErrorCodeByNativeError(resultCode)));
            } else if (isInner) {
                asyncTask->Reject(env, CreateJsErrorByNativeErr(env, resultCode));
            } else {
                asyncTask->Resolve(env, abilityResult);
            }
        }
    };
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_WARN("context is released");
        asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    } else {
        want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
        curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
        (unwrapArgc == 1) ? context->StartAbilityForResult(want, curRequestCode_, std::move(task)) :
            context->StartAbilityForResult(want, startOptions, curRequestCode_, std::move(task));
    }
    HILOG_DEBUG("OnStartAbilityForResult is called end");
    return result;
}

napi_value JsEmbeddableUIAbilityContext::OnConnectAbility(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    // only support two params
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("Connect ability failed, not enough params.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // unwrap want
    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want);
    HILOG_INFO("ConnectAbility, callee:%{public}s.%{public}s",
        want.GetBundle().c_str(),
        want.GetElement().GetAbilityName().c_str());

    // unwarp connection
    sptr<JSEmbeddableUIAbilityConnection> connection = new JSEmbeddableUIAbilityConnection(env);
    connection->SetJsConnectionObject(info.argv[INDEX_ONE]);
    int64_t connectId = g_serialNumber;
    ConnectionKey key;
    key.id = g_serialNumber;
    key.want = want;
    connection->SetConnectionId(key.id);
    g_connects.emplace(key, connection);
    if (g_serialNumber < INT32_MAX) {
        g_serialNumber++;
    } else {
        g_serialNumber = 0;
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, connection, connectId](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_ERROR("Connect ability failed, context is released.");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                RemoveConnection(connectId);
                return;
            }
            HILOG_DEBUG("ConnectAbility connection:%{public}d", static_cast<int32_t>(connectId));
            auto innerErrorCode = context->ConnectAbility(want, connection);
            int32_t errcode = static_cast<int32_t>(AbilityRuntime::GetJsErrorCodeByNativeError(innerErrorCode));
            if (errcode) {
                connection->CallJsFailed(errcode);
                RemoveConnection(connectId);
            }
            task.Resolve(env, CreateJsUndefined(env));
        };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnConnectAbility",
        env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
    return CreateJsValue(env, connectId);
}

napi_value JsEmbeddableUIAbilityContext::OnDisconnectAbility(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    // only support one or two params
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // unwrap want
    AAFwk::Want want;
    // unwrap connectId
    int64_t connectId = -1;
    sptr<JSEmbeddableUIAbilityConnection> connection = nullptr;
    napi_get_value_int64(env, info.argv[INDEX_ZERO], &connectId);
    HILOG_INFO("DisconnectAbility, connection:%{public}d.", static_cast<int32_t>(connectId));
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != g_connects.end()) {
        // match id
        want = item->first.want;
        connection = item->second;
    } else {
        HILOG_INFO("not find conn exist.");
    }
    // begin disconnect
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, connection](
            napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (context == nullptr) {
                HILOG_WARN("JsEmbeddableUIAbilityContext::OnDisconnectAbility context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            if (connection == nullptr) {
                HILOG_WARN("connection is nullptr");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            HILOG_DEBUG("context->DisconnectAbility");
            context->DisconnectAbility(want, connection);
            task.Resolve(env, CreateJsUndefined(env));
        };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsEmbeddableUIAbilityContext::OnDisconnectAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityAsCaller(napi_env env, NapiCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want);
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = 1;
    HILOG_INFO("OnStartAbilityAsCaller, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        HILOG_DEBUG("JsEmbeddableUIAbilityContext::OnStartAbilityAsCaller start options is used.");
        AppExecFwk::UnwrapStartOptions(env, info.argv[INDEX_ONE], startOptions);
        unwrapArgc++;
    }
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, startOptions, unwrapArgc](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (context == nullptr) {
                HILOG_WARN("context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto innerErrorCode = (unwrapArgc == 1) ?
                context->StartAbilityAsCaller(want, -1) : context->StartAbilityAsCaller(want, startOptions, -1);
            if (innerErrorCode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
            }
        };

    napi_value lastParam = (info.argc > unwrapArgc) ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnStartAbilityAsCaller",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    if (info.argc < ARGC_TWO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want);
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = 1;
    HILOG_INFO("StartAbilityWithAccount, ability:%{public}s", want.GetElement().GetAbilityName().c_str());
    int32_t accountId = 0;
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[INDEX_ONE], accountId)) {
        HILOG_DEBUG("the second parameter is invalid.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }
    unwrapArgc++;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_TWO && CheckTypeForNapiValue(env, info.argv[INDEX_TWO], napi_object)) {
        HILOG_DEBUG("JsEmbeddableUIAbilityContext::OnStartAbilityWithAccount start options is used.");
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
            HILOG_WARN("context is released");
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
        AddFreeInstallObserver(env, want, lastParam);
        NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnStartAbilityWithAccount", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), nullptr, &result));
    } else {
        NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnStartAbilityWithAccount", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    }
    return result;
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityByCall(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    if (info.argc < ARGC_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    if (!CheckTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_object) ||
        !AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        HILOG_ERROR("Failed to parse want!");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }
    InheritWindowMode(want);

    std::shared_ptr<StartAbilityByCallParameters> calls = std::make_shared<StartAbilityByCallParameters>();
    napi_value lastParam = nullptr;
    napi_value retsult = nullptr;
    int32_t userId = DEFAULT_INVAL_VALUE;
    if (info.argc > ARGC_ONE) {
        if (CheckTypeForNapiValue(env, info.argv[ARGC_ONE], napi_number)) {
            if (!ConvertFromJsValue(env, info.argv[ARGC_ONE], userId)) {
                HILOG_ERROR("Failed to parse accountId!");
                ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
                return CreateJsUndefined(env);
            }
        } else if (CheckTypeForNapiValue(env, info.argv[ARGC_ONE], napi_function)) {
            lastParam = info.argv[ARGC_ONE];
        } else {
            HILOG_ERROR("Failed, input param type invalid");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
    }

    if (info.argc > ARGC_TWO && CheckTypeForNapiValue(env, info.argv[ARGC_TWO], napi_function)) {
        lastParam = info.argv[ARGC_TWO];
    }

    auto callBackDone = [calldata = calls] (const sptr<IRemoteObject> &obj) {
        HILOG_DEBUG("OnStartAbilityByCall callBackDone mutexlock");
        std::unique_lock<std::mutex> lock(calldata->mutexlock);
        HILOG_DEBUG("OnStartAbilityByCall callBackDone remoteCallee assignment");
        calldata->remoteCallee = obj;
        calldata->condition.notify_all();
        HILOG_DEBUG("OnStartAbilityByCall callBackDone is called end");
    };

    auto releaseListen = [](const std::string &str) {
        HILOG_INFO("OnStartAbilityByCall releaseListen is called %{public}s", str.c_str());
    };

    auto callExecute = [calldata = calls] () {
        constexpr int CALLER_TIME_OUT = 10; // 10s
        std::unique_lock<std::mutex> lock(calldata->mutexlock);
        if (calldata->remoteCallee != nullptr) {
            HILOG_INFO("OnStartAbilityByCall callExecute callee isn`t nullptr");
            return;
        }

        if (calldata->condition.wait_for(lock, std::chrono::seconds(CALLER_TIME_OUT)) == std::cv_status::timeout) {
            HILOG_ERROR("OnStartAbilityByCall callExecute waiting callee timeout");
            calldata->err = -1;
        }
        HILOG_DEBUG("OnStartAbilityByCall callExecute end");
    };

    auto callerCallBack = std::make_shared<CallerCallBack>();
    auto callComplete = [weak = context_, calldata = calls, callerCallBack] (
        napi_env env, NapiAsyncTask& task, int32_t status) {
        if (calldata->err != 0) {
            HILOG_ERROR("OnStartAbilityByCall callComplete err is %{public}d", calldata->err);
            task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
            ClearFailedCallConnection(weak, callerCallBack);
            return;
        }

        auto context = weak.lock();
        if (context != nullptr && callerCallBack != nullptr && calldata->remoteCallee != nullptr) {
            auto releaseCallAbilityFunc = [weak] (
                const std::shared_ptr<CallerCallBack> &callback) -> ErrCode {
                auto contextForRelease = weak.lock();
                if (contextForRelease == nullptr) {
                    HILOG_ERROR("releaseCallAbilityFunction, context is nullptr");
                    return -1;
                }
                return contextForRelease->ReleaseCall(callback);
            };
            task.Resolve(env,
                CreateJsCallerComplex(
                    env, releaseCallAbilityFunc, calldata->remoteCallee, callerCallBack));
        } else {
            HILOG_ERROR("OnStartAbilityByCall callComplete params error %{public}s is nullptr",
                context == nullptr ? "context" :
                (calldata->remoteCallee == nullptr ? "remoteCallee" : "callerCallBack"));
            task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
        }

        HILOG_DEBUG("OnStartAbilityByCall callComplete end");
    };

    callerCallBack->SetCallBack(callBackDone);
    callerCallBack->SetOnRelease(releaseListen);

    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_ERROR("OnStartAbilityByCall context is nullptr");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }

    auto ret = context->StartAbilityByCall(want, callerCallBack, userId);
    if (ret != 0) {
        HILOG_ERROR("OnStartAbilityByCall StartAbility is failed");
        ThrowErrorByNativeErr(env, ret);
        return CreateJsUndefined(env);
    }

    if (calls->remoteCallee == nullptr) {
        HILOG_INFO("OnStartAbilityByCall async wait execute");
        NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnStartAbilityByCall", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(callExecute), std::move(callComplete), &retsult));
    } else {
        HILOG_INFO("OnStartAbilityByCall promiss return result execute");
        NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnStartAbilityByCall", env,
            CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(callComplete), &retsult));
    }

    HILOG_DEBUG("JsEmbeddableUIAbilityContext, called end");
    return retsult;
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityForResultWithAccount(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        HILOG_ERROR("This application is not system-app, can not use system-api");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        HILOG_ERROR("Failed to parse want!");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = 1;
    int32_t accountId = 0;
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[INDEX_ONE], accountId)) {
        HILOG_ERROR("the second parameter is invalid.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }
    unwrapArgc++;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_TWO && CheckTypeForNapiValue(env, info.argv[INDEX_TWO], napi_object)) {
        HILOG_DEBUG("OnStartAbilityForResultWithAccount start options is used.");
        AppExecFwk::UnwrapStartOptions(env, info.argv[INDEX_TWO], startOptions);
        unwrapArgc++;
    }
    napi_value lastParam = info.argc > unwrapArgc ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count());
        want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, lastParam, true);
    }
    std::unique_ptr<NapiAsyncTask> uasyncTask =
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);
    RuntimeTask task = [env, asyncTask, &observer = freeInstallObserver_](int resultCode, const AAFwk::Want& want,
        bool isInner) {
        HILOG_DEBUG("async callback is called");
        napi_value abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            HILOG_WARN("wrap abilityResult failed");
            asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
        } else {
            if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND &&
                resultCode != 0 && observer != nullptr) {
                std::string bundleName = want.GetElement().GetBundleName();
                std::string abilityName = want.GetElement().GetAbilityName();
                std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
                observer->OnInstallFinished(bundleName, abilityName, startTime,
                    static_cast<int>(GetJsErrorCodeByNativeError(resultCode)));
            } else if (isInner) {
                asyncTask->Reject(env, CreateJsErrorByNativeErr(env, resultCode));
            } else {
                asyncTask->Resolve(env, abilityResult);
            }
        }
        HILOG_DEBUG("async callback is called end");
    };
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_WARN("context is released");
        asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    } else {
        curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
        (unwrapArgc == INDEX_TWO) ? context->StartAbilityForResultWithAccount(
            want, accountId, curRequestCode_, std::move(task)) : context->StartAbilityForResultWithAccount(
                want, accountId, startOptions, curRequestCode_, std::move(task));
    }
    HILOG_DEBUG("OnStartAbilityForResultWithAccount is called end");
    return result;
}

napi_value JsEmbeddableUIAbilityContext::OnStartExtensionAbility(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    if (info.argc < ARGC_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        HILOG_ERROR("Failed to parse want!");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (context == nullptr) {
                HILOG_WARN("context is released");
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

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnStartExtensionAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsEmbeddableUIAbilityContext::OnStartExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("param is too few.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    int32_t accountId = -1;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want) ||
        !OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[INDEX_ONE], accountId)) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, accountId](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (context == nullptr) {
                HILOG_WARN("context has been released");
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

    napi_value lastParam = (info.argc > ARGC_TWO) ? info.argv[ARGC_TWO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnStartExtensionAbilityWithAccount",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsEmbeddableUIAbilityContext::OnStopExtensionAbility(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("param is too few for stop extension ability.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (context == nullptr) {
                HILOG_WARN("context is released");
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

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsEmbeddableUIAbilityContext::OnStopExtensionAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsEmbeddableUIAbilityContext::OnStopExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("param is too few for stop extension ability with account.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    int32_t accountId = -1;
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want) ||
        !AppExecFwk::UnwrapInt32FromJS2(env, info.argv[INDEX_ONE], accountId)) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, accountId](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
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

    napi_value lastParam = (info.argc > ARGC_TWO) ? info.argv[ARGC_TWO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsEmbeddableUIAbilityContext::OnStopExtensionAbilityWithAccount",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsEmbeddableUIAbilityContext::OnConnectAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    // only support three params
    if (info.argc < ARGC_THREE) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // unwrap want
    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want);
    HILOG_INFO("ConnectAbilityWithAccount, bundlename:%{public}s abilityname:%{public}s",
        want.GetBundle().c_str(),
        want.GetElement().GetAbilityName().c_str());

    int32_t accountId = 0;
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[INDEX_ONE], accountId)) {
        HILOG_ERROR("the second parameter is invalid.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    // unwarp connection
    sptr<JSEmbeddableUIAbilityConnection> connection = new JSEmbeddableUIAbilityConnection(env);
    connection->SetJsConnectionObject(info.argv[INDEX_TWO]);
    int64_t connectId = g_serialNumber;
    ConnectionKey key;
    key.id = g_serialNumber;
    key.want = want;
    connection->SetConnectionId(key.id);
    g_connects.emplace(key, connection);
    if (g_serialNumber < INT32_MAX) {
        g_serialNumber++;
    } else {
        g_serialNumber = 0;
    }
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, accountId, connection, connectId](
            napi_env env, NapiAsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_ERROR("context is released");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    RemoveConnection(connectId);
                    return;
                }
                HILOG_INFO("context->ConnectAbilityWithAccount connection:%{public}d", static_cast<int32_t>(connectId));
                auto innerErrorCode = context->ConnectAbilityWithAccount(want, accountId, connection);
                int32_t errcode = static_cast<int32_t>(AbilityRuntime::GetJsErrorCodeByNativeError(innerErrorCode));
                if (errcode) {
                    connection->CallJsFailed(errcode);
                    RemoveConnection(connectId);
                }
                task.Resolve(env, CreateJsUndefined(env));
        };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnConnectAbilityWithAccount",
        env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
    return CreateJsValue(env, connectId);
}

napi_value JsEmbeddableUIAbilityContext::OnTerminateSelf(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    auto abilityContext = context_.lock();
    if (abilityContext != nullptr) {
        abilityContext->SetTerminating(true);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (context == nullptr) {
                HILOG_WARN("context is released");
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

    napi_value lastParam = (info.argc > ARGC_ZERO) ? info.argv[ARGC_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnTerminateSelf",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsEmbeddableUIAbilityContext::OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    int resultCode = 0;
    AAFwk::Want want;
    if (!AppExecFwk::UnWrapAbilityResult(env, info.argv[INDEX_ZERO], resultCode, want)) {
        HILOG_ERROR("%s Failed to parse ability result!", __func__);
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    auto abilityContext = context_.lock();
    if (abilityContext != nullptr) {
        abilityContext->SetTerminating(true);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, resultCode](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (context == nullptr) {
                HILOG_WARN("context is released");
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

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnTerminateSelfWithResult",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    HILOG_DEBUG("OnTerminateSelfWithResult is called end");
    return result;
}

napi_value JsEmbeddableUIAbilityContext::OnRestoreWindowStage(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("RestoreWindowStage, info.argc = %{public}d", static_cast<int>(info.argc));
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("OnRestoreWindowStage need one parameters");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_ERROR("context is released");
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

napi_value JsEmbeddableUIAbilityContext::OnIsTerminating(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_ERROR("OnIsTerminating context is nullptr");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }
    return CreateJsValue(env, context->IsTerminating());
}

napi_value JsEmbeddableUIAbilityContext::OnStartRecentAbility(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    return OnStartAbility(env, info, true);
}

napi_value JsEmbeddableUIAbilityContext::OnRequestDialogService(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want);
    HILOG_INFO("RequestDialogService, target:%{public}s.%{public}s.", want.GetBundle().c_str(),
        want.GetElement().GetAbilityName().c_str());

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
    napi_value result = nullptr;
    auto uasyncTask = CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);
    RequestDialogResultTask task =
        [env, asyncTask](int32_t resultCode, const AAFwk::Want &resultWant) {
        napi_value requestResult = JsEmbeddableUIAbilityContext::WrapRequestDialogResult(env, resultCode, resultWant);
        if (requestResult == nullptr) {
            HILOG_WARN("wrap requestResult failed");
            asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
        } else {
            asyncTask->Resolve(env, requestResult);
        }
        HILOG_DEBUG("OnRequestDialogService async callback is called end");
    };
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_WARN("context is released, can not requestDialogService");
        asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    } else {
        auto errCode = context->RequestDialogService(env, want, std::move(task));
        if (errCode != ERR_OK) {
            asyncTask->Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(errCode)));
        }
    }
    HILOG_DEBUG("OnRequestDialogService is called end");
    return result;
}

napi_value JsEmbeddableUIAbilityContext::OnReportDrawnCompleted(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, innerErrorCode]() {
        auto context = weak.lock();
        if (context == nullptr) {
            HILOG_WARN("context is released");
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

    napi_value lastParam = info.argv[ARGC_ZERO];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnReportDrawnCompleted",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsEmbeddableUIAbilityContext::OnSetMissionContinueState(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("OnSetMissionContinueState, info.argc = %{public}d", static_cast<int>(info.argc));
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("OnSetMissionContinueState, Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::ContinueState state;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], state)) {
        HILOG_ERROR("OnSetMissionContinueState, parse state failed.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    if (state <= AAFwk::ContinueState::CONTINUESTATE_UNKNOWN || state >= AAFwk::ContinueState::CONTINUESTATE_MAX) {
        HILOG_ERROR("OnSetMissionContinueState, invalid state param, state = %{public}d.", state);
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, state](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (context == nullptr) {
                HILOG_WARN("context is released");
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

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::SetMissionContinueState",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityByType(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("JsEmbeddableUIAbilityContext::%{public}s, called", __func__);
    if (info.argc < ARGC_THREE) {
        HILOG_ERROR("OnStartAbilityByType, Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string type;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], type)) {
        HILOG_ERROR("OnStartAbilityByType, parse type failed.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    AAFwk::WantParams wantParam;
    if (!AppExecFwk::UnwrapWantParams(env, info.argv[INDEX_ONE], wantParam)) {
        HILOG_ERROR("OnStartAbilityByType, parse wantParam failed.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<JsUIExtensionCallback> callback = std::make_shared<JsUIExtensionCallback>(env);
    callback->SetJsCallbackObject(info.argv[INDEX_TWO]);
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, type, wantParam, callback](napi_env env, NapiAsyncTask& task, int32_t status) mutable {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("OnStartAbilityByType context is released");
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
    NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnStartAbilityByType",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

#ifdef SUPPORT_GRAPHICS
napi_value JsEmbeddableUIAbilityContext::SetMissionLabel(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnSetMissionLabel);
}

napi_value JsEmbeddableUIAbilityContext::SetMissionIcon(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnSetMissionIcon);
}

napi_value JsEmbeddableUIAbilityContext::OnSetMissionLabel(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("SetMissionLabel, info.argc = %{public}d", static_cast<int>(info.argc));
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("OnSetMissionLabel, Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string label;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], label)) {
        HILOG_ERROR("OnSetMissionLabel, parse label failed.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, label](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
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

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnSetMissionLabel",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsEmbeddableUIAbilityContext::OnSetMissionIcon(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("SetMissionIcon, info.argc = %{public}d", static_cast<int>(info.argc));
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("OnSetMissionIcon, Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    auto icon = OHOS::Media::PixelMapNapi::GetPixelMap(env, info.argv[INDEX_ZERO]);
    if (!icon) {
        HILOG_ERROR("OnSetMissionIcon, parse icon failed.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, icon](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released when set mission icon");
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

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsEmbeddableUIAbilityContext::OnSetMissionIcon",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}
#endif

JSEmbeddableUIAbilityConnection::JSEmbeddableUIAbilityConnection(napi_env env) : env_(env) {}

JSEmbeddableUIAbilityConnection::~JSEmbeddableUIAbilityConnection()
{
    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("~JSEmbeddableUIAbilityConnection: failed to get uv loop.");
        return;
    }

    ConnectCallback *cb = new (std::nothrow) ConnectCallback();
    if (cb == nullptr) {
        HILOG_ERROR("~JSEmbeddableUIAbilityConnection: failed to create cb.");
        return;
    }
    cb->jsConnectionObject_ = std::move(jsConnectionObject_);

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOG_ERROR("~JSEmbeddableUIAbilityConnection: failed to create work.");
        delete cb;
        cb = nullptr;
        return;
    }
    work->data = reinterpret_cast<void *>(cb);
    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
    [](uv_work_t *work, int status) {
        if (work == nullptr) {
            HILOG_ERROR("~JSEmbeddableUIAbilityConnection: work is nullptr.");
            return;
        }
        if (work->data == nullptr) {
            HILOG_ERROR("~JSEmbeddableUIAbilityConnection: data is nullptr.");
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

void JSEmbeddableUIAbilityConnection::SetJsConnectionObject(napi_value jsConnectionObject)
{
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsConnectionObject, 1, &ref);
    jsConnectionObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
}

void JSEmbeddableUIAbilityConnection::RemoveConnectionObject()
{
    jsConnectionObject_.reset();
}

void JSEmbeddableUIAbilityConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

napi_value JSEmbeddableUIAbilityConnection::ConvertElement(const AppExecFwk::ElementName &element)
{
    return AppExecFwk::WrapElementName(env_, element);
}

void JSEmbeddableUIAbilityConnection::CallJsFailed(int32_t errorCode)
{
    HILOG_INFO("CallJsFailed");
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ is nullptr");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onFailed", &method);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onFailed from object");
        return;
    }

    napi_value argv[] = {CreateJsValue(env_, errorCode)};
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
    HILOG_DEBUG("CallJsFailed end");
}

void JSEmbeddableUIAbilityConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("OnAbilityConnectDone, resultCode:%{public}d", resultCode);
    wptr<JSEmbeddableUIAbilityConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, remoteObject, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSEmbeddableUIAbilityConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                HILOG_ERROR("connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
        });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSEmbeddableUIAbilityConnection::OnAbilityConnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSEmbeddableUIAbilityConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("HandleOnAbilityConnectDone, resultCode:%{public}d", resultCode);
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        HILOG_ERROR("Failed to get object");
        return;
    }
    napi_value methodOnConnect = nullptr;
    napi_get_named_property(env_, obj, "onConnect", &methodOnConnect);
    if (methodOnConnect == nullptr) {
        HILOG_ERROR("Failed to get onConnect from object");
        return;
    }

    // wrap RemoteObject
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(env_, remoteObject);
    napi_value argv[] = { ConvertElement(element), napiRemoteObject };
    napi_call_function(env_, obj, methodOnConnect, ARGC_TWO, argv, nullptr);
    HILOG_DEBUG("OnAbilityConnectDone end");
}

void JSEmbeddableUIAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    HILOG_INFO("OnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    wptr<JSEmbeddableUIAbilityConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSEmbeddableUIAbilityConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                HILOG_INFO("connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSEmbeddableUIAbilityConnection::OnAbilityDisconnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSEmbeddableUIAbilityConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    HILOG_INFO("HandleOnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }

    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        HILOG_ERROR("Wrong to get object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onDisconnect", &method);
    if (method == nullptr) {
        HILOG_ERROR("Wrong to get onDisconnect from object");
        return;
    }

    // release connect
    HILOG_DEBUG("OnAbilityDisconnectDone g_connects.size:%{public}zu", g_connects.size());
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
        HILOG_DEBUG("OnAbilityDisconnectDone erase g_connects.size:%{public}zu", g_connects.size());
    }

    napi_value argv[] = { ConvertElement(element) };
    HILOG_DEBUG("OnAbilityDisconnectDone CallFunction success");
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
