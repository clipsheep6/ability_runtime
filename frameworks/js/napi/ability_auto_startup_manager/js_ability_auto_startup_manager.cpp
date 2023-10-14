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
#include "js_ability_auto_startup_manager.h"

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
constexpr const char *ON_OFF_TYPE_SYSTEM = "systemAutoStartup";
} // namespace

void JsAbilityAutoStartupManager::Finalizer(NativeEngine *engine, void *data, void *hint)
{
    HILOG_DEBUG("Called.");
    delete static_cast<JsAbilityAutoStartupManager *>(data);
}

NativeValue *JsAbilityAutoStartupManager::RegisterAutoStartupCallback(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsAbilityAutoStartupManager *me = CheckParamsAndGetThis<JsAbilityAutoStartupManager>(engine, info);
    return (me != nullptr) ? me->OnRegisterAutoStartupCallback(*engine, *info) : nullptr;
}

NativeValue *JsAbilityAutoStartupManager::UnregisterAutoStartupCallback(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsAbilityAutoStartupManager *me = CheckParamsAndGetThis<JsAbilityAutoStartupManager>(engine, info);
    return (me != nullptr) ? me->OnUnregisterAutoStartupCallback(*engine, *info) : nullptr;
}

NativeValue *JsAbilityAutoStartupManager::SetApplicationAutoStartup(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsAbilityAutoStartupManager *me = CheckParamsAndGetThis<JsAbilityAutoStartupManager>(engine, info);
    return (me != nullptr) ? me->OnSetApplicationAutoStartup(*engine, *info) : nullptr;
}

NativeValue *JsAbilityAutoStartupManager::CancelApplicationAutoStartup(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsAbilityAutoStartupManager *me = CheckParamsAndGetThis<JsAbilityAutoStartupManager>(engine, info);
    return (me != nullptr) ? me->OnCancelApplicationAutoStartup(*engine, *info) : nullptr;
}

NativeValue *JsAbilityAutoStartupManager::QueryAllAutoStartupApplications(
    NativeEngine *engine, NativeCallbackInfo *info)
{
    JsAbilityAutoStartupManager *me = CheckParamsAndGetThis<JsAbilityAutoStartupManager>(engine, info);
    return (me != nullptr) ? me->OnQueryAllAutoStartupApplications(*engine, *info) : nullptr;
}

NativeValue *JsAbilityAutoStartupManager::OnRegisterAutoStartupCallback(
    NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("Called.");

    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    std::string type;
    if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], type) || type != ON_OFF_TYPE_SYSTEM) {
        HILOG_ERROR("Parse type failed.");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    if (!CheckCallerIsSystemApp()) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return engine.CreateUndefined();
    }

    if (jsAutoStartupCallback_ == nullptr) {
        jsAutoStartupCallback_ = new (std::nothrow) JsAbilityAutoStartupCallBack(engine);
        if (jsAutoStartupCallback_ == nullptr) {
            HILOG_ERROR("jsAutoStartupCallback_ is nullptr.");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
            return engine.CreateUndefined();
        }

        auto ret =
            AbilityManagerClient::GetInstance()->RegisterAutoStartupSystemCallback(jsAutoStartupCallback_->AsObject());
        if (ret != ERR_OK) {
            HILOG_ERROR("Register auto start up listener error[%{public}d].", ret);
            if (ret == CHECK_PERMISSION_FAILED) {
                ThrowNoPermissionError(engine, PermissionConstants::PERMISSION_APP_BOOT_MANAGEMENT_CAPABILIT);
            } else {
                ThrowError(engine, GetJsErrorCodeByNativeError(ret));
            }
        }
    }

    jsAutoStartupCallback_->Register(info.argv[INDEX_ONE]);
    return engine.CreateUndefined();
}

NativeValue *JsAbilityAutoStartupManager::OnUnregisterAutoStartupCallback(
    NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("Called.");

    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    std::string type;
    if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], type) || type != ON_OFF_TYPE_SYSTEM) {
        HILOG_ERROR("Failed to parse type.");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    if (!CheckCallerIsSystemApp()) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return engine.CreateUndefined();
    }

    if (jsAutoStartupCallback_ == nullptr) {
        HILOG_ERROR("jsAutoStartupCallback_ is nullptr.");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
        return engine.CreateUndefined();
    }
    jsAutoStartupCallback_->UnRegister(info.argv[INDEX_ONE]);

    if (jsAutoStartupCallback_->IsCallbacksEmpty()) {
        auto ret = AbilityManagerClient::GetInstance()->UnregisterAutoStartupSystemCallback(
            jsAutoStartupCallback_->AsObject());
        if (ret != ERR_OK) {
            if (ret == CHECK_PERMISSION_FAILED) {
                ThrowNoPermissionError(engine, PermissionConstants::PERMISSION_APP_BOOT_MANAGEMENT_CAPABILIT);
            } else {
                ThrowError(engine, GetJsErrorCodeByNativeError(ret));
            }
        }
        jsAutoStartupCallback_ = nullptr;
    }

    return engine.CreateUndefined();
}

