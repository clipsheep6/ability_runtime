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

#ifndef OHOS_AAFWK_ABILITY_CONFIG_H
#define OHOS_AAFWK_ABILITY_CONFIG_H

#include <string>

namespace OHOS {
namespace AAFwk {
namespace AbilityConfig {
constexpr const char* NAME_ABILITY_MGR_SERVICE = "AbilityManagerService";
constexpr const char* SCHEME_DATA_ABILITY = "dataability";
constexpr const char* SYSTEM_UI_BUNDLE_NAME = "com.ohos.systemui";
constexpr const char* SYSTEM_UI_STATUS_BAR = "com.ohos.systemui.statusbar.ServiceExtAbility";
constexpr const char* SYSTEM_UI_NAVIGATION_BAR = "com.ohos.systemui.navigationbar.ServiceExtAbility";
constexpr const char* SYSTEM_DIALOG_NAME = "com.ohos.systemui.systemdialog.MainAbility";
constexpr const char* SYSTEM_UI_ABILITY_NAME = "com.ohos.systemui.ServiceExtAbility";

constexpr const char* SYSTEM_DIALOG_REQUEST_PERMISSIONS = "OHOS_RESULT_PERMISSIONS_LIST_YES";
constexpr const char* SYSTEM_DIALOG_CALLER_BUNDLENAME = "OHOS_RESULT_CALLER_BUNDLERNAME";
constexpr const char* SYSTEM_DIALOG_KEY = "OHOS_RESULT_PERMISSION_KEY";

constexpr const char* DEVICE_MANAGER_BUNDLE_NAME = "com.ohos.devicemanagerui";
constexpr const char* DEVICE_MANAGER_NAME = "com.ohos.devicemanagerui.MainAbility";

constexpr const char* EVENT_SYSTEM_WINDOW_MODE_CHANGED = "common.event.SYSTEM_WINDOW_MODE_CHANGED";
const int EVENT_CODE_SYSTEM_WINDOW_MODE_CHANGED = 1;

constexpr const char* MISSION_NAME_MARK_HEAD = "#";
constexpr const char* MISSION_NAME_SEPARATOR = ":";

constexpr const char* FLOATING_WINDOW_PERMISSION = "ohos.permission.SYSTEM_FLOAT_WINDOW";

constexpr const char* LAUNCHER_ABILITY_NAME = "com.ohos.launcher.MainAbility";
constexpr const char* LAUNCHER_BUNDLE_NAME = "com.ohos.launcher";
constexpr const char* LAUNCHER_RECENT_ABILITY_NAME = "com.ohos.launcher.recents.MainAbility";

constexpr const char* SETTINGS_DATA_ABILITY_NAME = "com.ohos.settingsdata.data";
constexpr const char* SETTINGS_DATA_BUNDLE_NAME = "com.ohos.settingsdata";

constexpr const char* PHONE_SERVICE_BUNDLE_NAME = "com.ohos.callui";
constexpr const char* PHONE_SERVICE_ABILITY_NAME = "com.ohos.callui.ServiceAbility";
constexpr const char* LOCK_SCREEN_EVENT_NAME = "lock_screen";

constexpr const char* MMS_ABILITY_NAME = "com.ohos.mms.ServiceAbility";
constexpr const char* MMS_BUNDLE_NAME = "com.ohos.mms";

constexpr const char* PARAMS_STREAM = "ability.params.stream";

constexpr const char* GRANT_ABILITY_BUNDLE_NAME = "com.ohos.permissionmanager";
constexpr const char* GRANT_ABILITY_ABILITY_NAME = "com.ohos.permissionmanager.GrantAbility";
}  // namespace AbilityConfig
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_AAFWK_ABILITY_CONFIG_H
