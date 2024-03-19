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

#include "ability_debug_response_proxy.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t CYCLE_LIMIT_MIN = 0;
constexpr int32_t CYCLE_LIMIT_MAX = 1000;
}
AbilityDebugResponseProxy::AbilityDebugResponseProxy(
    const sptr<IRemoteObject> &impl) : IRemoteProxy<IAbilityDebugResponse>(impl)
{}

bool AbilityDebugResponseProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AbilityDebugResponseProxy::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return false;
    }
    return true;
}

void AbilityDebugResponseProxy::OnAbilitysDebugStarted(const std::vector<sptr<IRemoteObject>> &tokens)
{
    HILOG_DEBUG("Called.");
    SendRequest(IAbilityDebugResponse::Message::ON_ABILITYS_DEBUG_STARTED, tokens);
}

void AbilityDebugResponseProxy::OnAbilitysDebugStoped(const std::vector<sptr<IRemoteObject>> &tokens)
{
    HILOG_DEBUG("Called.");
    SendRequest(IAbilityDebugResponse::Message::ON_ABILITYS_DEBUG_STOPED, tokens);
}

void AbilityDebugResponseProxy::OnAbilitysAssertDebugChange(
    const std::vector<sptr<IRemoteObject>> &tokens, bool isAssertDebug)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (tokens.size() <= CYCLE_LIMIT_MIN || tokens.size() > CYCLE_LIMIT_MAX ||
        !data.WriteInt32(tokens.size())) {
        HILOG_ERROR("Write data size failed.");
        return;
    }

    for (const auto &item : tokens) {
        if (!data.WriteRemoteObject(item)) {
            HILOG_ERROR("Write token failed.");
            return;
        }
    }

    if (!data.WriteBool(isAssertDebug)) {
        HILOG_ERROR("Write flag failed.");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote is nullptr.");
        return;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = remote->SendRequest(static_cast<uint32_t>(Message::ON_ABILITYS_ASSERT_DEBUG), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest is failed, error code: %{public}d", ret);
    }
}

void AbilityDebugResponseProxy::SendRequest(
    const IAbilityDebugResponse::Message &message, const std::vector<sptr<IRemoteObject>> &tokens)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (tokens.size() <= CYCLE_LIMIT_MIN || tokens.size() > CYCLE_LIMIT_MAX ||
        !data.WriteInt32(tokens.size())) {
        HILOG_ERROR("Write data size failed.");
        return;
    }

    for (auto iter = tokens.begin(); iter != tokens.end(); iter++) {
        if (!data.WriteRemoteObject(iter->GetRefPtr())) {
            HILOG_ERROR("Write token failed.");
            return;
        }
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote is nullptr.");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = remote->SendRequest(static_cast<uint32_t>(message), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest is failed, error code: %{public}d", ret);
    }
}
} // namespace AppExecFwk
} // namespace OHOS
