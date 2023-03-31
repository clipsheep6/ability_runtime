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
#include "hilog_wrapper.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "js_runtime.h"
#include "mock_runtime.h"
#include "runtime.h"
#undef protected
#undef private

using namespace testing::ext;
namespace OHOS {
namespace AbilityRuntime {

class RuntimeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void RuntimeTest::SetUpTestCase(void)
{}

void RuntimeTest::TearDownTestCase(void)
{}

void RuntimeTest::SetUp()
{}

void RuntimeTest::TearDown()
{}

/**
 * @tc.number: Create_0100
 * @tc.name: Create
 * @tc.desc: Create Test, runtime is not nullptr.
 */
HWTEST_F(RuntimeTest, Create_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "RuntimeTest Create_0100 start";
    Runtime::Options options;
    std::unique_ptr<Runtime> runtime = std::make_unique<MockRuntime>();
    auto result = runtime->Create(options);
    EXPECT_TRUE(runtime != nullptr);
    EXPECT_TRUE(result != nullptr);
    GTEST_LOG_(INFO) << "RuntimeTest Create_0100 end";
}

/**
 * @tc.number: Create_0200
 * @tc.name: Create
 * @tc.desc: Create Test, runtime is not nullptr.
 */
HWTEST_F(RuntimeTest, Create_0200, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "RuntimeTest Create_0200 start";
    Runtime::Options options;
    options.lang = Runtime::Language::JS;
    options.preload = true;
    std::unique_ptr<Runtime> runtime = std::make_unique<MockRuntime>();
    auto result = runtime->Create(options);
    EXPECT_TRUE(runtime != nullptr);
    EXPECT_TRUE(result != nullptr);
    GTEST_LOG_(INFO) << "RuntimeTest Create_0200 end";
}

/**
 * @tc.number: SavePreloaded_0100
 * @tc.name: SavePreloaded
 * @tc.desc: Runtime test for SavePreloaded, runtime is not nullptr.
 */
HWTEST_F(RuntimeTest, SavePreloaded_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "RuntimeTest SavePreloaded_0100 start";
    Runtime::Options options;
    std::unique_ptr<Runtime> runtime = std::make_unique<MockRuntime>();
    auto instance = std::make_unique<MockRuntime>();
    runtime->SavePreloaded(std::move(instance));
    EXPECT_TRUE(runtime != nullptr);
    GTEST_LOG_(INFO) << "RuntimeTest SavePreloaded_0100 end";
}

/**
 * @tc.number: GetPreloaded_0100
 * @tc.name: GetPreloaded
 * @tc.desc: Runtime test for GetPreloaded, runtime is not nullptr.
 */
HWTEST_F(RuntimeTest, GetPreloaded_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "RuntimeTest GetPreloaded_0100 start";
    Runtime::Options options;
    std::unique_ptr<Runtime> runtime = std::make_unique<MockRuntime>();
    auto instance = std::make_unique<MockRuntime>();
    runtime->SavePreloaded(std::move(instance));
    EXPECT_TRUE(runtime->GetPreloaded() != nullptr);
    GTEST_LOG_(INFO) << "RuntimeTest GetPreloaded_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS