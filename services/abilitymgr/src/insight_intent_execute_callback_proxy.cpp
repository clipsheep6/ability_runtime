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

#include "ability_manager_errors.h"
#include "insight_intent_execute_callback_proxy.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "iremote_object.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
void InsightIntentExecuteCallbackProxy::OnExecuteDone(uint64_t key, int32_t resultCode,
    const AppExecFwk::InsightIntentExecuteResult &executeResult)
{
    TAG_LOGD(AAFwkTag::INTENT, "call");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IInsightIntentExecuteCallback::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::INTENT, "Write interface token failed.");
        return;
    }
    if (!data.WriteUint64(key)) {
        TAG_LOGE(AAFwkTag::INTENT, "key write failed.");
        return;
    }
    if (!data.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::INTENT, "resultCode write failed.");
        return;
    }
    if (!data.WriteParcelable(&executeResult)) {
        TAG_LOGE(AAFwkTag::INTENT, "executeResult write failed.");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "Remote() is NULL");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int error = remote->SendRequest(ON_INSIGHT_INTENT_EXECUTE_DONE, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "SendRequest fail, error: %{public}d", error);
    }
}
} // namespace AAFwk
} // namespace OHOS
