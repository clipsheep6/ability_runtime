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
#include "extension_config_mgr.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
    constexpr int32_t DEFAULT_BLOCKLIST_EXTENSION_NUM = 12;
    constexpr int32_t EXTENSION_TYPE_FORM = 0;
    constexpr int32_t EXTENSION_TYPE_WORK_SCHEDULER = 1;
    constexpr int32_t EXTENSION_TYPE_INPUTMETHOD = 2;
    constexpr int32_t EXTENSION_TYPE_SERVICE = 3;
    constexpr int32_t EXTENSION_TYPE_ACCESSIBILITY = 4;
    constexpr int32_t EXTENSION_TYPE_DATASHARE = 5;
    constexpr int32_t EXTENSION_TYPE_STATICSUBSCRIBER = 7;
    constexpr int32_t EXTENSION_TYPE_WALLPAPER = 8;
    constexpr int32_t EXTENSION_TYPE_BACKUP = 9;
    constexpr int32_t EXTENSION_TYPE_WINDOW = 10;
    constexpr int32_t EXTENSION_TYPE_ENTERPRISE_ADMIN = 11;
    constexpr int32_t EXTENSION_TYPE_FILE_ACCESS = 12;
    constexpr char BLOCK_LIST_ITEM_SERVICE_EXTENSION[] = "ServiceExtension";
    constexpr char BLOCK_LIST_ITEM_FORM_EXTENSION[] = "FormExtension";
    constexpr char BLOCK_LIST_ITEM_FILE_ACCESS_EXTENSION[] = "FileAccessExtension";
    constexpr char BLOCK_LIST_ITEM_BACKUP_EXTENSION[] = "BackupExtension";
    constexpr char BLOCK_LIST_ITEM_ENTERPRISE_ADMIN_EXTENSION[] = "EnterpriseAdminExtension";
    constexpr char BLOCK_LIST_ITEM_WINDOW_EXTENSION_EXTENSION[] = "WindowExtensionExtension";
    constexpr char BLOCK_LIST_ITEM_WALLPAPER_EXTENSION[] = "WallpaperExtension";
    constexpr char BLOCK_LIST_ITEM_STATIC_SUBSCRIBER_EXTENSION[] = "StaticSubscriberExtension";
    constexpr char BLOCK_LIST_ITEM_ACCESSIBILITY_EXTENSION[] = "AccessibilityExtension";
    constexpr char BLOCK_LIST_ITEM_INPUT_METHOD_EXTENSION[] = "InputMethodExtension";
    constexpr char BLOCK_LIST_ITEM_WORK_SCHEDULER_EXTENSION[] = "WorkSchedulerExtension";
    constexpr char BLOCK_LIST_ITEM_DATA_SHARE_EXTENSION[] = "DataShareExtension";
    constexpr char INVAILD_BLOCK_LIST_ITEM[] = "InvaildExtension";
}

class ExtensionConfigMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ExtensionConfigMgrTest::SetUpTestCase()
{}

void ExtensionConfigMgrTest::TearDownTestCase()
{}

void ExtensionConfigMgrTest::SetUp()
{}

void ExtensionConfigMgrTest::TearDown()
{}

/**
 * @tc.name: Init_0100
 * @tc.desc: Init Test
 * @tc.type: FUNC
 * @tc.require: issueI581T3
 */
HWTEST_F(ExtensionConfigMgrTest, Init_0100, TestSize.Level0)
{
    ExtensionConfigMgr mgr;
    mgr.Init();
    EXPECT_EQ(static_cast<int32_t>(mgr.blocklistConfig_.size()), DEFAULT_BLOCKLIST_EXTENSION_NUM);
    bool result = false;
    result = (mgr.blocklistConfig_.find(BLOCK_LIST_ITEM_SERVICE_EXTENSION) != mgr.blocklistConfig_.end());
    EXPECT_TRUE(result);
    result = (mgr.blocklistConfig_.find(BLOCK_LIST_ITEM_FORM_EXTENSION) != mgr.blocklistConfig_.end());
    EXPECT_TRUE(result);
    result = (mgr.blocklistConfig_.find(BLOCK_LIST_ITEM_FILE_ACCESS_EXTENSION) != mgr.blocklistConfig_.end());
    EXPECT_TRUE(result);
    result = (mgr.blocklistConfig_.find(BLOCK_LIST_ITEM_BACKUP_EXTENSION) != mgr.blocklistConfig_.end());
    EXPECT_TRUE(result);
    result = (mgr.blocklistConfig_.find(BLOCK_LIST_ITEM_ENTERPRISE_ADMIN_EXTENSION) != mgr.blocklistConfig_.end());
    EXPECT_TRUE(result);
    result = (mgr.blocklistConfig_.find(BLOCK_LIST_ITEM_WINDOW_EXTENSION_EXTENSION) != mgr.blocklistConfig_.end());
    EXPECT_TRUE(result);
    result = (mgr.blocklistConfig_.find(BLOCK_LIST_ITEM_WALLPAPER_EXTENSION) != mgr.blocklistConfig_.end());
    EXPECT_TRUE(result);
    result = (mgr.blocklistConfig_.find(BLOCK_LIST_ITEM_STATIC_SUBSCRIBER_EXTENSION) != mgr.blocklistConfig_.end());
    EXPECT_TRUE(result);
    result = (mgr.blocklistConfig_.find(BLOCK_LIST_ITEM_ACCESSIBILITY_EXTENSION) != mgr.blocklistConfig_.end());
    EXPECT_TRUE(result);
    result = (mgr.blocklistConfig_.find(BLOCK_LIST_ITEM_INPUT_METHOD_EXTENSION) != mgr.blocklistConfig_.end());
    EXPECT_TRUE(result);
    result = (mgr.blocklistConfig_.find(BLOCK_LIST_ITEM_WORK_SCHEDULER_EXTENSION) != mgr.blocklistConfig_.end());
    EXPECT_TRUE(result);
    result = (mgr.blocklistConfig_.find(BLOCK_LIST_ITEM_DATA_SHARE_EXTENSION) != mgr.blocklistConfig_.end());
    EXPECT_TRUE(result);
}

