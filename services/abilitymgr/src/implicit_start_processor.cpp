/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "app_utils.h"
#include "default_app_interface.h"
#include "errors.h"
#include "ecological_rule/ability_ecological_rule_mgr_service.h"
#include "event_report.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "parameters.h"
#include "scene_board_judgement.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
const size_t IDENTITY_LIST_MAX_SIZE = 10;
const int32_t BROKER_UID = 5557;

const std::string BLACK_ACTION_SELECT_DATA = "ohos.want.action.select";
const std::string ACTION_VIEW = "ohos.want.action.viewData";
const std::string STR_PHONE = "phone";
const std::string STR_DEFAULT = "default";
const std::string TYPE_ONLY_MATCH_WILDCARD = "reserved/wildcard";
const std::string SHOW_DEFAULT_PICKER_FLAG = "ohos.ability.params.showDefaultPicker";
const std::string PARAM_ABILITY_APPINFOS = "ohos.ability.params.appInfos";
const std::string ANCO_PENDING_REQUEST = "ancoPendingRequest";
const std::string SHELL_ASSISTANT_BUNDLENAME = "com.huawei.shell_assistant";
const int NFC_CALLER_UID = 1027;
const int NFC_QUERY_LENGTH = 2;
const std::string OPEN_LINK_APP_LINKING_ONLY = "appLinkingOnly";
const std::string HTTP_SCHEME_NAME = "http";
const std::string HTTPS_SCHEME_NAME = "https";

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

    if (want.GetIntParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE) != ScreenMode::IDLE_SCREEN_MODE) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "The implicit startup process is not used for the startup of EmbeddaUIAbility");
        return false;
    }

    if (std::find(blackList.begin(), blackList.end(), want.GetAction()) == blackList.end()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "implicit start, the action is %{public}s", want.GetAction().data());
        return true;
    }

    return false;
}

