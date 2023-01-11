/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <regex>

#include "constants.h"
#include "ability_util.h"
#include "app_scheduler.h"
#include "display_manager.h"
#include "errors.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "locale_config.h"
#include "parameters.h"
#include "resource_manager.h"
#include "ui_service_mgr_client.h"

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
const int32_t UI_SELECTOR_DIALOG_PC_H2 = (70 * 2 + 85 + 2) * 2;
const int32_t UI_SELECTOR_DIALOG_PC_H3 = (70 * 3 + 85 + 2) * 2;
const int32_t UI_SELECTOR_DIALOG_PC_H4 = (70 * 4 + 85 + 2) * 2;
const int32_t UI_SELECTOR_DIALOG_PC_H5 = (70 * 4 + 85 + 38) * 2;

const int32_t UI_TIPS_DIALOG_WIDTH = 328 * 2;
const int32_t UI_TIPS_DIALOG_HEIGHT = 135 * 2;
const int32_t UI_TIPS_DIALOG_HEIGHT_NARROW = 135 * 2;
const int32_t UI_TIPS_DIALOG_WIDTH_NARROW = 328 * 2;

const int32_t UI_ANR_DIALOG_WIDTH = 328 * 2;
const int32_t UI_ANR_DIALOG_HEIGHT = 192 * 2;
const std::string APP_NAME = "appName";
const std::string DEVICE_TYPE = "deviceType";
const std::string OFF_SET_X = "offsetX";
const std::string OFF_SET_Y = "offsetY";
const std::string WIDTH = "width";
const std::string HEIGHT = "height";

const int32_t UI_HALF = 2;
const int32_t UI_DEFAULT_BUTTOM_CLIP = 100;
const int32_t UI_WIDTH_780DP = 1560;
const int32_t UI_DEFAULT_WIDTH = 2560;
const int32_t UI_DEFAULT_HEIGHT = 1600;

const std::string STR_PHONE = "phone";
const std::string STR_PC = "pc";
const std::string DIALOG_NAME_ANR = "dialog_anr_service";
const std::string DIALOG_NAME_TIPS = "dialog_tips_service";
const std::string DIALOG_SELECTOR_NAME = "dialog_selector_service";

const std::string BUNDLE_NAME = "bundleName";
const std::string BUNDLE_NAME_DIALOG = "com.acts.ohos.amsdialog1";
const std::string DIALOG_PARAMS = "params";
const std::string DIALOG_POSITION = "position";
const std::string ABILITY_NAME_ANR_DIALOG = "AnrDialog";
const std::string ABILITY_NAME_TIPS_DIALOG = "TipsDialog";
const std::string ABILITY_NAME_SELECTOR_DIALOG = "SelectorDialog";

const int32_t LINE_NUMS_ZERO = 0;
const int32_t LINE_NUMS_TWO = 2;
const int32_t LINE_NUMS_THREE = 3;
const int32_t LINE_NUMS_FOUR = 4;
const int32_t LINE_NUMS_EIGHT = 8;

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

Want SystemDialogScheduler::GetTipsDialogWant()
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
    return want;
}

Want SystemDialogScheduler::GetSelectorDialogWant(const std::vector<DialogAppInfo> &dialogAppInfos, Want &targetWant)
{
    HILOG_DEBUG("GetSelectorDialogWant start");
    DialogPosition position;
    GetDialogPositionAndSize(DialogType::DIALOG_SELECTOR, position, static_cast<int>(dialogAppInfos.size()));
    std::string params = GetSelectorParams(dialogAppInfos);

    targetWant.SetElementName(BUNDLE_NAME_DIALOG, ABILITY_NAME_SELECTOR_DIALOG);
    targetWant.SetParam(DIALOG_POSITION, GetDialogPositionParams(position));
    targetWant.SetParam(DIALOG_PARAMS, params);

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

const std::string SystemDialogScheduler::GetDialogPositionParams(const DialogPosition position) const
{
    nlohmann::json dialogPositionData;
    dialogPositionData[OFF_SET_X] = position.offsetX;
    dialogPositionData[OFF_SET_Y] = position.offsetY;
    dialogPositionData[WIDTH] = position.width;
    dialogPositionData[HEIGHT] = position.height;
    return dialogPositionData.dump();
}

void SystemDialogScheduler::InitDialogPosition(DialogType type, DialogPosition &position) const
{
    position.wideScreen = (deviceType_ == STR_PC);
    position.align = (deviceType_ == STR_PHONE) ? DialogAlign::BOTTOM : DialogAlign::CENTER;
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
        if (lineNums == LINE_NUMS_TWO) {
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
                position.offsetX = (display->GetWidth() - position.width) / UI_HALF;
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
        HILOG_ERROR("resourceManager init failed!");
        return;
    }

    AppExecFwk::BundleInfo bundleInfo;
    auto bms = GetBundleManager();
    CHECK_POINTER(bms);
    if (!IN_PROCESS_CALL(
        bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId))) {
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
        if (!hapModuleInfo.hapPath.empty()) {
            loadPath = hapModuleInfo.hapPath;
        } else {
            loadPath = hapModuleInfo.resourcePath;
        }
        if (loadPath.empty()) {
            continue;
        }
        HILOG_DEBUG("GetAppNameFromResource loadPath: %{public}s", loadPath.c_str());
        if (!resourceManager->AddResource(loadPath.c_str())) {
            HILOG_ERROR("ResourceManager add %{public}s resource path failed!", bundleInfo.name.c_str());
        }
    }
    resourceManager->GetStringById(static_cast<uint32_t>(labelId), appName);
    HILOG_DEBUG("Get app display info, labelId: %{public}d, appname: %{public}s", labelId, appName.c_str());
}

sptr<AppExecFwk::IBundleMgr> SystemDialogScheduler::GetBundleManager()
{
    if (iBundleManager_ == nullptr) {
        auto bundleObj =
            OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (bundleObj == nullptr) {
            HILOG_ERROR("Failed to get bundle manager service.");
            return nullptr;
        }
        iBundleManager_ = iface_cast<AppExecFwk::IBundleMgr>(bundleObj);
    }
    return iBundleManager_;
}
}  // namespace AAFwk
}  // namespace OHOS
