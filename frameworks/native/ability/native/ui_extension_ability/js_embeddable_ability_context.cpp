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

#include "js_embeddable_ability_context.h"

#include <cstdint>

#include "event_handler.h"
#include "hilog_wrapper.h"
#include "js_extension_context.h"
#include "js_error_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_ability.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "napi_common_start_options.h"
#include "napi_remote_object.h"
#include "start_options.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;

namespace {
int64_t g_serialNumber = 0;
static std::map<ConnectionKey, sptr<JSEmbeddableAbilityConnection>, KeyCompare> g_connects;

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

void JsEmbeddableAbilityContext::Finalizer(napi_env env, void* data, void* hint)
{
    HILOG_DEBUG("JsEmbeddableAbilityContext Finalizer is called");
    std::unique_ptr<JsEmbeddableAbilityContext>(static_cast<JsEmbeddableAbilityContext*>(data));
}
// 正常流程实现
napi_value JsEmbeddableAbilityContext::StartAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnStartAbility);  //ql 301
}

napi_value JsEmbeddableAbilityContext::StartAbilityForResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnStartAbilityForResult);  //ql 378
}

napi_value JsEmbeddableAbilityContext::ConnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnConnectAbility);  //ql 449
}

napi_value JsEmbeddableAbilityContext::DisconnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnDisconnectAbility);  //ql 505
}
//ql  暂时不支持 用返回errcode形式处理
napi_value JsEmbeddableAbilityContext::StartAbilityAsCaller(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnStartAbilityAsCaller);  //ql 560
}

napi_value JsEmbeddableAbilityContext::StartAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnStartAbilityWithAccount);  //ql 605
}

napi_value JsEmbeddableAbilityContext::StartAbilityByCall(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnStartAbilityByCall);
}

napi_value JsEmbeddableAbilityContext::StartAbilityByCallWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnStartAbilityByCallWithAccount);
}

napi_value JsEmbeddableAbilityContext::StartAbilityForResultWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnStartAbilityForResultWithAccount);
}

napi_value JsEmbeddableAbilityContext::StartServiceExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnStartServiceExtensionAbility);
}

napi_value JsEmbeddableAbilityContext::StartServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnStartServiceExtensionAbilityWithAccount);
}

napi_value JsEmbeddableAbilityContext::StopServiceExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnStopServiceExtensionAbility);
}

napi_value JsEmbeddableAbilityContext::StopServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnStopServiceExtensionAbilityWithAccount);
}

napi_value JsEmbeddableAbilityContext::ConnectAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnConnectAbilityWithAccount);
}

napi_value JsEmbeddableAbilityContext::TerminateSelf(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnTerminateSelf);
}

napi_value JsEmbeddableAbilityContext::TerminateSelfWithResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnTerminateSelfWithResult);
}

napi_value JsEmbeddableAbilityContext::RestoreWindowStage(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnRestoreWindowStage);
}

napi_value JsEmbeddableAbilityContext::IsTerminating(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnIsTerminating);
}

napi_value JsEmbeddableAbilityContext::StartRecentAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnStartRecentAbility);
}

napi_value JsEmbeddableAbilityContext::RequestDialogService(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnRequestDialogService);
}

napi_value JsEmbeddableAbilityContext::ReportDrawnCompleted(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnReportDrawnCompleted);
}

napi_value JsEmbeddableAbilityContext::SetMissionContinueState(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnSetMissionContinueState);
}

napi_value JsEmbeddableAbilityContext::StartAbilityByType(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnStartAbilityByType);
}

napi_value JsEmbeddableAbilityContext::SetMissionLabel(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnSetMissionLabel);
}

napi_value JsEmbeddableAbilityContext::SetMissionIcon(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableAbilityContext, OnSetMissionIcon);
}

napi_value JsEmbeddableAbilityContext::CreateJsEmbeddableAbilityContext(napi_env env,
    std::shared_ptr<AbilityContext> context)
{
    HILOG_DEBUG("CreateJsEmbeddableAbilityContext begin");
    napi_value objValue = CreateJsBaseContext(env, context);

    std::unique_ptr<JsEmbeddableAbilityContext> jsContext = std::make_unique<JsEmbeddableAbilityContext>(context);
    napi_wrap(env, objValue, jsContext.release(), Finalizer, nullptr, nullptr);
    //ql 缺少abilityinfo  configinfo等信息的构建补充   ？？？？？？
    auto resourceManager = context->GetResourceManager();
    if (resourceManager != nullptr) {
        napi_set_named_property(env, object, "resourceManager",
            CreateJsResourceManager(env, resourceManager, context));
    }

    auto abilityInfo = context->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        napi_set_named_property(
            env, object, "abilityInfo", CreateJsAbilityInfo(env, *abilityInfo));
    }

    auto configuration = context->GetConfiguration();
    if (configuration != nullptr) {
        napi_set_named_property(
            env, object, "config", CreateJsConfiguration(env, *configuration));
    }
    //ql ？？？？？？？？？
    const char *moduleName = "JsEmbeddableAbilityContext";
    BindNativeFunction(env, objValue, "startAbility", moduleName, StartAbility);  //ql 已实现
    BindNativeFunction(env, objValue, "startAbilityForResult", moduleName, StartAbilityForResult);  //ql 已实现
    BindNativeFunction(env, objValue, "connectServiceExtensionAbility", moduleName, ConnectAbility);  //ql 已实现
    BindNativeFunction(env, objValue, "disconnectServiceExtensionAbility", moduleName, DisconnectAbility);  //ql 已实现
    // 返回errcode形式处理
    BindNativeFunction(env, objValue, "startAbilityAsCaller", moduleName, StartAbilityAsCaller);  //ql 已实现
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
    BindNativeFunction(env, objValue, "setMissionLabel", moduleName, SetMissionLabel); //ql 新增
    BindNativeFunction(env, objValue, "setMissionIcon", moduleName, SetMissionIcon);  //ql 新增
    BindNativeFunction(env, objValue, "startAbilityByCallWithAccount", moduleName, StartAbilityByCallWithAccount);  //ql 新增

    return objValue;
}

