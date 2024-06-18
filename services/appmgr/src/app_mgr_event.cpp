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

#include "app_mgr_event.h"

#include "accesstoken_kit.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AppExecFwk {

int32_t AppMgrEventUtil::GetCallerPid(const std::shared_ptr<AppRunningRecord> &callerAppRecord)
{
    if (!callerAppRecord) {
        TAG_LOGW(AAFwkTag::APPMGR, "callerAppRecord is null");
        return IPCSkeleton::GetCallingPid();
    }

    if (!callerAppRecord->GetPriorityObject()) {
        TAG_LOGW(AAFwkTag::APPMGR, "callerAppRecord's priorityObject is null");
        return IPCSkeleton::GetCallingPid();
    }
    
    return callerAppRecord->GetPriorityObject()->GetPid();
}

void AppMgrEventUtil::UpdateStartupType(const std::shared_ptr<AbilityInfo> &abilityInfo, int32_t &abilityType,
    int32_t &extensionType)
{
    if (abilityInfo == nullptr) {
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s, abilityName:%{public}s", abilityInfo->bundleName.c_str(),
        abilityInfo->name.c_str());
    abilityType = static_cast<int32_t>(abilityInfo->type);
    if (abilityInfo->type != AbilityType::EXTENSION) {
        return;
    }
    extensionType = static_cast<int32_t>(abilityInfo->extensionAbilityType);
}

bool AppMgrEventUtil::SendCreateAtomicServiceProcessEvent(const std::shared_ptr<AppRunningRecord> &callerAppRecord,
    const std::shared_ptr<AppRunningRecord> &appRecord, const std::string &moduleName, const std::string &abilityName)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr");
        return false;
    }
    AAFwk::EventInfo eventInfo;
    eventInfo.abilityName = abilityName;
    eventInfo.moduleName = moduleName;
    eventInfo.bundleName = appRecord->GetBundleName();

    eventInfo.callerUid = appRecord->GetCallerUid() == -1 ? IPCSkeleton::GetCallingUid() : appRecord->GetCallerUid();
    if (callerAppRecord == nullptr) {
        Security::AccessToken::NativeTokenInfo nativeTokenInfo = {};
        auto token = appRecord->GetCallerTokenId() == -1 ?
            static_cast<int>(IPCSkeleton::GetCallingTokenID()) : appRecord->GetCallerTokenId();
        Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(token, nativeTokenInfo);
        eventInfo.callerBundleName = "";
        eventInfo.callerProcessName = nativeTokenInfo.processName;
    } else {
        eventInfo.callerBundleName = callerAppRecord->GetName();
        eventInfo.callerProcessName = callerAppRecord->GetProcessName();
    }
    auto eventName = AAFwk::EventName::CREATE_ATOMIC_SERVICE_PROCESS;
    AAFwk::EventReport::SendAtomicServiceEvent(eventName, HiSysEventType::BEHAVIOR, eventInfo);
    return true;
}

bool AppMgrEventUtil::SendProcessStartEvent(const std::shared_ptr<AppRunningRecord> &callerAppRecord,
    const std::shared_ptr<AppRunningRecord> &appRecord, AAFwk::EventInfo &eventInfo)
{
    if (!appRecord) {
        TAG_LOGW(AAFwkTag::APPMGR, "appRecord is null");
        return false;
    }
    time_t currentTime = time(nullptr);
    eventInfo.time = currentTime;
    eventInfo.callerUid = appRecord->GetCallerUid() == -1 ? IPCSkeleton::GetCallingUid() : appRecord->GetCallerUid();
    if (!appRecord->GetAbilities().empty()) {
        auto abilityinfo = appRecord->GetAbilities().begin()->second->GetAbilityInfo();
        UpdateStartupType(abilityinfo, eventInfo.abilityType, eventInfo.extensionType);
    } else {
        TAG_LOGI(AAFwkTag::APPMGR, "Abilities nullptr!");
    }
    if (!callerAppRecord) {
        Security::AccessToken::NativeTokenInfo nativeTokenInfo = {};
        auto token = appRecord->GetCallerTokenId() == -1 ?
            static_cast<int>(IPCSkeleton::GetCallingTokenID()) : appRecord->GetCallerTokenId();
        Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(token, nativeTokenInfo);
        eventInfo.callerBundleName = "";
        eventInfo.callerProcessName = nativeTokenInfo.processName;
        eventInfo.callerPid = IPCSkeleton::GetCallingPid();
    } else {
        if (callerAppRecord->GetBundleName().empty()) {
            eventInfo.callerBundleName = callerAppRecord->GetName();
        } else {
            eventInfo.callerBundleName = callerAppRecord->GetBundleName();
        }
        eventInfo.callerProcessName = callerAppRecord->GetProcessName();
        eventInfo.callerPid = GetCallerPid(callerAppRecord);
    }
    if (!appRecord->GetBundleName().empty()) {
        eventInfo.bundleName = appRecord->GetBundleName();
    }
    eventInfo.processName = appRecord->GetProcessName();
    if (!appRecord->GetPriorityObject()) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord's priorityObject is null");
    } else {
        eventInfo.pid = appRecord->GetPriorityObject()->GetPid();
    }
    AAFwk::EventReport::SendProcessStartEvent(AAFwk::EventName::PROCESS_START, eventInfo);
    return true;
}

void AppMgrEventUtil::SendReStartProcessEvent(AAFwk::EventInfo &eventInfo, int32_t appUid, int64_t restartTime)
{
    // eventInfo come from SendProcessStartEvent
    eventInfo.time = restartTime;
    eventInfo.appUid = appUid;
    AAFwk::EventReport::SendKeyEvent(AAFwk::EventName::RESTART_PROCESS_BY_SAME_APP,
        HiSysEventType::BEHAVIOR, eventInfo);
}
}  // namespace AppExecFwk
}  // namespace OHOS
