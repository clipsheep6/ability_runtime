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
#include "app_gallery_enable_util.h"
#include "default_app_interface.h"
#include "errors.h"
#include "event_report.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "parameters.h"
#include "scene_board_judgement.h"
#include "want.h"
#ifdef SUPPORT_ERMS
#include "ecological_rule_mgr_service_client.h"
#endif

namespace OHOS {
namespace AAFwk {
#ifdef SUPPORT_ERMS
using namespace OHOS::EcologicalRuleMgrService;

constexpr int32_t TYPE_HARMONY_INVALID = 0;
constexpr int32_t TYPE_HARMONY_APP = 1;
constexpr int32_t TYPE_HARMONY_SERVICE = 2;
#else
using ErmsCallerInfo = OHOS::AppExecFwk::ErmsParams::CallerInfo;
#endif
const size_t IDENTITY_LIST_MAX_SIZE = 10;

const std::string BLACK_ACTION_SELECT_DATA = "ohos.want.action.select";
const std::string STR_PHONE = "phone";
const std::string STR_DEFAULT = "default";
const std::string TYPE_ONLY_MATCH_WILDCARD = "reserved/wildcard";
const std::string SHOW_DEFAULT_PICKER_FLAG = "ohos.ability.params.showDefaultPicker";
const std::string PARAM_ABILITY_APPINFOS = "ohos.ability.params.appInfos";
const int NFC_CALLER_UID = 1027;

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
        if ((request.want.GetFlags() & Want::FLAG_START_WITHOUT_TIPS) == Want::FLAG_START_WITHOUT_TIPS) {
            HILOG_INFO("hint dialog doesn't generate.");
            return ERR_IMPLICIT_START_ABILITY_FAIL;
        }
        if (AppGalleryEnableUtil::IsEnableAppGallerySelector() && Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
            want = sysDialogScheduler->GetSelectorDialogWant(dialogAppInfos, request.want, request.callerToken);
            return NotifyCreateModalDialog(request, want, userId, dialogAppInfos);
        }
        HILOG_ERROR("implicit query ability infos failed, show tips dialog.");
        want = sysDialogScheduler->GetTipsDialogWant(request.callerToken);
        abilityMgr->StartAbility(want);
        return ERR_IMPLICIT_START_ABILITY_FAIL;
    } else if (dialogAppInfos.size() == 0 && deviceType != STR_PHONE && deviceType != STR_DEFAULT) {
        if (AppGalleryEnableUtil::IsEnableAppGallerySelector() && Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
            std::string type = MatchTypeAndUri(request.want);
            want = sysDialogScheduler->GetPcSelectorDialogWant(dialogAppInfos, request.want, type,
                userId, request.callerToken);
            return NotifyCreateModalDialog(request, want, userId, dialogAppInfos);
        }
        std::vector<DialogAppInfo> dialogAllAppInfos;
        bool isMoreHapList = true;
        ret = GenerateAbilityRequestByAction(userId, request, dialogAllAppInfos, deviceType, isMoreHapList);
        if (ret != ERR_OK) {
            HILOG_ERROR("generate ability request by action failed.");
            return ret;
        }
        if (dialogAllAppInfos.size() == 0) {
            if ((request.want.GetFlags() & Want::FLAG_START_WITHOUT_TIPS) == Want::FLAG_START_WITHOUT_TIPS) {
                HILOG_INFO("hint dialog doesn't generate.");
                return ERR_IMPLICIT_START_ABILITY_FAIL;
            }
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

    //There is a default opening method add Only one application supports
    bool defaultPicker = false;
    defaultPicker = request.want.GetBoolParam(SHOW_DEFAULT_PICKER_FLAG, defaultPicker);
    if (dialogAppInfos.size() == 1 && (!defaultPicker || deviceType == STR_PHONE || deviceType == STR_DEFAULT)) {
        auto info = dialogAppInfos.front();
        HILOG_INFO("ImplicitQueryInfos success, target ability: %{public}s", info.abilityName.data());
        return IN_PROCESS_CALL(startAbilityTask(info.bundleName, info.abilityName));
    }

    if (deviceType == STR_PHONE || deviceType == STR_DEFAULT) {
        HILOG_INFO("ImplicitQueryInfos success, Multiple apps to choose.");
        want = sysDialogScheduler->GetSelectorDialogWant(dialogAppInfos, request.want, request.callerToken);
        if (AppGalleryEnableUtil::IsEnableAppGallerySelector() && Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
            return NotifyCreateModalDialog(request, want, userId, dialogAppInfos);
        }
        ret = abilityMgr->StartAbilityAsCaller(want, request.callerToken, nullptr);
        // reset calling indentity
        IPCSkeleton::SetCallingIdentity(identity);
        int32_t tokenId = request.want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN,
            static_cast<int32_t>(IPCSkeleton::GetCallingTokenID()));
        AddIdentity(tokenId, identity);
        return ret;
    }

    HILOG_INFO("ImplicitQueryInfos success, Multiple apps to choose in pc.");
    std::string type = MatchTypeAndUri(request.want);

    want = sysDialogScheduler->GetPcSelectorDialogWant(dialogAppInfos, request.want, type, userId, request.callerToken);
    if (AppGalleryEnableUtil::IsEnableAppGallerySelector() && Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        return NotifyCreateModalDialog(request, want, userId, dialogAppInfos);
    }
    ret = abilityMgr->StartAbilityAsCaller(want, request.callerToken, nullptr);
    // reset calling indentity
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

int ImplicitStartProcessor::NotifyCreateModalDialog(AbilityRequest &abilityRequest, const Want &want, int32_t userId,
    std::vector<DialogAppInfo> &dialogAppInfos)
{
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    std::string dialogSessionId;
    if (abilityMgr->GenerateDialogSessionRecord(abilityRequest, userId, dialogSessionId, dialogAppInfos)) {
        HILOG_DEBUG("create dialog by ui extension");
        return abilityMgr->CreateModalDialog(want, abilityRequest.callerToken, dialogSessionId);
    }
    HILOG_ERROR("create dialog by ui extension failed");
    return INNER_ERR;
}

std::string ImplicitStartProcessor::MatchTypeAndUri(const AAFwk::Want &want)
{
    std::string type = want.GetType();
    if (type.empty()) {
        auto uri = want.GetUriString();
        auto suffixIndex = uri.rfind('.');
        if (suffixIndex == std::string::npos) {
            HILOG_ERROR("Get suffix failed, uri is %{public}s", uri.c_str());
            return "";
        }
        type = uri.substr(suffixIndex);
        if (type == ".dlp") {
            auto suffixDlpIndex = uri.rfind('.', suffixIndex - 1);
            if (suffixDlpIndex == std::string::npos) {
                HILOG_ERROR("Get suffix failed, uri is %{public}s", uri.c_str());
                return "";
            }
            type = uri.substr(suffixDlpIndex, suffixIndex - suffixDlpIndex);
        }
    }
    return type;
}

int ImplicitStartProcessor::GenerateAbilityRequestByAction(int32_t userId,
    AbilityRequest &request, std::vector<DialogAppInfo> &dialogAppInfos, std::string &deviceType, bool isMoreHapList)
{
    HILOG_DEBUG("%{public}s.", __func__);
    // get abilityinfos from bms
    auto bundleMgrHelper = GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bundleMgrHelper, GET_ABILITY_SERVICE_FAILED);
    auto abilityInfoFlag = AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT
        | AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_SKILL_URI;
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    bool withDefault = false;
    withDefault = request.want.GetBoolParam(SHOW_DEFAULT_PICKER_FLAG, withDefault) ? false : true;

    if (IPCSkeleton::GetCallingUid() == NFC_CALLER_UID && !request.want.GetStringArrayParam(PARAM_ABILITY_APPINFOS).empty()) {
        HILOG_INFO("The NFCNeed caller source is NFC.");

        ImplicitStartProcessor::queryBmsAppInfos(request, userId, dialogAppInfos);
        return ERR_OK;
    }

    IN_PROCESS_CALL_WITHOUT_RET(bundleMgrHelper->ImplicitQueryInfos(
        request.want, abilityInfoFlag, userId, withDefault, abilityInfos, extensionInfos));

    HILOG_INFO("ImplicitQueryInfos, abilityInfo size : %{public}zu, extensionInfos size: %{public}zu.",
        abilityInfos.size(), extensionInfos.size());

    if (abilityInfos.size() + extensionInfos.size() > 1) {
        HILOG_INFO("More than one target application, filter by erms");
        bool ret = FilterAbilityList(request.want, abilityInfos, extensionInfos, userId);
        if (!ret) {
            HILOG_ERROR("FilterAbilityList failed");
        }
    }

    dialogAppInfos = GetDialogAppInfos(userId, isMoreHapList, request, abilityInfos, extensionInfos);
    return ERR_OK;
}

std::vector<DialogAppInfo> ImplicitStartProcessor::GetDialogAppInfos(int32_t userId, bool isMoreHapList,
    AbilityRequest &request, std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
    std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfos)
{
    auto bms = GetBundleManager();
    auto isExtension = request.callType == AbilityCallType::START_EXTENSION_TYPE;
    std::string typeName = MatchTypeAndUri(request.want);
    auto abilityInfoFlag = AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT
        | AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_SKILL_URI;
    Want implicitwant;
    implicitwant.SetAction(request.want.GetAction());
    implicitwant.SetType(TYPE_ONLY_MATCH_WILDCARD);
    std::vector<AppExecFwk::AbilityInfo> implicitAbilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> implicitExtensionInfos;
    std::vector<std::string> infoNames;
    auto deviceType = OHOS::system::GetDeviceType();
    bool withDefault = false;
    withDefault = request.want.GetBoolParam(SHOW_DEFAULT_PICKER_FLAG, withDefault) ? false : true;
    if (deviceType != STR_PHONE && deviceType != STR_DEFAULT) {
        IN_PROCESS_CALL_WITHOUT_RET(bundleMgrHelper->ImplicitQueryInfos(implicitwant, abilityInfoFlag, userId,
            withDefault, implicitAbilityInfos, implicitExtensionInfos));
        if (implicitAbilityInfos.size() != 0 && typeName != TYPE_ONLY_MATCH_WILDCARD) {
            for (auto implicitAbilityInfo : implicitAbilityInfos) {
                infoNames.emplace_back(implicitAbilityInfo.bundleName + "#" +
                    implicitAbilityInfo.moduleName + "#" + implicitAbilityInfo.name);
            }
        }
    }
    std::vector<DialogAppInfo> dialogAppInfos;
    for (const auto &info : abilityInfos) {
        if (isExtension && info.type != AbilityType::EXTENSION) {
            continue;
        }
        auto isDefault = GetDefaultApplicationInfo(userId, request, isMoreHapList, infoNames, info);
        DialogAppInfo dialogAppInfo;
        if (!isDefault) {
            SetDialogAppInfoByAbilityInfo(dialogAppInfo, info);
            dialogAppInfos.emplace_back(dialogAppInfo);
        }
    }
    for (const auto &info : extensionInfos) {
        if (!isExtension || !CheckImplicitStartExtensionIsValid(request, info)) {
            continue;
        }
        DialogAppInfo dialogAppInfo;
        SetDialogAppInfoByExtensionAbilityInfo(dialogAppInfo, info);
        dialogAppInfos.emplace_back(dialogAppInfo);
    }
    return dialogAppInfos;
}

void ImplicitStartProcessor::SetDialogAppInfoByAbilityInfo(DialogAppInfo &dialogAppInfo,
    const AppExecFwk::AbilityInfo &abilityInfo)
{
    dialogAppInfo.abilityName = abilityInfo.name;
    dialogAppInfo.bundleName = abilityInfo.bundleName;
    dialogAppInfo.moduleName = abilityInfo.moduleName;
    dialogAppInfo.iconId = abilityInfo.iconId;
    dialogAppInfo.labelId = abilityInfo.labelId;
}

void ImplicitStartProcessor::SetDialogAppInfoByExtensionAbilityInfo(DialogAppInfo &dialogAppInfo,
    const AppExecFwk::ExtensionAbilityInfo &extensionAbilityInfo)
{
    dialogAppInfo.abilityName = extensionAbilityInfo.name;
    dialogAppInfo.bundleName = extensionAbilityInfo.bundleName;
    dialogAppInfo.iconId = extensionAbilityInfo.iconId;
    dialogAppInfo.labelId = extensionAbilityInfo.labelId;
}

bool ImplicitStartProcessor::GetDefaultApplicationInfo(int32_t userId, AbilityRequest &request,
    bool isMoreHapList, std::vector<std::string> &infoNames, const AppExecFwk::AbilityInfo &abilityInfo)
{
    auto deviceType = OHOS::system::GetDeviceType();
    if (deviceType != STR_PHONE && deviceType != STR_DEFAULT) {
        auto isDefaultFlag = false;
        bool withDefault = false;
        withDefault = request.want.GetBoolParam(SHOW_DEFAULT_PICKER_FLAG, withDefault) ? false : true;
        if (withDefault) {
            auto defaultMgr = GetDefaultAppProxy();
            AppExecFwk::BundleInfo bundleInfo;
            std::string typeName = MatchTypeAndUri(request.want);
            ErrCode ret = IN_PROCESS_CALL(defaultMgr->GetDefaultApplication(userId, typeName, bundleInfo));
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
            if (!isMoreHapList && !isDefaultFlag) {
                return MatchFlagFromInfoNames(infoNames, abilityInfo);
            }
        }
    }
    return false;
}

bool ImplicitStartProcessor::MatchFlagFromInfoNames(std::vector<std::string> &infoNames,
    const AppExecFwk::AbilityInfo &abilityInfo)
{
    if (std::find(infoNames.begin(), infoNames.end(),
        (abilityInfo.bundleName + "#" + abilityInfo.moduleName + "#" + abilityInfo.name)) != infoNames.end()) {
        return true;
    }
    return false;
}

int ImplicitStartProcessor::queryBmsAppInfos(AbilityRequest &request, int32_t userId, std::vector<DialogAppInfo> &dialogAppInfos) {
    auto bundleMgrHelper = GetBundleManagerHelper();
    std::vector<AppExecFwk::AbilityInfo> bmsApps;
    auto abilityInfoFlag = AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT
        | AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_SKILL_URI;

    std::vector<std::string> apps = request.want.GetStringArrayParam(PARAM_ABILITY_APPINFOS);    
    for (std::string appInfoStr : apps) {
        AppExecFwk::AbilityInfo abilityInfo;
        std::vector<std::string> appInfos = ImplicitStartProcessor::splitStr(appInfoStr, '/');
        std::string bundleName = appInfos[0];
        std::string abilityName = appInfos[1];
        Want want;
        want.SetElementName(bundleName, abilityName);

        IN_PROCESS_CALL_WITHOUT_RET(bundleMgrHelper->QueryAbilityInfo(want, abilityInfoFlag,
            userId, abilityInfo));
        if (!abilityInfo.name.empty() && !abilityInfo.bundleName.empty() && !abilityInfo.moduleName.empty()) {
            bmsApps.emplace_back(abilityInfo);
        }
    }
    if (!bmsApps.empty()) {
        for (const auto &abilityInfo : bmsApps) {
            DialogAppInfo dialogAppInfo;
            dialogAppInfo.abilityName = abilityInfo.name;
            dialogAppInfo.bundleName = abilityInfo.bundleName;
            dialogAppInfo.moduleName = abilityInfo.moduleName;
            dialogAppInfo.iconId = abilityInfo.iconId;
            dialogAppInfo.labelId = abilityInfo.labelId;
            dialogAppInfos.emplace_back(dialogAppInfo);
        }
    }
    return ERR_OK;
}

std::vector<std::string> ImplicitStartProcessor::splitStr(const std::string& str, char delimiter) {
    std::stringstream ss(str);  
    std::vector<std::string> result;  
    std::string s;  
    while (std::getline(ss, s, delimiter)) {  
        result.push_back(s);  
    }  
    return result;  
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
            result = abilityMgr->StartAbilityWrap(
                targetWant, request.callerToken, request.requestCode, userId);
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

std::shared_ptr<AppExecFwk::BundleMgrHelper> ImplicitStartProcessor::GetBundleManagerHelper()
{
    if (iBundleManagerHelper_ == nullptr) {
        iBundleManagerHelper_ = AbilityUtil::GetBundleManagerHelper();
    }
    return iBundleManagerHelper_;
}

sptr<AppExecFwk::IDefaultApp> ImplicitStartProcessor::GetDefaultAppProxy()
{
    auto bundleMgrHelper = GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        HILOG_ERROR("The bundleMgrHelper is nullptr.");
        return nullptr;
    }
    auto defaultAppProxy = bundleMgrHelper->GetDefaultAppProxy();
    if (defaultAppProxy == nullptr) {
        HILOG_ERROR("The defaultAppProxy is nullptr.");
        return nullptr;
    }
    return defaultAppProxy;
}

bool ImplicitStartProcessor::FilterAbilityList(const Want &want, std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos, int32_t userId)
{
#ifdef SUPPORT_ERMS
    ErmsCallerInfo callerInfo;
    GetEcologicalCallerInfo(want, callerInfo, userId);
    int ret = IN_PROCESS_CALL(EcologicalRuleMgrServiceClient::GetInstance()->EvaluateResolveInfos(want, callerInfo, 0,
        abilityInfos, extensionInfos));
#else
    auto erms = AbilityUtil::CheckEcologicalRuleMgr();
    if (!erms) {
        HILOG_ERROR("get ecological rule mgr failed.");
        return false;
    }

    ErmsCallerInfo callerInfo;
    int ret = IN_PROCESS_CALL(erms->EvaluateResolveInfos(want, callerInfo, 0, abilityInfos, extensionInfos));
#endif
    if (ret != ERR_OK) {
        HILOG_ERROR("Failed to evaluate resolve infos from erms.");
        return false;
    }
    return true;
}

#ifdef SUPPORT_ERMS
void ImplicitStartProcessor::GetEcologicalCallerInfo(const Want &want, ErmsCallerInfo &callerInfo, int32_t userId)
{
    callerInfo.packageName = want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    callerInfo.uid = want.GetIntParam(Want::PARAM_RESV_CALLER_UID, IPCSkeleton::GetCallingUid());
    callerInfo.pid = want.GetIntParam(Want::PARAM_RESV_CALLER_PID, IPCSkeleton::GetCallingPid());
    callerInfo.targetAppType = TYPE_HARMONY_INVALID;
    callerInfo.callerAppType = TYPE_HARMONY_INVALID;

    auto bundleMgrHelper = GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        HILOG_ERROR("Get Bubndle manager helper failed.");
        return;
    }

