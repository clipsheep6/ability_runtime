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

#include <gtest/gtest.h>
#define private public
#define protected public
#include "ability_manager_service.h"
#include "configuration.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {

class ConfigurationTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void OnStartAms();
    std::shared_ptr<AbilityManagerService> ams_ {nullptr};
};

void ConfigurationTest::SetUpTestCase(void)
{}

void ConfigurationTest::TearDownTestCase(void)
{}

void ConfigurationTest::SetUp(void)
{
    ams_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
}
void ConfigurationTest::TearDown(void)
{}

void OnStartAms()
{
    // if (ams_) {
    //     if (ams_->state_ == ServiceRunningState::STATE_RUNNING) {
    //         return;
    //     }

    //     ams_->state_ = ServiceRunningState::STATE_RUNNING;
    //     ams_->eventLoop_ =
    //         AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    //     EXPECT_TRUE(ams_->eventLoop_);

    //     ams_->handler_ = std::make_shared<AbilityEventHandler>(ams_->eventLoop_ , ams_);
    //     EXPECT_TRUE(ams_->handler_);
    //     EXPECT_TRUE(ams_->connectManager_);

    //     ams_->connectManager_->SetEventHandler(ams_->handler_);

    //     ams_->dataAbilityManager_ = std::make_shared<DataAbilityManager>();
    //     EXPECT_TRUE(ams_->dataAbilityManager_);

    //     ams_->amsConfigResolver_ = std::make_shared<AmsConfigurationParameter>();
    //     EXPECT_TRUE(ams_->amsConfigResolver_);
    //     ams_->amsConfigResolver_->Parse();

    //     ams_->pendingWantManager_ = std::make_shared<PendingWantManager>();
    //     EXPECT_TRUE(ams_->pendingWantManager_);

    //     ams_->configuration_ = std::make_shared<AppExecFwk::Configuration>();
    //     EXPECT_TRUE(ams_->configuration_);
    //     GetGlobalConfiguration();

    //     int userId = ams_->GetUserId();
    //     ams_->SetStackManager(userId);
    //     ams_->systemAppManager_ = std::make_shared<KernalSystemAppManager>(userId);
    //     EXPECT_TRUE(ams_->systemAppManager_);

    //     ams_->eventLoop_->Run();

    //     GTEST_LOG_(INFO)<< "OnStart success";
    //     return;
    // }

    GTEST_LOG_(INFO) << "OnStart fail";
}
/*
 * Feature: Configuration
 * Function: AddItem
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Update Configuration
 */
HWTEST_F(ConfigurationTest, AddItem_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string val {"中文"};
    EXPECT_EQ(0, config.GetItemSize());
    config.AddItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE, val);
    EXPECT_EQ(1, config.GetItemSize());

    // replace
    config.AddItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE, val);
    EXPECT_EQ(1, config.GetItemSize());
}

/*
 * Feature: Configuration
 * Function: GetItem
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Process Configuration Change
 */
HWTEST_F(ConfigurationTest, GetItem_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string val {"中文"};
    config.AddItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE, val);

    auto item = config.GetItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, val);

    // replace
    std::string english {"英文"};
    config.AddItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE, english);
    item = config.GetItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, english);

    int displayId2 = 1002;
    std::string non {""};
    item = config.GetItem(displayId2, GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, non);
}

/*
 * Feature: Configuration
 * Function: RemoveItem
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Process Configuration Change
 */
HWTEST_F(ConfigurationTest, RemoveItem_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string val {"中文"};
    config.AddItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE, val);

    auto item = config.GetItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, val);

    // remove it
    bool canRemove = config.RemoveItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_TRUE(canRemove);

    std::string non {""};
    item = config.GetItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, non);

    canRemove = config.RemoveItem(displayId, non);
    EXPECT_FALSE(canRemove);

    int displayId2 = 1002;
    canRemove = config.RemoveItem(displayId2, GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_FALSE(canRemove);
}

/*
 * Feature: Configuration
 * Function: GetItemSize
 * SubFunction: Process Configuration Change Inner
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Process Configuration Change Inner
 */
HWTEST_F(ConfigurationTest, GetItemSize_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string val {"中文"};
    EXPECT_EQ(0, config.GetItemSize());
    config.AddItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE, val);
    EXPECT_EQ(1, config.GetItemSize());

    int displayId2= 1002;
    config.AddItem(displayId2, GlobalConfigurationKey::SYSTEM_LANGUAGE, val);
    EXPECT_EQ(2, config.GetItemSize());
}

