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
#include "js_application_context_auto_startup_manager.h"

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "auto_startup_info.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "js_ability_auto_startup_manager_utils.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "permission_constants.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AAFwk;
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr const char *ON_OFF_TYPE_ABILITY = "abilityAutoStartup";
} // namespace

void JsApplicationContextAutoStartupManager::Finalizer(NativeEngine *engine, void *data, void *hint)
{
    HILOG_DEBUG("Called.");
    delete static_cast<JsApplicationContextAutoStartupManager *>(data);
}

NativeValue *JsApplicationContextAutoStartupManager::RegisterAutoStartupCallback(
    NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextAutoStartupManager *me =
        CheckParamsAndGetThis<JsApplicationContextAutoStartupManager>(engine, info);
    return (me != nullptr) ? me->OnRegisterAutoStartupCallback(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextAutoStartupManager::UnregisterAutoStartupCallback(
    NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextAutoStartupManager *me =
        CheckParamsAndGetThis<JsApplicationContextAutoStartupManager>(engine, info);
    return (me != nullptr) ? me->OnUnregisterAutoStartupCallback(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextAutoStartupManager::SetAutoStartup(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextAutoStartupManager *me =
        CheckParamsAndGetThis<JsApplicationContextAutoStartupManager>(engine, info);
    return (me != nullptr) ? me->OnSetAutoStartup(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextAutoStartupManager::CancelAutoStartup(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextAutoStartupManager *me =
        CheckParamsAndGetThis<JsApplicationContextAutoStartupManager>(engine, info);
    return (me != nullptr) ? me->OnCancelAutoStartup(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextAutoStartupManager::IsAutoStartup(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextAutoStartupManager *me =
        CheckParamsAndGetThis<JsApplicationContextAutoStartupManager>(engine, info);
    return (me != nullptr) ? me->OnIsAutoStartup(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextAutoStartupManager::OnRegisterAutoStartupCallback(
    NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("Called.");

    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    std::string type;
    if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], type) || type != ON_OFF_TYPE_ABILITY) {
        HILOG_ERROR("Parse type failed.");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    if (jsAutoStartupCallback_ == nullptr) {
        jsAutoStartupCallback_ = new (std::nothrow) JsAbilityAutoStartupCallBack(engine);
        if (jsAutoStartupCallback_ == nullptr) {
            HILOG_ERROR("jsAutoStartupCallback_ is nullptr.");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
            return engine.CreateUndefined();
        }

        auto ret = AbilityManagerClient::GetInstance()->RegisterAutoStartupCallback(jsAutoStartupCallback_->AsObject());
        if (ret != ERR_OK) {
            HILOG_ERROR("Register auto start up listener error[%{public}d].", ret);
            ThrowError(engine, GetJsErrorCodeByNativeError(ret));
            return engine.CreateUndefined();
        }
    }

    jsAutoStartupCallback_->Register(info.argv[INDEX_ONE]);
    return engine.CreateUndefined();
}

NativeValue *JsApplicationContextAutoStartupManager::OnUnregisterAutoStartupCallback(
    NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("Called.");

    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    std::string type;
    if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], type) || type != ON_OFF_TYPE_ABILITY) {
        HILOG_ERROR("Failed to parse type.");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    if (jsAutoStartupCallback_ == nullptr) {
        HILOG_ERROR("jsAutoStartupCallback_ is nullptr.");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
        return engine.CreateUndefined();
    }
    jsAutoStartupCallback_->UnRegister(info.argv[INDEX_ONE]);

    if (jsAutoStartupCallback_->IsCallbacksEmpty()) {
        auto ret =
            AbilityManagerClient::GetInstance()->UnregisterAutoStartupCallback(jsAutoStartupCallback_->AsObject());
        if (ret != ERR_OK) {
            ThrowError(engine, GetJsErrorCodeByNativeError(ret));
        }
        jsAutoStartupCallback_ = nullptr;
    }
    return engine.CreateUndefined();
}

NativeValue *JsApplicationContextAutoStartupManager::OnSetAutoStartup(
    NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("Called.");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    AutoStartupInfo autoStartUpInfo;
    if (!UnwrapAutoStartupInfo(engine, info.argv[INDEX_ZERO], autoStartUpInfo)) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [autoStartUpInfo](NativeEngine &engine, AsyncTask &task, int32_t status) {
        auto ret = AbilityManagerClient::GetInstance()->SetAutoStartup(autoStartUpInfo);
        if (ret == 0) {
            task.ResolveWithNoError(engine, engine.CreateUndefined());
        } else {
            HILOG_ERROR("Failed error:%{public}d.", ret);
            task.Reject(engine, CreateJsError(engine, GetJsErrorCodeByNativeError(ret)));
        }
    };

    NativeValue *lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsApplicationContextAutoStartupManager::OnSetAutoStartup", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsApplicationContextAutoStartupManager::OnCancelAutoStartup(
    NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("Called.");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    AutoStartupInfo autoStartUpInfo;
    if (!UnwrapAutoStartupInfo(engine, info.argv[INDEX_ZERO], autoStartUpInfo)) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [autoStartUpInfo](NativeEngine &engine, AsyncTask &task, int32_t status) {
        auto ret = AbilityManagerClient::GetInstance()->CancelAutoStartup(autoStartUpInfo);
        if (ret == 0) {
            task.ResolveWithNoError(engine, engine.CreateUndefined());
        } else {
            HILOG_ERROR("Failed error:%{public}d.", ret);
            task.Reject(engine, CreateJsError(engine, GetJsErrorCodeByNativeError(ret)));
        }
    };

    NativeValue *lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsApplicationContextAutoStartupManager::OnCancelAutoStartup", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsApplicationContextAutoStartupManager::OnIsAutoStartup(
    NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("Called.");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    AutoStartupInfo autoStartUpInfo;
    if (!UnwrapAutoStartupInfo(engine, info.argv[INDEX_ZERO], autoStartUpInfo)) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [autoStartUpInfo](NativeEngine &engine, AsyncTask &task, int32_t status) {
        bool isAutoStartup = false;
        auto ret = AbilityManagerClient::GetInstance()->IsAutoStartup(autoStartUpInfo, isAutoStartup);
        if (ret == 0) {
            task.Resolve(engine, CreateJsValue(engine, isAutoStartup));
        } else {
            HILOG_ERROR("Failed error:%{public}d.", ret);
            task.Reject(engine, CreateJsError(engine, GetJsErrorCodeByNativeError(ret)));
        }
    };

    NativeValue *lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsApplicationContextAutoStartupManager::OnIsAutoStartup", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}
} // namespace AbilityRuntime
} // namespace OHOS