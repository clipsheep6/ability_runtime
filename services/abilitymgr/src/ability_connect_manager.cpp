/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "ability_connect_manager.h"

#include <algorithm>
#include <mutex>
#include <regex>

#include "ability_connect_callback_stub.h"
#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "ability_util.h"
#include "appfreeze_manager.h"
#include "app_utils.h"
#include "assert_fault_callback_death_mgr.h"
#include "extension_config.h"
#include "hitrace_meter.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "parameter.h"
#include "session/host/include/zidl/session_interface.h"
#include "extension_record.h"
#include "ui_extension_utils.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr char EVENT_KEY_UID[] = "UID";
constexpr char EVENT_KEY_PID[] = "PID";
constexpr char EVENT_KEY_MESSAGE[] = "MSG";
constexpr char EVENT_KEY_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_KEY_PROCESS_NAME[] = "PROCESS_NAME";
const std::string DEBUG_APP = "debugApp";
const std::string FRS_APP_INDEX = "ohos.extra.param.key.frs_index";
const std::string FRS_BUNDLE_NAME = "com.ohos.formrenderservice";
const std::string UIEXTENSION_ABILITY_ID = "ability.want.params.uiExtensionAbilityId";
const std::string MAX_UINT64_VALUE = "18446744073709551615";
#ifdef SUPPORT_ASAN
const int LOAD_TIMEOUT_MULTIPLE = 150;
const int CONNECT_TIMEOUT_MULTIPLE = 45;
const int COMMAND_TIMEOUT_MULTIPLE = 75;
const int COMMAND_WINDOW_TIMEOUT_MULTIPLE = 75;
const int UI_EXTENSION_CONSUME_SESSION_TIMEOUT_MULTIPLE = 150;
#else
const int LOAD_TIMEOUT_MULTIPLE = 10;
const int CONNECT_TIMEOUT_MULTIPLE = 3;
const int COMMAND_TIMEOUT_MULTIPLE = 5;
const int COMMAND_WINDOW_TIMEOUT_MULTIPLE = 5;
const int UI_EXTENSION_CONSUME_SESSION_TIMEOUT_MULTIPLE = 10;
#endif
const int32_t AUTO_DISCONNECT_INFINITY = -1;
const std::unordered_map<std::string, std::string> trustMap = {
    { AbilityConfig::SCENEBOARD_BUNDLE_NAME, AbilityConfig::SCENEBOARD_ABILITY_NAME },
    { AbilityConfig::SYSTEM_UI_BUNDLE_NAME, AbilityConfig::SYSTEM_UI_ABILITY_NAME },
    { AbilityConfig::LAUNCHER_BUNDLE_NAME, AbilityConfig::LAUNCHER_ABILITY_NAME }
};
const std::unordered_set<std::string> FROZEN_WHITE_LIST {
    "com.huawei.hmos.huaweicast"
};
constexpr char BUNDLE_NAME_DIALOG[] = "com.ohos.amsdialog";
constexpr char ABILITY_NAME_ASSERT_FAULT_DIALOG[] = "AssertFaultDialog";

bool IsSpecialAbility(const AppExecFwk::AbilityInfo &abilityInfo)
{
    auto it = trustMap.find(abilityInfo.bundleName);
    return (it != trustMap.end() && it->second == abilityInfo.name);
}
}

AbilityConnectManager::AbilityConnectManager(int userId) : userId_(userId)
{
    uiExtensionAbilityRecordMgr_ = std::make_unique<AbilityRuntime::ExtensionRecordManager>(userId);
}

AbilityConnectManager::~AbilityConnectManager()
{}

int AbilityConnectManager::StartAbility(const AbilityRequest &abilityRequest)
{
    std::lock_guard guard(Lock_);
    return StartAbilityLocked(abilityRequest);
}

int AbilityConnectManager::TerminateAbility(const sptr<IRemoteObject> &token)
{
    std::lock_guard guard(Lock_);
    return TerminateAbilityInner(token);
}

int AbilityConnectManager::TerminateAbilityInner(const sptr<IRemoteObject> &token)
{
    auto abilityRecord = GetExtensionFromServiceMapInner(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    std::string element = abilityRecord->GetURI();
    HILOG_DEBUG("Terminate ability, ability is %{public}s.", element.c_str());
    if (IsUIExtensionAbility(abilityRecord) && !abilityRecord->IsConnectListEmpty()) {
        HILOG_INFO("There exist connection, don't terminate.");
        return ERR_OK;
    }
    MoveToTerminatingMap(abilityRecord);
    return TerminateAbilityLocked(token);
}

int AbilityConnectManager::StopServiceAbility(const AbilityRequest &abilityRequest)
{
    HILOG_INFO("call");
    std::lock_guard guard(Lock_);
    return StopServiceAbilityLocked(abilityRequest);
}

int AbilityConnectManager::StartAbilityLocked(const AbilityRequest &abilityRequest)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("ability_name:%{public}s", abilityRequest.want.GetElement().GetURI().c_str());

    std::vector<AppExecFwk::Metadata> metaData = abilityRequest.abilityInfo.metadata;
    bool isSingleton = std::any_of(metaData.begin(), metaData.end(), [](const auto &metaDataItem) {
        return metaDataItem.name == "UIExtensionAbilityLaunchTypeTemp" && metaDataItem.value == "singleton";
    });
    HILOG_DEBUG("State isSingleton: %{public}d.", isSingleton);

    std::shared_ptr<AbilityRecord> targetService;
    bool isLoadedAbility = false;
    if (UIExtensionUtils::IsUIExtension(abilityRequest.abilityInfo.extensionAbilityType) && !isSingleton) {
        auto callerAbilityRecord = AAFwk::Token::GetAbilityRecordByToken(abilityRequest.callerToken);
        if (callerAbilityRecord == nullptr) {
            HILOG_ERROR("Failed to get callerAbilityRecord.");
            return ERR_NULL_OBJECT;
        }
        std::string hostBundleName = callerAbilityRecord->GetAbilityInfo().bundleName;
        int32_t ret = GetOrCreateExtensionRecord(abilityRequest, false, hostBundleName, targetService, isLoadedAbility);
        if (ret != ERR_OK) {
            HILOG_ERROR("Failed to get or create extension record, ret: %{public}d", ret);
            return ret;
        }
    } else {
        GetOrCreateServiceRecord(abilityRequest, false, targetService, isLoadedAbility);
    }
    CHECK_POINTER_AND_RETURN(targetService, ERR_INVALID_VALUE);
    HILOG_INFO("Start ability: %{public}s", targetService->GetURI().c_str());

    targetService->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode);

    targetService->SetLaunchReason(LaunchReason::LAUNCHREASON_START_EXTENSION);

    targetService->DoBackgroundAbilityWindowDelayed(false);

    if (IsUIExtensionAbility(targetService) && abilityRequest.sessionInfo && abilityRequest.sessionInfo->sessionToken) {
        auto &remoteObj = abilityRequest.sessionInfo->sessionToken;
        uiExtensionMap_[remoteObj] = UIExtWindowMapValType(targetService, abilityRequest.sessionInfo);
        AddUIExtWindowDeathRecipient(remoteObj);
        if (!isLoadedAbility) {
            SaveUIExtRequestSessionInfo(targetService, abilityRequest.sessionInfo);
        }
    }

    if (!isLoadedAbility) {
        HILOG_DEBUG("Target service has not been loaded.");
        targetService->GrantUriPermissionForServiceExtension();
        LoadAbility(targetService);
        if (UIExtensionUtils::IsUIExtension(abilityRequest.abilityInfo.extensionAbilityType) && isSingleton) {
            HILOG_DEBUG("Start uiextension in singleton mode.");
            auto callerAbilityRecord = AAFwk::Token::GetAbilityRecordByToken(abilityRequest.callerToken);
            if (callerAbilityRecord == nullptr) {
                HILOG_ERROR("Failed to get callerAbilityRecord.");
                return ERR_NULL_OBJECT;
            }
            std::string hostBundleName = callerAbilityRecord->GetAbilityInfo().bundleName;
            int32_t inputId = abilityRequest.sessionInfo->want.GetIntParam(UIEXTENSION_ABILITY_ID,
                INVALID_EXTENSION_RECORD_ID);
            std::shared_ptr<ExtensionRecord> extensionRecord = nullptr;
            CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
            uiExtensionAbilityRecordMgr_->CreateExtensionRecord(
                targetService, hostBundleName, extensionRecord, inputId);
            HILOG_DEBUG("UIExtensionAbility id %{public}d.", inputId);
        }
    } else if (targetService->IsAbilityState(AbilityState::ACTIVE) && !IsUIExtensionAbility(targetService)) {
        // It may have been started through connect
        targetService->SetWant(abilityRequest.want);
        targetService->GrantUriPermissionForServiceExtension();
        CommandAbility(targetService);
    } else if (IsUIExtensionAbility(targetService)) {
        DoForegroundUIExtension(targetService, abilityRequest);
    } else {
        HILOG_INFO("Target service is already activating.");
        EnqueueStartServiceReq(abilityRequest);
        return ERR_OK;
    }

    sptr<Token> token = targetService->GetToken();
    sptr<Token> preToken = nullptr;
    if (targetService->GetPreAbilityRecord()) {
        preToken = targetService->GetPreAbilityRecord()->GetToken();
    }
    DelayedSingleton<AppScheduler>::GetInstance()->AbilityBehaviorAnalysis(token, preToken, 0, 1, 1);
    return ERR_OK;
}

void AbilityConnectManager::DoForegroundUIExtension(std::shared_ptr<AbilityRecord> abilityRecord,
    const AbilityRequest &abilityRequest)
{
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(abilityRequest.sessionInfo);
    HILOG_INFO("Foreground ability: %{public}s, persistentId: %{public}d", abilityRecord->GetURI().c_str(),
        abilityRequest.sessionInfo->persistentId);
    if (abilityRecord->IsReady() && !abilityRecord->IsAbilityState(AbilityState::INACTIVATING) &&
        !abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING) &&
        !abilityRecord->IsAbilityState(AbilityState::BACKGROUNDING) &&
        abilityRecord->IsAbilityWindowReady()) {
        if (abilityRecord->IsAbilityState(AbilityState::FOREGROUND)) {
            abilityRecord->SetWant(abilityRequest.want);
            CommandAbilityWindow(abilityRecord, abilityRequest.sessionInfo, WIN_CMD_FOREGROUND);
            return;
        } else {
            if (abilityRecord->GetUIExtRequestSessionInfo() == nullptr) {
                abilityRecord->SetWant(abilityRequest.want);
                SaveUIExtRequestSessionInfo(abilityRecord, abilityRequest.sessionInfo);
                DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(abilityRecord->GetToken());
                return;
            }
        }
    }
    EnqueueStartServiceReq(abilityRequest, abilityRecord->GetURI());
}

void AbilityConnectManager::SaveUIExtRequestSessionInfo(std::shared_ptr<AbilityRecord> abilityRecord,
    sptr<SessionInfo> sessionInfo)
{
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(taskHandler_);
    abilityRecord->SetUIExtRequestSessionInfo(sessionInfo);
    auto callback = [abilityRecord, connectManager = shared_from_this()]() {
        std::lock_guard guard{connectManager->Lock_};
        HILOG_ERROR("consume session timeout, abilityUri: %{public}s", abilityRecord->GetURI().c_str());
        abilityRecord->SetUIExtRequestSessionInfo(nullptr);
    };

    int consumeSessionTimeout = AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() *
        UI_EXTENSION_CONSUME_SESSION_TIMEOUT_MULTIPLE;
    std::string taskName = std::string("ConsumeSessionTimeout_") +  std::to_string(abilityRecord->GetRecordId());
    taskHandler_->SubmitTask(callback, taskName, consumeSessionTimeout);
}

void AbilityConnectManager::EnqueueStartServiceReq(const AbilityRequest &abilityRequest, const std::string &serviceUri)
{
    std::lock_guard guard(startServiceReqListLock_);
    auto abilityUri = abilityRequest.want.GetElement().GetURI();
    if (!serviceUri.empty()) {
        abilityUri = serviceUri;
    }
    HILOG_INFO("abilityUri is %{public}s", abilityUri.c_str());
    auto reqListIt = startServiceReqList_.find(abilityUri);
    if (reqListIt != startServiceReqList_.end()) {
        reqListIt->second->push_back(abilityRequest);
    } else {
        auto reqList = std::make_shared<std::list<AbilityRequest>>();
        reqList->push_back(abilityRequest);
        startServiceReqList_.emplace(abilityUri, reqList);

        CHECK_POINTER(taskHandler_);
        auto callback = [abilityUri, connectManager = shared_from_this()]() {
            std::lock_guard guard{connectManager->startServiceReqListLock_};
            auto exist = connectManager->startServiceReqList_.erase(abilityUri);
            if (exist) {
                HILOG_ERROR("Target service %{public}s start timeout", abilityUri.c_str());
            }
        };

        int connectTimeout =
            AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * CONNECT_TIMEOUT_MULTIPLE;
        taskHandler_->SubmitTask(callback, std::string("start_service_timeout:") + abilityUri,
            connectTimeout);
    }
}

