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

#include "static_subscriber_extension_context.h"

#include "ability_manager_client.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t StaticSubscriberExtensionContext::CONTEXT_TYPE_ID(
    std::hash<const char*> {} ("StaticSubscriberExtensionContext"));
int StaticSubscriberExtensionContext::ILLEGAL_REQUEST_CODE(-1);

StaticSubscriberExtensionContext::StaticSubscriberExtensionContext() {}

StaticSubscriberExtensionContext::~StaticSubscriberExtensionContext() {}

ErrCode StaticSubscriberExtensionContext::StartAbility(const AAFwk::Want &want) const
{
    HILOG_DEBUG("Start ability begin, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        HILOG_ERROR("StaticSubscriberExtensionContext::StartAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode StaticSubscriberExtensionContext::StartAbility(const AAFwk::Want &want,
    const AAFwk::StartOptions &startOptions) const
{
    HILOG_DEBUG("Start ability begin, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_,
        ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        HILOG_ERROR("StaticSubscriberExtensionContext::StartAbility is failed %{public}d", err);
    }
    return err;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
