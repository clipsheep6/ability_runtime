/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "system_dialog_scheduler.h"

#include <cmath>
#include <regex>

#include "display_info.h"
#include "constants.h"
#include "ability_record.h"
#include "ability_util.h"
#include "app_gallery_enable_util.h"
#include "app_scheduler.h"
#include "dm_common.h"
#include "display_manager.h"
#include "errors.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "locale_config.h"
#include "parameters.h"
#include "resource_manager.h"
#include "scene_board_judgement.h"
#include "ui_extension_utils.h"

namespace OHOS {
namespace AAFwk {
const int32_t UI_SELECTOR_DIALOG_WIDTH = 328 * 2;
const int32_t UI_SELECTOR_DIALOG_HEIGHT = 350 * 2;
const int32_t UI_SELECTOR_DIALOG_HEIGHT_NARROW = 350 * 2;
const int32_t UI_SELECTOR_DIALOG_WIDTH_NARROW = 328 * 2;
const int32_t UI_SELECTOR_DIALOG_PHONE_H1 = 240 * 2;
const int32_t UI_SELECTOR_DIALOG_PHONE_H2 = 340 * 2;
const int32_t UI_SELECTOR_DIALOG_PHONE_H3 = 350 * 2;
const int32_t UI_SELECTOR_DIALOG_PC_H0 = 1;
const int32_t UI_SELECTOR_DIALOG_PC_H2 = (64 * 2 + 56 + 48 + 54 + 64 + 48 + 2) * 2;
const int32_t UI_SELECTOR_DIALOG_PC_H3 = (64 * 3 + 56 + 48 + 54 + 64 + 48 + 2) * 2;
const int32_t UI_SELECTOR_DIALOG_PC_H4 = (64 * 4 + 56 + 48 + 54 + 64 + 48 + 2) * 2;
const int32_t UI_SELECTOR_DIALOG_PC_H5 = (64 * 4 + 56 + 48 + 54 + 64 + 48 + 58 + 2) * 2;

const int32_t UI_SELECTOR_PORTRAIT_PHONE_H1 = 280;
const int32_t UI_SELECTOR_PORTRAIT_PHONE_H2 = 400;
const int32_t UI_SELECTOR_PORTRAIT_PHONE_H3 = 410;
const int32_t UI_SELECTOR_LANDSCAPE_SIGNAL_BAR = 24;
const int32_t UI_SELECTOR_LANDSCAPE_HEIGHT = 350;
const int32_t UI_SELECTOR_LANDSCAPE_HEIGHT_NARROW = 350;
const int32_t UI_SELECTOR_LANDSCAPE_PHONE_H1 = 280;
const int32_t UI_SELECTOR_LANDSCAPE_PHONE_H2 = 400;
const int32_t UI_SELECTOR_LANDSCAPE_PHONE_H3 = 410;
const int32_t UI_SELECTOR_LANDSCAPE_COUNT_THREE = 3;
const int32_t UI_SELECTOR_LANDSCAPE_COUNT_FOUR = 4;
const float UI_SELECTOR_LANDSCAPE_GRILLE_LARGE = 0.107692;
const float UI_SELECTOR_LANDSCAPE_GRILLE_SAMLL = 0.015385;
const float UI_SELECTOR_LANDSCAPE_MAX_RATIO = 0.9;
const float UI_SELECTOR_PORTRAIT_WIDTH_RATIO = 0.8;
const float UI_SELECTOR_PORTRAIT_WIDTH_EDGE_RATIO = 0.1;
const float UI_SELECTOR_PORTRAIT_HEIGHT_RATIO = 0.98;

const int32_t UI_TIPS_DIALOG_WIDTH = 328 * 2;
const int32_t UI_TIPS_DIALOG_HEIGHT = 135 * 2;
const int32_t UI_TIPS_DIALOG_HEIGHT_NARROW = 135 * 2;
const int32_t UI_TIPS_DIALOG_WIDTH_NARROW = 328 * 2;

const int32_t UI_JUMP_INTERCEPTOR_DIALOG_WIDTH = 328 * 2;
const int32_t UI_JUMP_INTERCEPTOR_DIALOG_HEIGHT = 135 * 2;
const int32_t UI_JUMP_INTERCEPTOR_DIALOG_HEIGHT_NARROW = 135 * 2;
const int32_t UI_JUMP_INTERCEPTOR_DIALOG_WIDTH_NARROW = 328 * 2;

const int32_t UI_ANR_DIALOG_WIDTH = 328 * 2;
const int32_t UI_ANR_DIALOG_HEIGHT = 192 * 2;
const std::string APP_NAME = "appName";
const std::string DEVICE_TYPE = "deviceType";
const std::string OFF_SET_X = "offsetX";
const std::string OFF_SET_Y = "offsetY";
const std::string WIDTH = "width";
const std::string HEIGHT = "height";
const std::string MODEL_FLAG = "modelFlag";
const std::string ACTION = "action";
const std::string OVERSIZE_HEIGHT = "oversizeHeight";

const int32_t UI_HALF = 2;
const int32_t UI_DEFAULT_BUTTOM_CLIP = 100;
const int32_t UI_WIDTH_780DP = 1560;
const int32_t UI_DEFAULT_WIDTH = 2560;
const int32_t UI_DEFAULT_HEIGHT = 1600;

const std::string STR_PHONE = "phone";
const std::string STR_DEFAULT = "default";
const std::string DIALOG_NAME_ANR = "dialog_anr_service";
const std::string DIALOG_NAME_TIPS = "dialog_tips_service";
const std::string DIALOG_SELECTOR_NAME = "dialog_selector_service";
const std::string DIALOG_JUMP_INTERCEPTOR_NAME = "dialog_jump_interceptor_service";

const std::string BUNDLE_NAME = "bundleName";
const std::string BUNDLE_NAME_DIALOG = "com.ohos.amsdialog";
const std::string DIALOG_PARAMS = "params";
const std::string DIALOG_POSITION = "position";
const std::string VERTICAL_SCREEN_DIALOG_POSITION = "landscapeScreen";
const std::string ABILITY_NAME_ANR_DIALOG = "AnrDialog";
const std::string ABILITY_NAME_FREEZE_DIALOG = "SwitchUserDialog";
const std::string ABILITY_NAME_TIPS_DIALOG = "TipsDialog";
const std::string ABILITY_NAME_SELECTOR_DIALOG = "SelectorDialog";
const std::string ABILITY_NAME_APPGALLERY_SELECTOR_DIALOG = "AppSelectorExtensionAbility";
const std::string UIEXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
const std::string UIEXTENSION_SYS_COMMON_UI = "sys/commonUI";
const std::string CALLER_TOKEN = "callerToken";
const std::string ABILITY_NAME_JUMP_INTERCEPTOR_DIALOG = "JumpInterceptorDialog";
const std::string TYPE_ONLY_MATCH_WILDCARD = "reserved/wildcard";
const std::string ORIENTATION = "orientation";

const int32_t LINE_NUMS_ZERO = 0;
const int32_t LINE_NUMS_TWO = 2;
const int32_t LINE_NUMS_THREE = 3;
const int32_t LINE_NUMS_FOUR = 4;
const int32_t LINE_NUMS_EIGHT = 8;

const float WIDTH_MULTIPLE = 0.8;
const float HEIGHT_MULTIPLE = 0.3;
const float SETX_WIDTH_MULTIPLE = 0.1;

bool SystemDialogScheduler::GetANRDialogWant(int userId, int pid, AAFwk::Want &want)
{
    HILOG_DEBUG("GetANRDialogWant start");
    AppExecFwk::ApplicationInfo appInfo;
    bool debug;
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (appScheduler->GetApplicationInfoByProcessID(pid, appInfo, debug) != ERR_OK) {
        HILOG_ERROR("Get application info failed.");
        return false;
    }

    std::string appName {""};
    GetAppNameFromResource(appInfo.labelId, appInfo.bundleName, userId, appName);
    DialogPosition position;
    GetDialogPositionAndSize(DialogType::DIALOG_ANR, position);
    std::string params = GetAnrParams(position, appName);

    want.SetElementName(BUNDLE_NAME_DIALOG, ABILITY_NAME_ANR_DIALOG);
    want.SetParam(BUNDLE_NAME, appInfo.bundleName);
    want.SetParam(DIALOG_POSITION, GetDialogPositionParams(position));
    want.SetParam(DIALOG_PARAMS, params);
    return true;
}

const std::string SystemDialogScheduler::GetAnrParams(const DialogPosition position, const std::string &appName) const
{
    nlohmann::json anrData;
    anrData[APP_NAME] = appName;
    anrData[DEVICE_TYPE] = deviceType_;
    if (!position.wideScreen) {
        anrData[OFF_SET_X] = position.window_offsetX;
        anrData[OFF_SET_Y] = position.window_offsetY;
        anrData[WIDTH] = position.window_width;
        anrData[HEIGHT] = position.window_height;
    }
    return anrData.dump();
}

Want SystemDialogScheduler::GetTipsDialogWant(const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("GetTipsDialogWant start");

    DialogPosition position;
    GetDialogPositionAndSize(DialogType::DIALOG_TIPS, position);

    nlohmann::json jsonObj;
    jsonObj[DEVICE_TYPE] = deviceType_;
    const std::string params = jsonObj.dump();

    AAFwk::Want want;
    want.SetElementName(BUNDLE_NAME_DIALOG, ABILITY_NAME_TIPS_DIALOG);
    want.SetParam(DIALOG_POSITION, GetDialogPositionParams(position));
    want.SetParam(DIALOG_PARAMS, params);
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord && UIExtensionUtils::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        want.RemoveParam(CALLER_TOKEN);
    } else {
        want.SetParam(CALLER_TOKEN, callerToken);
    }
    return want;
}

Want SystemDialogScheduler::GetJumpInterceptorDialogWant(Want &targetWant)
{
    HILOG_DEBUG("GetJumpInterceptorDialogWant start");

    DialogPosition position;
    GetDialogPositionAndSize(DialogType::DIALOG_JUMP_INTERCEPTOR, position);

    nlohmann::json jsonObj;
    jsonObj[DEVICE_TYPE] = deviceType_;
    jsonObj["bundleName"] = targetWant.GetElement().GetBundleName();
    jsonObj["abilityName"] = targetWant.GetElement().GetAbilityName();
    jsonObj["moduleName"] = targetWant.GetElement().GetModuleName();
    const std::string params = jsonObj.dump();

    targetWant.SetElementName(BUNDLE_NAME_DIALOG, ABILITY_NAME_JUMP_INTERCEPTOR_DIALOG);
    targetWant.SetParam(DIALOG_POSITION, GetDialogPositionParams(position));
    targetWant.SetParam(DIALOG_PARAMS, params);
    targetWant.GetStringParam(DIALOG_PARAMS);
    return targetWant;
}

void SystemDialogScheduler::DialogPortraitPositionAdaptive(
    DialogPosition &position, float densityPixels, int lineNums) const
{
    if (lineNums > LINE_NUMS_EIGHT) {
        position.height = static_cast<int32_t>(UI_SELECTOR_PORTRAIT_PHONE_H3 * densityPixels);
        return;
    } else if (lineNums > LINE_NUMS_FOUR) {
        position.height = static_cast<int32_t>(UI_SELECTOR_PORTRAIT_PHONE_H2 * densityPixels);
        return;
    } else if (lineNums > LINE_NUMS_ZERO) {
        position.height = static_cast<int32_t>(UI_SELECTOR_PORTRAIT_PHONE_H1 * densityPixels);
        return;
    }

    HILOG_DEBUG("dialog portrait lineNums is zero.");
}

void SystemDialogScheduler::GetSelectorDialogPortraitPosition(
    DialogPosition &position, int32_t height, int32_t width, int lineNums, float densityPixels) const
{
    HILOG_DEBUG("PortraitPosition height %{public}d width %{public}d density %{public}f.",
        height, width, densityPixels);
    position.width = static_cast<int32_t>(width * UI_SELECTOR_PORTRAIT_WIDTH_RATIO);
    position.height = static_cast<int32_t>(UI_SELECTOR_DIALOG_HEIGHT * densityPixels);
    position.width_narrow = static_cast<int32_t>(width * UI_SELECTOR_PORTRAIT_WIDTH_RATIO);
    position.height_narrow = static_cast<int32_t>(UI_SELECTOR_DIALOG_HEIGHT_NARROW * densityPixels);

    if (width < UI_WIDTH_780DP) {
        HILOG_INFO("show dialog narrow.");
        position.width = position.width_narrow;
        position.height = position.height_narrow;
    }

    DialogPortraitPositionAdaptive(position, densityPixels, lineNums);

    int32_t portraitMax = static_cast<int32_t>(height * UI_SELECTOR_PORTRAIT_HEIGHT_RATIO);
    if (portraitMax < position.height) {
        position.oversizeHeight = true;
        position.height = static_cast<int32_t>(UI_SELECTOR_PORTRAIT_PHONE_H1 * densityPixels);
        HILOG_INFO("portrait ratio 0.9 height is %{public}d.", portraitMax);
    }

    position.offsetX = static_cast<int32_t>(width * UI_SELECTOR_PORTRAIT_WIDTH_EDGE_RATIO);
    position.offsetY = static_cast<int32_t>((height * UI_SELECTOR_PORTRAIT_HEIGHT_RATIO - position.height));
    HILOG_DEBUG("dialog offset x:%{public}d y:%{public}d h:%{public}d w:%{public}d",
        position.offsetX, position.offsetY, position.height, position.width);
}

void SystemDialogScheduler::DialogLandscapePositionAdaptive(
    DialogPosition &position, float densityPixels, int lineNums) const
{
    if (lineNums > LINE_NUMS_EIGHT) {
        position.height = static_cast<int32_t>(UI_SELECTOR_LANDSCAPE_PHONE_H3 * densityPixels);
        return;
    } else if (lineNums > LINE_NUMS_FOUR) {
        position.height = static_cast<int32_t>(UI_SELECTOR_LANDSCAPE_PHONE_H2 * densityPixels);
        return;
    } else if (lineNums > LINE_NUMS_ZERO) {
        position.height = static_cast<int32_t>(UI_SELECTOR_LANDSCAPE_PHONE_H1 * densityPixels);
        return;
    }

    HILOG_DEBUG("dialog landscape lineNums is zero.");
}

void SystemDialogScheduler::GetSelectorDialogLandscapePosition(
    DialogPosition &position, int32_t height, int32_t width, int lineNums, float densityPixels) const
{
    HILOG_DEBUG("LandscapePosition height %{public}d width %{public}d density %{public}f.",
        height, width, densityPixels);
    position.width = static_cast<int32_t>(width *
        (UI_SELECTOR_LANDSCAPE_GRILLE_LARGE * UI_SELECTOR_LANDSCAPE_COUNT_FOUR +
        UI_SELECTOR_LANDSCAPE_GRILLE_SAMLL * UI_SELECTOR_LANDSCAPE_COUNT_THREE));
    position.height = static_cast<int32_t>((UI_SELECTOR_LANDSCAPE_HEIGHT) * densityPixels);
    position.width_narrow = static_cast<int32_t>(width *
        (UI_SELECTOR_LANDSCAPE_GRILLE_LARGE * UI_SELECTOR_LANDSCAPE_COUNT_FOUR +
        UI_SELECTOR_LANDSCAPE_GRILLE_SAMLL * UI_SELECTOR_LANDSCAPE_COUNT_THREE));
    position.height_narrow = static_cast<int32_t>((UI_SELECTOR_LANDSCAPE_HEIGHT_NARROW) * densityPixels);
    DialogLandscapePositionAdaptive(position, densityPixels, lineNums);

    int32_t landscapeMax = static_cast<int32_t>(
        (height - UI_SELECTOR_LANDSCAPE_SIGNAL_BAR * densityPixels) * UI_SELECTOR_LANDSCAPE_MAX_RATIO);
    if (position.height > landscapeMax) {
        position.oversizeHeight = true;
        position.height = static_cast<int32_t>(UI_SELECTOR_LANDSCAPE_PHONE_H1 * densityPixels);
        HILOG_INFO("landscape ratio 0.9 height is %{public}d.", landscapeMax);
    }

    HILOG_DEBUG("dialog height is %{public}d.", position.height);
    position.offsetX = static_cast<int32_t>((width - position.width) / UI_HALF);
    position.offsetY = static_cast<int32_t>((height * UI_SELECTOR_PORTRAIT_HEIGHT_RATIO - position.height));
    HILOG_DEBUG("dialog offset x:%{public}d y:%{public}d h:%{public}d w:%{public}d",
        position.offsetX, position.offsetY, position.height, position.width);
}

void SystemDialogScheduler::GetSelectorDialogPositionAndSize(
    DialogPosition &portraitPosition, DialogPosition &landscapePosition, int lineNums) const
{
    portraitPosition.wideScreen = (deviceType_ != STR_PHONE) && (deviceType_ != STR_DEFAULT);
    portraitPosition.align = ((deviceType_ == STR_PHONE) || (deviceType_ == STR_DEFAULT)) ?
        DialogAlign::BOTTOM : DialogAlign::CENTER;
    landscapePosition.wideScreen = portraitPosition.wideScreen;
    landscapePosition.align = portraitPosition.align;

    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        HILOG_ERROR("share dialog GetDefaultDisplay fail, try again.");
        display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    }
    if (display == nullptr) {
        HILOG_ERROR("share dialog GetDefaultDisplay fail.");
        return;
    }

