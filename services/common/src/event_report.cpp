/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <map>

#include "event_report.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
// event params
constexpr const char *EVENT_KEY_APP_PID = "APP_PID";
constexpr const char *EVENT_KEY_USERID = "USER_ID";
constexpr const char *EVENT_KEY_ERROR_CODE = "ERROR_CODE";
constexpr const char *EVENT_KEY_BUNDLE_NAME = "BUNDLE_NAME";
constexpr const char *EVENT_KEY_MODULE_NAME = "MODULE_NAME";
constexpr const char *EVENT_KEY_ABILITY_NAME = "ABILITY_NAME";
constexpr const char *EVENT_KEY_ABILITY_TYPE = "ABILITY_TYPE";
constexpr const char *EVENT_KEY_VERSION_NAME = "VERSION_NAME";
constexpr const char *EVENT_KEY_VERSION_CODE = "VERSION_CODE";
constexpr const char *EVENT_KEY_PROCESS_NAME = "PROCESS_NAME";
constexpr const char *EVENT_KEY_EXTENSION_TYPE = "EXTENSION_TYPE";
constexpr const char *EVENT_KEY_STARTUP_TIME = "STARTUP_TIME";
constexpr const char *EVENT_KEY_STARTUP_ABILITY_TYPE = "STARTUP_ABILITY_TYPE";
constexpr const char *EVENT_KEY_STARTUP_EXTENSION_TYPE = "STARTUP_EXTENSION_TYPE";
constexpr const char *EVENT_KEY_CALLER_BUNDLE_NAME = "CALLER_BUNDLE_NAME";
constexpr const char *EVENT_KEY_CALLER_UID = "CALLER_UID";
constexpr const char *EVENT_KEY_CALLER_PROCESS_NAME = "CALLER_PROCESS_NAME";
constexpr const char *EVENT_KEY_EXIT_TIME = "EXIT_TIME";
constexpr const char *EVENT_KEY_EXIT_RESULT = "EXIT_RESULT";
constexpr const char *EVENT_KEY_EXIT_PID = "EXIT_PID";
constexpr const char *EVENT_KEY_BUNDLE_TYPE = "BUNDLE_TYPE";
constexpr const char *EVENT_KEY_START_TYPE = "START_TYPE";
constexpr const char *EVENT_KEY_CALLER_STATE = "CALLER_STATE";
constexpr const char *EVENT_KEY_CALLER_VERSION_NAME = "CALLER_VERSION_NAME";
constexpr const char *EVENT_KEY_CALLER_VERSION_CODE = "CALLER_VERSION_CODE";
constexpr const char *EVENT_KEY_URI = "URI";
constexpr const char *EVENT_KEY_RESTART_TIME = "RESTART_TIME";
constexpr const char *EVENT_KEY_APP_UID = "APP_UID";
constexpr const char *EVENT_KEY_PROCESS_TYPE = "PROCESS_TYPE";
const std::map<EventName, std::string> eventNameToStrMap_ = {
    std::map<EventName, std::string>::value_type(EventName::START_ABILITY_ERROR, "START_ABILITY_ERROR"),
    std::map<EventName, std::string>::value_type(EventName::TERMINATE_ABILITY_ERROR, "TERMINATE_ABILITY_ERROR"),
    std::map<EventName, std::string>::value_type(EventName::START_EXTENSION_ERROR, "START_EXTENSION_ERROR"),
    std::map<EventName, std::string>::value_type(EventName::STOP_EXTENSION_ERROR, "STOP_EXTENSION_ERROR"),
    std::map<EventName, std::string>::value_type(EventName::CONNECT_SERVICE_ERROR, "CONNECT_SERVICE_ERROR"),
    std::map<EventName, std::string>::value_type(EventName::DISCONNECT_SERVICE_ERROR, "DISCONNECT_SERVICE_ERROR"),
    std::map<EventName, std::string>::value_type(EventName::START_ABILITY, "START_ABILITY"),
    std::map<EventName, std::string>::value_type(EventName::TERMINATE_ABILITY, "TERMINATE_ABILITY"),
    std::map<EventName, std::string>::value_type(EventName::CLOSE_ABILITY, "CLOSE_ABILITY"),
    std::map<EventName, std::string>::value_type(EventName::ABILITY_ONFOREGROUND, "ABILITY_ONFOREGROUND"),
    std::map<EventName, std::string>::value_type(EventName::ABILITY_ONBACKGROUND, "ABILITY_ONBACKGROUND"),
    std::map<EventName, std::string>::value_type(EventName::ABILITY_ONACTIVE, "ABILITY_ONACTIVE"),
    std::map<EventName, std::string>::value_type(EventName::ABILITY_ONINACTIVE, "ABILITY_ONINACTIVE"),
    std::map<EventName, std::string>::value_type(EventName::START_SERVICE, "START_SERVICE"),
    std::map<EventName, std::string>::value_type(EventName::STOP_SERVICE, "STOP_SERVICE"),
    std::map<EventName, std::string>::value_type(EventName::CONNECT_SERVICE, "CONNECT_SERVICE"),
    std::map<EventName, std::string>::value_type(EventName::DISCONNECT_SERVICE, "DISCONNECT_SERVICE"),
    std::map<EventName, std::string>::value_type(EventName::APP_ATTACH, "APP_ATTACH"),
    std::map<EventName, std::string>::value_type(EventName::APP_LAUNCH, "APP_LAUNCH"),
    std::map<EventName, std::string>::value_type(EventName::APP_FOREGROUND, "APP_FOREGROUND"),
    std::map<EventName, std::string>::value_type(EventName::APP_BACKGROUND, "APP_BACKGROUND"),
    std::map<EventName, std::string>::value_type(EventName::APP_TERMINATE, "APP_TERMINATE"),
    std::map<EventName, std::string>::value_type(EventName::PROCESS_START, "PROCESS_START"),
    std::map<EventName, std::string>::value_type(EventName::PROCESS_EXIT, "PROCESS_EXIT"),
    std::map<EventName, std::string>::value_type(EventName::DRAWN_COMPLETED, "DRAWN_COMPLETED"),
    std::map<EventName, std::string>::value_type(EventName::APP_STARTUP_TYPE, "APP_STARTUP_TYPE"),
    std::map<EventName, std::string>::value_type(EventName::GRANT_URI_PERMISSION, "GRANT_URI_PERMISSION"),
    std::map<EventName, std::string>::value_type(EventName::FA_SHOW_ON_LOCK, "FA_SHOW_ON_LOCK"),
    std::map<EventName, std::string>::value_type(EventName::START_PRIVATE_ABILITY, "START_PRIVATE_ABILITY"),
    std::map<EventName, std::string>::value_type(EventName::RESTART_PROCESS_BY_SAME_APP,
        "RESTART_PROCESS_BY_SAME_APP"),
};
}

