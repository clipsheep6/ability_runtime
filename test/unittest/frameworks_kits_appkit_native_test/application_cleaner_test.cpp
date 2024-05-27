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

#include <algorithm>
#include <gtest/gtest.h>

#define private public
#define protected public
#include "application_cleaner.h"
#include "context_impl.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AppExecFwk {
class ApplicationCleanerTest : public testing::Test {
public:
    ApplicationCleanerTest()
    {}
    ~ApplicationCleanerTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ApplicationCleanerTest::SetUpTestCase(void)
{}

void ApplicationCleanerTest::TearDownTestCase(void)
{}

void ApplicationCleanerTest::SetUp(void)
{}

void ApplicationCleanerTest::TearDown(void)
{}

/**
 * @tc.number: ClearTempData_0100
 * @tc.name: ClearTempData
 * @tc.desc: Test whether ClearTempData and are called normally.
 */
HWTEST_F(ApplicationCleanerTest, ClearTempData_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ClearTempData_0100 start";
    auto cleaner = ApplicationCleaner::GetInstance();
    EXPECT_NE(cleaner, nullptr);
    cleaner->ClearTempData();

    GTEST_LOG_(INFO) << "ClearTempData_0100 end";
}

/**
 * @tc.number: RenameTempData_0100
 * @tc.name: RenameTempData
 * @tc.desc: Test whether RenameTempData and are called normally.
 */
HWTEST_F(ApplicationCleanerTest, RenameTempData_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "RenameTempData_0100 start";
    auto cleaner = ApplicationCleaner::GetInstance();

    std::vector<std::string> tempDir1{};
    cleaner->context_ = AbilityRuntime::ApplicationContext::GetInstance();
    cleaner->context_->GetAllTempDir(tempDir1);
    cleaner->RenameTempData();

    std::vector<std::string> tempDir2{};
    cleaner->context_->GetAllTempDir(tempDir2);
    bool res = std::equal(tempDir1.begin(), tempDir1.end(), tempDir2.begin());
    EXPECT_EQ(res, true);
    GTEST_LOG_(INFO) << "RenameTempData_0100 end";
}

/**
 * @tc.number: GetObsoleteBundleTempPath_0100
 * @tc.name: GetObsoleteBundleTempPath
 * @tc.desc: Test whether GetObsoleteBundleTempPath and are called normally.
 */
HWTEST_F(ApplicationCleanerTest, GetObsoleteBundleTempPath_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GetObsoleteBundleTempPath_0100 start";
    auto cleaner = ApplicationCleaner::GetInstance();
    std::string path = "/data/app/base";
    std::vector<std::string> rootPath {path};
    std::vector<std::string> tempPath;
    int res = cleaner->GetObsoleteBundleTempPath(rootPath, tempPath);
    EXPECT_EQ(res, 0);
    GTEST_LOG_(INFO) << "GetObsoleteBundleTempPath_0100 end";
}

/**
 * @tc.number: RemoveDir_0100
 * @tc.name: RemoveDir
 * @tc.desc: Test whether RemoveDir and are called normally.
 */
HWTEST_F(ApplicationCleanerTest, RemoveDir_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "RemoveDir_0100 start";
    auto cleaner = ApplicationCleaner::GetInstance();
    std::string currentPath = "/data/app/base";
    bool res = cleaner->RemoveDir(currentPath);
    EXPECT_EQ(res, false);
    GTEST_LOG_(INFO) << "RemoveDir_0100 end";
}
}
}