int ImplicitStartProcessor::ImplicitStartAbility(AbilityRequest &request, int32_t userId, int32_t windowMode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "implicit start ability by type: %{public}d", request.callType);
    auto sysDialogScheduler = DelayedSingleton<SystemDialogScheduler>::GetInstance();
    CHECK_POINTER_AND_RETURN(sysDialogScheduler, ERR_INVALID_VALUE);

    std::vector<DialogAppInfo> dialogAppInfos;
    auto ret = GenerateAbilityRequestByAction(userId, request, dialogAppInfos, false);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "generate ability request by action failed.");
        return ret;
    }
    AbilityUtil::WantSetParameterWindowMode(request.want, windowMode);

    auto identity = IPCSkeleton::ResetCallingIdentity();
    auto startAbilityTask = [imp = shared_from_this(), request, userId, identity]
        (const std::string& bundle, const std::string& abilityName) mutable {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "implicit start ability call back.");

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
    int32_t tokenId = request.want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN,
        static_cast<int32_t>(IPCSkeleton::GetCallingTokenID()));
    AddIdentity(tokenId, identity);
    if (dialogAppInfos.size() == 0 && AppUtils::GetInstance().IsSelectorDialogDefaultPossion()) {
        if ((request.want.GetFlags() & Want::FLAG_START_WITHOUT_TIPS) == Want::FLAG_START_WITHOUT_TIPS) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "hint dialog doesn't generate.");
            return ERR_IMPLICIT_START_ABILITY_FAIL;
        }
        ret = sysDialogScheduler->GetSelectorDialogWant(dialogAppInfos, request.want, request.callerToken);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GetSelectorDialogWant failed.");
            return ret;
        }
        if (request.want.GetBoolParam("isCreateAppGallerySelector", false)) {
            request.want.RemoveParam("isCreateAppGallerySelector");
            NotifyCreateModalDialog(request, request.want, userId, dialogAppInfos);
            return ERR_IMPLICIT_START_ABILITY_FAIL;
        }
        TAG_LOGE(AAFwkTag::ABILITYMGR, "implicit query ability infos failed, show tips dialog.");
        want = sysDialogScheduler->GetTipsDialogWant(request.callerToken);
        abilityMgr->StartAbility(want);
        return ERR_IMPLICIT_START_ABILITY_FAIL;
    } else if (dialogAppInfos.size() == 0 && !AppUtils::GetInstance().IsSelectorDialogDefaultPossion()) {
        std::string type = MatchTypeAndUri(request.want);
        ret = sysDialogScheduler->GetPcSelectorDialogWant(dialogAppInfos, request.want, type,
            userId, request.callerToken);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GetPcSelectorDialogWant failed.");
            return ret;
        }
        if (request.want.GetBoolParam("isCreateAppGallerySelector", false)) {
            request.want.RemoveParam("isCreateAppGallerySelector");
            NotifyCreateModalDialog(request, request.want, userId, dialogAppInfos);
            return ERR_IMPLICIT_START_ABILITY_FAIL;
        }
        std::vector<DialogAppInfo> dialogAllAppInfos;
        bool isMoreHapList = true;
        ret = GenerateAbilityRequestByAction(userId, request, dialogAllAppInfos, isMoreHapList);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "generate ability request by action failed.");
            return ret;
        }
        if (dialogAllAppInfos.size() == 0) {
            if ((request.want.GetFlags() & Want::FLAG_START_WITHOUT_TIPS) == Want::FLAG_START_WITHOUT_TIPS) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "hint dialog doesn't generate.");
                return ERR_IMPLICIT_START_ABILITY_FAIL;
            }
            Want dialogWant = sysDialogScheduler->GetTipsDialogWant(request.callerToken);
            abilityMgr->StartAbility(dialogWant);
            return ERR_IMPLICIT_START_ABILITY_FAIL;
        }
        ret = sysDialogScheduler->GetPcSelectorDialogWant(dialogAllAppInfos, request.want,
            TYPE_ONLY_MATCH_WILDCARD, userId, request.callerToken);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GetPcSelectorDialogWant failed.");
            return ret;
        }
        ret = abilityMgr->StartAbility(request.want, request.callerToken);
        // reset calling indentity
        IPCSkeleton::SetCallingIdentity(identity);
        return ret;
    }

    //There is a default opening method add Only one application supports
    bool defaultPicker = false;
    defaultPicker = request.want.GetBoolParam(SHOW_DEFAULT_PICKER_FLAG, defaultPicker);
    if (dialogAppInfos.size() == 1 && (!defaultPicker || AppUtils::GetInstance().IsSelectorDialogDefaultPossion())) {
        auto info = dialogAppInfos.front();
        TAG_LOGI(
            AAFwkTag::ABILITYMGR, "ImplicitQueryInfos success, target ability: %{public}s", info.abilityName.data());
        return IN_PROCESS_CALL(startAbilityTask(info.bundleName, info.abilityName));
    }

    if (AppUtils::GetInstance().IsSelectorDialogDefaultPossion()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ImplicitQueryInfos success, Multiple apps to choose.");
        ret = sysDialogScheduler->GetSelectorDialogWant(dialogAppInfos, request.want, request.callerToken);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GetSelectorDialogWant failed.");
            return ret;
        }
        if (request.want.GetBoolParam("isCreateAppGallerySelector", false)) {
            request.want.RemoveParam("isCreateAppGallerySelector");
            return NotifyCreateModalDialog(request, request.want, userId, dialogAppInfos);
        }
        ret = abilityMgr->ImplicitStartAbilityAsCaller(request.want, request.callerToken, nullptr);
        // reset calling indentity
        IPCSkeleton::SetCallingIdentity(identity);
        return ret;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ImplicitQueryInfos success, Multiple apps to choose in pc.");
    std::string type = MatchTypeAndUri(request.want);

    ret = sysDialogScheduler->GetPcSelectorDialogWant(dialogAppInfos, request.want, type, userId, request.callerToken);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetPcSelectorDialogWant failed.");
        return ret;
    }
    if (request.want.GetBoolParam("isCreateAppGallerySelector", false)) {
        request.want.RemoveParam("isCreateAppGallerySelector");
        return NotifyCreateModalDialog(request, request.want, userId, dialogAppInfos);
    }
    ret = abilityMgr->ImplicitStartAbilityAsCaller(request.want, request.callerToken, nullptr);
    // reset calling indentity
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

int ImplicitStartProcessor::NotifyCreateModalDialog(AbilityRequest &abilityRequest, const Want &want, int32_t userId,
    std::vector<DialogAppInfo> &dialogAppInfos)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    std::string dialogSessionId;
    if (abilityMgr->GenerateDialogSessionRecord(abilityRequest, userId, dialogSessionId, dialogAppInfos, true)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "create dialog by ui extension");
        return abilityMgr->CreateModalDialog(want, abilityRequest.callerToken, dialogSessionId);
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "create dialog by ui extension failed");
    return INNER_ERR;
}