/**
 * @tc.name: AddBlockListItem_0100
 * @tc.desc: AddBlockListItem Test
 * @tc.type: FUNC
 * @tc.require: issueI581T3
 */
HWTEST_F(ExtensionConfigMgrTest, AddBlockListItem_0100, TestSize.Level0)
{
    ExtensionConfigMgr mgr;
    mgr.Init();
    bool result = false;
    mgr.AddBlockListItem(BLOCK_LIST_ITEM_FORM_EXTENSION, EXTENSION_TYPE_FORM);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_FORM) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);
    mgr.AddBlockListItem(BLOCK_LIST_ITEM_WORK_SCHEDULER_EXTENSION, EXTENSION_TYPE_WORK_SCHEDULER);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_WORK_SCHEDULER) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);
    mgr.AddBlockListItem(BLOCK_LIST_ITEM_INPUT_METHOD_EXTENSION, EXTENSION_TYPE_INPUTMETHOD);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_INPUTMETHOD) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);
    mgr.AddBlockListItem(BLOCK_LIST_ITEM_SERVICE_EXTENSION, EXTENSION_TYPE_SERVICE);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_SERVICE) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);
    mgr.AddBlockListItem(BLOCK_LIST_ITEM_ACCESSIBILITY_EXTENSION, EXTENSION_TYPE_ACCESSIBILITY);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_ACCESSIBILITY) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);
    mgr.AddBlockListItem(BLOCK_LIST_ITEM_DATA_SHARE_EXTENSION, EXTENSION_TYPE_DATASHARE);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_DATASHARE) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);
    mgr.AddBlockListItem(BLOCK_LIST_ITEM_STATIC_SUBSCRIBER_EXTENSION, EXTENSION_TYPE_STATICSUBSCRIBER);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_STATICSUBSCRIBER) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);
    mgr.AddBlockListItem(BLOCK_LIST_ITEM_WALLPAPER_EXTENSION, EXTENSION_TYPE_WALLPAPER);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_WALLPAPER) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);
    mgr.AddBlockListItem(BLOCK_LIST_ITEM_BACKUP_EXTENSION, EXTENSION_TYPE_BACKUP);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_BACKUP) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);
    mgr.AddBlockListItem(BLOCK_LIST_ITEM_WINDOW_EXTENSION_EXTENSION, EXTENSION_TYPE_WINDOW);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_WINDOW) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);
    mgr.AddBlockListItem(BLOCK_LIST_ITEM_ENTERPRISE_ADMIN_EXTENSION, EXTENSION_TYPE_ENTERPRISE_ADMIN);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_ENTERPRISE_ADMIN) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);
    mgr.AddBlockListItem(BLOCK_LIST_ITEM_FILE_ACCESS_EXTENSION, EXTENSION_TYPE_FILE_ACCESS);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_FILE_ACCESS) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);   
}

/**
 * @tc.name: AddBlockListItem_0200
 * @tc.desc: AddBlockListItem Test
 * @tc.type: FUNC
 * @tc.require: issueI5825N
 */
HWTEST_F(ExtensionConfigMgrTest, AddBlockListItem_0200, TestSize.Level0)
{
    ExtensionConfigMgr mgr;
    mgr.Init();
    mgr.AddBlockListItem(INVAILD_BLOCK_LIST_ITEM, EXTENSION_TYPE_FORM);
    bool result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_FORM) != mgr.extensionBlocklist_.end());
    EXPECT_FALSE(result);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
