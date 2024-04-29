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

#include <gtest/gtest.h>

#define private public
#define protected public
#include "resident_process_manager.h"
#undef private
#undef protected
#include "ability_manager_service.h"
#include "user_controller.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {

class ResidentProcessManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ResidentProcessManagerTest::SetUpTestCase(void)
{}
void ResidentProcessManagerTest::TearDownTestCase(void)
{}
void ResidentProcessManagerTest::SetUp()
{}
void ResidentProcessManagerTest::TearDown()
{}

/*
 * Feature: ResidentProcessManager
 * Function: StartResidentProcessWithMainElement
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager StartResidentProcessWithMainElement
 * EnvConditions: NA
 * CaseDescription: Verify StartResidentProcessWithMainElement
 */
HWTEST_F(ResidentProcessManagerTest, StartResidentProcessWithMainElement_001, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    std::vector<BundleInfo> bundleInfos;
    BundleInfo bundleInfo1;
    BundleInfo bundleInfo2;
    HapModuleInfo hapModuleInfo1;
    HapModuleInfo hapModuleInfo2;
    hapModuleInfo1.isModuleJson = false;
    hapModuleInfo1.mainAbility = "";
    hapModuleInfo2.isModuleJson = true;
    hapModuleInfo2.mainElementName = "mainElementName";
    hapModuleInfo2.process = "process";
    bundleInfo1.isKeepAlive = true;
    bundleInfo1.applicationInfo.process = "";
    bundleInfo2.isKeepAlive = true;
    bundleInfo2.applicationInfo.process = "process";
    bundleInfo2.hapModuleInfos.emplace_back(hapModuleInfo1);
    bundleInfo2.hapModuleInfos.emplace_back(hapModuleInfo2);
    bundleInfos.emplace_back(bundleInfo1);
    bundleInfos.emplace_back(bundleInfo2);
    manager->StartResidentProcessWithMainElement(bundleInfos);
}

/*
 * Feature: ResidentProcessManager
 * Function: CheckMainElement
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager CheckMainElement
 * EnvConditions: NA
 * CaseDescription: Verify CheckMainElement
 */
HWTEST_F(ResidentProcessManagerTest, CheckMainElement_001, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    HapModuleInfo hapModuleInfo;
    std::string processName = "processName";
    std::string mainElement = "";
    std::set<uint32_t> needEraseIndexSet;
    size_t bundleInfoIndex = 0;
    AbilityInfo abilityInfo;
    hapModuleInfo.mainAbility = "mainAbility";
    hapModuleInfo.isModuleJson = false;
    abilityInfo.process = "process";
    abilityInfo.name = "mainAbility";
    hapModuleInfo.abilityInfos.push_back(abilityInfo);
    bool res = manager->CheckMainElement(hapModuleInfo, processName, mainElement, needEraseIndexSet, bundleInfoIndex);
    EXPECT_FALSE(res);
    manager.reset();
}

/*
 * Feature: ResidentProcessManager
 * Function: CheckMainElement
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager CheckMainElement
 * EnvConditions: NA
 * CaseDescription: Verify CheckMainElement
 */
HWTEST_F(ResidentProcessManagerTest, CheckMainElement_002, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    HapModuleInfo hapModuleInfo;
    std::string processName = "processName";
    std::string mainElement = "";
    std::set<uint32_t> needEraseIndexSet;
    size_t bundleInfoIndex = 0;
    AbilityInfo abilityInfo;
    hapModuleInfo.mainAbility = "mainAbility";
    hapModuleInfo.isModuleJson = false;
    abilityInfo.process = "processName";
    abilityInfo.name = "";
    hapModuleInfo.abilityInfos.push_back(abilityInfo);
    bool res = manager->CheckMainElement(hapModuleInfo, processName, mainElement, needEraseIndexSet, bundleInfoIndex);
    EXPECT_FALSE(res);
    manager.reset();
}

/*
 * Feature: ResidentProcessManager
 * Function: CheckMainElement
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager CheckMainElement
 * EnvConditions: NA
 * CaseDescription: Verify CheckMainElement
 */
HWTEST_F(ResidentProcessManagerTest, CheckMainElement_003, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    HapModuleInfo hapModuleInfo;
    std::string processName = "processName";
    std::string mainElement = "";
    std::set<uint32_t> needEraseIndexSet;
    size_t bundleInfoIndex = 0;
    AbilityInfo abilityInfo;
    hapModuleInfo.mainAbility = "mainAbility";
    hapModuleInfo.isModuleJson = false;
    abilityInfo.process = "processName";
    abilityInfo.name = "mainAbility";
    abilityInfo.uri = "//";
    abilityInfo.type = AbilityType::DATA;
    hapModuleInfo.abilityInfos.push_back(abilityInfo);
    bool res = manager->CheckMainElement(hapModuleInfo, processName, mainElement, needEraseIndexSet, bundleInfoIndex);
    EXPECT_FALSE(res);
    manager.reset();
}

/*
 * Feature: ResidentProcessManager
 * Function: CheckMainElement
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager CheckMainElement
 * EnvConditions: NA
 * CaseDescription: Verify CheckMainElement
 */
HWTEST_F(ResidentProcessManagerTest, CheckMainElement_004, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    HapModuleInfo hapModuleInfo;
    std::string processName = "processName";
    std::string mainElement = "";
    std::set<uint32_t> needEraseIndexSet;
    size_t bundleInfoIndex = 0;
    AbilityInfo abilityInfo;
    hapModuleInfo.mainAbility = "mainAbility";
    hapModuleInfo.isModuleJson = false;
    abilityInfo.process = "processName";
    abilityInfo.name = "mainAbility";
    abilityInfo.uri = "//";
    abilityInfo.type = AbilityType::PAGE;
    hapModuleInfo.abilityInfos.push_back(abilityInfo);
    bool res = manager->CheckMainElement(hapModuleInfo, processName, mainElement, needEraseIndexSet, bundleInfoIndex);
    EXPECT_FALSE(res);
    manager.reset();
}

/*
 * Feature: ResidentProcessManager
 * Function: CheckMainElement
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager CheckMainElement
 * EnvConditions: NA
 * CaseDescription: Verify CheckMainElement
 */
HWTEST_F(ResidentProcessManagerTest, CheckMainElement_005, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    HapModuleInfo hapModuleInfo;
    std::string processName = "processName";
    std::string mainElement = "";
    std::set<uint32_t> needEraseIndexSet;
    size_t bundleInfoIndex = 0;
    hapModuleInfo.mainAbility = "";
    hapModuleInfo.isModuleJson = true;
    bool res = manager->CheckMainElement(hapModuleInfo, processName, mainElement, needEraseIndexSet, bundleInfoIndex);
    EXPECT_FALSE(res);
    manager.reset();
}

/*
 * Feature: ResidentProcessManager
 * Function: CheckMainElement
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager CheckMainElement
 * EnvConditions: NA
 * CaseDescription: Verify CheckMainElement
 */
HWTEST_F(ResidentProcessManagerTest, CheckMainElement_006, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    HapModuleInfo hapModuleInfo;
    std::string processName = "processName";
    std::string mainElement = "";
    std::set<uint32_t> needEraseIndexSet;
    size_t bundleInfoIndex = 0;
    hapModuleInfo.mainAbility = "mainAbility";
    hapModuleInfo.isModuleJson = true;
    hapModuleInfo.process = "";
    bool res = manager->CheckMainElement(hapModuleInfo, processName, mainElement, needEraseIndexSet, bundleInfoIndex);
    EXPECT_FALSE(res);
    manager.reset();
}
}  // namespace AAFwk
}  // namespace OHOS
