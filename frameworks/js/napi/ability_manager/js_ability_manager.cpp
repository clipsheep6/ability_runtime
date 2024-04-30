/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include <regex>

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "acquire_share_data_callback_stub.h"
#include "app_mgr_interface.h"
#include "errors.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "js_ability_foreground_state_observer.h"
#include "js_ability_manager_utils.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_base_context.h"
#include "napi_common_configuration.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "system_ability_definition.h"
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

constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr const char *ON_OFF_TYPE_ABILITY_FOREGROUND_STATE = "abilityForegroundState";
const std::string MAX_UINT64_VALUE = "18446744073709551615";
static std::shared_ptr<AppExecFwk::EventHandler> mainHandler_ = nullptr;

class JsAbilityManager final {
public:
    JsAbilityManager() = default;
    ~JsAbilityManager() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "JsAbilityManager::Finalizer is called");
        std::unique_ptr<JsAbilityManager>(static_cast<JsAbilityManager*>(data));
    }

    static napi_value GetAbilityRunningInfos(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnGetAbilityRunningInfos);
    }

    static napi_value GetExtensionRunningInfos(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnGetExtensionRunningInfos);
    }

    static napi_value UpdateConfiguration(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnUpdateConfiguration);
    }

    static napi_value GetTopAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnGetTopAbility);
    }

    static napi_value AcquireShareData(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnAcquireShareData);
    }

    static napi_value NotifySaveAsResult(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnNotifySaveAsResult);
    }
    static napi_value GetForegroundUIAbilities(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAbilityManager, OnGetForegroundUIAbilities);
    }

    static napi_value On(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAbilityManager, OnOn);
    }

    static napi_value Off(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAbilityManager, OnOff);
    }

    static napi_value IsEmbeddedOpenAllowed(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnIsEmbeddedOpenAllowed);
    }

    static napi_value SetResidentProcessEnabled(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAbilityManager, OnSetResidentProcessEnabled);
    }

    static napi_value NotifyDebugAssertResult(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAbilityManager, OnNotifyDebugAssertResult);
    }

