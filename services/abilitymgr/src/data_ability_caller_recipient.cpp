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

#include "data_ability_caller_recipient.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
void DataAbilityCallerRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGE(AAFwkTag::DATA_ABILITY, "recv death notice");

    if (handler_) {
        handler_(remote);
    }
}

DataAbilityCallerRecipient::DataAbilityCallerRecipient(RemoteDiedHandler handler) : handler_(handler)
{
    TAG_LOGE(AAFwkTag::DATA_ABILITY, "called");
}

DataAbilityCallerRecipient::~DataAbilityCallerRecipient()
{
    TAG_LOGE(AAFwkTag::DATA_ABILITY, "called");
}
}  // namespace AAFwk
}  // namespace OHOS
