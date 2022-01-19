/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "mission_manager.h"

#include "ability_manager_client.h"
#include "event_handler.h"
#include "event_runner.h"
#include "hilog_wrapper.h"
#include "js_mission_info_utils.h"
#include "js_mission_listener.h"
#include "js_runtime_utils.h"
#include "mission_snapshot.h"
#include "pixel_map_napi.h"

#include <mutex>

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
using AbilityManagerClient = AAFwk::AbilityManagerClient;
namespace {
    constexpr int32_t ARG_COUNT_TWO = 2;
    constexpr int32_t ARG_COUNT_THREE = 3;
}
class JsMissionManager {
public:
    JsMissionManager() = default;
    ~JsMissionManager() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsMissionManager::Finalizer is called");
        std::unique_ptr<JsMissionManager>(static_cast<JsMissionManager*>(data));
    }

    static NativeValue* RegisterMissionListener(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsMissionManager* me = CheckParamsAndGetThis<JsMissionManager>(engine, info);
        return (me != nullptr) ? me->OnRegisterMissionListener(*engine, *info) : nullptr;
    }

    static NativeValue* UnregisterMissionListener(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsMissionManager* me = CheckParamsAndGetThis<JsMissionManager>(engine, info);
        return (me != nullptr) ? me->OnUnregisterMissionListener(*engine, *info) : nullptr;
    }

    static NativeValue* GetMissionInfos(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsMissionManager* me = CheckParamsAndGetThis<JsMissionManager>(engine, info);
        return (me != nullptr) ? me->OnGetMissionInfos(*engine, *info) : nullptr;
    }

    static NativeValue* GetMissionInfo(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsMissionManager* me = CheckParamsAndGetThis<JsMissionManager>(engine, info);
        return (me != nullptr) ? me->OnGetMissionInfo(*engine, *info) : nullptr;
    }

    static NativeValue* GetMissionSnapShot(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsMissionManager* me = CheckParamsAndGetThis<JsMissionManager>(engine, info);
        return (me != nullptr) ? me->OnGetMissionSnapShot(*engine, *info) : nullptr;
    }

    static NativeValue* LockMission(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsMissionManager* me = CheckParamsAndGetThis<JsMissionManager>(engine, info);
        return (me != nullptr) ? me->OnLockMission(*engine, *info) : nullptr;
    }

    static NativeValue* UnlockMission(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsMissionManager* me = CheckParamsAndGetThis<JsMissionManager>(engine, info);
        return (me != nullptr) ? me->OnUnlockMission(*engine, *info) : nullptr;
    }

    static NativeValue* ClearMission(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsMissionManager* me = CheckParamsAndGetThis<JsMissionManager>(engine, info);
        return (me != nullptr) ? me->OnClearMission(*engine, *info) : nullptr;
    }

    static NativeValue* ClearAllMissions(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsMissionManager* me = CheckParamsAndGetThis<JsMissionManager>(engine, info);
        return (me != nullptr) ? me->OnClearAllMissions(*engine, *info) : nullptr;
    }

    static NativeValue* MoveMissionToFront(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsMissionManager* me = CheckParamsAndGetThis<JsMissionManager>(engine, info);
        return (me != nullptr) ? me->OnMoveMissionToFront(*engine, *info) : nullptr;
    }

