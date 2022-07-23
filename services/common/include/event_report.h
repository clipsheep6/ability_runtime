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

#ifndef FOUNDATION_AAFWK_SERVICES_COMMON_INCLUDE_EVENT_REPORT_H
#define FOUNDATION_AAFWK_SERVICES_COMMON_INCLUDE_EVENT_REPORT_H

#include <string>
#include <unordered_map>

#include "hisysevent.h"

using HiSysEventType = OHOS::HiviewDFX::HiSysEvent::EventType;
using HiSysEvent = OHOS::HiviewDFX::HiSysEvent;

namespace OHOS {
namespace AAFWK {
namespace {
// fault event
constexpr const char* START_ABILITY_ERROR = "START_ABILITY_ERROR";
constexpr const char* TERMINATE_ABILITY_ERROR = "TERMINATE_ABILITY_ERROR";
constexpr const char* START_EXTENSION_ERROR = "START_EXTENSION_ERROR";
constexpr const char* STOP_EXTENSION_ERROR = "STOP_EXTENSION_ERROR";
constexpr const char* CONNECT_SERVICE_ERROR = "CONNECT_SERVICE_ERROR";
constexpr const char* DISCONNECT_SERVICE_ERROR = "DISCONNECT_SERVICE_ERROR";
// ability behavior event
constexpr const char* START_ABILITY = "START_ABILITY";
constexpr const char* TERMINATE_ABILITY = "TERMINATE_ABILITY";
constexpr const char* CLOSE_ABILITY = "CLOSE_ABILITY";
constexpr const char* ABILITY_ONFOREGROUND = "ABILITY_ONFOREGROUND";
constexpr const char* ABILITY_ONBACKGROUND = "ABILITY_ONBACKGROUND";
// serviceExtensionAbility behavior event
constexpr const char* START_SERVICE = "START_SERVICE";
constexpr const char* STOP_SERVICE = "STOP_SERVICE";
constexpr const char* CONNECT_SERVICE = "CONNECT_SERVICE";
constexpr const char* DISCONNECT_SERVICE = "DISCONNECT_SERVICE";
// form behavior event
constexpr const char* ADD_FORM = "ADD_FORM";
constexpr const char* REQUEST_FORM = "REQUEST_FORM";
constexpr const char* DELETE_FORM = "DELETE_FORM";
constexpr const char* CASTTEMP_FORM = "CASTTEMP_FORM";
constexpr const char* ACQUIREFORMSTATE_FORM = "ACQUIREFORMSTATE_FORM";
constexpr const char* MESSAGE_EVENT_FORM = "MESSAGE_EVENT_FORM";
constexpr const char* ROUTE_EVENT_FORM = "ROUTE_EVENT_FORM";
constexpr const char* RELEASE_FORM = "RELEASE_FORM";
constexpr const char* DELETE_INVALID_FORM = "DELETE_INVALID_FORM";
constexpr const char* SET_NEXT_REFRESH_TIME_FORM = "SET_NEXT_REFRESH_TIME_FORM";
// app behavior event
constexpr const char* APP_ATTACH = "APP_ATTACH";
constexpr const char* APP_LAUNCH = "APP_LAUNCH";
constexpr const char* APP_FOREGROUND = "APP_FOREGROUND";
constexpr const char* APP_BACKGROUND = "APP_BACKGROUND";
constexpr const char* APP_TERMINATE = "APP_TERMINATE";
}

struct EventInfo {
    int32_t pid = -1;
    int32_t userId = -1;
    int64_t formId = -1;
    int32_t extensionType = -1;
    uint32_t versionCode = 0;
    int32_t errCode = -1;
    std::string versionName;
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    std::string processName;
};

class EventReport {
public:
    static void SendAppEvent(const std::string &eventName, HiSysEventType type,
        const EventInfo& eventInfo);
    static void SendAbilityEvent(const std::string &eventName, HiSysEventType type,
        const EventInfo& eventInfo);
    static void SendExtensionEvent(const std::string &eventName, HiSysEventType type,
        const EventInfo& eventInfo);
    static void SendFormEvent(const std::string &eventName, HiSysEventType type,
        const EventInfo& eventInfo);
};
}  // namespace AAFWK
}  // namespace OHOS
#endif  // FOUNDATION_AAFWK_SERVICES_COMMON_INCLUDE_EVENT_REPORT_H
