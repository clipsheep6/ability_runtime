/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "app_config_data_manager.h"
#include "app_state_callback_host.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "mock_ability_token.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppConfigDataManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
public:
protected:
    static const std::string GetTestBundleName()
    {
        return "test_bundle_name";
    }
    static const std::string GetTestAbilityInfoName()
    {
        return "test_ability_info_name";
    }
    static const std::string GetTestModuleName()
    {
        return "test_module_name";
    }
};

void AppConfigDataManagerTest::SetUpTestCase()
{}

void AppConfigDataManagerTest::TearDownTestCase()
{}

void AppConfigDataManagerTest::SetUp()
{}

void AppConfigDataManagerTest::TearDown()
{}


/*
 * Feature: AppConfigDataManager
 * Function: SetAppWaitingDebugInfo
 * SubFunction: NA
 * FunctionPoints: AppConfigDataManager SetAppWaitingDebugInfo
 * EnvConditions: NA
 * CaseDescription: bundle name empty
 */
HWTEST_F(AppConfigDataManagerTest, SetAppWaitingDebugInfo_001, TestSize.Level1)
{
    auto manager = std::make_shared<AbilityRuntime::AppConfigDataManager>();
    const std::string bundleName;
    auto iret = manager->SetAppWaitingDebugInfo(bundleName);
    ASSERT_EQ(iret, ERR_INVALID_VALUE);
}

/*
 * Feature: AppConfigDataManager
 * Function: SetAppWaitingDebugInfo
 * SubFunction: NA
 * FunctionPoints: AppConfigDataManager SetAppWaitingDebugInfo
 * EnvConditions: NA
 * CaseDescription: set ok
 */
HWTEST_F(AppConfigDataManagerTest, SetAppWaitingDebugInfo_002, TestSize.Level1)
{
    auto manager = std::make_shared<AbilityRuntime::AppConfigDataManager>();
    const std::string bundleName = "bundle";
    auto iret = manager->SetAppWaitingDebugInfo(bundleName);
    ASSERT_EQ(iret, ERR_OK);
}

/*
 * Feature: AppConfigDataManager
 * Function: ClearAppWaitingDebugInfo
 * SubFunction: NA
 * FunctionPoints: AppConfigDataManager ClearAppWaitingDebugInfo
 * EnvConditions: NA
 * CaseDescription: clear ok
 */
HWTEST_F(AppConfigDataManagerTest, ClearAppWaitingDebugInfo_001, TestSize.Level1)
{
    auto manager = std::make_shared<AbilityRuntime::AppConfigDataManager>();
    auto iret = manager->ClearAppWaitingDebugInfo();
    ASSERT_EQ(iret, ERR_OK);
}

/*
 * Feature: AppConfigDataManager
 * Function: GetAppWaitingDebugList
 * SubFunction: NA
 * FunctionPoints: AppConfigDataManager GetAppWaitingDebugList
 * EnvConditions: NA
 * CaseDescription: get ok
 */
HWTEST_F(AppConfigDataManagerTest, GetAppWaitingDebugList_001, TestSize.Level1)
{
    auto manager = std::make_shared<AbilityRuntime::AppConfigDataManager>();
    std::vector<std::string> bundleNameList;
    auto iret = manager->GetAppWaitingDebugList(bundleNameList);
    ASSERT_EQ(iret, ERR_OK);
}

}  // namespace AppExecFwk
}  // namespace OHOS
