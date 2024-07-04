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

#include "event_report.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"

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
constexpr const char *EVENT_KEY_ABILITY_NUMBER = "ABILITY_NUMBER";
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
constexpr const char *EVENT_KEY_CALLER_PROCESS_ID = "CALLER_PROCESS_ID";
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
constexpr const char *EVENT_KEY_TIME = "TIME";
constexpr const char *EVENT_KEY_PID = "PID";
constexpr const char *INVALID_EVENT_NAME = "INVALIDEVENTNAME";
}

void EventReport::SendAppEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    switch (eventName) {
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
        case EventName::DRAWN_COMPLETED:
            TAG_LOGI(AAFwkTag::DEFAULT,
                "HiSysEvent name: DRAWN_COMPLETED, bundleName: %{public}s, abilityName: %{public}s",
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
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "EventName is %{public}s", name.c_str());
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
        case EventName::START_STANDARD_ABILITIES:
            TAG_LOGD(AAFwkTag::DEFAULT, "EventInfo is [%{public}d, %{public}s, %{public}s, %{public}s]",
                eventInfo.userId, eventInfo.bundleName.c_str(), eventInfo.moduleName.c_str(),
                eventInfo.abilityName.c_str());
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_USERID, eventInfo.userId,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
                EVENT_KEY_ABILITY_NAME, eventInfo.abilityName,
                EVENT_KEY_ABILITY_NUMBER, eventInfo.abilityNumber);
            break;
        default:
            break;
    }
}

void EventReport::SendAtomicServiceEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    switch (eventName) {
        case EventName::ATOMIC_SERVICE_DRAWN_COMPLETE:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
                EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
            break;
        case EventName::CREATE_ATOMIC_SERVICE_PROCESS:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                type,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
                EVENT_KEY_ABILITY_NAME, eventInfo.abilityName,
                EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName,
                EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName,
                EVENT_KEY_CALLER_UID, eventInfo.callerUid);
            break;
        default:
            break;
    }
}

void EventReport::SendGrantUriPermissionEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName: %{public}s", name.c_str());
        return;
    }
    switch (eventName) {
        case EventName::GRANT_URI_PERMISSION:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                HiSysEventType::BEHAVIOR,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
                EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName,
                EVENT_KEY_URI, eventInfo.uri);
            break;
        case EventName::SHARE_UNPRIVILEGED_FILE_URI:
            HiSysEventWrite(
                HiSysEvent::Domain::AAFWK,
                name,
                HiSysEventType::BEHAVIOR,
                EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName,
                EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
            break;
        default:
            break;
    }
}

void EventReport::SendExtensionEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    switch (eventName) {
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
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    TAG_LOGI(AAFwkTag::DEFAULT, "name is %{public}s", name.c_str());
    switch (eventName) {
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

void EventReport::SendAppLaunchEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    HiSysEventWrite(
        HiSysEvent::Domain::AAFWK,
        name,
        HiSysEventType::BEHAVIOR,
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
}

void EventReport::SendAppForegroundEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    auto ret = HiSysEventWrite(
        HiSysEvent::Domain::AAFWK,
        name,
        HiSysEventType::BEHAVIOR,
        EVENT_KEY_APP_PID, eventInfo.pid,
        EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_KEY_VERSION_NAME, eventInfo.versionName,
        EVENT_KEY_VERSION_CODE, eventInfo.versionCode,
        EVENT_KEY_PROCESS_NAME, eventInfo.processName,
        EVENT_KEY_BUNDLE_TYPE, eventInfo.bundleType,
        EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName,
        EVENT_KEY_PROCESS_TYPE, eventInfo.processType);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Write event fail: %{public}s, ret %{public}d", name.c_str(), ret);
    }
}

void EventReport::SendAppBackgroundEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    auto ret = HiSysEventWrite(
        HiSysEvent::Domain::AAFWK,
        name,
        HiSysEventType::BEHAVIOR,
        EVENT_KEY_APP_PID, eventInfo.pid,
        EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_KEY_VERSION_NAME, eventInfo.versionName,
        EVENT_KEY_VERSION_CODE, eventInfo.versionCode,
        EVENT_KEY_PROCESS_NAME, eventInfo.processName,
        EVENT_KEY_BUNDLE_TYPE, eventInfo.bundleType,
        EVENT_KEY_PROCESS_TYPE, eventInfo.processType);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Write event fail: %{public}s, ret %{public}d", name.c_str(), ret);
    }
}

void EventReport::SendProcessStartEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    constexpr int32_t defaultVal = -1;
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    if (eventInfo.extensionType == defaultVal) {
        HiSysEventWrite(
            HiSysEvent::Domain::AAFWK,
            name,
            HiSysEventType::BEHAVIOR,
            EVENT_KEY_STARTUP_TIME, eventInfo.time,
            EVENT_KEY_STARTUP_ABILITY_TYPE, eventInfo.abilityType,
            EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName,
            EVENT_KEY_CALLER_UID, eventInfo.callerUid,
            EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName,
            EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
            EVENT_KEY_PID, eventInfo.pid,
            EVENT_KEY_PROCESS_NAME, eventInfo.processName,
            EVENT_KEY_CALLER_PROCESS_ID, eventInfo.callerPid);
    } else {
        HiSysEventWrite(
            HiSysEvent::Domain::AAFWK,
            name,
            HiSysEventType::BEHAVIOR,
            EVENT_KEY_STARTUP_TIME, eventInfo.time,
            EVENT_KEY_STARTUP_ABILITY_TYPE, eventInfo.abilityType,
            EVENT_KEY_STARTUP_EXTENSION_TYPE, eventInfo.extensionType,
            EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName,
            EVENT_KEY_CALLER_UID, eventInfo.callerUid,
            EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName,
            EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
            EVENT_KEY_PID, eventInfo.pid,
            EVENT_KEY_PROCESS_NAME, eventInfo.processName,
            EVENT_KEY_CALLER_PROCESS_ID, eventInfo.callerPid);
    }
}

