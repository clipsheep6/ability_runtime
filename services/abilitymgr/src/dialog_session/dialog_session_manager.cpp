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
#include <string>
#include <chrono>
#include "ability_manager_service.h"
#include "ability_record.h"
#include "ability_util.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "int_wrapper.h"
#include "modal_system_ui_extension.h"
#include "parameters.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AAFwk {
using OHOS::AppExecFwk::BundleInfo;
namespace {
constexpr const char* UIEXTENSION_MODAL_TYPE = "ability.want.params.modalType";
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

void DialogSessionManager::SetDialogSessionInfo(const std::string dialogSessionId,
    sptr<DialogSessionInfo> &dilogSessionInfo, std::shared_ptr<DialogCallerInfo> &dialogCallerInfo)
{
    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    dialogSessionInfoMap_[dialogSessionId] = dilogSessionInfo;
    dialogCallerInfoMap_[dialogSessionId] = dialogCallerInfo;
}

sptr<DialogSessionInfo> DialogSessionManager::GetDialogSessionInfo(const std::string dialogSessionId) const
{
    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    auto it = dialogSessionInfoMap_.find(dialogSessionId);
    if (it != dialogSessionInfoMap_.end()) {
        return it->second;
    }
    TAG_LOGI(AAFwkTag::DIALOG, "not find");
    return nullptr;
}

std::shared_ptr<DialogCallerInfo> DialogSessionManager::GetDialogCallerInfo(const std::string dialogSessionId) const
{
    std::lock_guard<ffrt::mutex> guard(dialogSessionRecordLock_);
    auto it = dialogCallerInfoMap_.find(dialogSessionId);
    if (it != dialogCallerInfoMap_.end()) {
        return it->second;
    }
    TAG_LOGI(AAFwkTag::DIALOG, "not find");
    return nullptr;
}

void DialogSessionManager::ClearDialogContext(const std::string dialogSessionId)
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

bool DialogSessionManager::GenerateDialogSessionRecord(AbilityRequest &abilityRequest, int32_t userId,
    std::string &dialogSessionId, std::vector<DialogAppInfo> &dialogAppInfos, bool isSelector)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto dialogSessionInfo = sptr<DialogSessionInfo>::MakeSptr();
    CHECK_POINTER_AND_RETURN(dialogSessionInfo, ERR_INVALID_VALUE);
    sptr<IRemoteObject> callerToken = abilityRequest.callerToken;
    if (callerToken != nullptr) {
        auto callerRecord = Token::GetAbilityRecordByToken(callerToken);
        CHECK_POINTER_AND_RETURN(callerRecord, ERR_INVALID_VALUE);
        dialogSessionInfo->callerAbilityInfo.bundleName = callerRecord->GetAbilityInfo().bundleName;
        dialogSessionInfo->callerAbilityInfo.moduleName = callerRecord->GetAbilityInfo().moduleName;
        dialogSessionInfo->callerAbilityInfo.abilityName = callerRecord->GetAbilityInfo().name;
        dialogSessionInfo->callerAbilityInfo.abilityIconId = callerRecord->GetAbilityInfo().iconId;
        dialogSessionInfo->callerAbilityInfo.abilityLabelId = callerRecord->GetAbilityInfo().labelId;
        dialogSessionInfo->callerAbilityInfo.bundleIconId = callerRecord->GetApplicationInfo().iconId;
        dialogSessionInfo->callerAbilityInfo.bundleLabelId = callerRecord->GetApplicationInfo().labelId;
    }
    dialogSessionInfo->parameters.SetParam("deviceType", AAFwk::String::Box(OHOS::system::GetDeviceType()));
    dialogSessionInfo->parameters.SetParam("userId", AAFwk::Integer::Box(userId));
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
        dialogSessionInfo->targetAbilityInfos.emplace_back(targetDialogAbilityInfo);
    }
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = std::make_shared<DialogCallerInfo>();
    if (isSelector) {
        dialogSessionInfo->parameters.SetParam("action", AAFwk::String::Box(abilityRequest.want.GetAction()));
        dialogSessionInfo->parameters.SetParam("wantType", AAFwk::String::Box(abilityRequest.want.GetType()));
        dialogSessionInfo->parameters.SetParam("uri", AAFwk::String::Box(abilityRequest.want.GetUriString()));
        dialogCallerInfo->isSelector = true;
    }
    dialogCallerInfo->callerToken = callerToken;
    dialogCallerInfo->requestCode = abilityRequest.requestCode;
    dialogCallerInfo->targetWant = abilityRequest.want;
    dialogCallerInfo->userId = userId;
    dialogSessionId = GenerateDialogSessionId();
    SetDialogSessionInfo(dialogSessionId, dialogSessionInfo, dialogCallerInfo);
    return true;
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