void EventReport::SendAppEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo)
{
    constexpr int32_t defaultVal = -1;
    std::string name = ConvertEventName(eventName);
    if (name == "INVALIDEVENTNAME") {
        HILOG_ERROR("invalid eventName");
        return;
    }
    switch (eventName) {
        case EventName::PROCESS_START:
            if (eventInfo.extensionType == defaultVal) {
                HiSysEventWrite(
                    HiSysEvent::Domain::AAFWK,
                    name,
                    type,
                    EVENT_KEY_STARTUP_TIME, eventInfo.time,
                    EVENT_KEY_STARTUP_ABILITY_TYPE, eventInfo.abilityType,
                    EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName,
                    EVENT_KEY_CALLER_UID, eventInfo.callerUid,
                    EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName,
                    EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
            } else {
                HiSysEventWrite(
                    HiSysEvent::Domain::AAFWK,
                    name,
                    type,
                    EVENT_KEY_STARTUP_TIME, eventInfo.time,
                    EVENT_KEY_STARTUP_ABILITY_TYPE, eventInfo.abilityType,
                    EVENT_KEY_STARTUP_EXTENSION_TYPE, eventInfo.extensionType,
                    EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName,
                    EVENT_KEY_CALLER_UID, eventInfo.callerUid,
                    EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName,
                    EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
            }
            break;
        case EventName::PROCESS_EXIT:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_EXIT_TIME, eventInfo.time,
                EVENT_KEY_EXIT_RESULT, eventInfo.exitResult,
                EVENT_KEY_EXIT_PID, eventInfo.pid);
            break;
        case EventName::APP_STARTUP_TYPE:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_APP_PID, eventInfo.pid,
                EVENT_KEY_VERSION_CODE, eventInfo.versionCode,
                EVENT_KEY_VERSION_NAME, eventInfo.versionName,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_ABILITY_NAME, eventInfo.abilityName,
                EVENT_KEY_START_TYPE, eventInfo.startType);
            break;
        case EventName::APP_FOREGROUND:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_APP_PID, eventInfo.pid,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_VERSION_NAME, eventInfo.versionName,
                EVENT_KEY_VERSION_CODE, eventInfo.versionCode,
                EVENT_KEY_PROCESS_NAME, eventInfo.processName,
                EVENT_KEY_BUNDLE_TYPE, eventInfo.bundleType,
                EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName,
                EVENT_KEY_PROCESS_TYPE, eventInfo.processType);
            break;
        case EventName::APP_BACKGROUND:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_APP_PID, eventInfo.pid,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_VERSION_NAME, eventInfo.versionName,
                EVENT_KEY_VERSION_CODE, eventInfo.versionCode,
                EVENT_KEY_PROCESS_NAME, eventInfo.processName,
                EVENT_KEY_BUNDLE_TYPE, eventInfo.bundleType,
                EVENT_KEY_PROCESS_TYPE, eventInfo.processType);
            break;
        case EventName::DRAWN_COMPLETED:
            HILOG_INFO("HiSysEvent name: DRAWN_COMPLETED, bundleName: %{public}s, abilityName: %{public}s",
                eventInfo.bundleName.c_str(), eventInfo.abilityName.c_str());
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_USERID, eventInfo.userId,
                EVENT_KEY_APP_PID, eventInfo.pid,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
                EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
            break;
        case EventName::APP_LAUNCH:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_APP_PID, eventInfo.pid,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_VERSION_NAME, eventInfo.versionName,
                EVENT_KEY_VERSION_CODE, eventInfo.versionCode,
                EVENT_KEY_PROCESS_NAME, eventInfo.processName,
                EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName,
                EVENT_KEY_CALLER_VERSION_NAME, eventInfo.callerVersionName,
                EVENT_KEY_CALLER_VERSION_CODE, eventInfo.callerVersionCode,
                EVENT_KEY_CALLER_UID, eventInfo.callerUid,
                EVENT_KEY_CALLER_STATE, eventInfo.callerState);
            break;
        default:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_APP_PID, eventInfo.pid,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_VERSION_NAME, eventInfo.versionName,
                EVENT_KEY_VERSION_CODE, eventInfo.versionCode,
                EVENT_KEY_PROCESS_NAME, eventInfo.processName);
            break;
    }
}

