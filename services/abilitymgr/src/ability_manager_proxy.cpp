/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ability_manager_proxy.h"

#include "errors.h"
#include "string_ex.h"

#include "ability_connect_callback_proxy.h"
#include "ability_connect_callback_stub.h"
#include "ability_manager_errors.h"
#include "ability_scheduler_proxy.h"
#include "ability_scheduler_stub.h"
#include "ability_util.h"
#include "appexecfwk_errors.h"
#include "configuration.h"
#include "session_info.h"

namespace OHOS {
namespace AAFwk {
namespace {
#define PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(messageParcel, type, value) \
    do {                                                                  \
        if (!(messageParcel).Write##type(value)) {                        \
            HILOG_ERROR("failed to write %{public}s", #value);            \
            return INNER_ERR;                                             \
        }                                                                 \
    } while (0)
}
using AutoStartupInfo = AbilityRuntime::AutoStartupInfo;
constexpr int32_t CYCLE_LIMIT = 1000;
constexpr int32_t MAX_AUTO_STARTUP_COUNT = 100;
bool AbilityManagerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AbilityManagerProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed.");
        return false;
    }
    return true;
}

int AbilityManagerProxy::StartAbility(const Want &want, int32_t userId, int requestCode)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }

    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("userId write failed.");
        return INNER_ERR;
    }

    if (!data.WriteInt32(requestCode)) {
        HILOG_ERROR("requestCode write failed.");
        return INNER_ERR;
    }

    error = SendRequest(AbilityManagerInterfaceCode::START_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

AppExecFwk::ElementName AbilityManagerProxy::GetTopAbility(bool isNeedLocalDeviceId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return {};
    }
    if (!data.WriteBool(isNeedLocalDeviceId)) {
        return {};
    }

    int error = SendRequest(AbilityManagerInterfaceCode::GET_TOP_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return {};
    }
    std::unique_ptr<AppExecFwk::ElementName> name(reply.ReadParcelable<AppExecFwk::ElementName>());
    if (!name) {
        HILOG_ERROR("Read info failed.");
        return {};
    }
    AppExecFwk::ElementName result = *name;
    return result;
}

AppExecFwk::ElementName AbilityManagerProxy::GetElementNameByToken(sptr<IRemoteObject> token,
    bool isNeedLocalDeviceId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return {};
    }
    if (!data.WriteRemoteObject(token)) {
        return {};
    }
    if (!data.WriteBool(isNeedLocalDeviceId)) {
        return {};
    }
    int error = SendRequest(AbilityManagerInterfaceCode::GET_ELEMENT_NAME_BY_TOKEN, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return {};
    }
    std::unique_ptr<AppExecFwk::ElementName> name(reply.ReadParcelable<AppExecFwk::ElementName>());
    if (!name) {
        HILOG_ERROR("Read info failed.");
        return {};
    }
    AppExecFwk::ElementName result = *name;
    return result;
}

