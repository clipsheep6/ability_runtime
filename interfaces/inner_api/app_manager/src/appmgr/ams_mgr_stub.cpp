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

#include "ams_mgr_stub.h"
#include "ability_manager_errors.h"
#include "ability_info.h"
#include "app_debug_listener_interface.h"
#include "app_mgr_proxy.h"
#include "app_scheduler_interface.h"
#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "iapp_state_callback.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t MAX_APP_DEBUG_COUNT = 100;
constexpr int32_t MAX_KILL_PROCESS_PID_COUNT = 100;
}

AmsMgrStub::AmsMgrStub()
{
    CreateMemberFuncMap();
}

AmsMgrStub::~AmsMgrStub() {}

void AmsMgrStub::CreateMemberFuncMap() {}

int AmsMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (code != static_cast<uint32_t>(IAmsMgr::Message::Get_BUNDLE_NAME_BY_PID)) {
        TAG_LOGI(AAFwkTag::APPMGR, "AmsMgrStub::OnReceived, code = %{public}u, flags= %{public}d.", code,
            option.GetFlags());
    }
    std::u16string descriptor = AmsMgrStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "local descriptor is unequal to remote");
        return ERR_INVALID_STATE;
    }
    return OnRemoteRequestInner(code, data, reply, option);
}

int32_t AmsMgrStub::OnRemoteRequestInner(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    int retCode = ERR_OK;
    retCode = OnRemoteRequestInnerFirst(code, data, reply, option);
    if (retCode != AAFwk::ERR_CODE_NOT_EXIST) {
        return retCode;
    }
        retCode = OnRemoteRequestInnerSecond(code, data, reply, option);
    if (retCode != AAFwk::ERR_CODE_NOT_EXIST) {
        return retCode;
    }
        retCode = OnRemoteRequestInnerThird(code, data, reply, option);
    if (retCode != AAFwk::ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t AmsMgrStub::OnRemoteRequestInnerFirst(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
        case static_cast<uint32_t>(IAmsMgr::Message::LOAD_ABILITY):
            return HandleLoadAbility(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::TERMINATE_ABILITY):
            return HandleTerminateAbility(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::UPDATE_ABILITY_STATE):
            return HandleUpdateAbilityState(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::UPDATE_EXTENSION_STATE):
            return HandleUpdateExtensionState(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_STATE_CALLBACK):
            return HandleRegisterAppStateCallback(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::ABILITY_BEHAVIOR_ANALYSIS):
            return HandleAbilityBehaviorAnalysis(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::KILL_PEOCESS_BY_ABILITY_TOKEN):
            return HandleKillProcessByAbilityToken(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_BY_USERID):
            return HandleKillProcessesByUserId(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESS_WITH_ACCOUNT):
            return HandleKillProcessWithAccount(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION):
            return HandleKillApplication(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::ABILITY_ATTACH_TIMEOUT):
            return HandleAbilityAttachTimeOut(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::PREPARE_TERMINATE_ABILITY):
            return HandlePrepareTerminate(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION_BYUID):
            return HandleKillApplicationByUid(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION_SELF):
            return HandleKillApplicationSelf(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::GET_RUNNING_PROCESS_INFO_BY_TOKEN):
            return HandleGetRunningProcessInfoByToken(data, reply);
    }
    return AAFwk::ERR_CODE_NOT_EXIST;
}

int32_t AmsMgrStub::OnRemoteRequestInnerSecond(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
        case static_cast<uint32_t>(IAmsMgr::Message::SET_ABILITY_FOREGROUNDING_FLAG):
            return HandleSetAbilityForegroundingFlagToAppRecord(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::START_SPECIFIED_ABILITY):
            return HandleStartSpecifiedAbility(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::REGISTER_START_SPECIFIED_ABILITY_RESPONSE):
            return HandleRegisterStartSpecifiedAbilityResponse(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::GET_APPLICATION_INFO_BY_PROCESS_ID):
            return HandleGetApplicationInfoByProcessID(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_APP_MGR_RECORD_EXIT_REASON):
            return HandleNotifyAppMgrRecordExitReason(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::UPDATE_APPLICATION_INFO_INSTALLED):
            return HandleUpdateApplicationInfoInstalled(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::SET_CURRENT_USER_ID):
            return HandleSetCurrentUserId(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::Get_BUNDLE_NAME_BY_PID):
            return HandleGetBundleNameByPid(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_DEBUG_LISTENER):
            return HandleRegisterAppDebugListener(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::UNREGISTER_APP_DEBUG_LISTENER):
            return HandleUnregisterAppDebugListener(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::ATTACH_APP_DEBUG):
            return HandleAttachAppDebug(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::DETACH_APP_DEBUG):
            return HandleDetachAppDebug(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::SET_APP_WAITING_DEBUG):
            return HandleSetAppWaitingDebug(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::CANCEL_APP_WAITING_DEBUG):
            return HandleCancelAppWaitingDebug(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::GET_WAITING_DEBUG_APP):
            return HandleGetWaitingDebugApp(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::IS_WAITING_DEBUG_APP):
            return HandleIsWaitingDebugApp(data, reply);
    }
    return AAFwk::ERR_CODE_NOT_EXIST;
}