private:
    sptr<OHOS::AbilityRuntime::JSAbilityForegroundStateObserver> observerForeground_ = nullptr;
    sptr<OHOS::AppExecFwk::IAppMgr> appManager_ = nullptr;

    std::string ParseParamType(const napi_env &env, size_t argc, const napi_value *argv)
    {
        std::string type;
        if (argc > INDEX_ZERO && ConvertFromJsValue(env, argv[INDEX_ZERO], type)) {
            return type;
        }
        return "";
    }

    napi_value OnOn(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Not enough params.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (!AppExecFwk::IsTypeForNapiValue(env, argv[INDEX_ONE], napi_object)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid param.");
            ThrowInvalidParamError(env, "Parse param observer failed, must be a AbilityForegroundStateObserver");
            return CreateJsUndefined(env);
        }

        std::string type = ParseParamType(env, argc, argv);
        if (type == ON_OFF_TYPE_ABILITY_FOREGROUND_STATE) {
            return OnOnAbilityForeground(env, argc, argv);
        }
        ThrowInvalidParamError(env, "Parse param type failed, must be a string, value must be abilityForegroundState");
        return CreateJsUndefined(env);
    }

    napi_value OnOnAbilityForeground(napi_env env, size_t argc, napi_value *argv)
    {
        if (observerForeground_ == nullptr) {
            observerForeground_ = new (std::nothrow) JSAbilityForegroundStateObserver(env);
            if (observerForeground_ == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "observerForeground_ is nullptr.");
                ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
                return CreateJsUndefined(env);
            }
        }

        if (observerForeground_->IsEmpty()) {
            int32_t ret = GetAppManagerInstance()->RegisterAbilityForegroundStateObserver(observerForeground_);
            if (ret != NO_ERROR) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed error: %{public}d.", ret);
                ThrowErrorByNativeErr(env, ret);
                return CreateJsUndefined(env);
            }
        }
        observerForeground_->AddJsObserverObject(argv[INDEX_ONE]);

        return CreateJsUndefined(env);
    }

    napi_value OnOff(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Not enough params when off.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (argc == ARGC_TWO && !AppExecFwk::IsTypeForNapiValue(env, argv[INDEX_ONE], napi_object)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid param.");
            ThrowInvalidParamError(env, "Parse param observer failed, must be a AbilityForegroundStateObserver");
            return CreateJsUndefined(env);
        }

        std::string type = ParseParamType(env, argc, argv);
        if (type == ON_OFF_TYPE_ABILITY_FOREGROUND_STATE) {
            return OnOffAbilityForeground(env, argc, argv);
        }
        ThrowInvalidParamError(env, "Parse param type failed, must be a string, value must be abilityForegroundState");
        return CreateJsUndefined(env);
    }

    bool CheckIsNumString(const std::string &numStr)
    {
        const std::regex regexJsperf(R"(^\d*)");
        std::match_results<std::string::const_iterator> matchResults;
        if (numStr.empty() || !std::regex_match(numStr, matchResults, regexJsperf)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Number parsing error, %{public}s.", numStr.c_str());
            return false;
        }
        if (MAX_UINT64_VALUE.length() < numStr.length() ||
            (MAX_UINT64_VALUE.length() == numStr.length() && MAX_UINT64_VALUE.compare(numStr) < 0)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Number parsing error, %{public}s.", numStr.c_str());
            return false;
        }
        return true;
    }

    napi_value OnNotifyDebugAssertResult(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Not enough params when off.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string assertSessionStr;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], assertSessionStr) || !CheckIsNumString(assertSessionStr)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Convert session id error.");
            ThrowInvalidParamError(env, "Parse param sessionId failed, must be a string");
            return CreateJsUndefined(env);
        }
        uint64_t assertSessionId = std::stoull(assertSessionStr);
        if (assertSessionId == 0) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Convert session id failed.");
            ThrowInvalidParamError(env, "Parse param sessionId failed, value must not be equal to zero");
            return CreateJsUndefined(env);
        }
        int32_t userStatus;
        if (!ConvertFromJsValue(env, argv[INDEX_ONE], userStatus)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Convert status failed.");
            ThrowInvalidParamError(env, "Parse param status failed, must be a UserStatus");
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [assertSessionId, userStatus](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto amsClient = AbilityManagerClient::GetInstance();
            if (amsClient == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability manager service instance is nullptr.");
                task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(AAFwk::INNER_ERR)));
                return;
            }
            auto ret = amsClient->NotifyDebugAssertResult(assertSessionId, static_cast<AAFwk::UserStatus>(userStatus));
            if (ret != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Notify user action result failed, error is %{public}d.", ret);
                task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(ret)));
                return;
            }
            task.ResolveWithNoError(env, CreateJsUndefined(env));
        };

        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsAbilityManager::OnNotifyDebugAssertResult", env,
            CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnOffAbilityForeground(napi_env env, size_t argc, napi_value *argv)
    {
        if (observerForeground_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Observer is nullptr.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        if (argc == ARGC_TWO) {
            observerForeground_->RemoveJsObserverObject(argv[INDEX_ONE]);
        } else {
            observerForeground_->RemoveAllJsObserverObject();
        }

        if (observerForeground_->IsEmpty()) {
            int32_t ret = GetAppManagerInstance()->UnregisterAbilityForegroundStateObserver(observerForeground_);
            if (ret != NO_ERROR) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed error: %{public}d.", ret);
                ThrowErrorByNativeErr(env, ret);
                return CreateJsUndefined(env);
            }
        }
        return CreateJsUndefined(env);
    }

    napi_value OnGetAbilityRunningInfos(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s is called", __FUNCTION__);
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

        napi_value lastParam = (info.argc == 0) ? nullptr : info.argv[0];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsAbilityManager::OnGetAbilityRunningInfos",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnGetExtensionRunningInfos(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s is called", __FUNCTION__);
        if (info.argc == 0) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Not enough params");
#ifdef ENABLE_ERRCODE
            ThrowTooFewParametersError(env);