int DialogSessionManager::CreateJumpModalDialog(AbilityRequest &abilityRequest, int32_t userId,
    const Want &replaceWant)
{
    std::string dialogSessionId;
    std::vector<DialogAppInfo> dialogAppInfos(1);
    dialogAppInfos.front().bundleName = abilityRequest.abilityInfo.bundleName;
    dialogAppInfos.front().moduleName = abilityRequest.abilityInfo.moduleName;
    dialogAppInfos.front().abilityName = abilityRequest.abilityInfo.name;
    dialogAppInfos.front().abilityIconId = abilityRequest.abilityInfo.iconId;
    dialogAppInfos.front().abilityLabelId = abilityRequest.abilityInfo.labelId;
    dialogAppInfos.front().bundleIconId = abilityRequest.abilityInfo.applicationInfo.iconId;
    dialogAppInfos.front().bundleLabelId = abilityRequest.abilityInfo.applicationInfo.labelId;
    if (!GenerateDialogSessionRecord(abilityRequest, userId, dialogSessionId, dialogAppInfos, false)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "generate dialog session record failed");
        return INNER_ERR;
    }
    return CreateModalDialogCommon(replaceWant, abilityRequest.callerToken, dialogSessionId);
}

int DialogSessionManager::CreateSelectorModalDialog(AbilityRequest &abilityRequest, const Want &want, int32_t userId,
    std::vector<DialogAppInfo> &dialogAppInfos)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string dialogSessionId;
    if (!GenerateDialogSessionRecord(abilityRequest, userId, dialogSessionId, dialogAppInfos, true)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "generate dialog session record failed");
        return INNER_ERR;
    }
    return CreateModalDialogCommon(want, abilityRequest.callerToken, dialogSessionId);
}

int DialogSessionManager::CreateModalDialogCommon(const Want &replaceWant, sptr<IRemoteObject> callerToken,
    std::string dialogSessionId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    (const_cast<Want &>(replaceWant)).SetParam("dialogSessionId", dialogSessionId);
    auto connection = std::make_shared<OHOS::Rosen::ModalSystemUiExtension>();
    if (callerToken == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "create modal ui extension for system");
        (const_cast<Want &>(replaceWant)).SetParam(UIEXTENSION_MODAL_TYPE, 1);
        return connection->CreateModalUIExtension(replaceWant) ? ERR_OK : INNER_ERR;
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
        return connection->CreateModalUIExtension(replaceWant) ? ERR_OK : INNER_ERR;
    }

    if (callerRecord->GetAbilityInfo().type == AppExecFwk::AbilityType::PAGE && token == callerToken) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "create modal ui extension for application");
        return callerRecord->CreateModalUIExtension(replaceWant);
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "create modal ui extension for system");
    (const_cast<Want &>(replaceWant)).SetParam(UIEXTENSION_MODAL_TYPE, 1);
    return connection->CreateModalUIExtension(replaceWant) ? ERR_OK : INNER_ERR;
}
}  // namespace AAFwk
}  // namespace OHOS