void JsEmbeddableAbilityContext::InheritWindowMode(AAFwk::Want &want)
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

void JsEmbeddableAbilityContext::AddFreeInstallObserver(napi_env env, const AAFwk::Want &want, napi_value callback,
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

napi_value JsEmbeddableAbilityContext::OnStartAbility(napi_env env, NapiCallbackInfo& info, bool isStartRecent)
{
    StartAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, TRACE_ATOMIC_SERVICE, TRACE_ATOMIC_SERVICE_ID);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough arguments");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want);
    InheritWindowMode(want);  //ql  作用是啥？？？
    decltype(info.argc) unwrapArgc = 1;
    HILOG_INFO("StartAbility, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        HILOG_DEBUG("JsEmbeddableAbilityContext::OnStartAbility start options is used.");
        AppExecFwk::UnwrapStartOptions(env, info.argv[INDEX_ONE], startOptions);
        unwrapArgc++;
    }

    if (isStartRecent) {
        HILOG_DEBUG("JsEmbeddableAbilityContext::OnStartRecentAbility is enter");
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
        AddFreeInstallObserver(env, want, lastParam);  //ql  什么时候执行，目的是什么？？？
        NapiAsyncTask::ScheduleHighQos("JsEmbeddableAbilityContext::OnStartAbility", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), nullptr, &result));
    } else {
        NapiAsyncTask::ScheduleHighQos("JsEmbeddableAbilityContext::OnStartAbility", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    }
    return result;
}

napi_value JsEmbeddableAbilityContext::OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("OnStartAbilityForResult is called");

    if (info.argc == ARGC_ZERO) {
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
        HILOG_DEBUG("JsEmbeddableAbilityContext::OnStartAbilityForResult begin options is used.");
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

napi_value JsEmbeddableAbilityContext::OnConnectAbility(napi_env env, NapiCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
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
    sptr<JSEmbeddableAbilityConnection> connection = new JSEmbeddableAbilityConnection(env);
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
    NapiAsyncTask::ScheduleHighQos("JsEmbeddableAbilityContext::OnConnectAbility",
        env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
    return CreateJsValue(env, connectId);
}

napi_value JsEmbeddableAbilityContext::OnDisconnectAbility(napi_env env, NapiCallbackInfo& info)
{
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
    sptr<JSEmbeddableAbilityConnection> connection = nullptr;
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
                HILOG_WARN("JsEmbeddableAbilityContext::OnDisconnectAbility context is released");
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
    NapiAsyncTask::Schedule("JsEmbeddableAbilityContext::OnDisconnectAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

//ql 以下函数errorcode返回形式处理
napi_value JsEmbeddableAbilityContext::OnStartAbilityAsCaller(napi_env env, NapiCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want);
    InheritWindowMode(want);  //ql 不确定其做作用????
    decltype(info.argc) unwrapArgc = 1;
    HILOG_INFO("OnStartAbilityAsCaller, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        HILOG_DEBUG("JsEmbeddableAbilityContext::OnStartAbilityAsCaller start options is used.");
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
    NapiAsyncTask::ScheduleHighQos("JsEmbeddableAbilityContext::OnStartAbilityAsCaller",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsEmbeddableAbilityContext::OnStartAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
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
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[1], accountId)) {
        HILOG_DEBUG("the second parameter is invalid.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }
    unwrapArgc++;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_TWO && CheckTypeForNapiValue(env, info.argv[INDEX_TWO], napi_object)) {
        HILOG_DEBUG("JsEmbeddableAbilityContext::OnStartAbilityWithAccount start options is used.");
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
        NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnStartAbilityWithAccount", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), nullptr, &result));
    } else {
        NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnStartAbilityWithAccount", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    }
    return result;
}
//ql 
JSEmbeddableAbilityConnection::JSEmbeddableAbilityConnection(napi_env env) : env_(env) {}

JSEmbeddableAbilityConnection::~JSEmbeddableAbilityConnection()
{
    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("~JSEmbeddableAbilityConnection: failed to get uv loop.");
        return;
    }

    ConnectCallback *cb = new (std::nothrow) ConnectCallback();
    if (cb == nullptr) {
        HILOG_ERROR("~JSEmbeddableAbilityConnection: failed to create cb.");
        return;
    }
    cb->jsConnectionObject_ = std::move(jsConnectionObject_);

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOG_ERROR("~JSEmbeddableAbilityConnection: failed to create work.");
        delete cb;
        cb = nullptr;
        return;
    }
    work->data = reinterpret_cast<void *>(cb);
    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
    [](uv_work_t *work, int status) {
        if (work == nullptr) {
            HILOG_ERROR("~JSEmbeddableAbilityConnection: work is nullptr.");
            return;
        }
        if (work->data == nullptr) {
            HILOG_ERROR("~JSEmbeddableAbilityConnection: data is nullptr.");
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

void JSEmbeddableAbilityConnection::SetJsConnectionObject(napi_value jsConnectionObject)
{
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsConnectionObject, 1, &ref);
    jsConnectionObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
}

void JSEmbeddableAbilityConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

void JSEmbeddableAbilityConnection::CallJsFailed(int32_t errorCode)
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
}  // namespace AbilityRuntime
}  // namespace OHOS
