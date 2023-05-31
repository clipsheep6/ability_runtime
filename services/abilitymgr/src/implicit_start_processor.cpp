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
#include "implicit_start_processor.h"

#include <string>

#include "ability_manager_service.h"
#include "ability_util.h"
#include "default_app_interface.h"
#include "errors.h"
#include "event_report.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "parameters.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
using ErmsCallerInfo = OHOS::AppExecFwk::ErmsParams::CallerInfo;

const std::string BLACK_ACTION_SELECT_DATA = "ohos.want.action.select";
const std::string STR_PHONE = "phone";
const std::string STR_DEFAULT = "default";
const std::string TYPE_ONLY_MATCH_WILDCARD = "reserved/wildcard";
const std::string SHOW_DEFAULT_PICKER_FLAG = "ohos.ability.params.showDefaultPicker";

const std::vector<std::string> ImplicitStartProcessor::blackList = {
    std::vector<std::string>::value_type(BLACK_ACTION_SELECT_DATA),
};

const std::unordered_set<AppExecFwk::ExtensionAbilityType> ImplicitStartProcessor::extensionWhiteList = {
    AppExecFwk::ExtensionAbilityType::FORM,
    AppExecFwk::ExtensionAbilityType::INPUTMETHOD,
    AppExecFwk::ExtensionAbilityType::WALLPAPER,
    AppExecFwk::ExtensionAbilityType::WINDOW,
    AppExecFwk::ExtensionAbilityType::THUMBNAIL,
    AppExecFwk::ExtensionAbilityType::PREVIEW
};