    auto displayInfo = display->GetDisplayInfo();
    if (displayInfo == nullptr) {
        HILOG_ERROR("share dialog GetDisplayInfo fail.");
        return;
    }

    HILOG_DEBUG("GetDialogPositionAndSize GetOrientation, %{public}d %{public}f",
        displayInfo->GetDisplayOrientation(), display->GetVirtualPixelRatio());
    if (displayInfo->GetDisplayOrientation() == Rosen::DisplayOrientation::PORTRAIT ||
        displayInfo->GetDisplayOrientation() == Rosen::DisplayOrientation::PORTRAIT_INVERTED) {
        HILOG_INFO("GetDialogPositionAndSize GetOrientation, PORTRAIT or PORTRAIT_INVERTED");
        GetSelectorDialogPortraitPosition(portraitPosition, display->GetHeight(), display->GetWidth(),
            lineNums, display->GetVirtualPixelRatio());
        GetSelectorDialogLandscapePosition(landscapePosition, display->GetWidth(), display->GetHeight(),
            lineNums, display->GetVirtualPixelRatio());
        return;
    }

    HILOG_INFO("GetDialogPositionAndSize GetOrientation, LANDSCAPE or LANDSCAPE_INVERTED");
    GetSelectorDialogPortraitPosition(portraitPosition, display->GetWidth(), display->GetHeight(),
        lineNums, display->GetVirtualPixelRatio());
    GetSelectorDialogLandscapePosition(landscapePosition, display->GetHeight(), display->GetWidth(),
        lineNums, display->GetVirtualPixelRatio());
}