std::string ImplicitStartProcessor::MatchTypeAndUri(const AAFwk::Want &want)
{
    std::string type = want.GetType();
    if (type.empty()) {
        auto uri = want.GetUriString();
        auto suffixIndex = uri.rfind('.');
        if (suffixIndex == std::string::npos) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Get suffix failed, uri is %{public}s", uri.c_str());
            return "";
        }
        type = uri.substr(suffixIndex);
        if (type == ".dlp") {
            auto suffixDlpIndex = uri.rfind('.', suffixIndex - 1);
            if (suffixDlpIndex == std::string::npos) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Get suffix failed, uri is %{public}s", uri.c_str());
                return "";
            }
            type = uri.substr(suffixDlpIndex, suffixIndex - suffixDlpIndex);
        }
    }
    return type;
}

static void ProcessLinkType(std::vector<AppExecFwk::AbilityInfo> &abilityInfos)
{
    bool appLinkingExist = false;
    if (!abilityInfos.size()) {
        return;
    }
    for (const auto &info : abilityInfos) {
        if (info.linkType == AppExecFwk::LinkType::APP_LINK) {
            appLinkingExist = true;
            break;
        }
    }
    if (!appLinkingExist) {
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Open applink first!");
    for (auto it = abilityInfos.begin(); it != abilityInfos.end();) {
        if (it->linkType == AppExecFwk::LinkType::APP_LINK) {
            it++;
            continue;
        }
        if (it->linkType == AppExecFwk::LinkType::DEEP_LINK) {
            it = abilityInfos.erase(it);
            TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s deleted.", it->name.c_str());
        }
    }
}

void ImplicitStartProcessor::SetUriReservedFlag(const bool flag)
{
    uriReservedFlag_ = flag;
}

void ImplicitStartProcessor::SetUriReservedBundle(const std::string bundleName)
{
    reservedBundleName_ = bundleName;
}

void ImplicitStartProcessor::OnlyKeepReserveApp(std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
    std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfos)
{
    if (!uriReservedFlag_) {
        return;
    }
    if (extensionInfos.size() > 0) {
        extensionInfos.clear();
    }

    for (auto it = abilityInfos.begin(); it != abilityInfos.end();) {
        if (it->bundleName == reservedBundleName_) {
            it++;
            continue;
        } else {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "Reserve App %{public}s dismatch with bundleName %{public}s.",
                reservedBundleName_.c_str(), it->bundleName.c_str());
            it = abilityInfos.erase(it);
        }
    }
}

