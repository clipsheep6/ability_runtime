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

#ifndef OHOS_ABILITY_RUNTIME_DISTRIBUTED_MISSION_MANAGER_H
#define OHOS_ABILITY_RUNTIME_DISTRIBUTED_MISSION_MANAGER_H

#include <uv.h>

#include "mission_continue_interface.h"
#include "mission_continue_stub.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"
#include "want.h"
#include "remote_mission_listener_stub.h"

namespace OHOS {
namespace AAFwk {
using namespace std;
napi_value NAPI_StartSyncRemoteMissions(napi_env env, napi_callback_info info);
napi_value NAPI_StopSyncRemoteMissions(napi_env env, napi_callback_info info);
napi_value NAPI_RegisterMissionListener(napi_env env, napi_callback_info info);
napi_value NAPI_UnRegisterMissionListener(napi_env env, napi_callback_info info);
napi_value NAPI_ContinueAbility(napi_env env, napi_callback_info info);
napi_value WrapString(napi_env &env, const std::string &deviceId, const std::string &paramName);
napi_value WrapInt32(napi_env &env, int32_t num, const std::string &paramName);

class NAPIMissionContinue : public MissionContinueStub {
public:
    void OnContinueDone(int32_t result) override;
    void SetContinueEnv(const napi_env &env);
    void SetEnv(const napi_env &env);
    void SetContinueAbilityEnv(const napi_env &env);
    void SetContinueAbilityCBRef(const napi_ref &ref);

private:
    napi_env env_ = nullptr;
    napi_ref onContinueDoneRef_ = nullptr;
};

class NAPIRemoteMissionListener : public AAFwk::RemoteMissionListenerStub {
public:
    virtual ~NAPIRemoteMissionListener();

    void NotifyMissionsChanged(const std::string &deviceId) override;
    void NotifySnapshot(const std::string &deviceId, int32_t missionId) override;
    void NotifyNetDisconnect(const std::string &deviceId, int32_t state) override;
    void SetEnv(const napi_env &env);
    void SetNotifyMissionsChangedCBRef(const napi_ref &ref);
    void SetNotifySnapshotCBRef(const napi_ref &ref);
    void SetNotifyNetDisconnectCBRef(const napi_ref &ref);

private:
    napi_env env_ = nullptr;
    napi_ref notifyMissionsChangedRef_ = nullptr;
    napi_ref notifySnapshotRef_ = nullptr;
    napi_ref notifyNetDisconnectRef_ = nullptr;
};

struct CallbackInfo {
    napi_env env;
    napi_ref callback;
    napi_deferred deferred;
};

struct CBBase {
    CallbackInfo cbInfo;
    napi_async_work asyncWork;
    napi_deferred deferred;
    int errCode = 0;
};

struct MissionRegistrationCB {
    napi_env env = nullptr;
    napi_ref callback[3] = {0};
    int resultCode = 0;
};

struct RegisterMissionCB {
    CBBase cbBase;
    std::string deviceId;
    sptr<NAPIRemoteMissionListener> missionRegistration;
    MissionRegistrationCB missionRegistrationCB;
    int result = 0;
    int missionId = 0;
    int state = 0;
    napi_ref callbackRef;
};

struct AbilityContinuationCB {
    napi_env env;
    napi_ref callback[1] = {0};
};

struct ContinueAbilityCB {
    CBBase cbBase;
    std::string dstDeviceId;
    std::string srcDeviceId;
    sptr<NAPIMissionContinue> abilityContinuation;
    AbilityContinuationCB abilityContinuationCB;
    AAFwk::WantParams wantParams;
    ErrCode result = 0;
    int resultCode = 0;
    int missionId = 0;
    napi_ref callbackRef;
};

struct SyncRemoteMissionsContext {
    napi_env env;
    napi_async_work work;

    std::string deviceId;
    size_t valueLen = 0;
    bool fixConflict = false;
    int64_t tag = -1;
    int result = 0;

    napi_deferred deferred;
    napi_ref callbackRef;
};

bool SetSyncRemoteMissionsContext(const napi_env &env, const napi_value &value,
    SyncRemoteMissionsContext* context, std::string &errInfo);
bool ProcessSyncInput(napi_env &env, napi_callback_info info, bool isStart,
    SyncRemoteMissionsContext* syncContext, std::string &errInfo);
void ReturnValueToApplication(napi_env &env, napi_value *result, RegisterMissionCB *registerMissionCB);
void CallbackReturn(napi_value *result, RegisterMissionCB *registerMissionCB);
napi_value GetUndefined();
mutex registrationLock_;
map<std::string, sptr<NAPIRemoteMissionListener>> registration_;

enum ErrorCode {
    NO_ERROR = 0,
    INVALID_PARAMETER = -1,
    REMOTE_MISSION_NOT_FOUND = -2,
    PERMISSION_DENY = -3,
    REGISTRATION_NOT_FOUND = -4,
    /**
     * Result(201) for permission denied.
     */
    PERMISSION_DENIED = 201,
    /**
     * Result(401) for parameter check failed.
     */
    PARAMETER_CHECK_FAILED = 401,
    /**
     * Result(16300501) for the system ability work abnormally.
     */
    SYSTEM_WORK_ABNORMALLY = 16300501,
    /**
     * Result(16300502) for failed to get the missionInfo of the specified missionId.
     */
    NO_MISSION_INFO_FOR_MISSION_ID = 16300502,
    /**
     * Result(16300503) for the application is not installed on the remote end and installation-free is
     * not supported.
     */
    REMOTE_UNINSTALLED_AND_UNSUPPORT_FREEINSTALL_FOR_CONTINUE = 16300503,
    /**
     * Result(16300504) for the application is not installed on the remote end but installation-free is
     * supported, try again with freeInstall flag.
     */
    CONTINUE_WITHOUT_FREEINSTALL_FLAG = 16300504,
    /**
     * Result(16300505) for the operation device must be the device where the application to be continued
     * is located or the target device to be continued.
     */
    OPERATION_DEVICE_NOT_INITIATOR_OR_TARGET = 16300505,
    /**
     * Result(16300506) for the local continuation task is already in progress.
     */
    CONTINUE_ALREADY_IN_PROGRESS = 16300506,
    /**
     * Result(29360144) for get local deviceid fail.
     */
    GET_LOCAL_DEVICE_ERR = 29360144,
    /**
     * Result(29360174) for get remote dms fail.
     */
    GET_REMOTE_DMS_FAIL = 29360174,
    /*
     * Result(29360202) for continue remote not install and support free install.
     */
    CONTINUE_REMOTE_UNINSTALLED_SUPPORT_FREEINSTALL = 29360202,
    /*
     * Result(29360203) for continue remote not install and not support free install.
     */
    CONTINUE_REMOTE_UNINSTALLED_UNSUPPORT_FREEINSTALL = 29360203,
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DISTRIBUTED_MISSION_MANAGER_H
