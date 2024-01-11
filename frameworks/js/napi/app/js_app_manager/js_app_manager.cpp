/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "js_app_manager.h"

#include <cstdint>
#include <mutex>

#include "ability_manager_interface.h"
#include "ability_runtime_error_util.h"
#include "app_mgr_interface.h"
#include "event_runner.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "js_app_foreground_state_observer.h"
#include "js_app_manager_utils.h"
#include "js_app_state_observer.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_util.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr const char* ON_OFF_TYPE = "applicationState";
constexpr const char* ON_OFF_TYPE_SYNC = "applicationStateEvent";
constexpr const char *ON_OFF_TYPE_APP_FOREGROUND_STATE = "appForegroundState";

class JsAppManager final {
public:
    JsAppManager(sptr<OHOS::AppExecFwk::IAppMgr> appManager,
        sptr<OHOS::AAFwk::IAbilityManager> abilityManager) : appManager_(appManager),
        abilityManager_(abilityManager) {}
    ~JsAppManager()
    {
        if (observer_ != nullptr) {
            HILOG_INFO("Set valid false");
            observer_->SetValid(false);
        }
        if (observerForeground_ != nullptr) {
            observerForeground_->SetValid(false);
        }
    }

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        HILOG_INFO("JsAbilityContext::Finalizer is called");
        std::unique_ptr<JsAppManager>(static_cast<JsAppManager*>(data));
    }

    static napi_value On(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnOn);
    }

    static napi_value Off(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnOff);
    }

    static napi_value GetForegroundApplications(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnGetForegroundApplications);
    }

    static napi_value GetRunningProcessInformation(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnGetRunningProcessInformation);
    }

    static napi_value IsRunningInStabilityTest(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnIsRunningInStabilityTest);
    }

    static napi_value KillProcessWithAccount(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnKillProcessWithAccount);
    }

    static napi_value KillProcessesByBundleName(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnkillProcessesByBundleName);
    }

    static napi_value ClearUpApplicationData(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnClearUpApplicationData);
    }

    static napi_value IsSharedBundleRunning(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnIsSharedBundleRunning);
    }

    static napi_value GetAppMemorySize(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnGetAppMemorySize);
    }

    static napi_value IsRamConstrainedDevice(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnIsRamConstrainedDevice);
    }

    static napi_value GetProcessMemoryByPid(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnGetProcessMemoryByPid);
    }

    static napi_value GetRunningProcessInfoByBundleName(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnGetRunningProcessInfoByBundleName);
    }

    static napi_value IsApplicationRunning(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnIsApplicationRunning);
    }
