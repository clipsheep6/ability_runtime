/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#define private public
#define protected public
#include "ability_running_record.h"
#include "ability_window_configuration.h"
#include "app_running_record.h"
#include "app_mgr_service_event_handler.h"
#include "app_mgr_service_inner.h"
#include "module_running_record.h"
#include "want_params.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppRunningRecordTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppRunningRecordTest::SetUpTestCase(void)
{}

void AppRunningRecordTest::TearDownTestCase(void)
{}

void AppRunningRecordTest::SetUp()
{}

void AppRunningRecordTest::TearDown()
{}

/**
 * @tc.name: GetSplitModeAndFloatingMode_0100
 * @tc.desc: Test the return when abilityWant is not nullptr and
 *      the first determine of windowMode is conformed.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningRecordTest, GetSplitModeAndFloatingMode_001, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> info = nullptr;
    int32_t recordId = 0;
    std::string processName = "appRunningRecordProcessName";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(info, recordId, processName);
    
    std::map<std::string, std::vector<std::shared_ptr<ModuleRunningRecord>>> hapModules_;
    std::string hapModulesString = "hapModulesString";
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ModuleRunningRecord> moduleRunningRecord = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);    
    std::vector<std::shared_ptr<ModuleRunningRecord>> hapModulesVector;
    hapModulesVector.emplace_back(moduleRunningRecord);
    hapModules_.emplace(hapModulesString, hapModulesVector);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = nullptr;    
    std::shared_ptr<AbilityRunningRecord> abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token);

    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    want->SetParam(AAFwk::Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING);            
    abilityRecord->SetWant(want);

    bool isSplitScreenMode = false;
    bool isFloatingWindowMode = false;
    appRunningRecord->GetSplitModeAndFloatingMode(isSplitScreenMode, isFloatingWindowMode);
    EXPECT_EQ(true, isFloatingWindowMode);
}
} // namespace AppExecFwk
} // namespace OHOS
