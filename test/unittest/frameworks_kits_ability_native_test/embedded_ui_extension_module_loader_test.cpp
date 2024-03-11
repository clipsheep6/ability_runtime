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

#include <dlfcn.h>
#include <gtest/gtest.h>
#include "embedded_ui_extension_module_loader.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace testing::ext;

class EmbeddedUIExtensionModuleLoaderTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void EmbeddedUIExtensionModuleLoaderTest::SetUpTestCase(void)
{}

void EmbeddedUIExtensionModuleLoaderTest::TearDownTestCase(void)
{}

void EmbeddedUIExtensionModuleLoaderTest::SetUp(void)
{}

void EmbeddedUIExtensionModuleLoaderTest::TearDown(void)
{}

/**
 * @tc.number: EmbeddedUIExtensionModuleLoader_0100
 * @tc.name: OHOS_EXTENSION_GetExtensionModule
 * @tc.desc: Verify OHOS_EXTENSION_GetExtensionModule succeeded.
 */
HWTEST_F(EmbeddedUIExtensionModuleLoaderTest, EmbeddedUIExtensionModuleLoader_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "EmbeddedUIExtensionModuleLoader_0100 start";
    void* handle = dlopen("/system/lib/extensionability/libembedded_ui_extension_module.z.so", RTLD_LAZY);
    if (handle != nullptr) {
        auto obj = reinterpret_cast<EmbeddedUIExtensionModuleLoader*>(
            dlsym(handle, "OHOS_EXTENSION_GetExtensionModule"));
        EXPECT_TRUE(obj != nullptr);
    }
    dlclose(handle);
    GTEST_LOG_(INFO) << "EmbeddedUIExtensionModuleLoader_0100 end";
}

/**
 * @tc.number: EmbeddedUIExtensionModuleLoader_0200
 * @tc.name: Create
 * @tc.desc: Verify Create succeeded.
 */
HWTEST_F(EmbeddedUIExtensionModuleLoaderTest, EmbeddedUIExtensionModuleLoader_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "EmbeddedUIExtensionModuleLoader_0200 start";
    std::unique_ptr<Runtime> runtime;
    auto extension = EmbeddedUIExtensionModuleLoader::GetInstance().Create(runtime);
    EXPECT_TRUE(extension != nullptr);
    GTEST_LOG_(INFO) << "EmbeddedUIExtensionModuleLoader_0200 end";
}

/**
 * @tc.number: EmbeddedUIExtensionModuleLoader_0300
 * @tc.name: GetParams
 * @tc.desc: Verify GetParams succeeded.
 */
HWTEST_F(EmbeddedUIExtensionModuleLoaderTest, EmbeddedUIExtensionModuleLoader_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "EmbeddedUIExtensionModuleLoader_0300 start";
    auto params = EmbeddedUIExtensionModuleLoader::GetInstance().GetParams();

    std::string key = "type";
    auto finder = params.find(key);
    if (finder != params.end()) {
        EXPECT_STREQ(finder->second.c_str(), "21");
    }

    key = "name";
    auto iter = params.find(key);
    if (iter != params.end()) {
        EXPECT_STREQ(iter->second.c_str(), "EmbeddedUIExtensionAbility");
    }
    GTEST_LOG_(INFO) << "EmbeddedUIExtensionModuleLoader_0300 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
