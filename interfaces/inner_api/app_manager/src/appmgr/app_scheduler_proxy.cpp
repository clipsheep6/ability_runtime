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

#include "app_scheduler_proxy.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
AppSchedulerProxy::AppSchedulerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAppScheduler>(impl)
{}

bool AppSchedulerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AppSchedulerProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write interface token failed");
        return false;
    }
    return true;
}

void AppSchedulerProxy::ScheduleForegroundApplication()
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy::ScheduleForegroundApplication start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_FOREGROUND_APPLICATION_TRANSACTION),
            data,
            reply,
            option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleBackgroundApplication()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_BACKGROUND_APPLICATION_TRANSACTION),
            data,
            reply,
            option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is wrong, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleTerminateApplication(bool isLastProcess)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteBool(isLastProcess)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bool failed.");
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_TERMINATE_APPLICATION_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is unsuccessful, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleLowMemory()
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy::ScheduleLowMemory begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LOWMEMORY_APPLICATION_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleMemoryLevel(int32_t level)
{
    uint32_t operation = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_MEMORYLEVEL_APPLICATION_TRANSACTION);
    ScheduleMemoryCommon(level, operation);
}

void AppSchedulerProxy::ScheduleHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy::ScheduleHeapMemory start");
    uint32_t operation = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_HEAPMEMORY_APPLICATION_TRANSACTION);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSchedulerProxy !WriteInterfaceToken.");
        return;
    }
    data.WriteInt32(pid);
    int32_t ret = SendTransactCmd(operation, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }

    std::unique_ptr<MallocInfo> info(reply.ReadParcelable<MallocInfo>());
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "MallocInfo ReadParcelable nullptr");
        return;
    }
    mallocInfo = *info;
}

void AppSchedulerProxy::ScheduleJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy::ScheduleJsHeapMemory start");
    uint32_t operation = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_JSHEAP_MEMORY_APPLICATION_TRANSACTION);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSchedulerProxy !WriteInterfaceToken.");
        return;
    }
    data.WriteParcelable(&info);
    int32_t ret = SendTransactCmd(operation, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }
}

void AppSchedulerProxy::ScheduleShrinkMemory(const int32_t level)
{
    uint32_t operation = static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_SHRINK_MEMORY_APPLICATION_TRANSACTION);
    ScheduleMemoryCommon(level, operation);
}

void AppSchedulerProxy::ScheduleMemoryCommon(const int32_t level, const uint32_t operation)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteInt32(level);
    int32_t ret = SendTransactCmd(operation, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleLaunchAbility(const AbilityInfo &info, const sptr<IRemoteObject> &token,
    const std::shared_ptr<AAFwk::Want> &want, int32_t abilityRecordId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&info);

    if (token) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(token.GetRefPtr())) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write flag and token");
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write flag");
            return;
        }
    }

    if (!data.WriteParcelable(want.get())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write want fail.");
        return;
    }
    if (!data.WriteInt32(abilityRecordId)) {
        HILOG_ERROR("write ability record id fail.");
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LAUNCH_ABILITY_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleCleanAbility(const sptr<IRemoteObject> &token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CLEAN_ABILITY_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleLaunchApplication(const AppLaunchData &launchData, const Configuration &config)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy ScheduleLaunchApplication start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteParcelable(&launchData)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteParcelable launchData failed");
        return ;
    }

    if (!data.WriteParcelable(&config)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteParcelable config failed");
        return ;
    }

    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LAUNCH_APPLICATION_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleUpdateApplicationInfoInstalled(const ApplicationInfo &appInfo)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy ScheduleUpdateApplicationInfoInstalled begin");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&appInfo)) {
        return ;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_UPDATE_APPLICATION_INFO_INSTALLED), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy ScheduleUpdateApplicationInfoInstalled end");
}

void AppSchedulerProxy::ScheduleAbilityStage(const HapModuleInfo &abilityStage)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy ScheduleAbilityStage start");
    MessageParcel data;
    constexpr int32_t max = 10000;
    constexpr int32_t large = 60;
    constexpr int32_t mid = 20;
    auto abilityInfoSize = static_cast<int32_t>(abilityStage.abilityInfos.size());
    auto extensionInfoSize = static_cast<int32_t>(abilityStage.extensionInfos.size());
    if (abilityInfoSize > max || extensionInfoSize > max) {
        TAG_LOGE(AAFwkTag::APPMGR, "size exceeds max");
        return;
    }
    auto componentSize = abilityInfoSize + extensionInfoSize;
    if (componentSize > large) {
        constexpr int32_t size = 2 * 1024 * 1024; // 1.6 M
        data.SetDataCapacity(size);
    } else if (componentSize > mid) {
        constexpr int32_t size = 800 * 1024; // 800 kb
        data.SetDataCapacity(size);
    }
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteParcelable(&abilityStage)) {
        return ;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ABILITY_STAGE_INFO), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerProxy ScheduleAbilityStage end");
}

