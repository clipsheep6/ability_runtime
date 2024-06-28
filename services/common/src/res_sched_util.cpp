/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "res_sched_util.h"

#include <string>

#include "ability_info.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
#include "res_sched_client.h"
#include "res_type.h"
#endif

namespace OHOS {
namespace AAFwk {
using AssociatedStartType = ResourceSchedule::ResType::AssociatedStartType;
ResSchedUtil &ResSchedUtil::GetInstance()
{
    static ResSchedUtil instance;
    return instance;
}

int64_t ResSchedUtil::convertType(int64_t resSchedType)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    if (resSchedType == RES_TYPE_SCB_START_ABILITY) {
        return static_cast<int64_t>(AssociatedStartType::SCB_START_ABILITY);
    } else if (resSchedType == RES_TYPE_EXTENSION_START_ABILITY) {
        return static_cast<int64_t>(AssociatedStartType::EXTENSION_START_ABILITY);
    } else if (resSchedType == RES_TYPE_MISSION_LIST_START_ABILITY) {
        return static_cast<int64_t>(AssociatedStartType::MISSION_LIST_START_ABILITY);
    }
#endif
    TAG_LOGE(AAFwkTag::DEFAULT, "res sched type invalid");
    return -1;
}

void ResSchedUtil::ReportAbilitStartInfoToRSS(const AbilityInfo &abilityInfo, int32_t pid, bool isColdStart)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    uint32_t resType = ResourceSchedule::ResType::RES_TYPE_APP_ABILITY_START;
    std::unordered_map<std::string, std::string> eventParams {
        { "name", "ability_start" },
        { "uid", std::to_string(abilityInfo.applicationInfo.uid) },
        { "bundleName", abilityInfo.applicationInfo.bundleName },
        { "abilityName", abilityInfo.name },
        { "pid", std::to_string(pid) }
    };
    TAG_LOGD(AAFwkTag::DEFAULT, "call");
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(resType, isColdStart ? 1 : 0, eventParams);
#endif
}

void ResSchedUtil::ReportAbilitAssociatedStartInfoToRSS(
    const AbilityInfo &abilityInfo, int64_t resSchedType, int32_t callerUid, int32_t callerPid)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    uint32_t resType = ResourceSchedule::ResType::RES_TYPE_APP_ASSOCIATED_START;
    std::unordered_map<std::string, std::string> eventParams {
        { "name", "associated_start" },
        { "caller_uid", std::to_string(callerUid) },
        { "caller_pid", std::to_string(callerPid) },
        { "callee_uid", std::to_string(abilityInfo.applicationInfo.uid) },
        { "callee_bundle_name", abilityInfo.applicationInfo.bundleName }
    };
    int64_t type = convertType(resSchedType);
    TAG_LOGD(AAFwkTag::DEFAULT, "call");
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(resType, type, eventParams);
#endif
}

void ResSchedUtil::ReportEventToRSS(const int32_t uid, const std::string &bundleName, const std::string &reason,
    const int32_t callerPid)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    uint32_t resType = ResourceSchedule::ResType::SYNC_RES_TYPE_THAW_ONE_APP;
    nlohmann::json payload;
    payload.emplace("uid", uid);
    payload.emplace("pid", -1);
    payload.emplace("bundleName", bundleName);
    payload.emplace("reason", reason);
    payload.emplace("callerPid", callerPid);
    nlohmann::json reply;
    TAG_LOGD(AAFwkTag::DEFAULT, "call");
    ResourceSchedule::ResSchedClient::GetInstance().ReportSyncEvent(resType, 0, payload, reply);
#endif
}

void ResSchedUtil::GetAllFrozenPidsFromRSS(std::unordered_set<int32_t> &frozenPids)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    uint32_t resType = ResourceSchedule::ResType::SYNC_RES_TYPE_GET_ALL_SUSPEND_STATE;
    nlohmann::json payload;
    nlohmann::json reply;
    TAG_LOGD(AAFwkTag::DEFAULT, "call");
    int32_t ret = ResourceSchedule::ResSchedClient::GetInstance().ReportSyncEvent(resType, 0, payload, reply);
    if (ret != 0 || !reply.contains("allSuspendState") || !reply["allSuspendState"].is_array()) {
        TAG_LOGE(AAFwkTag::DEFAULT, "ReportSyncEvent fail");
        return;
    }

    for (nlohmann::json &appObj : reply["allSuspendState"]) {
        // Here can get uid if needed
        if (!appObj.contains("pidsState") || !appObj["pidsState"].is_array()) {
            continue;
        }

        for (nlohmann::json &pidObj : appObj["pidsState"]) {
            if (!pidObj.contains("pid") || !pidObj["pid"].is_number() ||
                !pidObj.contains("isFrozen") || !pidObj["isFrozen"].is_boolean()) {
                break;
            }
            int32_t pid = pidObj["pid"].get<int32_t>();
            bool isFrozen = pidObj["isFrozen"].get<bool>();
            if (isFrozen) {
                frozenPids.insert(pid);
            }
        }
    }
    if (frozenPids.empty()) {
        TAG_LOGW(AAFwkTag::DEFAULT, "Get frozen pids empty");
    }
#endif
}
} // namespace AAFwk
} // namespace OHOS