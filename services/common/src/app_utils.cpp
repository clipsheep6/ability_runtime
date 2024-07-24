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
#include "json_utils.h"
#include "hilog_tag_wrapper.h"
#include "nlohmann/json.hpp"
#include "parameters.h"
#ifdef SUPPORT_GRAPHICS
#include "scene_board_judgement.h"
#endif // SUPPORT_GRAPHICS


namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* BUNDLE_NAME_LAUNCHER = "com.ohos.launcher";
constexpr const char* BUNDLE_NAME_SCENEBOARD = "com.ohos.sceneboard";
constexpr const char* LAUNCHER_ABILITY_NAME = "com.ohos.launcher.MainAbility";
constexpr const char* SCENEBOARD_ABILITY_NAME = "com.ohos.sceneboard.MainAbility";
constexpr const char* INHERIT_WINDOW_SPLIT_SCREEN_MODE = "persist.sys.abilityms.inherit_window_split_screen_mode";
constexpr const char* SUPPORT_ANCO_APP = "persist.sys.abilityms.support_anco_app";
constexpr const char* TIMEOUT_UNIT_TIME_RATIO = "persist.sys.abilityms.timeout_unit_time_ratio";
constexpr const char* SELECTOR_DIALOG_POSSION = "persist.sys.abilityms.selector_dialog_possion";
constexpr const char* START_SPECIFIED_PROCESS = "persist.sys.abilityms.start_specified_process";
constexpr const char* USE_MULTI_RENDER_PROCESS = "persist.sys.abilityms.use_multi_render_process";
constexpr const char* LIMIT_MAXIMUM_OF_RENDER_PROCESS = "persist.sys.abilityms.limit_maximum_of_render_process";
constexpr const char* GRANT_PERSIST_URI_PERMISSION = "persist.sys.abilityms.grant_persist_uri_permission";
constexpr const char* START_OPTIONS_WITH_ANIMATION = "persist.sys.abilityms.start_options_with_animation";
constexpr const char* MULTI_PROCESS_MODEL = "persist.sys.abilityms.multi_process_model";
constexpr const char* START_OPTIONS_WITH_PROCESS_OPTION = "persist.sys.abilityms.start_options_with_process_option";
constexpr const char* MOVE_UI_ABILITY_TO_BACKGROUND_API_ENABLE =
    "persist.sys.abilityms.move_ui_ability_to_background_api_enable";
constexpr const char* CONFIG_PATH = "/etc/ability_runtime/resident_process_in_extreme_memory.json";
constexpr const char* RESIDENT_PROCESS_IN_EXTREME_MEMORY = "residentProcessInExtremeMemory";
constexpr const char* BUNDLE_NAME = "bundleName";
constexpr const char* ABILITY_NAME = "abilityName";
constexpr const char* LAUNCH_EMBEDED_UI_ABILITY = "const.abilityms.launch_embeded_ui_ability";
const std::string SUPPROT_NATIVE_CHILD_PROCESS = "persist.sys.abilityms.start_native_child_process";
const std::string LIMIT_MAXIMUM_EXTENSIONS_OF_PER_PROCESS =
    "const.sys.abilityms.limit_maximum_extensions_of_per_process";
const std::string LIMIT_MAXIMUM_EXTENSIONS_OF_PER_DEVICE =
    "const.sys.abilityms.limit_maximum_extensions_of_per_device";
const std::string CACHE_EXTENSION_TYPES = "const.sys.abilityms.cache_extension";
constexpr const char* START_ABILITY_WITHOUT_CALLERTOKEN = "/system/etc/start_ability_without_caller_token.json";
constexpr const char* START_ABILITY_WITHOUT_CALLERTOKEN_PATH =
    "/etc/ability_runtime/start_ability_without_caller_token.json";
constexpr const char* START_ABILITY_WITHOUT_CALLERTOKEN_TITLE = "startAbilityWithoutCallerToken";
}

AppUtils::~AppUtils() {}