int AbilityConnectManager::TerminateAbilityLocked(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("called");
    auto abilityRecord = GetExtensionFromTerminatingMapInner(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (abilityRecord->IsTerminating()) {
        HILOG_DEBUG("Ability is on terminating.");
        return ERR_OK;
    }

    if (!abilityRecord->GetConnectRecordList().empty()) {
        HILOG_INFO("Target service has been connected. Post disconnect task.");
        auto connectRecordList = abilityRecord->GetConnectRecordList();
        HandleTerminateDisconnectTask(connectRecordList);
    }

    auto timeoutTask = [abilityRecord, connectManager = shared_from_this()]() {
        HILOG_WARN("Disconnect ability terminate timeout.");
        connectManager->HandleStopTimeoutTask(abilityRecord);
    };
    abilityRecord->Terminate(timeoutTask);
    RemoveUIExtensionAbilityRecord(abilityRecord);

    return ERR_OK;
}

int AbilityConnectManager::StopServiceAbilityLocked(const AbilityRequest &abilityRequest)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("call");
    AppExecFwk::ElementName element(abilityRequest.abilityInfo.deviceId, abilityRequest.abilityInfo.bundleName,
        abilityRequest.abilityInfo.name, abilityRequest.abilityInfo.moduleName);
    auto abilityRecord = GetServiceRecordByElementNameInner(element.GetURI());
    if (FRS_BUNDLE_NAME == abilityRequest.abilityInfo.bundleName) {
        abilityRecord = GetServiceRecordByElementNameInner(
            element.GetURI() + std::to_string(abilityRequest.want.GetIntParam(FRS_APP_INDEX, 0)));
    }
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (abilityRecord->IsTerminating()) {
        HILOG_INFO("Ability is on terminating.");
        return ERR_OK;
    }

    if (!abilityRecord->GetConnectRecordList().empty()) {
        HILOG_INFO("Post disconnect task.");
        auto connectRecordList = abilityRecord->GetConnectRecordList();
        HandleTerminateDisconnectTask(connectRecordList);
    }

    TerminateRecord(abilityRecord);
    return ERR_OK;
}

int32_t AbilityConnectManager::GetOrCreateExtensionRecord(const AbilityRequest &abilityRequest, bool isCreatedByConnect,
    const std::string &hostBundleName, std::shared_ptr<AbilityRecord> &extensionRecord, bool &isLoaded)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppExecFwk::ElementName element(abilityRequest.abilityInfo.deviceId, abilityRequest.abilityInfo.bundleName,
        abilityRequest.abilityInfo.name, abilityRequest.abilityInfo.moduleName);
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    if (uiExtensionAbilityRecordMgr_->IsBelongToManager(abilityRequest.abilityInfo)) {
        int32_t ret = uiExtensionAbilityRecordMgr_->GetOrCreateExtensionRecord(
            abilityRequest, hostBundleName, extensionRecord, isLoaded);
        if (ret != ERR_OK) {
            return ret;
        }
        extensionRecord->SetCreateByConnectMode(isCreatedByConnect);
        std::string extensionRecordKey = element.GetURI() + std::to_string(extensionRecord->GetUIExtensionAbilityId());
        extensionRecord->SetURI(extensionRecordKey);
        HILOG_DEBUG("Service map add, hostBundleName:%{public}s, key: %{public}s", hostBundleName.c_str(),
            extensionRecordKey.c_str());
        serviceMap_.emplace(extensionRecordKey, extensionRecord);
        if (IsAbilityNeedKeepAlive(extensionRecord)) {
            extensionRecord->SetKeepAlive();
            extensionRecord->SetRestartTime(abilityRequest.restartTime);
            extensionRecord->SetRestartCount(abilityRequest.restartCount);
        }
        return ERR_OK;
    }
    return ERR_INVALID_VALUE;
}

void AbilityConnectManager::GetOrCreateServiceRecord(const AbilityRequest &abilityRequest,
    const bool isCreatedByConnect, std::shared_ptr<AbilityRecord> &targetService, bool &isLoadedAbility)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // lifecycle is not complete when window extension is reused
    bool noReuse = UIExtensionUtils::IsWindowExtension(abilityRequest.abilityInfo.extensionAbilityType);
    AppExecFwk::ElementName element(abilityRequest.abilityInfo.deviceId, abilityRequest.abilityInfo.bundleName,
        abilityRequest.abilityInfo.name, abilityRequest.abilityInfo.moduleName);
    auto serviceMapIter = serviceMap_.find(element.GetURI());
    std::string frsKey = "";
    if (FRS_BUNDLE_NAME == abilityRequest.abilityInfo.bundleName) {
        frsKey = element.GetURI() + std::to_string(abilityRequest.want.GetIntParam(FRS_APP_INDEX, 0));
        serviceMapIter = serviceMap_.find(frsKey);
    }
    if (noReuse && serviceMapIter != serviceMap_.end()) {
        if (FRS_BUNDLE_NAME == abilityRequest.abilityInfo.bundleName) {
            serviceMap_.erase(frsKey);
        } else {
            serviceMap_.erase(element.GetURI());
        }
        if (IsSpecialAbility(abilityRequest.abilityInfo)) {
            HILOG_INFO("Removing ability: %{public}s", element.GetURI().c_str());
        }
    }
    if (noReuse || serviceMapIter == serviceMap_.end()) {
        targetService = AbilityRecord::CreateAbilityRecord(abilityRequest);
        if (targetService) {
            targetService->SetOwnerMissionUserId(userId_);
        }

        if (isCreatedByConnect && targetService != nullptr) {
            targetService->SetCreateByConnectMode();
        }
        if (targetService && abilityRequest.abilityInfo.name == AbilityConfig::LAUNCHER_ABILITY_NAME) {
            targetService->SetLauncherRoot();
            targetService->SetKeepAlive();
            targetService->SetRestartTime(abilityRequest.restartTime);
            targetService->SetRestartCount(abilityRequest.restartCount);
        } else if (IsAbilityNeedKeepAlive(targetService)) {
            targetService->SetKeepAlive();
            targetService->SetRestartTime(abilityRequest.restartTime);
            targetService->SetRestartCount(abilityRequest.restartCount);
        }
        if (FRS_BUNDLE_NAME == abilityRequest.abilityInfo.bundleName) {
            serviceMap_.emplace(frsKey, targetService);
        } else {
            serviceMap_.emplace(element.GetURI(), targetService);
        }
        isLoadedAbility = false;
    } else {
        targetService = serviceMapIter->second;
        isLoadedAbility = true;
    }
}

void AbilityConnectManager::GetConnectRecordListFromMap(
    const sptr<IAbilityConnection> &connect, std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList)
{
    auto connectMapIter = connectMap_.find(connect->AsObject());
    if (connectMapIter != connectMap_.end()) {
        connectRecordList = connectMapIter->second;
    }
}

int32_t AbilityConnectManager::GetOrCreateTargetServiceRecord(
    const AbilityRequest &abilityRequest, const sptr<UIExtensionAbilityConnectInfo> &connectInfo,
    std::shared_ptr<AbilityRecord> &targetService, bool &isLoadedAbility)
{
    if (UIExtensionUtils::IsUIExtension(abilityRequest.abilityInfo.extensionAbilityType) && connectInfo != nullptr) {
        int32_t ret = GetOrCreateExtensionRecord(
            abilityRequest, true, connectInfo->hostBundleName, targetService, isLoadedAbility);
        if (ret != ERR_OK || targetService == nullptr) {
            HILOG_ERROR("Failed to get or create extension record.");
            return ERR_NULL_OBJECT;
        }
        connectInfo->uiExtensionAbilityId = targetService->GetUIExtensionAbilityId();
        HILOG_DEBUG("UIExtensionAbility id %{public}d.", connectInfo->uiExtensionAbilityId);
    } else {
        GetOrCreateServiceRecord(abilityRequest, true, targetService, isLoadedAbility);
    }
    CHECK_POINTER_AND_RETURN(targetService, ERR_INVALID_VALUE);
    return ERR_OK;
}

int AbilityConnectManager::ConnectAbilityLocked(const AbilityRequest &abilityRequest,
    const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken, sptr<SessionInfo> sessionInfo,
    sptr<UIExtensionAbilityConnectInfo> connectInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("callee:%{public}s.", abilityRequest.want.GetElement().GetURI().c_str());
    std::lock_guard guard(Lock_);

    // 1. get target service ability record, and check whether it has been loaded.
    std::shared_ptr<AbilityRecord> targetService;
    bool isLoadedAbility = false;
    int32_t ret = GetOrCreateTargetServiceRecord(abilityRequest, connectInfo, targetService, isLoadedAbility);
    if (ret != ERR_OK) {
        return ret;
    }
    // 2. get target connectRecordList, and check whether this callback has been connected.
    ConnectListType connectRecordList;
    GetConnectRecordListFromMap(connect, connectRecordList);
    bool isCallbackConnected = !connectRecordList.empty();
    // 3. If this service ability and callback has been connected, There is no need to connect repeatedly
    if (isLoadedAbility && (isCallbackConnected) && IsAbilityConnected(targetService, connectRecordList)) {
        HILOG_INFO("Service and callback was connected.");
        return ERR_OK;
    }

    // 4. Other cases , need to connect the service ability
    auto connectRecord = ConnectionRecord::CreateConnectionRecord(callerToken, targetService, connect);
    CHECK_POINTER_AND_RETURN(connectRecord, ERR_INVALID_VALUE);
    connectRecord->AttachCallerInfo();
    connectRecord->SetConnectState(ConnectionState::CONNECTING);
    targetService->AddConnectRecordToList(connectRecord);
    targetService->SetSessionInfo(sessionInfo);
    connectRecordList.push_back(connectRecord);
    if (isCallbackConnected) {
        RemoveConnectDeathRecipient(connect);
        connectMap_.erase(connectMap_.find(connect->AsObject()));
    }
    AddConnectDeathRecipient(connect);
    connectMap_.emplace(connect->AsObject(), connectRecordList);
    targetService->SetLaunchReason(LaunchReason::LAUNCHREASON_CONNECT_EXTENSION);

    if (UIExtensionUtils::IsWindowExtension(targetService->GetAbilityInfo().extensionAbilityType)
        && abilityRequest.sessionInfo) {
        windowExtensionMap_.emplace(connect->AsObject(),
            WindowExtMapValType(targetService->GetApplicationInfo().accessTokenId, abilityRequest.sessionInfo));
    }

    if (!isLoadedAbility) {
        LoadAbility(targetService);
    } else if (targetService->IsAbilityState(AbilityState::ACTIVE)) {
        targetService->SetWant(abilityRequest.want);
        HandleActiveAbility(targetService, connectRecord);
    } else {
        HILOG_DEBUG("Target service is activating, wait for callback");
    }

    auto token = targetService->GetToken();
    auto preToken = iface_cast<Token>(connectRecord->GetToken());
    DelayedSingleton<AppScheduler>::GetInstance()->AbilityBehaviorAnalysis(token, preToken, 0, 1, 1);
    return ret;
}

void AbilityConnectManager::HandleActiveAbility(std::shared_ptr<AbilityRecord> &targetService,
    std::shared_ptr<ConnectionRecord> &connectRecord)
{
    if (targetService == nullptr) {
        HILOG_WARN("null target service.");
        return;
    }
    if (targetService->GetConnectRecordList().size() > 1) {
        if (taskHandler_ != nullptr && targetService->GetConnRemoteObject()) {
            auto task = [connectRecord]() { connectRecord->CompleteConnect(ERR_OK); };
            taskHandler_->SubmitTask(task, TaskQoS::USER_INTERACTIVE);
        } else {
            HILOG_INFO("Target service is connecting, wait for callback");
        }
    } else {
        ConnectAbility(targetService);
    }
}

int AbilityConnectManager::DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect)
{
    std::lock_guard guard(Lock_);
    return DisconnectAbilityLocked(connect, false);
}

int AbilityConnectManager::DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect, bool force)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("call");

    // 1. check whether callback was connected.
    ConnectListType connectRecordList;
    GetConnectRecordListFromMap(connect, connectRecordList);
    if (connectRecordList.empty()) {
        HILOG_ERROR("Can't find the connect list from connect map by callback.");
        return CONNECTION_NOT_EXIST;
    }

    // 2. schedule disconnect to target service
    int result = ERR_OK;
    ConnectListType list;
    for (auto &connectRecord : connectRecordList) {
        if (connectRecord) {
            auto abilityRecord = connectRecord->GetAbilityRecord();
            CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
            HILOG_DEBUG("abilityName: %{public}s, bundleName: %{public}s",
                abilityRecord->GetAbilityInfo().name.c_str(), abilityRecord->GetAbilityInfo().bundleName.c_str());
            if (abilityRecord->GetAbilityInfo().type == AbilityType::EXTENSION) {
                RemoveExtensionDelayDisconnectTask(connectRecord);
            }
            if (connectRecord->GetCallerTokenId() != IPCSkeleton::GetCallingTokenID() &&
                static_cast<uint32_t>(IPCSkeleton::GetSelfTokenID() != IPCSkeleton::GetCallingTokenID())) {
                HILOG_WARN("The caller is inconsistent with the caller stored in the connectRecord.");
                continue;
            }

            if (force) {
                DisconnectRecordForce(list, connectRecord);
            } else {
                result = DisconnectRecordNormal(list, connectRecord);
            }

            if (result != ERR_OK) {
                HILOG_ERROR("Disconnect ability fail , ret = %{public}d.", result);
                break;
            }
        }
    }
    for (auto&& connectRecord : list) {
        RemoveConnectionRecordFromMap(connectRecord);
    }

    return result;
}

void AbilityConnectManager::TerminateRecord(std::shared_ptr<AbilityRecord> abilityRecord)
{
    auto timeoutTask = [abilityRecord, connectManager = shared_from_this()]() {
        HILOG_WARN("Disconnect ability terminate timeout.");
        connectManager->HandleStopTimeoutTask(abilityRecord);
    };

    MoveToTerminatingMap(abilityRecord);
    abilityRecord->Terminate(timeoutTask);
}

