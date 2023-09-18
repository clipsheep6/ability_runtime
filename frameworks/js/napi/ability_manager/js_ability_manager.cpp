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

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        HILOG_INFO("JsAbilityManager::Finalizer is called");
        std::unique_ptr<JsAbilityManager>(static_cast<JsAbilityManager*>(data));
    }

    static napi_value GetAbilityRunningInfos(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAbilityManager, OnGetAbilityRunningInfos);
    }

    static napi_value GetExtensionRunningInfos(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAbilityManager, OnGetExtensionRunningInfos);
    }

    static napi_value UpdateConfiguration(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAbilityManager, OnUpdateConfiguration);
    }

    static napi_value GetTopAbility(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAbilityManager, OnGetTopAbility);
    }

    static napi_value AcquireShareData(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAbilityManager, OnAcquireShareData);
    }

    static napi_value NotifySaveAsResult(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAbilityManager, OnNotifySaveAsResult);
    }

private:
    napi_value OnGetAbilityRunningInfos(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        NapiAsyncTask::CompleteCallback complete =
            [](napi_env env, NapiAsyncTask &task, int32_t status) {
                std::vector<AAFwk::AbilityRunningInfo> infos;
                auto errcode = AbilityManagerClient::GetInstance()->GetAbilityRunningInfos(infos);
                if (errcode == 0) {
#ifdef ENABLE_ERRCODE
                    task.ResolveWithNoError(env, CreateJsAbilityRunningInfoArray(env, infos));
                } else {
                    task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(errcode)));
#else
                    task.Resolve(env, CreateJsAbilityRunningInfoArray(env, infos));
                } else {
                    task.Reject(env, CreateJsError(env, errcode, "Get mission infos failed."));
#endif
                }
            };

        napi_value lastParam = (argc == 0) ? nullptr : argv[0];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsAbilityManager::OnGetAbilityRunningInfos",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnGetExtensionRunningInfos(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (argc == 0) {
            HILOG_ERROR("Not enough params");
#ifdef ENABLE_ERRCODE
            ThrowTooFewParametersError(env);