NativeValue *JsAbilityAutoStartupManager::OnSetApplicationAutoStartup(
    NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("Called.");

    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    if (!CheckCallerIsSystemApp()) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return engine.CreateUndefined();
    }

    AutoStartupInfo autoStartUpInfo;
    if (!UnwrapAutoStartupInfo(engine, info.argv[INDEX_ZERO], autoStartUpInfo)) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [autoStartUpInfo](NativeEngine &engine, AsyncTask &task, int32_t status) {
        auto ret = AbilityManagerClient::GetInstance()->SetApplicationAutoStartup(autoStartUpInfo);
        if (ret == 0) {
            task.ResolveWithNoError(engine, engine.CreateUndefined());
        } else {
            HILOG_ERROR("Failed error:%{public}d.", ret);
            task.Reject(engine, CreateJsError(engine, GetJsErrorCodeByNativeError(ret)));
        }
    };

    NativeValue *lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsAbilityAutoStartupManager::OnSetApplicationAutoStartup", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsAbilityAutoStartupManager::OnCancelApplicationAutoStartup(
    NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("Called.");

    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    if (!CheckCallerIsSystemApp()) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return engine.CreateUndefined();
    }

    AutoStartupInfo autoStartUpInfo;
    if (!UnwrapAutoStartupInfo(engine, info.argv[INDEX_ZERO], autoStartUpInfo)) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [autoStartUpInfo](NativeEngine &engine, AsyncTask &task, int32_t status) {
        auto ret = AbilityManagerClient::GetInstance()->CancelApplicationAutoStartup(autoStartUpInfo);
        if (ret == 0) {
            task.ResolveWithNoError(engine, engine.CreateUndefined());
        } else {
            HILOG_ERROR("Failed error:%{public}d.", ret);
            task.Reject(engine, CreateJsError(engine, GetJsErrorCodeByNativeError(ret)));
        }
    };

    NativeValue *lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsAbilityAutoStartupManager::OnCancelApplicationAutoStartup", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsAbilityAutoStartupManager::OnQueryAllAutoStartupApplications(
    NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("Called.");

    if (!CheckCallerIsSystemApp()) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [](NativeEngine &engine, AsyncTask &task, int32_t status) {
        std::vector<AutoStartupInfo> infoList;
        auto ret = AbilityManagerClient::GetInstance()->QueryAllAutoStartupApplications(infoList);
        if (ret == 0) {
            task.ResolveWithNoError(engine, CreateJsAutoStartupInfoArray(engine, infoList));
        } else {
            HILOG_ERROR("Failed error:%{public}d.", ret);
            task.Reject(engine, CreateJsError(engine, GetJsErrorCodeByNativeError(ret)));
        }
    };

    NativeValue *lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsAbilityAutoStartupManager::OnCancelApplicationAutoStartup", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

bool JsAbilityAutoStartupManager::CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        HILOG_ERROR("Current app is not system app, not allow.");
        return false;
    }
    return true;
}

NativeValue *JsAbilityAutoStartupManagerInit(NativeEngine *engine, NativeValue *exportObj)
{
    HILOG_DEBUG("Called.");
    if (engine == nullptr || exportObj == nullptr) {
        HILOG_ERROR("engine or exportObj nullptr.");
        return nullptr;
    }

    NativeObject *object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr.");
        return nullptr;
    }

    std::unique_ptr<JsAbilityAutoStartupManager> jsAbilityAutoStartupManager =
        std::make_unique<JsAbilityAutoStartupManager>();
    object->SetNativePointer(jsAbilityAutoStartupManager.release(), JsAbilityAutoStartupManager::Finalizer, nullptr);

    const char *moduleName = "JsAbilityAutoStartupManager";
    BindNativeFunction(*engine, *object, "on", moduleName, JsAbilityAutoStartupManager::RegisterAutoStartupCallback);
    BindNativeFunction(*engine, *object, "off", moduleName, JsAbilityAutoStartupManager::UnregisterAutoStartupCallback);
    BindNativeFunction(*engine, *object, "setApplicationAutoStartup", moduleName,
        JsAbilityAutoStartupManager::SetApplicationAutoStartup);
    BindNativeFunction(*engine, *object, "cancelApplicationAutoStartup", moduleName,
        JsAbilityAutoStartupManager::CancelApplicationAutoStartup);
    BindNativeFunction(*engine, *object, "queryAllAutoStartupApplications", moduleName,
        JsAbilityAutoStartupManager::QueryAllAutoStartupApplications);
    HILOG_DEBUG("End.");
    return engine->CreateUndefined();
}
} // namespace AbilityRuntime
} // namespace OHOS