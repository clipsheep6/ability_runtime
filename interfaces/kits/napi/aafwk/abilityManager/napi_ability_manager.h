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

#ifndef NAPI_ABILITY_MANAGER_H
#define NAPI_ABILITY_MANAGER_H

#include <vector>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#ifdef SUPPORT_GRAPHICS
#include "pixel_map_napi.h"
#endif
#include "ability_manager_client.h"
#include "running_process_info.h"
#include "system_memory_attr.h"
#include "mission_snapshot.h"

using RunningProcessInfo = OHOS::AppExecFwk::RunningProcessInfo;
using MissionSnapshot = OHOS::AAFwk::MissionSnapshot;

namespace OHOS {
namespace AppExecFwk {
namespace NapiAbilityMgr {
const int BUFFER_LENGTH_MAX = 128;
const int DEFAULT_STACK_ID = 1;
const int DEFAULT_LAST_MEMORY_LEVEL = -1;
const int DEFAULT_WEIGHT = -1;
const int MAX_MISSION_NUM = 65535;
const int QUERY_RECENT_RUNNING_MISSION_INFO_TYPE = 2;
const int BUSINESS_ERROR_CODE_OK = 0;
}  // namespace NapiAbilityMgr
const uint8_t NUMBER_OF_PARAMETERS_TWO = 2;
const uint8_t NUMBER_OF_PARAMETERS_THREE = 3;

struct AsyncBase {
    explicit AsyncBase(napi_env env);
    virtual ~AsyncBase();
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;
};

struct AsyncCallbackInfo : public AsyncBase {
    explicit AsyncCallbackInfo(napi_env env): AsyncBase(env) {};
    std::vector<RunningProcessInfo> info;
};

struct AsyncClearMissionsCallbackInfo : public AsyncBase {
    explicit AsyncClearMissionsCallbackInfo(napi_env env): AsyncBase(env) {};
    int32_t result = -1;
};

struct AsyncKillProcessCallbackInfo : public AsyncBase {
    explicit AsyncKillProcessCallbackInfo(napi_env env): AsyncBase(env) {};
    std::string bundleName;
    int32_t result = -1;
};

struct AsyncClearUpApplicationDataCallbackInfo : public AsyncBase {
    explicit AsyncClearUpApplicationDataCallbackInfo(napi_env env): AsyncBase(env) {};
    std::string bundleName;
    int32_t result = -1;
};

struct CallbackInfo : public AsyncBase {
    explicit CallbackInfo(napi_env env): AsyncBase(env) {};
    int result = -1;
    bool isRamConstrainedDevice = false;
};

napi_value NAPI_GetAllRunningProcesses(napi_env env, napi_callback_info info);
napi_value NAPI_GetActiveProcessInfos(napi_env env, napi_callback_info info);
napi_value NAPI_KillProcessesByBundleName(napi_env env, napi_callback_info info);
napi_value NAPI_ClearUpApplicationData(napi_env env, napi_callback_info info);
void CreateWeightReasonCodeObject(napi_env env, napi_value value);
napi_value GetCallbackErrorValue(napi_env env, int errCode);
napi_value NapiGetNull(napi_env env);
napi_value NAPI_GetSystemMemoryAttr(napi_env env, napi_callback_info);
napi_value NAPI_GetAppMemorySize(napi_env env, napi_callback_info info);
napi_value NAPI_IsRamConstrainedDevice(napi_env env, napi_callback_info info);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  //  NAPI_ABILITY_MANAGER_H
