/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_AAFwk_UI_EXTENSION_UTILS_H
#define OHOS_AAFwk_UI_EXTENSION_UTILS_H

#include <unordered_set>

#include "extension_ability_info.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AAFwk {
namespace UIExtensionUtils {
// ui extension type list
const std::unordered_set<AppExecFwk::ExtensionAbilityType> UI_EXTENSION_SET = {
    AppExecFwk::ExtensionAbilityType::SHARE,
    AppExecFwk::ExtensionAbilityType::ACTION,
    AppExecFwk::ExtensionAbilityType::EMBEDDED_UI,
    AppExecFwk::ExtensionAbilityType::AUTO_FILL_PASSWORD,
    AppExecFwk::ExtensionAbilityType::UI,
    AppExecFwk::ExtensionAbilityType::SYSPICKER_MEDIACONTROL,
    AppExecFwk::ExtensionAbilityType::SYSDIALOG_USERAUTH,
    AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON,
    AppExecFwk::ExtensionAbilityType::SYSDIALOG_ATOMICSERVICEPANEL,
    AppExecFwk::ExtensionAbilityType::SYSDIALOG_POWER,
    AppExecFwk::ExtensionAbilityType::SYSPICKER_SHARE,
    AppExecFwk::ExtensionAbilityType::HMS_ACCOUNT,
    AppExecFwk::ExtensionAbilityType::ADS,
    AppExecFwk::ExtensionAbilityType::VOIP,
    AppExecFwk::ExtensionAbilityType::SYSDIALOG_MEETIMECALL,
    AppExecFwk::ExtensionAbilityType::SYSDIALOG_MEETIMECONTACT,
    AppExecFwk::ExtensionAbilityType::SYSDIALOG_MEETIMEMESSAGE,
    AppExecFwk::ExtensionAbilityType::SYSDIALOG_PRINT,
    AppExecFwk::ExtensionAbilityType::SYSPICKER_MEETIMECONTACT,
    AppExecFwk::ExtensionAbilityType::SYSPICKER_MEETIMECALLLOG,
    AppExecFwk::ExtensionAbilityType::SYSPICKER_PHOTOPICKER,
    AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI,
    AppExecFwk::ExtensionAbilityType::SYSPICKER_CAMERA
};
const int EDM_SA_UID = 3057;

inline bool IsUIExtension(const AppExecFwk::ExtensionAbilityType type)
{
    return UI_EXTENSION_SET.find(type) != UI_EXTENSION_SET.end();
}

inline bool IsEnterpriseAdmin(const AppExecFwk::ExtensionAbilityType type)
{
    bool enterpriseAdminSa = (IPCSkeleton::GetCallingUid() == EDM_SA_UID);
    bool isEnterpriseAdmin = (type == AppExecFwk::ExtensionAbilityType::ENTERPRISE_ADMIN);
    return enterpriseAdminSa && isEnterpriseAdmin;
}

inline bool IsWindowExtension(const AppExecFwk::ExtensionAbilityType type)
{
    return type == AppExecFwk::ExtensionAbilityType::WINDOW;
}
} // namespace UIExtensionUtils
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_AAFwk_UI_EXTENSION_UTILS_H