int ImplicitStartProcessor::GenerateAbilityRequestByAction(int32_t userId,
    AbilityRequest &request, std::vector<DialogAppInfo> &dialogAppInfos, bool isMoreHapList)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s.", __func__);
    // get abilityinfos from bms
    auto bundleMgrHelper = GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bundleMgrHelper, GET_ABILITY_SERVICE_FAILED);
    auto abilityInfoFlag = AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT
        | AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_SKILL_URI
        | AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION;
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    bool withDefault = false;
    withDefault = request.want.GetBoolParam(SHOW_DEFAULT_PICKER_FLAG, withDefault) ? false : true;
    bool appLinkingOnly = false;
    bool isOpenLink = false;
    isOpenLink = request.want.HasParameter(OPEN_LINK_APP_LINKING_ONLY);
    appLinkingOnly = request.want.GetBoolParam(OPEN_LINK_APP_LINKING_ONLY, false);

    if (IPCSkeleton::GetCallingUid() == NFC_CALLER_UID &&
        !request.want.GetStringArrayParam(PARAM_ABILITY_APPINFOS).empty()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "The NFCNeed caller source is NFC.");
        ImplicitStartProcessor::QueryBmsAppInfos(request, userId, dialogAppInfos);
    }

    if (!IsCallFromAncoShellOrBroker(request.callerToken)) {
        request.want.RemoveParam(ANCO_PENDING_REQUEST);
    }

    if (appLinkingOnly) {
        abilityInfoFlag = static_cast<uint32_t>(abilityInfoFlag) |
            static_cast<uint32_t>(AppExecFwk::GetAbilityInfoFlag::GET_ABILITY_INFO_WITH_APP_LINKING);
    }

    if (uriReservedFlag_) {
        abilityInfoFlag = abilityInfoFlag |
            static_cast<uint32_t>(AppExecFwk::GetAbilityInfoFlag::GET_ABILITY_INFO_ONLY_SYSTEM_APP);
    }

    IN_PROCESS_CALL_WITHOUT_RET(bundleMgrHelper->ImplicitQueryInfos(
        request.want, abilityInfoFlag, userId, withDefault, abilityInfos, extensionInfos));

    OnlyKeepReserveApp(abilityInfos, extensionInfos);
    if (isOpenLink && extensionInfos.size() > 0) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Clear extensionInfos when isOpenLink.");
        extensionInfos.clear();
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "ImplicitQueryInfos, abilityInfo size : %{public}zu, extensionInfos size: %{public}zu.", abilityInfos.size(),
        extensionInfos.size());

    if (appLinkingOnly && abilityInfos.size() == 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "There isn't match app.");
        return ERR_IMPLICIT_START_ABILITY_FAIL;
    }

    if (!appLinkingOnly) {
        ProcessLinkType(abilityInfos);
    }

    if (abilityInfos.size() + extensionInfos.size() > 1) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "More than one target application, filter by erms");
        bool ret = FilterAbilityList(request.want, abilityInfos, extensionInfos, userId);
        if (!ret) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "FilterAbilityList failed");
        }
    }

    auto isExtension = request.callType == AbilityCallType::START_EXTENSION_TYPE;

    Want implicitwant;
    std::string typeName = MatchTypeAndUri(request.want);

    implicitwant.SetAction(request.want.GetAction());
    implicitwant.SetType(TYPE_ONLY_MATCH_WILDCARD);
    std::vector<AppExecFwk::AbilityInfo> implicitAbilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> implicitExtensionInfos;
    std::vector<std::string> infoNames;
    if (!AppUtils::GetInstance().IsSelectorDialogDefaultPossion()) {
        IN_PROCESS_CALL_WITHOUT_RET(bundleMgrHelper->ImplicitQueryInfos(implicitwant, abilityInfoFlag, userId,
            withDefault, implicitAbilityInfos, implicitExtensionInfos));
        if (implicitAbilityInfos.size() != 0 && typeName != TYPE_ONLY_MATCH_WILDCARD) {
            for (auto implicitAbilityInfo : implicitAbilityInfos) {
                infoNames.emplace_back(implicitAbilityInfo.bundleName + "#" +
                    implicitAbilityInfo.moduleName + "#" + implicitAbilityInfo.name);
            }
        }
    }

    if (isOpenLink) {
        std::string linkUriScheme = request.want.GetUri().GetScheme();
        if (linkUriScheme == HTTPS_SCHEME_NAME || linkUriScheme == HTTP_SCHEME_NAME) {
            request.want.SetAction(ACTION_VIEW);
        }
    }

    if (abilityInfos.size() == 1) {
        auto skillUri =  abilityInfos.front().skillUri;
        SetTargetLinkInfo(skillUri, request.want);
    }

    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "for (const auto &info : abilityInfos)");
        for (const auto &info : abilityInfos) {
            AddInfoParam param = {
                .info = info,
                .userId = userId,
                .isExtension = isExtension,
                .isMoreHapList = isMoreHapList,
                .withDefault = withDefault,
                .typeName = typeName,
                .infoNames = infoNames
            };
            AddAbilityInfoToDialogInfos(param, dialogAppInfos);
        }
    }

    for (const auto &info : extensionInfos) {
        if (!isExtension || !CheckImplicitStartExtensionIsValid(request, info)) {
            continue;
        }
        DialogAppInfo dialogAppInfo;
        dialogAppInfo.abilityName = info.name;
        dialogAppInfo.bundleName = info.bundleName;
        dialogAppInfo.abilityIconId = info.iconId;
        dialogAppInfo.abilityLabelId = info.labelId;
        dialogAppInfo.bundleIconId = info.applicationInfo.iconId;
        dialogAppInfo.bundleLabelId = info.applicationInfo.labelId;
        dialogAppInfo.visible = info.visible;
        dialogAppInfos.emplace_back(dialogAppInfo);
    }

    return ERR_OK;
}

