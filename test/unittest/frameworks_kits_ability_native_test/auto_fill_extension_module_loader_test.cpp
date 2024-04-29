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

#include <dlfcn.h>
#include <gtest/gtest.h>
#include "auto_fill_extension_module_loader.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace testing::ext;

class AutoFillExtensionModuleLoaderTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AutoFillExtensionModuleLoaderTest::SetUpTestCase(void)
{}

void AutoFillExtensionModuleLoaderTest::TearDownTestCase(void)
{}

void AutoFillExtensionModuleLoaderTest::SetUp(void)
{}

void AutoFillExtensionModuleLoaderTest::TearDown(void)
{}

/**
 * @tc.name: GetExtensionModule_0100
 * @tc.desc: Verify can load AutoFillExtensionModule successfully.
 * @tc.type: FUNC.
 */
HWTEST_F(AutoFillExtensionModuleLoaderTest, GetExtensionModule_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GetExtensionModule_0100 start";
    void* handle = dlopen("/system/lib/extensionability/libauto_fill_extension_module.z.so", RTLD_LAZY);
    if (handle != nullptr) {
        auto obj = reinterpret_cast<AutoFillExtensionModuleLoader*>(
            dlsym(handle, "OHOS_EXTENSION_GetExtensionModule"));
        EXPECT_TRUE(obj != nullptr);
    }
    dlclose(handle);
    GTEST_LOG_(INFO) << "GetExtensionModule_0100 end";
}

/**
 * @tc.name: Create_0100
 * @tc.desc: Verify can create AutoFillExtensionModule successfully.
 * @tc.type: FUNC
 */
HWTEST_F(AutoFillExtensionModuleLoaderTest, Create_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Create_0100 start";
    std::unique_ptr<Runtime> runtime;
    auto extension = AutoFillExtensionModuleLoader::GetInstance().Create(runtime);
    EXPECT_TRUE(extension != nullptr);
    GTEST_LOG_(INFO) << "Create_0100 end";
}

/**
 * @tc.name: GetParams_0100
 * @tc.desc: Verify can get params successfully.
 * @tc.type: FUNC
 */
HWTEST_F(AutoFillExtensionModuleLoaderTest, GetParams_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GetParams_0100 start";
    auto params = AutoFillExtensionModuleLoader::GetInstance().GetParams();

    std::string key = "type";
    auto finder = params.find(key);
    if (finder != params.end()) {
        EXPECT_STREQ(finder->second.c_str(), "501");
    }

    key = "name";
    auto iter = params.find(key);
    if (iter != params.end()) {
        EXPECT_STREQ(iter->second.c_str(), "AutoFillExtensionAbility");
    }
    GTEST_LOG_(INFO) << "GetParams_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
