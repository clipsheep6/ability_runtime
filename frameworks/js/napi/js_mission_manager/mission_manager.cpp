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

#include "mission_manager.h"

#include "ability_manager_client.h"
#include "event_handler.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_error_utils.h"
#include "js_mission_info_utils.h"
#include "js_mission_listener.h"
#include "js_runtime_utils.h"
#include "mission_snapshot.h"
#include "napi_common_start_options.h"
#include "napi_common_util.h"
#include "native_engine/native_value.h"
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
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr size_t ARGC_ONE = 1;
constexpr int32_t ARG_COUNT_TWO = 2;
constexpr const char* ON_OFF_TYPE = "mission";
constexpr const char* ON_OFF_TYPE_SYNC = "missionEvent";
}
class JsMissionManager {
public:
    JsMissionManager() = default;
    ~JsMissionManager() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGD(AAFwkTag::MISSION, "called");
        std::unique_ptr<JsMissionManager>(static_cast<JsMissionManager*>(data));
    }

    static napi_value On(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnOn);
    }

    static napi_value Off(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnOff);
    }

    static napi_value GetMissionInfos(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnGetMissionInfos);
    }

    static napi_value GetMissionInfo(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnGetMissionInfo);
    }

    static napi_value GetMissionSnapShot(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnGetMissionSnapShot);
    }

    static napi_value GetLowResolutionMissionSnapShot(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnGetLowResolutionMissionSnapShot);
    }

    static napi_value LockMission(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnLockMission);
    }

    static napi_value UnlockMission(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnUnlockMission);
    }

    static napi_value ClearMission(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnClearMission);
    }

    static napi_value ClearAllMissions(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnClearAllMissions);
    }

    static napi_value MoveMissionToFront(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnMoveMissionToFront);
    }

    static napi_value MoveMissionsToForeground(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnMoveMissionsToForeground);
    }

    static napi_value MoveMissionsToBackground(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnMoveMissionsToBackground);
    }

