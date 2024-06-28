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


#include "ability_background_connection.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
void AbilityBackgroundConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    if (resultCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s, abilityName:%{public}s, resultCode:%{public}d",
            __func__, element.GetAbilityName().c_str(), resultCode);
        return;
    }
}

void AbilityBackgroundConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OnAbilityDisconnectDone");
}
}  // namespace AppExecFwk
}  // namespace OHOS
