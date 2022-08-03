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

#include "quick_fix_manager_stub.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
QuickFixManagerStub::QuickFixManagerStub()
{
    requestFuncMap_[ON_APPLY_QUICK_FIX] = &QuickFixManagerStub::ApplyQuickFixInner;
}

QuickFixManagerStub::~QuickFixManagerStub()
{
    requestFuncMap_.clear();
}

int QuickFixManagerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != IQuickFixManager::GetDescriptor()) {
        HILOG_ERROR("local descriptor is not equal to remote.");
        return ERR_INVALID_VALUE;
    }

    auto itFunc = requestFuncMap_.find(code);
    if (itFunc != requestFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }

    HILOG_WARN("default case, need check value of code.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

void QuickFixManagerStub::ApplyQuickFixInner(const std::vector<std::string> &quickFixFiles)
{
    return;
}
} // namespace AAFwk
} // namespace OHOS