    std::string targetBundleName = want.GetBundle();
    AppExecFwk::ApplicationInfo targetAppInfo;
    bool getTargetResult = IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(targetBundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, targetAppInfo));
    if (!getTargetResult) {
        HILOG_ERROR("Get targetAppInfo failed.");
    } else if (targetAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        HILOG_DEBUG("the target type  is atomic service");
        callerInfo.targetAppType = TYPE_HARMONY_SERVICE;
    } else if (targetAppInfo.bundleType == AppExecFwk::BundleType::APP) {
        HILOG_DEBUG("the target type is app");
        callerInfo.targetAppType = TYPE_HARMONY_APP;
    } else {
        HILOG_DEBUG("the target type is invalid type");
    }

    std::string callerBundleName;
    ErrCode err = IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callerInfo.uid, callerBundleName));
    if (err != ERR_OK) {
        HILOG_ERROR("Get callerBundleName failed.");
        return;
    }
    AppExecFwk::ApplicationInfo callerAppInfo;
    bool getCallerResult = IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(callerBundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, callerAppInfo));
    if (!getCallerResult) {
        HILOG_DEBUG("Get callerAppInfo failed.");
    } else if (callerAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        HILOG_DEBUG("the caller type  is atomic service");
        callerInfo.callerAppType = TYPE_HARMONY_SERVICE;
    } else if (callerAppInfo.bundleType == AppExecFwk::BundleType::APP) {
        HILOG_DEBUG("the caller type is app");
        callerInfo.callerAppType = TYPE_HARMONY_APP;
    } else {
        HILOG_DEBUG("the caller type is invalid type");
    }
}
#endif

void ImplicitStartProcessor::AddIdentity(int32_t tokenId, std::string identity)
{
    std::lock_guard guard(identityListLock_);
    if (identityList_.size() == IDENTITY_LIST_MAX_SIZE) {
        identityList_.pop_front();
        identityList_.emplace_back(IdentityNode(tokenId, identity));
        return;
    }
    identityList_.emplace_back(IdentityNode(tokenId, identity));
}

void ImplicitStartProcessor::ResetCallingIdentityAsCaller(int32_t tokenId)
{
    std::lock_guard guard(identityListLock_);
    for (auto it = identityList_.begin(); it != identityList_.end(); it++) {
        if (it->tokenId == tokenId) {
            IPCSkeleton::SetCallingIdentity(it->identity);
            identityList_.erase(it);
            return;
        }
    }
}
}  // namespace AAFwk
}  // namespace OHOS
