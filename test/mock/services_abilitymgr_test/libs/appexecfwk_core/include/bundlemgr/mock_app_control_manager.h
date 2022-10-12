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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_CONTROL_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_CONTROL_MANAGER_H

#include "want.h"
#include <gmock/gmock.h>
#include "foundation/bundlemanager/bundle_framework/interfaces/inner_api/appexecfwk_core/include/app_control/app_control_interface.h"
#include "app_running_control_rule_result.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {
class AppControlProxy : public IRemoteProxy<IAppControlMgr> {
public:
    using Want = OHOS::AAFwk::Want;

    explicit AppControlProxy(const sptr<IRemoteObject>& object);
    virtual ~AppControlProxy();

    // for app install control rule
    virtual ErrCode AddAppInstallControlRule(const std::vector<std::string> &appIds,
        const AppInstallControlRuleType controlRuleType, int32_t userId) override;
    virtual ErrCode DeleteAppInstallControlRule(const AppInstallControlRuleType controlRuleType,
        const std::vector<std::string> &appIds, int32_t userId) override;
    virtual ErrCode DeleteAppInstallControlRule(
        const AppInstallControlRuleType controlRuleType, int32_t userId) override;
    virtual ErrCode GetAppInstallControlRule(
        const AppInstallControlRuleType controlRuleType, int32_t userId, std::vector<std::string> &appIds) override;
    // for app running control rule
    virtual ErrCode AddAppRunningControlRule(
        const std::vector<AppRunningControlRule> &controlRule, int32_t userId) override;
    virtual ErrCode DeleteAppRunningControlRule(
        const std::vector<AppRunningControlRule> &controlRule, int32_t userId) override;
    virtual ErrCode DeleteAppRunningControlRule(int32_t userId) override;
    virtual ErrCode GetAppRunningControlRule(int32_t userId, std::vector<std::string> &appIds) override;
    virtual ErrCode GetAppRunningControlRule(
        const std::string &bundleName, int32_t userId, AppRunningControlRuleResult &controlRuleResult) override;

    virtual ErrCode SetDisposedStatus(const std::string &appId, const Want &want) override;
    virtual ErrCode DeleteDisposedStatus(const std::string &appId) override;
    virtual ErrCode GetDisposedStatus(const std::string &appId, Want &want) override;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_CONTROL_MANAGER_H