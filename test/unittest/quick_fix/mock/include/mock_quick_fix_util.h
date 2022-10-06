/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_QUICK_FIX_UTIL_H
#define OHOS_ABILITY_RUNTIME_MOCK_QUICK_FIX_UTIL_H

#include "gmock/gmock.h"
#include "app_mgr_interface.h"
#include "bundlemgr/bundle_mgr_interface.h"

namespace OHOS {
namespace AAFwk {
class QuickFixUtil {
public:
    static sptr<IRemoteObject> GetRemoteObjectOfSystemAbility(const int32_t systemAbilityId);
    static sptr<AppExecFwk::IAppMgr> GetAppManagerProxy();
    static sptr<AppExecFwk::IBundleMgr> GetBundleManagerProxy();
    static sptr<AppExecFwk::IQuickFixManager> GetBundleQuickFixMgrProxy();

    void RegisterSystemAbility(const int32_t systemAbilityId, sptr<IRemoteObject> object);

private:
    static std::mutex saMutex_;
    static std::unordered_map<int32_t, sptr<IRemoteObject>> servicesMap_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MOCK_QUICK_FIX_UTIL_H