private:
    NativeValue* OnRegisterMissionListener(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc != 1) {
            HILOG_ERROR("Params not match");
            return engine.CreateUndefined();
        }

        missionListenerId_++;
        if (missionListener_ != nullptr) {
            missionListener_->AddJsListenerObject(missionListenerId_, info.argv[0]);
            return engine.CreateNumber(missionListenerId_);
        }

        auto mainHandler = GetMainHandler();
        missionListener_ = new JsMissionListener(&engine, mainHandler);
        auto errcode = AbilityManagerClient::GetInstance()->RegisterMissionListener(missionListener_);
        if (errcode == 0) {
            missionListener_->AddJsListenerObject(missionListenerId_, info.argv[0]);
            return engine.CreateNumber(missionListenerId_);
        } else {
            HILOG_ERROR("RegisterMissionListener failed, ret = %{public}d", errcode);
            missionListener_ = nullptr;
            return engine.CreateUndefined();
        }
    }

    NativeValue* OnUnregisterMissionListener(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc < 1) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }
        uint32_t missionListenerId = -1;
        if (!ConvertFromJsValue(engine, info.argv[0], missionListenerId)) {
            HILOG_ERROR("Parse missionListenerId failed");
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [&missionListener = missionListener_, missionListenerId]
            (NativeEngine &engine, AsyncTask &task, int32_t status) {
                missionListener->RemoveJsListenerObject(missionListenerId);
                if (!missionListener->IsEmpty()) {
                    task.Resolve(engine, engine.CreateUndefined());
                    return;
                }
                auto errcode = AbilityManagerClient::GetInstance()->UnRegisterMissionListener(missionListener);
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                    missionListener = nullptr;
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Unregister mission listener failed."));
                }
            };

        NativeValue* lastParam = (info.argc == 1) ? nullptr : info.argv[1];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetMissionInfos(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc < 2) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }
        std::string deviceId;
        if (!ConvertFromJsValue(engine, info.argv[0], deviceId)) {
            HILOG_ERROR("Parse deviceId failed");
            return engine.CreateUndefined();
        }
        int numMax = -1;
        if (!ConvertFromJsValue(engine, info.argv[1], numMax)) {
            HILOG_ERROR("Parse numMax failed");
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [deviceId, numMax](NativeEngine &engine, AsyncTask &task, int32_t status) {
                std::vector<AAFwk::MissionInfo> missionInfos;
                auto errcode = AbilityManagerClient::GetInstance()->GetMissionInfos(deviceId, numMax, missionInfos);
                if (errcode == 0) {
                    task.Resolve(engine, CreateJsMissionInfoArray(engine, missionInfos));
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Get mission infos failed."));
                }
            };

        NativeValue* lastParam = (info.argc == 2) ? nullptr : info.argv[2];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetMissionInfo(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc < 2) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }
        std::string deviceId;
        if (!ConvertFromJsValue(engine, info.argv[0], deviceId)) {
            HILOG_ERROR("Parse deviceId failed");
            return engine.CreateUndefined();
        }
        int missionId = -1;
        if (!ConvertFromJsValue(engine, info.argv[1], missionId)) {
            HILOG_ERROR("Parse missionId failed");
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [deviceId, missionId](NativeEngine &engine, AsyncTask &task, int32_t status) {
                AAFwk::MissionInfo missionInfo;
                auto errcode = AbilityManagerClient::GetInstance()->GetMissionInfo(deviceId, missionId, missionInfo);
                if (errcode == 0) {
                    task.Resolve(engine, CreateJsMissionInfo(engine, missionInfo));
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Get mission info failed."));
                }
            };

        NativeValue* lastParam = (info.argc == 2) ? nullptr : info.argv[2];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetMissionSnapShot(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc != ARG_COUNT_TWO && info.argc != ARG_COUNT_THREE) {
            HILOG_ERROR("missionSnapshot: need two or three params");
            return engine.CreateUndefined();
        }
        std::string deviceId;
        if (!ConvertFromJsValue(engine, info.argv[0], deviceId)) {
            HILOG_ERROR("missionSnapshot: Parse deviceId failed");
            return engine.CreateUndefined();
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(engine, info.argv[1], missionId)) {
            HILOG_ERROR("missionSnapshot: Parse missionId failed");
            return engine.CreateUndefined();
        }
        AsyncTask::CompleteCallback complete =
            [deviceId, missionId](NativeEngine &engine, AsyncTask &task, int32_t status) {
                AAFwk::MissionSnapshot missionSnapshot;
                auto errcode = AbilityManagerClient::GetInstance()->GetMissionSnapshot(
                    deviceId, missionId, missionSnapshot);
                if (errcode == 0) {
                    NativeValue* objValue = engine.CreateObject();
                    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
                    NativeValue* abilityValue = engine.CreateObject();
                    NativeObject* abilityObj = ConvertNativeValueTo<NativeObject>(abilityValue);
                    abilityObj->SetProperty(
                        "bundleName", CreateJsValue(engine, missionSnapshot.topAbility.GetBundleName()));
                    abilityObj->SetProperty(
                        "abilityName", CreateJsValue(engine, missionSnapshot.topAbility.GetAbilityName()));
                    object->SetProperty("ability", abilityValue);
                    auto snapshotValue = reinterpret_cast<NativeValue*>(Media::PixelMapNapi::CreatePixelMap(
                        reinterpret_cast<napi_env>(&engine), missionSnapshot.snapshot));
                    object->SetProperty("snapshot", snapshotValue);
                    task.Resolve(engine, objValue);
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Get mission snapshot failed."));
                }
            };
        NativeValue* lastParam = (info.argc == ARG_COUNT_TWO) ? nullptr : info.argv[2];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnLockMission(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc == 0) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }
        int missionId = -1;
        if (!ConvertFromJsValue(engine, info.argv[0], missionId)) {
            HILOG_ERROR("Parse missionId failed");
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [missionId](NativeEngine &engine, AsyncTask &task, int32_t status) {
                auto errcode = AbilityManagerClient::GetInstance()->LockMissionForCleanup(missionId);
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Lock mission failed."));
                }
            };

        NativeValue* lastParam = (info.argc == 1) ? nullptr : info.argv[1];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnUnlockMission(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc == 0) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }
        int missionId = -1;
        if (!ConvertFromJsValue(engine, info.argv[0], missionId)) {
            HILOG_ERROR("Parse missionId failed");
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [missionId](NativeEngine &engine, AsyncTask &task, int32_t status) {
                auto errcode = AbilityManagerClient::GetInstance()->UnlockMissionForCleanup(missionId);
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Unlock mission failed."));
                }
            };

        NativeValue* lastParam = (info.argc == 1) ? nullptr : info.argv[1];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnClearMission(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc == 0) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }
        int missionId = -1;
        if (!ConvertFromJsValue(engine, info.argv[0], missionId)) {
            HILOG_ERROR("Parse missionId failed");
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [missionId](NativeEngine &engine, AsyncTask &task, int32_t status) {
                auto errcode = AbilityManagerClient::GetInstance()->CleanMission(missionId);
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Clear mission failed."));
                }
            };

        NativeValue* lastParam = (info.argc == 1) ? nullptr : info.argv[1];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnClearAllMissions(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        AsyncTask::CompleteCallback complete =
            [](NativeEngine &engine, AsyncTask &task, int32_t status) {
                auto errcode = AbilityManagerClient::GetInstance()->CleanAllMissions();
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Clear all missions failed."));
                }
            };

        NativeValue* lastParam = (info.argc == 0) ? nullptr : info.argv[0];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnMoveMissionToFront(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc == 0) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }
        int missionId = -1;
        if (!ConvertFromJsValue(engine, info.argv[0], missionId)) {
            HILOG_ERROR("Parse missionId failed");
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [missionId](NativeEngine &engine, AsyncTask &task, int32_t status) {
                auto errcode = AbilityManagerClient::GetInstance()->MoveMissionToFront(missionId);
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Move mission to front failed."));
                }
            };

        NativeValue* lastParam = (info.argc == 1) ? nullptr : info.argv[1];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    std::shared_ptr<EventHandler> GetMainHandler()
    {
        if (!mainHandler_) {
            mainHandler_ = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
        }
        return mainHandler_;
    }

    sptr<JsMissionListener> missionListener_ = nullptr;
    uint32_t missionListenerId_ = 0;
    std::shared_ptr<EventHandler> mainHandler_;
};

