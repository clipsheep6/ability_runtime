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

namespace OHOS {
namespace AAFWK {
namespace {
const std::string EVENT_KEY_ABILITY_NAME = "ABILITY_NAME";
const std::string EVENT_KEY_BUNDLE_NAME = "BUNDLE_NAME";
const std::string EVENT_KEY_DEVICEID = "DEVICEID";
const std::string EVENT_KEY_URI = "URI";
const std::string EVENT_KEY_ACTION = "ACTION";
const std::string TYPE = "TYPE";
}
void EventReport::AbilitySomeCallbackEvent(const AAFwk::Want &want, const std::string &eventName, HiSysEventType type)
{
    AppExecFwk::ElementName element = want.GetElement();
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    std::string deviceId = element.GetDeviceID();
    std::string uri = want.GetUriString().c_str();
    std::string action = want.GetAction().c_str();
    EventReport::EventWrite(
        eventName,
        type,
        EVENT_KEY_ABILITY_NAME, abilityName,
        EVENT_KEY_BUNDLE_NAME, bundleName,
        EVENT_KEY_DEVICEID, deviceId,
        EVENT_KEY_URI, uri,
        EVENT_KEY_ACTION, action);
    HILOG_WARN("{eventName}: deviceId: %{public}s, abilityName: %{public}s,"
        "bundleName: %{public}s, uri: %{public}s, action: %{public}s",
        deviceId.c_str(),
        abilityName.c_str(),
        bundleName.c_str(),
        uri.c_str(),
        action.c_str());
}
void EventReport::AbilityOtherCallbackEvent(const std::string abilityName,
    const std::string bundleName, const std::string &eventName, HiSysEventType type)
{
    EventReport::EventWrite(
        eventName,
        type,
        EVENT_KEY_ABILITY_NAME, abilityName.c_str(),
        EVENT_KEY_BUNDLE_NAME, bundleName.c_str());
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
}  // namespace AAFWK
}  // namespace OHOS