int AbilityManagerProxy::StartAbility(const Want &want, const AbilityStartSetting &abilityStartSetting,
    const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&abilityStartSetting)) {
        HILOG_ERROR("abilityStartSetting write failed.");
        return INNER_ERR;
    }
    if (callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
            HILOG_ERROR("flag and callerToken write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("userId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(requestCode)) {
        HILOG_ERROR("requestCode write failed.");
        return INNER_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::START_ABILITY_FOR_SETTINGS, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StartAbility(
    const Want &want, const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }
    if (callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
            HILOG_ERROR("callerToken and flag write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("userId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(requestCode)) {
        HILOG_ERROR("requestCode write failed.");
        return INNER_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::START_ABILITY_ADD_CALLER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StartAbilityWithSpecifyTokenId(
    const Want &want, const sptr<IRemoteObject> &callerToken, uint32_t specifyTokenId, int32_t userId, int requestCode)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }
    if (callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
            HILOG_ERROR("callerToken and flag write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteInt32(specifyTokenId)) {
        HILOG_ERROR("specifyTokenId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("userId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(requestCode)) {
        HILOG_ERROR("requestCode write failed.");
        return INNER_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::START_ABILITY_WITH_SPECIFY_TOKENID, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::StartAbilityByInsightIntent(const Want &want, const sptr<IRemoteObject> &callerToken,
    uint64_t intentId, int32_t userId)
{
    MessageParcel data;
    if (callerToken == nullptr) {
        HILOG_ERROR("invalid callertoken.");
        return INNER_ERR;
    }

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }

    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }

    if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
        HILOG_ERROR("callerToken and flag write failed.");
        return INNER_ERR;
    }

    if (!data.WriteUint64(intentId)) {
        HILOG_ERROR("intentId write failed.");
        return INNER_ERR;
    }

    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("userId write failed.");
        return INNER_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = SendRequest(AbilityManagerInterfaceCode::START_ABILITY_BY_INSIGHT_INTENT, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("failed to start ability err: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StartAbility(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&startOptions)) {
        HILOG_ERROR("startOptions write failed.");
        return INNER_ERR;
    }
    if (callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
            HILOG_ERROR("flag and callerToken write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("userId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(requestCode)) {
        HILOG_ERROR("requestCode write failed.");
        return INNER_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::START_ABILITY_FOR_OPTIONS, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StartAbilityAsCaller(const Want &want, const sptr<IRemoteObject> &callerToken,
    sptr<IRemoteObject> asCallerSourceToken, int32_t userId, int requestCode, bool isSendDialogResult)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Parcelable, &want);
    if (callerToken) {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, true);
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, RemoteObject, callerToken);
    } else {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, false);
    }
    if (asCallerSourceToken) {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, true);
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, RemoteObject, asCallerSourceToken);
    } else {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, false);
    }
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Int32, userId);
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Int32, requestCode);
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, isSendDialogResult);
    int error = SendRequest(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_BY_TOKEN, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StartAbilityAsCaller(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, sptr<IRemoteObject> asCallerSourceToken,
    int32_t userId, int requestCode)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Parcelable, &want);
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Parcelable, &startOptions);
    if (callerToken) {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, true);
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, RemoteObject, callerToken);
    } else {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, false);
    }
    if (asCallerSourceToken) {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, true);
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, RemoteObject, asCallerSourceToken);
    } else {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, false);
    }
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Int32, userId);
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Int32, requestCode);

    error = SendRequest(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_FOR_OPTIONS, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::CheckUISessionParams(MessageParcel &data, const sptr<IRemoteObject> &callerToken,
    const sptr<SessionInfo> &sessionInfo, int32_t userId, int requestCode)
{
    if (callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
            HILOG_ERROR("callerToken and flag write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }
    if (sessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(sessionInfo)) {
            HILOG_ERROR("flag and sessionInfo write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("userId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(requestCode)) {
        HILOG_ERROR("requestCode write failed.");
        return INNER_ERR;
    }
    return ERR_OK;
}

int AbilityManagerProxy::StartAbilityByUIContentSession(const Want &want,
    const sptr<IRemoteObject> &callerToken, const sptr<SessionInfo> &sessionInfo,
    int32_t userId, int requestCode)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }
    if (CheckUISessionParams(data, callerToken, sessionInfo, userId, requestCode) == INNER_ERR) {
        return INNER_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_ADD_CALLER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StartAbilityByUIContentSession(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, const sptr<SessionInfo> &sessionInfo,
    int32_t userId, int requestCode)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&startOptions)) {
        HILOG_ERROR("startOptions write failed.");
        return INNER_ERR;
    }
    if (CheckUISessionParams(data, callerToken, sessionInfo, userId, requestCode) == INNER_ERR) {
        return INNER_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_FOR_OPTIONS, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StartExtensionAbility(const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }
    if (callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
            HILOG_ERROR("flag and callerToken write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("StartExtensionAbility, userId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(static_cast<int32_t>(extensionType))) {
        HILOG_ERROR("StartExtensionAbility, extensionType write failed.");
        return INNER_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::START_EXTENSION_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("StartExtensionAbility, Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::RequestModalUIExtension(const Want &want)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }

    int error;
    MessageParcel reply;
    MessageOption option;
    error = SendRequest(AbilityManagerInterfaceCode::REQUESET_MODAL_UIEXTENSION, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("RequestModalUIExtension, Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StartUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo, int32_t userId)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    CHECK_POINTER_AND_RETURN_LOG(extensionSessionInfo, ERR_INVALID_VALUE,
        "connect ability fail, extensionSessionInfo is nullptr");
    if (extensionSessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(extensionSessionInfo)) {
            HILOG_ERROR("flag and extensionSessionInfo write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }

    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("StartExtensionAbility, userId write failed.");
        return INNER_ERR;
    }

    error = SendRequest(AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("StartExtensionAbility, Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StartUIAbilityBySCB(sptr<SessionInfo> sessionInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (sessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(sessionInfo)) {
            HILOG_ERROR("flag and sessionInfo write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::START_UI_ABILITY_BY_SCB, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StopExtensionAbility(const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }
    if (callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
            HILOG_ERROR("flag and callerToken write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("userId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(static_cast<int32_t>(extensionType))) {
        HILOG_ERROR("extensionType write failed.");
        return INNER_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::STOP_EXTENSION_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    return TerminateAbility(token, resultCode, resultWant, true);
}

int AbilityManagerProxy::TerminateAbility(const sptr<IRemoteObject> &token,
    int resultCode, const Want *resultWant, bool flag)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (token) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(token)) {
            HILOG_ERROR("flag and token write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteInt32(resultCode) || !data.WriteParcelable(resultWant)) {
        HILOG_ERROR("data write failed.");
        return INNER_ERR;
    }
    if (!data.WriteBool(flag)) {
        HILOG_ERROR("data write flag failed.");
        return INNER_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::TERMINATE_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::TerminateUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo, int resultCode,
    const Want *resultWant)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    CHECK_POINTER_AND_RETURN_LOG(extensionSessionInfo, ERR_INVALID_VALUE,
        "connect ability fail, extensionSessionInfo is nullptr");
    if (extensionSessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(extensionSessionInfo)) {
            HILOG_ERROR("flag and extensionSessionInfo write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }

    if (!data.WriteInt32(resultCode) || !data.WriteParcelable(resultWant)) {
        HILOG_ERROR("data write failed.");
        return INNER_ERR;
    }

    error = SendRequest(AbilityManagerInterfaceCode::TERMINATE_UI_EXTENSION_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::CloseUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (sessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(sessionInfo)) {
            HILOG_ERROR("flag and sessionInfo write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }

    error = SendRequest(AbilityManagerInterfaceCode::CLOSE_UI_ABILITY_BY_SCB, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("failed, Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::SendResultToAbility(int32_t requestCode, int32_t resultCode, Want& resultWant)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(requestCode)) {
        HILOG_ERROR("requestCode write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(resultCode) || !data.WriteParcelable(&resultWant)) {
        HILOG_ERROR("data write failed.");
        return INNER_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::SEND_RESULT_TO_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::MoveAbilityToBackground(const sptr<IRemoteObject> &token)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (token) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(token)) {
            HILOG_ERROR("flag and token write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }
    error = SendRequest(AbilityManagerInterfaceCode::MOVE_ABILITY_TO_BACKGROUND, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d.", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::MoveUIAbilityToBackground(const sptr<IRemoteObject> token)
{
    CHECK_POINTER_AND_RETURN_LOG(token, ERR_INVALID_VALUE, "MoveUIAbilityToBackground fail, token is null");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return IPC_PROXY_ERR;
    }
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, RemoteObject, token);
    int32_t error = SendRequest(AbilityManagerInterfaceCode::MOVE_UI_ABILITY_TO_BACKGROUND, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d.", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::CloseAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    return TerminateAbility(token, resultCode, resultWant, false);
}

int AbilityManagerProxy::ConnectAbility(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    return ConnectAbilityCommon(want, connect, callerToken, AppExecFwk::ExtensionAbilityType::SERVICE, userId);
}

int AbilityManagerProxy::ConnectAbilityCommon(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken,
    AppExecFwk::ExtensionAbilityType extensionType, int32_t userId, bool isQueryExtensionOnly)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Parcelable, &want);
    CHECK_POINTER_AND_RETURN_LOG(connect, ERR_INVALID_VALUE, "connect ability fail, connect is nullptr");
    if (connect->AsObject()) {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, true);
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, RemoteObject, connect->AsObject());
    } else {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, false);
    }
    if (callerToken) {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, true);
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, RemoteObject, callerToken);
    } else {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, false);
    }
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Int32, userId);
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Int32, static_cast<int32_t>(extensionType));
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, isQueryExtensionOnly);
    int error = SendRequest(AbilityManagerInterfaceCode::CONNECT_ABILITY_WITH_TYPE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s, Send request error: %{public}d", __func__, error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::ConnectUIExtensionAbility(const Want &want, const sptr<IAbilityConnection> &connect,
    const sptr<SessionInfo> &sessionInfo, int32_t userId, sptr<UIExtensionAbilityConnectInfo> connectInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Parcelable, &want);
    CHECK_POINTER_AND_RETURN_LOG(connect, ERR_INVALID_VALUE, "connect ability fail, connect is nullptr");
    if (connect->AsObject()) {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, true);
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, RemoteObject, connect->AsObject());
    } else {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, false);
    }
    CHECK_POINTER_AND_RETURN_LOG(sessionInfo, ERR_INVALID_VALUE, "connect ability fail, sessionInfo is nullptr");
    if (sessionInfo) {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, true);
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Parcelable, sessionInfo);
    } else {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, false);
    }
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Int32, userId);
    if (connectInfo != nullptr) {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, true);
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Parcelable, connectInfo);
    } else {
        PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Bool, false);
    }

    int error = SendRequest(AbilityManagerInterfaceCode::CONNECT_UI_EXTENSION_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    if (connectInfo != nullptr) {
        sptr<UIExtensionAbilityConnectInfo> replyInfo = reply.ReadParcelable<UIExtensionAbilityConnectInfo>();
        if (replyInfo != nullptr) {
            connectInfo->uiExtensionAbilityId = replyInfo->uiExtensionAbilityId;
            HILOG_DEBUG("UIExtensionAbilityId is %{public}d.", connectInfo->uiExtensionAbilityId);
        }
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::DisconnectAbility(sptr<IAbilityConnection> connect)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (connect == nullptr) {
        HILOG_ERROR("disconnect ability fail, connect is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(connect->AsObject())) {
        HILOG_ERROR("connect write failed.");
        return ERR_INVALID_VALUE;
    }

    error = SendRequest(AbilityManagerInterfaceCode::DISCONNECT_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

sptr<IAbilityScheduler> AbilityManagerProxy::AcquireDataAbility(
    const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!callerToken) {
        HILOG_ERROR("invalid parameters for acquire data ability.");
        return nullptr;
    }
    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }
    if (!data.WriteString(uri.ToString()) || !data.WriteBool(tryBind) || !data.WriteRemoteObject(callerToken)) {
        HILOG_ERROR("data write failed.");
        return nullptr;
    }

    error = SendRequest(AbilityManagerInterfaceCode::ACQUIRE_DATA_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return nullptr;
    }

    return iface_cast<IAbilityScheduler>(reply.ReadRemoteObject());
}

int AbilityManagerProxy::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!dataAbilityScheduler || !callerToken) {
        return ERR_INVALID_VALUE;
    }
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(dataAbilityScheduler->AsObject()) || !data.WriteRemoteObject(callerToken)) {
        HILOG_ERROR("data write failed.");
        return INNER_ERR;
    }

    error = SendRequest(AbilityManagerInterfaceCode::RELEASE_DATA_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (scheduler == nullptr) {
        return ERR_INVALID_VALUE;
    }
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(scheduler->AsObject()) || !data.WriteRemoteObject(token)) {
        HILOG_ERROR("data write failed.");
        return ERR_INVALID_VALUE;
    }

    error = SendRequest(AbilityManagerInterfaceCode::ATTACH_ABILITY_THREAD, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(token) || !data.WriteInt32(state)) {
        HILOG_ERROR("token or state write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteParcelable(&saveData)) {
        HILOG_ERROR("saveData write failed.");
        return INNER_ERR;
    }

    error = SendRequest(AbilityManagerInterfaceCode::ABILITY_TRANSITION_DONE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::ScheduleConnectAbilityDone(
    const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (token) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(token)) {
            HILOG_ERROR("Failed to write flag and token.");
            return ERR_INVALID_VALUE;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("Failed to write flag.");
            return ERR_INVALID_VALUE;
        }
    }

    if (remoteObject) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(remoteObject)) {
            HILOG_ERROR("Failed to write flag and remoteObject.");
            return ERR_INVALID_VALUE;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("Failed to write flag.");
            return ERR_INVALID_VALUE;
        }
    }

    error = SendRequest(AbilityManagerInterfaceCode::CONNECT_ABILITY_DONE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("token write failed.");
        return ERR_INVALID_VALUE;
    }

    error = SendRequest(AbilityManagerInterfaceCode::DISCONNECT_ABILITY_DONE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("token write failed.");
        return ERR_INVALID_VALUE;
    }

    error = SendRequest(AbilityManagerInterfaceCode::COMMAND_ABILITY_DONE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::ScheduleCommandAbilityWindowDone(
    const sptr<IRemoteObject> &token,
    const sptr<SessionInfo> &sessionInfo,
    WindowCommand winCmd,
    AbilityCommand abilityCmd)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("token write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteParcelable(sessionInfo)) {
        HILOG_ERROR("sessionInfo write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteInt32(winCmd)) {
        HILOG_ERROR("winCmd write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteInt32(abilityCmd)) {
        HILOG_ERROR("abilityCmd write failed.");
        return ERR_INVALID_VALUE;
    }

    error = SendRequest(AbilityManagerInterfaceCode::COMMAND_ABILITY_WINDOW_DONE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

void AbilityManagerProxy::DumpSysState(
    const std::string& args, std::vector<std::string>& state, bool isClient, bool isUserId, int UserId)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteString16(Str8ToStr16(args));

    if (!data.WriteBool(isClient)) {
        HILOG_ERROR("data write failed.");
        return ;
    }
    if (!data.WriteBool(isUserId)) {
        HILOG_ERROR("data write failed.");
        return ;
    }
    if (!data.WriteInt32(UserId)) {
        HILOG_ERROR("data write failed.");
        return ;
    }

    error = SendRequest(AbilityManagerInterfaceCode::DUMPSYS_STATE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("AbilityManagerProxy: SendRequest err %{public}d", error);
        return;
    }
    int32_t stackNum = reply.ReadInt32();
    for (int i = 0; i < stackNum; i++) {
        std::string stac = Str16ToStr8(reply.ReadString16());
        state.emplace_back(stac);
    }
}

void AbilityManagerProxy::DumpState(const std::string &args, std::vector<std::string> &state)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteString16(Str8ToStr16(args));

    error = SendRequest(AbilityManagerInterfaceCode::DUMP_STATE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("AbilityManagerProxy: SendRequest err %{public}d", error);
        return;
    }
    int32_t stackNum = reply.ReadInt32();
    for (int i = 0; i < stackNum; i++) {
        std::string stac = Str16ToStr8(reply.ReadString16());
        state.emplace_back(stac);
    }
}

int AbilityManagerProxy::MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("token write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteBool(fromUser)) {
        HILOG_ERROR("data write failed.");
        return ERR_INVALID_VALUE;
    }

    error = SendRequest(AbilityManagerInterfaceCode::MINIMIZE_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::MinimizeUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo,
    bool fromUser)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    CHECK_POINTER_AND_RETURN_LOG(extensionSessionInfo, ERR_INVALID_VALUE,
        "connect ability fail, extensionSessionInfo is nullptr");
    if (extensionSessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(extensionSessionInfo)) {
            HILOG_ERROR("flag and extensionSessionInfo write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteBool(fromUser)) {
        HILOG_ERROR("data write failed.");
        return ERR_INVALID_VALUE;
    }

    error = SendRequest(AbilityManagerInterfaceCode::MINIMIZE_UI_EXTENSION_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::MinimizeUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool fromUser)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (sessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(sessionInfo)) {
            HILOG_ERROR("flag and sessionInfo write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteBool(fromUser)) {
        HILOG_ERROR("fromUser write failed.");
        return INNER_ERR;
    }

    error = SendRequest(AbilityManagerInterfaceCode::MINIMIZE_UI_ABILITY_BY_SCB, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("failed, Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StopServiceAbility(const Want &want, int32_t userId, const sptr<IRemoteObject> &token)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("userId write failed.");
        return INNER_ERR;
    }
    if (token) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(token)) {
            HILOG_ERROR("Failed to write flag and token.");
            return ERR_INVALID_VALUE;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("Failed to write flag.");
            return ERR_INVALID_VALUE;
        }
    }
    error = SendRequest(AbilityManagerInterfaceCode::STOP_SERVICE_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

template <typename T>
int AbilityManagerProxy::GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos)
{
    int32_t infoSize = reply.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        HILOG_ERROR("infoSize is too large");
        return ERR_INVALID_VALUE;
    }

    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<T> info(reply.ReadParcelable<T>());
        if (!info) {
            HILOG_ERROR("Read Parcelable infos failed.");
            return ERR_INVALID_VALUE;
        }
        parcelableInfos.emplace_back(*info);
    }
    return NO_ERROR;
}

int AbilityManagerProxy::GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
    MissionSnapshot& snapshot, bool isLowResolution)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString(deviceId)) {
        HILOG_ERROR("deviceId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("missionId write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteBool(isLowResolution)) {
        HILOG_ERROR("isLowResolution write failed.");
        return ERR_INVALID_VALUE;
    }
    error = SendRequest(AbilityManagerInterfaceCode::GET_MISSION_SNAPSHOT_INFO, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    std::unique_ptr<MissionSnapshot> info(reply.ReadParcelable<MissionSnapshot>());
    if (!info) {
        HILOG_ERROR("readParcelableInfo failed.");
        auto errorCode = reply.ReadInt32();
        return errorCode ? errorCode : ERR_UNKNOWN_OBJECT;
    }
    snapshot = *info;
    return reply.ReadInt32();
}

void AbilityManagerProxy::UpdateMissionSnapShot(const sptr<IRemoteObject> &token,
    const std::shared_ptr<Media::PixelMap> &pixelMap)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("write token failed.");
        return;
    }
    if (!data.WriteParcelable(pixelMap.get())) {
        HILOG_ERROR("write pixelMap failed.");
        return;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::UPDATE_MISSION_SNAPSHOT_FROM_WMS,
        data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
    }
}

