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

#include "reverse_continuation_scheduler_primary_stub.h"
#include "ability_scheduler_interface.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
const std::string ReverseContinuationSchedulerPrimaryStub::DESCRIPTOR(
    "ohos.abilityshell.ReverseContinuationSchedulerMaster");

ReverseContinuationSchedulerPrimaryStub::ReverseContinuationSchedulerPrimaryStub()
{
    requestFuncMap_[NOTIFY_REPLICA_TERMINATED] = &ReverseContinuationSchedulerPrimaryStub::NotifyReplicaTerminatedInner;
    requestFuncMap_[CONTINUATION_BACK] = &ReverseContinuationSchedulerPrimaryStub::ContinuationBackInner;
}

ReverseContinuationSchedulerPrimaryStub::~ReverseContinuationSchedulerPrimaryStub()
{
    requestFuncMap_.clear();
}

/**
 * @brief Sets an entry for receiving requests.
 *
 * @param code Indicates the service request code sent from the peer end.
 * @param data Indicates the MessageParcel object sent from the peer end.
 * @param reply Indicates the response message object sent from the remote service. The local service writes the
 * response data to the MessageParcel object.
 * @param option Indicates whether the operation is synchronous or asynchronous.
 * @return ERR_NONE if success, otherwise false.
 */
int ReverseContinuationSchedulerPrimaryStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    std::u16string token = data.ReadInterfaceToken();
    std::u16string descriptor = Str8ToStr16(DESCRIPTOR);
    if (descriptor != token) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ReverseContinuationSchedulerPrimaryStub::OnRemoteRequest failed, DESCRIPTOR != touken");
        return -1;
    }

    auto iter = requestFuncMap_.find(code);
    if (iter != requestFuncMap_.end()) {
        auto func = iter->second;
        if (func != nullptr) {
            return (this->*func)(data, reply);
        } else {
            TAG_LOGW(AAFwkTag::CONTINUATION,
                "ReverseContinuationSchedulerPrimaryStub::OnRemoteRequest failed, func is nullptr");
        }
    } else {
        TAG_LOGW(AAFwkTag::CONTINUATION,
            "ReverseContinuationSchedulerPrimaryStub::OnRemoteRequest failed, iter not find");
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int ReverseContinuationSchedulerPrimaryStub::NotifyReplicaTerminatedInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    NotifyReplicaTerminated();
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return 0;
}
int ReverseContinuationSchedulerPrimaryStub::ContinuationBackInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ReverseContinuationSchedulerPrimaryStub::ContinuationBackInner want is nullptr");
        return -1;
    }

    if (!ContinuationBack(*want)) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ReverseContinuationSchedulerPrimaryStub::NotifyReverseaTerminatedInner failed, ContinuationBack() "
                 "return false");
        return -1;
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return 0;
}
}  // namespace AppExecFwk
}  // namespace OHOS
