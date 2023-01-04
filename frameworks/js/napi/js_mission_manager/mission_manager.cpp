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

#include "mission_manager.h"

#include "ability_manager_client.h"
#include "event_handler.h"
#include "event_runner.h"
#include "hilog_wrapper.h"
#include "js_error_utils.h"
#include "js_mission_info_utils.h"
#include "js_mission_listener.h"
#include "js_runtime_utils.h"
#include "mission_snapshot.h"
#include "napi_common_start_options.h"
#include "permission_constants.h"
#ifdef SUPPORT_GRAPHICS
#include "pixel_map_napi.h"
#endif
#include "start_options.h"

#include <mutex>

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
using AbilityManagerClient = AAFwk::AbilityManagerClient;
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr int32_t ARG_COUNT_TWO = 2;
constexpr const char* ON_OFF_TYPE = "mission";
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
        return (me != nullptr) ? me->OnGetMissionSnapShot(*engine, *info, false) : nullptr;
    }

    static NativeValue* GetLowResolutionMissionSnapShot(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsMissionManager* me = CheckParamsAndGetThis<JsMissionManager>(engine, info);
        return (me != nullptr) ? me->OnGetMissionSnapShot(*engine, *info, true) : nullptr;
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
        if (info.argc < ARG_COUNT_TWO) {
            HILOG_ERROR("Params not match");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }

        if (!CheckOnOffType(engine, info)) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        missionListenerId_++;
        if (missionListener_ != nullptr) {
            missionListener_->AddJsListenerObject(missionListenerId_, info.argv[ARGC_ONE]);
            return engine.CreateNumber(missionListenerId_);
        }

        missionListener_ = new JsMissionListener(&engine);
        auto ret = AbilityManagerClient::GetInstance()->RegisterMissionListener(missionListener_);
        if (ret == 0) {
            missionListener_->AddJsListenerObject(missionListenerId_, info.argv[ARGC_ONE]);
            return engine.CreateNumber(missionListenerId_);
        } else {
            HILOG_ERROR("RegisterMissionListener failed, ret = %{public}d", ret);
            missionListener_ = nullptr;
            if (ret == CHECK_PERMISSION_FAILED) {
                ThrowNoPermissionError(engine, PermissionConstants::PERMISSION_MANAGE_MISSION);
            } else {
                ThrowError(engine, GetJsErrorCodeByNativeError(ret));
            }
            return engine.CreateUndefined();
        }
    }

    NativeValue* OnUnregisterMissionListener(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc < ARG_COUNT_TWO) {
            HILOG_ERROR("Not enough params");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }

        if (!CheckOnOffType(engine, info)) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        int32_t missionListenerId = -1;
        if (!ConvertFromJsValue(engine, info.argv[ARGC_ONE], missionListenerId)) {
            HILOG_ERROR("Parse missionListenerId failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [&missionListener = missionListener_, missionListenerId]
            (NativeEngine &engine, AsyncTask &task, int32_t status) {
                if (!missionListener || !missionListener->RemoveJsListenerObject(missionListenerId)) {
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_NO_MISSION_LISTENER));
                    return;
                }

                if (!missionListener->IsEmpty()) {
                    task.ResolveWithNoError(engine, engine.CreateUndefined());
                    return;
                }
                auto ret = AbilityManagerClient::GetInstance()->UnRegisterMissionListener(missionListener);
                if (ret == 0) {
                    task.ResolveWithNoError(engine, engine.CreateUndefined());
                    missionListener = nullptr;
                } else {
                    task.Reject(engine,
                        CreateJsErrorByNativeErr(engine, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        NativeValue* lastParam = (info.argc <= 1) ? nullptr : info.argv[1];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("MissioManager::OnUnregisterMissionListener",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetMissionInfos(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc < 2) { // at least 2 parameters.
            HILOG_ERROR("Not enough params");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }
        std::string deviceId;
        if (!ConvertFromJsValue(engine, info.argv[0], deviceId)) {
            HILOG_ERROR("Parse deviceId failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
        int numMax = -1;
        if (!ConvertFromJsValue(engine, info.argv[1], numMax)) {
            HILOG_ERROR("Parse numMax failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [deviceId, numMax](NativeEngine &engine, AsyncTask &task, int32_t status) {
                std::vector<AAFwk::MissionInfo> missionInfos;
                auto ret = AbilityManagerClient::GetInstance()->GetMissionInfos(deviceId, numMax, missionInfos);
                if (ret == 0) {
                    task.ResolveWithNoError(engine, CreateJsMissionInfoArray(engine, missionInfos));
                } else {
                    task.Reject(engine,
                        CreateJsErrorByNativeErr(engine, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        NativeValue* lastParam = (info.argc <= 2) ? nullptr : info.argv[2];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("MissioManager::OnGetMissionInfos",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetMissionInfo(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc < 2) {
            HILOG_ERROR("Not enough params");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }
        std::string deviceId;
        if (!ConvertFromJsValue(engine, info.argv[0], deviceId)) {
            HILOG_ERROR("Parse deviceId failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(engine, info.argv[1], missionId)) {
            HILOG_ERROR("Parse missionId failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [deviceId, missionId](NativeEngine &engine, AsyncTask &task, int32_t status) {
                AAFwk::MissionInfo missionInfo;
                auto ret = AbilityManagerClient::GetInstance()->GetMissionInfo(deviceId, missionId, missionInfo);
                if (ret == 0) {
                    task.ResolveWithNoError(engine, CreateJsMissionInfo(engine, missionInfo));
                } else {
                    task.Reject(engine,
                        CreateJsErrorByNativeErr(engine, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        NativeValue* lastParam = (info.argc <= 2) ? nullptr : info.argv[2];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("MissioManager::OnGetMissionInfo",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetMissionSnapShot(NativeEngine &engine, NativeCallbackInfo &info, bool isLowResolution)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        std::string deviceId;
        int32_t missionId = -1;
        if (!CheckMissionSnapShotParams(engine, info, deviceId, missionId)) {
            return engine.CreateUndefined();
        }

        class MissionSnapshotWrap {
        public:
            int result = -1;
            AAFwk::MissionSnapshot missionSnapshot;
        };

        std::shared_ptr<MissionSnapshotWrap> snapshotWrap = std::make_shared<MissionSnapshotWrap>();
        auto excute = [deviceId, missionId, isLowResolution, snapshotWrap]() {
            snapshotWrap->result = AbilityManagerClient::GetInstance()->GetMissionSnapshot(
                deviceId, missionId, snapshotWrap->missionSnapshot, isLowResolution);
        };

        auto complete = [snapshotWrap](NativeEngine &engine, AsyncTask &task, int32_t status) {
            if (snapshotWrap->result == 0) {
                NativeValue* objValue = engine.CreateObject();
                NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
                NativeValue* abilityValue = engine.CreateObject();
                NativeObject* abilityObj = ConvertNativeValueTo<NativeObject>(abilityValue);
                abilityObj->SetProperty(
                    "bundleName", CreateJsValue(engine, snapshotWrap->missionSnapshot.topAbility.GetBundleName()));
                abilityObj->SetProperty(
                    "abilityName", CreateJsValue(engine, snapshotWrap->missionSnapshot.topAbility.GetAbilityName()));
                object->SetProperty("ability", abilityValue);
#ifdef SUPPORT_GRAPHICS
                auto snapshotValue = reinterpret_cast<NativeValue*>(Media::PixelMapNapi::CreatePixelMap(
                    reinterpret_cast<napi_env>(&engine), snapshotWrap->missionSnapshot.snapshot));
                object->SetProperty("snapshot", snapshotValue);
#endif
                task.ResolveWithNoError(engine, objValue);
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, snapshotWrap->result,
                    PermissionConstants::PERMISSION_MANAGE_MISSION));
            }
        };
        NativeValue* lastParam = (info.argc > ARG_COUNT_TWO) ? info.argv[ARG_COUNT_TWO] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("MissioManager::OnGetMissionSnapShot",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, std::move(excute), std::move(complete), &result));
        return result;
    }

    bool CheckMissionSnapShotParams(NativeEngine &engine, NativeCallbackInfo &info,
        std::string &deviceId, int32_t &missionId)
    {
        if (info.argc < ARG_COUNT_TWO) {
            ThrowTooFewParametersError(engine);
            return false;
        }

        if (!ConvertFromJsValue(engine, info.argv[0], deviceId)) {
            HILOG_ERROR("missionSnapshot: Parse deviceId failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return false;
        }

        if (!ConvertFromJsValue(engine, info.argv[1], missionId)) {
            HILOG_ERROR("missionSnapshot: Parse missionId failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return false;
        }

        return true;
    }

    NativeValue* OnLockMission(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc == 0) {
            HILOG_ERROR("OnLockMission Not enough params");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(engine, info.argv[0], missionId)) {
            HILOG_ERROR("OnLockMission Parse missionId failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [missionId](NativeEngine &engine, AsyncTask &task, int32_t status) {
                auto ret = AbilityManagerClient::GetInstance()->LockMissionForCleanup(missionId);
                if (ret == 0) {
                    task.ResolveWithNoError(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine,
                        CreateJsErrorByNativeErr(engine, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        NativeValue* lastParam = (info.argc > 1) ?  info.argv[1] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("MissioManager::OnLockMission",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnUnlockMission(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc == 0) {
            HILOG_ERROR("OnUnlockMission Not enough params");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(engine, info.argv[0], missionId)) {
            HILOG_ERROR("OnUnlockMission Parse missionId failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [missionId](NativeEngine &engine, AsyncTask &task, int32_t status) {
                auto ret = AbilityManagerClient::GetInstance()->UnlockMissionForCleanup(missionId);
                if (ret == 0) {
                    task.ResolveWithNoError(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine,
                        CreateJsErrorByNativeErr(engine, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        NativeValue* lastParam = (info.argc > 1) ? info.argv[1] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("MissioManager::OnUnlockMission",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnClearMission(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc == 0) {
            HILOG_ERROR("OnClearMission Not enough params");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(engine, info.argv[0], missionId)) {
            HILOG_ERROR("OnClearMission Parse missionId failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [missionId](NativeEngine &engine, AsyncTask &task, int32_t status) {
                auto ret = AbilityManagerClient::GetInstance()->CleanMission(missionId);
                if (ret == 0) {
                    task.ResolveWithNoError(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine,
                        CreateJsErrorByNativeErr(engine, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        NativeValue* lastParam = (info.argc > 1) ? info.argv[1] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("MissioManager::OnClearMission",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnClearAllMissions(NativeEngine &engine, const NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        AsyncTask::CompleteCallback complete =
            [](NativeEngine &engine, AsyncTask &task, int32_t status) {
                auto ret = AbilityManagerClient::GetInstance()->CleanAllMissions();
                if (ret == 0) {
                    task.ResolveWithNoError(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine,
                        CreateJsErrorByNativeErr(engine, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        NativeValue* lastParam = (info.argc > 0) ? info.argv[0] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("MissioManager::OnMoveMissionToFront",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnMoveMissionToFront(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc == 0) {
            HILOG_ERROR("OnMoveMissionToFront Not enough params");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(engine, info.argv[0], missionId)) {
            HILOG_ERROR("OnMoveMissionToFront Parse missionId failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
        decltype(info.argc) unwrapArgc = 1;

        AAFwk::StartOptions startOptions;
        if (info.argc > ARGC_ONE && info.argv[1]->TypeOf() == NATIVE_OBJECT) {
            HILOG_INFO("OnMoveMissionToFront start options is used.");
            AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[1]), startOptions);
            unwrapArgc++;
        }
        AsyncTask::CompleteCallback complete =
            [missionId, startOptions, unwrapArgc](NativeEngine &engine, AsyncTask &task, int32_t status) {
                auto ret = (unwrapArgc == 1) ? AbilityManagerClient::GetInstance()->MoveMissionToFront(missionId) :
                    AbilityManagerClient::GetInstance()->MoveMissionToFront(missionId, startOptions);
                if (ret == 0) {
                    task.ResolveWithNoError(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine,
                        CreateJsErrorByNativeErr(engine, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        NativeValue* lastParam = (info.argc > unwrapArgc) ? info.argv[unwrapArgc] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("MissioManager::OnMoveMissionToFront",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

private:
    bool CheckOnOffType(NativeEngine &engine, NativeCallbackInfo &info)
    {
        if (info.argc < ARGC_ONE) {
            return false;
        }

        if (info.argv[0]->TypeOf() != NATIVE_STRING) {
            HILOG_ERROR("CheckOnOffType, Param 0 is not string");
            return false;
        }

        std::string type;
        if (!ConvertFromJsValue(engine, info.argv[0], type)) {
            HILOG_ERROR("CheckOnOffType, Parse on off type failed");
            return false;
        }

        if (type != ON_OFF_TYPE) {
            HILOG_ERROR("CheckOnOffType, args[0] should be mission.");
            return false;
        }
        return true;
    }

    sptr<JsMissionListener> missionListener_ = nullptr;
    uint32_t missionListenerId_ = 0;
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

    const char *moduleName = "JsMissionManager";
    BindNativeFunction(*engine, *object, "on",
        moduleName, JsMissionManager::RegisterMissionListener);
    BindNativeFunction(*engine, *object, "off",
        moduleName, JsMissionManager::UnregisterMissionListener);
    BindNativeFunction(*engine, *object, "getMissionInfos", moduleName, JsMissionManager::GetMissionInfos);
    BindNativeFunction(*engine, *object, "getMissionInfo", moduleName, JsMissionManager::GetMissionInfo);
    BindNativeFunction(*engine, *object, "getMissionSnapShot", moduleName, JsMissionManager::GetMissionSnapShot);
    BindNativeFunction(*engine, *object, "getLowResolutionMissionSnapShot", moduleName,
        JsMissionManager::GetLowResolutionMissionSnapShot);
    BindNativeFunction(*engine, *object, "lockMission", moduleName, JsMissionManager::LockMission);
    BindNativeFunction(*engine, *object, "unlockMission", moduleName, JsMissionManager::UnlockMission);
    BindNativeFunction(*engine, *object, "clearMission", moduleName, JsMissionManager::ClearMission);
    BindNativeFunction(*engine, *object, "clearAllMissions", moduleName, JsMissionManager::ClearAllMissions);
    BindNativeFunction(*engine, *object, "moveMissionToFront", moduleName, JsMissionManager::MoveMissionToFront);
    return engine->CreateUndefined();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
