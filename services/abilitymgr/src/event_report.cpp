/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#include "event_report.h"
#include "hisysevent.h"
#include "hilog_wrapper.h"

#include <chrono>

namespace OHOS {
namespace AAFWK {
namespace {
const std::string EVENT_KEY_PID = "PID";
const std::string EVENT_KEY_UID = "UID";
const std::string EVENT_KEY_RID = "RID";
const std::string EVENT_KEY_ABILITY_NAME = "ABILITY_NAME";
const std::string EVENT_KEY_ABILITY_TYPE = "ABILITY_TYPE";
const std::string EVENT_KEY_MODEL_TYPE = "MODEL_TYPE";
const std::string EVENT_KEY_BUNDLE_NAME = "BUNDLE_NAME";
const std::string EVENT_KEY_DEVICEID = "DEVICEID";
const std::string EVENT_KEY_URI = "URI";
const std::string EVENT_KEY_APP_NAME = "NAME";
const std::string EVENT_KEY_VERSION_NAME = "VERSION_NAME";
const std::string EVENT_KEY_VERSION_CODE = "VERSION_CODE";
const std::string EVENT_KEY_PROCESS = "PROCESS";
const std::string EVENT_KEY_TIME_STAMP = "TIME_STAMP";
const std::string TYPE = "TYPE";
}
void EventReport::AbilityEntranceEvent(int32_t &pid, int32_t &uid, int32_t &rid,
    const std::string &eventName, HiSysEventType type, AppExecFwk::AbilityInfo &abilityInfo)
{
    std::string abilityName = abilityInfo.name.c_str();
    std::string bundleName = abilityInfo.bundleName.c_str();
    std::string abilityType;
    std::string modelType;
    EventReport::GetAbilityType(abilityType, modelType, abilityInfo);
    EventReport::EventWrite(
        eventName,
        type,
        EVENT_KEY_RID, std::to_string(rid),
        EVENT_KEY_UID, std::to_string(uid),
        EVENT_KEY_PID, std::to_string(pid),
        EVENT_KEY_ABILITY_NAME, abilityName,
        EVENT_KEY_ABILITY_TYPE, abilityType,
        EVENT_KEY_MODEL_TYPE, modelType,
        EVENT_KEY_BUNDLE_NAME, bundleName);
    HILOG_WARN("{eventName}: rid: %{public}d, uid: %{public}d, pid: %{pid}d, abilityName: %{public}s,"
        "abilityType: %{public}s, modelType: %{public}s, bundleName: %{public}s",
        rid,
        uid,
        pid,
        abilityName.c_str(),
        abilityType.c_str(),
        modelType.c_str(),
        bundleName.c_str());
}
void EventReport::AppEvent(AppExecFwk::ApplicationInfo &applicationInfo,
    const std::string &eventName, HiSysEventType type)
{
    std::string name = applicationInfo.name.c_str();
    std::string versionName = applicationInfo.versionName.c_str();
    uint32_t versionCode = applicationInfo.versionCode;
    std::string process = applicationInfo.process.c_str();
    int32_t timeStamp =
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
    EventReport::EventWrite(
        eventName,
        type,
        EVENT_KEY_APP_NAME, name,
        EVENT_KEY_VERSION_NAME, versionName,
        EVENT_KEY_VERSION_CODE, std::to_string(versionCode),
        EVENT_KEY_PROCESS, process,
        EVENT_KEY_TIME_STAMP, std::to_string(timeStamp));
    HILOG_WARN("{eventName}: name: %{public}s, versionName: %{public}s,"
        "versionCode: %{public}d, process: %{public}s, timeStamp: %{public}d",
        name.c_str(),
        versionName.c_str(),
        versionCode,
        process.c_str(),
        timeStamp);
}
template<typename... Types>
void EventReport::EventWrite(
    const std::string &eventName,
    HiSysEventType type,
    Types... keyValues)
{
    OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK,
        eventName,
        static_cast<OHOS::HiviewDFX::HiSysEvent::EventType>(type),
        keyValues...);
}
void EventReport::GetAbilityType(std::string &abilityType,
    std::string &modelType, AppExecFwk::AbilityInfo &abilityInfo)
{
    AppExecFwk::AbilityType type = abilityInfo.type;
    switch (type) {
#ifdef SUPPORT_GRAPHICS
        case AppExecFwk::AbilityType::PAGE: {
            abilityType = "PAGE";
            break;
        }
#endif
        case AppExecFwk::AbilityType::SERVICE: {
            abilityType = "SERVICE";
            break;
        }
        // for config.json type
        case AppExecFwk::AbilityType::DATA: {
            abilityType = "DATA";
            break;
        }
        case AppExecFwk::AbilityType::FORM: {
            abilityType = "FORM";
            break;
        }
        case AppExecFwk::AbilityType::EXTENSION: {
            abilityType = "EXTENSION";
            break;
        }
        default: {
            abilityType = "UNKNOWN";
            break;
        }
    }
    if (abilityInfo.isStageBasedModel) {
        modelType = "stage";
    } else {
        modelType = "FA";
    }
}
}  // namespace AAFWK
}  // namespace OHOS