/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "js_ability_manager.h"

#include <cstdint>
#include <memory>

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "acquire_share_data_callback_stub.h"
#include "app_mgr_interface.h"
#include "errors.h"
#include "hilog_wrapper.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "js_ability_manager_utils.h"
#include "event_runner.h"
#include "napi_common_configuration.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "tokenid_kit.h"
#include "js_api_utils.h"

namespace OHOS {
namespace AbilityRuntime {
using AbilityManagerClient = AAFwk::AbilityManagerClient;
namespace {
OHOS::sptr<OHOS::AppExecFwk::IAppMgr> GetAppManagerInstance()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> appObject = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(appObject);
}


constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
static std::shared_ptr<AppExecFwk::EventHandler> mainHandler_ = nullptr;

class JsAbilityManager final {
public:
    JsAbilityManager() = default;
    ~JsAbilityManager() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsAbilityManager::Finalizer is called");
        std::unique_ptr<JsAbilityManager>(static_cast<JsAbilityManager*>(data));
    }

    static NativeValue* GetAbilityRunningInfos(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAbilityManager* me = CheckParamsAndGetThis<JsAbilityManager>(engine, info);
        return (me != nullptr) ? me->OnGetAbilityRunningInfos(*engine, *info) : nullptr;
    }

    static NativeValue* GetExtensionRunningInfos(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAbilityManager* me = CheckParamsAndGetThis<JsAbilityManager>(engine, info);
        return (me != nullptr) ? me->OnGetExtensionRunningInfos(*engine, *info) : nullptr;
    }

    static NativeValue* UpdateConfiguration(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAbilityManager* me = CheckParamsAndGetThis<JsAbilityManager>(engine, info);
        return (me != nullptr) ? me->OnUpdateConfiguration(*engine, *info) : nullptr;
    }

    static NativeValue* GetTopAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAbilityManager* me = CheckParamsAndGetThis<JsAbilityManager>(engine, info);
        return (me != nullptr) ? me->OnGetTopAbility(*engine, *info) : nullptr;
    }

    static NativeValue* AcquireShareData(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAbilityManager* me = CheckParamsAndGetThis<JsAbilityManager>(engine, info);
        return (me != nullptr) ? me->OnAcquireShareData(*engine, *info) : nullptr;
    }

