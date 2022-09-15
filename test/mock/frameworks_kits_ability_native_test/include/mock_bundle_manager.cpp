/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "mock_bundle_manager.h"
#include <gtest/gtest.h>
#include "ability_info.h"
#include "application_info.h"

namespace OHOS {
namespace AppExecFwk {
const long int UNEXPIRED_TIME = 1860000000;
bool BundleMgrProxy::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
{
    if (appName.empty()) {
        return false;
    }
    appInfo.name = "Helloworld";
    appInfo.bundleName = "com.ohos.hiworld";
    return true;
}

std::string BundleMgrProxy::GetAppType(const std::string &bundleName)
{
    GTEST_LOG_(INFO) << " BundleMgrProxy::GetAppTyp";
    return "system";
}

bool BundleMgrProxy::QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo)
{
    ElementName eleName = want.GetElement();
    if (eleName.GetBundleName().empty()) {
        return false;
    }
    abilityInfo.visible = true;
    abilityInfo.name = eleName.GetAbilityName();
    abilityInfo.bundleName = eleName.GetBundleName();
    abilityInfo.applicationName = "Helloworld";
    return true;
}

bool BundleMgrProxy::GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo)
{
    GTEST_LOG_(INFO) << " BundleMgrProxy::GetHapModuleInfo";
    hapModuleInfo.name = abilityInfo.package;
    return true;
}

bool BundleMgrProxy::GetHapModuleInfo(const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo)
{
    GTEST_LOG_(INFO) << " BundleMgrProxy::GetHapModuleInfo";
    hapModuleInfo.name = abilityInfo.package;
    return true;
}

int BundleMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    GTEST_LOG_(INFO) << " BundleMgrStub::OnRemoteRequest";
    return 0;
}

bool BundleMgrService::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
{
    if (appName.empty()) {
        return false;
    }
    appInfo.name = "Helloworld";
    appInfo.bundleName = "com.foobar.hiworld";
    if (appName == "com.crowdtest.expired") {
        appInfo.appDistributionType = "crowdtesting";
        appInfo.crowdtestDeadline = 0;
    }
    if (appName == "com.crowdtest.unexpired") {
        appInfo.appDistributionType = "crowdtesting";
        appInfo.crowdtestDeadline = UNEXPIRED_TIME;
    }
    return true;
}

std::string BundleMgrService::GetAppType(const std::string &bundleName)
{
    GTEST_LOG_(INFO) << " BundleMgrService::GetAppType";
    return "system";
}

bool BundleMgrService::QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo)
{
    ElementName elementName = want.GetElement();
    if (elementName.GetBundleName().empty()) {
        return false;
    }
    if (std::string::npos != elementName.GetBundleName().find("service")) {
        abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    }
    abilityInfo.visible = true;
    abilityInfo.name = elementName.GetAbilityName();
    abilityInfo.bundleName = elementName.GetBundleName();
    abilityInfo.applicationName = elementName.GetBundleName();
    if (want.HasEntity(Want::ENTITY_HOME) && want.GetAction() == Want::ACTION_HOME) {
        abilityInfo.applicationInfo.isLauncherApp = true;
    } else {
        abilityInfo.applicationInfo.isLauncherApp = false;
    }
    return true;
}

bool BundleMgrService::GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo)
{
    GTEST_LOG_(INFO) << " BundleMgrService::GetHapModuleInfo";
    hapModuleInfo.name = abilityInfo.package;
    return true;
}
bool BundleMgrService::GetHapModuleInfo(const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo)
{
    GTEST_LOG_(INFO) << " BundleMgrService::GetHapModuleInfo";
    hapModuleInfo.name = abilityInfo.package;
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
