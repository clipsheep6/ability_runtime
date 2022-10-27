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
#include "hitrace_meter.h"
#include "quick_fix_error_utils.h"

namespace OHOS {
namespace AAFwk {
QuickFixManagerStub::QuickFixManagerStub()
{
    requestFuncMap_[ON_APPLY_QUICK_FIX] = &QuickFixManagerStub::ApplyQuickFixInner;
    requestFuncMap_[ON_GET_APPLYED_QUICK_FIX_INFO] = &QuickFixManagerStub::GetApplyedQuickFixInfoInner;
}

QuickFixManagerStub::~QuickFixManagerStub()
{
    requestFuncMap_.clear();
}

int QuickFixManagerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != AAFwk::IQuickFixManager::GetDescriptor()) {
        HILOG_ERROR("local descriptor is not equal to remote.");
        return QUICK_FIX_INVALID_PARAM;
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

int32_t QuickFixManagerStub::ApplyQuickFixInner(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::vector<std::string> hapQuickFixFiles;
    if (!data.ReadStringVector(&hapQuickFixFiles)) {
        HILOG_ERROR("Read quick fix files failed.");
        return QUICK_FIX_READ_PARCEL_FAILED;
    }

    auto ret = ApplyQuickFix(hapQuickFixFiles);
    reply.WriteInt32(ret);
    return QUICK_FIX_OK;
}

int32_t QuickFixManagerStub::GetApplyedQuickFixInfoInner(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string bundleName = data.ReadString();
    ApplicationQuickFixInfo quickFixInfo;
    auto ret = GetApplyedQuickFixInfo(bundleName, quickFixInfo);
    reply.WriteInt32(ret);
    if (ret == QUICK_FIX_OK) {
        if (!reply.WriteParcelable(&quickFixInfo)) {
            HILOG_ERROR("Write parcelable failed.");
            return QUICK_FIX_WRITE_PARCEL_FAILED;
        }
    }
    return QUICK_FIX_OK;
}
} // namespace AAFwk
} // namespace OHOS