int AbilityConnectManager::DisconnectRecordNormal(ConnectListType &list,
    std::shared_ptr<ConnectionRecord> connectRecord) const
{
    auto result = connectRecord->DisconnectAbility();
    if (result != ERR_OK) {
        HILOG_ERROR("Disconnect ability fail , ret = %{public}d.", result);
        return result;
    }

    if (connectRecord->GetConnectState() == ConnectionState::DISCONNECTED) {
        HILOG_WARN("This record: %{public}d complete disconnect directly.", connectRecord->GetRecordId());
        connectRecord->CompleteDisconnect(ERR_OK, false);
        list.emplace_back(connectRecord);
    }
    return ERR_OK;
}

void AbilityConnectManager::DisconnectRecordForce(ConnectListType &list,
    std::shared_ptr<ConnectionRecord> connectRecord)
{
    auto abilityRecord = connectRecord->GetAbilityRecord();
    if (abilityRecord == nullptr) {
        HILOG_ERROR("Disconnect force abilityRecord null");
        return;
    }
    abilityRecord->RemoveConnectRecordFromList(connectRecord);
    connectRecord->CompleteDisconnect(ERR_OK, true);
    list.emplace_back(connectRecord);
    if (abilityRecord->IsConnectListEmpty() && abilityRecord->GetStartId() == 0) {
        HILOG_WARN("Force terminate ability record state: %{public}d.", abilityRecord->GetAbilityState());
        TerminateRecord(abilityRecord);
    }
}

int AbilityConnectManager::AttachAbilityThreadLocked(
    const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard guard(Lock_);
    auto abilityRecord = GetExtensionFromServiceMapInner(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (taskHandler_ != nullptr) {
        int recordId = abilityRecord->GetRecordId();
        std::string taskName = std::string("LoadTimeout_") + std::to_string(recordId);
        taskHandler_->CancelTask(taskName);
    }
    if (eventHandler_) {
        eventHandler_->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG,
            abilityRecord->GetAbilityRecordId());
    }
    std::string element = abilityRecord->GetURI();
    HILOG_DEBUG("Ability: %{public}s", element.c_str());
    if (abilityRecord->IsSceneBoard()) {
        HILOG_INFO("Attach Ability: %{public}s", element.c_str());
    }
    abilityRecord->SetScheduler(scheduler);
    if (IsUIExtensionAbility(abilityRecord) && !abilityRecord->IsCreateByConnect()) {
        DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token);
    } else {
        abilityRecord->Inactivate();
    }

    return ERR_OK;
}

void AbilityConnectManager::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state)
{
    HILOG_DEBUG("state: %{public}d", state);
    std::lock_guard guard(Lock_);
    AppAbilityState abilityState = DelayedSingleton<AppScheduler>::GetInstance()->ConvertToAppAbilityState(state);
    if (abilityState == AppAbilityState::ABILITY_STATE_FOREGROUND) {
        auto abilityRecord = GetExtensionFromServiceMapInner(token);
        CHECK_POINTER(abilityRecord);
        if (!IsUIExtensionAbility(abilityRecord)) {
            HILOG_ERROR("Not ui extension.");
            return;
        }
        if (abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING)) {
            HILOG_WARN("abilityRecord is foregrounding.");
            return;
        }
        std::string element = abilityRecord->GetURI();
        HILOG_DEBUG("Ability is %{public}s, start to foreground.", element.c_str());
        MoveToForeground(abilityRecord);
    }
}

void AbilityConnectManager::OnAppStateChanged(const AppInfo &info)
{
    std::lock_guard guard(Lock_);
    std::for_each(serviceMap_.begin(), serviceMap_.end(), [&info](ServiceMapType::reference service) {
        if (service.second && (info.processName == service.second->GetAbilityInfo().process ||
                                  info.processName == service.second->GetApplicationInfo().bundleName)) {
            auto appName = service.second->GetApplicationInfo().name;
            auto uid = service.second->GetAbilityInfo().applicationInfo.uid;
            auto isExist = [&appName, &uid](
                               const AppData &appData) { return appData.appName == appName && appData.uid == uid; };
            auto iter = std::find_if(info.appData.begin(), info.appData.end(), isExist);
            if (iter != info.appData.end()) {
                service.second->SetAppState(info.state);
            }
        }
    });
}

int AbilityConnectManager::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard guard(Lock_);
    int targetState = AbilityRecord::ConvertLifeCycleToAbilityState(static_cast<AbilityLifeCycleState>(state));
    std::string abilityState = AbilityRecord::ConvertAbilityState(static_cast<AbilityState>(targetState));
    std::shared_ptr<AbilityRecord> abilityRecord;
    if (targetState == AbilityState::INACTIVE
        || targetState == AbilityState::FOREGROUND
        || targetState == AbilityState::BACKGROUND) {
        abilityRecord = GetExtensionFromServiceMapInner(token);
    } else if (targetState == AbilityState::INITIAL) {
        abilityRecord = GetExtensionFromTerminatingMapInner(token);
    } else {
        abilityRecord = nullptr;
    }
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    std::string element = abilityRecord->GetURI();
    HILOG_DEBUG("Ability: %{public}s, state: %{public}s", element.c_str(), abilityState.c_str());

    switch (targetState) {
        case AbilityState::INACTIVE: {
            if (abilityRecord->GetAbilityInfo().type == AbilityType::SERVICE) {
                DelayedSingleton<AppScheduler>::GetInstance()->UpdateAbilityState(
                    token, AppExecFwk::AbilityState::ABILITY_STATE_CREATE);
            } else {
                DelayedSingleton<AppScheduler>::GetInstance()->UpdateExtensionState(
                    token, AppExecFwk::ExtensionState::EXTENSION_STATE_CREATE);
                auto preloadTask = [owner = weak_from_this(), abilityRecord] {
                    auto acm = owner.lock();
                    if (acm == nullptr) {
                        HILOG_ERROR("AbilityConnectManager is nullptr.");
                        return;
                    }
                    acm->ProcessPreload(abilityRecord);
                };
                if (taskHandler_ != nullptr) {
                    taskHandler_->SubmitTask(preloadTask);
                }
            }
            return DispatchInactive(abilityRecord, state);
        }
        case AbilityState::FOREGROUND: {
            return DispatchForeground(abilityRecord);
        }
        case AbilityState::BACKGROUND: {
            return DispatchBackground(abilityRecord);
        }
        case AbilityState::INITIAL: {
            if (abilityRecord->GetAbilityInfo().type == AbilityType::SERVICE) {
                DelayedSingleton<AppScheduler>::GetInstance()->UpdateAbilityState(
                    token, AppExecFwk::AbilityState::ABILITY_STATE_TERMINATED);
            } else {
                DelayedSingleton<AppScheduler>::GetInstance()->UpdateExtensionState(
                    token, AppExecFwk::ExtensionState::EXTENSION_STATE_TERMINATED);
            }
            return DispatchTerminate(abilityRecord);
        }
        default: {
            HILOG_WARN("Don't support transiting state: %{public}d", state);
            return ERR_INVALID_VALUE;
        }
    }
}

void AbilityConnectManager::ProcessPreload(const std::shared_ptr<AbilityRecord> &record) const
{
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER(bundleMgrHelper);
    auto abilityInfo = record->GetAbilityInfo();
    Want want;
    want.SetElementName(abilityInfo.deviceId, abilityInfo.bundleName, abilityInfo.name, abilityInfo.moduleName);
    auto uid = record->GetUid();
    want.SetParam("uid", uid);
    bundleMgrHelper->ProcessPreload(want);
}

int AbilityConnectManager::ScheduleConnectAbilityDoneLocked(
    const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard guard(Lock_);
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    std::string element = abilityRecord->GetURI();
    HILOG_DEBUG("Connect ability done, ability: %{public}s.", element.c_str());

    if ((!abilityRecord->IsAbilityState(AbilityState::INACTIVE)) &&
        (!abilityRecord->IsAbilityState(AbilityState::ACTIVE))) {
        HILOG_ERROR("Ability record state is not inactive ,state: %{public}d", abilityRecord->GetAbilityState());
        return INVALID_CONNECTION_STATE;
    }

    if (abilityRecord->GetAbilityInfo().type == AbilityType::SERVICE) {
        DelayedSingleton<AppScheduler>::GetInstance()->UpdateAbilityState(
            token, AppExecFwk::AbilityState::ABILITY_STATE_CONNECTED);
    } else {
        DelayedSingleton<AppScheduler>::GetInstance()->UpdateExtensionState(
            token, AppExecFwk::ExtensionState::EXTENSION_STATE_CONNECTED);
    }

    abilityRecord->SetConnRemoteObject(remoteObject);
    // There may be multiple callers waiting for the connection result
    auto connectRecordList = abilityRecord->GetConnectRecordList();
    for (auto &connectRecord : connectRecordList) {
        connectRecord->ScheduleConnectAbilityDone();
        if (abilityRecord->GetAbilityInfo().type == AbilityType::EXTENSION &&
            abilityRecord->GetAbilityInfo().extensionAbilityType != AppExecFwk::ExtensionAbilityType::SERVICE) {
            PostExtensionDelayDisconnectTask(connectRecord);
        }
    }
    CompleteStartServiceReq(abilityRecord->GetURI());
    return ERR_OK;
}

int AbilityConnectManager::ScheduleDisconnectAbilityDoneLocked(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard guard(Lock_);
    auto abilityRecord = GetExtensionFromServiceMapInner(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, CONNECTION_NOT_EXIST);

    auto connect = abilityRecord->GetDisconnectingRecord();
    CHECK_POINTER_AND_RETURN(connect, CONNECTION_NOT_EXIST);

    if (!abilityRecord->IsAbilityState(AbilityState::ACTIVE)) {
        if (IsUIExtensionAbility(abilityRecord) && (abilityRecord->IsForeground() ||
            abilityRecord->IsAbilityState(AbilityState::BACKGROUND) ||
            abilityRecord->IsAbilityState(AbilityState::BACKGROUNDING))) {
            // uiextension ability support connect and start, so the ability state maybe others
            HILOG_INFO("Disconnect when ability state is %{public}d", abilityRecord->GetAbilityState());
        } else {
            HILOG_ERROR("The service ability state is not active ,state: %{public}d", abilityRecord->GetAbilityState());
            return INVALID_CONNECTION_STATE;
        }
    }

    if (abilityRecord->GetAbilityInfo().type == AbilityType::SERVICE) {
        DelayedSingleton<AppScheduler>::GetInstance()->UpdateAbilityState(
            token, AppExecFwk::AbilityState::ABILITY_STATE_DISCONNECTED);
    } else {
        DelayedSingleton<AppScheduler>::GetInstance()->UpdateExtensionState(
            token, AppExecFwk::ExtensionState::EXTENSION_STATE_DISCONNECTED);
    }

    std::string element = abilityRecord->GetURI();
    HILOG_DEBUG("Disconnect ability done, service:%{public}s.", element.c_str());

    // complete disconnect and remove record from conn map
    connect->ScheduleDisconnectAbilityDone();
    abilityRecord->RemoveConnectRecordFromList(connect);
    if (abilityRecord->IsConnectListEmpty() && abilityRecord->GetStartId() == 0) {
        if (IsUIExtensionAbility(abilityRecord) && CheckUIExtensionAbilitySessionExistLocked(abilityRecord)) {
            HILOG_INFO("There exist ui extension component, don't terminate when disconnect.");
        } else {
            HILOG_DEBUG("Service ability has no any connection, and not started, need terminate.");
            RemoveUIExtensionAbilityRecord(abilityRecord);
            if (!IsSceneBoard(abilityRecord)) {
                TerminateRecord(abilityRecord);
            }
        }
    }
    RemoveConnectionRecordFromMap(connect);

    return ERR_OK;
}

int AbilityConnectManager::ScheduleCommandAbilityDoneLocked(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard guard(Lock_);
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    std::string element = abilityRecord->GetURI();
    HILOG_DEBUG("Ability: %{public}s", element.c_str());

    if ((!abilityRecord->IsAbilityState(AbilityState::INACTIVE)) &&
        (!abilityRecord->IsAbilityState(AbilityState::ACTIVE))) {
        HILOG_ERROR("Ability record state is not inactive ,state: %{public}d", abilityRecord->GetAbilityState());
        return INVALID_CONNECTION_STATE;
    }
    // complete command and pop waiting start ability from queue.
    CompleteCommandAbility(abilityRecord);

    return ERR_OK;
}

int AbilityConnectManager::ScheduleCommandAbilityWindowDone(
    const sptr<IRemoteObject> &token,
    const sptr<SessionInfo> &sessionInfo,
    WindowCommand winCmd,
    AbilityCommand abilityCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard guard(Lock_);
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(sessionInfo, ERR_INVALID_VALUE);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    std::string element = abilityRecord->GetURI();
    HILOG_DEBUG("Ability: %{public}s, persistentId: %{private}d, winCmd: %{public}d, abilityCmd: %{public}d",
        element.c_str(), sessionInfo->persistentId, winCmd, abilityCmd);

    // Only foreground mode need cancel, cause only foreground CommandAbilityWindow post timeout task.
    if (taskHandler_ && winCmd == WIN_CMD_FOREGROUND) {
        int recordId = abilityRecord->GetRecordId();
        std::string taskName = std::string("CommandWindowTimeout_") + std::to_string(recordId) + std::string("_") +
                               std::to_string(sessionInfo->persistentId) + std::string("_") + std::to_string(winCmd);
        taskHandler_->CancelTask(taskName);
    }

    if (winCmd == WIN_CMD_DESTROY) {
        HandleCommandDestroy(sessionInfo);
    }

    abilityRecord->SetAbilityWindowState(sessionInfo, winCmd, true);

    CompleteStartServiceReq(element);
    return ERR_OK;
}

