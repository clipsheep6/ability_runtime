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

    NativeValue* OnRegisterApplicationStateObserver(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        // only support 1 params
        if (info.argc != ARGC_ONE) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }
        if (appManager_ == nullptr) {
            HILOG_ERROR("appManager nullptr");
            return engine.CreateUndefined();
        }
        // unwarp observer
        sptr<JSApplicationStateObserver> observer = new JSApplicationStateObserver(engine);
        observer->SetJsObserverObject(info.argv[0]);
        int32_t ret = appManager_->RegisterApplicationStateObserver(observer);
        if (ret == 0) {
            HILOG_INFO("RegisterApplicationStateObserver success.");
            int64_t observerId = serialNumber_;
            observerIds_.emplace(observerId, observer);
            if (serialNumber_ < INT64_MAX) {
                serialNumber_++;
            } else {
                serialNumber_ = 0;
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
        sptr<JSApplicationStateObserver> observer = nullptr;

        // only support 1 or 2 params
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("Not enough params");
            errCode = ERR_NOT_OK;
        } else {
            // unwrap connectId
            napi_get_value_int64(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), &observerId);
            HILOG_INFO("OnUnregisterApplicationStateObserver observer:%{public}d", (int32_t)observerId);
            auto item = observerIds_.find(observerId);
            if (item != observerIds_.end()) {
                // match id
                observer = item->second;
                HILOG_INFO("%{public}s find observer exist", __func__);
            } else {
                HILOG_INFO("%{public}s not find observer exist.", __func__);
                errCode = ERR_NOT_OK;
            }
        }

        AsyncTask::CompleteCallback complete =
            [appManager = appManager_, observer, observerId, errCode](
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
                HILOG_INFO("observer->UnregisterApplicationStateObserver");
                int32_t ret = appManager->UnregisterApplicationStateObserver(observer);
                if (ret == 0) {
                    HILOG_INFO("UnregisterApplicationStateObserver success.");
                    task.Resolve(engine, engine.CreateUndefined());
                    observerIds_.erase(observerId);
                    HILOG_INFO("UnregisterApplicationStateObserver erase size:%{public}zu", observerIds_.size());
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

    NativeValue* OnGetForegroundApplications(NativeEngine& engine, NativeCallbackInfo& info)
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
                    HILOG_INFO("OnGetForegroundApplications success.");
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

    NativeValue* OnGetProcessRunningInfos(NativeEngine &engine, NativeCallbackInfo &info)
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

    NativeValue* OnIsRunningInStabilityTest(NativeEngine& engine, NativeCallbackInfo& info)
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
                HILOG_INFO("IsRunningInStabilityTest begin");
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

    NativeValue* OnkillProcessByBundleName(NativeEngine &engine, NativeCallbackInfo &info)
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

    NativeValue* OnClearUpApplicationData(NativeEngine &engine, NativeCallbackInfo &info)
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
        int accountId = -1;
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
    
    NativeValue* OnGetAppMemorySize(NativeEngine& engine, NativeCallbackInfo& info)
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
                HILOG_INFO("GetAppMemorySize begin");
                int memorySize = abilityManager->GetAppMemorySize();
                HILOG_INFO("GetAppMemorySize memorySize:%{public}d", memorySize);
                task.Resolve(engine, CreateJsValue(engine, memorySize));
            };

        NativeValue* lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnGetAppMemorySize",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnIsRamConstrainedDevice(NativeEngine& engine, NativeCallbackInfo& info)
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
                HILOG_INFO("IsRamConstrainedDevice begin");
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
        HILOG_INFO("engine or exportObj null");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_INFO("object null");
        return nullptr;
    }

    std::unique_ptr<JsAppManager> jsAppManager =
        std::make_unique<JsAppManager>(GetAppManagerInstance(), GetAbilityManagerInstance());
    object->SetNativePointer(jsAppManager.release(), JsAppManager::Finalizer, nullptr);

    // make handler
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());

    HILOG_INFO("JsAppManagerInit BindNativeFunction called");
    BindNativeFunction(*engine, *object, "registerApplicationStateObserver",
        JsAppManager::RegisterApplicationStateObserver);
    BindNativeFunction(*engine, *object, "unregisterApplicationStateObserver",
        JsAppManager::UnregisterApplicationStateObserver);
    BindNativeFunction(*engine, *object, "getForegroundApplications",
        JsAppManager::GetForegroundApplications);
    BindNativeFunction(*engine, *object, "getProcessRunningInfos",
        JsAppManager::GetProcessRunningInfos);
    BindNativeFunction(*engine, *object, "isRunningInStabilityTest",
        JsAppManager::IsRunningInStabilityTest);
    BindNativeFunction(*engine, *object, "killProcessWithAccount",
        JsAppManager::KillProcessWithAccount);
    BindNativeFunction(*engine, *object, "killProcessesByBundleName",
        JsAppManager::KillProcessesByBundleName);
    BindNativeFunction(*engine, *object, "clearUpApplicationData",
        JsAppManager::ClearUpApplicationData);
    BindNativeFunction(*engine, *object, "getAppMemorySize",
        JsAppManager::GetAppMemorySize);
    BindNativeFunction(*engine, *object, "isRamConstrainedDevice",
        JsAppManager::IsRamConstrainedDevice);
    HILOG_INFO("JsAppManagerInit end");
    return engine->CreateUndefined();
}