private:
    napi_value OnOn(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::MISSION, "called");
        std::string type = ParseParamType(env, argc, argv);
        if (type == ON_OFF_TYPE_SYNC) {
            return OnOnNew(env, argc, argv);
        }
        return OnOnOld(env, argc, argv);
    }

    napi_value OnOnOld(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::MISSION, "called");
        if (argc < ARG_COUNT_TWO) {
            TAG_LOGE(AAFwkTag::MISSION, "Params not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        if (!CheckOnOffType(env, argc, argv)) {
            ThrowInvalidParamError(env, "Parse param type failed, must be a string, value must be mission");
            return CreateJsUndefined(env);
        }

        missionListenerId_++;
        if (missionListener_ != nullptr) {
            missionListener_->AddJsListenerObject(missionListenerId_, argv[ARGC_ONE]);
            return CreateJsValue(env, missionListenerId_);
        }

        missionListener_ = new JsMissionListener(env);
        auto ret = AbilityManagerClient::GetInstance()->RegisterMissionListener(missionListener_);
        if (ret == 0) {
            missionListener_->AddJsListenerObject(missionListenerId_, argv[ARGC_ONE]);
            return CreateJsValue(env, missionListenerId_);
        } else {
            TAG_LOGE(AAFwkTag::MISSION, "RegisterMissionListener failed, ret = %{public}d", ret);
            missionListener_ = nullptr;
            if (ret == CHECK_PERMISSION_FAILED) {
                ThrowNoPermissionError(env, PermissionConstants::PERMISSION_MANAGE_MISSION);
            } else {
                ThrowError(env, GetJsErrorCodeByNativeError(ret));
            }
            return CreateJsUndefined(env);
        }
    }

    napi_value OnOnNew(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::MISSION, "called");
        if (argc < ARG_COUNT_TWO) {
            TAG_LOGE(AAFwkTag::MISSION, "Params not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (!AppExecFwk::IsTypeForNapiValue(env, argv[1], napi_object)) {
            TAG_LOGE(AAFwkTag::MISSION, "Invalid param");
            ThrowInvalidParamError(env, "Parse param listener failed, must be a MissionListener");
            return CreateJsUndefined(env);
        }

        missionListenerId_++;
        if (missionListener_ != nullptr) {
            missionListener_->AddJsListenerObject(missionListenerId_, argv[INDEX_ONE], true);
            return CreateJsValue(env, missionListenerId_);
        }

        missionListener_ = new JsMissionListener(env);
        auto ret = AbilityManagerClient::GetInstance()->RegisterMissionListener(missionListener_);
        if (ret == 0) {
            missionListener_->AddJsListenerObject(missionListenerId_, argv[INDEX_ONE], true);
            return CreateJsValue(env, missionListenerId_);
        } else {
            TAG_LOGE(AAFwkTag::MISSION, "RegisterMissionListener failed, ret = %{public}d", ret);
            missionListener_ = nullptr;
            if (ret == CHECK_PERMISSION_FAILED) {
                ThrowNoPermissionError(env, PermissionConstants::PERMISSION_MANAGE_MISSION);
            } else {
                ThrowError(env, GetJsErrorCodeByNativeError(ret));
            }
            return CreateJsUndefined(env);
        }
    }

    napi_value OnOff(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::MISSION, "called");
        std::string type = ParseParamType(env, argc, argv);
        if (type == ON_OFF_TYPE_SYNC) {
            return OnOffNew(env, argc, argv);
        }
        return OnOffOld(env, argc, argv);
    }

    napi_value OnOffOld(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::MISSION, "called");
        if (argc < ARG_COUNT_TWO) {
            TAG_LOGE(AAFwkTag::MISSION, "Not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        if (!CheckOnOffType(env, argc, argv)) {
            ThrowInvalidParamError(env, "Parse param type failed, must be a string, value must be mission");
            return CreateJsUndefined(env);
        }

        int32_t missionListenerId = -1;
        if (!ConvertFromJsValue(env, argv[ARGC_ONE], missionListenerId)) {
            TAG_LOGE(AAFwkTag::MISSION, "Parse missionListenerId failed");
            ThrowInvalidParamError(env, "Parse param listenerId failed, must be a number");
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [&missionListener = missionListener_, missionListenerId]
            (napi_env env, NapiAsyncTask &task, int32_t status) {
                if (!missionListener || !missionListener->RemoveJsListenerObject(missionListenerId)) {
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_NO_MISSION_LISTENER));
                    return;
                }

                if (!missionListener->IsEmpty()) {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                    return;
                }
                auto ret = AbilityManagerClient::GetInstance()->UnRegisterMissionListener(missionListener);
                if (ret == 0) {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                    missionListener = nullptr;
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc <= ARG_COUNT_TWO) ? nullptr : argv[INDEX_TWO];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("MissioManager::OnUnregisterMissionListener",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnOffNew(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::MISSION, "called");
        if (argc < ARG_COUNT_TWO) {
            TAG_LOGE(AAFwkTag::MISSION, "Not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        int32_t missionListenerId = -1;
        if (!ConvertFromJsValue(env, argv[INDEX_ONE], missionListenerId)) {
            TAG_LOGE(AAFwkTag::MISSION, "Parse missionListenerId failed");
            ThrowInvalidParamError(env, "Parse param listenerId failed, must be a number");
            return CreateJsUndefined(env);
        }

        if (missionListener_ == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "missionListener_ is nullptr");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        if (!missionListener_->RemoveJsListenerObject(missionListenerId, true)) {
            TAG_LOGE(AAFwkTag::MISSION, "missionListenerId not found");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_NO_MISSION_LISTENER);
            return CreateJsUndefined(env);
        }
        if (!missionListener_->IsEmpty()) {
            TAG_LOGD(AAFwkTag::MISSION, "Off success, missionListener is not empty");
            return CreateJsUndefined(env);
        }
        auto ret = AbilityManagerClient::GetInstance()->UnRegisterMissionListener(missionListener_);
        if (ret == 0) {
            TAG_LOGD(AAFwkTag::MISSION, "UnRegisterMissionListener success");
            missionListener_ = nullptr;
        } else {
            TAG_LOGE(AAFwkTag::MISSION, "UnRegisterMissionListener failed");
            if (ret == CHECK_PERMISSION_FAILED) {
                ThrowNoPermissionError(env, PermissionConstants::PERMISSION_MANAGE_MISSION);
            } else {
                ThrowError(env, GetJsErrorCodeByNativeError(ret));
            }
        }
        return CreateJsUndefined(env);
    }

    napi_value OnGetMissionInfos(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "%{public}s is called", __FUNCTION__);
        if (argc < ARG_COUNT_TWO) {
            TAG_LOGE(AAFwkTag::MISSION, "Not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        std::string deviceId;
        if (!ConvertFromJsValue(env, argv[0], deviceId)) {
            TAG_LOGE(AAFwkTag::MISSION, "Parse deviceId failed");
            ThrowInvalidParamError(env, "Parse param deviceId failed, must be a string");
            return CreateJsUndefined(env);
        }
        int numMax = -1;
        if (!ConvertFromJsValue(env, argv[1], numMax)) {
            TAG_LOGE(AAFwkTag::MISSION, "Parse numMax failed");
            ThrowInvalidParamError(env, "Parse param numMax failed, must be a number");
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [deviceId, numMax](napi_env env, NapiAsyncTask &task, int32_t status) {
                std::vector<AAFwk::MissionInfo> missionInfos;
                auto ret = AbilityManagerClient::GetInstance()->GetMissionInfos(deviceId, numMax, missionInfos);
                if (ret == 0) {
                    task.ResolveWithNoError(env, CreateJsMissionInfoArray(env, missionInfos));
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc <= 2) ? nullptr : argv[2];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("MissioManager::OnGetMissionInfos",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnGetMissionInfo(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "%{public}s is called", __FUNCTION__);
        if (argc < ARG_COUNT_TWO) {
            TAG_LOGE(AAFwkTag::MISSION, "Not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        std::string deviceId;
        if (!ConvertFromJsValue(env, argv[0], deviceId)) {
            TAG_LOGE(AAFwkTag::MISSION, "Parse deviceId failed");
            ThrowInvalidParamError(env, "Parse param deviceId failed, must be a string");
            return CreateJsUndefined(env);
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(env, argv[1], missionId)) {
            TAG_LOGE(AAFwkTag::MISSION, "Parse missionId failed");
            ThrowInvalidParamError(env, "Parse param missionId failed, must be a number");
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [deviceId, missionId](napi_env env, NapiAsyncTask &task, int32_t status) {
                AAFwk::MissionInfo missionInfo;
                auto ret = AbilityManagerClient::GetInstance()->GetMissionInfo(deviceId, missionId, missionInfo);
                if (ret == 0) {
                    task.ResolveWithNoError(env, CreateJsMissionInfo(env, missionInfo));
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc <= 2) ? nullptr : argv[2];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("MissioManager::OnGetMissionInfo",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnGetMissionSnapShot(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "called");
        return GetMissionSnapShot(env, argc, argv, false);
    }

    napi_value OnGetLowResolutionMissionSnapShot(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "called");
        return GetMissionSnapShot(env, argc, argv, true);
    }

    napi_value GetMissionSnapShot(napi_env env, size_t argc, napi_value* argv, bool isLowResolution)
    {
        TAG_LOGI(AAFwkTag::MISSION, "%{public}s is called", __FUNCTION__);
        std::string deviceId;
        int32_t missionId = -1;
        if (!CheckMissionSnapShotParams(env, argc, argv, deviceId, missionId)) {
            return CreateJsUndefined(env);
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

        auto complete = [snapshotWrap](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (snapshotWrap->result == 0) {
                napi_value object = nullptr;
                napi_create_object(env, &object);
                napi_value abilityObj = nullptr;
                napi_create_object(env, &abilityObj);
                napi_set_named_property(env, abilityObj, "bundleName",
                    CreateJsValue(env, snapshotWrap->missionSnapshot.topAbility.GetBundleName()));
                napi_set_named_property(env, abilityObj, "abilityName",
                    CreateJsValue(env, snapshotWrap->missionSnapshot.topAbility.GetAbilityName()));
                napi_set_named_property(env, object, "ability", abilityObj);
#ifdef SUPPORT_GRAPHICS
                auto snapshotValue = Media::PixelMapNapi::CreatePixelMap(
                    env, snapshotWrap->missionSnapshot.snapshot);
                napi_set_named_property(env, object, "snapshot", snapshotValue);
#endif
                task.ResolveWithNoError(env, object);
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, snapshotWrap->result,
                    PermissionConstants::PERMISSION_MANAGE_MISSION));
            }
        };
        napi_value lastParam = (argc > ARG_COUNT_TWO) ? argv[ARG_COUNT_TWO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("MissioManager::OnGetMissionSnapShot",
            env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(excute), std::move(complete), &result));
        return result;
    }

    bool CheckMissionSnapShotParams(napi_env env, size_t argc, napi_value* argv,
        std::string &deviceId, int32_t &missionId)
    {
        if (argc < ARG_COUNT_TWO) {
            ThrowTooFewParametersError(env);
            return false;
        }

        if (!ConvertFromJsValue(env, argv[0], deviceId)) {
            TAG_LOGE(AAFwkTag::MISSION, "missionSnapshot: Parse deviceId failed");
            ThrowInvalidParamError(env, "Parse param deviceId failed, must be a string");
            return false;
        }

        if (!ConvertFromJsValue(env, argv[1], missionId)) {
            TAG_LOGE(AAFwkTag::MISSION, "missionSnapshot: Parse missionId failed");
            ThrowInvalidParamError(env, "Parse param missionId failed, must be a number");
            return false;
        }

        return true;
    }

    napi_value OnLockMission(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "%{public}s is called", __FUNCTION__);
        if (argc == 0) {
            TAG_LOGE(AAFwkTag::MISSION, "OnLockMission Not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(env, argv[0], missionId)) {
            TAG_LOGE(AAFwkTag::MISSION, "OnLockMission Parse missionId failed");
            ThrowInvalidParamError(env, "Parse param missionId failed, must be a number");
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [missionId](napi_env env, NapiAsyncTask &task, int32_t status) {
                auto ret = AbilityManagerClient::GetInstance()->LockMissionForCleanup(missionId);
                if (ret == 0) {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc > 1) ?  argv[1] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("MissioManager::OnLockMission",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnUnlockMission(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "%{public}s is called", __FUNCTION__);
        if (argc == 0) {
            TAG_LOGE(AAFwkTag::MISSION, "OnUnlockMission Not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(env, argv[0], missionId)) {
            TAG_LOGE(AAFwkTag::MISSION, "OnUnlockMission Parse missionId failed");
            ThrowInvalidParamError(env, "Parse param missionId failed, must be a number");
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [missionId](napi_env env, NapiAsyncTask &task, int32_t status) {
                auto ret = AbilityManagerClient::GetInstance()->UnlockMissionForCleanup(missionId);
                if (ret == 0) {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc > 1) ? argv[1] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("MissioManager::OnUnlockMission",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnClearMission(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "%{public}s is called", __FUNCTION__);
        if (argc == 0) {
            TAG_LOGE(AAFwkTag::MISSION, "OnClearMission Not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(env, argv[0], missionId)) {
            TAG_LOGE(AAFwkTag::MISSION, "OnClearMission Parse missionId failed");
            ThrowInvalidParamError(env, "Parse param missionId failed, must be a number");
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [missionId](napi_env env, NapiAsyncTask &task, int32_t status) {
                auto ret = AbilityManagerClient::GetInstance()->CleanMission(missionId);
                if (ret == 0) {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc > 1) ? argv[1] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("MissioManager::OnClearMission",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnClearAllMissions(napi_env env, const size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "%{public}s is called", __FUNCTION__);
        NapiAsyncTask::CompleteCallback complete =
            [](napi_env env, NapiAsyncTask &task, int32_t status) {
                auto ret = AbilityManagerClient::GetInstance()->CleanAllMissions();
                if (ret == 0) {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc > 0) ? argv[0] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("MissioManager::OnMoveMissionToFront",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnMoveMissionToFront(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "%{public}s is called", __FUNCTION__);
        if (argc == 0) {
            TAG_LOGE(AAFwkTag::MISSION, "OnMoveMissionToFront Not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(env, argv[0], missionId)) {
            TAG_LOGE(AAFwkTag::MISSION, "OnMoveMissionToFront Parse missionId failed");
            ThrowInvalidParamError(env, "Parse param missionId failed, must be a number");
            return CreateJsUndefined(env);
        }
        decltype(argc) unwrapArgc = 1;

        AAFwk::StartOptions startOptions;
        if (argc > ARGC_ONE && AppExecFwk::IsTypeForNapiValue(env, argv[1], napi_object)) {
            TAG_LOGI(AAFwkTag::MISSION, "OnMoveMissionToFront start options is used.");
            AppExecFwk::UnwrapStartOptions(env, argv[1], startOptions);
            unwrapArgc++;
        }
        NapiAsyncTask::CompleteCallback complete =
            [missionId, startOptions, unwrapArgc](napi_env env, NapiAsyncTask &task, int32_t status) {
                auto ret = (unwrapArgc == 1) ? AbilityManagerClient::GetInstance()->MoveMissionToFront(missionId) :
                    AbilityManagerClient::GetInstance()->MoveMissionToFront(missionId, startOptions);
                if (ret == 0) {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc > unwrapArgc) ? argv[unwrapArgc] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("MissioManager::OnMoveMissionToFront",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnMoveMissionsToForeground(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "%{public}s is called", __FUNCTION__);
        std::vector<int32_t> missionIds;
        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::MISSION, "OnMoveMissionsToForeground Not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        uint32_t nativeArrayLen = 0;
        napi_get_array_length(env, argv[0], &nativeArrayLen);
        if (nativeArrayLen == 0) {
            TAG_LOGE(AAFwkTag::MISSION, "OnMoveMissionsToForeground MissionId is null");
            ThrowInvalidParamError(env, "Parse param missionIds failed, the size of missionIds must above zero");
            return CreateJsUndefined(env);
        }
        napi_value element = nullptr;
        for (uint32_t i = 0; i < nativeArrayLen; i++) {
            int32_t missionId = 0;
            napi_get_element(env, argv[0], i, &element);
            if (!ConvertFromJsValue(env, element, missionId)) {
                TAG_LOGE(AAFwkTag::MISSION, "OnMoveMissionsToForeground Parse missionId failed");
                ThrowInvalidParamError(env, "Parse param missionIds failed, missionId must be a number");
                return CreateJsUndefined(env);
            }
            missionIds.push_back(missionId);
        }

        int topMissionId = -1;
        decltype(argc) unwrapArgc = 1;
        if (argc > ARGC_ONE && AppExecFwk::IsTypeForNapiValue(env, argv[1], napi_number)) {
            if (!ConvertFromJsValue(env, argv[1], topMissionId)) {
                TAG_LOGE(AAFwkTag::MISSION, "OnMoveMissionsToForeground Parse topMissionId failed");
                ThrowInvalidParamError(env, "Parse param topMission failed, must be a number");
                return CreateJsUndefined(env);
            }
            unwrapArgc++;
        }

        NapiAsyncTask::CompleteCallback complete =
            [missionIds, topMissionId](napi_env env, NapiAsyncTask &task, int32_t status) {
                auto ret =
                    AAFwk::AbilityManagerClient::GetInstance()->MoveMissionsToForeground(missionIds, topMissionId);
                if (ret == 0) {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc > unwrapArgc) ? argv[unwrapArgc] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("MissioManager::OnMoveMissionsToForeground", env,
            CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnMoveMissionsToBackground(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "%{public}s is called", __FUNCTION__);
        std::vector<int32_t> missionIds;

        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::MISSION, "OnMoveMissionsToBackground Not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        uint32_t nativeArrayLen = 0;
        napi_get_array_length(env, argv[0], &nativeArrayLen);
        if (nativeArrayLen == 0) {
            TAG_LOGE(AAFwkTag::MISSION, "OnMoveMissionsToBackground MissionId is null");
            ThrowInvalidParamError(env, "Parse param missionIds failed, the size of missionIds must above zero");
            return CreateJsUndefined(env);
        }
        napi_value element = nullptr;
        for (uint32_t i = 0; i < nativeArrayLen; i++) {
            int32_t missionId;
            napi_get_element(env, argv[0], i, &element);
            if (!ConvertFromJsValue(env, element, missionId)) {
                TAG_LOGE(AAFwkTag::MISSION, "OnMoveMissionsToBackground Parse topMissionId failed");
                ThrowInvalidParamError(env, "Parse param missionIds failed, missionId must be a number");
                return CreateJsUndefined(env);
            }
            missionIds.push_back(missionId);
        }

        NapiAsyncTask::CompleteCallback complete =
            [missionIds](napi_env env, NapiAsyncTask &task, int32_t status) {
                std::vector<int32_t> resultMissionIds;
                auto ret  = AbilityManagerClient::GetInstance()->MoveMissionsToBackground(missionIds, resultMissionIds);
                if (ret == 0) {
                    napi_value arrayValue = nullptr;
                    napi_create_array_with_length(env, resultMissionIds.size(), &arrayValue);
                    uint32_t index = 0;
                    for (const auto &missionId : resultMissionIds) {
                        napi_set_element(env, arrayValue, index++, CreateJsValue(env, missionId));
                    }
                    task.ResolveWithNoError(env, arrayValue);
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc <= 1) ? nullptr : argv[1];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("MissioManager::OnMoveMissionsToBackground",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

private:
    bool CheckOnOffType(napi_env env, size_t argc, napi_value* argv)
    {
        if (argc < ARGC_ONE) {
            return false;
        }

        if (!AppExecFwk::IsTypeForNapiValue(env, argv[0], napi_string)) {
            TAG_LOGE(AAFwkTag::MISSION, "CheckOnOffType, Param 0 is not string");
            return false;
        }

        std::string type;
        if (!ConvertFromJsValue(env, argv[0], type)) {
            TAG_LOGE(AAFwkTag::MISSION, "CheckOnOffType, Parse on off type failed");
            return false;
        }

        if (type != ON_OFF_TYPE) {
            TAG_LOGE(AAFwkTag::MISSION, "CheckOnOffType, args[0] should be mission.");
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

    sptr<JsMissionListener> missionListener_ = nullptr;
    uint32_t missionListenerId_ = 0;
};

napi_value JsMissionManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::MISSION, "called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGI(AAFwkTag::MISSION, "Invalid input parameters");
        return nullptr;
    }

    std::unique_ptr<JsMissionManager> jsMissionManager = std::make_unique<JsMissionManager>();
    napi_wrap(env, exportObj, jsMissionManager.release(), JsMissionManager::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsMissionManager";
    BindNativeFunction(env, exportObj, "on", moduleName, JsMissionManager::On);
    BindNativeFunction(env, exportObj, "off", moduleName, JsMissionManager::Off);
    BindNativeFunction(env, exportObj, "getMissionInfos", moduleName, JsMissionManager::GetMissionInfos);
    BindNativeFunction(env, exportObj, "getMissionInfo", moduleName, JsMissionManager::GetMissionInfo);
    BindNativeFunction(env, exportObj, "getMissionSnapShot", moduleName, JsMissionManager::GetMissionSnapShot);
    BindNativeFunction(env, exportObj, "getLowResolutionMissionSnapShot", moduleName,
        JsMissionManager::GetLowResolutionMissionSnapShot);
    BindNativeFunction(env, exportObj, "lockMission", moduleName, JsMissionManager::LockMission);
    BindNativeFunction(env, exportObj, "unlockMission", moduleName, JsMissionManager::UnlockMission);
    BindNativeFunction(env, exportObj, "clearMission", moduleName, JsMissionManager::ClearMission);
    BindNativeFunction(env, exportObj, "clearAllMissions", moduleName, JsMissionManager::ClearAllMissions);
    BindNativeFunction(env, exportObj, "moveMissionToFront", moduleName, JsMissionManager::MoveMissionToFront);
    BindNativeFunction(env, exportObj,
        "moveMissionsToForeground", moduleName, JsMissionManager::MoveMissionsToForeground);
    BindNativeFunction(env, exportObj,
        "moveMissionsToBackground", moduleName, JsMissionManager::MoveMissionsToBackground);
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
