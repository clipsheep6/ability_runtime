/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "app_mgr_interface.h"
#include "hilog_wrapper.h"
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
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr int32_t ERR_NOT_OK = -1;

std::mutex g_observerMutex;

class JsAppManager final {
public:
    JsAppManager(sptr<OHOS::AppExecFwk::IAppMgr> appManager,
        sptr<OHOS::AAFwk::IAbilityManager> abilityManager) : appManager_(appManager),
        abilityManager_(abilityManager) {}
    ~JsAppManager() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsAbilityContext::Finalizer is called");
        std::unique_ptr<JsAppManager>(static_cast<JsAppManager*>(data));
    }

    static NativeValue* RegisterApplicationStateObserver(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnRegisterApplicationStateObserver(*engine, *info) : nullptr;
    }

    static NativeValue* UnregisterApplicationStateObserver(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnUnregisterApplicationStateObserver(*engine, *info) : nullptr;
    }

    static NativeValue* GetForegroundApplications(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnGetForegroundApplications(*engine, *info) : nullptr;
    }

    static NativeValue* GetProcessRunningInfos(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAppManager* me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnGetProcessRunningInfos(*engine, *info) : nullptr;
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
        return (me != nullptr) ? me->OnkillProcessByBundleName(*engine, *info) : nullptr;
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

    NativeValue* OnRegisterApplicationStateObserver(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        // only support 1 or 2 params
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }
        if (appManager_ == nullptr) {
            HILOG_ERROR("appManager nullptr");
            return engine.CreateUndefined();
        }
        static int64_t serialNumber = 0;
        std::vector<std::string> bundleNameList;
        // unwarp observer
        if (observer_ == nullptr) {
            observer_ = new JSAppStateObserver(engine);
        }
        if (info.argc == ARGC_TWO) {
            AppExecFwk::UnwrapArrayStringFromJS(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[INDEX_ONE]), bundleNameList);
        }
        int32_t ret = appManager_->RegisterApplicationStateObserver(observer_, bundleNameList);
        if (ret == 0) {
            HILOG_DEBUG("RegisterApplicationStateObserver success.");
            std::lock_guard<std::mutex> lock(g_observerMutex);
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
            return engine.CreateUndefined();
        }
    }

    NativeValue* OnUnregisterApplicationStateObserver(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        int32_t errCode = 0;
        int64_t observerId = -1;

        // only support 1 or 2 params
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("Not enough params");
            errCode = ERR_NOT_OK;
        } else {
            // unwrap connectId
            napi_get_value_int64(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), &observerId);
            std::lock_guard<std::mutex> lock(g_observerMutex);
            bool isExist = observer_->FindObserverByObserverId(observerId);
            if (isExist) {
                // match id
                HILOG_INFO("%{public}s find observer exist observer:%{public}d", __func__,
                    static_cast<int32_t>(observerId));
            } else {
                HILOG_INFO("%{public}s not find observer, observer:%{public}d", __func__,
                    static_cast<int32_t>(observerId));
                errCode = ERR_NOT_OK;
            }
        }

        AsyncTask::CompleteCallback complete =
            [appManager = appManager_, observer = observer_, observerId, errCode](
                NativeEngine& engine, AsyncTask& task, int32_t status) {
                if (errCode != 0) {
                    task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
                    return;
                }
                if (observer == nullptr || appManager == nullptr) {
                    HILOG_ERROR("observer or appManager nullptr");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "observer or appManager nullptr"));
                    return;
                }
                int32_t ret = appManager->UnregisterApplicationStateObserver(observer);
                if (ret == 0 && observer->RemoveJsObserverObject(observerId)) {
                    task.Resolve(engine, engine.CreateUndefined());
                    std::lock_guard<std::mutex> lock(g_observerMutex);
                    HILOG_DEBUG("UnregisterApplicationStateObserver success size:%{public}zu",
                        observer->GetJsObserverMapSize());
                } else {
                    HILOG_ERROR("UnregisterApplicationStateObserver failed error:%{public}d", ret);
                    task.Reject(engine, CreateJsError(engine, ret, "UnregisterApplicationStateObserver failed"));
                }
            };

        NativeValue* lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnUnregisterApplicationStateObserver",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetForegroundApplications(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        int32_t errCode = 0;

        // only support 0 or 1 params
        if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
            HILOG_ERROR("Not enough params");
            errCode = ERR_NOT_OK;
        }
        AsyncTask::CompleteCallback complete =
            [appManager = appManager_, errCode](NativeEngine& engine, AsyncTask& task, int32_t status) {
                if (errCode != 0) {
                    task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
                    return;
                }
                if (appManager == nullptr) {
                    HILOG_ERROR("appManager nullptr");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "appManager nullptr"));
                    return;
                }
                std::vector<AppExecFwk::AppStateData> list;
                int32_t ret = appManager->GetForegroundApplications(list);
                if (ret == 0) {
                    HILOG_DEBUG("OnGetForegroundApplications success.");
                    task.Resolve(engine, CreateJsAppStateDataArray(engine, list));
                } else {
                    HILOG_ERROR("OnGetForegroundApplications failed error:%{public}d", ret);
                    task.Reject(engine, CreateJsError(engine, ret, "OnGetForegroundApplications failed"));
                }
            };

        NativeValue* lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnGetForegroundApplications",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetProcessRunningInfos(NativeEngine &engine, const NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        int32_t errCode = 0;

        // only support 0 or 1 params
        if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
            HILOG_ERROR("Not enough params");
            errCode = ERR_NOT_OK;
        }
        AsyncTask::CompleteCallback complete =
            [appManager = appManager_, errCode](NativeEngine &engine, AsyncTask &task, int32_t status) {
                if (errCode != 0) {
                    task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
                    return;
                }
                std::vector<AppExecFwk::RunningProcessInfo> infos;
                auto ret = appManager->GetAllRunningProcesses(infos);
                HILOG_INFO("xts_test_tag %{public}s, GetAllRunningProcesses ret: %{public}d", __FUNCTION__, ret);
                if (ret == 0) {
                    task.Resolve(engine, CreateJsProcessRunningInfoArray(engine, infos));
                } else {
                    task.Reject(engine, CreateJsError(engine, ret, "Get mission infos failed."));
                }
            };

        NativeValue* lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnGetProcessRunningInfos",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnIsRunningInStabilityTest(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        int32_t errCode = 0;

        // only support 0 or 1 params
        if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
            HILOG_ERROR("Not enough params");
            errCode = ERR_NOT_OK;
        }
        AsyncTask::CompleteCallback complete =
            [abilityManager = abilityManager_, errCode](NativeEngine& engine, AsyncTask& task, int32_t status) {
                if (errCode != 0) {
                    task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
                    return;
                }
                if (abilityManager == nullptr) {
                    HILOG_WARN("abilityManager nullptr");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "abilityManager nullptr"));
                    return;
                }
                bool ret = abilityManager->IsRunningInStabilityTest();
                HILOG_INFO("IsRunningInStabilityTest result:%{public}d", ret);
                task.Resolve(engine, CreateJsValue(engine, ret));
            };

        NativeValue* lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnIsRunningInStabilityTest",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnkillProcessByBundleName(NativeEngine &engine, const NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        int32_t errCode = 0;
        std::string bundleName;

        // only support 1 or 2 params
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("Not enough params");
            errCode = ERR_NOT_OK;
        } else {
            if (!ConvertFromJsValue(engine, info.argv[0], bundleName)) {
                HILOG_ERROR("get bundleName failed!");
                errCode = ERR_NOT_OK;
            }
        }

        HILOG_INFO("kill process [%{public}s]", bundleName.c_str());
        AsyncTask::CompleteCallback complete =
            [bundleName, abilityManager = abilityManager_, errCode](NativeEngine& engine, AsyncTask& task,
                int32_t status) {
            if (errCode != 0) {
                task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
                return;
            }
            if (abilityManager == nullptr) {
                HILOG_WARN("abilityManager nullptr");
                task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "abilityManager nullptr"));
                return;
            }
            auto ret = abilityManager->KillProcess(bundleName);
            if (ret == 0) {
                task.Resolve(engine, CreateJsValue(engine, ret));
            } else {
                task.Reject(engine, CreateJsError(engine, ret, "kill process failed."));
            }
        };

        NativeValue* lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnkillProcessByBundleName",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnClearUpApplicationData(NativeEngine &engine, const NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        int32_t errCode = 0;
        std::string bundleName;

        // only support 1 or 2 params
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("Not enough params");
            errCode = ERR_NOT_OK;
        } else {
            if (!ConvertFromJsValue(engine, info.argv[0], bundleName)) {
                HILOG_ERROR("get bundleName failed!");
                errCode = ERR_NOT_OK;
            } else {
                HILOG_INFO("kill process [%{public}s]", bundleName.c_str());
            }
        }

        AsyncTask::CompleteCallback complete =
            [bundleName, abilityManager = abilityManager_, errCode](NativeEngine& engine, AsyncTask& task,
                int32_t status) {
            if (errCode != 0) {
                task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
                return;
            }
            if (abilityManager == nullptr) {
                HILOG_WARN("abilityManager nullptr");
                task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "abilityManager nullptr"));
                return;
            }
            auto ret = abilityManager->ClearUpApplicationData(bundleName);
            if (ret == 0) {
                task.Resolve(engine, CreateJsValue(engine, ret));
            } else {
                task.Reject(engine, CreateJsError(engine, ret, "clear up application failed."));
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
        int32_t errCode = 0;
        int32_t accountId = -1;
        std::string bundleName;

        // only support 2 or 3 params
        if (info.argc != ARGC_TWO && info.argc != ARGC_THREE) {
            HILOG_ERROR("Not enough params");
            errCode = ERR_NOT_OK;
        } else {
            if (!ConvertFromJsValue(engine, info.argv[0], bundleName)) {
                HILOG_ERROR("Parse bundleName failed");
                errCode = ERR_NOT_OK;
            }
            if (!ConvertFromJsValue(engine, info.argv[1], accountId)) {
                HILOG_ERROR("Parse userId failed");
                errCode = ERR_NOT_OK;
            }
        }

        AsyncTask::CompleteCallback complete =
            [appManager = appManager_, bundleName, accountId, errCode](NativeEngine &engine, AsyncTask &task,
                int32_t status) {
                if (errCode != 0) {
                    task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
                    return;
                }
                auto ret = appManager->GetAmsMgr()->KillProcessWithAccount(bundleName, accountId);
                if (ret == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, ret, "Kill processes failed."));
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
        int32_t errCode = 0;

        // only support 0 or 1 params
        if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
            HILOG_ERROR("Not enough params");
            errCode = ERR_NOT_OK;
        }
        AsyncTask::CompleteCallback complete =
            [abilityManager = abilityManager_, errCode](NativeEngine& engine, AsyncTask& task, int32_t status) {
                if (errCode != 0) {
                    task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
                    return;
                }
                if (abilityManager == nullptr) {
                    HILOG_WARN("abilityManager nullptr");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "abilityManager nullptr"));
                    return;
                }
                int32_t memorySize = abilityManager->GetAppMemorySize();
                HILOG_INFO("GetAppMemorySize memorySize:%{public}d", memorySize);
                task.Resolve(engine, CreateJsValue(engine, memorySize));
            };

        NativeValue* lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnGetAppMemorySize",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnIsRamConstrainedDevice(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        int32_t errCode = 0;

        // only support 0 or 1 params
        if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
            HILOG_ERROR("Not enough params");
            errCode = ERR_NOT_OK;
        }
        AsyncTask::CompleteCallback complete =
            [abilityManager = abilityManager_, errCode](NativeEngine& engine, AsyncTask& task, int32_t status) {
                if (errCode != 0) {
                    task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
                    return;
                }
                if (abilityManager == nullptr) {
                    HILOG_WARN("abilityManager nullptr");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "abilityManager nullptr"));
                    return;
                }
                bool ret = abilityManager->IsRamConstrainedDevice();
                HILOG_INFO("IsRamConstrainedDevice result:%{public}d", ret);
                task.Resolve(engine, CreateJsValue(engine, ret));
            };

        NativeValue* lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnIsRamConstrainedDevice",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
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

    const char *moduleName = "JsAppManager";
    BindNativeFunction(*engine, *object, "registerApplicationStateObserver", moduleName,
        JsAppManager::RegisterApplicationStateObserver);
    BindNativeFunction(*engine, *object, "unregisterApplicationStateObserver", moduleName,
        JsAppManager::UnregisterApplicationStateObserver);
    BindNativeFunction(*engine, *object, "getForegroundApplications", moduleName,
        JsAppManager::GetForegroundApplications);
    BindNativeFunction(*engine, *object, "getProcessRunningInfos", moduleName,
        JsAppManager::GetProcessRunningInfos);
    BindNativeFunction(*engine, *object, "getProcessRunningInformation", moduleName,
        JsAppManager::GetProcessRunningInfos);
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
