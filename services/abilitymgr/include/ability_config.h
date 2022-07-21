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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_CONFIG_H
#define OHOS_ABILITY_RUNTIME_ABILITY_CONFIG_H

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
constexpr const char* DEVICE_MANAGER_BUNDLE_NAME = "com.ohos.devicemanagerui";
constexpr const char* DEVICE_MANAGER_NAME = "com.ohos.devicemanagerui.MainAbility";
constexpr const char* LAUNCHER_ABILITY_NAME = "com.ohos.launcher.MainAbility";
constexpr const char* LAUNCHER_BUNDLE_NAME = "com.ohos.launcher";
constexpr const char* LAUNCHER_RECENT_ABILITY_NAME = "com.ohos.launcher.recents.MainAbility";
constexpr const char* GRANT_ABILITY_BUNDLE_NAME = "com.ohos.permissionmanager";
constexpr const char* GRANT_ABILITY_ABILITY_NAME = "com.ohos.permissionmanager.GrantAbility";
constexpr const char* PARAMS_STREAM = "ability.params.stream";
constexpr const char* MISSION_NAME_MARK_HEAD = "#";
constexpr const char* MISSION_NAME_SEPARATOR = ":";
}  // namespace AbilityConfig
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ABILITY_CONFIG_H