AppUtils::AppUtils()
{
    #ifdef SUPPORT_GRAPHICS
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        isSceneBoard_ = true;
    }
    #endif // SUPPORT_GRAPHICS
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
    TAG_LOGD(AAFwkTag::DEFAULT, "isInheritWindowSplitScreenMode is %{public}d", isInheritWindowSplitScreenMode_.value);
    return isInheritWindowSplitScreenMode_.value;
}

bool AppUtils::IsSupportAncoApp()
{
    if (!isSupportAncoApp_.isLoaded) {
        isSupportAncoApp_.value = system::GetBoolParameter(SUPPORT_ANCO_APP, false);
        isSupportAncoApp_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "isSupportAncoApp is %{public}d", isSupportAncoApp_.value);
    return isSupportAncoApp_.value;
}

int32_t AppUtils::GetTimeoutUnitTimeRatio()
{
    if (!timeoutUnitTimeRatio_.isLoaded) {
        timeoutUnitTimeRatio_.value = system::GetIntParameter<int32_t>(TIMEOUT_UNIT_TIME_RATIO, 1);
        timeoutUnitTimeRatio_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "timeoutUnitTimeRatio is %{public}d", timeoutUnitTimeRatio_.value);
    return timeoutUnitTimeRatio_.value;
}

bool AppUtils::IsSelectorDialogDefaultPossion()
{
    if (!isSelectorDialogDefaultPossion_.isLoaded) {
        isSelectorDialogDefaultPossion_.value = system::GetBoolParameter(SELECTOR_DIALOG_POSSION, true);
        isSelectorDialogDefaultPossion_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "isSelectorDialogDefaultPossion is %{public}d", isSelectorDialogDefaultPossion_.value);
    return isSelectorDialogDefaultPossion_.value;
}

bool AppUtils::IsStartSpecifiedProcess()
{
    if (!isStartSpecifiedProcess_.isLoaded) {
        isStartSpecifiedProcess_.value = system::GetBoolParameter(START_SPECIFIED_PROCESS, false);
        isStartSpecifiedProcess_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "isStartSpecifiedProcess is %{public}d", isStartSpecifiedProcess_.value);
    return isStartSpecifiedProcess_.value;
}

bool AppUtils::IsUseMultiRenderProcess()
{
    if (!isUseMultiRenderProcess_.isLoaded) {
        isUseMultiRenderProcess_.value = system::GetBoolParameter(USE_MULTI_RENDER_PROCESS, true);
        isUseMultiRenderProcess_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "isUseMultiRenderProcess is %{public}d", isUseMultiRenderProcess_.value);
    return isUseMultiRenderProcess_.value;
}

bool AppUtils::IsLimitMaximumOfRenderProcess()
{
    if (!isLimitMaximumOfRenderProcess_.isLoaded) {
        isLimitMaximumOfRenderProcess_.value = system::GetBoolParameter(LIMIT_MAXIMUM_OF_RENDER_PROCESS, true);
        isLimitMaximumOfRenderProcess_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "isLimitMaximumOfRenderProcess_ is %{public}d", isLimitMaximumOfRenderProcess_.value);
    return isLimitMaximumOfRenderProcess_.value;
}

bool AppUtils::IsGrantPersistUriPermission()
{
    if (!isGrantPersistUriPermission_.isLoaded) {
        isGrantPersistUriPermission_.value = system::GetBoolParameter(GRANT_PERSIST_URI_PERMISSION, false);
        isGrantPersistUriPermission_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "isGrantPersistUriPermission_ is %{public}d", isGrantPersistUriPermission_.value);
    return isGrantPersistUriPermission_.value;
}

bool AppUtils::IsStartOptionsWithAnimation()
{
    if (!isStartOptionsWithAnimation_.isLoaded) {
        isStartOptionsWithAnimation_.value = system::GetBoolParameter(START_OPTIONS_WITH_ANIMATION, false);
        isStartOptionsWithAnimation_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "isStartOptionsWithAnimation_ is %{public}d", isStartOptionsWithAnimation_.value);
    return isStartOptionsWithAnimation_.value;
}

