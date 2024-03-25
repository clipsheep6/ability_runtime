/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "extension_record_manager.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class ExtensionRecordManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ExtensionRecordManagerTest::SetUpTestCase() {}

void ExtensionRecordManagerTest::TearDownTestCase() {}

void ExtensionRecordManagerTest::SetUp() {}

void ExtensionRecordManagerTest::TearDown() {}

/**
 * @tc.name: RemoveExtensionRecord_0100
 * @tc.desc: RemoveExtensionRecord Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, RemoveExtensionRecord_0100, TestSize.Level1)
{
    auto extensionRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    constexpr int32_t recordId = 0;
    extensionRecordMgr->terminateRecords_.emplace(recordId, nullptr);
    extensionRecordMgr->extensionRecords_.emplace(recordId, nullptr);
    EXPECT_FALSE(extensionRecordMgr->terminateRecords_.empty());
    EXPECT_FALSE(extensionRecordMgr->extensionRecords_.empty());
    extensionRecordMgr->RemoveExtensionRecord(recordId);
    EXPECT_TRUE(extensionRecordMgr->terminateRecords_.empty());
    EXPECT_TRUE(extensionRecordMgr->extensionRecords_.empty());
}

/**
 * @tc.name: AddExtensionRecordToTerminatedList_0100
 * @tc.desc: AddExtensionRecordToTerminatedList Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, AddExtensionRecordToTerminatedList_0100, TestSize.Level1)
{
    auto extensionRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    constexpr int32_t recordId = 0;
    extensionRecordMgr->extensionRecords_.emplace(recordId, nullptr);
    EXPECT_TRUE(extensionRecordMgr->terminateRecords_.empty());
    EXPECT_FALSE(extensionRecordMgr->extensionRecords_.empty());
    extensionRecordMgr->AddExtensionRecordToTerminatedList(recordId);
    EXPECT_FALSE(extensionRecordMgr->terminateRecords_.empty());
    EXPECT_TRUE(extensionRecordMgr->extensionRecords_.empty());
    extensionRecordMgr->AddExtensionRecordToTerminatedList(recordId);
    EXPECT_TRUE(extensionRecordMgr->extensionRecords_.empty());
}

/**
 * @tc.name: LoadTimeout_0100
 * @tc.desc: LoadTimeout Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, LoadTimeout_0100, TestSize.Level1)
{
    auto extensionRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    constexpr int32_t recordId = 0;
    extensionRecordMgr->LoadTimeout(recordId);
    EXPECT_TRUE(extensionRecordMgr->extensionRecords_.empty());
    EXPECT_TRUE(extensionRecordMgr->terminateRecords_.empty());
}

/**
 * @tc.name: ForegroundTimeout_0100
 * @tc.desc: ForegroundTimeout Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, ForegroundTimeout_0100, TestSize.Level1)
{
    auto extensionRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    constexpr int32_t recordId = 0;
    extensionRecordMgr->ForegroundTimeout(recordId);
    EXPECT_TRUE(extensionRecordMgr->extensionRecords_.empty());
    EXPECT_TRUE(extensionRecordMgr->terminateRecords_.empty());
}

/**
 * @tc.name: BackgroundTimeout_0100
 * @tc.desc: BackgroundTimeout Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, BackgroundTimeout_0100, TestSize.Level1)
{
    auto extensionRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    constexpr int32_t recordId = 0;
    extensionRecordMgr->BackgroundTimeout(recordId);
    EXPECT_TRUE(extensionRecordMgr->extensionRecords_.empty());
    EXPECT_TRUE(extensionRecordMgr->terminateRecords_.empty());
}

/**
 * @tc.name: TerminateTimeout_0100
 * @tc.desc: TerminateTimeout Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, TerminateTimeout_0100, TestSize.Level1)
{
    auto extensionRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    constexpr int32_t recordId = 0;
    extensionRecordMgr->TerminateTimeout(recordId);
    EXPECT_TRUE(extensionRecordMgr->extensionRecords_.empty());
    EXPECT_TRUE(extensionRecordMgr->terminateRecords_.empty());
}
} // namespace AbilityRuntime
} // namespace OHOS
