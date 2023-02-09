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

#include "js_app_manager.h"

#include <cstdint>
#include <mutex>

#include "ability_manager_interface.h"
#include "ability_runtime_error_util.h"
#include "app_mgr_interface.h"
#include "hilog_wrapper.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "js_app_manager_utils.h"
#include "event_runner.h"
#include "napi_common_util.h"
#include "js_app_state_observer.h"

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

std::mutex g_observerMutex;

class JsAppManager final {
public:
    JsAppManager(sptr<OHOS::AppExecFwk::IAppMgr> appManager,
        sptr<OHOS::AAFwk::IAbilityManager> abilityManager) : appManager_(appManager),
        abilityManager_(abilityManager) {}
    ~JsAppManager()
    {
        if (observer_ != nullptr){
            HILOG_INFO("Set valid false");
            observer_->SetValid(false);
        }
    }

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsAbilityContext::Finalizer is called");
        std::unique_ptr<JsAppManager>(static_cast<JsAppManager*>(data));
    }

    static NativeValue* On(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnOn(*engine, *info) : nullptr;
    }

    static NativeValue* Off(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnOff(*engine, *info) : nullptr;
    }

    static NativeValue* GetForegroundApplications(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnGetForegroundApplications(*engine, *info) : nullptr;
    }

    static NativeValue* GetRunningProcessInformation(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnGetRunningProcessInformation(*engine, *info) : nullptr;
    }

    static NativeValue* IsRunningInStabilityTest(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnIsRunningInStabilityTest(*engine, *info) : nullptr;
    }

    static NativeValue* KillProcessWithAccount(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnKillProcessWithAccount(*engine, *info) : nullptr;
    }

    static NativeValue* KillProcessesByBundleName(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnkillProcessesByBundleName(*engine, *info) : nullptr;
    }

    static NativeValue* ClearUpApplicationData(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnClearUpApplicationData(*engine, *info) : nullptr;
    }

    static NativeValue* GetAppMemorySize(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnGetAppMemorySize(*engine, *info) : nullptr;
    }

    static NativeValue* IsRamConstrainedDevice(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnIsRamConstrainedDevice(*engine, *info) : nullptr;
    }