bool ImplicitStartProcessor::IsImplicitStartAction(const Want &want)
{
    auto element = want.GetElement();
    if (!element.GetAbilityName().empty()) {
        return false;
    }

    if (std::find(blackList.begin(), blackList.end(), want.GetAction()) == blackList.end()) {
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
    auto deviceType = OHOS::system::GetDeviceType();
    HILOG_DEBUG("deviceType is %{public}s", deviceType.c_str());
    auto ret = GenerateAbilityRequestByAction(userId, request, dialogAppInfos, deviceType, false);
    if (ret != ERR_OK) {
        HILOG_ERROR("generate ability request by action failed.");
        return ret;
    }

    auto identity = IPCSkeleton::ResetCallingIdentity();
    auto startAbilityTask = [imp = shared_from_this(), request, userId, identity]
        (const std::string& bundle, const std::string& abilityName) mutable {
        HILOG_INFO("implicit start ability call back.");

        // reset calling indentity
        IPCSkeleton::SetCallingIdentity(identity);

        AAFwk::Want targetWant = request.want;
        targetWant.SetElementName(bundle, abilityName);
        auto callBack = [imp, targetWant, request, userId]() -> int32_t {
            return imp->ImplicitStartAbilityInner(targetWant, request, userId);
        };
        return imp->CallStartAbilityInner(userId, targetWant, callBack, request.callType);
    };

    AAFwk::Want want;
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (dialogAppInfos.size() == 0 && (deviceType == STR_PHONE || deviceType == STR_DEFAULT)) {
        HILOG_ERROR("implicit query ability infos failed, show tips dialog.");
        want = sysDialogScheduler->GetTipsDialogWant(request.callerToken);
        abilityMgr->StartAbility(want);
        return ERR_IMPLICIT_START_ABILITY_FAIL;
    } else if (dialogAppInfos.size() == 0 && deviceType != STR_PHONE && deviceType != STR_DEFAULT) {
        std::vector<DialogAppInfo> dialogAllAppInfos;
        bool isMoreHapList = true;
        ret = GenerateAbilityRequestByAction(userId, request, dialogAllAppInfos, deviceType, isMoreHapList);
        if (ret != ERR_OK) {
            HILOG_ERROR("generate ability request by action failed.");
            return ret;
        }
        if (dialogAllAppInfos.size() == 0) {
            Want dialogWant = sysDialogScheduler->GetTipsDialogWant(request.callerToken);
            abilityMgr->StartAbility(dialogWant);
            return ERR_IMPLICIT_START_ABILITY_FAIL;
        }
        want = sysDialogScheduler->GetPcSelectorDialogWant(dialogAllAppInfos, request.want,
            TYPE_ONLY_MATCH_WILDCARD, userId, request.callerToken);
        ret = abilityMgr->StartAbility(want, request.callerToken);
        // reset calling indentity
        IPCSkeleton::SetCallingIdentity(identity);
        return ret;
    }

    //There is a default opening method or Only one application supports
    if (dialogAppInfos.size() == 1) {
        auto info = dialogAppInfos.front();
        HILOG_INFO("ImplicitQueryInfos success, target ability: %{public}s", info.abilityName.data());
        return IN_PROCESS_CALL(startAbilityTask(info.bundleName, info.abilityName));
    }

    if (deviceType == STR_PHONE || deviceType == STR_DEFAULT) {
        HILOG_INFO("ImplicitQueryInfos success, Multiple apps to choose.");
        want = sysDialogScheduler->GetSelectorDialogWant(dialogAppInfos, request.want, request.callerToken);
        ret = abilityMgr->StartAbility(want, request.callerToken);
        // reset calling indentity
        IPCSkeleton::SetCallingIdentity(identity);
        return ret;
    }

    HILOG_INFO("ImplicitQueryInfos success, Multiple apps to choose in pc.");
    auto type = request.want.GetType();
    want = sysDialogScheduler->GetPcSelectorDialogWant(dialogAppInfos, request.want, type, userId, request.callerToken);
    ret = abilityMgr->StartAbility(want, request.callerToken);
    // reset calling indentity
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

int ImplicitStartProcessor::GenerateAbilityRequestByAction(int32_t userId,
    AbilityRequest &request, std::vector<DialogAppInfo> &dialogAppInfos, std::string &deviceType, bool isMoreHapList)
{
    HILOG_DEBUG("%{public}s", __func__);
    // get abilityinfos from bms
    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);
    auto abilityInfoFlag = AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT;
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    bool withDefault = false;
    withDefault = request.want.GetBoolParam(SHOW_DEFAULT_PICKER_FLAG, withDefault) ? false : true;
    IN_PROCESS_CALL_WITHOUT_RET(bms->ImplicitQueryInfos(
        request.want, abilityInfoFlag, userId, withDefault, abilityInfos, extensionInfos));

    HILOG_INFO("ImplicitQueryInfos, abilityInfo size : %{public}zu, extensionInfos size: %{public}zu",
        abilityInfos.size(), extensionInfos.size());

    if (abilityInfos.size() + extensionInfos.size() > 1) {
        HILOG_INFO("More than one target application, filter by erms");
        bool ret = FilterAbilityList(request.want, abilityInfos, extensionInfos, userId);
        if (!ret) {
            HILOG_ERROR("FilterAbilityList failed");
        }
    }

    auto isExtension = request.callType == AbilityCallType::START_EXTENSION_TYPE;

    Want implicitwant;
    implicitwant.SetAction(request.want.GetAction());
    implicitwant.SetType(TYPE_ONLY_MATCH_WILDCARD);
    std::vector<AppExecFwk::AbilityInfo> implicitAbilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> implicitExtensionInfos;
    std::vector<std::string> infoNames;
    if (deviceType != STR_PHONE && deviceType != STR_DEFAULT) {
        IN_PROCESS_CALL_WITHOUT_RET(bms->ImplicitQueryInfos(implicitwant, abilityInfoFlag, userId,
            withDefault, implicitAbilityInfos, implicitExtensionInfos));
        if (implicitAbilityInfos.size() != 0 && request.want.GetType() != TYPE_ONLY_MATCH_WILDCARD) {
            for (auto implicitAbilityInfo : implicitAbilityInfos) {
                infoNames.emplace_back(implicitAbilityInfo.bundleName + "#" +
                    implicitAbilityInfo.moduleName + "#" + implicitAbilityInfo.name);
            }
        }
    }
    for (const auto &info : abilityInfos) {
        if (isExtension && info.type != AbilityType::EXTENSION) {
            continue;
        }
        if (deviceType != STR_PHONE && deviceType != STR_DEFAULT) {
            auto isDefaultFlag = false;
            if (withDefault) {
                auto defaultMgr = GetDefaultAppProxy();
                AppExecFwk::BundleInfo bundleInfo;
                ErrCode ret =
                    IN_PROCESS_CALL(defaultMgr->GetDefaultApplication(userId, request.want.GetType(), bundleInfo));
                if (ret == ERR_OK) {
                    if (bundleInfo.abilityInfos.size() == 1) {
                        HILOG_INFO("find default ability.");
                        isDefaultFlag = true;
                    } else if (bundleInfo.extensionInfos.size() == 1) {
                        HILOG_INFO("find default extension.");
                        isDefaultFlag = true;
                    } else {
                        HILOG_INFO("GetDefaultApplication failed.");
                    }
                }
            }
            if (!isMoreHapList && !isDefaultFlag) {
                if (std::find(infoNames.begin(), infoNames.end(),
                    (info.bundleName + "#" + info.moduleName + "#" + info.name)) != infoNames.end()) {
                    continue;
                }
            }
        }

        DialogAppInfo dialogAppInfo;
        dialogAppInfo.abilityName = info.name;
        dialogAppInfo.bundleName = info.bundleName;
        dialogAppInfo.moduleName = info.moduleName;
        dialogAppInfo.iconId = info.iconId;
        dialogAppInfo.labelId = info.labelId;
        dialogAppInfos.emplace_back(dialogAppInfo);
    }

    for (const auto &info : extensionInfos) {
        if (!isExtension || !CheckImplicitStartExtensionIsValid(request, info)) {
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

bool ImplicitStartProcessor::CheckImplicitStartExtensionIsValid(const AbilityRequest &request,
    const AppExecFwk::ExtensionAbilityInfo &extensionInfo)
{
    if (!request.want.GetElement().GetBundleName().empty()) {
        return true;
    }
    HILOG_DEBUG("ImplicitStartExtension type: %{public}d.", static_cast<int32_t>(extensionInfo.type));
    if (extensionWhiteList.find(extensionInfo.type) == extensionWhiteList.end()) {
        HILOG_ERROR("The extension without UI is not allowed ImplicitStart");
        return false;
    }
    return true;
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
    EventInfo eventInfo;
    eventInfo.userId = userId;
    eventInfo.bundleName = want.GetElement().GetBundleName();
    eventInfo.moduleName = want.GetElement().GetModuleName();
    eventInfo.abilityName = want.GetElement().GetAbilityName();

    if (callType == AbilityCallType::INVALID_TYPE) {
        EventReport::SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    }

    HILOG_INFO("ability:%{public}s, bundle:%{public}s", eventInfo.abilityName.c_str(), eventInfo.bundleName.c_str());

    auto ret = callBack();
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        if (callType == AbilityCallType::INVALID_TYPE) {
            EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        }
    }
    return ret;
}

sptr<AppExecFwk::IBundleMgr> ImplicitStartProcessor::GetBundleManager()
{
    if (iBundleManager_ == nullptr) {
        iBundleManager_ = AbilityUtil::GetBundleManager();
    }
    return iBundleManager_;
}

sptr<AppExecFwk::IDefaultApp> ImplicitStartProcessor::GetDefaultAppProxy()
{
    auto bundleMgr = GetBundleManager();
    auto defaultAppProxy = bundleMgr->GetDefaultAppProxy();
    if (defaultAppProxy == nullptr) {
        HILOG_ERROR("GetDefaultAppProxy failed.");
        return nullptr;
    }
    return defaultAppProxy;
}

bool ImplicitStartProcessor::FilterAbilityList(const Want &want, std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos, int32_t userId)
{
    auto erms = AbilityUtil::CheckEcologicalRuleMgr();
    if (!erms) {
        HILOG_ERROR("get ecological rule mgr failed.");
        return false;
    }

    ErmsCallerInfo callerInfo;
    AbilityUtil::GetEcologicalCallerInfo(want, callerInfo, userId);
    int ret = IN_PROCESS_CALL(erms->EvaluateResolveInfos(want, callerInfo, 0, abilityInfos, extensionInfos));
    if (ret != ERR_OK) {
        HILOG_ERROR("Failed to evaluate resolve infos from erms.");
        return false;
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS