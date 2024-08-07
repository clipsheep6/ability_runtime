/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "dialog_session_manager.h"

#include <random>
#include "ability_manager_service.h"
#include "ability_util.h"
#include "hitrace_meter.h"
#include "int_wrapper.h"
#include "modal_system_ui_extension.h"
#include "start_ability_utils.h"
#include "string_wrapper.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
using OHOS::AppExecFwk::BundleInfo;
namespace {
constexpr const char* UIEXTENSION_MODAL_TYPE = "ability.want.params.modalType";
constexpr int32_t ERMS_ISALLOW_RESULTCODE = 10;
constexpr const char* SUPPORT_CLOSE_ON_BLUR = "supportCloseOnBlur";
}

DialogSessionManager &DialogSessionManager::GetInstance()
{
    static DialogSessionManager instance;
    return instance;
}

std::string DialogSessionManager::GenerateDialogSessionId()
{
    auto timestamp = std::chrono::system_clock::now().time_since_epoch();
    auto time = std::chrono::duration_cast<std::chrono::seconds>(timestamp).count();
    std::random_device seed;
    std::mt19937 rng(seed());
    std::uniform_int_distribution<int> uni(0, INT_MAX);
    int randomDigit = uni(rng);
    return std::to_string(time) + "_" + std::to_string(randomDigit);
    std::string dialogSessionId = std::to_string(time) + "_" + std::to_string(randomDigit);

    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    auto iter = dialogSessionInfoMap_.find(dialogSessionId);
    while (iter != dialogSessionInfoMap_.end()) {
        dialogSessionId += "_1";
        iter = dialogSessionInfoMap_.find(dialogSessionId);
    }
    return dialogSessionId;
}

void DialogSessionManager::SetDialogSessionInfo(const std::string &dialogSessionId,
    sptr<DialogSessionInfo> &dilogSessionInfo, std::shared_ptr<DialogCallerInfo> &dialogCallerInfo)
{
    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    dialogSessionInfoMap_[dialogSessionId] = dilogSessionInfo;
    dialogCallerInfoMap_[dialogSessionId] = dialogCallerInfo;
}

sptr<DialogSessionInfo> DialogSessionManager::GetDialogSessionInfo(const std::string &dialogSessionId) const
{
    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    auto it = dialogSessionInfoMap_.find(dialogSessionId);
    if (it != dialogSessionInfoMap_.end()) {
        return it->second;
    }
    TAG_LOGI(AAFwkTag::DIALOG, "not find");
    return nullptr;
}

std::shared_ptr<DialogCallerInfo> DialogSessionManager::GetDialogCallerInfo(const std::string &dialogSessionId) const
{
    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    auto it = dialogCallerInfoMap_.find(dialogSessionId);
    if (it != dialogCallerInfoMap_.end()) {
        return it->second;
    }
    TAG_LOGI(AAFwkTag::DIALOG, "not find");
    return nullptr;
}

void DialogSessionManager::ClearDialogContext(const std::string &dialogSessionId)
{
    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    auto it = dialogSessionInfoMap_.find(dialogSessionId);
    if (it != dialogSessionInfoMap_.end()) {
        dialogSessionInfoMap_.erase(it);
    }
    auto iter = dialogCallerInfoMap_.find(dialogSessionId);
    if (iter != dialogCallerInfoMap_.end()) {
        dialogCallerInfoMap_.erase(iter);
    }
    return;
}

void DialogSessionManager::ClearAllDialogContexts()
{
    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    dialogSessionInfoMap_.clear();
    dialogCallerInfoMap_.clear();
}

void DialogSessionManager::GenerateCallerAbilityInfo(AbilityRequest &abilityRequest,
    DialogAbilityInfo &callerAbilityInfo)
{
    sptr<IRemoteObject> callerToken = abilityRequest.callerToken;
    if (callerToken != nullptr) {
        auto callerRecord = Token::GetAbilityRecordByToken(callerToken);
        CHECK_POINTER(callerRecord);
        callerAbilityInfo.bundleName = callerRecord->GetAbilityInfo().bundleName;
        callerAbilityInfo.moduleName = callerRecord->GetAbilityInfo().moduleName;
        callerAbilityInfo.abilityName = callerRecord->GetAbilityInfo().name;
        callerAbilityInfo.abilityIconId = callerRecord->GetAbilityInfo().iconId;
        callerAbilityInfo.abilityLabelId = callerRecord->GetAbilityInfo().labelId;
        callerAbilityInfo.bundleIconId = callerRecord->GetApplicationInfo().iconId;
        callerAbilityInfo.bundleLabelId = callerRecord->GetApplicationInfo().labelId;
        callerAbilityInfo.visible = callerRecord->GetAbilityInfo().visible;
        callerAbilityInfo.appIndex = callerRecord->GetApplicationInfo().appIndex;
        callerAbilityInfo.multiAppMode = callerRecord->GetApplicationInfo().multiAppMode;
    }
}