Want SystemDialogScheduler::GetSelectorDialogWant(const std::vector<DialogAppInfo> &dialogAppInfos, Want &targetWant,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("GetSelectorDialogWant start");
    DialogPosition portraitPosition;
    DialogPosition landscapePosition;
    GetSelectorDialogPositionAndSize(portraitPosition, landscapePosition, static_cast<int>(dialogAppInfos.size()));
    std::string params = GetSelectorParams(dialogAppInfos);

    targetWant.SetElementName(BUNDLE_NAME_DIALOG, ABILITY_NAME_SELECTOR_DIALOG);
    targetWant.SetParam(DIALOG_POSITION, GetDialogPositionParams(portraitPosition));
    targetWant.SetParam(VERTICAL_SCREEN_DIALOG_POSITION, GetDialogPositionParams(landscapePosition));
    targetWant.SetParam(DIALOG_PARAMS, params);
    if (callerToken != nullptr) {
        HILOG_DEBUG("set callertoken to targetWant");
        auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
        if (abilityRecord && UIExtensionUtils::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
            targetWant.RemoveParam(CALLER_TOKEN);
        } else {
            targetWant.SetParam(CALLER_TOKEN, callerToken);
        }
    }
    if (AppGalleryEnableUtil::IsEnableAppGallerySelector() && Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
        if (bundleMgrHelper == nullptr) {
            HILOG_ERROR("The bundleMgrHelper is nullptr.");
            return targetWant;
        }
        std::string bundleName;
        auto ret = IN_PROCESS_CALL(bundleMgrHelper->QueryAppGalleryBundleName(bundleName));
        if (ret) {
            targetWant.SetElementName(bundleName, ABILITY_NAME_APPGALLERY_SELECTOR_DIALOG);
            targetWant.SetParam(UIEXTENSION_TYPE_KEY, UIEXTENSION_SYS_COMMON_UI);
            return targetWant;
        }
    }
    return targetWant;
}