void AbilityManagerProxy::EnableRecoverAbility(const sptr<IRemoteObject>& token)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("AppRecovery WriteInterfaceToken failed.");
        return;
    }

    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("AppRecovery WriteRemoteObject failed.");
        return;
    }

    error = SendRequest(AbilityManagerInterfaceCode::ABILITY_RECOVERY_ENABLE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("AppRecovery Send request error: %{public}d", error);
        return;
    }
    return;
}

void AbilityManagerProxy::ScheduleRecoverAbility(const sptr<IRemoteObject>& token, int32_t reason, const Want *want)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("AppRecovery WriteInterfaceToken failed.");
        return;
    }

    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("AppRecovery WriteRemoteObject failed.");
        return;
    }

    data.WriteInt32(reason);

    if (!data.WriteParcelable(want)) {
        HILOG_ERROR("AppRecovery write want failed.");
        return;
    }

    error = SendRequest(AbilityManagerInterfaceCode::ABILITY_RECOVERY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("AppRecovery Send request error: %{public}d", error);
        return;
    }
    return;
}

int AbilityManagerProxy::KillProcess(const std::string &bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString16(Str8ToStr16(bundleName))) {
        HILOG_ERROR("bundleName write failed.");
        return ERR_INVALID_VALUE;
    }
    int error = SendRequest(AbilityManagerInterfaceCode::KILL_PROCESS, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AbilityManagerProxy::ForceTimeoutForTest(const std::string &abilityName, const std::string &state)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString16(Str8ToStr16(abilityName))) {
        HILOG_ERROR("abilityName write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteString16(Str8ToStr16(state))) {
        HILOG_ERROR("abilityName write failed.");
        return ERR_INVALID_VALUE;
    }
    int error = SendRequest(AbilityManagerInterfaceCode::FORCE_TIMEOUT, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}
#endif

int AbilityManagerProxy::ClearUpApplicationData(const std::string &bundleName, const int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString16(Str8ToStr16(bundleName))) {
        HILOG_ERROR("bundleName write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("userId write failed.");
        return ERR_INVALID_VALUE;
    }
    int error = SendRequest(AbilityManagerInterfaceCode::CLEAR_UP_APPLICATION_DATA, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::UninstallApp(const std::string &bundleName, int32_t uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString16(Str8ToStr16(bundleName))) {
        HILOG_ERROR("bundleName write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteInt32(uid)) {
        HILOG_ERROR("uid write failed.");
        return ERR_INVALID_VALUE;
    }
    int error = SendRequest(AbilityManagerInterfaceCode::UNINSTALL_APP, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::UpgradeApp(const std::string &bundleName, const int32_t uid, const std::string &exitMsg)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, String16, Str8ToStr16(bundleName));
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Int32, uid);
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, String16, Str8ToStr16(exitMsg));
    int error = SendRequest(AbilityManagerInterfaceCode::UPGRADE_APP, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

sptr<IWantSender> AbilityManagerProxy::GetWantSender(
    const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }
    if (!data.WriteParcelable(&wantSenderInfo)) {
        HILOG_ERROR("wantSenderInfo write failed.");
        return nullptr;
    }
    if (callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
            HILOG_ERROR("flag and callerToken write failed.");
            return nullptr;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return nullptr;
        }
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return nullptr;
    }
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(reply.ReadRemoteObject());
    if (!wantSender) {
        return nullptr;
    }
    return wantSender;
}

int AbilityManagerProxy::SendWantSender(sptr<IWantSender> target, const SenderInfo &senderInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (target == nullptr || !data.WriteRemoteObject(target->AsObject())) {
        HILOG_ERROR("target write failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&senderInfo)) {
        HILOG_ERROR("senderInfo write failed.");
        return INNER_ERR;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::SEND_PENDING_WANT_SENDER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

void AbilityManagerProxy::CancelWantSender(const sptr<IWantSender> &sender)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (sender == nullptr || !data.WriteRemoteObject(sender->AsObject())) {
        HILOG_ERROR("sender write failed.");
        return;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::CANCEL_PENDING_WANT_SENDER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return;
    }
}

int AbilityManagerProxy::GetPendingWantUid(const sptr<IWantSender> &target)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (target == nullptr || !data.WriteRemoteObject(target->AsObject())) {
        HILOG_ERROR("target write failed.");
        return ERR_INVALID_VALUE;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::GET_PENDING_WANT_UID, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::GetPendingWantUserId(const sptr<IWantSender> &target)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (target == nullptr || !data.WriteRemoteObject(target->AsObject())) {
        HILOG_ERROR("target write failed.");
        return ERR_INVALID_VALUE;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::GET_PENDING_WANT_USERID, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

std::string AbilityManagerProxy::GetPendingWantBundleName(const sptr<IWantSender> &target)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return "";
    }
    if (target == nullptr || !data.WriteRemoteObject(target->AsObject())) {
        HILOG_ERROR("target write failed.");
        return "";
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::GET_PENDING_WANT_BUNDLENAME, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return "";
    }
    return Str16ToStr8(reply.ReadString16());
}

int AbilityManagerProxy::GetPendingWantCode(const sptr<IWantSender> &target)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (target == nullptr || !data.WriteRemoteObject(target->AsObject())) {
        HILOG_ERROR("target write failed.");
        return ERR_INVALID_VALUE;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::GET_PENDING_WANT_CODE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::GetPendingWantType(const sptr<IWantSender> &target)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (target == nullptr || !data.WriteRemoteObject(target->AsObject())) {
        HILOG_ERROR("target write failed.");
        return ERR_INVALID_VALUE;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::GET_PENDING_WANT_TYPE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

void AbilityManagerProxy::RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (sender == nullptr || !data.WriteRemoteObject(sender->AsObject())) {
        HILOG_ERROR("sender write failed.");
        return;
    }
    if (receiver == nullptr || !data.WriteRemoteObject(receiver->AsObject())) {
        HILOG_ERROR("receiver write failed.");
        return;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::REGISTER_CANCEL_LISTENER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return;
    }
}

void AbilityManagerProxy::UnregisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (sender == nullptr || !data.WriteRemoteObject(sender->AsObject())) {
        HILOG_ERROR("sender write failed.");
        return;
    }
    if (receiver == nullptr || !data.WriteRemoteObject(receiver->AsObject())) {
        HILOG_ERROR("receiver write failed.");
        return;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::UNREGISTER_CANCEL_LISTENER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return;
    }
}

int AbilityManagerProxy::GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (target == nullptr || !data.WriteRemoteObject(target->AsObject())) {
        HILOG_ERROR("target write failed.");
        return INNER_ERR;
    }
    if (want == nullptr || !data.WriteParcelable(want.get())) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::GET_PENDING_REQUEST_WANT, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    std::unique_ptr<Want> wantInfo(reply.ReadParcelable<Want>());
    if (!wantInfo) {
        HILOG_ERROR("readParcelableInfo failed");
        return INNER_ERR;
    }
    want = std::move(wantInfo);

    return NO_ERROR;
}

int AbilityManagerProxy::GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (target == nullptr || !data.WriteRemoteObject(target->AsObject())) {
        HILOG_ERROR("target write failed.");
        return INNER_ERR;
    }
    if (info == nullptr || !data.WriteParcelable(info.get())) {
        HILOG_ERROR("info write failed.");
        return INNER_ERR;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER_INFO, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    std::unique_ptr<WantSenderInfo> wantSenderInfo(reply.ReadParcelable<WantSenderInfo>());
    if (!wantSenderInfo) {
        HILOG_ERROR("readParcelable Info failed");
        return INNER_ERR;
    }
    info = std::move(wantSenderInfo);

    return NO_ERROR;
}

int AbilityManagerProxy::GetAppMemorySize()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("WriteInterfaceToken faild");
        return INNER_ERR;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::GET_APP_MEMORY_SIZE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

bool AbilityManagerProxy::IsRamConstrainedDevice()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("WriteInterfaceToken faild");
        return false;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::IS_RAM_CONSTRAINED_DEVICE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

int AbilityManagerProxy::ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
    int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams)
{
    HILOG_INFO("amsProxy %{public}s called.", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString(srcDeviceId)) {
        HILOG_ERROR("srcDeviceId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteString(dstDeviceId)) {
        HILOG_ERROR("dstDeviceId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("missionId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(callBack)) {
        HILOG_ERROR("callBack write failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&wantParams)) {
        HILOG_ERROR("wantParams write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::CONTINUE_MISSION, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
    const std::string &bundleName, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString(srcDeviceId)) {
        HILOG_ERROR("srcDeviceId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteString(dstDeviceId)) {
        HILOG_ERROR("dstDeviceId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("missionId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(callBack)) {
        HILOG_ERROR("callBack write failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&wantParams)) {
        HILOG_ERROR("wantParams write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::CONTINUE_MISSION_OF_BUNDLENAME, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString(deviceId)) {
        HILOG_ERROR("deviceId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("missionId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteUint32(versionCode)) {
        HILOG_ERROR("versionCode write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::CONTINUE_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(abilityToken)) {
        HILOG_ERROR("abilityToken write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(status)) {
        HILOG_ERROR("status write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::START_CONTINUATION, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

void AbilityManagerProxy::NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteString(deviceId)) {
        HILOG_ERROR("deviceId write failed.");
        return;
    }
    if (!data.WriteInt32(sessionId)) {
        HILOG_ERROR("sessionId write failed.");
        return;
    }
    if (!data.WriteBool(isSuccess)) {
        HILOG_ERROR("result write failed.");
        return;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::NOTIFY_COMPLETE_CONTINUATION, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return;
    }
}

int AbilityManagerProxy::NotifyContinuationResult(int32_t missionId, int32_t result)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("missionId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(result)) {
        HILOG_ERROR("result write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::NOTIFY_CONTINUATION_RESULT, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::LockMissionForCleanup(int32_t missionId)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("lock mission by id , WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }

    error = SendRequest(AbilityManagerInterfaceCode::LOCK_MISSION_FOR_CLEANUP, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("lock mission by id , error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::UnlockMissionForCleanup(int32_t missionId)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("unlock mission by id , WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }
    error = SendRequest(AbilityManagerInterfaceCode::UNLOCK_MISSION_FOR_CLEANUP, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("unlock mission by id , error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

void AbilityManagerProxy::SetLockedState(int32_t sessionId, bool lockedState)
{
    MessageParcel data;

    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteInt32(sessionId)) {
        HILOG_ERROR("lock abilityRecord by id , WriteInt32 fail.");
        return;
    }

    if (!data.WriteBool(lockedState)) {
        HILOG_ERROR("WriteBool fail.");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto error = SendRequest(AbilityManagerInterfaceCode::SET_SESSION_LOCKED_STATE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("lock or unlock abilityRecord by sessionId , error: %d", error);
        return;
    }
    return;
}

int AbilityManagerProxy::RegisterMissionListener(const sptr<IMissionListener> &listener)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!listener) {
        HILOG_ERROR("register mission listener, listener is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        HILOG_ERROR("write mission listener failed when register mission listener.");
        return ERR_INVALID_VALUE;
    }

    error = SendRequest(AbilityManagerInterfaceCode::REGISTER_MISSION_LISTENER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::RegisterSessionHandler(const sptr<IRemoteObject> &object)
{
    if (!object) {
        HILOG_ERROR("register session handler, handler is nullptr");
        return ERR_INVALID_VALUE;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(object)) {
        HILOG_ERROR("write session handler failed when register session handler.");
        return ERR_INVALID_VALUE;
    }
    int error = SendRequest(AbilityManagerInterfaceCode::REGISTER_SESSION_HANDLER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::RegisterMissionListener(const std::string &deviceId,
    const sptr<IRemoteMissionListener> &listener)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString(deviceId)) {
        HILOG_ERROR("deviceId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        HILOG_ERROR("listener write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::REGISTER_REMOTE_MISSION_LISTENER,
        data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::RegisterOnListener(const std::string &type,
    const sptr<IRemoteOnListener> &listener)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString(type)) {
        HILOG_ERROR("type write failed.");
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        HILOG_ERROR("listener write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::REGISTER_REMOTE_ON_LISTENER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::RegisterOffListener(const std::string &type,
    const sptr<IRemoteOnListener> &listener)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString(type)) {
        HILOG_ERROR("type write failed.");
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        HILOG_ERROR("listener write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::REGISTER_REMOTE_OFF_LISTENER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::UnRegisterMissionListener(const sptr<IMissionListener> &listener)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!listener) {
        HILOG_ERROR("unregister mission listener, listener is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        HILOG_ERROR("write mission listener failed when unregister mission listener.");
        return ERR_INVALID_VALUE;
    }

    error = SendRequest(AbilityManagerInterfaceCode::UNREGISTER_MISSION_LISTENER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::GetMissionInfos(const std::string& deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString16(Str8ToStr16(deviceId))) {
        HILOG_ERROR("write deviceId failed when GetMissionInfos.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteInt32(numMax)) {
        HILOG_ERROR("GetMissionInfos numMax write failed.");
        return ERR_INVALID_VALUE;
    }
    error = SendRequest(AbilityManagerInterfaceCode::GET_MISSION_INFOS, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("GetMissionInfos Send request error: %{public}d", error);
        return error;
    }
    error = GetParcelableInfos<MissionInfo>(reply, missionInfos);
    if (error != NO_ERROR) {
        HILOG_ERROR("GetMissionInfos error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::GetMissionInfo(const std::string& deviceId, int32_t missionId,
    MissionInfo &missionInfo)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString16(Str8ToStr16(deviceId))) {
        HILOG_ERROR("write deviceId failed when GetMissionInfo.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("GetMissionInfo write missionId failed.");
        return ERR_INVALID_VALUE;
    }
    error = SendRequest(AbilityManagerInterfaceCode::GET_MISSION_INFO_BY_ID, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("GetMissionInfo Send request error: %{public}d", error);
        return error;
    }

    std::unique_ptr<MissionInfo> info(reply.ReadParcelable<MissionInfo>());
    if (!info) {
        HILOG_ERROR("read missioninfo failed.");
        return ERR_UNKNOWN_OBJECT;
    }
    missionInfo = *info;
    return reply.ReadInt32();
}

int AbilityManagerProxy::CleanMission(int32_t missionId)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("clean mission by id , WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }
    error = SendRequest(AbilityManagerInterfaceCode::CLEAN_MISSION, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("clean mission by id , error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::CleanAllMissions()
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::CLEAN_ALL_MISSIONS, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("lock mission by id ,SendRequest error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::MoveMissionToFront(int32_t missionId)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("move mission to front , WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }
    error = SendRequest(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("move mission to front, SendRequest error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::MoveMissionToFront(int32_t missionId, const StartOptions &startOptions)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("move mission to front , WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteParcelable(&startOptions)) {
        HILOG_ERROR("startOptions write failed.");
        return INNER_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT_BY_OPTIONS, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("move mission to front, SendRequest error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (!data.WriteInt32Vector(missionIds)) {
        HILOG_ERROR("mission id write failed.");
        return INNER_ERR;
    }

    if (!data.WriteInt32(topMissionId)) {
        HILOG_ERROR("top mission id write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_FOREGROUND, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("query front missionInfo failed: send request error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int AbilityManagerProxy::MoveMissionsToBackground(const std::vector<int32_t>& missionIds, std::vector<int32_t>& result)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (!data.WriteInt32Vector(missionIds)) {
        HILOG_ERROR("mission id write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_BACKGROUND, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("query front missionInfo failed: send request error: %{public}d", error);
        return error;
    }

    if (!reply.ReadInt32Vector(&result)) {
        HILOG_ERROR("read result failed");
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StartUser(int userId, sptr<IUserCallback> callback)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("StartUser:WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }
    if (!callback) {
        data.WriteBool(false);
    } else {
        data.WriteBool(true);
        if (!data.WriteRemoteObject(callback->AsObject())) {
            HILOG_ERROR("write IUserCallback fail.");
            return ERR_INVALID_VALUE;
        }
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto error = SendRequest(AbilityManagerInterfaceCode::START_USER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("StartUser:SendRequest error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::SetMissionContinueState(const sptr<IRemoteObject> &token, const AAFwk::ContinueState &state)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("SetMissionContinueState write token failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteInt32(static_cast<int32_t>(state))) {
        HILOG_ERROR("SetMissionContinueState write state failed.");
        return ERR_INVALID_VALUE;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::SET_MISSION_CONTINUE_STATE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("SetMissionContinueState Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StopUser(int userId, const sptr<IUserCallback> &callback)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("StopUser:WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }

    if (!callback) {
        data.WriteBool(false);
    } else {
        data.WriteBool(true);
        if (!data.WriteRemoteObject(callback->AsObject())) {
            HILOG_ERROR("write IUserCallback fail.");
            return ERR_INVALID_VALUE;
        }
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto error = SendRequest(AbilityManagerInterfaceCode::STOP_USER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("StopUser:SendRequest error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::LogoutUser(int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("LogoutUser:WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }
    int error = SendRequest(AbilityManagerInterfaceCode::LOGOUT_USER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("LogoutUser:SendRequest error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

#ifdef SUPPORT_GRAPHICS
int AbilityManagerProxy::SetMissionLabel(const sptr<IRemoteObject> &token, const std::string &label)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("SetMissionLabel write token failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteString16(Str8ToStr16(label))) {
        HILOG_ERROR("SetMissionLabel write label failed.");
        return ERR_INVALID_VALUE;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::SET_MISSION_LABEL, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("SetMissionLabel Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::SetMissionIcon(const sptr<IRemoteObject> &token,
    const std::shared_ptr<OHOS::Media::PixelMap> &icon)
{
    if (!token || !icon) {
        HILOG_ERROR("SetMissionIcon abilitytoken or icon is invalid.");
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("SetMissionIcon write token failed.");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteParcelable(icon.get())) {
        HILOG_ERROR("SetMissionIcon write icon failed.");
        return ERR_INVALID_VALUE;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::SET_MISSION_ICON, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("SetMissionIcon Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler)
{
    if (!handler) {
        HILOG_ERROR("%{public}s: handler is nullptr.", __func__);
        return INNER_ERR;
    }
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("%{public}s: write interface token failed.", __func__);
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(handler->AsObject())) {
        HILOG_ERROR("%{public}s: handler write failed.", __func__);
        return INNER_ERR;
    }
    MessageOption option;
    MessageParcel reply;
    auto error = SendRequest(AbilityManagerInterfaceCode::REGISTER_WMS_HANDLER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s: send request error: %{public}d", __func__, error);
        return error;
    }
    return reply.ReadInt32();
}

void AbilityManagerProxy::CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("%{public}s: write interface token failed.", __func__);
        return;
    }
    if (!data.WriteRemoteObject(abilityToken)) {
        HILOG_ERROR("%{public}s: abilityToken write failed.", __func__);
        return;
    }
    MessageOption option;
    MessageParcel reply;
    auto error = SendRequest(AbilityManagerInterfaceCode::COMPLETEFIRSTFRAMEDRAWING, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s: send request error: %{public}d", __func__, error);
    }
}

int AbilityManagerProxy::PrepareTerminateAbility(const sptr<IRemoteObject> &token,
    sptr<IPrepareTerminateCallback> &callback)
{
    if (!callback) {
        HILOG_ERROR("callback is nullptr.");
        return INNER_ERR;
    }
    int error = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("write interface token failed.");
        return INNER_ERR;
    }
    if (token) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(token)) {
            HILOG_ERROR("write token failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("write token failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        HILOG_ERROR("weite callback failed.");
        return INNER_ERR;
    }

    error = SendRequest(AbilityManagerInterfaceCode::PREPARE_TERMINATE_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("send request failed. error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int AbilityManagerProxy::GetDialogSessionInfo(const std::string dialogSessionId, sptr<DialogSessionInfo> &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("write interface fail.");
        return INNER_ERR;
    }
    if (!data.WriteString(dialogSessionId)) {
        HILOG_ERROR("write dialogSessionId fail.");
        return ERR_INVALID_VALUE;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::GET_DIALOG_SESSION_INFO, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Get extension running info failed., error: %{public}d", error);
        return error;
    }
    info = reply.ReadParcelable<DialogSessionInfo>();
    if (!info) {
        HILOG_ERROR("read IRemoteObject failed.");
        return ERR_UNKNOWN_OBJECT;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::SendDialogResult(const Want &want, const std::string dialogSessionId, const bool isAllow)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteString(dialogSessionId)) {
        HILOG_ERROR("write dialogSessionId fail.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteBool(isAllow)) {
        HILOG_ERROR("write dialogSessionId fail.");
        return ERR_INVALID_VALUE;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::SEND_DIALOG_RESULT, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Get extension running info failed., error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}
#endif

int AbilityManagerProxy::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::GET_ABILITY_RUNNING_INFO, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Get ability running info, error: %{public}d", error);
        return error;
    }
    error = GetParcelableInfos<AbilityRunningInfo>(reply, info);
    if (error != NO_ERROR) {
        HILOG_ERROR("GetParcelableInfos fail, error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (!data.WriteInt32(upperLimit)) {
        HILOG_ERROR("upperLimit write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::GET_EXTENSION_RUNNING_INFO, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Get extension running info failed., error: %{public}d", error);
        return error;
    }
    error = GetParcelableInfos<ExtensionRunningInfo>(reply, info);
    if (error != NO_ERROR) {
        HILOG_ERROR("GetParcelableInfos fail, error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::GET_PROCESS_RUNNING_INFO, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Get process running info, error: %{public}d", error);
        return error;
    }
    error = GetParcelableInfos<AppExecFwk::RunningProcessInfo>(reply, info);
    if (error != NO_ERROR) {
        HILOG_ERROR("GetParcelableInfos fail, error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag)
{
    HILOG_INFO("called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("WriteInterfaceToken failed");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteString(devId)) {
        HILOG_ERROR("write deviceId fail.");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteBool(fixConflict)) {
        HILOG_ERROR("WriteBool fail.");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteInt64(tag)) {
        HILOG_ERROR("WriteInt64 fail.");
        return ERR_INVALID_VALUE;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::START_SYNC_MISSIONS, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::StopSyncRemoteMissions(const std::string& devId)
{
    HILOG_INFO("called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("WriteInterfaceToken failed");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteString(devId)) {
        HILOG_ERROR("write deviceId fail.");
        return ERR_INVALID_VALUE;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::STOP_SYNC_MISSIONS, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::UnRegisterMissionListener(const std::string &deviceId,
    const sptr<IRemoteMissionListener> &listener)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString(deviceId)) {
        HILOG_ERROR("deviceId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        HILOG_ERROR("listener write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::UNREGISTER_REMOTE_MISSION_LISTENER,
        data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StartAbilityByCall(const Want &want, const sptr<IAbilityConnection> &connect,
    const sptr<IRemoteObject> &callerToken, int32_t accountId)
{
    HILOG_DEBUG("AbilityManagerProxy::StartAbilityByCall begin.");
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return ERR_INVALID_VALUE;
    }
    if (connect == nullptr) {
        HILOG_ERROR("resolve ability fail, connect is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteRemoteObject(connect->AsObject())) {
        HILOG_ERROR("resolve write failed.");
        return ERR_INVALID_VALUE;
    }
    if (callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
            HILOG_ERROR("Failed to write flag and callerToken.");
            return ERR_INVALID_VALUE;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("Failed to write flag.");
            return ERR_INVALID_VALUE;
        }
    }
    if (!data.WriteInt32(accountId)) {
        HILOG_ERROR("accountId write failed.");
        return ERR_INVALID_VALUE;
    }

    HILOG_DEBUG("AbilityManagerProxy::StartAbilityByCall SendRequest Call.");
    error = SendRequest(AbilityManagerInterfaceCode::START_CALL_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    HILOG_DEBUG("AbilityManagerProxy::StartAbilityByCall end.");
    return reply.ReadInt32();
}

void AbilityManagerProxy::CallRequestDone(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callStub)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (token == nullptr) {
        HILOG_ERROR("Call request done fail, ability token is nullptr.");
        return;
    }
    if (callStub == nullptr) {
        HILOG_ERROR("Call request done fail, callStub is nullptr.");
        return;
    }

    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("WriteRemoteObject fail, write token fail.");
        return;
    }
    if (!data.WriteRemoteObject(callStub)) {
        HILOG_ERROR("WriteRemoteObject fail, write callStub fail.");
        return;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::CALL_REQUEST_DONE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return;
    }
}

int AbilityManagerProxy::ReleaseCall(
    const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (connect == nullptr) {
        HILOG_ERROR("release calll ability fail, connect is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(connect->AsObject())) {
        HILOG_ERROR("release ability connect write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteParcelable(&element)) {
        HILOG_ERROR("element error.");
        return ERR_INVALID_VALUE;
    }

    error = SendRequest(AbilityManagerInterfaceCode::RELEASE_CALL_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

void AbilityManagerProxy::GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(callStub)) {
        HILOG_ERROR("WriteRemoteObject fail, write callStub fail.");
        return;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::GET_ABILITY_TOKEN, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return;
    }
    token = sptr<IRemoteObject>(reply.ReadRemoteObject());
}

int AbilityManagerProxy::RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(handler->AsObject())) {
        HILOG_ERROR("snapshot: handler write failed.");
        return INNER_ERR;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::REGISTER_SNAPSHOT_HANDLER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("snapshot: send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::SetAbilityController(const sptr<AppExecFwk::IAbilityController> &abilityController,
    bool imAStabilityTest)
{
    if (!abilityController) {
        HILOG_ERROR("abilityController nullptr");
        return ERR_INVALID_VALUE;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(abilityController->AsObject())) {
        HILOG_ERROR("abilityController write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteBool(imAStabilityTest)) {
        HILOG_ERROR("imAStabilityTest write failed.");
        return ERR_INVALID_VALUE;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::SET_ABILITY_CONTROLLER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

bool AbilityManagerProxy::IsRunningInStabilityTest()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return false;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::IS_USER_A_STABILITY_TEST, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

int AbilityManagerProxy::StartUserTest(const Want &want, const sptr<IRemoteObject> &observer)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(observer)) {
        HILOG_ERROR("observer write failed.");
        return INNER_ERR;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::START_USER_TEST, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::FinishUserTest(
    const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString(msg)) {
        HILOG_ERROR("msg write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteInt64(resultCode)) {
        HILOG_ERROR("resultCode:WriteInt64 fail.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("bundleName write failed.");
        return ERR_INVALID_VALUE;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::FINISH_USER_TEST, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::GetTopAbility(sptr<IRemoteObject> &token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::GET_TOP_ABILITY_TOKEN, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }

    token = sptr<IRemoteObject>(reply.ReadRemoteObject());
    if (!token) {
        HILOG_ERROR("read IRemoteObject failed.");
        return ERR_UNKNOWN_OBJECT;
    }

    return reply.ReadInt32();
}

int AbilityManagerProxy::CheckUIExtensionIsFocused(uint32_t uiExtensionTokenId, bool& isFocused)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (!data.WriteUint32(uiExtensionTokenId)) {
        HILOG_ERROR("uiExtensionTokenId write failed.");
        return ERR_INVALID_VALUE;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::CHECK_UI_EXTENSION_IS_FOCUSED, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }

    isFocused = reply.ReadBool();
    return NO_ERROR;
}

int AbilityManagerProxy::DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("data write failed.");
        return ERR_INVALID_VALUE;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::DELEGATOR_DO_ABILITY_FOREGROUND,
        data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int AbilityManagerProxy::DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("data write failed.");
        return ERR_INVALID_VALUE;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::DELEGATOR_DO_ABILITY_BACKGROUND,
        data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int AbilityManagerProxy::DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("data write failed.");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteUint32(flag)) {
        HILOG_ERROR("flag write failed.");
        return ERR_INVALID_VALUE;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::DO_ABILITY_FOREGROUND, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int AbilityManagerProxy::DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("data write failed.");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteUint32(flag)) {
        HILOG_ERROR("flag write failed.");
        return ERR_INVALID_VALUE;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::DO_ABILITY_BACKGROUND, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int AbilityManagerProxy::SendANRProcessID(int pid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(pid)) {
        HILOG_ERROR("pid WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::SEND_APP_NOT_RESPONSE_PROCESS_ID,
        data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("SendANRProcessID error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::GetMissionIdByToken(const sptr<IRemoteObject> &token)
{
    if (!token) {
        HILOG_ERROR("token is nullptr.");
        return -1;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("data interface token failed.");
        return -1;
    }

    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("data write failed.");
        return -1;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::GET_MISSION_ID_BY_ABILITY_TOKEN,
        data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return -1;
    }

    return reply.ReadInt32();
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AbilityManagerProxy::BlockAmsService()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::BLOCK_AMS_SERVICE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("BlockAmsService error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::BlockAbility(int32_t abilityRecordId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(abilityRecordId)) {
        HILOG_ERROR("pid WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::BLOCK_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("BlockAbility error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::BlockAppService()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::BLOCK_APP_SERVICE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("BlockAmsService error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}
#endif
int AbilityManagerProxy::FreeInstallAbilityFromRemote(const Want &want, const sptr<IRemoteObject> &callback,
    int32_t userId, int requestCode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("write interface token failed.");
        return INNER_ERR;
    }

    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }

    if (!data.WriteRemoteObject(callback)) {
        HILOG_ERROR("callback write failed.");
        return INNER_ERR;
    }

    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("userId write failed.");
        return INNER_ERR;
    }

    if (!data.WriteInt32(requestCode)) {
        HILOG_ERROR("requestCode write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::FREE_INSTALL_ABILITY_FROM_REMOTE,
        data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int AbilityManagerProxy::AddFreeInstallObserver(const sptr<AbilityRuntime::IFreeInstallObserver> &observer)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("write interface token failed.");
        return INNER_ERR;
    }

    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOG_ERROR("observer write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::ADD_FREE_INSTALL_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::DumpAbilityInfoDone(std::vector<std::string> &infos, const sptr<IRemoteObject> &callerToken)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("write interface token failed.");
        return INNER_ERR;
    }

    if (!data.WriteStringVector(infos)) {
        HILOG_ERROR("infos write failed.");
        return INNER_ERR;
    }

    if (!data.WriteRemoteObject(callerToken)) {
        HILOG_ERROR("infos write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::DUMP_ABILITY_INFO_DONE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::IsValidMissionIds(
    const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results)
{
    HILOG_INFO("IsValidMissionIds Call. Quert size is %{public}zu", missionIds.size());
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("write interface token failed.");
        return INNER_ERR;
    }

    constexpr int32_t MAX_COUNT = 20;
    int32_t num = missionIds.size() > MAX_COUNT ? MAX_COUNT : missionIds.size();
    data.WriteInt32(num);
    for (auto i = 0; i < num; ++i) {
        data.WriteInt32(missionIds.at(i));
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::QUERY_MISSION_VAILD, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }

    auto resultCode = reply.ReadInt32();
    if (resultCode != ERR_OK) {
        HILOG_ERROR("Send request reply error: %{public}d", resultCode);
        return resultCode;
    }

    auto infoSize = reply.ReadInt32();
    for (auto i = 0; i < infoSize && i < MAX_COUNT; ++i) {
        std::unique_ptr<MissionValidResult> info(reply.ReadParcelable<MissionValidResult>());
        if (!info) {
            HILOG_ERROR("Read Parcelable result infos failed.");
            return INNER_ERR;
        }
        results.emplace_back(*info);
    }

    return resultCode;
}

int AbilityManagerProxy::VerifyPermission(const std::string &permission, int pid, int uid)
{
    HILOG_INFO("VerifyPermission Call");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("write interface token failed.");
        return INNER_ERR;
    }

    if (!data.WriteString(permission)) {
        HILOG_ERROR("permission write failed.");
        return INNER_ERR;
    }

    if (!data.WriteInt32(pid)) {
        HILOG_ERROR("pid write failed.");
        return INNER_ERR;
    }

    if (!data.WriteInt32(uid)) {
        HILOG_ERROR("uid write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::VERIFY_PERMISSION, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::RequestDialogService(const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("RequestDialogService Call");
    if (!callerToken) {
        HILOG_ERROR("callerToken is invalid.");
        return ERR_INVALID_CALLER;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return INNER_ERR;
    }

    if (!data.WriteRemoteObject(callerToken)) {
        HILOG_ERROR("infos write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::REQUEST_DIALOG_SERVICE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("request dialog service Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::ReportDrawnCompleted(const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("called.");
    if (callerToken == nullptr) {
        HILOG_ERROR("callerToken is nullptr");
        return INNER_ERR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (!data.WriteRemoteObject(callerToken)) {
        HILOG_ERROR("callerToken write failed.");
        return INNER_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::REPORT_DRAWN_COMPLETED, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::AcquireShareData(
    const int32_t &missionId, const sptr<IAcquireShareDataCallback> &shareData)
{
    HILOG_INFO("AbilityManagerProxy::AcquireShareData start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("write interface token failed.");
        return INNER_ERR;
    }

    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("missionId write failed.");
        return INNER_ERR;
    }

    if (shareData == nullptr || !data.WriteRemoteObject(shareData->AsObject())) {
        HILOG_ERROR("shareData write failed.");
        return INNER_ERR;
    }

    int32_t error = SendRequest(AbilityManagerInterfaceCode::ACQUIRE_SHARE_DATA, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("AcquireShareData fail to Send request, err: %{public}d.", error);
        return INNER_ERR;
    }
    HILOG_INFO("AbilityManagerProxy::AcquireShareData end.");
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::ShareDataDone(
    const sptr<IRemoteObject> &token, const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam)
{
    HILOG_INFO("AbilityManagerProxy::ShareDataDone start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("write interface token failed.");
        return INNER_ERR;
    }

    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("token write failed.");
        return INNER_ERR;
    }

    if (!data.WriteInt32(resultCode)) {
        HILOG_ERROR("requestCode write failed.");
        return INNER_ERR;
    }

    if (!data.WriteInt32(uniqueId)) {
        HILOG_ERROR("uniqueId write failed.");
        return INNER_ERR;
    }

    if (!data.WriteParcelable(&wantParam)) {
        HILOG_ERROR("wantParam write failed.");
        return INNER_ERR;
    }

    int32_t error = SendRequest(AbilityManagerInterfaceCode::SHARE_DATA_DONE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("ShareDataDone fail to SendRequest, err: %{public}d.", error);
        return error;
    }
    HILOG_INFO("AbilityManagerProxy::ShareDataDone end.");
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::ForceExitApp(const int32_t pid, const ExitReason &exitReason)
{
    HILOG_DEBUG("start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("write interface token failed.");
        return INNER_ERR;
    }
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Int32, pid);
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Parcelable, &exitReason);

    int32_t error = SendRequest(AbilityManagerInterfaceCode::FORCE_EXIT_APP, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("fail to SendRequest, err: %{public}d.", error);
        return error;
    }

    HILOG_DEBUG("end.");
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::RecordAppExitReason(const ExitReason &exitReason)
{
    HILOG_DEBUG("start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("write interface token failed.");
        return INNER_ERR;
    }
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Parcelable, &exitReason);

    int32_t error = SendRequest(AbilityManagerInterfaceCode::RECORD_APP_EXIT_REASON, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("fail to SendRequest, err: %{public}d.", error);
        return error;
    }

    HILOG_DEBUG("end.");
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::RecordProcessExitReason(const int32_t pid, const ExitReason &exitReason)
{
    HILOG_DEBUG("start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("write interface token failed.");
        return INNER_ERR;
    }
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Int32, pid);
    PROXY_WRITE_PARCEL_AND_RETURN_IF_FAIL(data, Parcelable, &exitReason);

    int32_t error = SendRequest(AbilityManagerInterfaceCode::RECORD_PROCESS_EXIT_REASON, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("fail to SendRequest, err: %{public}d.", error);
        return error;
    }

    HILOG_DEBUG("end.");
    return reply.ReadInt32();
}

void AbilityManagerProxy::SetRootSceneSession(const sptr<IRemoteObject> &rootSceneSession)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("WriteInterfaceToken failed.");
        return;
    }
    if (!data.WriteRemoteObject(rootSceneSession)) {
        HILOG_ERROR("WriteRemoteObject failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto error = SendRequest(AbilityManagerInterfaceCode::SET_ROOT_SCENE_SESSION, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
    }
}

void AbilityManagerProxy::CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("WriteInterfaceToken failed.");
        return;
    }
    if (sessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(sessionInfo)) {
            HILOG_ERROR("flag and sessionInfo write failed.");
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return;
        }
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto error = SendRequest(AbilityManagerInterfaceCode::CALL_ABILITY_BY_SCB, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
    }
}

void AbilityManagerProxy::StartSpecifiedAbilityBySCB(const Want &want)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("WriteInterfaceToken failed.");
        return;
    }

    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto error = SendRequest(AbilityManagerInterfaceCode::START_SPECIFIED_ABILITY_BY_SCB, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
    }
}

int32_t AbilityManagerProxy::NotifySaveAsResult(const Want &want, int resultCode, int requestCode)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("WriteInterfaceToken failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("WriteWantObject failed.");
        return INNER_ERR;
    }

    if (!data.WriteInt32(resultCode)) {
        HILOG_ERROR("resultCode write failed.");
        return INNER_ERR;
    }

    if (!data.WriteInt32(requestCode)) {
        HILOG_ERROR("requestCode write failed.");
        return INNER_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    auto error = SendRequest(AbilityManagerInterfaceCode::NOTIFY_SAVE_AS_RESULT, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
    }

    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::SetSessionManagerService(const sptr<IRemoteObject> &sessionManagerService)
{
    HILOG_INFO("AbilityManagerProxy::SetSessionManagerService start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("write interface token failed.");
        return INNER_ERR;
    }

    if (!data.WriteRemoteObject(sessionManagerService)) {
        HILOG_ERROR("token write failed.");
        return INNER_ERR;
    }

    int32_t error = SendRequest(AbilityManagerInterfaceCode::SET_SESSIONMANAGERSERVICE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    HILOG_INFO("AbilityManagerProxy::SetSessionManagerService end.");
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::RegisterIAbilityManagerCollaborator(
    int32_t type, const sptr<IAbilityManagerCollaborator> &impl)
{
    if (!impl) {
        HILOG_ERROR("impl is nullptr");
        return ERR_INVALID_VALUE;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(type)) {
        HILOG_ERROR("type write failed.");
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(impl->AsObject())) {
        HILOG_ERROR("impl write failed.");
        return INNER_ERR;
    }

    auto ret = SendRequest(AbilityManagerInterfaceCode::REGISTER_COLLABORATOR, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::UnregisterIAbilityManagerCollaborator(int32_t type)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(type)) {
        HILOG_ERROR("type write failed.");
        return INNER_ERR;
    }

    auto ret = SendRequest(AbilityManagerInterfaceCode::UNREGISTER_COLLABORATOR, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::RegisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(callback)) {
        HILOG_ERROR("Callback write failed.");
        return INNER_ERR;
    }

    auto ret = SendRequest(AbilityManagerInterfaceCode::REGISTER_AUTO_STARTUP_SYSTEM_CALLBACK, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::UnregisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(callback)) {
        HILOG_ERROR("Callback write failed.");
        return INNER_ERR;
    }

    auto ret = SendRequest(AbilityManagerInterfaceCode::UNREGISTER_AUTO_STARTUP_SYSTEM_CALLBACK, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::SetApplicationAutoStartup(const AutoStartupInfo &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&info)) {
        HILOG_ERROR("Write AutoStartupInfo failed.");
        return INNER_ERR;
    }

    auto ret = SendRequest(AbilityManagerInterfaceCode::SET_APPLICATION_AUTO_STARTUP, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::CancelApplicationAutoStartup(const AutoStartupInfo &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&info)) {
        HILOG_ERROR("Write AutoStartupInfo failed.");
        return INNER_ERR;
    }

    auto ret = SendRequest(AbilityManagerInterfaceCode::CANCEL_APPLICATION_AUTO_STARTUP, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }

    auto ret = SendRequest(AbilityManagerInterfaceCode::QUERY_ALL_AUTO_STARTUP_APPLICATION, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d.", ret);
        return ret;
    }

    auto resultCode = reply.ReadInt32();
    if (resultCode != ERR_OK) {
        HILOG_ERROR("Reply error: %{public}d.", resultCode);
        return resultCode;
    }

    auto infoSize = reply.ReadInt32();
    for (auto i = 0; i < infoSize && i < MAX_AUTO_STARTUP_COUNT; ++i) {
        std::unique_ptr<AutoStartupInfo> info(reply.ReadParcelable<AutoStartupInfo>());
        if (!info) {
            HILOG_ERROR("Read Parcelable result infos failed.");
            return INNER_ERR;
        }
        infoList.emplace_back(*info);
    }
    return ERR_OK;
}

int AbilityManagerProxy::PrepareTerminateAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool &isPrepareTerminate)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }
    if (sessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(sessionInfo)) {
            HILOG_ERROR("flag and sessionInfo write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::PREPARE_TERMINATE_ABILITY_BY_SCB,
        data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }

    isPrepareTerminate = reply.ReadBool();
    return NO_ERROR;
}

int32_t AbilityManagerProxy::RegisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }

    if (listener == nullptr || !data.WriteRemoteObject(listener->AsObject())) {
        HILOG_ERROR("Write listener failed.");
        return INNER_ERR;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    int32_t error = SendRequest(AbilityManagerInterfaceCode::REGISTER_APP_DEBUG_LISTENER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request failed, err: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::UnregisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }

    if (listener == nullptr || !data.WriteRemoteObject(listener->AsObject())) {
        HILOG_ERROR("Write listener failed.");
        return INNER_ERR;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    int32_t error = SendRequest(AbilityManagerInterfaceCode::UNREGISTER_APP_DEBUG_LISTENER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request failed, err: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::AttachAppDebug(const std::string &bundleName)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }

    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("bundleName write failed.");
        return INNER_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = SendRequest(AbilityManagerInterfaceCode::ATTACH_APP_DEBUG, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request failed, err: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::DetachAppDebug(const std::string &bundleName)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }

    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("Write bundleName failed.");
        return INNER_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = SendRequest(AbilityManagerInterfaceCode::DETACH_APP_DEBUG, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request failed, err: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::ExecuteIntent(uint64_t key,  const sptr<IRemoteObject> &callerToken,
    const InsightIntentExecuteParam &param)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }

    if (!data.WriteUint64(key)) {
        HILOG_ERROR("Write key failed.");
        return INNER_ERR;
    }

    if (!data.WriteRemoteObject(callerToken)) {
        HILOG_ERROR("failed to write callerToken.");
        return INNER_ERR;
    }

    if (!data.WriteParcelable(&param)) {
        HILOG_ERROR("Write param failed.");
        return INNER_ERR;
    }

    int32_t error = SendRequest(AbilityManagerInterfaceCode::EXECUTE_INTENT, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request failed, err: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

bool AbilityManagerProxy::IsAbilityControllerStart(const Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return true;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("WriteWantObject failed.");
        return true;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::IS_ABILITY_CONTROLLER_START,
        data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return true;
    }
    return reply.ReadBool();
}

int32_t AbilityManagerProxy::ExecuteInsightIntentDone(const sptr<IRemoteObject> &token, uint64_t intentId,
    const InsightIntentExecuteResult &result)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write remote object failed.");
        return INNER_ERR;
    }

    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("Write token failed.");
        return INNER_ERR;
    }

    if (!data.WriteInt64(intentId) || !data.WriteParcelable(&result)) {
        HILOG_ERROR("Write insight intent params failed.");
        return INNER_ERR;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto ret = SendRequest(AbilityManagerInterfaceCode::EXECUTE_INSIGHT_INTENT_DONE, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request failed with %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    auto error = SendRequest(AbilityManagerInterfaceCode::GET_FOREGROUND_UI_ABILITIES, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request failed, error: %{public}d.", error);
        return error;
    }

    auto errorCode = GetParcelableInfos<AppExecFwk::AbilityStateData>(reply, list);
    if (errorCode != NO_ERROR) {
        HILOG_ERROR("Get foreground ui abilities error: %{public}d.", errorCode);
        return errorCode;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::OpenFile(const Uri& uri, uint32_t flag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return false;
    }
    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("Write uri failed.");
        return false;
    }
    if (!data.WriteInt32(flag)) {
        HILOG_ERROR("Write flag failed.");
        return false;
    }

    auto ret = SendRequest(AbilityManagerInterfaceCode::OPEN_FILE, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request failed with %{public}d", ret);
        return ret;
    }
    return reply.ReadFileDescriptor();
}

void AbilityManagerProxy::UpdateSessionInfoBySCB(const std::vector<SessionInfo> &sessionInfos, int32_t userId)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }
    auto size = static_cast<int32_t>(sessionInfos.size());
    int32_t threshold = 512;
    if (size > threshold) {
        HILOG_ERROR("Size of vector too large.");
        return;
    }
    if (!data.WriteInt32(size)) {
        HILOG_ERROR("Write size failed.");
        return;
    }
    for (int32_t i = 0; i < size; i++) {
        if (!data.WriteParcelable(&sessionInfos[i])) {
            HILOG_ERROR("Write sessionInfo failed.");
            return;
        }
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("Write userId failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = SendRequest(AbilityManagerInterfaceCode::UPDATE_SESSION_INFO, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request failed with %{public}d", ret);
    }
}

ErrCode AbilityManagerProxy::SendRequest(AbilityManagerInterfaceCode code, MessageParcel &data, MessageParcel &reply,
    MessageOption& option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return INNER_ERR;
    }

    return remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
}

int32_t AbilityManagerProxy::SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&info)) {
        HILOG_ERROR("Write AutoStartupInfo failed.");
        return INNER_ERR;
    }
    if (!data.WriteBool(flag)) {
        HILOG_ERROR("Write flag failed.");
        return INNER_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = SendRequest(AbilityManagerInterfaceCode::SET_APPLICATION_AUTO_STARTUP_BY_EDM, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&info)) {
        HILOG_ERROR("Write AutoStartupInfo failed.");
        return INNER_ERR;
    }
    if (!data.WriteBool(flag)) {
        HILOG_ERROR("Write flag failed.");
        return INNER_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = SendRequest(AbilityManagerInterfaceCode::CANCEL_APPLICATION_AUTO_STARTUP_BY_EDM, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token,
    UIExtensionHostInfo &hostInfo, int32_t userId)
{
    if (token == nullptr) {
        HILOG_ERROR("Input param invalid.");
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    if (!WriteInterfaceToken (data)) {
        HILOG_ERROR("Write remote object failed.");
        return INNER_ERR;
    }

    if (!data.WriteBool(true) || !data.WriteRemoteObject(token)) {
        HILOG_ERROR("Write flag and token failed.");
        return INNER_ERR;
    }

    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("Write userId failed.");
        return INNER_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    auto error = SendRequest(AbilityManagerInterfaceCode::GET_UI_EXTENSION_ROOT_HOST_INFO, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }

    std::unique_ptr<UIExtensionHostInfo> info(reply.ReadParcelable<UIExtensionHostInfo>());
    if (info == nullptr) {
        HILOG_ERROR("Get host info failed.");
        return INNER_ERR;
    }
    hostInfo = *info;
    return reply.ReadInt32();
}

int32_t AbilityManagerProxy::RestartApp(const AAFwk::Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return IPC_PROXY_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return IPC_PROXY_ERR;
    }
    auto ret = SendRequest(AbilityManagerInterfaceCode::RESTART_APP, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request is failed, error code: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

AppExecFwk::ElementName AbilityManagerProxy::GetElementNameByAppId(const std::string &appId)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return {};
    }
    if (!data.WriteString(appId)) {
        HILOG_ERROR("Write appId failed.");
        return {};
    }
    MessageParcel reply;
    MessageOption option;
    auto ret = SendRequest(AbilityManagerInterfaceCode::GET_ELEMENT_NAME_BY_APP_ID, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d.", ret);
        return {};
    }
    std::unique_ptr<AppExecFwk::ElementName> elementName(reply.ReadParcelable<AppExecFwk::ElementName>());
    if (elementName == nullptr) {
        HILOG_ERROR("elementName is nullptr");
        return {};
    }
    return *elementName;
}

int32_t AbilityManagerProxy::OpenAtomicService(Want& want, sptr<IRemoteObject> callerToken, int32_t requestCode,
    int32_t userId)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("Write want failed.");
        return INNER_ERR;
    }
    if (callerToken != nullptr) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
            HILOG_ERROR("flag and callerToken write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteInt32(requestCode)) {
        HILOG_ERROR("requestCode write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("userId write failed.");
        return INNER_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = SendRequest(AbilityManagerInterfaceCode::OPEN_ATOMIC_SERVICE, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

bool AbilityManagerProxy::IsEmbeddedOpenAllowed(sptr<IRemoteObject> callerToken, const std::string &appId)
{
    if (callerToken == nullptr) {
        HILOG_ERROR("Input param invalid.");
        return false;
    }

    MessageParcel data;
    if (!WriteInterfaceToken (data)) {
        HILOG_ERROR("Write remote object failed.");
        return false;
    }

    if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
        HILOG_ERROR("Write flag and callerToken failed.");
        return false;
    }

    if (!data.WriteString(appId)) {
        HILOG_ERROR("Write userId failed.");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    auto error = SendRequest(AbilityManagerInterfaceCode::IS_EMBEDDED_OPEN_ALLOWED, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}
} // namespace AAFwk
} // namespace OHOS
