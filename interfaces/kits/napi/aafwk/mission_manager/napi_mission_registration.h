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

#ifndef OHOS_DISTRIBUTED_MISSION_MANAGER_H
#define OHOS_DISTRIBUTED_MISSION_MANAGER_H

#include <uv.h>

#include "distributed_sched_interface.h"
#include "mission_callback_interface.h"
#include "mission_callback_stub.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "parcel_helper.h"
#include "securec.h"
#include "want.h"

namespace OHOS {
namespace DistributedSchedule {
napi_value NAPI_StartSyncRemoteMissions(napi_env env, napi_callback_info info);
napi_value NAPI_StopSyncRemoteMissions(napi_env env, napi_callback_info info);
napi_value NAPI_RegisterMissionListener(napi_env env, napi_callback_info info);
napi_value NAPI_UnRegisterMissionListener(napi_env env, napi_callback_info info);
napi_value NAPI_ContinueAbility(napi_env env, napi_callback_info info);
napi_value WrapString(napi_env env, const std::string& deviceId, const std::string& paramName);
napi_value WrapInt32(napi_env env, int32_t num, const std::string& paramName);

class NAPIMissionRegistration : public MissionCallbackStub {
public:
    void NotifyMissionsChanged(const std::u16string deviceId) override;
    void NotifySnapshot(const std::u16string deviceId, int32_t missionId) override;
    void NotifyNetDisconnect(const std::u16string deviceId, int32_t state) override;
    void OnContinueDone(int32_t result) override;
    void SetContinueEnv(const napi_env &env);
    void SetEnv(const napi_env &env);
    void SetNotifyMissionsChangedCBRef(const napi_ref &ref);
    void SetNotifySnapshotCBRef(const napi_ref &ref);
    void SetNotifyNetDisconnectCBRef(const napi_ref &ref);
    void SetContinueAbilityEnv(const napi_env &env);
    void SetContinueAbilityCBRef(const napi_ref &ref);

private:
    napi_env env_ = nullptr;
    napi_ref notifyMissionsChangedRef_ = nullptr;
    napi_ref notifySnapshotRef_ = nullptr;
    napi_ref notifyNetDisconnectRef_ = nullptr;
    napi_ref onContinueDoneRef_ = nullptr;
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

struct RegisterMissonCB {
    CBBase cbBase;
    std::u16string deviceId;
    sptr<NAPIMissionRegistration> missionRegistration;
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
    sptr<NAPIMissionRegistration> abilityContinuation;
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

    char16_t deviceId[128] = {0};
    size_t valueLen = 0;
    bool fixConflict = false;
    int64_t tag = -1;
    int result = 0;

    napi_deferred deferred;
    napi_ref callbackRef;
};

bool SetSyncRemoteMissionsContext(
    const napi_env &env, const napi_value &value, SyncRemoteMissionsContext* context);
bool ProcessSyncInput(napi_env env, napi_callback_info info, bool isStart,
    SyncRemoteMissionsContext* syncContext);
std::mutex registrationLock_;
static std::map<std::u16string, sptr<NAPIMissionRegistration>> registration_;

enum ErrorCode {
    NO_ERROR = 0,
    INVALID_PARAMETER = -1,
    MISSION_NOT_FOUND = -2,
    PERMISSION_DENY = -3,
    REGISTRATION_NOT_FOUND = -4,
};
}  // namespace DistributedSchedule
}  // namespace OHOS
#endif  // OHOS_DISTRIBUTED_MISSION_MANAGER_H