int32_t AmsMgrStub::OnRemoteRequestInnerThird(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
        case static_cast<uint32_t>(IAmsMgr::Message::CLEAR_NON_PERSIST_WAITING_DEBUG_FLAG):
            return HandleClearNonPersistWaitingDebugFlag(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::REGISTER_ABILITY_DEBUG_RESPONSE):
            return HandleRegisterAbilityDebugResponse(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::IS_ATTACH_DEBUG):
            return HandleIsAttachDebug(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::CLEAR_PROCESS_BY_TOKEN):
            return HandleClearProcessByToken(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_BY_PIDS):
            return HandleKillProcessesByPids(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::ATTACH_PID_TO_PARENT):
            return HandleAttachPidToParent(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::IS_MEMORY_SIZE_SUFFICIENT):
            return HandleIsMemorySizeSufficent(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::SET_KEEP_ALIVE_ENABLE_STATE):
            return HandleSetKeepAliveEnableState(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::ATTACHED_TO_STATUS_BAR):
            return HandleAttachedToStatusBar(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::UPDATE_CONFIGURATION):
            return 0;
        case static_cast<uint32_t>(IAmsMgr::Message::GET_CONFIGURATION):
            return 0;
        case static_cast<uint32_t>(IAmsMgr::Message::START_SPECIFIED_PROCESS):
            return 0;
        case static_cast<uint32_t>(IAmsMgr::Message::REGISTER_ABILITY_MS_DELEGATE):
            return 0;
        case static_cast<uint32_t>(IAmsMgr::Message::BLOCK_PROCESS_CACHE_BY_PIDS):
            return HandleBlockProcessCacheByPids(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::IS_KILLED_FOR_UPGRADE_WEB):
            return HandleIsKilledForUpgradeWeb(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::FORCE_KILL_APPLICATION):
            return HandleForceKillApplication(data, reply);
    }
    return AAFwk::ERR_CODE_NOT_EXIST;
}