    static NativeValue* NotifySaveAsResult(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAbilityManager* me = CheckParamsAndGetThis<JsAbilityManager>(engine, info);
        return (me != nullptr) ? me->OnNotifySaveAsResult(*engine, *info) : nullptr;
    }

private:
    NativeValue* OnGetAbilityRunningInfos(NativeEngine &engine, const NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        AsyncTask::CompleteCallback complete =
            [](NativeEngine &engine, AsyncTask &task, int32_t status) {
                std::vector<AAFwk::AbilityRunningInfo> infos;
                auto errcode = AbilityManagerClient::GetInstance()->GetAbilityRunningInfos(infos);
                if (errcode == 0) {
#ifdef ENABLE_ERRCODE
                    task.ResolveWithNoError(engine, CreateJsAbilityRunningInfoArray(engine, infos));
                } else {
                    task.Reject(engine, CreateJsError(engine, GetJsErrorCodeByNativeError(errcode)));
#else
                    task.Resolve(engine, CreateJsAbilityRunningInfoArray(engine, infos));
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Get mission infos failed."));
#endif
                }
            };

        NativeValue* lastParam = (info.argc == 0) ? nullptr : info.argv[0];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JsAbilityManager::OnGetAbilityRunningInfos",
            engine, CreateAsyncTaskWithLastParam(engine,
            lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetExtensionRunningInfos(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc == 0) {
            HILOG_ERROR("Not enough params");
#ifdef ENABLE_ERRCODE
            ThrowTooFewParametersError(engine);
#endif
            return engine.CreateUndefined();
        }
        int upperLimit = -1;
        if (!ConvertFromJsValue(engine, info.argv[0], upperLimit)) {
#ifdef ENABLE_ERRCODE
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
#endif
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [upperLimit](NativeEngine &engine, AsyncTask &task, int32_t status) {
                std::vector<AAFwk::ExtensionRunningInfo> infos;
                auto errcode = AbilityManagerClient::GetInstance()->GetExtensionRunningInfos(upperLimit, infos);
                if (errcode == 0) {
#ifdef ENABLE_ERRCODE
                    task.ResolveWithNoError(engine, CreateJsExtensionRunningInfoArray(engine, infos));
                } else {
                    task.Reject(engine, CreateJsError(engine, GetJsErrorCodeByNativeError(errcode)));
#else
                    task.Resolve(engine, CreateJsExtensionRunningInfoArray(engine, infos));
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Get mission infos failed."));
#endif
                }
            };

        NativeValue* lastParam = (info.argc == 1) ? nullptr : info.argv[1];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JsAbilityManager::OnGetExtensionRunningInfos",
            engine, CreateAsyncTaskWithLastParam(engine,
            lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnUpdateConfiguration(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        AsyncTask::CompleteCallback complete;

        do {
            if (info.argc == 0) {
                HILOG_ERROR("Not enough params");
#ifdef ENABLE_ERRCODE
                ThrowTooFewParametersError(engine);
#else
                complete = [](NativeEngine& engine, AsyncTask& task, int32_t status) {
                    task.Reject(engine, CreateJsError(engine, ERR_INVALID_VALUE, "no enough params."));
                };
#endif
                break;
            }

            AppExecFwk::Configuration changeConfig;
            if (!UnwrapConfiguration(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[0]), changeConfig)) {
#ifdef ENABLE_ERRCODE
                ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
#else
                complete = [](NativeEngine& engine, AsyncTask& task, int32_t status) {
                    task.Reject(engine, CreateJsError(engine, ERR_INVALID_VALUE, "config is invalid."));
                };
#endif
                break;
            }

            complete = [changeConfig](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto errcode = GetAppManagerInstance()->UpdateConfiguration(changeConfig);
                if (errcode == 0) {
#ifdef ENABLE_ERRCODE
                    task.ResolveWithNoError(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, GetJsErrorCodeByNativeError(errcode)));
#else
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "update config failed."));
#endif
                }
            };
        } while (0);

        NativeValue* lastParam = (info.argc == 1) ? nullptr : info.argv[1];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JsAbilityManager::OnGetExtensionRunningInfos",
            engine, CreateAsyncTaskWithLastParam(engine,
            lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetTopAbility(NativeEngine &engine, const NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
#ifdef ENABLE_ERRCODE
        auto selfToken = IPCSkeleton::GetSelfTokenID();
        if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
            HILOG_ERROR("This application is not system-app, can not use system-api");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
            return engine.CreateUndefined();
        }
#endif
        AsyncTask::CompleteCallback complete =
            [](NativeEngine &engine, AsyncTask &task, int32_t status) {
                AppExecFwk::ElementName elementName = AbilityManagerClient::GetInstance()->GetTopAbility();
#ifdef ENABLE_ERRCOE
                task.ResolveWithNoError(engine, CreateJsElementName(engine, elementName));
#else
                task.Resolve(engine, CreateJsElementName(engine, elementName));
#endif
            };

        NativeValue* lastParam = (info.argc == 0) ? nullptr : info.argv[0];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JsAbilityManager::OnGetTopAbility",
            engine, CreateAsyncTaskWithLastParam(engine,
            lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnAcquireShareData(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc < ARGC_ONE) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], missionId)) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
        NativeValue* lastParam = info.argc > ARGC_ONE  ? info.argv[INDEX_ONE] : nullptr;
        NativeValue *result = nullptr;
        std::unique_ptr<AsyncTask> uasyncTask = CreateAsyncTaskWithLastParam(
            engine, lastParam, nullptr, nullptr, &result);
        std::shared_ptr<AsyncTask> asyncTask = std::move(uasyncTask);

