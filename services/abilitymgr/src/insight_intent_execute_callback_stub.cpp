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

#include "insight_intent_execute_callback_stub.h"
#include "insight_intent_host_client.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {

InsightIntentExecuteCallbackStub::InsightIntentExecuteCallbackStub()
{
    requestFuncMap_[ON_INSIGHT_INTENT_EXECUTE_DONE] = &InsightIntentExecuteCallbackStub::OnExecuteDoneInner;
}

InsightIntentExecuteCallbackStub::~InsightIntentExecuteCallbackStub()
{
    TAG_LOGD(AAFwkTag::INTENT, "call");
    requestFuncMap_.clear();
}

int32_t InsightIntentExecuteCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != IInsightIntentExecuteCallback::GetDescriptor()) {
        TAG_LOGE(AAFwkTag::INTENT, "InterfaceToken not equal IInsightIntentExecuteCallback's descriptor.");
        return ERR_INVALID_STATE;
    }

    auto itFunc = requestFuncMap_.find(code);
    if (itFunc != requestFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }
    TAG_LOGW(AAFwkTag::INTENT, "default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t InsightIntentExecuteCallbackStub::OnExecuteDoneInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::INTENT, "call");
    uint64_t key = data.ReadUint64();
    int32_t resultCode = data.ReadInt32();
    std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> executeResult(
        data.ReadParcelable<AppExecFwk::InsightIntentExecuteResult>());
    if (executeResult == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "executeResult is nullptr");
        return ERR_INVALID_VALUE;
    }
    OnExecuteDone(key, resultCode, *executeResult);
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS
