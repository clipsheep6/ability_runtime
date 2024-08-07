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
#include "ability_auto_startup_data_manager.h"
#include "mock_single_kv_store.h"
#undef private
#undef protected

#include "auto_startup_info.h"
#include "types.h"
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {
class AbilityAutoStartupDataManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityAutoStartupDataManagerTest::SetUpTestCase() {}

void AbilityAutoStartupDataManagerTest::TearDownTestCase() {}

void AbilityAutoStartupDataManagerTest::SetUp() {}

void AbilityAutoStartupDataManagerTest::TearDown() {}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: GetKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager GetKvStore
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, GetKvStore_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupDataManagerTest_100 start";
    auto abilityMs = std::make_shared<AbilityAutoStartupDataManager>();
    auto result = abilityMs->GetKvStore();
    EXPECT_EQ(result, DistributedKv::Status::SUCCESS);
    GTEST_LOG_(INFO) << "AbilityAutoStartupDataManagerTest_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: CheckKvStore
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager CheckKvStore
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, CheckKvStore_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckKvStore_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    auto result = abilityAutoStartupDataManager.CheckKvStore();
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << "CheckKvStore_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: InsertAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager InsertAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, InsertAutoStartupData_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertAutoStartupData_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.abilityName = "testDemoAbility";
    bool isAutoStartup = false;
    bool isEdmForce = false;
    auto result = abilityAutoStartupDataManager.InsertAutoStartupData(info, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "InsertAutoStartupData_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: InsertAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager InsertAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, InsertAutoStartupData_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertAutoStartupData_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    bool isAutoStartup = false;
    bool isEdmForce = false;
    auto result = abilityAutoStartupDataManager.InsertAutoStartupData(info, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "InsertAutoStartupData_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: InsertAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager InsertAutoStartupData
 */

HWTEST_F(AbilityAutoStartupDataManagerTest, InsertAutoStartupData_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertAutoStartupData_300 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.userId = 100;
    struct AutoStartupStatus AutoStartupStatus;
    AutoStartupStatus.isAutoStartup = false;
    AutoStartupStatus.isEdmForce = false;
    auto result = abilityAutoStartupDataManager.InsertAutoStartupData(
        info, AutoStartupStatus.isAutoStartup, AutoStartupStatus.isEdmForce);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "InsertAutoStartupData_300 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: InsertAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager InsertAutoStartupData
 */

HWTEST_F(AbilityAutoStartupDataManagerTest, InsertAutoStartupData_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertAutoStartupData_400 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());

    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.userId = 100;
    struct AutoStartupStatus AutoStartupStatus;
    AutoStartupStatus.isAutoStartup = false;
    AutoStartupStatus.isEdmForce = false;
    auto result = abilityAutoStartupDataManager.InsertAutoStartupData(
        info, AutoStartupStatus.isAutoStartup, AutoStartupStatus.isEdmForce);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "InsertAutoStartupData_400 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: UpdateAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager UpdateAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, UpdateAutoStartupData_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    bool isAutoStartup = false;
    bool isEdmForce = false;
    auto result = abilityAutoStartupDataManager.UpdateAutoStartupData(info, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: UpdateAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager UpdateAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, UpdateAutoStartupData_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.userId = 100;
    bool isAutoStartup = false;
    bool isEdmForce = false;
    auto result = abilityAutoStartupDataManager.UpdateAutoStartupData(info, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: UpdateAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager UpdateAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, UpdateAutoStartupData_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_300 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.abilityName = "testDemoAbility";
    bool isAutoStartup = false;
    bool isEdmForce = false;
    auto result = abilityAutoStartupDataManager.UpdateAutoStartupData(info, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_300 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: UpdateAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager UpdateAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, UpdateAutoStartupData_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_400 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());

    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.userId = 100;
    bool isAutoStartup = false;
    bool isEdmForce = false;
    auto result = abilityAutoStartupDataManager.UpdateAutoStartupData(info, isAutoStartup, isEdmForce);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "UpdateAutoStartupData_400 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, DeleteAutoStartupData_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.abilityName = "testDemoAbility";
    auto result = abilityAutoStartupDataManager.DeleteAutoStartupData(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, DeleteAutoStartupData_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    auto result = abilityAutoStartupDataManager.DeleteAutoStartupData(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, DeleteAutoStartupData_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_300 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.userId = 100;
    auto result = abilityAutoStartupDataManager.DeleteAutoStartupData(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_300 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, DeleteAutoStartupData_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_400 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());

    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.userId = 100;
    auto result = abilityAutoStartupDataManager.DeleteAutoStartupData(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_400 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, DeleteAutoStartupData_bundleName_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_bundleName_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::string bundleName = "";
    int32_t accessTokenId = 0;
    auto result = abilityAutoStartupDataManager.DeleteAutoStartupData(bundleName, accessTokenId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_bundleName_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, DeleteAutoStartupData_bundleName_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_bundleName_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::string bundleName = "com.example.testbundle";
    int32_t accessTokenId = 0;
    auto result = abilityAutoStartupDataManager.DeleteAutoStartupData(bundleName, accessTokenId);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_bundleName_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: DeleteAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager DeleteAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, DeleteAutoStartupData_bundleName_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_bundleName_300 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());

    std::string bundleName = "com.example.testbundle";
    int32_t accessTokenId = 0;
    auto result = abilityAutoStartupDataManager.DeleteAutoStartupData(bundleName, accessTokenId);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DeleteAutoStartupData_bundleName_300 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: QueryAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager QueryAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, QueryAutoStartupData_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAutoStartupData_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    auto result = abilityAutoStartupDataManager.QueryAutoStartupData(info);
    EXPECT_EQ(result.code, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "QueryAutoStartupData_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: QueryAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager QueryAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, QueryAutoStartupData_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAutoStartupData_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.abilityName = "testDemoAbility";
    auto result = abilityAutoStartupDataManager.QueryAutoStartupData(info);
    EXPECT_EQ(result.code, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "QueryAutoStartupData_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: QueryAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager QueryAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, QueryAutoStartupData_300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAutoStartupData_300 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.userId = 100;
    auto result = abilityAutoStartupDataManager.QueryAutoStartupData(info);
    EXPECT_EQ(result.code, ERR_NAME_NOT_FOUND);
    GTEST_LOG_(INFO) << "QueryAutoStartupData_300 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: QueryAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager QueryAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, QueryAutoStartupData_400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAutoStartupData_400 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());

    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    info.abilityName = "testDemoAbility";
    info.accessTokenId = "123";
    info.userId = 100;
    auto result = abilityAutoStartupDataManager.QueryAutoStartupData(info);
    EXPECT_EQ(result.code, ERR_NAME_NOT_FOUND);
    GTEST_LOG_(INFO) << "QueryAutoStartupData_400 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: QueryAllAutoStartupApplications
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager QueryAllAutoStartupApplications
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, QueryAllAutoStartupApplications_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAllAutoStartupApplications_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::vector<AutoStartupInfo> infoList;
    int32_t userID = 100;
    auto result = abilityAutoStartupDataManager.QueryAllAutoStartupApplications(infoList, userID);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "QueryAllAutoStartupApplications_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: QueryAllAutoStartupApplications
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager QueryAllAutoStartupApplications
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, QueryAllAutoStartupApplications_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAllAutoStartupApplications_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());

    std::vector<AutoStartupInfo> infoList;
    int32_t userID = 100;
    auto result = abilityAutoStartupDataManager.QueryAllAutoStartupApplications(infoList, userID);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "QueryAllAutoStartupApplications_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: GetCurrentAppAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager GetCurrentAppAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, GetCurrentAppAutoStartupData_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCurrentAppAutoStartupData_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    std::vector<AutoStartupInfo> infoList;
    std::string accessTokenId = "0";
    auto result = abilityAutoStartupDataManager.GetCurrentAppAutoStartupData(info.bundleName, infoList, accessTokenId);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "GetCurrentAppAutoStartupData_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: GetCurrentAppAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager GetCurrentAppAutoStartupData
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, GetCurrentAppAutoStartupData_200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCurrentAppAutoStartupData_200 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    abilityAutoStartupDataManager.kvStorePtr_ = kvStorePtr;
    EXPECT_EQ(true, abilityAutoStartupDataManager.CheckKvStore());

    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    std::vector<AutoStartupInfo> infoList;
    std::string accessTokenId = "0";
    auto result = abilityAutoStartupDataManager.GetCurrentAppAutoStartupData(info.bundleName, infoList, accessTokenId);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "GetCurrentAppAutoStartupData_200 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: ConvertAutoStartupStatusFromValue
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager ConvertAutoStartupStatusFromValue
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, ConvertAutoStartupStatusFromValue_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertAutoStartupStatusFromValue_100 start";
    auto abilityAutoStartupDataManager = DelayedSingleton<AbilityAutoStartupDataManager>::GetInstance();
    DistributedKv::Value value;
    bool isAutoStartup = false;
    bool isEdmForce = false;
    abilityAutoStartupDataManager->ConvertAutoStartupStatusFromValue(value, isAutoStartup, isEdmForce);
    EXPECT_NE(abilityAutoStartupDataManager, nullptr);
    GTEST_LOG_(INFO) << "ConvertAutoStartupStatusFromValue_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_info_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_info_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    AutoStartupInfo info;
    info.bundleName = "com.example.testbundle";
    DistributedKv::Key key;
    auto result = abilityAutoStartupDataManager.IsEqual(key, info);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqual_info_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_userId_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_userId_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    DistributedKv::Key key;
    int32_t userId = 100;
    auto result = abilityAutoStartupDataManager.IsEqual(key, userId);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqual_userId_100 end";
}

/**
 * Feature: AbilityAutoStartupDataManager
 * Function: IsEqual
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupDataManager IsEqual
 */
HWTEST_F(AbilityAutoStartupDataManagerTest, IsEqual_accessTokenId_100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEqual_userId_100 start";
    AbilityAutoStartupDataManager abilityAutoStartupDataManager;
    DistributedKv::Key key;
    std::string accessTokenId = "123";
    auto result = abilityAutoStartupDataManager.IsEqual(key, accessTokenId);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsEqual_accessTokenId_100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