void AbilityConnectManager::HandleCommandDestroy(const sptr<SessionInfo> &sessionInfo)
{
    if (sessionInfo == nullptr) {
        HILOG_WARN("null session info.");
        return;
    }
    if (sessionInfo->sessionToken) {
        RemoveUIExtWindowDeathRecipient(sessionInfo->sessionToken);
        size_t ret = uiExtensionMap_.erase(sessionInfo->sessionToken);
        if (ret > 0) {
            return;
        }

        for (auto& item : windowExtensionMap_) {
            auto sessionInfoVal = item.second.second;
            if (sessionInfoVal && sessionInfoVal->callerToken == sessionInfo->sessionToken) {
                windowExtensionMap_.erase(item.first);
                break;
            }
        }
    }
}

void AbilityConnectManager::CompleteCommandAbility(std::shared_ptr<AbilityRecord> abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    if (taskHandler_) {
        int recordId = abilityRecord->GetRecordId();
        std::string taskName = std::string("CommandTimeout_") + std::to_string(recordId) + std::string("_") +
                               std::to_string(abilityRecord->GetStartId());
        taskHandler_->CancelTask(taskName);
    }

    abilityRecord->SetAbilityState(AbilityState::ACTIVE);

    // manage queued request
    CompleteStartServiceReq(abilityRecord->GetURI());
    if (abilityRecord->NeedConnectAfterCommand()) {
        ConnectAbility(abilityRecord);
    }
}

void AbilityConnectManager::CompleteStartServiceReq(const std::string &serviceUri)
{
    std::shared_ptr<std::list<OHOS::AAFwk::AbilityRequest>> reqList;
    {
        std::lock_guard guard(startServiceReqListLock_);
        auto it = startServiceReqList_.find(serviceUri);
        if (it != startServiceReqList_.end()) {
            reqList = it->second;
            startServiceReqList_.erase(it);
            if (taskHandler_) {
                taskHandler_->CancelTask(std::string("start_service_timeout:") + serviceUri);
            }
        }
    }

    if (reqList) {
        HILOG_INFO("Target service is activating : %{public}zu, uri: %{public}s", reqList->size(), serviceUri.c_str());
        for (const auto &req: *reqList) {
            StartAbilityLocked(req);
        }
    }
}

std::shared_ptr<AbilityRecord> AbilityConnectManager::GetServiceRecordByElementName(const std::string &element)
{
    std::lock_guard guard(Lock_);
    return GetServiceRecordByElementNameInner(element);
}

std::shared_ptr<AbilityRecord> AbilityConnectManager::GetServiceRecordByElementNameInner(const std::string &element)
{
    auto mapIter = serviceMap_.find(element);
    if (mapIter != serviceMap_.end()) {
        return mapIter->second;
    }
    return nullptr;
}

std::shared_ptr<AbilityRecord> AbilityConnectManager::GetExtensionByTokenFromServiceMap(
    const sptr<IRemoteObject> &token)
{
    std::lock_guard guard(Lock_);
    return GetExtensionFromServiceMapInner(token);
}

std::shared_ptr<AbilityRecord> AbilityConnectManager::GetExtensionFromServiceMapInner(
    const sptr<IRemoteObject> &token)
{
    auto IsMatch = [token](auto service) {
        if (!service.second) {
            return false;
        }
        sptr<IRemoteObject> srcToken = service.second->GetToken();
        return srcToken == token;
    };
    auto serviceRecord = std::find_if(serviceMap_.begin(), serviceMap_.end(), IsMatch);
    if (serviceRecord != serviceMap_.end()) {
        return serviceRecord->second;
    }
    return nullptr;
}

std::shared_ptr<AbilityRecord> AbilityConnectManager::GetExtensionFromServiceMapInner(
    int32_t abilityRecordId)
{
    for (const auto &[key, value] : serviceMap_) {
        if (value && value->GetAbilityRecordId() == abilityRecordId) {
            return value;
        }
    }
    return nullptr;
}

std::shared_ptr<AbilityRecord> AbilityConnectManager::GetUIExtensioBySessionInfo(
    const sptr<SessionInfo> &sessionInfo)
{
    std::lock_guard guard(Lock_);
    CHECK_POINTER_AND_RETURN(sessionInfo, nullptr);
    auto sessionToken = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    CHECK_POINTER_AND_RETURN(sessionToken, nullptr);
    std::string descriptor = Str16ToStr8(sessionToken->GetDescriptor());
    if (descriptor != "OHOS.ISession") {
        HILOG_ERROR("Input token is not a sessionToken, token->GetDescriptor(): %{public}s",
            descriptor.c_str());
        return nullptr;
    }

    auto it = uiExtensionMap_.find(sessionToken->AsObject());
    if (it != uiExtensionMap_.end()) {
        auto abilityRecord = it->second.first.lock();
        if (abilityRecord == nullptr) {
            HILOG_WARN("abilityRecord is nullptr.");
            RemoveUIExtWindowDeathRecipient(sessionToken->AsObject());
            uiExtensionMap_.erase(it);
            return nullptr;
        }
        auto savedSessionInfo = it->second.second;
        if (!savedSessionInfo || savedSessionInfo->sessionToken != sessionInfo->sessionToken
            || savedSessionInfo->callerToken != sessionInfo->callerToken) {
            HILOG_WARN("Inconsistent sessionInfo.");
            return nullptr;
        }
        return abilityRecord;
    } else {
        HILOG_ERROR("UIExtension not found.");
    }
    return nullptr;
}

std::shared_ptr<AbilityRecord> AbilityConnectManager::GetExtensionByTokenFromTerminatingMap(
    const sptr<IRemoteObject> &token)
{
    std::lock_guard guard(Lock_);
    return GetExtensionFromTerminatingMapInner(token);
}

std::shared_ptr<AbilityRecord> AbilityConnectManager::GetExtensionFromTerminatingMapInner(
    const sptr<IRemoteObject> &token)
{
    auto IsMatch = [token](auto& extension) {
        if (extension.second == nullptr) {
            return false;
        }
        auto&& terminatingToken = extension.second->GetToken();
        if (terminatingToken != nullptr) {
            return terminatingToken->AsObject() == token;
        }
        return false;
    };

    auto terminatingExtensionRecord =
        std::find_if(terminatingExtensionMap_.begin(), terminatingExtensionMap_.end(), IsMatch);
    if (terminatingExtensionRecord != terminatingExtensionMap_.end()) {
        return terminatingExtensionRecord->second;
    }
    return nullptr;
}

std::list<std::shared_ptr<ConnectionRecord>> AbilityConnectManager::GetConnectRecordListByCallback(
    sptr<IAbilityConnection> callback)
{
    std::lock_guard guard(Lock_);
    std::list<std::shared_ptr<ConnectionRecord>> connectList;
    auto connectMapIter = connectMap_.find(callback->AsObject());
    if (connectMapIter != connectMap_.end()) {
        connectList = connectMapIter->second;
    }
    return connectList;
}

std::shared_ptr<AbilityRecord> AbilityConnectManager::GetAbilityRecordById(int64_t abilityRecordId)
{
    auto IsMatch = [abilityRecordId](auto service) {
        if (!service.second) {
            return false;
        }
        return abilityRecordId == service.second->GetAbilityRecordId();
    };
    auto serviceRecord = std::find_if(serviceMap_.begin(), serviceMap_.end(), IsMatch);
    if (serviceRecord != serviceMap_.end()) {
        return serviceRecord->second;
    }
    return nullptr;
}

void AbilityConnectManager::LoadAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(abilityRecord);
    abilityRecord->SetStartTime();

    if (!abilityRecord->CanRestartRootLauncher()) {
        HILOG_ERROR("Root launcher restart is out of max count.");
        RemoveServiceAbility(abilityRecord);
        return;
    }
    if (!abilityRecord->IsDebugApp()) {
        HILOG_DEBUG("IsDebug is false, here is not debug app");
        PostTimeOutTask(abilityRecord, AbilityManagerService::LOAD_TIMEOUT_MSG);
    }
    sptr<Token> token = abilityRecord->GetToken();
    sptr<Token> perToken = nullptr;
    if (abilityRecord->IsCreateByConnect()) {
        perToken = iface_cast<Token>(abilityRecord->GetConnectingRecord()->GetToken());
    } else {
        auto callerList = abilityRecord->GetCallerRecordList();
        if (!callerList.empty() && callerList.back()) {
            auto caller = callerList.back()->GetCaller();
            if (caller) {
                perToken = caller->GetToken();
            }
        }
    }
    DelayedSingleton<AppScheduler>::GetInstance()->LoadAbility(
        token, perToken, abilityRecord->GetAbilityInfo(), abilityRecord->GetApplicationInfo(),
        abilityRecord->GetWant());
}

void AbilityConnectManager::PostRestartResidentTask(const AbilityRequest &abilityRequest)
{
    HILOG_INFO("PostRestartResidentTask start.");
    CHECK_POINTER(taskHandler_);
    std::string taskName = std::string("RestartResident_") + std::string(abilityRequest.abilityInfo.name);
    auto task = [abilityRequest, connectManager = shared_from_this()]() {
        CHECK_POINTER(connectManager);
        connectManager->HandleRestartResidentTask(abilityRequest);
    };
    int restartIntervalTime = 0;
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (abilityMgr) {
        restartIntervalTime = AmsConfigurationParameter::GetInstance().GetRestartIntervalTime();
    }
    HILOG_DEBUG("PostRestartResidentTask, time:%{public}d", restartIntervalTime);
    taskHandler_->SubmitTask(task, taskName, restartIntervalTime);
    HILOG_INFO("PostRestartResidentTask end.");
}

void AbilityConnectManager::HandleRestartResidentTask(const AbilityRequest &abilityRequest)
{
    HILOG_INFO("HandleRestartResidentTask start.");
    std::lock_guard guard(Lock_);
    auto findRestartResidentTask = [abilityRequest](const AbilityRequest &requestInfo) {
        return (requestInfo.want.GetElement().GetBundleName() == abilityRequest.want.GetElement().GetBundleName() &&
            requestInfo.want.GetElement().GetModuleName() == abilityRequest.want.GetElement().GetModuleName() &&
            requestInfo.want.GetElement().GetAbilityName() == abilityRequest.want.GetElement().GetAbilityName());
    };
    auto findIter = find_if(restartResidentTaskList_.begin(), restartResidentTaskList_.end(), findRestartResidentTask);
    if (findIter != restartResidentTaskList_.end()) {
        restartResidentTaskList_.erase(findIter);
    }
    StartAbilityLocked(abilityRequest);
}

void AbilityConnectManager::PostTimeOutTask(const std::shared_ptr<AbilityRecord> &abilityRecord, uint32_t messageId)
{
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(taskHandler_);
    if (messageId != AbilityConnectManager::LOAD_TIMEOUT_MSG &&
        messageId != AbilityConnectManager::CONNECT_TIMEOUT_MSG) {
        HILOG_ERROR("Timeout task messageId is error.");
        return;
    }

    int recordId;
    std::string taskName;
    int resultCode;
    uint32_t delayTime;
    if (messageId == AbilityManagerService::LOAD_TIMEOUT_MSG) {
        // first load ability, There is at most one connect record.
        recordId = abilityRecord->GetRecordId();
        taskName = std::string("LoadTimeout_") + std::to_string(recordId);
        resultCode = LOAD_ABILITY_TIMEOUT;
        delayTime = AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * LOAD_TIMEOUT_MULTIPLE;
    } else {
        auto connectRecord = abilityRecord->GetConnectingRecord();
        CHECK_POINTER(connectRecord);
        recordId = connectRecord->GetRecordId();
        taskName = std::string("ConnectTimeout_") + std::to_string(recordId);
        resultCode = CONNECTION_TIMEOUT;
        delayTime = AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * CONNECT_TIMEOUT_MULTIPLE;
    }

    // check libc.hook_mode
    const int bufferLen = 128;
    char paramOutBuf[bufferLen] = {0};
    const char *hook_mode = "startup:";
    int ret = GetParameter("libc.hook_mode", "", paramOutBuf, bufferLen - 1);
    if (ret > 0 && strncmp(paramOutBuf, hook_mode, strlen(hook_mode)) == 0) {
        HILOG_DEBUG("Hook_mode: no timeoutTask");
        return;
    }

    auto timeoutTask = [abilityRecord, connectManager = shared_from_this(), resultCode]() {
        HILOG_WARN("Connect or load ability timeout.");
        connectManager->HandleStartTimeoutTask(abilityRecord, resultCode);
    };
    taskHandler_->SubmitTask(timeoutTask, taskName, delayTime);
}

