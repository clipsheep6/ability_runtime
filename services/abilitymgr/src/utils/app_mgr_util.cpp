/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "utils/app_mgr_util.h"

#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
sptr<OHOS::AppExecFwk::IAppMgr> AppMgrUtil::appMgr_ = nullptr;

OHOS::sptr<OHOS::AppExecFwk::IAppMgr> AppMgrUtil::GetAppMgr()
{
    if (appMgr_) {
        return appMgr_;
    }

    auto sysMgrClient = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (sysMgrClient == nullptr) {
        return nullptr;
    }
    auto object = sysMgrClient->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (object == nullptr) {
        return nullptr;
    }
    appMgr_ = OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(object);
    return appMgr_;
}
}  // namespace AAFwk
}  // namespace OHOS