void AppSchedulerProxy::ScheduleProfileChanged(const Profile &profile)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&profile);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_PROFILE_CHANGED_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleConfigurationUpdated(const Configuration &config)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&config);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CONFIGURATION_UPDATED), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleProcessSecurityExit()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_PROCESS_SECURITY_EXIT_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteParcelable(&want) || !data.WriteString(moduleName)) {
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ACCEPT_WANT), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteParcelable(&want) || !data.WriteString(moduleName)) {
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NEW_PROCESS_REQUEST), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

int32_t AppSchedulerProxy::ScheduleNotifyLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ScheduleNotifyLoadRepairPatch, Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ScheduleNotifyLoadRepairPatch, Write bundle name failed.");
        return ERR_INVALID_DATA;
    }

    if (callback == nullptr || !data.WriteRemoteObject(callback->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write callback failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(recordId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write record id failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_LOAD_REPAIR_PATCH),
        data, reply, option);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "ScheduleNotifyLoadRepairPatch, Send request failed with errno %{public}d.", ret);
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppSchedulerProxy::ScheduleNotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed!");
        return ERR_INVALID_DATA;
    }

    if (callback == nullptr || !data.WriteRemoteObject(callback->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write callback failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(recordId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write record id failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_HOT_RELOAD_PAGE),
        data, reply, option);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with errno %{public}d.", ret);
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppSchedulerProxy::ScheduleNotifyUnLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Schedule notify unload patch, Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Schedule notify unload patch, Write bundle name failed.");
        return ERR_INVALID_DATA;
    }

    if (callback == nullptr || !data.WriteRemoteObject(callback->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write callback failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(recordId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write record id failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_UNLOAD_REPAIR_PATCH), data, reply, option);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "Schedule notify unload patch, Send request failed with errno %{public}d.", ret);
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppSchedulerProxy::ScheduleNotifyUnLoadPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
	HILOG_ERROR("functiob called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Schedule notify unload patch, Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("Schedule notify unload patch, Write bundle name failed.");
        return ERR_INVALID_DATA;
    }

    if (callback == nullptr || !data.WriteRemoteObject(callback->AsObject())) {
        HILOG_ERROR("Write callback failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(recordId)) {
        HILOG_ERROR("Write record id failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_UNLOAD_PATCH), data,
        reply, option);
    if (ret != 0) {
        HILOG_ERROR("Schedule notify unload patch, Send request failed with errno %{public}d.", ret);
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppSchedulerProxy::ScheduleNotifyLoadPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback, 
                                                   const int32_t recordId, const int &patchVersion)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
	HILOG_ERROR("function");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Schedule notify unload patch, Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("Schedule notify unload patch, Write bundle name failed.");
        return ERR_INVALID_DATA;
    }

    if (callback == nullptr || !data.WriteRemoteObject(callback->AsObject())) {
        HILOG_ERROR("Write callback failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(recordId)) {
        HILOG_ERROR("Write record id failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(patchVersion)) {
        HILOG_ERROR("Write record id failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_LOAD_PATCH), data, reply, option);
    if (ret != 0) {
        HILOG_ERROR("Schedule notify unload patch, Send request failed with errno %{public}d.", ret);
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppSchedulerProxy::ScheduleNotifyAppFault(const FaultData &faultData)
{
    MessageParcel data;

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteParcelable(&faultData)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write FaultData error.");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_FAULT),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with error code %{public}d.", ret);
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppSchedulerProxy::ScheduleChangeAppGcState(int32_t state)
{
    MessageParcel data;

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed");
        return ERR_INVALID_DATA;
    }

    data.WriteInt32(state);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAppScheduler::Message::APP_GC_STATE_CHANGE),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with error code %{public}d.", ret);
        return ret;
    }

    return NO_ERROR;
}

void AppSchedulerProxy::AttachAppDebug()
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ATTACH_APP_DEBUG), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to send request with error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::DetachAppDebug()
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DETACH_APP_DEBUG), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with error code: %{public}d", ret);
    }
}

int32_t AppSchedulerProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    auto ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with error code: %{public}d", ret);
        return ret;
    }
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