void DialogSessionManager::GenerateSelectorTargetAbilityInfos(std::vector<DialogAppInfo> &dialogAppInfos,
    std::vector<DialogAbilityInfo> &targetAbilityInfos)
{
    for (auto &dialogAppInfo : dialogAppInfos) {
        DialogAbilityInfo targetDialogAbilityInfo;
        targetDialogAbilityInfo.bundleName = dialogAppInfo.bundleName;
        targetDialogAbilityInfo.moduleName = dialogAppInfo.moduleName;
        targetDialogAbilityInfo.abilityName = dialogAppInfo.abilityName;
        targetDialogAbilityInfo.abilityIconId = dialogAppInfo.abilityIconId;
        targetDialogAbilityInfo.abilityLabelId = dialogAppInfo.abilityLabelId;
        targetDialogAbilityInfo.bundleIconId = dialogAppInfo.bundleIconId;
        targetDialogAbilityInfo.bundleLabelId = dialogAppInfo.bundleLabelId;
        targetDialogAbilityInfo.visible = dialogAppInfo.visible;
        targetDialogAbilityInfo.appIndex = dialogAppInfo.appIndex;
        targetDialogAbilityInfo.multiAppMode = dialogAppInfo.multiAppMode;
        targetAbilityInfos.emplace_back(targetDialogAbilityInfo);
    }
}

void DialogSessionManager::GenerateJumpTargetAbilityInfos(AbilityRequest &abilityRequest,
    std::vector<DialogAbilityInfo> &targetAbilityInfos)
{
    DialogAbilityInfo targetDialogAbilityInfo;
    targetDialogAbilityInfo.bundleName = abilityRequest.abilityInfo.bundleName;
    targetDialogAbilityInfo.moduleName = abilityRequest.abilityInfo.moduleName;
    targetDialogAbilityInfo.abilityName = abilityRequest.abilityInfo.name;
    targetDialogAbilityInfo.abilityIconId = abilityRequest.abilityInfo.iconId;
    targetDialogAbilityInfo.abilityLabelId = abilityRequest.abilityInfo.labelId;
    targetDialogAbilityInfo.bundleIconId = abilityRequest.abilityInfo.applicationInfo.iconId;
    targetDialogAbilityInfo.bundleLabelId = abilityRequest.abilityInfo.applicationInfo.labelId;
    targetDialogAbilityInfo.visible = abilityRequest.abilityInfo.visible;
    targetDialogAbilityInfo.appIndex = abilityRequest.abilityInfo.applicationInfo.appIndex;
    targetDialogAbilityInfo.multiAppMode = abilityRequest.abilityInfo.applicationInfo.multiAppMode;
    targetAbilityInfos.emplace_back(targetDialogAbilityInfo);
}

void DialogSessionManager::GenerateDialogCallerInfo(AbilityRequest &abilityRequest, int32_t userId,
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo, bool isSelector)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(dialogCallerInfo);
    dialogCallerInfo->isSelector = isSelector;
    dialogCallerInfo->callerToken = abilityRequest.callerToken;
    dialogCallerInfo->requestCode = abilityRequest.requestCode;
    dialogCallerInfo->targetWant = abilityRequest.want;
    dialogCallerInfo->userId = userId;
}

int DialogSessionManager::SendDialogResult(const Want &want, const std::string &dialogSessionId, bool isAllowed)
{
    if (!isAllowed) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "user refuse to jump");
        ClearDialogContext(dialogSessionId);
        return ERR_OK;
    }
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = GetDialogCallerInfo(dialogSessionId);
    if (dialogCallerInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "dialog caller info is nullptr");
        ClearDialogContext(dialogSessionId);
        return ERR_INVALID_VALUE;
    }
    auto targetWant = dialogCallerInfo->targetWant;
    targetWant.SetElement(want.GetElement());
    targetWant.SetParam("isSelector", dialogCallerInfo->isSelector);
    targetWant.SetParam("dialogSessionId", dialogSessionId);
    if (want.HasParameter(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY)) {
        int32_t appIndex = want.GetIntParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0);
        targetWant.SetParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, appIndex);
    }
    if (!targetWant.HasParameter(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY)) {
        targetWant.SetParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0);
    }
    sptr<IRemoteObject> callerToken = dialogCallerInfo->callerToken;
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!abilityMgr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityMgr is nullptr.");
        return INNER_ERR;
    }
    int ret = abilityMgr->StartAbilityAsCaller(targetWant, callerToken, callerToken, dialogCallerInfo->userId,
        dialogCallerInfo->requestCode);
    if (ret == ERR_OK) {
        ClearDialogContext(dialogSessionId);
    }
    return ret;
}

std::string DialogSessionManager::GenerateDialogSessionRecordCommon(AbilityRequest &abilityRequest, int32_t userId,
    const AAFwk::WantParams &parameters, std::vector<DialogAppInfo> &dialogAppInfos, bool isSelector)
{
    auto dialogSessionInfo = sptr<DialogSessionInfo>::MakeSptr();
    CHECK_POINTER_AND_RETURN(dialogSessionInfo, "");

    GenerateCallerAbilityInfo(abilityRequest, dialogSessionInfo->callerAbilityInfo);

    if (isSelector) {
        GenerateSelectorTargetAbilityInfos(dialogAppInfos, dialogSessionInfo->targetAbilityInfos);
    } else {
        GenerateJumpTargetAbilityInfos(abilityRequest, dialogSessionInfo->targetAbilityInfos);
    }

    dialogSessionInfo->parameters = parameters;

    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    GenerateDialogCallerInfo(abilityRequest, userId, dialogCallerInfo, isSelector);

    std::string dialogSessionId = GenerateDialogSessionId();
    SetDialogSessionInfo(dialogSessionId, dialogSessionInfo, dialogCallerInfo);

    return dialogSessionId;
}

int DialogSessionManager::CreateJumpModalDialog(AbilityRequest &abilityRequest, int32_t userId,
    const Want &replaceWant)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    AAFwk::WantParams parameters;

    parameters.SetParam("deviceType", AAFwk::String::Box(OHOS::system::GetDeviceType()));
    parameters.SetParam("userId", AAFwk::Integer::Box(userId));

    std::vector<DialogAppInfo> dialogAppInfos;
    std::string dialogSessionId = GenerateDialogSessionRecordCommon(abilityRequest, userId, parameters,
        dialogAppInfos, false);
    if (dialogSessionId == "") {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "generate dialog session record failed");
        return ERR_INVALID_VALUE;
    }

    return CreateModalDialogCommon(replaceWant, abilityRequest.callerToken, dialogSessionId);
}

int DialogSessionManager::CreateImplicitSelectorModalDialog(AbilityRequest &abilityRequest, const Want &want,
    int32_t userId, std::vector<DialogAppInfo> &dialogAppInfos)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    AAFwk::Want sessionWant;

    sessionWant.SetParam("deviceType", OHOS::system::GetDeviceType());
    sessionWant.SetParam("userId", userId);
    sessionWant.SetParam("action", abilityRequest.want.GetAction());
    sessionWant.SetParam("wantType", abilityRequest.want.GetType());
    sessionWant.SetParam("uri", abilityRequest.want.GetUriString());
    sessionWant.SetParam("entities", abilityRequest.want.GetEntities());
    sessionWant.SetParam("appselector.selectorType", static_cast<int>(SelectorType::IMPLICIT_START_SELECTOR));
    bool showCaller = abilityRequest.want.GetBoolParam("showCaller", false);
    sessionWant.SetParam("showCaller", showCaller);

    std::string dialogSessionId = GenerateDialogSessionRecordCommon(abilityRequest, userId, sessionWant.GetParams(),
        dialogAppInfos, true);
    if (dialogSessionId == "") {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "generate dialog session record failed");
        return ERR_INVALID_VALUE;
    }

    return CreateModalDialogCommon(want, abilityRequest.callerToken, dialogSessionId);
}

int DialogSessionManager::CreateCloneSelectorModalDialog(AbilityRequest &abilityRequest, const Want &want,
    int32_t userId, std::vector<DialogAppInfo> &dialogAppInfos, const std::string &replaceWant)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AAFwk::WantParams parameters;

    parameters.SetParam("deviceType", AAFwk::String::Box(OHOS::system::GetDeviceType()));
    parameters.SetParam("userId", AAFwk::Integer::Box(userId));
    parameters.SetParam("appselector.selectorType",
        AAFwk::Integer::Box(static_cast<int>(SelectorType::APP_CLONR_SELECTOR)));
    if (replaceWant !=  "") {
        parameters.SetParam("ecological.replaceWant", AAFwk::String::Box(replaceWant));
    }

    std::string dialogSessionId = GenerateDialogSessionRecordCommon(abilityRequest, userId, parameters,
        dialogAppInfos, true);
    if (dialogSessionId == "") {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "generate dialog session record failed");
        return ERR_INVALID_VALUE;
    }

    return CreateModalDialogCommon(want, abilityRequest.callerToken, dialogSessionId);
}

int DialogSessionManager::CreateModalDialogCommon(const Want &replaceWant, sptr<IRemoteObject> callerToken,
    const std::string &dialogSessionId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    (const_cast<Want &>(replaceWant)).SetParam("dialogSessionId", dialogSessionId);
    auto connection = std::make_shared<OHOS::Rosen::ModalSystemUiExtension>();
    if (callerToken == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "create modal ui extension for system");
        (const_cast<Want &>(replaceWant)).SetParam(UIEXTENSION_MODAL_TYPE, 1);
        (const_cast<Want &>(replaceWant)).SetParam(SUPPORT_CLOSE_ON_BLUR, true);
        return IN_PROCESS_CALL(connection->CreateModalUIExtension(replaceWant)) ? ERR_OK : INNER_ERR;
    }
    auto callerRecord = Token::GetAbilityRecordByToken(callerToken);
    if (!callerRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callerRecord is nullptr.");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> token;
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!abilityMgr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityMgr is nullptr.");
        return INNER_ERR;
    }
    int ret = IN_PROCESS_CALL(abilityMgr->GetTopAbility(token));
    if (ret != ERR_OK || token == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "create modal ui extension for system");
        (const_cast<Want &>(replaceWant)).SetParam(UIEXTENSION_MODAL_TYPE, 1);
        (const_cast<Want &>(replaceWant)).SetParam(SUPPORT_CLOSE_ON_BLUR, true);
        return IN_PROCESS_CALL(connection->CreateModalUIExtension(replaceWant)) ? ERR_OK : INNER_ERR;
    }

    if (callerRecord->GetAbilityInfo().type == AppExecFwk::AbilityType::PAGE && token == callerToken) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "create modal ui extension for application");
        return callerRecord->CreateModalUIExtension(replaceWant);
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "create modal ui extension for system");
    (const_cast<Want &>(replaceWant)).SetParam(UIEXTENSION_MODAL_TYPE, 1);
    (const_cast<Want &>(replaceWant)).SetParam(SUPPORT_CLOSE_ON_BLUR, true);
    return IN_PROCESS_CALL(connection->CreateModalUIExtension(replaceWant)) ? ERR_OK : INNER_ERR;
}

int DialogSessionManager::HandleErmsResult(AbilityRequest &abilityRequest, int32_t userId,
    const Want &replaceWant)
{
    std::string bundleName = abilityRequest.abilityInfo.bundleName;
    if (StartAbilityUtils::ermsResultCode < ERMS_ISALLOW_RESULTCODE ||
        !IsCreateCloneSelectorDialog(bundleName, userId)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "create jump modal dialog");
        return CreateJumpModalDialog(abilityRequest, userId, replaceWant);
    }
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!abilityMgr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityMgr is nullptr.");
        return INNER_ERR;
    }
    (const_cast<Want &>(replaceWant)).RemoveParam("ecological_experience_original_target");
    return abilityMgr->CreateCloneSelectorDialog(abilityRequest, userId, replaceWant.ToString());
}

bool DialogSessionManager::IsCreateCloneSelectorDialog(const std::string &bundleName, int32_t userId)
{
    if (StartAbilityUtils::isWantWithAppCloneIndex) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "want with app clone index.");
        StartAbilityUtils::isWantWithAppCloneIndex = false;
        return false;
    }
    auto appIndexes = StartAbilityUtils::GetCloneAppIndexes(bundleName, userId);
    if (appIndexes.empty()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "The application do not create clone index.");
        return false;
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