NativeValue* JsMissionManagerInit(NativeEngine* engine, NativeValue* exportObj)
{
    HILOG_INFO("JsMissionManagerInit is called");
    if (engine == nullptr || exportObj == nullptr) {
        HILOG_INFO("Invalid input parameters");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_INFO("object is nullptr");
        return nullptr;
    }

    std::unique_ptr<JsMissionManager> jsMissionManager = std::make_unique<JsMissionManager>();
    object->SetNativePointer(jsMissionManager.release(), JsMissionManager::Finalizer, nullptr);

    BindNativeFunction(*engine, *object, "registerMissionListener", JsMissionManager::RegisterMissionListener);
    BindNativeFunction(*engine, *object, "unregisterMissionListener", JsMissionManager::UnregisterMissionListener);
    BindNativeFunction(*engine, *object, "getMissionInfos", JsMissionManager::GetMissionInfos);
    BindNativeFunction(*engine, *object, "getMissionInfo", JsMissionManager::GetMissionInfo);
    BindNativeFunction(*engine, *object, "getMissionSnapShot", JsMissionManager::GetMissionSnapShot);
    BindNativeFunction(*engine, *object, "lockMission", JsMissionManager::LockMission);
    BindNativeFunction(*engine, *object, "unlockMission", JsMissionManager::UnlockMission);
    BindNativeFunction(*engine, *object, "clearMission", JsMissionManager::ClearMission);
    BindNativeFunction(*engine, *object, "clearAllMissions", JsMissionManager::ClearAllMissions);
    BindNativeFunction(*engine, *object, "moveMissionToFront", JsMissionManager::MoveMissionToFront);
    return engine->CreateUndefined();
}
}  // namespace AbilityRuntime
}  // namespace OHOS