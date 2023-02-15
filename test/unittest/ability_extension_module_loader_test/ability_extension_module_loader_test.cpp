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

#include "extension_module_loader.h"
#include "hilog_wrapper.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class AbilityExtensionModuleLoaderTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityExtensionModuleLoaderTest::SetUpTestCase(void)
{}

void AbilityExtensionModuleLoaderTest::TearDownTestCase(void)
{}

void AbilityExtensionModuleLoaderTest::SetUp()
{}

void AbilityExtensionModuleLoaderTest::TearDown()
{}

/**
 * @tc.name: ExtensionModuleLoader_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityExtensionModuleLoaderTest, ExtensionModuleLoader_0100, TestSize.Level1)
{
    HILOG_INFO("ExtensionModuleLoader start");

    Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    Extension* extension = ExtensionModuleLoader::GetLoader(nullptr).Create(runtime);
    EXPECT_EQ(extension, nullptr);

    HILOG_INFO("ExtensionModuleLoader end");
}

/**
 * @tc.name: GetParams_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityExtensionModuleLoaderTest, GetParams_0100, TestSize.Level1)
{
    HILOG_INFO("ExtensionModuleLoader start");

    auto params = ExtensionModuleLoader::GetLoader(nullptr).GetParams();
    bool ret = params.empty();
    EXPECT_TRUE(ret);

    HILOG_INFO("ExtensionModuleLoader end");
}

/**
 * @tc.number: ExtensionModuleLoader_GetExtensionModuleLoader_0100
 * @tc.name: GetExtensionModuleLoader
 * @tc.desc: call GetExtensionModuleLoader with open extension failed
 */
HWTEST_F(AbilityExtensionModuleLoaderTest, ExtensionModuleLoader_GetExtensionModuleLoader_0100, TestSize.Level1)
{
    HILOG_INFO("ExtensionModuleLoader_GetExtensionModuleLoader_0100 start");
    Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto result = ExtensionModuleLoader::GetLoader("system").Create(runtime);
    EXPECT_TRUE(result == nullptr);
    HILOG_INFO("ExtensionModuleLoader_GetExtensionModuleLoader_0100 end");
}

/**
 * @tc.number: ExtensionModuleLoader_GetExtensionModuleLoader_0200
 * @tc.name: GetExtensionModuleLoader
 * @tc.desc: call GetExtensionModuleLoader with get extension symbol failed
 */
HWTEST_F(AbilityExtensionModuleLoaderTest, ExtensionModuleLoader_GetExtensionModuleLoader_0200, TestSize.Level1)
{
    HILOG_INFO("ExtensionModuleLoader_GetExtensionModuleLoader_0200 start");
    Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto result = ExtensionModuleLoader::GetLoader("/system/lib/libc++.so").Create(runtime);
    EXPECT_TRUE(result == nullptr);
    HILOG_INFO("ExtensionModuleLoader_GetExtensionModuleLoader_0200 end");
}
}  // namespace AbilityRuntime
}  // namespace OHOS
