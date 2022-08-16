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

#include <csignal>

#include "ability_util.h"
#include "display_manager.h"
#include "errors.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "locale_config.h"
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
const std::string EVENT_WAITING_CODE = "0";
const std::string EVENT_CLOSE_CODE = "1";
const std::string APP_NAME = "appName";
const std::string DEVICE_TYPE = "deviceType";

const int32_t UI_HALF = 2;
const int32_t UI_DEFAULT_BUTTOM_CLIP = 100;
const int32_t UI_WIDTH_780DP = 1560;
const int32_t UI_DEFAULT_WIDTH = 2560;
const int32_t UI_DEFAULT_HEIGHT = 1600;
const std::string EVENT_CLOSE = "EVENT_CLOSE";
const std::string EVENT_CHOOSE_APP = "EVENT_CHOOSE_APP";

const std::string STR_PHONE = "phone";
const std::string STR_PC = "pc";
const std::string DIALOG_NAME_ANR = "dialog_anr_service";
const std::string DIALOG_NAME_TIPS = "dialog_tips_service";
const std::string DIALOG_SELECTOR_NAME = "dialog_selector_service";
const std::string OFF_SET_X = "offsetX";
const std::string OFF_SET_Y = "offsetY";
const std::string WIDTH = "width";
const std::string HEIGHT = "height";

const int32_t LINE_NUMS_ZERO = 0;
const int32_t LINE_NUMS_TWO = 2;
const int32_t LINE_NUMS_THREE = 3;
const int32_t LINE_NUMS_FOUR = 4;
const int32_t LINE_NUMS_EIGHT = 8;

void SystemDialogScheduler::ScheduleShowDialog(const std::string &name, const DialogPosition &position,
    const std::string &params, DialogCallback callback) const
{
    if (name.empty()) {
        HILOG_ERROR("dialog name is empty.");
        return;
    }

    HILOG_INFO("Show Dialog:[%{public}s],Dialog position:[%{public}d,%{public}d,%{public}d,%{public}d],str:%{public}s",
        name.data(), position.offsetX, position.offsetY, position.width, position.height, params.data());

    Ace::UIServiceMgrClient::GetInstance()->ShowDialog(
        name,
        params,
        OHOS::Rosen::WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW,
        position.offsetX, position.offsetY, position.width, position.height,
        callback);

    HILOG_INFO("Show UI Dialog finished.");
}

int32_t SystemDialogScheduler::ShowANRDialog(const std::string &appName, const Closure &anrCallBack)
{
    HILOG_DEBUG("ShowAnrDialog start");

    DialogPosition position;
    GetDialogPositionAndSize(DialogType::DIALOG_ANR, position);

    nlohmann::json jsonObj;
    jsonObj[APP_NAME] = appName;
    const std::string params = jsonObj.dump();

    auto callback = [anrCallBack] (int32_t id, const std::string& event, const std::string& params) {
        HILOG_INFO("Dialog anr callback: id : %{public}d, event: %{public}s, params: %{public}s",
            id, event.data(), params.data());

        Ace::UIServiceMgrClient::GetInstance()->CancelDialog(id);

        if (params == EVENT_WAITING_CODE) {
            HILOG_WARN("user choose to wait no response app.");
            return;
        }
        if (params == EVENT_CLOSE_CODE) {
            HILOG_WARN("user choose to kill no response app.");
            anrCallBack();
        }
    };

    ScheduleShowDialog(DIALOG_NAME_ANR, position, params, callback);

    HILOG_DEBUG("ShowAnrDialog end");
    return ERR_OK;
}