private:
    sptr<OHOS::AppExecFwk::IAppMgr> appManager_ = nullptr;
    sptr<OHOS::AAFwk::IAbilityManager> abilityManager_ = nullptr;
    sptr<JSAppStateObserver> observer_ = nullptr;

    NativeValue* OnOn(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc < ARGC_TWO) { // support 2 or 3 params, if > 3 params, ignore other params
            HILOG_ERROR("Not enough params");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }

        if (!CheckOnOffType(engine, info)) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        if (appManager_ == nullptr) {
            HILOG_ERROR("appManager nullptr");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
            return engine.CreateUndefined();
        }

        std::vector<std::string> bundleNameList;
        // unwarp observer
        if (observer_ == nullptr) {
            observer_ = new JSAppStateObserver(engine);
        }
        if (info.argc > ARGC_TWO) {
            AppExecFwk::UnwrapArrayStringFromJS(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[INDEX_TWO]), bundleNameList);
        }
        int32_t ret = appManager_->RegisterApplicationStateObserver(observer_, bundleNameList);
        if (ret == 0) {
            HILOG_DEBUG("RegisterApplicationStateObserver success.");
            std::lock_guard<std::mutex> lock(g_observerMutex);
            static int64_t serialNumber = 0;
            int64_t observerId = serialNumber;
            observer_->AddJsObserverObject(observerId, info.argv[INDEX_ZERO]);
            if (serialNumber < INT32_MAX) {
                serialNumber++;
            } else {
                serialNumber = 0;
            }
            return engine.CreateNumber(observerId);
        } else {
            HILOG_ERROR("RegisterApplicationStateObserver failed error:%{public}d.", ret);
            ThrowErrorByNativeErr(engine, ret);
            return engine.CreateUndefined();
        }
    }

    NativeValue* OnOff(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc < ARGC_TWO) {
            HILOG_ERROR("Not enough params when off.");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }

        if (!CheckOnOffType(engine, info)) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        int64_t observerId = -1;
        napi_get_value_int64(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_ONE]), &observerId);
        std::lock_guard<std::mutex> lock(g_observerMutex);
        if (!observer_->FindObserverByObserverId(observerId)) {
            HILOG_INFO("not find observer, observer:%{public}d", static_cast<int32_t>(observerId));
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
        HILOG_INFO("%{public}s find observer exist observer:%{public}d", __func__, static_cast<int32_t>(observerId));

        AsyncTask::CompleteCallback complete =
            [appManager = appManager_, observer = observer_, observerId](
                NativeEngine& engine, AsyncTask& task, int32_t status) {
                if (observer == nullptr || appManager == nullptr) {
                    HILOG_ERROR("observer or appManager nullptr");
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                    return;
                }
                int32_t ret = appManager->UnregisterApplicationStateObserver(observer);
                if (ret == 0 && observer->RemoveJsObserverObject(observerId)) {
                    task.ResolveWithNoError(engine, engine.CreateUndefined());
                    std::lock_guard<std::mutex> lock(g_observerMutex);
                    HILOG_DEBUG("UnregisterApplicationStateObserver success size:%{public}zu",
                        observer->GetJsObserverMapSize());
                } else {
                    HILOG_ERROR("UnregisterApplicationStateObserver failed error:%{public}d", ret);
                    task.Reject(engine, CreateJsErrorByNativeErr(engine, ret));
                }
            };

        NativeValue* lastParam = (info.argc > ARGC_TWO) ? info.argv[INDEX_TWO] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnUnregisterApplicationStateObserver",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetForegroundApplications(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        AsyncTask::CompleteCallback complete =
            [appManager = appManager_](NativeEngine& engine, AsyncTask& task, int32_t status) {
                if (appManager == nullptr) {
                    HILOG_ERROR("appManager nullptr");
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                    return;
                }
                std::vector<AppExecFwk::AppStateData> list;
                int32_t ret = appManager->GetForegroundApplications(list);
                if (ret == 0) {
                    HILOG_DEBUG("OnGetForegroundApplications success.");
                    task.ResolveWithNoError(engine, CreateJsAppStateDataArray(engine, list));
                } else {
                    HILOG_ERROR("OnGetForegroundApplications failed error:%{public}d", ret);
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED));
                }
            };

        NativeValue* lastParam = (info.argc > ARGC_ZERO) ? info.argv[INDEX_ZERO] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnGetForegroundApplications",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetRunningProcessInformation(NativeEngine &engine, const NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        AsyncTask::CompleteCallback complete =
            [appManager = appManager_](NativeEngine &engine, AsyncTask &task, int32_t status) {
                if (appManager == nullptr) {
                    HILOG_WARN("abilityManager nullptr");
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                    return;
                }
                std::vector<AppExecFwk::RunningProcessInfo> infos;
                auto ret = appManager->GetAllRunningProcesses(infos);
                if (ret == 0) {
                    task.ResolveWithNoError(engine, CreateJsRunningProcessInfoArray(engine, infos));
                } else {
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED));
                }
            };

        NativeValue* lastParam = (info.argc > ARGC_ZERO) ? info.argv[INDEX_ZERO] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnGetRunningProcessInformation",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnIsRunningInStabilityTest(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        AsyncTask::CompleteCallback complete =
            [abilityManager = abilityManager_](NativeEngine& engine, AsyncTask& task, int32_t status) {
                if (abilityManager == nullptr) {
                    HILOG_WARN("abilityManager nullptr");
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                    return;
                }
                bool ret = abilityManager->IsRunningInStabilityTest();
                HILOG_INFO("IsRunningInStabilityTest result:%{public}d", ret);
                task.ResolveWithNoError(engine, CreateJsValue(engine, ret));
            };

        NativeValue* lastParam = (info.argc > ARGC_ZERO) ? info.argv[INDEX_ZERO] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnIsRunningInStabilityTest",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnkillProcessesByBundleName(NativeEngine &engine, const NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("Params not match");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }

        std::string bundleName;
        if (!ConvertFromJsValue(engine, info.argv[0], bundleName)) {
            HILOG_ERROR("get bundleName failed!");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        HILOG_INFO("kill process [%{public}s]", bundleName.c_str());
        AsyncTask::CompleteCallback complete =
            [bundleName, abilityManager = abilityManager_](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (abilityManager == nullptr) {
                HILOG_WARN("abilityManager nullptr");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto ret = abilityManager->KillProcess(bundleName);
            if (ret == 0) {
                task.ResolveWithNoError(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, ret, "kill process failed."));
            }
        };

        NativeValue* lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnkillProcessesByBundleName",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnClearUpApplicationData(NativeEngine &engine, const NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("Params not match");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }

        std::string bundleName;
        if (!ConvertFromJsValue(engine, info.argv[0], bundleName)) {
            HILOG_ERROR("get bundleName failed!");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [bundleName, abilityManager = abilityManager_](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (abilityManager == nullptr) {
                HILOG_WARN("abilityManager nullptr");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto ret = abilityManager->ClearUpApplicationData(bundleName);
            if (ret == 0) {
                task.ResolveWithNoError(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, ret, "clear up application failed."));
            }
        };

        NativeValue* lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnClearUpApplicationData",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnKillProcessWithAccount(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc != ARGC_TWO && info.argc != ARGC_THREE) {
            HILOG_ERROR("Params not match");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }

        std::string bundleName;
        if (!ConvertFromJsValue(engine, info.argv[0], bundleName)) {
            HILOG_ERROR("Parse bundleName failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
        int32_t accountId = -1;
        if (!ConvertFromJsValue(engine, info.argv[1], accountId)) {
            HILOG_ERROR("Parse userId failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [appManager = appManager_, bundleName, accountId](NativeEngine &engine, AsyncTask &task, int32_t status) {
                if (appManager == nullptr || appManager->GetAmsMgr() == nullptr) {
                    HILOG_WARN("appManager is nullptr or amsMgr is nullptr.");
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                    return;
                }
                auto ret = appManager->GetAmsMgr()->KillProcessWithAccount(bundleName, accountId);
                if (ret == 0) {
                    task.ResolveWithNoError(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsErrorByNativeErr(engine, ret, "Kill processes failed."));
                }
            };

        NativeValue* lastParam = (info.argc == ARGC_THREE) ? info.argv[INDEX_TWO] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnKillProcessWithAccount",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetAppMemorySize(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        AsyncTask::CompleteCallback complete =
            [abilityManager = abilityManager_](NativeEngine& engine, AsyncTask& task, int32_t status) {
                if (abilityManager == nullptr) {
                    HILOG_WARN("abilityManager nullptr");
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                    return;
                }
                int32_t memorySize = abilityManager->GetAppMemorySize();
                HILOG_INFO("GetAppMemorySize memorySize:%{public}d", memorySize);
                task.ResolveWithNoError(engine, CreateJsValue(engine, memorySize));
            };

        NativeValue* lastParam = (info.argc > ARGC_ZERO) ? info.argv[INDEX_ZERO] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnGetAppMemorySize",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnIsRamConstrainedDevice(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        AsyncTask::CompleteCallback complete =
            [abilityManager = abilityManager_](NativeEngine& engine, AsyncTask& task, int32_t status) {
                if (abilityManager == nullptr) {
                    HILOG_WARN("abilityManager nullptr");
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                    return;
                }
                bool ret = abilityManager->IsRamConstrainedDevice();
                HILOG_INFO("IsRamConstrainedDevice result:%{public}d", ret);
                task.ResolveWithNoError(engine, CreateJsValue(engine, ret));
            };

        NativeValue* lastParam = (info.argc > ARGC_ZERO) ? info.argv[INDEX_ZERO] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnIsRamConstrainedDevice",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    bool CheckOnOffType(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        if (info.argc < ARGC_ONE) {
            return false;
        }

        if (info.argv[0]->TypeOf() != NATIVE_STRING) {
            HILOG_ERROR("Param 0 is not string");
            return false;
        }

        std::string type;
        if (!ConvertFromJsValue(engine, info.argv[0], type)) {
            HILOG_ERROR("Parse on off type failed");
            return false;
        }

        if (type != ON_OFF_TYPE) {
            HILOG_ERROR("args[0] should be %{public}s.", ON_OFF_TYPE);
            return false;
        }
        return true;
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

NativeValue* JsAppManagerInit(NativeEngine* engine, NativeValue* exportObj)
{
    HILOG_INFO("JsAppManagerInit is called");

    if (engine == nullptr || exportObj == nullptr) {
        HILOG_WARN("engine or exportObj null");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_WARN("object null");
        return nullptr;
    }

    std::unique_ptr<JsAppManager> jsAppManager =
        std::make_unique<JsAppManager>(GetAppManagerInstance(), GetAbilityManagerInstance());
    object->SetNativePointer(jsAppManager.release(), JsAppManager::Finalizer, nullptr);

    const char *moduleName = "AppManager";
    BindNativeFunction(*engine, *object, "on", moduleName, JsAppManager::On);
    BindNativeFunction(*engine, *object, "off", moduleName, JsAppManager::Off);
    BindNativeFunction(*engine, *object, "getForegroundApplications", moduleName,
        JsAppManager::GetForegroundApplications);
    BindNativeFunction(*engine, *object, "getProcessRunningInfos", moduleName,
        JsAppManager::GetRunningProcessInformation);
    BindNativeFunction(*engine, *object, "getProcessRunningInformation", moduleName,
        JsAppManager::GetRunningProcessInformation);
    BindNativeFunction(*engine, *object, "getRunningProcessInformation", moduleName,
        JsAppManager::GetRunningProcessInformation);
    BindNativeFunction(*engine, *object, "isRunningInStabilityTest", moduleName,
        JsAppManager::IsRunningInStabilityTest);
    BindNativeFunction(*engine, *object, "killProcessWithAccount", moduleName,
        JsAppManager::KillProcessWithAccount);
    BindNativeFunction(*engine, *object, "killProcessesByBundleName", moduleName,
        JsAppManager::KillProcessesByBundleName);
    BindNativeFunction(*engine, *object, "clearUpApplicationData", moduleName,
        JsAppManager::ClearUpApplicationData);
    BindNativeFunction(*engine, *object, "getAppMemorySize", moduleName,
        JsAppManager::GetAppMemorySize);
    BindNativeFunction(*engine, *object, "isRamConstrainedDevice", moduleName,
        JsAppManager::IsRamConstrainedDevice);
    HILOG_INFO("JsAppManagerInit end");
    return engine->CreateUndefined();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
