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
const int32_t UI_ANR_DIALOG_WIDTH = 328 * 2;
const int32_t UI_ANR_DIALOG_HEIGHT = 192 * 2;
const std::string EVENT_WAITING_CODE = "0";
const std::string EVENT_CLOSE_CODE = "1";
const std::string APP_NAME = "appName";

const int32_t UI_HALF = 2;
const int32_t UI_DEFAULT_BUTTOM_CLIP = 100;
const int32_t UI_WIDTH_780DP = 1560;
const int32_t UI_DEFAULT_WIDTH = 2560;
const int32_t UI_DEFAULT_HEIGHT = 1600;

const std::string STR_PHONE = "phone";
const std::string STR_PC = "pc";
const std::string DIALOG_NAME_ANR = "dialog_anr_service";

SystemDialogScheduler::SystemDialogScheduler(const std::string &deviceType): deviceType_(deviceType) {}

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

void SystemDialogScheduler::InitDialogPosition(DialogType type, DialogPosition &position) const
{
    position.wideScreen = (deviceType_ == STR_PC);
    position.align = (deviceType_ == STR_PHONE) ? DialogAlign::BOTTOM : DialogAlign::CENTER;

    switch (type) {
        case DialogType::DIALOG_ANR:
            position.width = UI_ANR_DIALOG_WIDTH;
            position.height = UI_ANR_DIALOG_HEIGHT;
            position.width_narrow = UI_ANR_DIALOG_WIDTH;
            position.height_narrow = UI_ANR_DIALOG_HEIGHT;
            position.align = DialogAlign::CENTER;
            break;
        default:
            position.width = UI_DEFAULT_WIDTH;
            position.height = UI_DEFAULT_HEIGHT;
            position.width_narrow = UI_DEFAULT_WIDTH;
            position.height_narrow = UI_DEFAULT_HEIGHT;
            break;
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
        if (display->GetWidth() < UI_WIDTH_780DP) {
            HILOG_INFO("show dialog narrow.");
            position.width = position.width_narrow;
            position.height = position.height_narrow;
        }
        switch (position.align) {
            case DialogAlign::CENTER:
                position.offsetX = (display->GetWidth() - position.width) / UI_HALF;
                position.offsetY = (display->GetHeight() - position.height - UI_DEFAULT_BUTTOM_CLIP) / UI_HALF;
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