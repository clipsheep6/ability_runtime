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

#include "app_debug_listener_stub.h"

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
AppDebugListenerStub::AppDebugListenerStub()
{
    memberFuncMap_[static_cast<uint32_t>(IAppDebugListener::Message::ON_APP_DEBUG_STARTED)] =
        &AppDebugListenerStub::HandleOnAppDebugStarted;
    memberFuncMap_[static_cast<uint32_t>(IAppDebugListener::Message::ON_APP_DEBUG_STOPED)] =
        &AppDebugListenerStub::HandleOnAppDebugStoped;
}

AppDebugListenerStub::~AppDebugListenerStub()
{
    memberFuncMap_.clear();
}

int AppDebugListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::APPMGR, "code = %{public}u, flags= %{public}d", code, option.GetFlags());
    std::u16string descriptor = AppDebugListenerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "Local descriptor is not equal to remote.");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    TAG_LOGD(AAFwkTag::APPMGR, "AppDebugListenerStub::OnRemoteRequest end");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t AppDebugListenerStub::HandleOnAppDebugStarted(MessageParcel &data, MessageParcel &reply)
{
    auto infoSize = data.ReadInt32();
    if (infoSize <= CYCLE_LIMIT_MIN || infoSize > CYCLE_LIMIT_MAX) {
        TAG_LOGE(AAFwkTag::APPMGR, "Token size exceeds limit.");
        return ERR_INVALID_DATA;
    }

    std::vector<AppDebugInfo> appDebugInfos;
    for (int32_t index = 0; index < infoSize; index++) {
        std::unique_ptr<AppDebugInfo> appDebugInfo(data.ReadParcelable<AppDebugInfo>());
        if (appDebugInfo == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "Read app debug infos failed.");
            return ERR_INVALID_DATA;
        }
        appDebugInfos.emplace_back(*appDebugInfo);
    }

    OnAppDebugStarted(appDebugInfos);
    return NO_ERROR;
}

int32_t AppDebugListenerStub::HandleOnAppDebugStoped(MessageParcel &data, MessageParcel &reply)
{
    auto infoSize = data.ReadInt32();
    if (infoSize <= CYCLE_LIMIT_MIN || infoSize > CYCLE_LIMIT_MAX) {
        TAG_LOGE(AAFwkTag::APPMGR, "Token size exceeds limit.");
        return ERR_INVALID_DATA;
    }

    std::vector<AppDebugInfo> appDebugInfos;
    for (int32_t index = 0; index < infoSize; index++) {
        std::unique_ptr<AppDebugInfo> appDebugInfo(data.ReadParcelable<AppDebugInfo>());
        if (appDebugInfo == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "Read app debug infos failed.");
            return ERR_INVALID_DATA;
        }
        appDebugInfos.emplace_back(*appDebugInfo);
    }

    OnAppDebugStoped(appDebugInfos);
    return NO_ERROR;
}
} // namespace AppExecFwk
} // namespace OHOS
