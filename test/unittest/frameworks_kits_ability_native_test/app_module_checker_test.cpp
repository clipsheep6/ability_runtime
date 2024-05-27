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
#include <memory>

#include "ability_handler.h"
#include "app_module_checker.h"
#include "context_deal.h"
#include "hilog_wrapper.h"
#include "locale_config.h"
#include "ohos_application.h"
#include "process_options.h"
#include "session_info.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;
namespace {
static const int32_t EXTENSION_TYPE = 10;
static const int32_t EXTENSION_TYPE1 = 2;
}
class AppModuleCheckTest : public testing::Test {
public:
    AppModuleCheckTest() : appModuleChecker_(nullptr) {}
    ~AppModuleCheckTest() {}
    std::shared_ptr<class AppModuleChecker> appModuleChecker_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppModuleCheckTest::SetUpTestCase(void) {}

void AppModuleCheckTest::TearDownTestCase(void) {}

void AppModuleCheckTest::SetUp(void) {}

void AppModuleCheckTest::TearDown(void) {}

/*
 * Feature: DiskCheckOnly_001
 * Function: DiskCheckOnly
 */
HWTEST_F(AppModuleCheckTest, DiskCheckOnly_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskCheckOnly_001 start";
    std::unordered_map<int32_t, std::unordered_set<std::string>> blocklist = {
        {1, {"module1"}},
        {2, {"module2"}},
        {3, {"module3"}}
    };
    appModuleChecker_ = std::make_shared<AppModuleChecker>(EXTENSION_TYPE, std::move(blocklist));
    bool ret = appModuleChecker_->DiskCheckOnly();
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "DiskCheckOnly_001 end";
}

/*
 * Feature: CheckModuleLoadable_001
 * Function: CheckModuleLoadable
 */
HWTEST_F(AppModuleCheckTest, CheckModuleLoadable_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckModuleLoadable_001 start";
    std::unordered_map<int32_t, std::unordered_set<std::string>> blocklist = {
        {1, {"module1"}},
        {2, {"module2"}},
        {3, {"module3"}}
    };
    appModuleChecker_ = std::make_shared<AppModuleChecker>(EXTENSION_TYPE, std::move(blocklist));
    std::unique_ptr<ApiAllowListChecker> apiAllowListChecker(nullptr);
    bool ret = appModuleChecker_->CheckModuleLoadable("module4", apiAllowListChecker);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "CheckModuleLoadable_001 end";
}

/*
 * Feature: CheckModuleLoadable_002
 * Function: CheckModuleLoadable
 */
HWTEST_F(AppModuleCheckTest, CheckModuleLoadable_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckModuleLoadable_002 start";
    std::unordered_map<int32_t, std::unordered_set<std::string>> blocklist = {
        {1, {"module1"}},
        {2, {"module2"}},
        {3, {"module3"}}
    };
    appModuleChecker_ = std::make_shared<AppModuleChecker>(EXTENSION_TYPE1, std::move(blocklist));
    std::unique_ptr<ApiAllowListChecker> apiAllowListChecker(nullptr);
    bool ret = appModuleChecker_->CheckModuleLoadable("module4", apiAllowListChecker);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "CheckModuleLoadable_002 end";
}

/*
 * Feature: CheckModuleLoadable_003
 * Function: CheckModuleLoadable
 */
HWTEST_F(AppModuleCheckTest, CheckModuleLoadable_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckModuleLoadable_003 start";
    std::unordered_map<int32_t, std::unordered_set<std::string>> blocklist = {
        {1, {"module1"}},
        {2, {"module2"}},
        {3, {"module3"}}
    };
    appModuleChecker_ = std::make_shared<AppModuleChecker>(EXTENSION_TYPE1, std::move(blocklist));
    std::unique_ptr<ApiAllowListChecker> apiAllowListChecker(nullptr);
    bool ret = appModuleChecker_->CheckModuleLoadable("module2", apiAllowListChecker);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "CheckModuleLoadable_003 end";
}
} // namespace AppExecFwk
} // namespace OHOS