void AbilityConnectManager::HandleStartTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord, int resultCode)
{
    HILOG_DEBUG("Complete connect or load ability timeout.");
    std::lock_guard guard(Lock_);
    CHECK_POINTER(abilityRecord);
    auto connectingList = abilityRecord->GetConnectingRecordList();
    for (auto &connectRecord : connectingList) {
        if (connectRecord == nullptr) {
            HILOG_WARN("ConnectRecord is nullptr.");
            continue;
        }
        connectRecord->CompleteDisconnect(ERR_OK, true);
        abilityRecord->RemoveConnectRecordFromList(connectRecord);
        RemoveConnectionRecordFromMap(connectRecord);
    }

    if (GetExtensionFromServiceMapInner(abilityRecord->GetToken()) == nullptr) {
        HILOG_ERROR("Timeout ability record is not exist in service map.");
        return;
    }
    MoveToTerminatingMap(abilityRecord);

    if (resultCode == LOAD_ABILITY_TIMEOUT) {
        HILOG_WARN("Load time out , remove target service record from services map.");
        RemoveServiceAbility(abilityRecord);
        if (abilityRecord->GetAbilityInfo().name != AbilityConfig::LAUNCHER_ABILITY_NAME) {
            DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(abilityRecord->GetToken());
            if (IsAbilityNeedKeepAlive(abilityRecord)) {
                HILOG_WARN("Load time out , try to restart.");
                RestartAbility(abilityRecord, userId_);
            }
        }
    }

    if (abilityRecord->GetAbilityInfo().name == AbilityConfig::LAUNCHER_ABILITY_NAME) {
        // terminate the timeout root launcher.
        DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(abilityRecord->GetToken());
        if (resultCode == LOAD_ABILITY_TIMEOUT) {
            StartRootLauncher(abilityRecord);
        }
    }
}

void AbilityConnectManager::HandleCommandTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HILOG_DEBUG("HandleCommandTimeoutTask start");
    std::lock_guard guard(Lock_);
    CHECK_POINTER(abilityRecord);
    if (abilityRecord->GetAbilityInfo().name == AbilityConfig::LAUNCHER_ABILITY_NAME) {
        HILOG_DEBUG("Handle root launcher command timeout.");
        // terminate the timeout root launcher.
        DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(abilityRecord->GetToken());
    }
    HILOG_DEBUG("HandleCommandTimeoutTask end");
}

void AbilityConnectManager::HandleCommandWindowTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd)
{
    HILOG_DEBUG("start");
    std::lock_guard guard(Lock_);
    CHECK_POINTER(abilityRecord);
    abilityRecord->SetAbilityWindowState(sessionInfo, winCmd, true);
    // manage queued request
    CompleteStartServiceReq(abilityRecord->GetURI());
    HILOG_DEBUG("end");
}

void AbilityConnectManager::StartRootLauncher(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    AbilityRequest requestInfo;
    requestInfo.want = abilityRecord->GetWant();
    requestInfo.abilityInfo = abilityRecord->GetAbilityInfo();
    requestInfo.appInfo = abilityRecord->GetApplicationInfo();
    requestInfo.restartTime = abilityRecord->GetRestartTime();
    requestInfo.restart = true;
    requestInfo.restartCount = abilityRecord->GetRestartCount() - 1;

    HILOG_DEBUG("restart root launcher, number:%{public}d", requestInfo.restartCount);
    StartAbilityLocked(requestInfo);
}

void AbilityConnectManager::HandleStopTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HILOG_DEBUG("Complete stop ability timeout start.");
    std::lock_guard guard(Lock_);
    CHECK_POINTER(abilityRecord);
    TerminateDone(abilityRecord);
}

void AbilityConnectManager::HandleTerminateDisconnectTask(const ConnectListType& connectlist)
{
    HILOG_DEBUG("Disconnect ability when terminate.");
    for (auto& connectRecord : connectlist) {
        if (!connectRecord) {
            continue;
        }
        auto targetService = connectRecord->GetAbilityRecord();
        if (targetService) {
            HILOG_WARN("This record complete disconnect directly. recordId:%{public}d", connectRecord->GetRecordId());
            connectRecord->CompleteDisconnect(ERR_OK, true);
            targetService->RemoveConnectRecordFromList(connectRecord);
            RemoveConnectionRecordFromMap(connectRecord);
        };
    }
}

int AbilityConnectManager::DispatchInactive(const std::shared_ptr<AbilityRecord> &abilityRecord, int state)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(eventHandler_, ERR_INVALID_VALUE);
    if (!abilityRecord->IsAbilityState(AbilityState::INACTIVATING)) {
        HILOG_ERROR("Ability transition life state error. expect %{public}d, actual %{public}d callback %{public}d",
            AbilityState::INACTIVATING,
            abilityRecord->GetAbilityState(),
            state);
        return ERR_INVALID_VALUE;
    }
    eventHandler_->RemoveEvent(AbilityManagerService::INACTIVE_TIMEOUT_MSG, abilityRecord->GetAbilityRecordId());

    // complete inactive
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    if (abilityRecord->IsCreateByConnect()) {
        ConnectAbility(abilityRecord);
    } else {
        CommandAbility(abilityRecord);
        if (abilityRecord->GetConnectRecordList().size() > 0) {
            // It means someone called connectAbility when service was loading
            ConnectAbility(abilityRecord);
        }
    }

    return ERR_OK;
}

int AbilityConnectManager::DispatchForeground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(taskHandler_, ERR_INVALID_VALUE);
    // remove foreground timeout task.
    taskHandler_->CancelTask("foreground_" + std::to_string(abilityRecord->GetAbilityRecordId()));

    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteForeground(abilityRecord); };
    taskHandler_->SubmitTask(task, TaskQoS::USER_INTERACTIVE);

    return ERR_OK;
}

int AbilityConnectManager::DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(taskHandler_, ERR_INVALID_VALUE);
    // remove background timeout task.
    taskHandler_->CancelTask("background_" + std::to_string(abilityRecord->GetAbilityRecordId()));

    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteBackground(abilityRecord); };
    taskHandler_->SubmitTask(task, TaskQoS::USER_INTERACTIVE);

    return ERR_OK;
}

int AbilityConnectManager::DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    // remove terminate timeout task
    if (taskHandler_ != nullptr) {
        taskHandler_->CancelTask("terminate_" + std::to_string(abilityRecord->GetAbilityRecordId()));
    }
    // complete terminate
    TerminateDone(abilityRecord);
    return ERR_OK;
}

void AbilityConnectManager::ConnectAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(abilityRecord);
    PostTimeOutTask(abilityRecord, AbilityConnectManager::CONNECT_TIMEOUT_MSG);
    abilityRecord->ConnectAbility();
}

void AbilityConnectManager::CommandAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (taskHandler_ != nullptr) {
        // first connect ability, There is at most one connect record.
        int recordId = abilityRecord->GetRecordId();
        abilityRecord->AddStartId();
        std::string taskName = std::string("CommandTimeout_") + std::to_string(recordId) + std::string("_") +
                               std::to_string(abilityRecord->GetStartId());
        auto timeoutTask = [abilityRecord, connectManager = shared_from_this()]() {
            HILOG_ERROR("Command ability timeout. %{public}s", abilityRecord->GetAbilityInfo().name.c_str());
            connectManager->HandleCommandTimeoutTask(abilityRecord);
        };
        int commandTimeout =
            AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * COMMAND_TIMEOUT_MULTIPLE;
        taskHandler_->SubmitTask(timeoutTask, taskName, commandTimeout);
        // scheduling command ability
        abilityRecord->CommandAbility();
    }
}

void AbilityConnectManager::CommandAbilityWindow(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(sessionInfo);
    HILOG_DEBUG("ability: %{public}s, persistentId: %{private}d, wincmd: %{public}d",
        abilityRecord->GetURI().c_str(), sessionInfo->persistentId, winCmd);
    abilityRecord->SetAbilityWindowState(sessionInfo, winCmd, false);
    if (taskHandler_ != nullptr) {
        int recordId = abilityRecord->GetRecordId();
        std::string taskName = std::string("CommandWindowTimeout_") + std::to_string(recordId) + std::string("_") +
            std::to_string(sessionInfo->persistentId) + std::string("_") + std::to_string(winCmd);
        auto timeoutTask = [abilityRecord, sessionInfo, winCmd, connectManager = shared_from_this()]() {
            HILOG_ERROR("Command window timeout. %{public}s", abilityRecord->GetAbilityInfo().name.c_str());
            connectManager->HandleCommandWindowTimeoutTask(abilityRecord, sessionInfo, winCmd);
        };
        int commandWindowTimeout =
            AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * COMMAND_WINDOW_TIMEOUT_MULTIPLE;
        taskHandler_->SubmitTask(timeoutTask, taskName, commandWindowTimeout);
        // scheduling command ability
        abilityRecord->CommandAbilityWindow(sessionInfo, winCmd);
    }
}

void AbilityConnectManager::ForegroundAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const sptr<SessionInfo> &sessionInfo)
{
    std::lock_guard guard(Lock_);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("abilityRecord is nullptr");
        return;
    }
    if (sessionInfo == nullptr) {
        HILOG_ERROR("sessionInfo is nullptr");
        return;
    }
    CommandAbilityWindow(abilityRecord, sessionInfo, WIN_CMD_FOREGROUND);
}

void AbilityConnectManager::BackgroundAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const sptr<SessionInfo> &sessionInfo)
{
    std::lock_guard guard(Lock_);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("abilityRecord is nullptr");
        return;
    }
    if (sessionInfo == nullptr) {
        HILOG_ERROR("sessionInfo is nullptr");
        return;
    }

    DoBackgroundAbilityWindow(abilityRecord, sessionInfo);
}

void AbilityConnectManager::DoBackgroundAbilityWindow(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const sptr<SessionInfo> &sessionInfo)
{
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(sessionInfo);
    HILOG_INFO("Background ability: %{public}s, persistentId: %{public}d", abilityRecord->GetURI().c_str(),
        sessionInfo->persistentId);

    std::vector<AppExecFwk::Metadata> metaData = abilityRecord->GetAbilityInfo().metadata;
    bool isSingleton = std::any_of(metaData.begin(), metaData.end(), [](const auto &metaDataItem) {
        return metaDataItem.name == "UIExtensionAbilityLaunchTypeTemp" && metaDataItem.value == "singleton";
    });
    HILOG_DEBUG("State isSingleton: %{public}d.", isSingleton);

    if (abilityRecord->IsAbilityState(AbilityState::FOREGROUND) || isSingleton) {
        MoveToBackground(abilityRecord);
    } else if (abilityRecord->IsAbilityState(AbilityState::INITIAL) ||
        abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING)) {
        HILOG_INFO("There exist initial or foregrounding task.");
        abilityRecord->DoBackgroundAbilityWindowDelayed(true);
    } else {
        HILOG_WARN("Invalid ability state when background.");
    }
}

void AbilityConnectManager::TerminateAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const sptr<SessionInfo> &sessionInfo)
{
    std::lock_guard guard(Lock_);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("abilityRecord is nullptr");
        return;
    }
    if (sessionInfo == nullptr) {
        HILOG_ERROR("sessionInfo is nullptr");
        return;
    }
    DoTerminateUIExtensionAbility(abilityRecord, sessionInfo);
}

void AbilityConnectManager::DoTerminateUIExtensionAbility(std::shared_ptr<AbilityRecord> abilityRecord,
    sptr<SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(sessionInfo);
    HILOG_INFO("Terminate ability: %{public}s, persistentId: %{public}d", abilityRecord->GetURI().c_str(),
        sessionInfo->persistentId);

    EventInfo eventInfo;
    eventInfo.bundleName = abilityRecord->GetAbilityInfo().bundleName;
    eventInfo.abilityName = abilityRecord->GetAbilityInfo().name;
    EventReport::SendAbilityEvent(EventName::TERMINATE_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    eventInfo.errCode = TerminateAbilityInner(abilityRecord->GetToken());
    if (eventInfo.errCode != ERR_OK) {
        EventReport::SendAbilityEvent(EventName::TERMINATE_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
}

void AbilityConnectManager::TerminateDone(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(abilityRecord);
    if (!abilityRecord->IsAbilityState(AbilityState::TERMINATING)) {
        std::string expect = AbilityRecord::ConvertAbilityState(AbilityState::TERMINATING);
        std::string actual = AbilityRecord::ConvertAbilityState(abilityRecord->GetAbilityState());
        HILOG_ERROR(
            "Transition life state error. expect %{public}s, actual %{public}s", expect.c_str(), actual.c_str());
        return;
    }
    IN_PROCESS_CALL_WITHOUT_RET(abilityRecord->RevokeUriPermission());
    abilityRecord->RemoveAbilityDeathRecipient();
    if (IsSceneBoard(abilityRecord)) {
        HILOG_INFO("To kill processes because scb exit.");
        KillProcessesByUserId();
    }
    DelayedSingleton<AppScheduler>::GetInstance()->TerminateAbility(abilityRecord->GetToken(), false);
    RemoveServiceAbility(abilityRecord);
}

bool AbilityConnectManager::IsAbilityConnected(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList)
{
    auto isMatch = [abilityRecord](auto connectRecord) -> bool {
        if (abilityRecord == nullptr || connectRecord == nullptr) {
            return false;
        }
        if (abilityRecord != connectRecord->GetAbilityRecord()) {
            return false;
        }
        return true;
    };
    return std::any_of(connectRecordList.begin(), connectRecordList.end(), isMatch);
}

void AbilityConnectManager::RemoveConnectionRecordFromMap(std::shared_ptr<ConnectionRecord> connection)
{
    for (auto &connectCallback : connectMap_) {
        auto &connectList = connectCallback.second;
        auto connectRecord = std::find(connectList.begin(), connectList.end(), connection);
        if (connectRecord != connectList.end()) {
            HILOG_DEBUG("connrecord(%{public}d)", (*connectRecord)->GetRecordId());
            connectList.remove(connection);
            if (connectList.empty()) {
                HILOG_DEBUG("connlist");
                sptr<IAbilityConnection> connect = iface_cast<IAbilityConnection>(connectCallback.first);
                RemoveConnectDeathRecipient(connect);
                connectMap_.erase(connectCallback.first);
            }
            return;
        }
    }
}

void AbilityConnectManager::RemoveServiceAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    HILOG_DEBUG("Remove service(%{public}s) from terminating map.", abilityRecord->GetURI().c_str());
    terminatingExtensionMap_.erase(abilityRecord->GetURI());
}

void AbilityConnectManager::AddConnectDeathRecipient(const sptr<IAbilityConnection> &connect)
{
    CHECK_POINTER(connect);
    auto connectObject = connect->AsObject();
    CHECK_POINTER(connectObject);
    {
        std::lock_guard guard(recipientMapMutex_);
        auto it = recipientMap_.find(connectObject);
        if (it != recipientMap_.end()) {
            HILOG_ERROR("This death recipient has been added.");
            return;
        }
    }

    std::weak_ptr<AbilityConnectManager> thisWeakPtr(shared_from_this());
    sptr<IRemoteObject::DeathRecipient> deathRecipient =
        new AbilityConnectCallbackRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
            auto abilityConnectManager = thisWeakPtr.lock();
            if (abilityConnectManager) {
                abilityConnectManager->OnCallBackDied(remote);
            }
        });
    if (!connectObject->AddDeathRecipient(deathRecipient)) {
        HILOG_ERROR("AddDeathRecipient failed.");
        return;
    }
    std::lock_guard guard(recipientMapMutex_);
    recipientMap_.emplace(connectObject, deathRecipient);
}