bool AppUtils::IsMultiProcessModel()
{
    if (!isMultiProcessModel_.isLoaded) {
        isMultiProcessModel_.value = system::GetBoolParameter(MULTI_PROCESS_MODEL, false);
        isMultiProcessModel_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "isMultiProcessModel_ is %{public}d", isMultiProcessModel_.value);
    return isMultiProcessModel_.value;
}

bool AppUtils::IsStartOptionsWithProcessOptions()
{
    if (!isStartOptionsWithProcessOptions_.isLoaded) {
        isStartOptionsWithProcessOptions_.value = system::GetBoolParameter(START_OPTIONS_WITH_PROCESS_OPTION, false);
        isStartOptionsWithProcessOptions_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT,
        "isStartOptionsWithProcessOptions_ is %{public}d", isStartOptionsWithProcessOptions_.value);
    return isStartOptionsWithProcessOptions_.value;
}

bool AppUtils::EnableMoveUIAbilityToBackgroundApi()
{
    if (!enableMoveUIAbilityToBackgroundApi_.isLoaded) {
        enableMoveUIAbilityToBackgroundApi_.value =
            system::GetBoolParameter(MOVE_UI_ABILITY_TO_BACKGROUND_API_ENABLE, true);
        enableMoveUIAbilityToBackgroundApi_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT,
        "enableMoveUIAbilityToBackgroundApi_ is %{public}d", enableMoveUIAbilityToBackgroundApi_.value);
    return enableMoveUIAbilityToBackgroundApi_.value;
}

bool AppUtils::IsLaunchEmbededUIAbility()
{
    if (!isLaunchEmbededUIAbility_.isLoaded) {
        isLaunchEmbededUIAbility_.value = system::GetBoolParameter(LAUNCH_EMBEDED_UI_ABILITY, false);
        isLaunchEmbededUIAbility_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "isLaunchEmbededUIAbility_ is %{public}d", isLaunchEmbededUIAbility_.value);
    return isLaunchEmbededUIAbility_.value;
}

bool AppUtils::IsSupportNativeChildProcess()
{
    if (!isSupportNativeChildProcess_.isLoaded) {
        isSupportNativeChildProcess_.value = system::GetBoolParameter(SUPPROT_NATIVE_CHILD_PROCESS, false);
        isSupportNativeChildProcess_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "isSupportNativeChildProcess_ is %{public}d", isSupportNativeChildProcess_.value);
    return isSupportNativeChildProcess_.value;
}

bool AppUtils::IsAllowResidentInExtremeMemory(const std::string& bundleName, const std::string& abilityName)
{
    if (!residentProcessInExtremeMemory_.isLoaded) {
        LoadResidentProcessInExtremeMemory();
        residentProcessInExtremeMemory_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "isSupportNativeChildProcess_ is %{public}d", isSupportNativeChildProcess_.value);
    for (auto &element : residentProcessInExtremeMemory_.value) {
        if (bundleName == element.first &&
            (abilityName == "" || abilityName == element.second)) {
            return true;
        }
    }
    return false;
}

void AppUtils::LoadResidentProcessInExtremeMemory()
{
    nlohmann::json object;
    if (!JsonUtils::GetInstance().LoadConfiguration(CONFIG_PATH, object)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "load resident process in extreme memory failed.");
        return;
    }
    if (!object.contains(RESIDENT_PROCESS_IN_EXTREME_MEMORY)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "resident process in extreme memory config not existed.");
        return;
    }

    for (auto &item : object.at(RESIDENT_PROCESS_IN_EXTREME_MEMORY).items()) {
        const nlohmann::json& jsonObject = item.value();
        if (!jsonObject.contains(BUNDLE_NAME) || !jsonObject.at(BUNDLE_NAME).is_string()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to load bundleName.");
            return;
        }
        if (!jsonObject.contains(ABILITY_NAME) || !jsonObject.at(ABILITY_NAME).is_string()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to load abilityName.");
            return;
        }
        std::string bundleName = jsonObject.at(BUNDLE_NAME).get<std::string>();
        std::string abilityName = jsonObject.at(ABILITY_NAME).get<std::string>();
        residentProcessInExtremeMemory_.value.emplace_back(std::make_pair(bundleName, abilityName));
    }
}

