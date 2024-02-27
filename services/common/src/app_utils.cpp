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
const std::string INHERIT_WINDOW_SPLIT_SCREEN_MODE = "persist.sys.ams.inherit_window_split_screen_mode";
const std::string SUPPORT_ANCO_APP = "persist.sys.ams.support_anco_app";
const std::string TIMEOUT_UNIT_TIME_RATIO = "persist.sys.ams.timeout_unit_time_ratio";
const std::string SELECTOR_DIALOG_POSSION = "persist.sys.ams.selector_dialog_possion";
const std::string START_SPECIFIED_PROCESS = "persist.sys.ams.start_specified_process";
const std::string USE_MULTI_RENDER_PROCESS = "persist.sys.ams.use_multi_render_process";
const std::string LIMIT_MAXIMUM_OF_RENDER_PROCESS = "persist.sys.ams.limit_maximum_of_render_process";
const std::string GRANT_PERSIST_URI_PERMISSION = "persist.sys.ams.grant_persist_uri_permission";
const std::string START_OPTIONS_WITH_ANIMATION = "persist.sys.ams.start_options_with_animation";
const std::string MULTI_PROCESS_MODEL = "persist.sys.ams.multi_process_model";
const std::string START_OPTIONS_WITH_PROCESS_OPTION = "persist.sys.ams.start_options_with_process_option";
const std::string MOVE_UI_ABILITY_TO_BACKGROUND_API_ENABLE = 
    "persist.sys.ams.move_ui_ability_to_background_api_enable";
}
AppUtils::~AppUtils() {}

AppUtils::AppUtils()
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        isSceneBoard_ = true;
    }
    isInheritWindowSplitScreenMode_ = system::GetBoolParameter(INHERIT_WINDOW_SPLIT_SCREEN_MODE, true);
    isSupportAncoApp_ = system::GetBoolParameter(SUPPORT_ANCO_APP, false);
    timeoutUnitTimeRatio_ = system::GetIntParameter<int32_t>(TIMEOUT_UNIT_TIME_RATIO, 1);
    isSelectorDialogDefaultPossion_ = system::GetBoolParameter(SELECTOR_DIALOG_POSSION, true);
    isStartSpecifiedProcess_ = system::GetBoolParameter(START_SPECIFIED_PROCESS, false);
    isUseMultiRenderProcess_ = system::GetBoolParameter(USE_MULTI_RENDER_PROCESS, true);
    isLimitMaximumOfRenderProcess_ = system::GetBoolParameter(LIMIT_MAXIMUM_OF_RENDER_PROCESS, true);
    isGrantPersistUriPermission_ = system::GetBoolParameter(GRANT_PERSIST_URI_PERMISSION, false);
    isStartOptionsWithAnimation_ = system::GetBoolParameter(START_OPTIONS_WITH_ANIMATION, false);
    isMultiProcessModel_ = system::GetBoolParameter(MULTI_PROCESS_MODEL, false);
    isStartOptionsWithProcessOption_ = system::GetBoolParameter(START_OPTIONS_WITH_PROCESS_OPTION, false);
    enableMoveUIAbilityToBackgroundApi_ = system::GetBoolParameter(MOVE_UI_ABILITY_TO_BACKGROUND_API_ENABLE, true);
    HILOG_INFO("isInheritWindowSplitScreenMode is %{public}d", isInheritWindowSplitScreenMode_);
    HILOG_INFO("isSupportAncoApp is %{public}d", isSupportAncoApp_);
    HILOG_INFO("timeoutUnitTimeRatio is %{public}d", timeoutUnitTimeRatio_);
    HILOG_INFO("isSelectorDialogDefaultPossion is %{public}d", isSelectorDialogDefaultPossion_);
    HILOG_INFO("isStartSpecifiedProcess is %{public}d", isStartSpecifiedProcess_);
    HILOG_INFO("isUseMultiRenderProcess is %{public}d", isUseMultiRenderProcess_);
    HILOG_INFO("isLimitMaximumOfRenderProcess is %{public}d", isLimitMaximumOfRenderProcess_);
    HILOG_INFO("isGrantPersistUriPermission is %{public}d", isGrantPersistUriPermission_);
    HILOG_INFO("isStartOptionsWithAnimation is %{public}d", isStartOptionsWithAnimation_);
    HILOG_INFO("isMultiProcessModel is %{public}d", isMultiProcessModel_);
    HILOG_INFO("isStartOptionsWithProcessOption is %{public}d", isStartOptionsWithProcessOption_);
    HILOG_INFO("enableMoveUIAbilityToBackgroundApi is %{public}d", enableMoveUIAbilityToBackgroundApi_);
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

bool AppUtils::JudgePCDevice() const
{
    return isPcDevice_;
}

bool AppUtils::IsInheritWindowSplitScreenMode() const
{
    return isInheritWindowSplitScreenMode_;
}

bool AppUtils::IsSupportAncoApp() const
{
    return isSupportAncoApp_;
}

int32_t AppUtils::GetTimeoutUnitTimeRatio() const
{
    return timeoutUnitTimeRatio_;
}

bool AppUtils::IsSelectorDialogDefaultPossion() const
{
    return isSelectorDialogDefaultPossion_;
}

bool AppUtils::IsStartSpecifiedProcess() const
{
    return isStartSpecifiedProcess_;
}

bool AppUtils::IsUseMultiRenderProcess() const
{
    return isUseMultiRenderProcess_;
}

bool AppUtils::IsLimitMaximumOfRenderProcess() const
{
    return isLimitMaximumOfRenderProcess_;
}

bool AppUtils::IsGrantPersistUriPermission() const
{
    return isGrantPersistUriPermission_;
}

bool AppUtils::IsStartOptionsWithAnimation() const
{
    return isStartOptionsWithAnimation_;
}

bool AppUtils::IsMultiProcessModel() const
{
    return isMultiProcessModel_;
}

bool AppUtils::IsStartOptionsWithProcessOption() const
{
    return isStartOptionsWithProcessOption_;
}

bool AppUtils::EnableMoveUIAbilityToBackgroundApi() const
{
    return enableMoveUIAbilityToBackgroundApi_;
}
}  // namespace AAFwk
}  // namespace OHOS