#endif
            return CreateJsUndefined(env);
        }
        int upperLimit = -1;
        if (!ConvertFromJsValue(env, info.argv[0], upperLimit)) {
#ifdef ENABLE_ERRCODE
            ThrowInvalidParamError(env, "Parse param upperLimit failed, must be a number");
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

        napi_value lastParam = (info.argc == 1) ? nullptr : info.argv[1];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsAbilityManager::OnGetExtensionRunningInfos",
            env, CreateAsyncTaskWithLastParam(env,
            lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnUpdateConfiguration(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s is called", __FUNCTION__);
        NapiAsyncTask::CompleteCallback complete;

        do {
            if (info.argc == 0) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Not enough params");
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
            if (!UnwrapConfiguration(env, info.argv[0], changeConfig)) {
#ifdef ENABLE_ERRCODE
                ThrowInvalidParamError(env, "Parse param config failed, must be a Configuration");
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

        napi_value lastParam = (info.argc == 1) ? nullptr : info.argv[1];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsAbilityManager::OnGetExtensionRunningInfos",
            env, CreateAsyncTaskWithLastParam(env,
            lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnGetTopAbility(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s is called", __FUNCTION__);
#ifdef ENABLE_ERRCODE
        auto selfToken = IPCSkeleton::GetSelfTokenID();
        if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "This application is not system-app, can not use system-api");
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

        napi_value lastParam = (info.argc == 0) ? nullptr : info.argv[0];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsAbilityManager::OnGetTopAbility",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnAcquireShareData(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s is called", __FUNCTION__);
        if (info.argc < ARGC_ONE) {
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], missionId)) {
            ThrowInvalidParamError(env, "Parse param missionId failed, must be a number");
            return CreateJsUndefined(env);
        }
        napi_value lastParam = info.argc > ARGC_ONE  ? info.argv[INDEX_ONE] : nullptr;
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> uasyncTask = CreateAsyncTaskWithLastParam(
            env, lastParam, nullptr, nullptr, &result);
        std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);

        AAFwk::ShareRuntimeTask task = [env, asyncTask](int32_t resultCode, const AAFwk::WantParams &wantParam) {
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

    napi_value OnNotifySaveAsResult(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
        NapiAsyncTask::CompleteCallback complete;
        NapiAsyncTask::ExecuteCallback execute;

        do {
            if (info.argc < ARGC_TWO) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Not enough params");
                ThrowTooFewParametersError(env);
                break;
            }

            int reqCode = 0;
            if (!ConvertFromJsValue(env, info.argv[1], reqCode)) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Get requestCode param error");
                ThrowInvalidParamError(env, "Parse param requestCode failed, must be a number");
                break;
            }

            AppExecFwk::Want want;
            int resultCode = ERR_OK;
            if (!AppExecFwk::UnWrapAbilityResult(env, info.argv[0], resultCode, want)) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Unrwrap abilityResult param error");
                ThrowInvalidParamError(env, "Parse param parameter failed, must be a AbilityResult");
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

        napi_value lastParam = (info.argc == ARGC_TWO) ? nullptr : info.argv[ARGC_TWO];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsAbilityManager::OnNotifySaveAsResult", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return result;
    }

    napi_value OnGetForegroundUIAbilities(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
        NapiAsyncTask::CompleteCallback complete = [](napi_env env, NapiAsyncTask &task, int32_t status) {
            std::vector<AppExecFwk::AbilityStateData> list;
            int32_t ret = AbilityManagerClient::GetInstance()->GetForegroundUIAbilities(list);
            if (ret == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsAbilityStateDataArray(env, list));
            } else {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed error: %{public}d.", ret);
                task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(ret)));
            }
        };

        napi_value lastParam = (argc > ARGC_ZERO) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsAbilityManager::OnGetForegroundUIAbilities", env,
            CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnSetResidentProcessEnabled(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Not enough params when off.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string bundleName;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], bundleName) || bundleName.empty()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Convert session id error.");
            auto errMsg = "Non empty package name needs to be provided";
            ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), errMsg);
            return CreateJsUndefined(env);
        }

        bool enableState = false;
        if (!ConvertFromJsValue(env, argv[INDEX_ONE], enableState)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Convert status failed.");
            auto errMsg = "The second parameter needs to provide a Boolean type setting value";
            ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), errMsg);
            return CreateJsUndefined(env);
        }

        auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute = [bundleName, enableState, innerErrorCode, env]() {
            auto amsClient = AbilityManagerClient::GetInstance();
            if (amsClient == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability manager service instance is nullptr.");
                *innerErrorCode = static_cast<int32_t>(AAFwk::INNER_ERR);
                return;
            }
            *innerErrorCode = amsClient->SetResidentProcessEnabled(bundleName, enableState);
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (*innerErrorCode != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Set resident process result failed, error is %{public}d.",
                    *innerErrorCode);
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
                return;
            }
            task.ResolveWithNoError(env, CreateJsUndefined(env));
        };

        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsAbilityManager::OnSetResidentProcessEnabled", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
        return result;
    }

    napi_value OnIsEmbeddedOpenAllowed(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
        if (info.argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        bool stageMode = false;
        napi_status status = OHOS::AbilityRuntime::IsStageContext(env, info.argv[0], stageMode);
        if (status != napi_ok || !stageMode) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "it is not a stage mode");
            ThrowInvalidParamError(env, "Parse param context failed, must be stageMode");
            return CreateJsUndefined(env);
        }
        auto context = OHOS::AbilityRuntime::GetStageModeContext(env, info.argv[0]);
        if (context == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "get context failed");
            ThrowInvalidParamError(env, "Parse param context failed, must not be nullptr");
            return CreateJsUndefined(env);
        }
        auto uiAbilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
        if (uiAbilityContext == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "convert to UIAbility context failed");
            ThrowInvalidParamError(env, "Parse param context failed, must be UIAbilityContext");
            return CreateJsUndefined(env);
        }

        std::string appId;
        if (!ConvertFromJsValue(env, info.argv[1], appId)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "OnOpenAtomicService, parse appId failed.");
            ThrowInvalidParamError(env, "Parse param appId failed, must be a string");
            return CreateJsUndefined(env);
        }

        auto token = uiAbilityContext->GetToken();
        auto sharedResult = std::make_shared<bool>(false);
        NapiAsyncTask::ExecuteCallback execute = [sharedResult, token, appId]() {
            *sharedResult = AbilityManagerClient::GetInstance()->IsEmbeddedOpenAllowed(token, appId);
        };

        NapiAsyncTask::CompleteCallback complete = [sharedResult](napi_env env, NapiAsyncTask &task, int32_t status) {
            task.Resolve(env, CreateJsValue(env, *sharedResult));
        };

        napi_value lastParam = (info.argc > ARGC_TWO) ? info. argv[ARGC_TWO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsAbilityManager::OnIsEmbeddedOpenAllowed", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return result;
    }
};
} // namespace

