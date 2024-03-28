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

#include "ability_debug_response_stub.h"

#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t CYCLE_LIMIT_MIN = 0;
constexpr int32_t CYCLE_LIMIT_MAX = 1000;
}
AbilityDebugResponseStub::AbilityDebugResponseStub()
{
    responseFuncMap_[static_cast<uint32_t>(IAbilityDebugResponse::Message::ON_ABILITYS_DEBUG_STARTED)] =
        &AbilityDebugResponseStub::HandleOnAbilitysDebugStarted;
    responseFuncMap_[static_cast<uint32_t>(IAbilityDebugResponse::Message::ON_ABILITYS_DEBUG_STOPED)] =
        &AbilityDebugResponseStub::HandleOnAbilitysDebugStoped;
}

AbilityDebugResponseStub::~AbilityDebugResponseStub()
{
    responseFuncMap_.clear();
}

int32_t AbilityDebugResponseStub::HandleOnAbilitysDebugStarted(MessageParcel &data, MessageParcel &reply)
{
    auto tokenSize = data.ReadInt32();
    if (tokenSize <= CYCLE_LIMIT_MIN || tokenSize > CYCLE_LIMIT_MAX) {
        TAG_LOGE(AAFwkTag::APPMGR, "Token size exceeds limit.");
        return ERR_INVALID_DATA;
    }

    std::vector<sptr<IRemoteObject>> tokens;
    for (int32_t index = 0; index < tokenSize; index++) {
        auto token = data.ReadRemoteObject();
        if (token == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "Token is nullptr.");
            return ERR_INVALID_DATA;
        }
        tokens.push_back(token);
    }
    OnAbilitysDebugStarted(tokens);
    return NO_ERROR;
}

int32_t AbilityDebugResponseStub::HandleOnAbilitysDebugStoped(MessageParcel &data, MessageParcel &reply)
{
    auto tokenSize = data.ReadInt32();
    if (tokenSize <= CYCLE_LIMIT_MIN || tokenSize > CYCLE_LIMIT_MAX) {
        TAG_LOGE(AAFwkTag::APPMGR, "Token size exceeds limit.");
        return ERR_INVALID_DATA;
    }

    std::vector<sptr<IRemoteObject>> tokens;
    for (int32_t index = 0; index < tokenSize; index++) {
        auto token = data.ReadRemoteObject();
        if (token == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "Token is nullptr.");
            return ERR_INVALID_DATA;
        }
        tokens.push_back(token);
    }
    OnAbilitysDebugStoped(tokens);
    return NO_ERROR;
}

int32_t AbilityDebugResponseStub::HandleOnAbilitysAssertDebugChange(MessageParcel &data, MessageParcel &reply)
{
    auto tokenSize = data.ReadInt32();
    if (tokenSize <= CYCLE_LIMIT_MIN || tokenSize > CYCLE_LIMIT_MAX) {
        TAG_LOGE(AAFwkTag::APPMGR, "Token size exceeds limit.");
        return ERR_INVALID_DATA;
    }

    std::vector<sptr<IRemoteObject>> tokens;
    for (int32_t index = 0; index < tokenSize; index++) {
        auto token = data.ReadRemoteObject();
        if (token == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "Token is nullptr.");
            return ERR_INVALID_DATA;
        }
        tokens.push_back(token);
    }
    auto isAssertDebug = data.ReadBool();
    OnAbilitysAssertDebugChange(tokens, isAssertDebug);
    return NO_ERROR;
}

int AbilityDebugResponseStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::APPMGR, "code = %{public}u, flags= %{public}d", code, option.GetFlags());
    std::u16string descriptor = AbilityDebugResponseStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "Local descriptor is not equal to remote.");
        return ERR_INVALID_STATE;
    }

    auto itFunc = responseFuncMap_.find(code);
    if (itFunc != responseFuncMap_.end()) {
        auto responseFunc = itFunc->second;
        if (responseFunc != nullptr) {
            return (this->*responseFunc)(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
} // namespace AppExecFwk
} // namespace OHOS