void AbilityConnectManager::RemoveConnectDeathRecipient(const sptr<IAbilityConnection> &connect)
{
    CHECK_POINTER(connect);
    auto connectObject = connect->AsObject();
    CHECK_POINTER(connectObject);
    sptr<IRemoteObject::DeathRecipient> deathRecipient;
    {
        std::lock_guard guard(recipientMapMutex_);
        auto it = recipientMap_.find(connectObject);
        if (it == recipientMap_.end()) {
            return;
        }
        deathRecipient = it->second;
        recipientMap_.erase(it);
    }

    connectObject->RemoveDeathRecipient(deathRecipient);
}

void AbilityConnectManager::OnCallBackDied(const wptr<IRemoteObject> &remote)
{
    auto object = remote.promote();
    CHECK_POINTER(object);
    if (taskHandler_) {
        auto task = [object, connectManager = shared_from_this()]() { connectManager->HandleCallBackDiedTask(object); };
        taskHandler_->SubmitTask(task, TASK_ON_CALLBACK_DIED);
    }
}

void AbilityConnectManager::HandleCallBackDiedTask(const sptr<IRemoteObject> &connect)
{
    HILOG_DEBUG("called");
    std::lock_guard guard(Lock_);
    CHECK_POINTER(connect);
    auto item = windowExtensionMap_.find(connect);
    if (item != windowExtensionMap_.end()) {
        windowExtensionMap_.erase(item);
    }
    auto it = connectMap_.find(connect);
    if (it != connectMap_.end()) {
        ConnectListType connectRecordList = it->second;
        for (auto &connRecord : connectRecordList) {
            connRecord->ClearConnCallBack();
        }
    } else {
        HILOG_INFO("Died object can't find from conn map.");
        return;
    }
    sptr<IAbilityConnection> object = iface_cast<IAbilityConnection>(connect);
    DisconnectAbilityLocked(object, true);
}

void AbilityConnectManager::OnAbilityDied(const std::shared_ptr<AbilityRecord> &abilityRecord, int32_t currentUserId)
{
    CHECK_POINTER(abilityRecord);
    HILOG_DEBUG("On ability died: %{public}s.", abilityRecord->GetURI().c_str());
    if (abilityRecord->GetAbilityInfo().type != AbilityType::SERVICE &&
        abilityRecord->GetAbilityInfo().type != AbilityType::EXTENSION) {
        HILOG_DEBUG("Ability type is not service.");
        return;
    }
    if (taskHandler_) {
        auto task = [abilityRecord, connectManager = shared_from_this(), currentUserId]() {
            connectManager->HandleAbilityDiedTask(abilityRecord, currentUserId);
        };
        taskHandler_->SubmitTask(task, TASK_ON_ABILITY_DIED);
    }
}

void AbilityConnectManager::OnTimeOut(uint32_t msgId, int64_t abilityRecordId)
{
    HILOG_DEBUG("On timeout, msgId is %{public}d", msgId);
    std::lock_guard guard(Lock_);
    auto abilityRecord = GetAbilityRecordById(abilityRecordId);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("AbilityConnectManager on time out event: ability record is nullptr.");
        return;
    }
    HILOG_DEBUG("Ability timeout ,msg:%{public}d,name:%{public}s", msgId,
        abilityRecord->GetAbilityInfo().name.c_str());

    switch (msgId) {
        case AbilityManagerService::INACTIVE_TIMEOUT_MSG:
            HandleInactiveTimeout(abilityRecord);
            break;
        default:
            break;
    }
}

void AbilityConnectManager::HandleInactiveTimeout(const std::shared_ptr<AbilityRecord> &ability)
{
    HILOG_DEBUG("HandleInactiveTimeout start");
    CHECK_POINTER(ability);
    if (ability->GetAbilityInfo().name == AbilityConfig::LAUNCHER_ABILITY_NAME) {
        HILOG_DEBUG("Handle root launcher inactive timeout.");
        // terminate the timeout root launcher.
        DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(ability->GetToken());
    }

    HILOG_DEBUG("HandleInactiveTimeout end");
}

bool AbilityConnectManager::IsAbilityNeedKeepAlive(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (IsSpecialAbility(abilityRecord->GetAbilityInfo())) {
        return true;
    }
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bundleMgrHelper, false);
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    bool getBundleInfos = bundleMgrHelper->GetBundleInfos(
        OHOS::AppExecFwk::GET_BUNDLE_DEFAULT, bundleInfos, USER_ID_NO_HEAD);
    if (!getBundleInfos) {
        HILOG_ERROR("Handle ability died task, get bundle infos failed.");
        return false;
    }

    auto CheckIsAbilityNeedKeepAlive = [](const AppExecFwk::HapModuleInfo &hapModuleInfo,
        const std::string processName, std::string &mainElement) {
        if (!hapModuleInfo.isModuleJson) {
            // old application model
            mainElement = hapModuleInfo.mainAbility;
            for (auto abilityInfo : hapModuleInfo.abilityInfos) {
                if (abilityInfo.process == processName && abilityInfo.name == mainElement) {
                    return true;
                }
            }
            return false;
        }

        // new application model
        if (hapModuleInfo.process == processName) {
            mainElement = hapModuleInfo.mainElementName;
            return true;
        }
        return false;
    };

    auto GetKeepAliveAbilities = [&](std::vector<std::pair<std::string, std::string>> &keepAliveAbilities) {
        for (size_t i = 0; i < bundleInfos.size(); i++) {
            std::string processName = bundleInfos[i].applicationInfo.process;
            if (!bundleInfos[i].isKeepAlive || processName.empty()) {
                continue;
            }
            std::string bundleName = bundleInfos[i].name;
            for (auto hapModuleInfo : bundleInfos[i].hapModuleInfos) {
                std::string mainElement;
                if (CheckIsAbilityNeedKeepAlive(hapModuleInfo, processName, mainElement) && !mainElement.empty()) {
                    keepAliveAbilities.push_back(std::make_pair(bundleName, mainElement));
                }
            }
        }
    };

    auto findKeepAliveAbility = [abilityRecord](const std::pair<std::string, std::string> &keepAlivePair) {
        return ((abilityRecord->GetAbilityInfo().bundleName == keepAlivePair.first &&
                abilityRecord->GetAbilityInfo().name == keepAlivePair.second));
    };

    std::vector<std::pair<std::string, std::string>> keepAliveAbilities;
    GetKeepAliveAbilities(keepAliveAbilities);
    auto findIter = find_if(keepAliveAbilities.begin(), keepAliveAbilities.end(), findKeepAliveAbility);
    if (findIter != keepAliveAbilities.end()) {
        abilityRecord->SetKeepAlive();
        return true;
    }
    return false;
}

void AbilityConnectManager::HandleAbilityDiedTask(
    const std::shared_ptr<AbilityRecord> &abilityRecord, int32_t currentUserId)
{
    HILOG_DEBUG("called.");
    std::lock_guard guard(Lock_);
    CHECK_POINTER(abilityRecord);
    HILOG_INFO("Ability died: %{public}s", abilityRecord->GetURI().c_str());
    abilityRecord->SetConnRemoteObject(nullptr);
    ConnectListType connlist = abilityRecord->GetConnectRecordList();
    for (auto &connectRecord : connlist) {
        HILOG_WARN("This record complete disconnect directly. recordId:%{public}d", connectRecord->GetRecordId());
        RemoveExtensionDelayDisconnectTask(connectRecord);
        connectRecord->CompleteDisconnect(ERR_OK, true);
        abilityRecord->RemoveConnectRecordFromList(connectRecord);
        RemoveConnectionRecordFromMap(connectRecord);
    }

    if (abilityRecord->IsTerminating()) {
        HILOG_INFO("Handle extension DiedByTerminating.");
        RemoveServiceAbility(abilityRecord);
        if (IsAbilityNeedKeepAlive(abilityRecord)) {
            HILOG_INFO("restart ability: %{public}s", abilityRecord->GetAbilityInfo().name.c_str());
            RestartAbility(abilityRecord, currentUserId);
        }
        return;
    }

    if (IsUIExtensionAbility(abilityRecord)) {
        HandleUIExtensionDied(abilityRecord);
    }

    auto token = abilityRecord->GetToken();
    bool isRemove = false;
    if (GetExtensionFromServiceMapInner(abilityRecord->GetAbilityRecordId()) != nullptr) {
        MoveToTerminatingMap(abilityRecord);
        RemoveServiceAbility(abilityRecord);
        isRemove = true;
    }

    if (IsAbilityNeedKeepAlive(abilityRecord)) {
        HILOG_INFO("restart ability: %{public}s", abilityRecord->GetAbilityInfo().name.c_str());
        if ((IsLauncher(abilityRecord) || IsSceneBoard(abilityRecord)) && token != nullptr) {
            IN_PROCESS_CALL_WITHOUT_RET(DelayedSingleton<AppScheduler>::GetInstance()->ClearProcessByToken(
                token->AsObject()));
        }
        RestartAbility(abilityRecord, currentUserId);
    } else {
        if (isRemove) {
            HandleNotifyAssertFaultDialogDied(abilityRecord);
        }
    }
}

static bool CheckIsNumString(const std::string &numStr)
{
    const std::regex regexJsperf(R"(^\d*)");
    std::match_results<std::string::const_iterator> matchResults;
    if (numStr.empty() || !std::regex_match(numStr, matchResults, regexJsperf)) {
        HILOG_ERROR("Number parsing error, %{public}s.", numStr.c_str());
        return false;
    }
    if (MAX_UINT64_VALUE.length() < numStr.length() ||
        (MAX_UINT64_VALUE.length() == numStr.length() && MAX_UINT64_VALUE.compare(numStr) < 0)) {
        HILOG_ERROR("Number parsing error, %{public}s.", numStr.c_str());
        return false;
    }

    return true;
}

void AbilityConnectManager::HandleNotifyAssertFaultDialogDied(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HILOG_DEBUG("Called.");
    CHECK_POINTER(abilityRecord);
    if (abilityRecord->GetAbilityInfo().name != ABILITY_NAME_ASSERT_FAULT_DIALOG ||
        abilityRecord->GetAbilityInfo().bundleName != BUNDLE_NAME_DIALOG) {
        HILOG_ERROR("Is not assert fault dialog.");
        return;
    }

    auto want = abilityRecord->GetWant();
    auto assertSessionStr = want.GetStringParam(Want::PARAM_ASSERT_FAULT_SESSION_ID);
    if (!CheckIsNumString(assertSessionStr)) {
        HILOG_ERROR("Check assert session str is number failed.");
        return;
    }

    auto callbackDeathMgr = DelayedSingleton<AbilityRuntime::AssertFaultCallbackDeathMgr>::GetInstance();
    if (callbackDeathMgr == nullptr) {
        HILOG_ERROR("Get callback death manager instance is nullptr.");
        return;
    }
    callbackDeathMgr->CallAssertFaultCallback(std::stoull(assertSessionStr));
}

void AbilityConnectManager::HandleUIExtensionDied(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HILOG_DEBUG("called");
    CHECK_POINTER(abilityRecord);
    for (auto it = uiExtensionMap_.begin(); it != uiExtensionMap_.end();) {
        std::shared_ptr<AbilityRecord> uiExtAbility = it->second.first.lock();
        if (uiExtAbility == nullptr) {
            HILOG_WARN("uiExtAbility is nullptr");
            RemoveUIExtWindowDeathRecipient(it->first);
            it = uiExtensionMap_.erase(it);
            continue;
        }

        if (abilityRecord == uiExtAbility) {
            sptr<Rosen::ISession> sessionProxy = iface_cast<Rosen::ISession>(it->first);
            if (sessionProxy) {
                HILOG_DEBUG("start NotifyExtensionDied");
                sessionProxy->NotifyExtensionDied();
            }
            RemoveUIExtWindowDeathRecipient(it->first);
            it = uiExtensionMap_.erase(it);
            continue;
        }
        it++;
    }
}