int ImplicitStartProcessor::QueryBmsAppInfos(AbilityRequest &request, int32_t userId,
    std::vector<DialogAppInfo> &dialogAppInfos)
{
    auto bundleMgrHelper = GetBundleManagerHelper();
    std::vector<AppExecFwk::AbilityInfo> bmsApps;
    auto abilityInfoFlag = AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT
        | AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_SKILL_URI
        | AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION;
    std::vector<std::string> apps = request.want.GetStringArrayParam(PARAM_ABILITY_APPINFOS);
    for (std::string appInfoStr : apps) {
        AppExecFwk::AbilityInfo abilityInfo;
        std::vector<std::string> appInfos = ImplicitStartProcessor::SplitStr(appInfoStr, '/');
        if (appInfos.empty() || appInfos.size() != NFC_QUERY_LENGTH) {
            continue;
        }
        std::string bundleName = appInfos[0];
        std::string abilityName = appInfos[1];
        std::string queryAbilityName = bundleName.append(abilityName);
        Want want;
        want.SetElementName(appInfos[0], queryAbilityName);

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
            dialogAppInfo.abilityIconId = abilityInfo.iconId;
            dialogAppInfo.abilityLabelId = abilityInfo.labelId;
            dialogAppInfo.bundleIconId = abilityInfo.applicationInfo.iconId;
            dialogAppInfo.bundleLabelId = abilityInfo.applicationInfo.labelId;
            dialogAppInfos.emplace_back(dialogAppInfo);
        }
    }
    return ERR_OK;
}

std::vector<std::string> ImplicitStartProcessor::SplitStr(const std::string& str, char delimiter)
{
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
    TAG_LOGD(
        AAFwkTag::ABILITYMGR, "ImplicitStartExtension type: %{public}d.", static_cast<int32_t>(extensionInfo.type));
    if (extensionWhiteList.find(extensionInfo.type) == extensionWhiteList.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The extension without UI is not allowed ImplicitStart");
        return false;
    }
    return true;
}

int32_t ImplicitStartProcessor::ImplicitStartAbilityInner(const Want &targetWant,
    const AbilityRequest &request, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
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
            result = abilityMgr->ImplicitStartAbility(
                targetWant, startOptions, request.callerToken, userId, request.requestCode);
            break;
        }
        case AbilityCallType::START_SETTINGS_TYPE: {
            CHECK_POINTER_AND_RETURN(request.startSetting, ERR_INVALID_VALUE);
            result = abilityMgr->ImplicitStartAbility(
                targetWant, *request.startSetting, request.callerToken, userId, request.requestCode);
            break;
        }
        case AbilityCallType::START_EXTENSION_TYPE:
            result = abilityMgr->ImplicitStartExtensionAbility(
                targetWant, request.callerToken, userId, request.extensionType);
            break;
        default:
            result = abilityMgr->StartAbilityWrap(
                targetWant, request.callerToken, request.requestCode, userId, false, false, 0, false, true);
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

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ability:%{public}s, bundle:%{public}s", eventInfo.abilityName.c_str(),
        eventInfo.bundleName.c_str());

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
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The bundleMgrHelper is nullptr.");
        return nullptr;
    }
    auto defaultAppProxy = bundleMgrHelper->GetDefaultAppProxy();
    if (defaultAppProxy == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The defaultAppProxy is nullptr.");
        return nullptr;
    }
    return defaultAppProxy;
}

bool ImplicitStartProcessor::FilterAbilityList(const Want &want, std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos, int32_t userId)
{
    ErmsCallerInfo callerInfo;
    GetEcologicalCallerInfo(want, callerInfo, userId);
    int ret = IN_PROCESS_CALL(AbilityEcologicalRuleMgrServiceClient::GetInstance()->
        EvaluateResolveInfos(want, callerInfo, 0, abilityInfos, extensionInfos));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to evaluate resolve infos from erms.");
        return false;
    }
    return true;
}

void ImplicitStartProcessor::GetEcologicalCallerInfo(const Want &want, ErmsCallerInfo &callerInfo, int32_t userId)
{
    callerInfo.packageName = want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    callerInfo.uid = want.GetIntParam(Want::PARAM_RESV_CALLER_UID, IPCSkeleton::GetCallingUid());
    callerInfo.pid = want.GetIntParam(Want::PARAM_RESV_CALLER_PID, IPCSkeleton::GetCallingRealPid());
    callerInfo.targetAppType = ErmsCallerInfo::TYPE_INVALID;
    callerInfo.callerAppType = ErmsCallerInfo::TYPE_INVALID;

    auto bundleMgrHelper = GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get Bubndle manager helper failed.");
        return;
    }

    std::string targetBundleName = want.GetBundle();
    AppExecFwk::ApplicationInfo targetAppInfo;
    bool getTargetResult = IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(targetBundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, targetAppInfo));
    if (!getTargetResult) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get targetAppInfo failed.");
    } else if (targetAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "the target type  is atomic service");
        callerInfo.targetAppType = ErmsCallerInfo::TYPE_ATOM_SERVICE;
    } else if (targetAppInfo.bundleType == AppExecFwk::BundleType::APP) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "the target type is app");
        callerInfo.targetAppType = ErmsCallerInfo::TYPE_HARMONY_APP;
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "the target type is invalid type");
    }

    std::string callerBundleName;
    ErrCode err = IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callerInfo.uid, callerBundleName));
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get callerBundleName failed.");
        return;
    }
    AppExecFwk::ApplicationInfo callerAppInfo;
    bool getCallerResult = IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(callerBundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, callerAppInfo));
    if (!getCallerResult) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Get callerAppInfo failed.");
    } else if (callerAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "the caller type  is atomic service");
        callerInfo.callerAppType = ErmsCallerInfo::TYPE_ATOM_SERVICE;
    } else if (callerAppInfo.bundleType == AppExecFwk::BundleType::APP) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "the caller type is app");
        callerInfo.callerAppType = ErmsCallerInfo::TYPE_HARMONY_APP;
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "the caller type is invalid type");
    }
}

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