JSApplicationStateObserver::JSApplicationStateObserver(NativeEngine& engine) : engine_(engine) {}

JSApplicationStateObserver::~JSApplicationStateObserver() = default;

void JSApplicationStateObserver::OnForegroundApplicationChanged(const AppStateData &appStateData)
{
    HILOG_DEBUG("onForegroundApplicationChanged bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    wptr<JSApplicationStateObserver> jsObserver = this;
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([jsObserver, appStateData](NativeEngine &engine, AsyncTask &task, int32_t status) {
            sptr<JSApplicationStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_INFO("jsObserverSptr nullptr");
                return;
            }
            jsObserverSptr->HandleOnForegroundApplicationChanged(appStateData);
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JSApplicationStateObserver::OnForegroundApplicationChanged",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSApplicationStateObserver::HandleOnForegroundApplicationChanged(const AppStateData &appStateData)
{
    HILOG_DEBUG("HandleOnForegroundApplicationChanged bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    NativeValue* argv[] = {CreateJsAppStateData(engine_, appStateData)};
    CallJsFunction("onForegroundApplicationChanged", argv, ARGC_ONE);
}

void JSApplicationStateObserver::OnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_INFO("OnAbilityStateChanged begin");
    wptr<JSApplicationStateObserver> jsObserver = this;
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([jsObserver, abilityStateData](NativeEngine &engine, AsyncTask &task, int32_t status) {
            sptr<JSApplicationStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_INFO("jsObserverSptr nullptr");
                return;
            }
            jsObserverSptr->HandleOnAbilityStateChanged(abilityStateData);
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JSApplicationStateObserver::OnAbilityStateChanged",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSApplicationStateObserver::HandleOnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_INFO("HandleOnAbilityStateChanged begin");
    NativeValue* argv[] = {CreateJsAbilityStateData(engine_, abilityStateData)};
    CallJsFunction("onAbilityStateChanged", argv, ARGC_ONE);
}

void JSApplicationStateObserver::OnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_INFO("OnExtensionStateChanged begin");
    wptr<JSApplicationStateObserver> jsObserver = this;
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([jsObserver, abilityStateData](NativeEngine &engine, AsyncTask &task, int32_t status) {
            sptr<JSApplicationStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_INFO("jsObserverSptr nullptr");
                return;
            }
            jsObserverSptr->HandleOnExtensionStateChanged(abilityStateData);
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JSApplicationStateObserver::OnExtensionStateChanged",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSApplicationStateObserver::HandleOnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_INFO("HandleOnExtensionStateChanged begin");
    NativeValue* argv[] = {CreateJsAbilityStateData(engine_, abilityStateData)};
    CallJsFunction("onAbilityStateChanged", argv, ARGC_ONE);
}

void JSApplicationStateObserver::OnProcessCreated(const ProcessData &processData)
{
    HILOG_INFO("OnProcessCreated begin");
    wptr<JSApplicationStateObserver> jsObserver = this;
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([jsObserver, processData](NativeEngine &engine, AsyncTask &task, int32_t status) {
            sptr<JSApplicationStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_INFO("jsObserverSptr nullptr");
                return;
            }
            jsObserverSptr->HandleOnProcessCreated(processData);
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JSApplicationStateObserver::OnProcessCreated",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSApplicationStateObserver::HandleOnProcessCreated(const ProcessData &processData)
{
    HILOG_INFO("HandleOnProcessCreated begin");
    NativeValue* argv[] = {CreateJsProcessData(engine_, processData)};
    CallJsFunction("onProcessCreated", argv, ARGC_ONE);
}

void JSApplicationStateObserver::OnProcessDied(const ProcessData &processData)
{
    HILOG_INFO("OnProcessDied begin");
    wptr<JSApplicationStateObserver> jsObserver = this;
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([jsObserver, processData](NativeEngine &engine, AsyncTask &task, int32_t status) {
            sptr<JSApplicationStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_INFO("jsObserverSptr nullptr");
                return;
            }
            jsObserverSptr->HandleOnProcessDied(processData);
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JSApplicationStateObserver::OnProcessCreated",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSApplicationStateObserver::HandleOnProcessDied(const ProcessData &processData)
{
    HILOG_INFO("HandleOnProcessDied begin");
    NativeValue* argv[] = {CreateJsProcessData(engine_, processData)};
    CallJsFunction("onProcessDied", argv, ARGC_ONE);
}

void JSApplicationStateObserver::CallJsFunction(const char* methodName, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("CallJsFunction begin, method:%{public}s", methodName);
    if (jsObserverObject_ == nullptr) {
        HILOG_ERROR("jsObserverObject_ nullptr");
        return;
    }
    NativeValue* value = jsObserverObject_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue* method = obj->GetProperty(methodName);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get from object");
        return;
    }
    HILOG_INFO("CallJsFunction CallFunction success");
    engine_.CallFunction(value, method, argv, argc);
    HILOG_INFO("CallJsFunction end");
}

void JSApplicationStateObserver::SetJsObserverObject(NativeValue* jsObserverObject)
{
    jsObserverObject_ = std::unique_ptr<NativeReference>(engine_.CreateReference(jsObserverObject, 1));
}
}  // namespace AbilityRuntime
}  // namespace OHOS