ErrCode AmsMgrStub::HandleLoadAbility(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> preToke = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    if (data.ReadBool()) {
        preToke = data.ReadRemoteObject();
    }
    std::shared_ptr<AbilityInfo> abilityInfo(data.ReadParcelable<AbilityInfo>());
    if (!abilityInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<AbilityInfo> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    std::shared_ptr<ApplicationInfo> appInfo(data.ReadParcelable<ApplicationInfo>());
    if (!appInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<ApplicationInfo> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    std::shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (!want) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable want failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    int32_t abilityRecordId = data.ReadInt32();

    LoadAbility(token, preToke, abilityInfo, appInfo, want, abilityRecordId);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleTerminateAbility(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    bool clearMissionFlag = data.ReadBool();
    TerminateAbility(token, clearMissionFlag);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleUpdateAbilityState(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    int32_t state = data.ReadInt32();
    UpdateAbilityState(token, static_cast<AbilityState>(state));
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleUpdateExtensionState(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    int32_t state = data.ReadInt32();
    UpdateExtensionState(token, static_cast<ExtensionState>(state));
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleRegisterAppStateCallback(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IAppStateCallback> callback = nullptr;
    if (data.ReadBool()) {
        sptr<IRemoteObject> obj = data.ReadRemoteObject();
        callback = iface_cast<IAppStateCallback>(obj);
    }
    RegisterAppStateCallback(callback);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleAbilityBehaviorAnalysis(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    sptr<IRemoteObject> preToke = nullptr;
    if (data.ReadBool()) {
        preToke = data.ReadRemoteObject();
    }
    int32_t visibility = data.ReadInt32();
    int32_t perceptibility = data.ReadInt32();
    int32_t connectionState = data.ReadInt32();

    AbilityBehaviorAnalysis(token, preToke, visibility, perceptibility, connectionState);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillProcessByAbilityToken(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();

    KillProcessByAbilityToken(token);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillProcessesByUserId(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t userId = data.ReadInt32();

    KillProcessesByUserId(userId);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillProcessesByPids(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    auto size = data.ReadUint32();
    if (size == 0 || size > MAX_KILL_PROCESS_PID_COUNT) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid size.");
        return ERR_INVALID_VALUE;
    }
    std::vector<int32_t> pids;
    for (uint32_t i = 0; i < size; i++) {
        pids.emplace_back(data.ReadInt32());
    }

    KillProcessesByPids(pids);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleAttachPidToParent(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    AttachPidToParent(token, callerToken);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillProcessWithAccount(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::APPMGR, "enter");

    HITRACE_METER(HITRACE_TAG_APP);

    std::string bundleName = data.ReadString();
    int accountId = data.ReadInt32();
    bool clearPageStack = data.ReadBool();

    TAG_LOGI(AAFwkTag::APPMGR,
        "bundleName = %{public}s, accountId = %{public}d, clearPageStack = %{public}d",
        bundleName.c_str(), accountId, clearPageStack);

    int32_t result = KillProcessWithAccount(bundleName, accountId, clearPageStack);
    reply.WriteInt32(result);

    TAG_LOGI(AAFwkTag::APPMGR, "end");

    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillApplication(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    bool clearPageStack = data.ReadBool();

    TAG_LOGI(AAFwkTag::APPMGR, "bundleName = %{public}s, clearPageStack = %{public}d",
        bundleName.c_str(), clearPageStack);

    int32_t result = KillApplication(bundleName, clearPageStack);
    reply.WriteInt32(result);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleForceKillApplication(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    int userId = data.ReadInt32();
    int appIndex = data.ReadInt32();

    TAG_LOGI(AAFwkTag::APPMGR, "bundleName = %{public}s,userId=%{public}d,appIndex=%{public}d",
        bundleName.c_str(), userId, appIndex);

    int32_t result = ForceKillApplication(bundleName, userId, appIndex);
    reply.WriteInt32(result);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillApplicationByUid(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    int uid = data.ReadInt32();
    int32_t result = KillApplicationByUid(bundleName, uid);
    reply.WriteInt32(result);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillApplicationSelf(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    bool clearPageStack = data.ReadBool();
    int32_t result = KillApplicationSelf(clearPageStack);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "result write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleAbilityAttachTimeOut(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    AbilityAttachTimeOut(token);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandlePrepareTerminate(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    bool clearMissionFlag = data.ReadBool();
    PrepareTerminate(token, clearMissionFlag);
    return NO_ERROR;
}

void AmsMgrStub::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{}

int32_t AmsMgrStub::HandleGetRunningProcessInfoByToken(MessageParcel &data, MessageParcel &reply)
{
    RunningProcessInfo processInfo;
    auto token = data.ReadRemoteObject();
    GetRunningProcessInfoByToken(token, processInfo);
    if (reply.WriteParcelable(&processInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "process info write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleSetAbilityForegroundingFlagToAppRecord(MessageParcel &data, MessageParcel &reply)
{
    RunningProcessInfo processInfo;
    auto pid = static_cast<pid_t>(data.ReadInt32());
    SetAbilityForegroundingFlagToAppRecord(pid);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleStartSpecifiedAbility(MessageParcel &data, MessageParcel &reply)
{
    AAFwk::Want *want = data.ReadParcelable<AAFwk::Want>();
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }

    AbilityInfo *abilityInfo = data.ReadParcelable<AbilityInfo>();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityInfo is nullptr.");
        delete want;
        return ERR_INVALID_VALUE;
    }
    StartSpecifiedAbility(*want, *abilityInfo, data.ReadInt32());
    delete want;
    delete abilityInfo;
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleRegisterStartSpecifiedAbilityResponse(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    sptr<IStartSpecifiedAbilityResponse> response = iface_cast<IStartSpecifiedAbilityResponse>(obj);
    RegisterStartSpecifiedAbilityResponse(response);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleGetApplicationInfoByProcessID(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t pid = data.ReadInt32();
    AppExecFwk::ApplicationInfo application;
    bool debug;
    int32_t result = GetApplicationInfoByProcessID(pid, application, debug);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write result error.");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteParcelable(&application)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write application info failed");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteBool(debug)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write debug info failed");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleNotifyAppMgrRecordExitReason(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    int32_t pid = data.ReadInt32();
    int32_t reason = data.ReadInt32();
    std::string exitMsg = Str16ToStr8(data.ReadString16());
    int32_t result = NotifyAppMgrRecordExitReason(pid, reason, exitMsg);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write result failed.");
        return IPC_PROXY_ERR;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleUpdateApplicationInfoInstalled(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    int uid = data.ReadInt32();
    int32_t result = UpdateApplicationInfoInstalled(bundleName, uid);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleSetCurrentUserId(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    SetCurrentUserId(userId);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleGetBundleNameByPid(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = data.ReadInt32();
    std::string bundleName;
    int32_t uid;
    GetBundleNameByPid(pid, bundleName, uid);

    reply.WriteString(bundleName);
    reply.WriteInt32(uid);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleRegisterAppDebugListener(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appDebugLister = iface_cast<IAppDebugListener>(data.ReadRemoteObject());
    if (appDebugLister == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "App debug lister is null.");
        return ERR_INVALID_VALUE;
    }

    auto result = RegisterAppDebugListener(appDebugLister);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleUnregisterAppDebugListener(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appDebugLister = iface_cast<IAppDebugListener>(data.ReadRemoteObject());
    if (appDebugLister == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "App debug lister is nullptr.");
        return ERR_INVALID_VALUE;
    }

    auto result = UnregisterAppDebugListener(appDebugLister);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleAttachAppDebug(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }

    auto result = AttachAppDebug(bundleName);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleDetachAppDebug(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }

    auto result = DetachAppDebug(bundleName);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleSetAppWaitingDebug(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }
    auto isPersist = data.ReadBool();
    auto result = SetAppWaitingDebug(bundleName, isPersist);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleCancelAppWaitingDebug(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto result = CancelAppWaitingDebug();
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleGetWaitingDebugApp(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::vector<std::string> debugInfoList;
    auto result = GetWaitingDebugApp(debugInfoList);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }

    int32_t listSize = static_cast<int32_t>(debugInfoList.size());
    if (listSize > MAX_APP_DEBUG_COUNT) {
        TAG_LOGE(AAFwkTag::APPMGR, "Max app debug count is %{public}d.", listSize);
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(listSize)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write list size.");
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteStringVector(debugInfoList)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write string vector debug info list.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleIsWaitingDebugApp(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }

    auto result = IsWaitingDebugApp(bundleName);
    if (!reply.WriteBool(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleSetKeepAliveEnableState(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto bundleName = data.ReadString();
    auto enable = data.ReadBool();
    SetKeepAliveEnableState(bundleName, enable);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleClearNonPersistWaitingDebugFlag(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    ClearNonPersistWaitingDebugFlag();
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleRegisterAbilityDebugResponse(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto response = iface_cast<IAbilityDebugResponse>(data.ReadRemoteObject());
    if (response == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Response is nullptr.");
        return ERR_INVALID_VALUE;
    }

    auto result = RegisterAbilityDebugResponse(response);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleIsAttachDebug(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }

    auto result = IsAttachDebug(bundleName);
    if (!reply.WriteBool(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleClearProcessByToken(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    ClearProcessByToken(token);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleIsMemorySizeSufficent(MessageParcel &data, MessageParcel &reply)
{
    auto result = IsMemorySizeSufficent();
    if (!reply.WriteBool(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleAttachedToStatusBar(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    AttachedToStatusBar(token);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleBlockProcessCacheByPids(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    auto size = data.ReadUint32();
    if (size == 0 || size > MAX_KILL_PROCESS_PID_COUNT) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid size.");
        return ERR_INVALID_VALUE;
    }
    std::vector<int32_t> pids;
    for (uint32_t i = 0; i < size; i++) {
        pids.emplace_back(data.ReadInt32());
    }

    BlockProcessCacheByPids(pids);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleIsKilledForUpgradeWeb(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }

    auto result = IsKilledForUpgradeWeb(bundleName);
    if (!reply.WriteBool(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