        AAFwk::ShareRuntimeTask task = [&engine, asyncTask](int32_t resultCode, const AAFwk::WantParams &wantParam) {
            if (resultCode != 0) {
                asyncTask->Reject(engine, CreateJsError(engine,  GetJsErrorCodeByNativeError(resultCode)));
                return;
            }
            NativeValue* abilityResult = AppExecFwk::CreateJsWantParams(engine, wantParam);
            if (abilityResult == nullptr) {
                asyncTask->Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
            } else {
                asyncTask->ResolveWithNoError(engine, abilityResult);
            }
        };
        sptr<AAFwk::AcquireShareDataCallbackStub> shareDataCallbackStub = new AAFwk::AcquireShareDataCallbackStub();
        mainHandler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
        shareDataCallbackStub->SetHandler(mainHandler_);
        shareDataCallbackStub->SetShareRuntimeTask(task);
        auto err = AbilityManagerClient::GetInstance()->AcquireShareData(missionId, shareDataCallbackStub);
        if (err != 0) {
            asyncTask->Reject(engine, CreateJsError(engine, GetJsErrorCodeByNativeError(err)));
        }
        return result;
    }

    NativeValue* OnNotifySaveAsResult(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("called");
        AsyncTask::CompleteCallback complete;
        AsyncTask::ExecuteCallback execute;

        do {
            if (info.argc < ARGC_TWO) {
                HILOG_ERROR("Not enough params");
                ThrowTooFewParametersError(engine);
                break;
            }

            int reqCode = 0;
            if (!JsApiUtils::UnwrapNumberValue(info.argv[1], reqCode)) {
                HILOG_ERROR("Get requestCode param error");
                ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
                break;
            }

            AppExecFwk::Want want;
            int resultCode = ERR_OK;
            if (!JsApiUtils::UnWrapAbilityResult(engine, info.argv[0], resultCode, want)) {
                HILOG_ERROR("Unrwrap abilityResult param error");
                ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
                break;
            }

            auto sharedCode = std::make_shared<ErrCode>(ERR_OK);
            execute = [sharedCode, want, resultCode, reqCode]() {
                *sharedCode = AbilityManagerClient::GetInstance()->NotifySaveAsResult(want, resultCode, reqCode);
            };
            complete = [sharedCode](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto errCode = *sharedCode;
                if (errCode == ERR_OK) {
                    task.ResolveWithNoError(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, GetJsErrorCodeByNativeError(errCode)));
                }
            };
        } while (0);

        NativeValue* lastParam = (info.argc == ARGC_TWO) ? nullptr : info.argv[ARGC_TWO];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JsAbilityManager::OnNotifySaveAsResult",
            engine, CreateAsyncTaskWithLastParam(engine,
            lastParam, std::move(execute), std::move(complete), &result));
        return result;
    }
};
} // namespace

NativeValue* JsAbilityManagerInit(NativeEngine* engine, NativeValue* exportObj)
{
    HILOG_INFO("JsAbilityManagerInit is called");

    if (engine == nullptr || exportObj == nullptr) {
        HILOG_INFO("engine or exportObj null");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_INFO("object null");
        return nullptr;
    }

    std::unique_ptr<JsAbilityManager> jsAbilityManager = std::make_unique<JsAbilityManager>();
    object->SetNativePointer(jsAbilityManager.release(), JsAbilityManager::Finalizer, nullptr);

    object->SetProperty("AbilityState", AbilityStateInit(engine));

    HILOG_INFO("JsAbilityManagerInit BindNativeFunction called");
    const char *moduleName = "JsAbilityManager";
    BindNativeFunction(*engine, *object, "getAbilityRunningInfos", moduleName,
        JsAbilityManager::GetAbilityRunningInfos);
    BindNativeFunction(*engine, *object, "getExtensionRunningInfos", moduleName,
        JsAbilityManager::GetExtensionRunningInfos);
    BindNativeFunction(*engine, *object, "updateConfiguration", moduleName, JsAbilityManager::UpdateConfiguration);
    BindNativeFunction(*engine, *object, "getTopAbility", moduleName, JsAbilityManager::GetTopAbility);
    BindNativeFunction(*engine, *object, "acquireShareData", moduleName, JsAbilityManager::AcquireShareData);
    BindNativeFunction(*engine, *object, "notifySaveAsResult", moduleName, JsAbilityManager::NotifySaveAsResult);
    HILOG_INFO("JsAbilityManagerInit end");
    return engine->CreateUndefined();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
