/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "hilog_wrapper.h"
#include "quick_fix_load_callback.h"
#include "quick_fix_manager_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
void QuickFixLoadCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    if (systemAbilityId != QUICK_FIX_MGR_SERVICE_ID) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "System ability id %{public}d mismatch.", systemAbilityId);
        return;
    }

    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Object is nullptr.");
        return;
    }

    TAG_LOGD(AAFwkTag::QUICKFIX, "Load system ability %{public}d succeed.", systemAbilityId);
    QuickFixManagerClient::GetInstance()->OnLoadSystemAbilitySuccess(remoteObject);
}

void QuickFixLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    if (systemAbilityId != QUICK_FIX_MGR_SERVICE_ID) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "System ability id %{public}d mismatch.", systemAbilityId);
        return;
    }

    TAG_LOGD(AAFwkTag::QUICKFIX, "Load system ability %{public}d failed.", systemAbilityId);
    QuickFixManagerClient::GetInstance()->OnLoadSystemAbilityFail();
}
}  // namespace AAFwk
}  // namespace OHOS
