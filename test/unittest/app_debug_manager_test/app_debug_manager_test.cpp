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
#include "app_debug_manager.h"
#include "app_debug_listener_proxy.h"
#undef private

#include "mock_app_debug_listener_stub.h"
#include "parcel.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace AppExecFwk {
namespace {
    std::string DEBUG_START_NAME = "debugStartBundle";
    std::string NO_DEBUG_START_NAME = "noDebugStartBundle";
    const bool IS_DEBUG_START = true;
    const bool NO_DEBUG_START = false;
    const unsigned int SIZE_ONE = 1;
    const unsigned int SIZE_TWO = 2;
}
class AppDebugManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AppDebugManager> manager_;
    sptr<MockAppDebugListenerStub> listener_;
};

void AppDebugManagerTest::SetUpTestCase(void)
{}

void AppDebugManagerTest::TearDownTestCase(void)
{}

void AppDebugManagerTest::SetUp()
{
    manager_ = std::make_shared<AppDebugManager>();
    listener_ = new MockAppDebugListenerStub();
    manager_->listeners_.insert(listener_);
}

void AppDebugManagerTest::TearDown()
{}

/**
 * @tc.name: RegisterAppDebugListener_0100
 * @tc.desc: Register listener for app debug listener, check nullptr listener.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugManagerTest, RegisterAppDebugListener_0100, TestSize.Level1)
{
    sptr<MockAppDebugListenerStub> listener = new MockAppDebugListenerStub();
    AppDebugInfo appDebugInfo;
    manager_->debugInfos_.push_back(appDebugInfo);

    EXPECT_CALL(*listener, OnAppDebugStarted(_)).Times(1);
    auto result = manager_->RegisterAppDebugListener(listener);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(manager_->listeners_.size(), SIZE_TWO);

    listener = nullptr;
    result = manager_->RegisterAppDebugListener(listener);
    EXPECT_EQ(result, ERR_INVALID_DATA);
    EXPECT_EQ(manager_->listeners_.size(), SIZE_TWO);
}

/**
 * @tc.name: UnregisterAppDebugListener_0100
 * @tc.desc: Unregister listener for app debug listener, check nullptr listener.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugManagerTest, UnregisterAppDebugListener_0100, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    sptr<MockAppDebugListenerStub> listener = nullptr;
    auto result = manager_->UnregisterAppDebugListener(listener);
    EXPECT_EQ(result, ERR_INVALID_DATA);
    EXPECT_EQ(manager_->listeners_.size(), SIZE_ONE);

    EXPECT_NE(listener_, nullptr);
    result = manager_->UnregisterAppDebugListener(listener_);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(manager_->listeners_.empty());
}

/**
 * @tc.name: StartDebug_0100
 * @tc.desc: Start debug by AppDebugInfo, notify AppDebugListener.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugManagerTest, StartDebug_0100, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    EXPECT_TRUE(manager_->debugInfos_.empty());
    std::vector<AppDebugInfo> debugInfos;
    AppDebugInfo info;
    debugInfos.push_back(info);

    EXPECT_NE(listener_, nullptr);
    EXPECT_CALL(*listener_, OnAppDebugStarted(_)).Times(1);
    manager_->StartDebug(debugInfos);
    EXPECT_FALSE(manager_->debugInfos_.empty());
}

/**
 * @tc.name: StopDebug_0100
 * @tc.desc: Start debug by AppDebugInfo, notify AppDebugListener.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugManagerTest, StopDebug_0100, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    EXPECT_TRUE(manager_->debugInfos_.empty());
    std::vector<AppDebugInfo> debugInfos;
    AppDebugInfo info;
    info.bundleName = DEBUG_START_NAME;
    info.isDebugStart = IS_DEBUG_START;
    info.pid = 10;
    info.uid = 12345;
    debugInfos.push_back(info);
    manager_->debugInfos_ = debugInfos;

    EXPECT_NE(listener_, nullptr);
    EXPECT_CALL(*listener_, OnAppDebugStoped(_)).Times(1);
    manager_->StopDebug(debugInfos);
    EXPECT_TRUE(manager_->debugInfos_.empty());
}

/**
 * @tc.name: IsAttachDebug_0100
 * @tc.desc: Given the bundleName, return true if not DebugStart, otherwise return false.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugManagerTest, IsAttachDebug_0100, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    AppDebugInfo debugStart_info;
    debugStart_info.bundleName = DEBUG_START_NAME;
    debugStart_info.isDebugStart = IS_DEBUG_START;
    manager_->debugInfos_.push_back(debugStart_info);

    AppDebugInfo noDebugStart_info;
    noDebugStart_info.bundleName = NO_DEBUG_START_NAME;
    noDebugStart_info.isDebugStart = NO_DEBUG_START;
    manager_->debugInfos_.push_back(noDebugStart_info);

    auto result = manager_->IsAttachDebug(DEBUG_START_NAME);
    EXPECT_FALSE(result);

    result = manager_->IsAttachDebug(NO_DEBUG_START_NAME);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: GetIncrementAppDebugInfo_0100
 * @tc.desc: Add new debug info into debugInfos, or update the isDebugStart flag.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugManagerTest, GetIncrementAppDebugInfo_0100, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    EXPECT_TRUE(manager_->debugInfos_.empty());

    std::vector<AppDebugInfo> debugInfos;
    std::vector<AppDebugInfo> increment;
    AppDebugInfo debugStart_info;
    debugStart_info.bundleName = DEBUG_START_NAME;
    debugStart_info.isDebugStart = IS_DEBUG_START;

    int pid = 10;
    int uid = 12345;
    AppDebugInfo noDebugStart_info;
    noDebugStart_info.bundleName = NO_DEBUG_START_NAME;
    noDebugStart_info.isDebugStart = NO_DEBUG_START;
    noDebugStart_info.pid = pid;
    noDebugStart_info.pid = uid;
    debugInfos.push_back(debugStart_info);
    debugInfos.push_back(noDebugStart_info);

    manager_->GetIncrementAppDebugInfos(debugInfos, increment);
    EXPECT_EQ(manager_->debugInfos_.size(), SIZE_TWO);

    increment.clear();
    debugInfos.clear();
    noDebugStart_info.isDebugStart = IS_DEBUG_START;
    debugInfos.push_back(noDebugStart_info);

    manager_->GetIncrementAppDebugInfos(debugInfos, increment);
    EXPECT_EQ(manager_->debugInfos_.size(), SIZE_TWO);
    EXPECT_TRUE(manager_->debugInfos_.at(1).isDebugStart);
}

/**
 * @tc.name: RemoveAppDebugInfo_0100
 * @tc.desc: Remove app debug info with bundleName, pid, uid and isDebugStart flag.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugManagerTest, RemoveAppDebugInfo_0100, TestSize.Level1)
{
    EXPECT_NE(manager_, nullptr);
    EXPECT_TRUE(manager_->debugInfos_.empty());

    std::vector<AppDebugInfo> debugInfos;
    int pid = 10;
    int uid = 12345;
    AppDebugInfo debugInfo;
    debugInfo.bundleName = DEBUG_START_NAME;
    debugInfo.pid = pid;
    debugInfo.uid = uid;
    debugInfo.isDebugStart = IS_DEBUG_START;

    manager_->debugInfos_.push_back(debugInfo);
    EXPECT_EQ(manager_->debugInfos_.size(), SIZE_ONE);

    EXPECT_CALL(*listener_, OnAppDebugStoped(_)).Times(1);
    manager_->RemoveAppDebugInfo(debugInfo);
    EXPECT_TRUE(manager_->debugInfos_.empty());
}
} // namespace AppExecFwk
} // namespace OHOS
