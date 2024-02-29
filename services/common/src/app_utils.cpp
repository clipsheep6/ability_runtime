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
}

AppUtils &AppUtils::GetInstance()
{
    static AppUtils utils;
    return utils;
}

void AppUtils::PrintParamInfo() const
{
    HILOG_INFO("PrintParamInfo isInheritWindowSplitScreenMode is %{public}d", isInheritWindowSplitScreenMode_);
    HILOG_INFO("PrintParamInfo isSupportAncoApp is %{public}d", isSupportAncoApp_);
    HILOG_INFO("PrintParamInfo timeoutUnitTimeRatio is %{public}d", timeoutUnitTimeRatio_);
    HILOG_INFO("PrintParamInfo isSelectorDialogDefaultPossion is %{public}d", isSelectorDialogDefaultPossion_);
    HILOG_INFO("PrintParamInfo isStartSpecifiedProcess is %{public}d", isStartSpecifiedProcess_);
    HILOG_INFO("PrintParamInfo isUseMultiRenderProcess is %{public}d", isUseMultiRenderProcess_);
    HILOG_INFO("PrintParamInfo isLimitMaximumOfRenderProcess is %{public}d", isLimitMaximumOfRenderProcess_);
    HILOG_INFO("PrintParamInfo isGrantPersistUriPermission is %{public}d", isGrantPersistUriPermission_);
    HILOG_INFO("PrintParamInfo isStartOptionsWithAnimation is %{public}d", isStartOptionsWithAnimation_);
    HILOG_INFO("PrintParamInfo isMultiProcessModel is %{public}d", isMultiProcessModel_);
    HILOG_INFO("PrintParamInfo isStartOptionsWithProcessOption is %{public}d", isStartOptionsWithProcessOption_);
    HILOG_INFO("PrintParamInfo enableMoveUIAbilityToBackgroundApi is %{public}d", enableMoveUIAbilityToBackgroundApi_);
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

bool AppUtils::IsInheritWindowSplitScreenMode() const
{
    PrintParamInfo();
    return isInheritWindowSplitScreenMode_;
}

bool AppUtils::IsSupportAncoApp() const
{
    PrintParamInfo();
    return isSupportAncoApp_;
}

int32_t AppUtils::GetTimeoutUnitTimeRatio() const
{
    PrintParamInfo();
    return timeoutUnitTimeRatio_;
}

bool AppUtils::IsSelectorDialogDefaultPossion() const
{
    PrintParamInfo();
    return isSelectorDialogDefaultPossion_;
}

bool AppUtils::IsStartSpecifiedProcess() const
{
    PrintParamInfo();
    return isStartSpecifiedProcess_;
}

bool AppUtils::IsUseMultiRenderProcess() const
{
    PrintParamInfo();
    return isUseMultiRenderProcess_;
}

bool AppUtils::IsLimitMaximumOfRenderProcess() const
{
    PrintParamInfo();
    return isLimitMaximumOfRenderProcess_;
}

bool AppUtils::IsGrantPersistUriPermission() const
{
    PrintParamInfo();
    return isGrantPersistUriPermission_;
}

bool AppUtils::IsStartOptionsWithAnimation() const
{
    PrintParamInfo();
    return isStartOptionsWithAnimation_;
}

bool AppUtils::IsMultiProcessModel() const
{
    PrintParamInfo();
    return isMultiProcessModel_;
}

bool AppUtils::IsStartOptionsWithProcessOption() const
{
    PrintParamInfo();
    return isStartOptionsWithProcessOption_;
}

bool AppUtils::EnableMoveUIAbilityToBackgroundApi() const
{
    PrintParamInfo();
    return enableMoveUIAbilityToBackgroundApi_;
}
}  // namespace AAFwk
}  // namespace OHOS
