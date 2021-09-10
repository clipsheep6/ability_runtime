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

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "napi_ability_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {

EXTERN_C_START
/*
 * function for module exports
 */
static napi_value Init(napi_env env, napi_value exports)
{
    HILOG_INFO("napi_moudule Init start...");

    napi_value nWeightReasonCode = nullptr;
    NAPI_CALL(env, napi_create_object(env, &nWeightReasonCode));
    CreateWeightReasonCodeObject(env, nWeightReasonCode);

    napi_property_descriptor desc[] = {DECLARE_NAPI_FUNCTION("getAllRunningProcesses", NAPI_GetAllRunningProcesses),
        DECLARE_NAPI_FUNCTION("getActiveProcessInfos", NAPI_GetAllRunningProcesses),
        DECLARE_NAPI_FUNCTION("queryRunningAbilityMissionInfos", NAPI_QueryRunningAbilityMissionInfos),
        DECLARE_NAPI_FUNCTION("getActiveAbilityMissionInfos", NAPI_QueryRunningAbilityMissionInfos),
        DECLARE_NAPI_FUNCTION("queryRecentAbilityMissionInfos", NAPI_QueryRecentAbilityMissionInfos),
        DECLARE_NAPI_FUNCTION("getPreviousAbilityMissionInfos", NAPI_GetPreviousAbilityMissionInfos),
        DECLARE_NAPI_FUNCTION("removeMission", NAPI_RemoveMission),
        DECLARE_NAPI_FUNCTION("removeMissions", NAPI_RemoveMissions),
        DECLARE_NAPI_FUNCTION("deleteMissions", NAPI_RemoveMissions),
        DECLARE_NAPI_FUNCTION("clearMissions", NAPI_ClearMissions),
        DECLARE_NAPI_FUNCTION("moveMissionToTop", NAPI_MoveMissionToTop),
        DECLARE_NAPI_FUNCTION("killProcessesByBundleName", NAPI_KillProcessesByBundleName),
        DECLARE_NAPI_FUNCTION("clearUpApplicationData", NAPI_ClearUpApplicationData),
        DECLARE_NAPI_PROPERTY("WeightReasonCode", nWeightReasonCode)};

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    HILOG_INFO("napi_moudule Init end...");
    return exports;
}
EXTERN_C_END

/*
 * Module define
 */
static napi_module _module = {.nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "app.abilitymanager",
    .nm_priv = ((void *)0),
    .reserved = {0}};

/*
 * Module register function
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}

}  // namespace AppExecFwk
}  // namespace OHOS