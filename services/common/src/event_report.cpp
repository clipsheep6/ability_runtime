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

#include "event_report.h"

namespace OHOS {
namespace AAFWK {
namespace {
// event params
const std::string TYPE = "TYPE";
const std::string EVENT_KEY_APP_PID = "APP_PID";
const std::string EVENT_KEY_USERID = "USER_ID";
const std::string EVENT_KEY_FORM_ID = "FORM_ID";
const std::string EVENT_KEY_ERROR_CODE = "ERROR_CODE";
const std::string EVENT_KEY_BUNDLE_NAME = "BUNDLE_NAME";
const std::string EVENT_KEY_MODULE_NAME = "MODULE_NAME";
const std::string EVENT_KEY_ABILITY_NAME = "ABILITY_NAME";
const std::string EVENT_KEY_VERSION_NAME = "VERSION_NAME";
const std::string EVENT_KEY_VERSION_CODE = "VERSION_CODE";
const std::string EVENT_KEY_PROCESS_NAME = "PROCESS_NAME";
const std::string EVENT_KEY_EXTENSION_TYPE = "EXTENSION_TYPE";
}

void EventReport::SendAppEvent(const std::string &eventName, HiSysEventType type,
    const EventInfo& eventInfo)
{
    HiSysEvent::Write(
        HiSysEvent::Domain::AAFWK,
        eventName,
        type,
        EVENT_KEY_APP_PID, eventInfo.pid,
        EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
        EVENT_KEY_VERSION_NAME, eventInfo.versionName,
        EVENT_KEY_VERSION_CODE, eventInfo.versionCode,
        EVENT_KEY_PROCESS_NAME, eventInfo.processName);
}

void EventReport::SendAbilityEvent(const std::string &eventName, HiSysEventType type,
    const EventInfo& eventInfo)
{
    if (eventName == START_ABILITY_ERROR || eventName == TERMINATE_ABILITY_ERROR) {
        HiSysEvent::Write(
            HiSysEvent::Domain::AAFWK,
            eventName,
            type,
            EVENT_KEY_USERID, eventInfo.userId,
            EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
            EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
            EVENT_KEY_ABILITY_NAME, eventInfo.abilityName,
            EVENT_KEY_ERROR_CODE, eventInfo.errCode);
    } else if (eventName == START_ABILITY) {
        HiSysEvent::Write(
            HiSysEvent::Domain::AAFWK,
            eventName,
            type,
            EVENT_KEY_USERID, eventInfo.userId,
            EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
            EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
            EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    } else if (eventName == TERMINATE_ABILITY || eventName == CLOSE_ABILITY) {
        HiSysEvent::Write(
            HiSysEvent::Domain::AAFWK,
            eventName,
            type,
            EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
            EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    } else if (eventName == ABILITY_ONFOREGROUND || eventName == ABILITY_ONBACKGROUND) {
        HiSysEvent::Write(
            HiSysEvent::Domain::AAFWK,
            eventName,
            type,
            EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
            EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
            EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    }
}

void EventReport::SendExtensionEvent(const std::string &eventName, HiSysEventType type,
    const EventInfo& eventInfo)
{
    if (eventName == DISCONNECT_SERVICE) {
        HiSysEvent::Write(
            HiSysEvent::Domain::AAFWK,
            eventName,
            type);
    } else if (eventName == CONNECT_SERVICE) {
        HiSysEvent::Write(
            HiSysEvent::Domain::AAFWK,
            eventName,
            type,
            EVENT_KEY_USERID, eventInfo.userId,
            EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
            EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
            EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    } else if (eventName == START_SERVICE || eventName == STOP_SERVICE) {
        HiSysEvent::Write(
            HiSysEvent::Domain::AAFWK,
            eventName,
            type,
            EVENT_KEY_USERID, eventInfo.userId,
            EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
            EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
            EVENT_KEY_ABILITY_NAME, eventInfo.abilityName,
            EVENT_KEY_EXTENSION_TYPE, eventInfo.extensionType);
    } else if (eventName == START_EXTENSION_ERROR || eventName == STOP_EXTENSION_ERROR ||
        eventName == CONNECT_SERVICE_ERROR) {
        HiSysEvent::Write(
            HiSysEvent::Domain::AAFWK,
            eventName,
            type,
            EVENT_KEY_USERID, eventInfo.userId,
            EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
            EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
            EVENT_KEY_ABILITY_NAME, eventInfo.abilityName,
            EVENT_KEY_ERROR_CODE, eventInfo.errCode);
    } else if (eventName == DISCONNECT_SERVICE_ERROR) {
        HiSysEvent::Write(
            HiSysEvent::Domain::AAFWK,
            eventName,
            type,
            EVENT_KEY_ERROR_CODE, eventInfo.errCode);
    }
}

void EventReport::SendFormEvent(const std::string &eventName, HiSysEventType type,
    const EventInfo& eventInfo)
{
    if (eventName == DELETE_INVALID_FORM) {
        HiSysEvent::Write(
            HiSysEvent::Domain::AAFWK,
            eventName,
            type);
    } else if (eventName == ACQUIREFORMSTATE_FORM || eventName == MESSAGE_EVENT_FORM) {
        HiSysEvent::Write(
            HiSysEvent::Domain::AAFWK,
            eventName,
            type,
            EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
            EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
            EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    } else if (eventName == ADD_FORM || eventName == REQUEST_FORM || eventName == ROUTE_EVENT_FORM) {
        HiSysEvent::Write(
            HiSysEvent::Domain::AAFWK,
            eventName,
            type,
            EVENT_KEY_FORM_ID, eventInfo.formId,
            EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName,
            EVENT_KEY_MODULE_NAME, eventInfo.moduleName,
            EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    } else if (eventName == DELETE_FORM || eventName == CASTTEMP_FORM || eventName == RELEASE_FORM ||
        eventName == SET_NEXT_REFRESH_TIME_FORM) {
        HiSysEvent::Write(
            HiSysEvent::Domain::AAFWK,
            eventName,
            type,
            EVENT_KEY_FORM_ID, eventInfo.formId);
    }
}
}  // namespace AAFWK
}  // namespace OHOS