void EventReport::SendAbilityEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == "INVALIDEVENTNAME") {
        HILOG_ERROR("invalid eventName");
        return;
    }
    switch (eventName) {
        case EventName::START_ABILITY_ERROR:
        case EventName::TERMINATE_ABILITY_ERROR:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_USERID, eventInfo.userId,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
                EVENT_KEY_ABILITY_NAME, eventInfo.abilityName,
                EVENT_KEY_ERROR_CODE, eventInfo.errCode);
            break;
        case EventName::START_ABILITY:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_USERID, eventInfo.userId,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
                EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
            break;
        case EventName::TERMINATE_ABILITY:
        case EventName::CLOSE_ABILITY:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
            break;
        case EventName::ABILITY_ONFOREGROUND:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
                EVENT_KEY_ABILITY_NAME, eventInfo.abilityName,
                EVENT_KEY_BUNDLE_TYPE, eventInfo.bundleType,
                EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
            break;
        case EventName::ABILITY_ONBACKGROUND:
        case EventName::ABILITY_ONINACTIVE:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
                EVENT_KEY_ABILITY_NAME, eventInfo.abilityName,
                EVENT_KEY_BUNDLE_TYPE, eventInfo.bundleType);
            break;
        case EventName::ABILITY_ONACTIVE:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
                EVENT_KEY_ABILITY_NAME, eventInfo.abilityName,
                EVENT_KEY_ABILITY_TYPE, eventInfo.abilityType,
                EVENT_KEY_BUNDLE_TYPE, eventInfo.bundleType,
                EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
            break;
        default:
            break;
    }
}

void EventReport::SendExtensionEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == "INVALIDEVENTNAME") {
        HILOG_ERROR("invalid eventName");
        return;
    }
    switch (eventName) {
        case EventName::DISCONNECT_SERVICE:
            HiSysEventWrite(HiSysEvent::Domain::AAFWK, name, type);
            break;
        case EventName::CONNECT_SERVICE:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_USERID, eventInfo.userId,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
                EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
            break;
        case EventName::START_SERVICE:
        case EventName::STOP_SERVICE:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_USERID, eventInfo.userId,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
                EVENT_KEY_ABILITY_NAME, eventInfo.abilityName,
                EVENT_KEY_EXTENSION_TYPE, eventInfo.extensionType);
            break;
        case EventName::START_EXTENSION_ERROR:
        case EventName::STOP_EXTENSION_ERROR:
        case EventName::CONNECT_SERVICE_ERROR:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_USERID, eventInfo.userId,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
                EVENT_KEY_ABILITY_NAME, eventInfo.abilityName,
                EVENT_KEY_ERROR_CODE, eventInfo.errCode);
            break;
        case EventName::DISCONNECT_SERVICE_ERROR:
            HiSysEventWrite(HiSysEvent::Domain::AAFWK, name, type, EVENT_KEY_ERROR_CODE, eventInfo.errCode);
            break;
        default:
            break;
    }
}

void EventReport::SendKeyEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == "INVALIDEVENTNAME") {
        HILOG_ERROR("invalid eventName");
        return;
    }
    HILOG_INFO("name is %{public}s", name.c_str());
    switch (eventName) {
        case EventName::GRANT_URI_PERMISSION:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName,
                EVENT_KEY_URI, eventInfo.uri);
            break;
        case EventName::FA_SHOW_ON_LOCK:
        case EventName::START_PRIVATE_ABILITY:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
                EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
            break;
        case EventName::RESTART_PROCESS_BY_SAME_APP:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_RESTART_TIME, eventInfo.time,
                EVENT_KEY_APP_UID, eventInfo.appUid,
                EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName,
                EVENT_KEY_PROCESS_NAME, eventInfo.processName,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
            break;
        default:
            break;
    }
}

std::string EventReport::ConvertEventName(const EventName &eventName)
{
    auto it = eventNameToStrMap_.find(eventName);
    if (it != eventNameToStrMap_.end()) {
        return it->second;
    }
    return "INVALIDEVENTNAME";
}
}  // namespace AAFwk
}  // namespace OHOS