int32_t AppUtils::GetLimitMaximumExtensionsPerProc()
{
    if (!limitMaximumExtensionsPerProc_.isLoaded) {
        limitMaximumExtensionsPerProc_.value =
            system::GetIntParameter<int32_t>(LIMIT_MAXIMUM_EXTENSIONS_OF_PER_PROCESS, DEFAULT_MAX_EXT_PER_PROC);
        limitMaximumExtensionsPerProc_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "limitMaximumExtensionsPerProc is %{public}d", limitMaximumExtensionsPerProc_.value);
    return limitMaximumExtensionsPerProc_.value;
}

int32_t AppUtils::GetLimitMaximumExtensionsPerDevice()
{
    if (!limitMaximumExtensionsPerDevice_.isLoaded) {
        limitMaximumExtensionsPerDevice_.value =
            system::GetIntParameter<int32_t>(LIMIT_MAXIMUM_EXTENSIONS_OF_PER_DEVICE, DEFAULT_MAX_EXT_PER_DEV);
        limitMaximumExtensionsPerDevice_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "limitMaximumExtensionsPerDevice is %{public}d",
        limitMaximumExtensionsPerDevice_.value);
    return limitMaximumExtensionsPerDevice_.value;
}

std::string AppUtils::GetCacheExtensionTypeList()
{
    std::string cacheExtAbilityTypeList = system::GetParameter(CACHE_EXTENSION_TYPES, "260");
    TAG_LOGD(AAFwkTag::DEFAULT, "cacheExtAbilityTypeList is %{public}s", cacheExtAbilityTypeList.c_str());
    return cacheExtAbilityTypeList;
}

bool AppUtils::IsAllowStartAbilityWithoutCallerToken(const std::string& bundleName, const std::string& abilityName)
{
    if (!startAbilityWithoutCallerToken_.isLoaded) {
        LoadStartAbilityWithoutCallerToken();
        startAbilityWithoutCallerToken_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "isLoaded: %{public}d", startAbilityWithoutCallerToken_.isLoaded);
    for (auto &element : startAbilityWithoutCallerToken_.value) {
        if (bundleName == element.first && abilityName == element.second) {
            TAG_LOGI(AAFwkTag::DEFAULT, "call");
            return true;
        }
    }
    return false;
}

void AppUtils::LoadStartAbilityWithoutCallerToken()
{
    nlohmann::json object;
    if (!JsonUtils::GetInstance().LoadConfiguration(
        START_ABILITY_WITHOUT_CALLERTOKEN_PATH, object, START_ABILITY_WITHOUT_CALLERTOKEN)) {
        TAG_LOGE(AAFwkTag::DEFAULT, "load start ability without caller token list failed.");
        return;
    }
    if (!object.contains(START_ABILITY_WITHOUT_CALLERTOKEN_TITLE)) {
        TAG_LOGE(AAFwkTag::DEFAULT, "start ability without caller token config not existed.");
        return;
    }

    for (auto &item : object.at(START_ABILITY_WITHOUT_CALLERTOKEN_TITLE).items()) {
        const nlohmann::json& jsonObject = item.value();
        if (!jsonObject.contains(BUNDLE_NAME) || !jsonObject.at(BUNDLE_NAME).is_string()) {
            TAG_LOGE(AAFwkTag::DEFAULT, "failed to load bundleName.");
            return;
        }
        if (!jsonObject.contains(ABILITY_NAME) || !jsonObject.at(ABILITY_NAME).is_string()) {
            TAG_LOGE(AAFwkTag::DEFAULT, "failed to load abilityName.");
            return;
        }
        std::string bundleName = jsonObject.at(BUNDLE_NAME).get<std::string>();
        std::string abilityName = jsonObject.at(ABILITY_NAME).get<std::string>();
        startAbilityWithoutCallerToken_.value.emplace_back(std::make_pair(bundleName, abilityName));
    }
}
}  // namespace AAFwk
}  // namespace OHOS