int32_t SystemDialogScheduler::ShowTipsDialog()
{
    HILOG_DEBUG("ShowTipsDialog start");
    
    DialogPosition position;
    GetDialogPositionAndSize(DialogType::DIALOG_TIPS, position);
    std::string params = GetTipParams(position);

    auto callback = [] (int32_t id, const std::string& event, const std::string& params) {
        HILOG_INFO("Dialog tips callback: id : %{public}d, event: %{public}s, params: %{public}s",
            id, event.data(), params.data());
        Ace::UIServiceMgrClient::GetInstance()->CancelDialog(id);
        if (event == EVENT_CLOSE) {
            HILOG_WARN("the user abandoned implicit start ability.");
        }
    };

    ScheduleShowDialog(DIALOG_NAME_TIPS, position, params, callback);

    HILOG_DEBUG("ShowTipsDialog end");
    return ERR_OK;
}

const std::string SystemDialogScheduler::GetTipParams(DialogPosition position) const
{
    nlohmann::json tipData;
    if (!position.wideScreen) {
        auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
        tipData[OFF_SET_X] = position.window_offsetX;
        tipData[OFF_SET_Y] = position.window_offsetY;
        tipData[WIDTH] = position.window_width;
        tipData[HEIGHT] = position.window_height;
    }
    tipData[DEVICE_TYPE] = deviceType_;
    return tipData.dump();
}

int32_t SystemDialogScheduler::ShowSelectorDialog(
    const std::vector<DialogAppInfo> &infos, const SelectorClosure &startAbilityCallBack)
{
    HILOG_DEBUG("ShowSelectorDialog start");
    if (infos.empty()) {
        HILOG_WARN("Invalid abilityInfos.");
        return ERR_INVALID_VALUE;
    }

    DialogPosition position;
    GetDialogPositionAndSize(DialogType::DIALOG_SELECTOR, position, static_cast<int>(infos.size()));

    std::string params = GetSelectorParams(infos, position);

    auto callback = [startAbilityCallBack] (int32_t id, const std::string& event, const std::string& params) {
        HILOG_INFO("Dialog selector callback: id : %{public}d, event: %{public}s, params: %{public}s",
            id, event.data(), params.data());
        Ace::UIServiceMgrClient::GetInstance()->CancelDialog(id);

        if (event == EVENT_CLOSE) {
            HILOG_WARN("the user abandoned implicit start ability.");
            return;
        }
        if (event == EVENT_CHOOSE_APP) {
            std::string bundleName {""};
            std::string abilityName {""};
            auto pos = params.find(";");
            if (pos != std::string::npos) {
                bundleName = params.substr(0, pos);
                abilityName = params.substr(pos + 1, params.length() - (pos + 1));
                HILOG_INFO("dialog callback, bundle:%{public}s, ability:%{public}s",
                    bundleName.c_str(), abilityName.c_str());
            }
            if (!bundleName.empty() && !abilityName.empty()) {
                IN_PROCESS_CALL_WITHOUT_RET(startAbilityCallBack(bundleName, abilityName));
            }
        }
    };

    ScheduleShowDialog(DIALOG_SELECTOR_NAME, position, params, callback);

    HILOG_DEBUG("ShowSelectorDialog end");
    return ERR_OK;
}

const std::string SystemDialogScheduler::GetSelectorParams(const std::vector<DialogAppInfo> &infos,
    DialogPosition position) const
{
    if (infos.empty()) {
        HILOG_WARN("Invalid abilityInfos.");
        return {};
    }

    nlohmann::json jsonObject;
    if(!position.wideScreen) {
        auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
        jsonObject[OFF_SET_X] = position.window_offsetX;
        jsonObject[OFF_SET_Y] = position.window_offsetY;
        jsonObject[WIDTH] = position.window_width;
        jsonObject[HEIGHT] = position.window_height;
    }
    jsonObject[DEVICE_TYPE] = deviceType_;

    nlohmann::json hapListObj = nlohmann::json::array();
    for (auto &aInfo : infos) {
        nlohmann::json aObj;
        aObj["name"] = std::to_string(aInfo.labelId);
        aObj["icon"] = std::to_string(aInfo.iconId);
        aObj["bundle"] = aInfo.bundleName;
        aObj["ability"] = aInfo.abilityName;
        hapListObj.emplace_back(aObj);
    }
    jsonObject["hapList"] = hapListObj;

    return jsonObject.dump();
}