const std::string SystemDialogScheduler::GetSelectorParams(const std::vector<DialogAppInfo> &infos) const
{
    if (infos.empty()) {
        HILOG_WARN("Invalid abilityInfos.");
        return {};
    }

    nlohmann::json jsonObject;
    jsonObject[DEVICE_TYPE] = deviceType_;

    nlohmann::json hapListObj = nlohmann::json::array();
    for (const auto &aInfo : infos) {
        nlohmann::json aObj;
        aObj["label"] = std::to_string(aInfo.labelId);
        aObj["icon"] = std::to_string(aInfo.iconId);
        aObj["bundle"] = aInfo.bundleName;
        aObj["ability"] = aInfo.abilityName;
        aObj["module"] = aInfo.moduleName;
        hapListObj.emplace_back(aObj);
    }
    jsonObject["hapList"] = hapListObj;

    return jsonObject.dump();
}

Want SystemDialogScheduler::GetPcSelectorDialogWant(const std::vector<DialogAppInfo> &dialogAppInfos,
    Want &targetWant, const std::string &type, int32_t userId, const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("GetPcSelectorDialogWant start");
    DialogPosition position;
    GetDialogPositionAndSize(DialogType::DIALOG_SELECTOR, position, static_cast<int>(dialogAppInfos.size()));

    std::string params = GetPcSelectorParams(dialogAppInfos, type, userId, targetWant.GetAction());
    targetWant.SetElementName(BUNDLE_NAME_DIALOG, ABILITY_NAME_SELECTOR_DIALOG);
    targetWant.SetParam(DIALOG_POSITION, GetDialogPositionParams(position));
    targetWant.SetParam(DIALOG_PARAMS, params);
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord && UIExtensionUtils::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        // SelectorDialog can't bind to the window of UIExtension, so set CALLER_TOKEN to null.
        targetWant.RemoveParam(CALLER_TOKEN);
    } else {
        targetWant.SetParam(CALLER_TOKEN, callerToken);
    }
    if (AppGalleryEnableUtil::IsEnableAppGallerySelector() && Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
        if (bundleMgrHelper == nullptr) {
            HILOG_ERROR("The bundleMgrHelper is nullptr.");
            return targetWant;
        }
        std::string bundleName;
        auto ret = IN_PROCESS_CALL(bundleMgrHelper->QueryAppGalleryBundleName(bundleName));
        if (ret) {
            targetWant.SetElementName(bundleName, ABILITY_NAME_APPGALLERY_SELECTOR_DIALOG);
            targetWant.SetParam(UIEXTENSION_TYPE_KEY, UIEXTENSION_SYS_COMMON_UI);
            return targetWant;
        }
    }
    return targetWant;
}

const std::string SystemDialogScheduler::GetPcSelectorParams(const std::vector<DialogAppInfo> &infos,
    const std::string &type, int32_t userId, const std::string &action) const
{
    HILOG_DEBUG("GetPcSelectorParams start");
    if (infos.empty()) {
        HILOG_WARN("Invalid abilityInfos.");
        return {};
    }

    nlohmann::json jsonObject;
    jsonObject[DEVICE_TYPE] = deviceType_;
    jsonObject[ACTION] = action;
    if (type == TYPE_ONLY_MATCH_WILDCARD) {
        jsonObject[MODEL_FLAG] = true;
    } else {
        jsonObject[MODEL_FLAG] = false;
    }

    nlohmann::json hapListObj = nlohmann::json::array();
    for (const auto &info : infos) {
        nlohmann::json aObj;
        aObj["label"] = std::to_string(info.labelId);
        aObj["icon"] = std::to_string(info.iconId);
        aObj["bundle"] = info.bundleName;
        aObj["ability"] = info.abilityName;
        aObj["module"] = info.moduleName;
        aObj["type"] = type;
        aObj["userId"] = std::to_string(userId);
        hapListObj.emplace_back(aObj);
    }
    jsonObject["hapList"] = hapListObj;

    return jsonObject.dump();
}

const std::string SystemDialogScheduler::GetDialogPositionParams(const DialogPosition position) const
{
    nlohmann::json dialogPositionData;
    dialogPositionData[OFF_SET_X] = position.offsetX;
    dialogPositionData[OFF_SET_Y] = position.offsetY;
    dialogPositionData[WIDTH] = position.width;
    dialogPositionData[HEIGHT] = position.height;
    dialogPositionData[OVERSIZE_HEIGHT] = position.oversizeHeight;
    return dialogPositionData.dump();
}

void SystemDialogScheduler::InitDialogPosition(DialogType type, DialogPosition &position) const
{
    position.wideScreen = (deviceType_ == STR_PHONE) ? false : (deviceType_ != STR_DEFAULT);
    position.align = ((deviceType_ == STR_PHONE) ? true : (deviceType_ == STR_DEFAULT)) ?
        DialogAlign::BOTTOM : DialogAlign::CENTER;
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();

    switch (type) {
        case DialogType::DIALOG_ANR:
            if (position.wideScreen) {
                position.width = UI_ANR_DIALOG_WIDTH;
                position.height = UI_ANR_DIALOG_HEIGHT;
                position.width_narrow = UI_ANR_DIALOG_WIDTH;
                position.height_narrow = UI_ANR_DIALOG_HEIGHT;
                position.align = DialogAlign::CENTER;
            } else {
                position.width =  display->GetWidth();
                position.height = display->GetHeight();
                position.width_narrow =  display->GetWidth();
                position.height_narrow = display->GetHeight();
                position.window_width = UI_ANR_DIALOG_WIDTH;
                position.window_height = UI_ANR_DIALOG_HEIGHT;
                position.align = DialogAlign::CENTER;
            }
            break;
        case DialogType::DIALOG_SELECTOR:
            position.width = UI_SELECTOR_DIALOG_WIDTH;
            position.height = UI_SELECTOR_DIALOG_HEIGHT;
            position.width_narrow = UI_SELECTOR_DIALOG_WIDTH_NARROW;
            position.height_narrow = UI_SELECTOR_DIALOG_HEIGHT_NARROW;
            break;
        case DialogType::DIALOG_TIPS:
            position.width = UI_TIPS_DIALOG_WIDTH;
            position.height = UI_TIPS_DIALOG_HEIGHT;
            position.width_narrow = UI_TIPS_DIALOG_WIDTH_NARROW;
            position.height_narrow = UI_TIPS_DIALOG_HEIGHT_NARROW;
            break;
        case DialogType::DIALOG_JUMP_INTERCEPTOR:
            position.width = UI_JUMP_INTERCEPTOR_DIALOG_WIDTH;
            position.height = UI_JUMP_INTERCEPTOR_DIALOG_HEIGHT;
            position.width_narrow = UI_JUMP_INTERCEPTOR_DIALOG_WIDTH_NARROW;
            position.height_narrow = UI_JUMP_INTERCEPTOR_DIALOG_HEIGHT_NARROW;
            break;
        default:
            position.width = UI_DEFAULT_WIDTH;
            position.height = UI_DEFAULT_HEIGHT;
            position.width_narrow = UI_DEFAULT_WIDTH;
            position.height_narrow = UI_DEFAULT_HEIGHT;
            break;
    }
}

