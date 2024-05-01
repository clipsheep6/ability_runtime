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

#include <thread>
#include <functional>
#include <fstream>
#include <nlohmann/json.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#define private public
#define protected public
#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#undef private
#undef protected

using namespace std::placeholders;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {
class SpecifiedAbilityServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    Want CreateWant(const std::string& entity);
    AbilityInfo CreateAbilityInfo(const std::string& name, const std::string& appName, const std::string& bundleName);
    ApplicationInfo CreateAppInfo(const std::string& appName, const std::string& name);
};

Want SpecifiedAbilityServiceTest::CreateWant(const std::string& entity)
{
    Want want;
    if (!entity.empty()) {
        want.AddEntity(entity);
    }
    return want;
}

AbilityInfo SpecifiedAbilityServiceTest::CreateAbilityInfo(
    const std::string& name, const std::string& appName, const std::string& bundleName)
{
    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.name = name;
    abilityInfo.applicationName = appName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.applicationInfo.bundleName = bundleName;
    abilityInfo.applicationName = "hiMusic";
    abilityInfo.applicationInfo.name = "hiMusic";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = false;

    return abilityInfo;
}

ApplicationInfo SpecifiedAbilityServiceTest::CreateAppInfo(const std::string& appName, const std::string& bundleName)
{
    ApplicationInfo appInfo;
    appInfo.name = appName;
    appInfo.bundleName = bundleName;

    return appInfo;
}

void SpecifiedAbilityServiceTest::SetUpTestCase(void) {}

void SpecifiedAbilityServiceTest::TearDownTestCase(void) {}

void SpecifiedAbilityServiceTest::SetUp(void) {}

void SpecifiedAbilityServiceTest::TearDown(void) {}

/**
 * @tc.name: OnAcceptWantResponse_001
 * @tc.desc: test OnAcceptWantResponse
 * @tc.type: FUNC
 * @tc.require: AR000GJUND
 */
HWTEST_F(SpecifiedAbilityServiceTest, OnAcceptWantResponse_001, TestSize.Level1)
{
    auto abilityMgrServ_ = std::make_shared<AbilityManagerService>();
    abilityMgrServ_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    std::string abilityName = "MusicAbility";
    std::string appName = "test_app";
    std::string bundleName = "com.ix.hiMusic";

    AbilityRequest abilityRequest;
    abilityRequest.want = CreateWant("");
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName + "1", appName, bundleName);
    abilityRequest.appInfo = CreateAppInfo(appName, bundleName);

    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::FOREGROUND);

    abilityMgrServ_->subManagersHelper_->InitMissionListManager(11, true);
    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    EXPECT_TRUE(abilityMgrServ_->subManagersHelper_->currentMissionListManager_);
    abilityMgrServ_->subManagersHelper_->currentMissionListManager_->EnqueueWaitingAbility(abilityRequest);
    abilityMgrServ_->OnAcceptWantResponse(want, "flag");

    EXPECT_EQ(false, abilityRecord->IsNewWant());
}
}  // namespace AAFwk
}  // namespace OHOS