napi_value JsAbilityManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");

    std::unique_ptr<JsAbilityManager> jsAbilityManager = std::make_unique<JsAbilityManager>();
    napi_wrap(env, exportObj, jsAbilityManager.release(), JsAbilityManager::Finalizer, nullptr, nullptr);

    napi_set_named_property(env, exportObj, "AbilityState", AbilityStateInit(env));
    napi_set_named_property(env, exportObj, "UserStatus", UserStatusInit(env));

    const char *moduleName = "JsAbilityManager";
    BindNativeFunction(env, exportObj, "getAbilityRunningInfos", moduleName,
        JsAbilityManager::GetAbilityRunningInfos);
    BindNativeFunction(env, exportObj, "getExtensionRunningInfos", moduleName,
        JsAbilityManager::GetExtensionRunningInfos);
    BindNativeFunction(env, exportObj, "updateConfiguration", moduleName, JsAbilityManager::UpdateConfiguration);
    BindNativeFunction(env, exportObj, "getTopAbility", moduleName, JsAbilityManager::GetTopAbility);
    BindNativeFunction(env, exportObj, "acquireShareData", moduleName, JsAbilityManager::AcquireShareData);
    BindNativeFunction(env, exportObj, "notifySaveAsResult", moduleName, JsAbilityManager::NotifySaveAsResult);
    BindNativeFunction(
        env, exportObj, "getForegroundUIAbilities", moduleName, JsAbilityManager::GetForegroundUIAbilities);
    BindNativeFunction(env, exportObj, "on", moduleName, JsAbilityManager::On);
    BindNativeFunction(env, exportObj, "off", moduleName, JsAbilityManager::Off);
    BindNativeFunction(
        env, exportObj, "notifyDebugAssertResult", moduleName, JsAbilityManager::NotifyDebugAssertResult);
    BindNativeFunction(env, exportObj, "isEmbeddedOpenAllowed", moduleName, JsAbilityManager::IsEmbeddedOpenAllowed);
    BindNativeFunction(
        env, exportObj, "setResidentProcessEnabled", moduleName, JsAbilityManager::SetResidentProcessEnabled);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "end");
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
