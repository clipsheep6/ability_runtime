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
#include "implicit_start_processor.h"

#include "ability_manager_service.h"
#include "ability_util.h"
#include "errors.h"
#include "event_report.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"

namespace OHOS {
namespace AAFwk {
const std::string BLACK_ACTION_SELECT_DATA = "ohos.want.action.select";

const std::vector<std::string> ImplicitStartProcessor::blackList = {
    std::vector<std::string>::value_type(BLACK_ACTION_SELECT_DATA),
};

bool ImplicitStartProcessor::IsImplicitStartAction(const Want &want)
{
    auto element = want.GetElement();
    if (!element.GetAbilityName().empty()) {
        return false;
    }
    
    if (!want.GetAction().empty() &&
        std::find(blackList.begin(), blackList.end(), want.GetAction()) == blackList.end()) {
        HILOG_INFO("implicit start, the action is %{public}s", want.GetAction().data());
        return true;
    }

    return false;
}

int ImplicitStartProcessor::ImplicitStartAbility(AbilityRequest &request, int32_t userId)
{
    HILOG_INFO("implicit start ability by type: %{public}d", request.callType);

    auto sysDialogScheduler = DelayedSingleton<SystemDialogScheduler>::GetInstance();
    CHECK_POINTER_AND_RETURN(sysDialogScheduler, ERR_INVALID_VALUE);

    std::vector<DialogAppInfo> dialogAppInfos;
    auto ret = GenerateAbilityRequestByAction(userId, request, dialogAppInfos);
    if (ret != ERR_OK) {
        HILOG_ERROR("generate ability request by action failed.");
        return ret;
    }

     auto startAbilityTask = [imp = shared_from_this(), request, userId,
        identity = IPCSkeleton::ResetCallingIdentity()](const std::string& bundle, const std::string& abilityName) {
        HILOG_INFO("implicit start ability call back.");

        auto oldIdentity = identity;
        // reset calling indentity
        IPCSkeleton::SetCallingIdentity(oldIdentity);

        AAFwk::Want targetWant = request.want;
        targetWant.SetElementName(bundle, abilityName);
        auto callBack = [imp, targetWant, request, userId]() -> int32_t {
            return imp->ImplicitStartAbilityInner(targetWant, request, userId);
        };
        return imp->CallStartAbilityInner(userId, targetWant, callBack, request.callType);
    };
    if (dialogAppInfos.size() == 0) {
        HILOG_ERROR("implicit query ability infos failed, show tips dialog.");
        Want want = sysDialogScheduler->ShowTipsDialog();
        auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
        abilityMgr->StartAbility(want);
        return ERR_IMPLICIT_START_ABILITY_FAIL;
    }

    if (dialogAppInfos.size() == 1) {
        auto info = dialogAppInfos.front();
        HILOG_INFO("ImplicitQueryInfos success, target ability: %{public}s", info.abilityName.data());
        return IN_PROCESS_CALL(startAbilityTask(info.bundleName, info.abilityName));
    }

    HILOG_INFO("ImplicitQueryInfos success, Multiple apps to choose.");
    Want want = sysDialogScheduler->ShowSelectorDialog(dialogAppInfos);
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    return abilityMgr->StartAbility(want);
}

int ImplicitStartProcessor::GenerateAbilityRequestByAction(int32_t userId,
    AbilityRequest &request, std::vector<DialogAppInfo> &dialogAppInfos)
{
    HILOG_DEBUG("%{public}s", __func__);
    // get abilityinfos from bms
    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);
    auto abilityInfoFlag = AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT;
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    IN_PROCESS_CALL_WITHOUT_RET(bms->ImplicitQueryInfos(
        request.want, abilityInfoFlag, userId, abilityInfos, extensionInfos));

    HILOG_INFO("ImplicitQueryInfos, abilityInfo size : %{public}zu, extensionInfos size: %{public}zu",
        abilityInfos.size(), extensionInfos.size());

    auto isExtension = request.callType == AbilityCallType::START_EXTENSION_TYPE;
    
    for (auto &info : abilityInfos) {
        if (isExtension && info.type != AbilityType::EXTENSION) {
            continue;
        }
        DialogAppInfo dialogAppInfo;
        dialogAppInfo.abilityName = info.name;
        dialogAppInfo.bundleName = info.bundleName;
        dialogAppInfo.iconId = info.iconId;
        dialogAppInfo.labelId = info.labelId;
        dialogAppInfos.emplace_back(dialogAppInfo);
    }
    
    for (auto &info : extensionInfos) {
        if (request.callType == AbilityCallType::START_OPTIONS_TYPE ||
            request.callType == AbilityCallType::START_SETTINGS_TYPE) {
            continue;
        }
        DialogAppInfo dialogAppInfo;
        dialogAppInfo.abilityName = info.name;
        dialogAppInfo.bundleName = info.bundleName;
        dialogAppInfo.iconId = info.iconId;
        dialogAppInfo.labelId = info.labelId;
        dialogAppInfos.emplace_back(dialogAppInfo);
    }

    return ERR_OK;
}

int32_t ImplicitStartProcessor::ImplicitStartAbilityInner(const Want &targetWant,
    const AbilityRequest &request, int32_t userId)
{
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER_AND_RETURN(abilityMgr, ERR_INVALID_VALUE);

    int32_t result = ERR_OK;
    switch (request.callType) {
        case AbilityCallType::START_OPTIONS_TYPE: {
            StartOptions startOptions;
            auto displayId = targetWant.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, 0);
            auto windowMode = targetWant.GetIntParam(Want::PARAM_RESV_WINDOW_MODE, 0);
            startOptions.SetDisplayID(static_cast<int32_t>(displayId));
            startOptions.SetWindowMode(static_cast<int32_t>(windowMode));
            result = abilityMgr->StartAbility(
                targetWant, startOptions, request.callerToken, userId, request.requestCode);
            break;
        }
        case AbilityCallType::START_SETTINGS_TYPE: {
            CHECK_POINTER_AND_RETURN(request.startSetting, ERR_INVALID_VALUE);
            result = abilityMgr->StartAbility(
                targetWant, *request.startSetting, request.callerToken, userId, request.requestCode);
            break;
        }
        case AbilityCallType::START_EXTENSION_TYPE:
            result = abilityMgr->StartExtensionAbility(
                targetWant, request.callerToken, userId, request.extensionType);
            break;
        default:
            result = abilityMgr->StartAbilityInner(
                targetWant, request.callerToken, request.requestCode, request.callerUid, userId);
            break;
    }

    return result;
}

int ImplicitStartProcessor::CallStartAbilityInner(int32_t userId,
    const Want &want, const StartAbilityClosure &callBack, const AbilityCallType &callType)
{
    AAFWK::EventInfo eventInfo;
    eventInfo.userId = userId;
    eventInfo.bundleName = want.GetElement().GetBundleName();
    eventInfo.moduleName = want.GetElement().GetModuleName();
    eventInfo.abilityName = want.GetElement().GetAbilityName();

    if (callType == AbilityCallType::INVALID_TYPE) {
        AAFWK::EventReport::SendAbilityEvent(AAFWK::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    }

    HILOG_INFO("ability:%{public}s, bundle:%{public}s", eventInfo.abilityName.c_str(), eventInfo.bundleName.c_str());

    auto ret = callBack();
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        if (callType == AbilityCallType::INVALID_TYPE) {
            AAFWK::EventReport::SendAbilityEvent(AAFWK::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        }
    }
    return ret;
}

sptr<AppExecFwk::IBundleMgr> ImplicitStartProcessor::GetBundleManager()
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