void EventReport::SendProcessExitEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    HiSysEventWrite(
        HiSysEvent::Domain::AAFWK,
        name,
        HiSysEventType::BEHAVIOR,
        EVENT_KEY_EXIT_TIME, eventInfo.time,
        EVENT_KEY_EXIT_RESULT, eventInfo.exitResult,
        EVENT_KEY_EXIT_PID, eventInfo.pid,
        EVENT_KEY_PROCESS_NAME, eventInfo.processName,
        EVENT_KEY_EXTENSION_TYPE, eventInfo.extensionType);
}

void EventReport::SendStartServiceEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    HiSysEventWrite(
        HiSysEvent::Domain::AAFWK,
        name,
        HiSysEventType::BEHAVIOR,
        EVENT_KEY_TIME, eventInfo.time,
        EVENT_KEY_USERID, eventInfo.userId,
        EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
        EVENT_KEY_ABILITY_NAME, eventInfo.abilityName,
        EVENT_KEY_EXTENSION_TYPE, eventInfo.extensionType,
        EVENT_KEY_PID, eventInfo.pid,
        EVENT_KEY_PROCESS_NAME, eventInfo.processName,
        EVENT_KEY_CALLER_PROCESS_ID, eventInfo.callerPid,
        EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName);
}

void EventReport::SendStopServiceEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    HiSysEventWrite(
        HiSysEvent::Domain::AAFWK,
        name,
        HiSysEventType::BEHAVIOR,
        EVENT_KEY_USERID, eventInfo.userId,
        EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
        EVENT_KEY_ABILITY_NAME, eventInfo.abilityName,
        EVENT_KEY_EXTENSION_TYPE, eventInfo.extensionType,
        EVENT_KEY_TIME, eventInfo.time,
        EVENT_KEY_PID, eventInfo.pid,
        EVENT_KEY_PROCESS_NAME, eventInfo.processName,
        EVENT_KEY_CALLER_PROCESS_ID, eventInfo.callerPid,
        EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName);
}

void EventReport::SendConnectServiceEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    HiSysEventWrite(
        HiSysEvent::Domain::AAFWK,
        name,
        HiSysEventType::BEHAVIOR,
        EVENT_KEY_TIME, eventInfo.time,
        EVENT_KEY_USERID, eventInfo.userId,
        EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
        EVENT_KEY_ABILITY_NAME, eventInfo.abilityName,
        EVENT_KEY_PID, eventInfo.pid,
        EVENT_KEY_PROCESS_NAME, eventInfo.processName,
        EVENT_KEY_CALLER_PROCESS_ID, eventInfo.callerPid,
        EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName);
}

void EventReport::SendDisconnectServiceEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    HiSysEventWrite(HiSysEvent::Domain::AAFWK,
        name,
        HiSysEventType::BEHAVIOR,
        EVENT_KEY_TIME, eventInfo.time,
        EVENT_KEY_PID, eventInfo.pid,
        EVENT_KEY_PROCESS_NAME, eventInfo.processName,
        EVENT_KEY_CALLER_PROCESS_ID, eventInfo.callerPid,
        EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName);
}

std::string EventReport::ConvertEventName(const EventName &eventName)
{
    const char* eventNames[] = {
        // fault event
        "START_ABILITY_ERROR", "TERMINATE_ABILITY_ERROR", "START_EXTENSION_ERROR",
        "STOP_EXTENSION_ERROR", "CONNECT_SERVICE_ERROR", "DISCONNECT_SERVICE_ERROR",

        // ability behavior event
        "START_ABILITY", "TERMINATE_ABILITY", "CLOSE_ABILITY",
        "ABILITY_ONFOREGROUND", "ABILITY_ONBACKGROUND", "ABILITY_ONACTIVE", "ABILITY_ONINACTIVE",

        // serviceExtensionAbility behavior event
        "START_SERVICE", "STOP_SERVICE", "CONNECT_SERVICE", "DISCONNECT_SERVICE",

        // app behavior event
        "APP_ATTACH", "APP_LAUNCH", "APP_FOREGROUND", "APP_BACKGROUND", "APP_TERMINATE",
        "PROCESS_START", "PROCESS_EXIT", "DRAWN_COMPLETED", "APP_STARTUP_TYPE",

        // key behavior event
        "GRANT_URI_PERMISSION", "FA_SHOW_ON_LOCK", "START_PRIVATE_ABILITY",
        "RESTART_PROCESS_BY_SAME_APP", "START_STANDARD_ABILITIES",

        // atomic service event
        "CREATE_ATOMIC_SERVICE_PROCESS", "ATOMIC_SERVICE_DRAWN_COMPLETE",
        
        // uri permission
        "SHARE_UNPRIVILEGED_FILE_URI"
    };
    uint32_t eventIndex = static_cast<uint32_t> (eventName);
    if (eventIndex >= sizeof(eventNames) / sizeof(const char*)) {
        return INVALID_EVENT_NAME;
    }
    return eventNames[eventIndex];
}
}  // namespace AAFwk
}  // namespace OHOS