void ImplicitStartProcessor::AddAbilityInfoToDialogInfos(const AddInfoParam &param,
    std::vector<DialogAppInfo> &dialogAppInfos)
{
    if (param.isExtension && param.info.type != AbilityType::EXTENSION) {
        return;
    }
    if (!AppUtils::GetInstance().IsSelectorDialogDefaultPossion()) {
        bool isDefaultFlag = param.withDefault && IsExistDefaultApp(param.userId, param.typeName);
        if (!param.isMoreHapList && !isDefaultFlag &&
            std::find(param.infoNames.begin(), param.infoNames.end(),
            (param.info.bundleName + "#" + param.info.moduleName + "#" + param.info.name)) != param.infoNames.end()) {
            return;
        }
    }
    DialogAppInfo dialogAppInfo;
    dialogAppInfo.abilityName = param.info.name;
    dialogAppInfo.bundleName = param.info.bundleName;
    dialogAppInfo.moduleName = param.info.moduleName;
    dialogAppInfo.abilityIconId = param.info.iconId;
    dialogAppInfo.abilityLabelId = param.info.labelId;
    dialogAppInfo.bundleIconId = param.info.applicationInfo.iconId;
    dialogAppInfo.bundleLabelId = param.info.applicationInfo.labelId;
    dialogAppInfo.visible = param.info.visible;
    dialogAppInfos.emplace_back(dialogAppInfo);
}

bool ImplicitStartProcessor::IsExistDefaultApp(int32_t userId, const std::string &typeName)
{
    auto defaultMgr = GetDefaultAppProxy();
    AppExecFwk::BundleInfo bundleInfo;
    ErrCode ret =
        IN_PROCESS_CALL(defaultMgr->GetDefaultApplication(userId, typeName, bundleInfo));
    if (ret != ERR_OK) {
        return false;
    }

    if (bundleInfo.abilityInfos.size() == 1) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "find default ability.");
        return true;
    } else if (bundleInfo.extensionInfos.size() == 1) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "find default extension.");
        return true;
    } else {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "GetDefaultApplication failed.");
        return false;
    }
}

bool ImplicitStartProcessor::IsCallFromAncoShellOrBroker(const sptr<IRemoteObject> &token)
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid == BROKER_UID) {
        return true;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        return false;
    }
    std::string callerBundleName = abilityRecord->GetAbilityInfo().bundleName;
    if (callerBundleName == SHELL_ASSISTANT_BUNDLENAME) {
        return true;
    }
    return false;
}

void ImplicitStartProcessor::SetTargetLinkInfo(const std::vector<AppExecFwk::SkillUriForAbilityAndExtension> &skillUri,
    Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    for (const auto& iter : skillUri) {
        if (iter.isMatch) {
            want.RemoveParam("send_to_erms_targetLinkFeature");
            want.SetParam("send_to_erms_targetLinkFeature", iter.linkFeature);
            want.RemoveParam("send_to_erms_targetLinkType");
            if (want.GetBoolParam(OPEN_LINK_APP_LINKING_ONLY, false)) {
                want.SetParam("send_to_erms_targetLinkType", AbilityCallerInfo::LINK_TYPE_UNIVERSAL_LINK);
            } else if ((iter.scheme == "https" || iter.scheme == "http") &&
                want.GetAction().compare(ACTION_VIEW) == 0) {
                want.SetParam("send_to_erms_targetLinkType", AbilityCallerInfo::LINK_TYPE_WEB_LINK);
            } else {
                want.SetParam("send_to_erms_targetLinkType", AbilityCallerInfo::LINK_TYPE_DEEP_LINK);
            }
        }
    }
}
}  // namespace AAFwk
}  // namespace OHOS
