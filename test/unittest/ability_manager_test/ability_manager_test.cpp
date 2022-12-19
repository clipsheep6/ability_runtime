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

#include <gtest/gtest.h>
#define private public
#define protected public
#include "ability_manager.h"
#include "ability_manager_stub_mock.h"
#undef private
#undef protected
#include "hilog_wrapper.h"
using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AAFwk {
namespace {
    constexpr int32_t ONE = 1;
    constexpr int32_t NEGATIVE = -1;
}
using namespace OHOS::AppExecFwk;
class AbilityManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/*
 * @tc.number    : AbilityManagerTest_0100
 * @tc.name      : AbilityManager
 * @tc.desc      : Test Function AbilityManager::GetInstance() and AbilityManager::StartAbility
 */
HWTEST_F(AbilityManagerTest, AbilityManagerTest_0100, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerTest_0100 is start");
    int32_t requestCode = NEGATIVE;
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    sptr<AAFwk::AbilityManagerStubTestMock> mock = new AAFwk::AbilityManagerStubTestMock();
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = mock;
    ErrCode error = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, requestCode);
    EXPECT_EQ(error, ERR_OK);
    AbilityManager::GetInstance().StartAbility(want, requestCode);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = nullptr;
    HILOG_INFO("AbilityManagerTest_0100 is end");
}

/*
 * @tc.number    : AbilityManagerTest_0200
 * @tc.name      : AbilityManager
 * @tc.desc      : Test Function AbilityManager::GetInstance() and AbilityManager::StartAbility
 */
HWTEST_F(AbilityManagerTest, AbilityManagerTest_0200, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerTest_0200 is start");
    int32_t requestCode = NEGATIVE;
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    ErrCode error = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, requestCode);
    EXPECT_NE(error, ERR_OK);
    AbilityManager::GetInstance().StartAbility(want, requestCode);
    HILOG_INFO("AbilityManagerTest_0200 is end");
}

/*
 * @tc.number    : AbilityManagerTest_0300
 * @tc.name      : AbilityManager
 * @tc.desc      : Test Function AbilityManager::ClearUpApplicationData
 */
HWTEST_F(AbilityManagerTest, AbilityManagerTest_0300, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerTest_0300 is start");
    const std::string bundleName = "test";
    auto res = AbilityManager::GetInstance().ClearUpApplicationData(bundleName);
    EXPECT_EQ(res, ONE);
    HILOG_INFO("AbilityManagerTest_0300 is end");
}

/*
 * @tc.number    : AbilityManagerTest_0400
 * @tc.name      : AbilityManager
 * @tc.desc      : Test Function AbilityManager::GetAllRunningProcesses
 */
HWTEST_F(AbilityManagerTest, AbilityManagerTest_0400, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerTest_0400 is start");
    auto res = AbilityManager::GetInstance().GetAllRunningProcesses();
    EXPECT_TRUE(res.empty());
    HILOG_INFO("AbilityManagerTest_0400 is end");
}

/*
 * @tc.number    : AbilityManagerTest_0500
 * @tc.name      : AbilityManager
 * @tc.desc      : Test Function AbilityManager::KillProcessesByBundleName
 */
HWTEST_F(AbilityManagerTest, AbilityManagerTest_0500, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerTest_0500 is start");
    const std::string bundleName = "test";
    auto res = AbilityManager::GetInstance().KillProcessesByBundleName(bundleName);
    EXPECT_NE(res, ERR_OK);
    HILOG_INFO("AbilityManagerTest_0500 is end");
}

/*
 * @tc.number    : AbilityManagerTest_0600
 * @tc.name      : AbilityManager
 * @tc.desc      : Test Function AbilityManager::KillProcessesByBundleName
 */
HWTEST_F(AbilityManagerTest, AbilityManagerTest_0600, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerTest_0600 is start");
    const std::string bundleName = "test";
    sptr<AAFwk::AbilityManagerStubTestMock> mock = new AAFwk::AbilityManagerStubTestMock();
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = mock;
    auto res = AbilityManager::GetInstance().KillProcessesByBundleName(bundleName);
    EXPECT_EQ(res, ERR_OK);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = nullptr;
    HILOG_INFO("AbilityManagerTest_0600 is end");
}
} // namespace AAFwk
} // namespace OHOS