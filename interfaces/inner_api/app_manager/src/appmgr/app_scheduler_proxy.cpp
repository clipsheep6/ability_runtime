/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
        HILOG_ERROR("write interface token failed");
        return false;
    }
    return true;
}

void AppSchedulerProxy::ScheduleForegroundApplication()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    IAppScheduler::Message code(IAppScheduler::Message::SCHEDULE_FOREGROUND_APPLICATION_TRANSACTION);
    if (SendRequest(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return;
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

    IAppScheduler::Message code(IAppScheduler::Message::SCHEDULE_BACKGROUND_APPLICATION_TRANSACTION);
    if (SendRequest(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void AppSchedulerProxy::ScheduleTerminateApplication()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    IAppScheduler::Message code(IAppScheduler::Message::SCHEDULE_TERMINATE_APPLICATION_TRANSACTION);
    if (SendRequest(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void AppSchedulerProxy::ScheduleLowMemory()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    IAppScheduler::Message code(IAppScheduler::Message::SCHEDULE_LOWMEMORY_APPLICATION_TRANSACTION);
    if (SendRequest(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void AppSchedulerProxy::ScheduleMemoryLevel(int32_t level)
{
    ScheduleMemoryCommon(IAppScheduler::Message::SCHEDULE_MEMORYLEVEL_APPLICATION_TRANSACTION, level);
}

void AppSchedulerProxy::ScheduleHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("AppSchedulerProxy !WriteInterfaceToken.");
        return;
    }
    data.WriteInt32(pid);
    IAppScheduler::Message code(IAppScheduler::Message::SCHEDULE_HEAPMEMORY_APPLICATION_TRANSACTION);
    if (SendRequest(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return;
    }

    std::unique_ptr<MallocInfo> info(reply.ReadParcelable<MallocInfo>());
    if (info == nullptr) {
        HILOG_ERROR("MallocInfo ReadParcelable nullptr");
        return;
    }
    mallocInfo = *info;
}

void AppSchedulerProxy::ScheduleShrinkMemory(const int32_t level)
{
    ScheduleMemoryCommon(IAppScheduler::Message::SCHEDULE_SHRINK_MEMORY_APPLICATION_TRANSACTION, level);
}

void AppSchedulerProxy::ScheduleMemoryCommon(IAppScheduler::Message code, const int32_t level)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteInt32(level);
    if (SendRequest(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void AppSchedulerProxy::ScheduleLaunchAbility(const AbilityInfo &info, const sptr<IRemoteObject> &token,
    const std::shared_ptr<AAFwk::Want> &want)
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
            HILOG_ERROR("Failed to write flag and token");
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("Failed to write flag");
            return;
        }
    }

    if (!data.WriteParcelable(want.get())) {
        HILOG_ERROR("write want fail.");
        return;
    }

    IAppScheduler::Message code(IAppScheduler::Message::SCHEDULE_LAUNCH_ABILITY_TRANSACTION);
    if (SendRequest(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return;
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
        HILOG_ERROR("Failed to write token");
        return;
    }
    IAppScheduler::Message code(IAppScheduler::Message::SCHEDULE_CLEAN_ABILITY_TRANSACTION);
    if (SendRequest(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void AppSchedulerProxy::ScheduleLaunchApplication(const AppLaunchData &launchData, const Configuration &config)
{
    HILOG_DEBUG("AppSchedulerProxy ScheduleLaunchApplication start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteParcelable(&launchData)) {
        HILOG_ERROR("WriteParcelable launchData failed");
        return ;
    }

    if (!data.WriteParcelable(&config)) {
        HILOG_ERROR("WriteParcelable config failed");
        return ;
    }

    IAppScheduler::Message code(IAppScheduler::Message::SCHEDULE_LAUNCH_APPLICATION_TRANSACTION);
    if (SendRequest(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

void AppSchedulerProxy::ScheduleUpdateApplicationInfoInstalled(const ApplicationInfo &appInfo)
{
    HILOG_INFO("AppSchedulerProxy ScheduleUpdateApplicationInfoInstalled start");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&appInfo)) {
        return ;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    IAppScheduler::Message code(IAppScheduler::Message::SCHEDULE_UPDATE_APPLICATION_INFO_INSTALLED);
    if (SendRequest(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
    HILOG_INFO("AppSchedulerProxy ScheduleUpdateApplicationInfoInstalled end");
}

void AppSchedulerProxy::ScheduleAbilityStage(const HapModuleInfo &abilityStage)
{
    HILOG_INFO("AppSchedulerProxy ScheduleAbilityStage start");
    MessageParcel data;
    constexpr int32_t max = 10000;
    constexpr int32_t large = 60;
    constexpr int32_t mid = 20;
    auto abilityInfoSize = static_cast<int32_t>(abilityStage.abilityInfos.size());
    auto extensionInfoSize = static_cast<int32_t>(abilityStage.extensionInfos.size());
    if (abilityInfoSize > max || extensionInfoSize > max) {
        HILOG_ERROR("size exceeds max");
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
    if (SendRequest(IAppScheduler::Message::SCHEDULE_ABILITY_STAGE_INFO, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
    HILOG_INFO("AppSchedulerProxy ScheduleAbilityStage end");
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
    IAppScheduler::Message code(IAppScheduler::Message::SCHEDULE_PROFILE_CHANGED_TRANSACTION);
    if (SendRequest(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return;
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
    if (SendRequest(IAppScheduler::Message::SCHEDULE_CONFIGURATION_UPDATED, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return;
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
    IAppScheduler::Message code(IAppScheduler::Message::SCHEDULE_PROCESS_SECURITY_EXIT_TRANSACTION);
    if (SendRequest(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return;
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

    if (!SendRequest(IAppScheduler::Message::SCHEDULE_ACCEPT_WANT, data, reply, option)) {
        HILOG_ERROR("SendRequest failed");
        return;
    }
}

int32_t AppSchedulerProxy::ScheduleNotifyLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("ScheduleNotifyLoadRepairPatch, Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("ScheduleNotifyLoadRepairPatch, Write bundle name failed.");
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
    int32_t ret = SendRequest(IAppScheduler::Message::SCHEDULE_NOTIFY_LOAD_REPAIR_PATCH, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppSchedulerProxy::ScheduleNotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
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
    int32_t ret = SendRequest(IAppScheduler::Message::SCHEDULE_NOTIFY_HOT_RELOAD_PAGE, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
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
    int32_t ret = SendRequest(IAppScheduler::Message::SCHEDULE_NOTIFY_UNLOAD_REPAIR_PATCH, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppSchedulerProxy::ScheduleNotifyAppFault(const FaultData &faultData)
{
    MessageParcel data;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteParcelable(&faultData)) {
        HILOG_ERROR("Write FaultData error.");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = SendRequest(IAppScheduler::Message::SCHEDULE_NOTIFY_FAULT, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest failed");
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppSchedulerProxy::SendRequest(IAppScheduler::Message code, MessageParcel &data,
                                       MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("remote object is nullptr.");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest failed. code is %{public}d, ret is %{public}d.", code, ret);
        return ret;
    }
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
