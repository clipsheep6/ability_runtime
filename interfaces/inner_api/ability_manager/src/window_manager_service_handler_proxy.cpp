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

#ifdef SUPPORT_GRAPHICS
#include "window_manager_service_handler_proxy.h"

#include "ability_manager_errors.h"
#include "hilog_wrapper.h"
#include "parcel.h"

namespace OHOS {
namespace AAFwk {
WindowManagerServiceHandlerProxy::WindowManagerServiceHandlerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IWindowManagerServiceHandler>(impl) {}

void WindowManagerServiceHandlerProxy::NotifyWindowTransition(sptr<AbilityTransitionInfo> fromInfo,
    sptr<AbilityTransitionInfo> toInfo, bool& animaEnabled)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }
    if (!data.WriteParcelable(fromInfo.GetRefPtr())) {
        HILOG_ERROR("Write fromInfo failed.");
        return;
    }
    if (!data.WriteParcelable(toInfo.GetRefPtr())) {
        HILOG_ERROR("Write toInfo failed.");
        return;
    }
    if (!data.WriteBool(animaEnabled)) {
        HILOG_ERROR("Write animaEnabled failed.");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int error = Remote()->SendRequest(WMSCmd::ON_NOTIFY_WINDOW_TRANSITION, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fail, error: %{public}d", error);
    }
    animaEnabled = reply.ReadBool();
}

int32_t WindowManagerServiceHandlerProxy::GetFocusWindow(sptr<IRemoteObject>& abilityToken)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_AAFWK_PARCEL_FAIL;
    }

    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(WMSCmd::ON_GET_FOCUS_ABILITY, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fail, error: %{public}d", error);
        return ERR_AAFWK_PARCEL_FAIL;
    }
    auto ret = reply.ReadInt32();
    if (ret == 0 && reply.ReadBool()) {
        abilityToken = reply.ReadObject<IRemoteObject>();
    }
    return ret;
}

void WindowManagerServiceHandlerProxy::StartingWindow(sptr<AbilityTransitionInfo> info,
    std::shared_ptr<Media::PixelMap> pixelMap, uint32_t bgColor)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }
    if (!data.WriteParcelable(info.GetRefPtr())) {
        HILOG_ERROR("Write info failed.");
        return;
    }
    if (!data.WriteParcelable(pixelMap.get())) {
        HILOG_ERROR("Write pixelMap failed.");
        return;
    }
    if (!data.WriteUint32(bgColor)) {
        HILOG_ERROR("Write bgColor failed.");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int error = Remote()->SendRequest(WMSCmd::ON_COLD_STARTING_WINDOW, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fail, error: %{public}d", error);
    }
}

void WindowManagerServiceHandlerProxy::StartingWindow(sptr<AbilityTransitionInfo> info,
    std::shared_ptr<Media::PixelMap> pixelMap)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }
    if (!data.WriteParcelable(info.GetRefPtr())) {
        HILOG_ERROR("Write info failed.");
        return;
    }
    if (!data.WriteParcelable(pixelMap.get())) {
        HILOG_ERROR("Write pixelMap failed.");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int error = Remote()->SendRequest(WMSCmd::ON_HOT_STARTING_WINDOW, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fail, error: %{public}d", error);
    }
}

void WindowManagerServiceHandlerProxy::CancelStartingWindow(sptr<IRemoteObject> abilityToken)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }
    if (!abilityToken) {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("Write false failed.");
            return;
        }
    } else {
        if (!data.WriteBool(true)) {
            HILOG_ERROR("Write true failed.");
            return;
        }
        if (!data.WriteObject(abilityToken)) {
            HILOG_ERROR("Write abilityToken failed.");
            return;
        }
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int error = Remote()->SendRequest(WMSCmd::ON_CANCEL_STARTING_WINDOW, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fail, error: %{public}d", error);
    }
}

void WindowManagerServiceHandlerProxy::NotifyAnimationAbilityDied(sptr<AbilityTransitionInfo> info)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }
    if (!data.WriteParcelable(info.GetRefPtr())) {
        HILOG_ERROR("Write info failed.");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int error = Remote()->SendRequest(WMSCmd::ON_NOTIFY_ANIMATION_ABILITY_DIED, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fail, error: %{public}d", error);
    }
}

int32_t WindowManagerServiceHandlerProxy::MoveMissionsToForeground(const std::vector<int32_t>& missionIds,
    int32_t topMissionId)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        HILOG_ERROR("WriteInterfaceToken failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }

    if (!data.WriteInt32Vector(missionIds)) {
        HILOG_ERROR("Write missionIds failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }

    if (!data.WriteInt32(topMissionId)) {
        HILOG_ERROR("Write TopMissionId failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }

    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("remote is nullptr.");
        return ERR_INVALID_CONTINUATION_FLAG;
    }
    int error = remote->SendRequest(WMSCmd::ON_MOVE_MISSINONS_TO_FOREGROUND, data, reply, option);
    if (error != ERR_NONE) {
        HILOG_ERROR("SendoRequest failed, error: %{public}d", error);
        return ERR_AAFWK_PARCEL_FAIL;
    }
    return reply.ReadInt32();
}

int32_t WindowManagerServiceHandlerProxy::MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
    std::vector<int32_t>& result)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        HILOG_ERROR("WriteInterfaceToken failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }

    if (!data.WriteInt32Vector(missionIds)) {
        HILOG_ERROR("Write missionIds failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }

    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("remote is nullptr.");
        return ERR_INVALID_CONTINUATION_FLAG;
    }
    int error = remote->SendRequest(WMSCmd::ON_MOVE_MISSIONS_TO_BACKGROUND, data, reply, option);
    if (error != ERR_NONE) {
        HILOG_ERROR("SendoRequest failed, error: %{public}d", error);
        return ERR_AAFWK_PARCEL_FAIL;
    }
    if (!reply.ReadInt32Vector(&result)) {
        HILOG_ERROR("Read hide result failed");
        return ERR_AAFWK_PARCEL_FAIL;
    };
    return reply.ReadInt32();
}

}  // namespace AAFwk
}  // namespace OHOS
#endif