#endif
            return CreateJsUndefined(env);
        }
        int upperLimit = -1;
        if (!ConvertFromJsValue(env, argv[0], upperLimit)) {
#ifdef ENABLE_ERRCODE
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
#endif
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [upperLimit](napi_env env, NapiAsyncTask &task, int32_t status) {
                std::vector<AAFwk::ExtensionRunningInfo> infos;
                auto errcode = AbilityManagerClient::GetInstance()->GetExtensionRunningInfos(upperLimit, infos);
                if (errcode == 0) {
#ifdef ENABLE_ERRCODE
                    task.ResolveWithNoError(env, CreateJsExtensionRunningInfoArray(env, infos));
                } else {
                    task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(errcode)));
#else
                    task.Resolve(env, CreateJsExtensionRunningInfoArray(env, infos));
                } else {
                    task.Reject(env, CreateJsError(env, errcode, "Get mission infos failed."));
#endif
                }
            };

        napi_value lastParam = (argc == 1) ? nullptr : argv[1];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsAbilityManager::OnGetExtensionRunningInfos",
            env, CreateAsyncTaskWithLastParam(env,
            lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnUpdateConfiguration(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        NapiAsyncTask::CompleteCallback complete;

        do {
            if (argc == 0) {
                HILOG_ERROR("Not enough params");
#ifdef ENABLE_ERRCODE
                ThrowTooFewParametersError(env);
#else
                complete = [](napi_env env, NapiAsyncTask& task, int32_t status) {
                    task.Reject(env, CreateJsError(env, ERR_INVALID_VALUE, "no enough params."));
                };
#endif
                break;
            }

            AppExecFwk::Configuration changeConfig;
            if (!UnwrapConfiguration(env, argv[0], changeConfig)) {
#ifdef ENABLE_ERRCODE
                ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
#else
                complete = [](napi_env env, NapiAsyncTask& task, int32_t status) {
                    task.Reject(env, CreateJsError(env, ERR_INVALID_VALUE, "config is invalid."));
                };
#endif
                break;
            }

            complete = [changeConfig](napi_env env, NapiAsyncTask& task, int32_t status) {
                auto errcode = GetAppManagerInstance()->UpdateConfiguration(changeConfig);
                if (errcode == 0) {
#ifdef ENABLE_ERRCODE
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(errcode)));
#else
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsError(env, errcode, "update config failed."));
#endif
                }
            };
        } while (0);

        napi_value lastParam = (argc == 1) ? nullptr : argv[1];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsAbilityManager::OnGetExtensionRunningInfos",
            env, CreateAsyncTaskWithLastParam(env,
            lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnGetTopAbility(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
#ifdef ENABLE_ERRCODE
        auto selfToken = IPCSkeleton::GetSelfTokenID();
        if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
            HILOG_ERROR("This application is not system-app, can not use system-api");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
            return CreateJsUndefined(env);
        }
#endif
        NapiAsyncTask::CompleteCallback complete =
            [](napi_env env, NapiAsyncTask &task, int32_t status) {
                AppExecFwk::ElementName elementName = AbilityManagerClient::GetInstance()->GetTopAbility();
#ifdef ENABLE_ERRCOE
                task.ResolveWithNoError(env, CreateJsElementName(env, elementName));
#else
                task.Resolve(env, CreateJsElementName(env, elementName));
#endif
            };

        napi_value lastParam = (argc == 0) ? nullptr : argv[0];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsAbilityManager::OnGetTopAbility",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnAcquireShareData(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (argc < ARGC_ONE) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], missionId)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        napi_value lastParam = argc > ARGC_ONE  ? argv[INDEX_ONE] : nullptr;
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> uasyncTask = CreateAsyncTaskWithLastParam(
            env, lastParam, nullptr, nullptr, &result);
        std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);

        AAFwk::ShareRuntimeTask task = [&env, asyncTask](int32_t resultCode, const AAFwk::WantParams &wantParam) {
            if (resultCode != 0) {
                asyncTask->Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(resultCode)));
                return;
            }
            napi_value abilityResult = AppExecFwk::WrapWantParams(env, wantParam);
            if (abilityResult == nullptr) {
                asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
            } else {
                asyncTask->ResolveWithNoError(env, abilityResult);
            }
        };
        sptr<AAFwk::AcquireShareDataCallbackStub> shareDataCallbackStub = new AAFwk::AcquireShareDataCallbackStub();
        mainHandler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
        shareDataCallbackStub->SetHandler(mainHandler_);
        shareDataCallbackStub->SetShareRuntimeTask(task);
        auto err = AbilityManagerClient::GetInstance()->AcquireShareData(missionId, shareDataCallbackStub);
        if (err != 0) {
            asyncTask->Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(err)));
        }
        return result;
    }

    napi_value OnNotifySaveAsResult(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_INFO("called");
        NapiAsyncTask::CompleteCallback complete;
        NapiAsyncTask::ExecuteCallback execute;

        do {
            if (argc < ARGC_TWO) {
                HILOG_ERROR("Not enough params");
                ThrowTooFewParametersError(env);
                break;
            }

            int reqCode = 0;
            if (!ConvertFromJsValue(env, argv[1], reqCode)) {
                HILOG_ERROR("Get requestCode param error");
                ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
                break;
            }

            AppExecFwk::Want want;
            int resultCode = ERR_OK;
            if (!AppExecFwk::UnWrapAbilityResult(env, argv[0], resultCode, want)) {
                HILOG_ERROR("Unrwrap abilityResult param error");
                ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
                break;
            }

            auto sharedCode = std::make_shared<ErrCode>(ERR_OK);
            execute = [sharedCode, want, resultCode, reqCode]() {
                *sharedCode = AbilityManagerClient::GetInstance()->NotifySaveAsResult(want, resultCode, reqCode);
            };
            complete = [sharedCode](napi_env env, NapiAsyncTask& task, int32_t status) {
                auto errCode = *sharedCode;
                if (errCode == ERR_OK) {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(errCode)));
                }
            };
        } while (0);

        napi_value lastParam = (argc == ARGC_TWO) ? nullptr : argv[ARGC_TWO];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsAbilityManager::OnNotifySaveAsResult",
            env, CreateAsyncTaskWithLastParam(env,
            lastParam, std::move(execute), std::move(complete), &result));
        return result;
    }
};
} // namespace

napi_value JsAbilityManagerInit(napi_env env, napi_value exportObj)
{
    HILOG_INFO("JsAbilityManagerInit is called");

    std::unique_ptr<JsAbilityManager> jsAbilityManager = std::make_unique<JsAbilityManager>();
    napi_wrap(env, exportObj, jsAbilityManager.release(), JsAbilityManager::Finalizer, nullptr, nullptr);

    napi_set_named_property(env, exportObj, "AbilityState", AbilityStateInit(env));

    HILOG_INFO("JsAbilityManagerInit BindNativeFunction called");
    const char *moduleName = "JsAbilityManager";
    BindNativeFunction(env, exportObj, "getAbilityRunningInfos", moduleName,
        JsAbilityManager::GetAbilityRunningInfos);
    BindNativeFunction(env, exportObj, "getExtensionRunningInfos", moduleName,
        JsAbilityManager::GetExtensionRunningInfos);
    BindNativeFunction(env, exportObj, "updateConfiguration", moduleName, JsAbilityManager::UpdateConfiguration);
    BindNativeFunction(env, exportObj, "getTopAbility", moduleName, JsAbilityManager::GetTopAbility);
    BindNativeFunction(env, exportObj, "acquireShareData", moduleName, JsAbilityManager::AcquireShareData);
    BindNativeFunction(env, exportObj, "notifySaveAsResult", moduleName, JsAbilityManager::NotifySaveAsResult);
    HILOG_INFO("JsAbilityManagerInit end");
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