void AbilityConnectManager::RestartAbility(const std::shared_ptr<AbilityRecord> &abilityRecord, int32_t currentUserId)
{
    HILOG_INFO("Restart ability: %{public}s.", abilityRecord->GetURI().c_str());
    AbilityRequest requestInfo;
    requestInfo.want = abilityRecord->GetWant();
    requestInfo.abilityInfo = abilityRecord->GetAbilityInfo();
    requestInfo.appInfo = abilityRecord->GetApplicationInfo();
    requestInfo.restartTime = abilityRecord->GetRestartTime();
    requestInfo.restart = true;
    abilityRecord->SetRestarting(true);

    if (AppUtils::GetInstance().IsLauncherAbility(abilityRecord->GetAbilityInfo().name)) {
        if (currentUserId != userId_) {
            HILOG_WARN("delay restart root launcher until switch user.");
            return;
        }
        requestInfo.want.SetParam("ohos.app.recovery", true);
        requestInfo.restartCount = abilityRecord->GetRestartCount();
        HILOG_DEBUG("restart root launcher, number:%{public}d", requestInfo.restartCount);
        StartAbilityLocked(requestInfo);
        return;
    }

    // restart other resident ability
    if (abilityRecord->CanRestartResident()) {
        requestInfo.restartCount = abilityRecord->GetRestartCount();
        requestInfo.restartTime = AbilityUtil::SystemTimeMillis();
        StartAbilityLocked(requestInfo);
    } else {
        auto findRestartResidentTask = [requestInfo](const AbilityRequest &abilityRequest) {
            return (requestInfo.want.GetElement().GetBundleName() == abilityRequest.want.GetElement().GetBundleName() &&
                requestInfo.want.GetElement().GetModuleName() == abilityRequest.want.GetElement().GetModuleName() &&
                requestInfo.want.GetElement().GetAbilityName() == abilityRequest.want.GetElement().GetAbilityName());
        };
        auto findIter = find_if(restartResidentTaskList_.begin(), restartResidentTaskList_.end(),
            findRestartResidentTask);
        if (findIter != restartResidentTaskList_.end()) {
            HILOG_WARN("The restart task has been registered.");
            return;
        }
        restartResidentTaskList_.emplace_back(requestInfo);
        PostRestartResidentTask(requestInfo);
    }
}

void AbilityConnectManager::DumpState(std::vector<std::string> &info, bool isClient, const std::string &args)
{
    HILOG_INFO("args:%{public}s.", args.c_str());
    ServiceMapType serviceMapBack;
    {
        std::lock_guard guard(Lock_);
        serviceMapBack = serviceMap_;
    }
    if (!args.empty()) {
        auto it = std::find_if(serviceMapBack.begin(), serviceMapBack.end(), [&args](const auto &service) {
            return service.first.compare(args) == 0;
        });
        if (it != serviceMapBack.end()) {
            info.emplace_back("uri [ " + it->first + " ]");
            if (it->second != nullptr) {
                it->second->DumpService(info, isClient);
            }
        } else {
            info.emplace_back(args + ": Nothing to dump.");
        }
    } else {
        info.emplace_back("  ExtensionRecords:");
        for (auto &&service : serviceMapBack) {
            info.emplace_back("    uri [" + service.first + "]");
            if (service.second != nullptr) {
                service.second->DumpService(info, isClient);
            }
        }
    }
}

void AbilityConnectManager::DumpStateByUri(std::vector<std::string> &info, bool isClient, const std::string &args,
    std::vector<std::string> &params)
{
    HILOG_INFO("args:%{public}s, params size: %{public}zu", args.c_str(), params.size());
    std::shared_ptr<AbilityRecord> extensionAbilityRecord = nullptr;
    {
        std::lock_guard guard(Lock_);
        auto it = std::find_if(serviceMap_.begin(), serviceMap_.end(), [&args](const auto &service) {
            return service.first.compare(args) == 0;
        });
        if (it != serviceMap_.end()) {
            info.emplace_back("uri [ " + it->first + " ]");
            extensionAbilityRecord = it->second;
        } else {
            info.emplace_back(args + ": Nothing to dump.");
        }
    }
    if (extensionAbilityRecord != nullptr) {
        extensionAbilityRecord->DumpService(info, params, isClient);
    }
}

void AbilityConnectManager::GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info,
    const int32_t userId, bool isPerm)
{
    HILOG_DEBUG("Get extension running info.");
    std::lock_guard guard(Lock_);
    auto mgr = shared_from_this();
    auto queryInfo = [&info, upperLimit, userId, isPerm, mgr](ServiceMapType::reference service) {
        if (static_cast<int>(info.size()) >= upperLimit) {
            return;
        }
        auto abilityRecord = service.second;
        CHECK_POINTER(abilityRecord);

        if (isPerm) {
            mgr->GetExtensionRunningInfo(abilityRecord, userId, info);
        } else {
            auto callingTokenId = IPCSkeleton::GetCallingTokenID();
            auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
            if (callingTokenId == tokenID) {
                mgr->GetExtensionRunningInfo(abilityRecord, userId, info);
            }
        }
    };
    std::for_each(serviceMap_.begin(), serviceMap_.end(), queryInfo);
}

void AbilityConnectManager::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm)
{
    HILOG_DEBUG("call");
    std::lock_guard guard(Lock_);

    auto queryInfo = [&info, isPerm](ServiceMapType::reference service) {
        auto abilityRecord = service.second;
        CHECK_POINTER(abilityRecord);

        if (isPerm) {
            DelayedSingleton<AbilityManagerService>::GetInstance()->GetAbilityRunningInfo(info, abilityRecord);
        } else {
            auto callingTokenId = IPCSkeleton::GetCallingTokenID();
            auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
            if (callingTokenId == tokenID) {
                DelayedSingleton<AbilityManagerService>::GetInstance()->GetAbilityRunningInfo(info, abilityRecord);
            }
        }
    };

    std::for_each(serviceMap_.begin(), serviceMap_.end(), queryInfo);
}

void AbilityConnectManager::GetExtensionRunningInfo(std::shared_ptr<AbilityRecord> &abilityRecord,
    const int32_t userId, std::vector<ExtensionRunningInfo> &info)
{
    ExtensionRunningInfo extensionInfo;
    AppExecFwk::RunningProcessInfo processInfo;
    extensionInfo.extension = abilityRecord->GetElementName();
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER(bundleMgrHelper);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    bool queryResult = IN_PROCESS_CALL(bundleMgrHelper->QueryExtensionAbilityInfos(abilityRecord->GetWant(),
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId, extensionInfos));
    if (queryResult) {
        HILOG_DEBUG("Success");
        auto abilityInfo = abilityRecord->GetAbilityInfo();
        auto isExist = [&abilityInfo](const AppExecFwk::ExtensionAbilityInfo &extensionInfo) {
            HILOG_DEBUG("%{public}s, %{public}s", extensionInfo.bundleName.c_str(), extensionInfo.name.c_str());
            return extensionInfo.bundleName == abilityInfo.bundleName && extensionInfo.name == abilityInfo.name
                && extensionInfo.applicationInfo.uid == abilityInfo.applicationInfo.uid;
        };
        auto infoIter = std::find_if(extensionInfos.begin(), extensionInfos.end(), isExist);
        if (infoIter != extensionInfos.end()) {
            HILOG_DEBUG("Get target success.");
            extensionInfo.type = (*infoIter).type;
        }
    }
    DelayedSingleton<AppScheduler>::GetInstance()->
        GetRunningProcessInfoByToken(abilityRecord->GetToken(), processInfo);
    extensionInfo.pid = processInfo.pid_;
    extensionInfo.uid = processInfo.uid_;
    extensionInfo.processName = processInfo.processName_;
    extensionInfo.startTime = abilityRecord->GetStartTime();
    ConnectListType connectRecordList = abilityRecord->GetConnectRecordList();
    for (auto &connectRecord : connectRecordList) {
        if (connectRecord == nullptr) {
            HILOG_DEBUG("connectRecord is nullptr.");
            continue;
        }
        auto callerAbilityRecord = Token::GetAbilityRecordByToken(connectRecord->GetToken());
        if (callerAbilityRecord == nullptr) {
            HILOG_DEBUG("callerAbilityRecord is nullptr.");
            continue;
        }
        std::string package = callerAbilityRecord->GetAbilityInfo().bundleName;
        extensionInfo.clientPackage.emplace_back(package);
    }
    info.emplace_back(extensionInfo);
}

void AbilityConnectManager::PauseExtensions()
{
    HILOG_DEBUG("begin.");
    std::lock_guard guard(Lock_);
    for (auto it = serviceMap_.begin(); it != serviceMap_.end();) {
        auto targetExtension = it->second;
        if (targetExtension != nullptr && targetExtension->GetAbilityInfo().type == AbilityType::EXTENSION &&
            (IsLauncher(targetExtension) || IsSceneBoard(targetExtension))) {
            terminatingExtensionMap_.emplace(it->first, it->second);
            serviceMap_.erase(it++);
            HILOG_INFO("terminate ability:%{public}s.", targetExtension->GetAbilityInfo().name.c_str());
            TerminateAbilityLocked(targetExtension->GetToken());
        } else {
            it++;
        }
    }
}

void AbilityConnectManager::RemoveLauncherDeathRecipient()
{
    HILOG_INFO("Call.");
    std::lock_guard guard(Lock_);
    for (auto it = serviceMap_.begin(); it != serviceMap_.end();) {
        auto targetExtension = it->second;
        if (targetExtension != nullptr && targetExtension->GetAbilityInfo().type == AbilityType::EXTENSION &&
            (IsLauncher(targetExtension) || IsSceneBoard(targetExtension))) {
            targetExtension->RemoveAbilityDeathRecipient();
            break;
        } else {
            it++;
        }
    }
}

bool AbilityConnectManager::IsLauncher(std::shared_ptr<AbilityRecord> serviceExtension) const
{
    if (serviceExtension == nullptr) {
        HILOG_ERROR("param is nullptr");
        return false;
    }
    return serviceExtension->GetAbilityInfo().name == AbilityConfig::LAUNCHER_ABILITY_NAME &&
        serviceExtension->GetAbilityInfo().bundleName == AbilityConfig::LAUNCHER_BUNDLE_NAME;
}

bool AbilityConnectManager::IsSceneBoard(std::shared_ptr<AbilityRecord> serviceExtension) const
{
    if (serviceExtension == nullptr) {
        HILOG_ERROR("param is nullptr");
        return false;
    }
    return serviceExtension->GetAbilityInfo().name == AbilityConfig::SCENEBOARD_ABILITY_NAME &&
        serviceExtension->GetAbilityInfo().bundleName == AbilityConfig::SCENEBOARD_BUNDLE_NAME;
}

void AbilityConnectManager::KillProcessesByUserId() const
{
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (appScheduler == nullptr) {
        HILOG_ERROR("appScheduler is nullptr");
        return;
    }
    IN_PROCESS_CALL_WITHOUT_RET(appScheduler->KillProcessesByUserId(userId_));
}

void AbilityConnectManager::MoveToForeground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("ability record is null.");
        return;
    }

    auto self(weak_from_this());
    auto task = [abilityRecord, self]() {
        auto selfObj = self.lock();
        if (selfObj == nullptr) {
            HILOG_WARN("mgr is invalid.");
            return;
        }
        HILOG_ERROR("move to foreground timeout.");
        selfObj->PrintTimeOutLog(abilityRecord, AbilityManagerService::FOREGROUND_TIMEOUT_MSG);
        selfObj->HandleForegroundTimeoutTask(abilityRecord);
    };
    auto sessionInfo = abilityRecord->GetUIExtRequestSessionInfo();
    if (sessionInfo != nullptr) {
        abilityRecord->ForegroundAbility(task, sessionInfo);
        abilityRecord->SetUIExtRequestSessionInfo(nullptr);
    } else {
        HILOG_WARN("SessionInfo is nullptr. Move to background");
        abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
        DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(abilityRecord->GetToken());
    }
    if (taskHandler_) {
        taskHandler_->CancelTask(std::string("ConsumeSessionTimeout_") +  std::to_string(abilityRecord->GetRecordId()));
    }
}

void AbilityConnectManager::MoveToBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("Move the ui extension ability to background fail, ability record is null.");
        return;
    }
    HILOG_DEBUG("Move the ui extension ability to background, ability:%{public}s.",
        abilityRecord->GetAbilityInfo().name.c_str());
    abilityRecord->SetIsNewWant(false);

    auto self(weak_from_this());
    auto task = [abilityRecord, self]() {
        auto selfObj = self.lock();
        if (selfObj == nullptr) {
            HILOG_WARN("mgr is invalid.");
            return;
        }
        HILOG_ERROR("move to background timeout.");
        selfObj->PrintTimeOutLog(abilityRecord, AbilityManagerService::BACKGROUND_TIMEOUT_MSG);
        selfObj->CompleteBackground(abilityRecord);
    };
    abilityRecord->BackgroundAbility(task);
}

void AbilityConnectManager::CompleteForeground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    std::lock_guard guard(Lock_);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("abilityRecord is nullptr");
        return;
    }
    if (abilityRecord->GetAbilityState() != AbilityState::FOREGROUNDING) {
        HILOG_ERROR("Ability state is %{public}d, it can't complete foreground.", abilityRecord->GetAbilityState());
        return;
    }

    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
    if (abilityRecord->BackgroundAbilityWindowDelayed()) {
        HILOG_INFO("Response background request.");
        abilityRecord->DoBackgroundAbilityWindowDelayed(false);
        DoBackgroundAbilityWindow(abilityRecord, abilityRecord->GetSessionInfo());
    }
    CompleteStartServiceReq(abilityRecord->GetURI());
}