private:
    sptr<OHOS::AppExecFwk::IAppMgr> appManager_ = nullptr;
    sptr<OHOS::AAFwk::IAbilityManager> abilityManager_ = nullptr;
    sptr<JSAppStateObserver> observer_ = nullptr;
    sptr<JSAppStateObserver> observerSync_ = nullptr;
    sptr<JSAppForegroundStateObserver> observerForeground_ = nullptr;
    int32_t serialNumber_ = 0;

    napi_value OnOn(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("OnOn called");
        std::string type = ParseParamType(env, argc, argv);
        if (type == ON_OFF_TYPE_SYNC) {
            return OnOnNew(env, argc, argv);
        } else if (type == ON_OFF_TYPE_APP_FOREGROUND_STATE) {
            return OnOnForeground(env, argc, argv);
        }

        return OnOnOld(env, argc, argv);
    }

    napi_value OnOnOld(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("OnOnOld called");
        if (argc < ARGC_TWO) { // support 2 or 3 params, if > 3 params, ignore other params
            HILOG_ERROR("Not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        if (!CheckOnOffType(env, argc, argv)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        if (appManager_ == nullptr) {
            HILOG_ERROR("appManager nullptr");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }

        std::vector<std::string> bundleNameList;
        // unwarp observer
        if (observer_ == nullptr) {
            observer_ = new JSAppStateObserver(env);
        }
        if (argc > ARGC_TWO) {
            AppExecFwk::UnwrapArrayStringFromJS(env, argv[INDEX_TWO], bundleNameList);
        }
        int32_t ret = appManager_->RegisterApplicationStateObserver(observer_, bundleNameList);
        if (ret == 0) {
            HILOG_DEBUG("success.");
            int64_t observerId = serialNumber_;
            observer_->AddJsObserverObject(observerId, argv[INDEX_ONE]);
            if (serialNumber_ < INT32_MAX) {
                serialNumber_++;
            } else {
                serialNumber_ = 0;
            }
            return CreateJsValue(env, observerId);
        } else {
            HILOG_ERROR("wrong error:%{public}d.", ret);
            ThrowErrorByNativeErr(env, ret);
            return CreateJsUndefined(env);
        }
    }

    napi_value OnOnNew(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("called");
        if (argc < ARGC_TWO) { // support 2 or 3 params, if > 3 params, ignore other params
            HILOG_ERROR("Not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (!AppExecFwk::IsTypeForNapiValue(env, argv[INDEX_ONE], napi_object)) {
            HILOG_ERROR("Invalid param");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        std::vector<std::string> bundleNameList;
        if (argc > ARGC_TWO) {
            AppExecFwk::UnwrapArrayStringFromJS(env, argv[INDEX_TWO], bundleNameList);
        }
        if (observerSync_ == nullptr) {
            observerSync_ = new JSAppStateObserver(env);
        }
        if (appManager_ == nullptr || observerSync_ == nullptr) {
            HILOG_ERROR("appManager or observer is nullptr");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        int32_t ret = appManager_->RegisterApplicationStateObserver(observerSync_, bundleNameList);
        if (ret == 0) {
            HILOG_DEBUG("success.");
            int32_t observerId = serialNumber_;
            observerSync_->AddJsObserverObject(observerId, argv[INDEX_ONE]);
            if (serialNumber_ < INT32_MAX) {
                serialNumber_++;
            } else {
                serialNumber_ = 0;
            }
            return CreateJsValue(env, observerId);
        } else {
            HILOG_ERROR("Wrong error:%{public}d.", ret);
            ThrowErrorByNativeErr(env, ret);
            return CreateJsUndefined(env);
        }
    }

    napi_value OnOnForeground(napi_env env, size_t argc, napi_value *argv)
    {
        HILOG_DEBUG("Called.");
        if (argc < ARGC_TWO) {
            HILOG_ERROR("Not enough params.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (!AppExecFwk::IsTypeForNapiValue(env, argv[INDEX_ONE], napi_object)) {
            HILOG_ERROR("Invalid param.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        if (observerForeground_ == nullptr) {
            observerForeground_ = new (std::nothrow) JSAppForegroundStateObserver(env);
        }

        if (appManager_ == nullptr || observerForeground_ == nullptr) {
            HILOG_ERROR("AppManager or observer is nullptr.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }

        if (observerForeground_->IsEmpty()) {
            int32_t ret = appManager_->RegisterAppForegroundStateObserver(observerForeground_);
            if (ret != NO_ERROR) {
                HILOG_ERROR("Failed error: %{public}d.", ret);
                ThrowErrorByNativeErr(env, ret);
                return CreateJsUndefined(env);
            }
        }
        observerForeground_->AddJsObserverObject(argv[INDEX_ONE]);
        return CreateJsUndefined(env);
    }

    napi_value OnOff(napi_env env, size_t argc, napi_value *argv)
    {
        HILOG_DEBUG("called");
        std::string type = ParseParamType(env, argc, argv);
        if (type == ON_OFF_TYPE_SYNC) {
            return OnOffNew(env, argc, argv);
        } else if (type == ON_OFF_TYPE_APP_FOREGROUND_STATE) {
            return OnOffForeground(env, argc, argv);
        }

        return OnOffOld(env, argc, argv);
    }

    napi_value OnOffOld(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("called");
        if (argc < ARGC_TWO) {
            HILOG_ERROR("Not enough params when off.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (!CheckOnOffType(env, argc, argv)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        int64_t observerId = -1;
        napi_get_value_int64(env, argv[INDEX_ONE], &observerId);
        if (observer_ == nullptr) {
            HILOG_ERROR("observer_ is nullpter, please register first");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        if (!observer_->FindObserverByObserverId(observerId)) {
            HILOG_ERROR("not find observer, observer:%{public}d", static_cast<int32_t>(observerId));
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        HILOG_DEBUG("find observer exist observer:%{public}d", static_cast<int32_t>(observerId));

        NapiAsyncTask::CompleteCallback complete =
            [appManager = appManager_, observer = observer_, observerId](
                napi_env env, NapiAsyncTask& task, int32_t status) {
                if (observer == nullptr || appManager == nullptr) {
                    HILOG_ERROR("observer or appManager nullptr");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                    return;
                }
                int32_t ret = appManager->UnregisterApplicationStateObserver(observer);
                if (ret == 0 && observer->RemoveJsObserverObject(observerId)) {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                    HILOG_DEBUG("success size:%{public}zu",
                        observer->GetJsObserverMapSize());
                } else {
                    HILOG_ERROR("failed error:%{public}d", ret);
                    task.Reject(env, CreateJsErrorByNativeErr(env, ret));
                }
            };

        napi_value lastParam = (argc > ARGC_TWO) ? argv[INDEX_TWO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSAppManager::OnUnregisterApplicationStateObserver",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnOffNew(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("called");
        if (argc < ARGC_TWO) {
            HILOG_ERROR("Not enough params when off.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int32_t observerId = -1;
        if (!ConvertFromJsValue(env, argv[INDEX_ONE], observerId)) {
            HILOG_ERROR("Parse observerId failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        if (observerSync_ == nullptr || appManager_ == nullptr) {
            HILOG_ERROR("observer or appManager nullptr");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        if (!observerSync_->FindObserverByObserverId(observerId)) {
            HILOG_ERROR("not find observer, observer:%{public}d", static_cast<int32_t>(observerId));
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        int32_t ret = appManager_->UnregisterApplicationStateObserver(observerSync_);
        if (ret == 0 && observerSync_->RemoveJsObserverObject(observerId)) {
            HILOG_DEBUG("success size:%{public}zu", observerSync_->GetJsObserverMapSize());
            return CreateJsUndefined(env);
        } else {
            HILOG_ERROR("failed error:%{public}d", ret);
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
    }

    napi_value OnOffForeground(napi_env env, size_t argc, napi_value *argv)
    {
        HILOG_DEBUG("Called.");
        if (argc < ARGC_ONE) {
            HILOG_ERROR("Not enough params when off.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (argc == ARGC_TWO && !AppExecFwk::IsTypeForNapiValue(env, argv[INDEX_ONE], napi_object)) {
            HILOG_ERROR("Invalid param.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        if (observerForeground_ == nullptr || appManager_ == nullptr) {
            HILOG_ERROR("Observer or appManager nullptr.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }

        if (argc == ARGC_ONE) {
            observerForeground_->RemoveAllJsObserverObjects();
        } else if (argc == ARGC_TWO) {
            observerForeground_->RemoveJsObserverObject(argv[INDEX_ONE]);
        }
        if (observerForeground_->IsEmpty()) {
            int32_t ret = appManager_->UnregisterAppForegroundStateObserver(observerForeground_);
            if (ret != NO_ERROR) {
                HILOG_ERROR("Failed error: %{public}d.", ret);
                ThrowErrorByNativeErr(env, ret);
                return CreateJsUndefined(env);
            }
        }
        return CreateJsUndefined(env);
    }

    napi_value OnGetForegroundApplications(napi_env env, size_t argc, napi_value *argv)
    {
        HILOG_DEBUG("called");
        NapiAsyncTask::CompleteCallback complete =
            [appManager = appManager_](napi_env env, NapiAsyncTask& task, int32_t status) {
                if (appManager == nullptr) {
                    HILOG_ERROR("appManager nullptr");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                    return;
                }
                std::vector<AppExecFwk::AppStateData> list;
                int32_t ret = appManager->GetForegroundApplications(list);
                if (ret == 0) {
                    HILOG_DEBUG("success.");
                    task.ResolveWithNoError(env, CreateJsAppStateDataArray(env, list));
                } else {
                    HILOG_ERROR("failed error:%{public}d", ret);
                    task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(ret)));
                }
            };

        napi_value lastParam = (argc > ARGC_ZERO) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSAppManager::OnGetForegroundApplications",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnGetRunningProcessInformation(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("called");
        NapiAsyncTask::CompleteCallback complete =
            [appManager = appManager_](napi_env env, NapiAsyncTask &task, int32_t status) {
                if (appManager == nullptr) {
                    HILOG_WARN("abilityManager nullptr");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                    return;
                }
                std::vector<AppExecFwk::RunningProcessInfo> infos;
                auto ret = appManager->GetAllRunningProcesses(infos);
                if (ret == 0) {
                    task.ResolveWithNoError(env, CreateJsRunningProcessInfoArray(env, infos));
                } else {
                    task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(ret)));
                }
            };

        napi_value lastParam = (argc > ARGC_ZERO) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSAppManager::OnGetRunningProcessInformation",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnIsRunningInStabilityTest(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("called");
        NapiAsyncTask::CompleteCallback complete =
            [abilityManager = abilityManager_](napi_env env, NapiAsyncTask& task, int32_t status) {
                if (abilityManager == nullptr) {
                    HILOG_WARN("abilityManager nullptr");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                    return;
                }
                bool ret = abilityManager->IsRunningInStabilityTest();
                HILOG_INFO("result:%{public}d", ret);
                task.ResolveWithNoError(env, CreateJsValue(env, ret));
            };

        napi_value lastParam = (argc > ARGC_ZERO) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSAppManager::OnIsRunningInStabilityTest",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnkillProcessesByBundleName(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("OnkillProcessesByBundleName called");
        if (argc < ARGC_ONE) {
            HILOG_ERROR("Params not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string bundleName;
        if (!ConvertFromJsValue(env, argv[0], bundleName)) {
            HILOG_ERROR("get bundleName error!");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        HILOG_INFO("kill process [%{public}s]", bundleName.c_str());
        NapiAsyncTask::CompleteCallback complete =
            [bundleName, abilityManager = abilityManager_](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (abilityManager == nullptr) {
                HILOG_WARN("abilityManager nullptr");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto ret = abilityManager->KillProcess(bundleName);
            if (ret == 0) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, ret, "kill process failed."));
            }
        };

        napi_value lastParam = (argc == ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSAppManager::OnkillProcessesByBundleName",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnClearUpApplicationData(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("OnClearUpApplicationData called");
        if (argc < ARGC_ONE) {
            HILOG_ERROR("arguments not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string bundleName;
        if (!ConvertFromJsValue(env, argv[0], bundleName)) {
            HILOG_ERROR("get bundleName failed!");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [bundleName, abilityManager = abilityManager_](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (abilityManager == nullptr) {
                HILOG_WARN("abilityManager nullptr");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto ret = abilityManager->ClearUpApplicationData(bundleName);
            if (ret == 0) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, ret, "clear up application failed."));
            }
        };

        napi_value lastParam = (argc == ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSAppManager::OnClearUpApplicationData",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnIsSharedBundleRunning(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("OnIsSharedBundleRunning called");
        if (argc < ARGC_TWO) {
            HILOG_ERROR("Params not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string bundleName;
        if (!ConvertFromJsValue(env, argv[0], bundleName)) {
            HILOG_ERROR("get bundleName wrong!");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        uint32_t versionCode = 0;
        if (!ConvertFromJsValue(env, argv[1], versionCode)) {
            HILOG_ERROR("get versionCode failed!");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [bundleName, versionCode, appManager = appManager_](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (appManager == nullptr) {
                HILOG_WARN("appManager nullptr");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            bool ret = appManager->IsSharedBundleRunning(bundleName, versionCode);
            HILOG_INFO("result:%{public}d", ret);
            task.ResolveWithNoError(env, CreateJsValue(env, ret));
        };

        napi_value lastParam = (argc == ARGC_THREE) ? argv[INDEX_TWO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSAppManager::OnIsSharedBundleRunning",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnKillProcessWithAccount(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("called");
        if (argc < ARGC_TWO) {
            HILOG_ERROR("Params not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string bundleName;
        if (!ConvertFromJsValue(env, argv[0], bundleName)) {
            HILOG_ERROR("Parse bundleName failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        int32_t accountId = -1;
        if (!ConvertFromJsValue(env, argv[1], accountId)) {
            HILOG_ERROR("Parse userId failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [appManager = appManager_, bundleName, accountId](napi_env env, NapiAsyncTask &task, int32_t status) {
                if (appManager == nullptr || appManager->GetAmsMgr() == nullptr) {
                    HILOG_WARN("appManager is nullptr or amsMgr is nullptr.");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                    return;
                }
                auto ret = appManager->GetAmsMgr()->KillProcessWithAccount(bundleName, accountId);
                if (ret == 0) {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, ret, "Kill processes failed."));
                }
            };

        napi_value lastParam = (argc == ARGC_THREE) ? argv[INDEX_TWO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSAppManager::OnKillProcessWithAccount",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnGetAppMemorySize(napi_env env, size_t argc, napi_value* argv)
    {
        NapiAsyncTask::CompleteCallback complete =
            [abilityManager = abilityManager_](napi_env env, NapiAsyncTask& task, int32_t status) {
                if (abilityManager == nullptr) {
                    HILOG_WARN("abilityManager nullptr");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                    return;
                }
                int32_t memorySize = abilityManager->GetAppMemorySize();
                HILOG_INFO("memorySize:%{public}d", memorySize);
                task.ResolveWithNoError(env, CreateJsValue(env, memorySize));
            };

        napi_value lastParam = (argc > ARGC_ZERO) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSAppManager::OnGetAppMemorySize",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnIsRamConstrainedDevice(napi_env env, size_t argc, napi_value* argv)
    {
        NapiAsyncTask::CompleteCallback complete =
            [abilityManager = abilityManager_](napi_env env, NapiAsyncTask& task, int32_t status) {
                if (abilityManager == nullptr) {
                    HILOG_WARN("abilityManager nullptr");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                    return;
                }
                bool ret = abilityManager->IsRamConstrainedDevice();
                HILOG_INFO("result:%{public}d", ret);
                task.ResolveWithNoError(env, CreateJsValue(env, ret));
            };

        napi_value lastParam = (argc > ARGC_ZERO) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSAppManager::OnIsRamConstrainedDevice",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnGetProcessMemoryByPid(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("called");
        if (argc < ARGC_ONE) {
            HILOG_ERROR("Params not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        int32_t pid;
        if (!ConvertFromJsValue(env, argv[0], pid)) {
            HILOG_ERROR("get pid failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [pid, appManager = appManager_](napi_env env, NapiAsyncTask &task, int32_t status) {
                if (appManager == nullptr) {
                    HILOG_WARN("appManager is nullptr");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                    return;
                }
                int32_t memSize = 0;
                int32_t ret = appManager->GetProcessMemoryByPid(pid, memSize);
                if (ret == 0) {
                    task.ResolveWithNoError(env, CreateJsValue(env, memSize));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, ret));
                }
            };

        napi_value lastParam = (argc == ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSAppManager::OnGetProcessMemoryByPid",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnGetRunningProcessInfoByBundleName(napi_env env, size_t argc, napi_value* argv)
    {
        if (argc < ARGC_ONE) {
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string bundleName;
        int userId = IPCSkeleton::GetCallingUid() / AppExecFwk::Constants::BASE_USER_RANGE;
        bool isPromiseType = false;
        if (!ConvertFromJsValue(env, argv[0], bundleName)) {
            HILOG_ERROR("First parameter must be string");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        if (argc == ARGC_ONE) {
            isPromiseType = true;
        } else if (argc == ARGC_TWO) {
            if (ConvertFromJsValue(env, argv[1], userId)) {
                isPromiseType = true;
            }
        } else if (argc == ARGC_THREE) {
            if (!ConvertFromJsValue(env, argv[1], userId)) {
                HILOG_WARN("Must input userid and use callback when argc is three.");
                ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
                return CreateJsUndefined(env);
            }
        } else {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [bundleName, userId, appManager = appManager_](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (appManager == nullptr) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            std::vector<AppExecFwk::RunningProcessInfo> infos;
            int32_t ret = appManager->GetRunningProcessInformation(bundleName, userId, infos);
            if (ret == 0) {
                task.ResolveWithNoError(env, CreateJsRunningProcessInfoArray(env, infos));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, ret));
            }
        };
        napi_value lastParam = isPromiseType ? nullptr : argv[argc - 1];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSAppManager::OnGetRunningProcessInfoByBundleName",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnIsApplicationRunning(napi_env env, size_t argc, napi_value *argv)
    {
        HILOG_DEBUG("Called.");
        if (argc < ARGC_ONE) {
            HILOG_ERROR("Params not match.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string bundleName;
        if (!ConvertFromJsValue(env, argv[0], bundleName)) {
            HILOG_ERROR("Get bundle name wrong.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
        auto isRunning = std::make_shared<bool>(false);
        wptr<OHOS::AppExecFwk::IAppMgr> appManager = appManager_;
        NapiAsyncTask::ExecuteCallback execute =
            [bundleName, appManager, innerErrorCode, isRunning]() {
            sptr<OHOS::AppExecFwk::IAppMgr> appMgr = appManager.promote();
            if (appMgr == nullptr) {
                HILOG_ERROR("App manager is nullptr.");
                *innerErrorCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
                return;
            }
            *innerErrorCode = appMgr->IsApplicationRunning(bundleName, *isRunning);
        };
        NapiAsyncTask::CompleteCallback complete =
            [innerErrorCode, isRunning](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (*innerErrorCode == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsValue(env, *isRunning));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
            }
        };

        napi_value lastParam = (argc == ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSAppManager::OnIsApplicationRunning",
            env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return result;
    }

    bool CheckOnOffType(napi_env env, size_t argc, napi_value* argv)
    {
        if (argc < ARGC_ONE) {
            return false;
        }

        if (!AppExecFwk::IsTypeForNapiValue(env, argv[0], napi_string)) {
            HILOG_ERROR("Param 0 is not string");
            return false;
        }

        std::string type;
        if (!ConvertFromJsValue(env, argv[0], type)) {
            HILOG_ERROR("Parse on off type failed");
            return false;
        }

        if (type != ON_OFF_TYPE) {
            HILOG_ERROR("args[0] should be %{public}s.", ON_OFF_TYPE);
            return false;
        }
        return true;
    }

    std::string ParseParamType(napi_env env, size_t argc, napi_value* argv)
    {
        std::string type;
        if (argc > INDEX_ZERO && ConvertFromJsValue(env, argv[INDEX_ZERO], type)) {
            return type;
        }
        return "";
    }
};
} // namespace

OHOS::sptr<OHOS::AppExecFwk::IAppMgr> GetAppManagerInstance()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> appObject = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(appObject);
}

OHOS::sptr<OHOS::AAFwk::IAbilityManager> GetAbilityManagerInstance()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> abilityObject =
        systemAbilityManager->GetSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AAFwk::IAbilityManager>(abilityObject);
}

napi_value JsAppManagerInit(napi_env env, napi_value exportObj)
{
    HILOG_DEBUG("called");
    if (env == nullptr || exportObj == nullptr) {
        HILOG_WARN("env or exportObj null");
        return nullptr;
    }

    std::unique_ptr<JsAppManager> jsAppManager = std::make_unique<JsAppManager>(
        GetAppManagerInstance(), GetAbilityManagerInstance());
    napi_wrap(env, exportObj, jsAppManager.release(), JsAppManager::Finalizer, nullptr, nullptr);

    napi_set_named_property(env, exportObj, "ApplicationState", ApplicationStateInit(env));
    napi_set_named_property(env, exportObj, "ProcessState", ProcessStateInit(env));

    const char *moduleName = "AppManager";
    BindNativeFunction(env, exportObj, "on", moduleName, JsAppManager::On);
    BindNativeFunction(env, exportObj, "off", moduleName, JsAppManager::Off);
    BindNativeFunction(env, exportObj, "getForegroundApplications", moduleName,
        JsAppManager::GetForegroundApplications);
    BindNativeFunction(env, exportObj, "getProcessRunningInfos", moduleName,
        JsAppManager::GetRunningProcessInformation);
    BindNativeFunction(env, exportObj, "getProcessRunningInformation", moduleName,
        JsAppManager::GetRunningProcessInformation);
    BindNativeFunction(env, exportObj, "getRunningProcessInformation", moduleName,
        JsAppManager::GetRunningProcessInformation);
    BindNativeFunction(env, exportObj, "isRunningInStabilityTest", moduleName,
        JsAppManager::IsRunningInStabilityTest);
    BindNativeFunction(env, exportObj, "killProcessWithAccount", moduleName,
        JsAppManager::KillProcessWithAccount);
    BindNativeFunction(env, exportObj, "killProcessesByBundleName", moduleName,
        JsAppManager::KillProcessesByBundleName);
    BindNativeFunction(env, exportObj, "clearUpApplicationData", moduleName,
        JsAppManager::ClearUpApplicationData);
    BindNativeFunction(env, exportObj, "getAppMemorySize", moduleName,
        JsAppManager::GetAppMemorySize);
    BindNativeFunction(env, exportObj, "isRamConstrainedDevice", moduleName,
        JsAppManager::IsRamConstrainedDevice);
    BindNativeFunction(env, exportObj, "isSharedBundleRunning", moduleName,
        JsAppManager::IsSharedBundleRunning);
    BindNativeFunction(env, exportObj, "getProcessMemoryByPid", moduleName,
        JsAppManager::GetProcessMemoryByPid);
    BindNativeFunction(env, exportObj, "getRunningProcessInfoByBundleName", moduleName,
        JsAppManager::GetRunningProcessInfoByBundleName);
    BindNativeFunction(env, exportObj, "isApplicationRunning", moduleName,
        JsAppManager::IsApplicationRunning);
    HILOG_DEBUG("end");
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
