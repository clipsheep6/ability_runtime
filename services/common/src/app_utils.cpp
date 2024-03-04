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

#include "app_utils.h"

#include "hilog_wrapper.h"
#include "parameters.h"
#include "scene_board_judgement.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string BUNDLE_NAME_LAUNCHER = "com.ohos.launcher";
const std::string BUNDLE_NAME_SCENEBOARD = "com.ohos.sceneboard";
const std::string LAUNCHER_ABILITY_NAME = "com.ohos.launcher.MainAbility";
const std::string SCENEBOARD_ABILITY_NAME = "com.ohos.sceneboard.MainAbility";
const std::string INHERIT_WINDOW_SPLIT_SCREEN_MODE = "persist.sys.abilityms.inherit_window_split_screen_mode";
const std::string SUPPORT_ANCO_APP = "persist.sys.abilityms.support_anco_app";
const std::string TIMEOUT_UNIT_TIME_RATIO = "persist.sys.abilityms.timeout_unit_time_ratio";
const std::string SELECTOR_DIALOG_POSSION = "persist.sys.abilityms.selector_dialog_possion";
const std::string START_SPECIFIED_PROCESS = "persist.sys.abilityms.start_specified_process";
const std::string USE_MULTI_RENDER_PROCESS = "persist.sys.abilityms.use_multi_render_process";
const std::string LIMIT_MAXIMUM_OF_RENDER_PROCESS = "persist.sys.abilityms.limit_maximum_of_render_process";
const std::string GRANT_PERSIST_URI_PERMISSION = "persist.sys.abilityms.grant_persist_uri_permission";
const std::string START_OPTIONS_WITH_ANIMATION = "persist.sys.abilityms.start_options_with_animation";
const std::string MULTI_PROCESS_MODEL = "persist.sys.abilityms.multi_process_model";
const std::string START_OPTIONS_WITH_PROCESS_OPTION = "persist.sys.abilityms.start_options_with_process_option";
const std::string MOVE_UI_ABILITY_TO_BACKGROUND_API_ENABLE =
    "persist.sys.abilityms.move_ui_ability_to_background_api_enable";
}
AppUtils::~AppUtils() {}

AppUtils::AppUtils()
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        isSceneBoard_ = true;
    }
}

AppUtils &AppUtils::GetInstance()
{
    static AppUtils utils;
    return utils;
}

bool AppUtils::IsLauncher(const std::string &bundleName) const
{
    if (isSceneBoard_) {
        return bundleName == BUNDLE_NAME_SCENEBOARD;
    }

    return bundleName == BUNDLE_NAME_LAUNCHER;
}

bool AppUtils::IsLauncherAbility(const std::string &abilityName) const
{
    if (isSceneBoard_) {
        return abilityName == SCENEBOARD_ABILITY_NAME;
    }

    return abilityName == LAUNCHER_ABILITY_NAME;
}

bool AppUtils::IsInheritWindowSplitScreenMode()
{
    if (!isInheritWindowSplitScreenMode_.isLoaded) {
        isInheritWindowSplitScreenMode_.value = system::GetBoolParameter(INHERIT_WINDOW_SPLIT_SCREEN_MODE, true);
        isInheritWindowSplitScreenMode_.isLoaded = true;
    }
    HILOG_INFO("isInheritWindowSplitScreenMode is %{public}d", isInheritWindowSplitScreenMode_.value);
    return isInheritWindowSplitScreenMode_.value;
}

bool AppUtils::IsSupportAncoApp()
{
    if (!isSupportAncoApp_.isLoaded) {
        isSupportAncoApp_.value = system::GetBoolParameter(SUPPORT_ANCO_APP, false);
        isSupportAncoApp_.isLoaded = true;
    }
    HILOG_INFO("isSupportAncoApp is %{public}d", isSupportAncoApp_.value);
    return isSupportAncoApp_.value;
}

int32_t AppUtils::GetTimeoutUnitTimeRatio()
{
    if (!timeoutUnitTimeRatio_.isLoaded) {
        timeoutUnitTimeRatio_.value = system::GetIntParameter<int32_t>(TIMEOUT_UNIT_TIME_RATIO, 1);
        timeoutUnitTimeRatio_.isLoaded = true;
    }
    HILOG_INFO("timeoutUnitTimeRatio is %{public}d", timeoutUnitTimeRatio_.value);
    return timeoutUnitTimeRatio_.value;
}

