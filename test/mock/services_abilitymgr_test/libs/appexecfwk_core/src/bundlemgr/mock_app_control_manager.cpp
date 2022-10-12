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

#include "mock_app_control_manager.h"

namespace OHOS {
namespace AppExecFwk {
AppControlProxy::AppControlProxy(const sptr<IRemoteObject> &object) : IRemoteProxy<IAppControlMgr>(object)
{
}

AppControlProxy::~AppControlProxy()
{
}

ErrCode AppControlProxy::AddAppInstallControlRule(const std::vector<std::string> &appIds,
    const AppInstallControlRuleType controlRuleType, int32_t userId)
{
    return ERR_OK;
}

ErrCode AppControlProxy::DeleteAppInstallControlRule(const AppInstallControlRuleType controlRuleType,
    const std::vector<std::string> &appIds, int32_t userId)
{
    return ERR_OK;
}

ErrCode AppControlProxy::DeleteAppInstallControlRule(
    const AppInstallControlRuleType controlRuleType, int32_t userId)
{
    return ERR_OK;
}

ErrCode AppControlProxy::GetAppInstallControlRule(
    const AppInstallControlRuleType controlRuleType, int32_t userId, std::vector<std::string> &appIds)
{
    return ERR_OK;
}

ErrCode AppControlProxy::AddAppRunningControlRule(
    const std::vector<AppRunningControlRule> &controlRules, int32_t userId)
{
    return ERR_OK;
}

ErrCode AppControlProxy::DeleteAppRunningControlRule(
    const std::vector<AppRunningControlRule> &controlRules, int32_t userId)
{
    return ERR_OK;
}

ErrCode AppControlProxy::DeleteAppRunningControlRule(int32_t userId)
{
    return ERR_OK;
}

ErrCode AppControlProxy::GetAppRunningControlRule(int32_t userId, std::vector<std::string> &appIds)
{
    return ERR_OK;
}

ErrCode AppControlProxy::GetAppRunningControlRule(
    const std::string &bundleName, int32_t userId, AppRunningControlRuleResult &controlRuleResult)
{
    Want want;
    ElementName element("", "com.huawei.hmos.appgallery", "MainAbility");
    want.SetElement(element);
    controlRuleResult.controlWant = std::make_shared<Want>(want);
    controlRuleResult.controlMessage = "the test app is not available";
    return ERR_OK;
}

ErrCode AppControlProxy::SetDisposedStatus(const std::string &appId, const Want &want)
{
    return ERR_OK;
}

ErrCode AppControlProxy::DeleteDisposedStatus(const std::string &appId)
{
    return ERR_OK;
}

ErrCode AppControlProxy::GetDisposedStatus(const std::string &appId, Want &want)
{
    return ERR_OK;
}
} // AppExecFwk
} // OHOS