void SystemDialogScheduler::DialogPositionAdaptive(DialogPosition &position, int lineNums) const
{
    if (position.wideScreen) {
        if (lineNums <= LINE_NUMS_TWO) {
            position.height = UI_SELECTOR_DIALOG_PC_H2;
        } else if (lineNums == LINE_NUMS_THREE) {
            position.height = UI_SELECTOR_DIALOG_PC_H3;
        } else if (lineNums == LINE_NUMS_FOUR) {
            position.height = UI_SELECTOR_DIALOG_PC_H4;
        } else if (lineNums > LINE_NUMS_FOUR) {
            position.height = UI_SELECTOR_DIALOG_PC_H5;
        } else {
            position.height = UI_SELECTOR_DIALOG_PC_H0;
        }
    } else {
        position.height = (lineNums > LINE_NUMS_EIGHT) ? UI_SELECTOR_DIALOG_PHONE_H3 :
            (lineNums > LINE_NUMS_THREE ? UI_SELECTOR_DIALOG_PHONE_H2 :
            (lineNums > LINE_NUMS_ZERO ? UI_SELECTOR_DIALOG_PHONE_H1 : position.height));
    }
}

void SystemDialogScheduler::GetDialogPositionAndSize(DialogType type, DialogPosition &position, int lineNums) const
{
    InitDialogPosition(type, position);

    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        HILOG_WARN("share dialog GetDefaultDisplay fail, try again.");
        display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    }
    if (display != nullptr) {
        HILOG_INFO("display width: %{public}d, height: %{public}d", display->GetWidth(), display->GetHeight());
        if (display->GetWidth() < UI_WIDTH_780DP) {
            HILOG_INFO("show dialog narrow.");
            position.width = position.width_narrow;
            position.height = position.height_narrow;
        }

        if (type == DialogType::DIALOG_SELECTOR) {
            DialogPositionAdaptive(position, lineNums);
        }
        switch (position.align) {
            case DialogAlign::CENTER:
                if (position.wideScreen) {
                    position.offsetX = (display->GetWidth() - position.width) / UI_HALF;
                    position.offsetY = (display->GetHeight() - position.height) / UI_HALF;
                } else {
                    position.window_width = position.window_width / UI_HALF;
                    position.window_height = position.window_height / UI_HALF;
                    position.offsetX = LINE_NUMS_ZERO;
                    position.offsetY = LINE_NUMS_ZERO;
                }
                break;
            case DialogAlign::BOTTOM:
                position.width = display->GetWidth() * WIDTH_MULTIPLE;
                position.height = display->GetHeight() * HEIGHT_MULTIPLE;
                position.offsetX = display->GetWidth() * SETX_WIDTH_MULTIPLE;
                position.offsetY = display->GetHeight() - position.height - UI_DEFAULT_BUTTOM_CLIP;
                break;
            default:
                position.offsetX = (display->GetWidth() - position.width) / UI_HALF;
                position.offsetY = (display->GetHeight() - position.height - UI_DEFAULT_BUTTOM_CLIP) / UI_HALF;
                break;
        }
    } else {
        HILOG_WARN("share dialog get display fail, use default wide.");
        if (type == DialogType::DIALOG_SELECTOR) {
            DialogPositionAdaptive(position, lineNums);
        }
        position.offsetX = (UI_DEFAULT_WIDTH - position.width) / UI_HALF;
        position.offsetY = UI_DEFAULT_HEIGHT - position.height - UI_DEFAULT_BUTTOM_CLIP;
    }
}