void SystemDialogScheduler::InitDialogPosition(DialogType type, DialogPosition &position) const
{
    position.wideScreen = (deviceType_ == STR_PC);
    position.align = (deviceType_ == STR_PHONE) ? DialogAlign::BOTTOM : DialogAlign::CENTER;
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();

    switch (type) {
        case DialogType::DIALOG_ANR:
            position.width = UI_ANR_DIALOG_WIDTH;
            position.height = UI_ANR_DIALOG_HEIGHT;
            position.width_narrow = UI_ANR_DIALOG_WIDTH;
            position.height_narrow = UI_ANR_DIALOG_HEIGHT;
            position.align = DialogAlign::CENTER;
            break;
        case DialogType::DIALOG_SELECTOR:
            if (position.wideScreen) {
                position.width = UI_SELECTOR_DIALOG_WIDTH;
                position.height = UI_SELECTOR_DIALOG_HEIGHT;
                position.width_narrow = UI_SELECTOR_DIALOG_WIDTH_NARROW;
                position.height_narrow = UI_SELECTOR_DIALOG_HEIGHT_NARROW;
            } else {
                position.width = display->GetWidth();
                position.height = display->GetHeight();
                position.width_narrow = display->GetWidth();
                position.height_narrow = display->GetHeight();
                position.window_width = UI_SELECTOR_DIALOG_WIDTH/UI_HALF;
                position.window_height = UI_SELECTOR_DIALOG_HEIGHT;
            }
            break;
        case DialogType::DIALOG_TIPS:
            if (position.wideScreen) {
                position.width = UI_TIPS_DIALOG_WIDTH;
                position.height = UI_TIPS_DIALOG_HEIGHT;
                position.width_narrow = UI_TIPS_DIALOG_WIDTH_NARROW;
                position.height_narrow = UI_TIPS_DIALOG_HEIGHT_NARROW;
            } else {
                position.width = display->GetWidth();
                position.height = display->GetHeight();
                position.width_narrow = display->GetWidth();
                position.height_narrow = display->GetHeight();
                position.window_width = UI_TIPS_DIALOG_WIDTH/UI_HALF;
                position.window_height = UI_TIPS_DIALOG_HEIGHT;
            }
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
        position.window_height = (lineNums > LINE_NUMS_EIGHT) ? UI_SELECTOR_DIALOG_PHONE_H3 :
            (lineNums > LINE_NUMS_THREE ? UI_SELECTOR_DIALOG_PHONE_H2 :
            (lineNums > LINE_NUMS_ZERO ? UI_SELECTOR_DIALOG_PHONE_H1 : position.window_height));
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
                position.offsetX = (display->GetWidth() - position.width) / UI_HALF;
                position.offsetY = (display->GetHeight() - position.height - UI_DEFAULT_BUTTOM_CLIP) / UI_HALF;
                break;
            case DialogAlign::BOTTOM:
                position.window_offsetX = LINE_NUMS_ZERO;
                position.window_offsetY = (display->GetHeight() - position.window_height) / UI_HALF - UI_DEFAULT_BUTTOM_CLIP;
                position.window_height =  position.window_height/2;
                position.offsetX = LINE_NUMS_ZERO;
                position.offsetY = LINE_NUMS_ZERO;
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

    for (auto resPath = bundleInfo.moduleResPaths.begin(); resPath != bundleInfo.moduleResPaths.end(); resPath++) {
        if (resPath->empty()) {
            continue;
        }
        if (!resourceManager->AddResource(resPath->c_str())) {
            HILOG_INFO("resourceManager add %{public}s resource path failed!", bundleInfo.name.c_str());
        }
    }
    resourceManager->GetStringById(static_cast<uint32_t>(labelId), appName);
    HILOG_INFO("get app display info, labelId: %{public}d, appname: %{public}s", labelId, appName.c_str());
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