/*
 * Feature: Configuration
 * Function: CompareDifferent
 * SubFunction: Process Configuration Change Inner
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Check according to key 
 */
HWTEST_F(ConfigurationTest, CompareDifferent_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string val {"中文"};
    config.AddItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE, val);


    AppExecFwk::Configuration config2;
    int displayId2= 1001;
    std::string English {"英文"};
    config2.AddItem(displayId2, GlobalConfigurationKey::SYSTEM_LANGUAGE, English);

    std::vector<std::string> changeKeyV;

    config.CompareDifferent(changeKeyV, config2);
    int size = changeKeyV.size();
    EXPECT_EQ(size, 1);
}

/*
 * Feature: Configuration
 * Function: CompareDifferent
 * SubFunction: Process Configuration Change Inner
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Check according to key 
 */
HWTEST_F(ConfigurationTest, CompareDifferent_002, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string val {"中文"};
    config.AddItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE, val);


    AppExecFwk::Configuration config2;
    int displayId2= 1002;
    std::string English {"英文"};
    config2.AddItem(displayId2, GlobalConfigurationKey::SYSTEM_LANGUAGE, English);

    std::vector<std::string> changeKeyV;

    config.CompareDifferent(changeKeyV, config2);
    int size = changeKeyV.size();
    EXPECT_EQ(size, 1);
}

/*
 * Feature: Configuration
 * Function: Merge
 * SubFunction: Process Configuration Change Inner
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Check according to key 
 */
HWTEST_F(ConfigurationTest, Merge_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string val {"中文"};
    config.AddItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE, val);


    AppExecFwk::Configuration config2;
    int displayId2= 1002;
    std::string English {"英文"};
    config2.AddItem(displayId2, GlobalConfigurationKey::SYSTEM_LANGUAGE, English);

    std::vector<std::string> changeKeyV;

    config.CompareDifferent(changeKeyV, config2);
    int size = changeKeyV.size();
    EXPECT_EQ(size, 1);

    for (const auto &s : changeKeyV) {
        config.Merge(s, config2);
    }

    auto item = config.GetItem(displayId2, GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, English);
}

/*
 * Feature: Configuration
 * Function: Merge
 * SubFunction: Process Configuration Change Inner
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Check according to key 
 */
HWTEST_F(ConfigurationTest, Merge_002, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string chinese {"Chinese "};
    config.AddItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE, chinese);


    AppExecFwk::Configuration config2;
    std::string english {"English"};
    config2.AddItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE, english);

    std::vector<std::string> changeKeyV;

    config.CompareDifferent(changeKeyV, config2);
    int size = changeKeyV.size();
    EXPECT_EQ(size, 1);

    for (const auto &s : changeKeyV) {
        config.Merge(s, config2);
    }

    auto item = config.GetItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, english);
}
/*
 * Feature: Configuration
 * Function: GetName
 * SubFunction: Process Configuration Change Inner
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Check according to key 
 */
HWTEST_F(ConfigurationTest, GetName_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string chinese {"Chinese "};
    config.AddItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE, chinese);

    int displayId2= 1002;
    std::string english {"English"};
    config.AddItem(displayId2, GlobalConfigurationKey::SYSTEM_LANGUAGE, english);

    auto name = config.GetName();
    EXPECT_TRUE( name != "");
    GTEST_LOG_(INFO) << "get name pair ["<<name<<"]";
}

/*
 * Feature: Configuration
 * Function: Assignment structure 
 * SubFunction: Process Configuration Change Inner
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Check according to key 
 */
HWTEST_F(ConfigurationTest, structure_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string chinese {"Chinese "};
    config.AddItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE, chinese);

    AppExecFwk::Configuration configOther(config);
    auto item = configOther.GetItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_TRUE( item == chinese);

    AppExecFwk::Configuration configThird;
    configThird = configOther;

    auto item2 = configThird.GetItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_TRUE( item2 == chinese);

    AppExecFwk::Configuration configFourth  = configThird;
    auto item3 = configFourth.GetItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_TRUE( item3 == chinese);
}
}  // namespace AAFwk
}  // namespace OHOS