void SystemDialogScheduler::GetAppNameFromResource(int32_t labelId,
    const std::string &bundleName, int32_t userId, std::string &appName)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    if (resourceManager == nullptr) {
        HILOG_ERROR("The resourceManager is nullptr.");
        return;
    }

    AppExecFwk::BundleInfo bundleInfo;
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    CHECK_POINTER(bundleMgrHelper);
    if (!IN_PROCESS_CALL(
        bundleMgrHelper->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId))) {
        HILOG_ERROR("Failed to get bundle info.");
        return;
    }
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(Global::I18n::LocaleConfig::GetSystemLanguage(), status);
    resConfig->SetLocaleInfo(locale);
    resourceManager->UpdateResConfig(*resConfig);

    std::regex pattern(std::string(AbilityBase::Constants::ABS_CODE_PATH) +
        std::string(AbilityBase::Constants::FILE_SEPARATOR) + bundleInfo.name);
    for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
        std::string loadPath;
        HILOG_DEBUG("make a judgment.");
        if (!hapModuleInfo.hapPath.empty()) {
            loadPath = hapModuleInfo.hapPath;
        } else {
            loadPath = hapModuleInfo.resourcePath;
        }
        if (loadPath.empty()) {
            continue;
        }
        HILOG_DEBUG("GetAppNameFromResource loadPath: %{public}s.", loadPath.c_str());
        if (!resourceManager->AddResource(loadPath.c_str())) {
            HILOG_ERROR("ResourceManager add %{public}s resource path failed.", bundleInfo.name.c_str());
        }
    }
    resourceManager->GetStringById(static_cast<uint32_t>(labelId), appName);
    HILOG_DEBUG("Get app display info, labelId: %{public}d, appname: %{public}s.", labelId, appName.c_str());
}

Want SystemDialogScheduler::GetSwitchUserDialogWant()
{
    HILOG_DEBUG("start");
    AAFwk::Want dialogWant;
    dialogWant.SetElementName(BUNDLE_NAME_DIALOG, ABILITY_NAME_FREEZE_DIALOG);

    return dialogWant;
}
}  // namespace AAFwk
}  // namespace OHOS
