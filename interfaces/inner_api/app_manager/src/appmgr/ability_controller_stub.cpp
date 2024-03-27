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

#include "ability_controller_stub.h"
#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
AbilityControllerStub::AbilityControllerStub()
{
    memberFuncMap_[static_cast<uint32_t>(
        IAbilityController::Message::TRANSACT_ON_ALLOW_ABILITY_START)] =
        &AbilityControllerStub::HandleAllowAbilityStart;
    memberFuncMap_[static_cast<uint32_t>(
        IAbilityController::Message::TRANSACT_ON_ALLOW_ABILITY_BACKGROUND)] =
        &AbilityControllerStub::HandleAllowAbilityBackground;
}

AbilityControllerStub::~AbilityControllerStub()
{
    memberFuncMap_.clear();
}

int AbilityControllerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGI(AAFwkTag::APPMGR, "AbilityControllerStub::OnReceived, code = %{public}u, flags= %{public}d.", code,
        option.GetFlags());
    std::u16string descriptor = AbilityControllerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "local descriptor is unequal to remote.");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    TAG_LOGI(AAFwkTag::APPMGR, "AbilityControllerStub::OnRemoteRequest finish");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

bool AbilityControllerStub::AllowAbilityStart(const Want &want, const std::string &bundleName)
{
    return true;
}

bool AbilityControllerStub::AllowAbilityBackground(const std::string &bundleName)
{
    return true;
}

int32_t AbilityControllerStub::HandleAllowAbilityStart(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::APPMGR, "HandleAllowAbilityStart");
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (!want) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<Want> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    std::string pkg = data.ReadString();
    bool ret = AllowAbilityStart(*want, pkg);
    reply.WriteBool(ret);
    return NO_ERROR;
}

int32_t AbilityControllerStub::HandleAllowAbilityBackground(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::APPMGR, "HandleAllowAbilityBackground");
    std::string pkg = data.ReadString();
    bool ret = AllowAbilityBackground(pkg);
    reply.WriteBool(ret);
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