void AbilityConnectManager::HandleForegroundTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    std::lock_guard guard(Lock_);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("abilityRecord is nullptr");
        return;
    }
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    abilityRecord->DoBackgroundAbilityWindowDelayed(false);
    CompleteStartServiceReq(abilityRecord->GetURI());
}

void AbilityConnectManager::CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    std::lock_guard guard(Lock_);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("abilityRecord is nullptr");
        return;
    }
    if (abilityRecord->GetAbilityState() != AbilityState::BACKGROUNDING) {
        HILOG_ERROR("Ability state is %{public}d, it can't complete background.", abilityRecord->GetAbilityState());
        return;
    }

    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    // send application state to AppMS.
    // notify AppMS to update application state.
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(abilityRecord->GetToken());
    CompleteStartServiceReq(abilityRecord->GetURI());
}

void AbilityConnectManager::PrintTimeOutLog(const std::shared_ptr<AbilityRecord> &ability, uint32_t msgId)
{
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }

    AppExecFwk::RunningProcessInfo processInfo = {};
    DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByToken(ability->GetToken(), processInfo);
    if (processInfo.pid_ == 0) {
        HILOG_ERROR("error: the ability[%{public}s], app may fork fail or not running.",
            ability->GetAbilityInfo().name.data());
        return;
    }
    int typeId = AppExecFwk::AppfreezeManager::TypeAttribute::NORMAL_TIMEOUT;
    std::string msgContent = "ability:" + ability->GetAbilityInfo().name + " ";
    switch (msgId) {
        case AbilityManagerService::LOAD_TIMEOUT_MSG:
            msgContent += "load timeout";
            typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        case AbilityManagerService::ACTIVE_TIMEOUT_MSG:
            msgContent += "active timeout";
            break;
        case AbilityManagerService::INACTIVE_TIMEOUT_MSG:
            msgContent += "inactive timeout";
            break;
        case AbilityManagerService::FOREGROUND_TIMEOUT_MSG:
            msgContent += "foreground timeout";
            typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        case AbilityManagerService::BACKGROUND_TIMEOUT_MSG:
            msgContent += "background timeout";
            break;
        case AbilityManagerService::TERMINATE_TIMEOUT_MSG:
            msgContent += "terminate timeout";
            break;
        default:
            return;
    }

    HILOG_WARN("LIFECYCLE_TIMEOUT: uid: %{public}d, pid: %{public}d, bundleName: %{public}s, abilityName: %{public}s,"
        "msg: %{public}s", processInfo.uid_, processInfo.pid_, ability->GetAbilityInfo().bundleName.c_str(),
        ability->GetAbilityInfo().name.c_str(), msgContent.c_str());
    AppExecFwk::AppfreezeManager::ParamInfo info = {
        .typeId = typeId,
        .pid = processInfo.pid_,
        .eventName = AppExecFwk::AppFreezeType::LIFECYCLE_TIMEOUT,
        .bundleName = ability->GetAbilityInfo().bundleName,
        .msg = msgContent
    };
    AppExecFwk::AppfreezeManager::GetInstance()->LifecycleTimeoutHandle(info);
}

void AbilityConnectManager::MoveToTerminatingMap(const std::shared_ptr<AbilityRecord>& abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    auto& abilityInfo = abilityRecord->GetAbilityInfo();
    terminatingExtensionMap_.emplace(abilityRecord->GetURI(), abilityRecord);
    if (FRS_BUNDLE_NAME == abilityInfo.bundleName) {
        AppExecFwk::ElementName element(abilityInfo.deviceId, abilityInfo.bundleName, abilityInfo.name,
            abilityInfo.moduleName);
        serviceMap_.erase(
            element.GetURI() + std::to_string(abilityRecord->GetWant().GetIntParam(FRS_APP_INDEX, 0)));
    } else {
        serviceMap_.erase(abilityRecord->GetURI());
    }
    if (IsSpecialAbility(abilityRecord->GetAbilityInfo())) {
        HILOG_INFO("Moving ability: %{public}s", abilityRecord->GetURI().c_str());
    }
}

void AbilityConnectManager::AddUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session)
{
    CHECK_POINTER(session);
    auto it = uiExtRecipientMap_.find(session);
    if (it != uiExtRecipientMap_.end()) {
        HILOG_ERROR("This death recipient has been added.");
        return;
    } else {
        std::weak_ptr<AbilityConnectManager> thisWeakPtr(shared_from_this());
        sptr<IRemoteObject::DeathRecipient> deathRecipient =
            new AbilityConnectCallbackRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto abilityConnectManager = thisWeakPtr.lock();
                if (abilityConnectManager) {
                    abilityConnectManager->OnUIExtWindowDied(remote);
                }
            });
        if (!session->AddDeathRecipient(deathRecipient)) {
            HILOG_ERROR("AddDeathRecipient failed.");
        }
        uiExtRecipientMap_.emplace(session, deathRecipient);
    }
}

void AbilityConnectManager::RemoveUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session)
{
    CHECK_POINTER(session);
    auto it = uiExtRecipientMap_.find(session);
    if (it != uiExtRecipientMap_.end() && it->first != nullptr) {
        it->first->RemoveDeathRecipient(it->second);
        uiExtRecipientMap_.erase(it);
        return;
    }
}

void AbilityConnectManager::OnUIExtWindowDied(const wptr<IRemoteObject> &remote)
{
    auto object = remote.promote();
    CHECK_POINTER(object);
    if (taskHandler_) {
        auto task = [object, connectManager = shared_from_this()]() {
            connectManager->HandleUIExtWindowDiedTask(object);
        };
        taskHandler_->SubmitTask(task);
    }
}

void AbilityConnectManager::HandleUIExtWindowDiedTask(const sptr<IRemoteObject> &remote)
{
    HILOG_DEBUG("call.");
    std::lock_guard guard(Lock_);
    CHECK_POINTER(remote);
    auto it = uiExtensionMap_.find(remote);
    if (it != uiExtensionMap_.end()) {
        auto abilityRecord = it->second.first.lock();
        if (abilityRecord) {
            DoTerminateUIExtensionAbility(abilityRecord, it->second.second);
        } else {
            HILOG_INFO("abilityRecord is nullptr");
        }
        RemoveUIExtWindowDeathRecipient(remote);
        uiExtensionMap_.erase(it);
    } else {
        HILOG_INFO("Died object can't find from map.");
        return;
    }
}

bool AbilityConnectManager::IsUIExtensionFocused(uint32_t uiExtensionTokenId, const sptr<IRemoteObject>& focusToken)
{
    HILOG_DEBUG("called, id: %{public}u", uiExtensionTokenId);
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, false);
    std::lock_guard guard(Lock_);
    for (auto& item: uiExtensionMap_) {
        auto uiExtension = item.second.first.lock();
        auto sessionInfo = item.second.second;
        if (uiExtension && uiExtension->GetApplicationInfo().accessTokenId == uiExtensionTokenId) {
            if (uiExtensionAbilityRecordMgr_->IsFocused(uiExtension->GetUIExtensionAbilityId(), focusToken)) {
                HILOG_INFO("id: %{public}u, isFocused.", uiExtensionTokenId);
                return true;
            }
            if (sessionInfo && sessionInfo->callerToken == focusToken) {
                return true;
            }
        }
    }
    return false;
}

sptr<IRemoteObject> AbilityConnectManager::GetUIExtensionSourceToken(const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("Called");
    std::lock_guard guard(Lock_);
    for (auto &item : uiExtensionMap_) {
        auto sessionInfo = item.second.second;
        auto uiExtension = item.second.first.lock();
        if (sessionInfo != nullptr && uiExtension->GetToken() != nullptr &&
            uiExtension->GetToken()->AsObject() == token) {
            HILOG_DEBUG("The source token found.");
            return sessionInfo->callerToken;
        }
    }
    return nullptr;
}

bool AbilityConnectManager::IsWindowExtensionFocused(uint32_t extensionTokenId, const sptr<IRemoteObject>& focusToken)
{
    std::lock_guard guard(Lock_);
    for (auto& item: windowExtensionMap_) {
        uint32_t windowExtTokenId = item.second.first;
        auto sessionInfo = item.second.second;
        if (windowExtTokenId == extensionTokenId && sessionInfo && sessionInfo->callerToken == focusToken) {
            return true;
        }
    }
    return false;
}

void AbilityConnectManager::HandleProcessFrozen(const std::vector<int32_t> &pidList, int32_t uid)
{
    auto taskHandler = taskHandler_;
    if (!taskHandler) {
        HILOG_ERROR("taskHandler null");
        return;
    }
    HILOG_INFO("HandleProcessFrozen: %{public}d", uid);
    std::unordered_set<int32_t> pidSet(pidList.begin(), pidList.end());
    std::lock_guard guard(Lock_);
    auto weakthis = weak_from_this();
    for (auto [key, abilityRecord] : serviceMap_) {
        if (abilityRecord && abilityRecord->GetUid() == uid &&
            abilityRecord->GetAbilityInfo().extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE &&
            pidSet.count(abilityRecord->GetPid()) > 0 &&
            FROZEN_WHITE_LIST.count(abilityRecord->GetAbilityInfo().bundleName) == 0 &&
            abilityRecord->IsConnectListEmpty() &&
            !abilityRecord->GetKeepAlive() &&
            abilityRecord->GetStartId() != 0) { // To be honest, this is expected to be true
            taskHandler->SubmitTask([weakthis, record = abilityRecord]() {
                    auto connectManager = weakthis.lock();
                    if (record && connectManager) {
                        HILOG_INFO("TerminateRecord: %{public}s", record->GetAbilityInfo().bundleName.c_str());
                        std::lock_guard guard(connectManager->Lock_);
                        connectManager->TerminateRecord(record);
                    } else {
                        HILOG_ERROR("connectManager null");
                    }
                });
        }
    }
}

void AbilityConnectManager::PostExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord)
{
    HILOG_DEBUG("call");
    CHECK_POINTER(taskHandler_);
    CHECK_POINTER(connectRecord);
    int32_t recordId = connectRecord->GetRecordId();
    std::string taskName = std::string("DelayDisconnectTask_") + std::to_string(recordId);

    auto abilityRecord = connectRecord->GetAbilityRecord();
    CHECK_POINTER(abilityRecord);
    auto typeName = abilityRecord->GetAbilityInfo().extensionTypeName;
    int32_t delayTime = DelayedSingleton<ExtensionConfig>::GetInstance()->GetExtensionAutoDisconnectTime(typeName);
    if (delayTime == AUTO_DISCONNECT_INFINITY) {
        HILOG_DEBUG("This extension needn't auto disconnect.");
        return;
    }

    auto task = [connectRecord, self = weak_from_this()]() {
        auto selfObj = self.lock();
        if (selfObj == nullptr) {
            HILOG_WARN("mgr is invalid.");
            return;
        }
        HILOG_WARN("Auto disconnect the Extension's connection.");
        selfObj->HandleExtensionDisconnectTask(connectRecord);
    };
    taskHandler_->SubmitTask(task, taskName, delayTime);
}

void AbilityConnectManager::RemoveExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord)
{
    HILOG_DEBUG("call");
    CHECK_POINTER(taskHandler_);
    CHECK_POINTER(connectRecord);
    int32_t recordId = connectRecord->GetRecordId();
    std::string taskName = std::string("DelayDisconnectTask_") + std::to_string(recordId);
    taskHandler_->CancelTask(taskName);
}

void AbilityConnectManager::HandleExtensionDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord)
{
    HILOG_DEBUG("call");
    std::lock_guard guard(Lock_);
    CHECK_POINTER(connectRecord);
    int result = connectRecord->DisconnectAbility();
    if (result != ERR_OK) {
        HILOG_WARN("Auto disconnect extension error, ret: %{public}d.", result);
    }
    if (connectRecord->GetConnectState() == ConnectionState::DISCONNECTED) {
        connectRecord->CompleteDisconnect(ERR_OK, false);
        RemoveConnectionRecordFromMap(connectRecord);
    }
}

bool AbilityConnectManager::IsUIExtensionAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, false);
    return UIExtensionUtils::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType);
}

bool AbilityConnectManager::CheckUIExtensionAbilitySessionExistLocked(
    const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, false);

    for (auto it = uiExtensionMap_.begin(); it != uiExtensionMap_.end();) {
        std::shared_ptr<AbilityRecord> uiExtAbility = it->second.first.lock();
        if (abilityRecord == uiExtAbility) {
            return true;
        }
        it++;
    }

    return false;
}

void AbilityConnectManager::RemoveUIExtensionAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(uiExtensionAbilityRecordMgr_);
    uiExtensionAbilityRecordMgr_->RemoveExtensionRecord(abilityRecord->GetUIExtensionAbilityId());
}

int32_t AbilityConnectManager::GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token,
    UIExtensionHostInfo &hostInfo)
{
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_INVALID_VALUE);
    return uiExtensionAbilityRecordMgr_->GetUIExtensionRootHostInfo(token, hostInfo);
}

void AbilityConnectManager::SignRestartAppFlag(const std::string &bundleName)
{
    std::lock_guard guard(Lock_);
    for (auto &[key, abilityRecord] : serviceMap_) {
        if (abilityRecord == nullptr || abilityRecord->GetApplicationInfo().bundleName != bundleName) {
            continue;
        }
        abilityRecord->SetRestartAppFlag(true);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
