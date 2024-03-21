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

#include "hilog_wrapper.h"
#include "uri_permission_load_callback.h"
#include "uri_permission_manager_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
void UriPermissionLoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    if (systemAbilityId != URI_PERMISSION_MGR_SERVICE_ID) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "System ability id %{public}d mismatch.", systemAbilityId);
        return;
    }

    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Object is nullptr.");
        return;
    }

    TAG_LOGD(AAFwkTag::URIPERMMGR, "Load system ability %{public}d succeed.", systemAbilityId);
    UriPermissionManagerClient::GetInstance().OnLoadSystemAbilitySuccess(remoteObject);
}

void UriPermissionLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    if (systemAbilityId != URI_PERMISSION_MGR_SERVICE_ID) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "System ability id %{public}d mismatch.", systemAbilityId);
        return;
    }

    TAG_LOGD(AAFwkTag::URIPERMMGR, "Load system ability %{public}d failed.", systemAbilityId);
    UriPermissionManagerClient::GetInstance().OnLoadSystemAbilityFail();
}
}  // namespace AAFwk
}  // namespace OHOS