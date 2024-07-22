/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "startup_util.h"

#include "ability_info.h"
#include "extension_ability_info.h"
#include "global_constant.h"
#include "server_constant.h"
#include "want.h"

namespace OHOS::AbilityRuntime {
bool StartupUtil::GetAppIndex(const AAFwk::Want &want, int32_t &appIndex)
{
    appIndex = want.GetIntParam(ServerConstant::DLP_INDEX, 0);
    if (appIndex > GlobalConstant::MAX_APP_CLONE_INDEX) {
        return true;
    }
    if (appIndex == 0) {
        appIndex = want.GetIntParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0);
        if (appIndex < 0 || appIndex > GlobalConstant::MAX_APP_CLONE_INDEX) {
            return false;
        }
        return true;
    }
    return false;
}

int32_t StartupUtil::BuildAbilityInfoFlag()
{
    return AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_PERMISSION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_METADATA;
}

bool StartupUtil::IsSupportAppClone(AppExecFwk::ExtensionAbilityType type)
{
    return type == AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER ||
        type == AppExecFwk::ExtensionAbilityType::BACKUP ||
        type == AppExecFwk::ExtensionAbilityType::SHARE;
}

void StartupUtil::InitAbilityInfoFromExtension(AppExecFwk::ExtensionAbilityInfo &extensionInfo,
    AppExecFwk::AbilityInfo &abilityInfo)
{
    abilityInfo.applicationName = extensionInfo.applicationInfo.name;
    abilityInfo.applicationInfo = extensionInfo.applicationInfo;
    abilityInfo.bundleName = extensionInfo.bundleName;
    abilityInfo.package = extensionInfo.moduleName;
    abilityInfo.moduleName = extensionInfo.moduleName;
    abilityInfo.name = extensionInfo.name;
    abilityInfo.srcEntrance = extensionInfo.srcEntrance;
    abilityInfo.srcPath = extensionInfo.srcEntrance;
    abilityInfo.iconPath = extensionInfo.icon;
    abilityInfo.iconId = extensionInfo.iconId;
    abilityInfo.label = extensionInfo.label;
    abilityInfo.labelId = extensionInfo.labelId;
    abilityInfo.description = extensionInfo.description;
    abilityInfo.descriptionId = extensionInfo.descriptionId;
    abilityInfo.priority = extensionInfo.priority;
    abilityInfo.permissions = extensionInfo.permissions;
    abilityInfo.readPermission = extensionInfo.readPermission;
    abilityInfo.writePermission = extensionInfo.writePermission;
    abilityInfo.uri = extensionInfo.uri;
    abilityInfo.extensionAbilityType = extensionInfo.type;
    abilityInfo.visible = extensionInfo.visible;
    abilityInfo.resourcePath = extensionInfo.resourcePath;
    abilityInfo.enabled = extensionInfo.enabled;
    abilityInfo.isModuleJson = true;
    abilityInfo.isStageBasedModel = true;
    abilityInfo.process = extensionInfo.process;
    abilityInfo.metadata = extensionInfo.metadata;
    abilityInfo.compileMode = extensionInfo.compileMode;
    abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityInfo.extensionTypeName = extensionInfo.extensionTypeName;
    if (!extensionInfo.hapPath.empty()) {
        abilityInfo.hapPath = extensionInfo.hapPath;
    }
}
}  // namespace OHOS::AbilityRuntime
