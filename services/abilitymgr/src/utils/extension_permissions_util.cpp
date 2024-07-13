/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "utils/extension_permissions_util.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "permission_verification.h"

namespace OHOS {
namespace AAFwk {

bool ExtensionPermissionsUtil::CheckSAPermission(const AppExecFwk::ExtensionAbilityType &extensionType)
{
    auto checkRet = false;
    if (!PermissionVerification::GetInstance()->IsSACall()) {
        return true;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "CheckSAPermission, extensionType: %{public}d.", extensionType);
    if (extensionType == AppExecFwk::ExtensionAbilityType::FORM) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_FORM_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_WORK_SCHEDULER_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::INPUTMETHOD) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_INPUT_METHOD_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::ACCESSIBILITY) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_ACCESSIBILITY_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::STATICSUBSCRIBER) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_STATIC_SUBSCRIBER_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::WALLPAPER) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_WALLPAPER_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::BACKUP) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_BACKUP_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::ENTERPRISE_ADMIN) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_ENTERPRISE_ADMIN_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::PRINT) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_PRINT_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::VPN) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_VPN_EXTENSION");
    } else {
        checkRet = CheckSAPermissionMore(extensionType);
    }
    if (!checkRet) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SA connect permission verification failed.");
        return false;
    }

    return true;
}

bool ExtensionPermissionsUtil::CheckSAPermissionMore(const AppExecFwk::ExtensionAbilityType &extensionType)
{
    auto checkRet = false;
    if (extensionType == AppExecFwk::ExtensionAbilityType::FILEACCESS_EXTENSION) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_FILE_ACCESS_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::REMOTE_NOTIFICATION) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_REMOTE_NOTIFICATION_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::REMOTE_LOCATION) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_REMOTE_LOCATION_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::DRIVER) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_DRIVER_EXTENSION");
    } else {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "No need connect permission for extension type %{public}d.", extensionType);
        return true;
    }

    return checkRet;
}

} // namespace AAFwk
} // namespace OHOS