bool AppUtils::IsSelectorDialogDefaultPossion()
{
    if (!isSelectorDialogDefaultPossion_.isLoaded) {
        isSelectorDialogDefaultPossion_.value = system::GetBoolParameter(SELECTOR_DIALOG_POSSION, true);
        isSelectorDialogDefaultPossion_.isLoaded = true;
    }
    HILOG_INFO("isSelectorDialogDefaultPossion is %{public}d", isSelectorDialogDefaultPossion_.value);
    return isSelectorDialogDefaultPossion_.value;
}

bool AppUtils::IsStartSpecifiedProcess()
{
    if (!isStartSpecifiedProcess_.isLoaded) {
        isStartSpecifiedProcess_.value = system::GetBoolParameter(START_SPECIFIED_PROCESS, false);
        isStartSpecifiedProcess_.isLoaded = true;
    }
    HILOG_INFO("isStartSpecifiedProcess is %{public}d", isStartSpecifiedProcess_.value);
    return isStartSpecifiedProcess_.value;
}

bool AppUtils::IsUseMultiRenderProcess()
{
    if (!isUseMultiRenderProcess_.isLoaded) {
        isUseMultiRenderProcess_.value = system::GetBoolParameter(USE_MULTI_RENDER_PROCESS, true);
        isUseMultiRenderProcess_.isLoaded = true;
    }
    HILOG_INFO("isUseMultiRenderProcess is %{public}d", isUseMultiRenderProcess_.value);
    return isUseMultiRenderProcess_.value;
}

bool AppUtils::IsLimitMaximumOfRenderProcess()
{
    if (!isLimitMaximumOfRenderProcess_.isLoaded) {
        isLimitMaximumOfRenderProcess_.value = system::GetBoolParameter(LIMIT_MAXIMUM_OF_RENDER_PROCESS, true);
        isLimitMaximumOfRenderProcess_.isLoaded = true;
    }
    HILOG_INFO("isLimitMaximumOfRenderProcess_ is %{public}d", isLimitMaximumOfRenderProcess_.value);
    return isLimitMaximumOfRenderProcess_.value;
}

bool AppUtils::IsGrantPersistUriPermission()
{
    if (!isGrantPersistUriPermission_.isLoaded) {
        isGrantPersistUriPermission_.value = system::GetBoolParameter(GRANT_PERSIST_URI_PERMISSION, false);
        isGrantPersistUriPermission_.isLoaded = true;
    }
    HILOG_INFO("isGrantPersistUriPermission_ is %{public}d", isGrantPersistUriPermission_.value);
    return isGrantPersistUriPermission_.value;
}

bool AppUtils::IsStartOptionsWithAnimation()
{
    if (!isStartOptionsWithAnimation_.isLoaded) {
        isStartOptionsWithAnimation_.value = system::GetBoolParameter(START_OPTIONS_WITH_ANIMATION, false);
        isStartOptionsWithAnimation_.isLoaded = true;
    }
    HILOG_INFO("isStartOptionsWithAnimation_ is %{public}d", isStartOptionsWithAnimation_.value);
    return isStartOptionsWithAnimation_.value;
}

bool AppUtils::IsMultiProcessModel()
{
    if (!isMultiProcessModel_.isLoaded) {
        isMultiProcessModel_.value = system::GetBoolParameter(MULTI_PROCESS_MODEL, false);
        isMultiProcessModel_.isLoaded = true;
    }
    HILOG_INFO("isMultiProcessModel_ is %{public}d", isMultiProcessModel_.value);
    return isMultiProcessModel_.value;
}

bool AppUtils::IsStartOptionsWithProcessOption()
{
    if (!isStartOptionsWithProcessOption_.isLoaded) {
        isStartOptionsWithProcessOption_.value = system::GetBoolParameter(START_OPTIONS_WITH_PROCESS_OPTION, false);
        isStartOptionsWithProcessOption_.isLoaded = true;
    }
    HILOG_INFO("isStartOptionsWithProcessOption_ is %{public}d", isStartOptionsWithProcessOption_.value);
    return isStartOptionsWithProcessOption_.value;
}

bool AppUtils::EnableMoveUIAbilityToBackgroundApi()
{
    if (!enableMoveUIAbilityToBackgroundApi_.isLoaded) {
        enableMoveUIAbilityToBackgroundApi_.value =
            system::GetBoolParameter(MOVE_UI_ABILITY_TO_BACKGROUND_API_ENABLE, true);
        enableMoveUIAbilityToBackgroundApi_.isLoaded = true;
    }
    HILOG_INFO("enableMoveUIAbilityToBackgroundApi_ is %{public}d", enableMoveUIAbilityToBackgroundApi_.value);
    return enableMoveUIAbilityToBackgroundApi_.value;
}
}  // namespace AAFwk
}  // namespace OHOS
