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
#include "photo_editor_extension_module_loader.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace testing::ext;

class PhotoEditorExtensionModuleLoaderTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PhotoEditorExtensionModuleLoaderTest::SetUpTestCase(void)
{}

void PhotoEditorExtensionModuleLoaderTest::TearDownTestCase(void)
{}

void PhotoEditorExtensionModuleLoaderTest::SetUp(void)
{}

void PhotoEditorExtensionModuleLoaderTest::TearDown(void)
{}

/**
 * @tc.number: PhotoEditorExtensionModuleLoader_0100
 * @tc.name: OHOS_EXTENSION_GetExtensionModule
 * @tc.desc: Verify OHOS_EXTENSION_GetExtensionModule succeeded.
 */
HWTEST_F(PhotoEditorExtensionModuleLoaderTest, PhotoEditorExtensionModuleLoader_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "PhotoEditorExtensionModuleLoader_0100 start";
    void *handle = dlopen("/system/lib/extensionability/libphoto_editor_extension_module.z.so", RTLD_LAZY);
    if (handle != nullptr) {
        auto obj =
            reinterpret_cast<PhotoEditorExtensionModuleLoader *>(dlsym(handle, "OHOS_EXTENSION_GetExtensionModule"));
        EXPECT_TRUE(obj != nullptr);
    }
    dlclose(handle);
    GTEST_LOG_(INFO) << "PhotoEditorExtensionModuleLoader_0100 end";
}

/**
 * @tc.number: PhotoEditorExtensionModuleLoader_0200
 * @tc.name: Create
 * @tc.desc: Verify Create succeeded.
 */
HWTEST_F(PhotoEditorExtensionModuleLoaderTest, PhotoEditorExtensionModuleLoader_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "PhotoEditorExtensionModuleLoader_0200 start";
    std::unique_ptr<Runtime> runtime;
    auto extension = PhotoEditorExtensionModuleLoader::GetInstance().Create(runtime);
    EXPECT_TRUE(extension != nullptr);
    GTEST_LOG_(INFO) << "PhotoEditorExtensionModuleLoader_0200 end";
}

/**
 * @tc.number: PhotoEditorExtensionModuleLoader_0300
 * @tc.name: GetParams
 * @tc.desc: Verify GetParams succeeded.
 */
HWTEST_F(PhotoEditorExtensionModuleLoaderTest, PhotoEditorExtensionModuleLoader_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "PhotoEditorExtensionModuleLoader_0300 start";
    auto params = PhotoEditorExtensionModuleLoader::GetInstance().GetParams();

    std::string key = "type";
    auto finder = params.find(key);
    if (finder != params.end()) {
        EXPECT_STREQ(finder->second.c_str(), "266");
    }

    key = "name";
    auto iter = params.find(key);
    if (iter != params.end()) {
        EXPECT_STREQ(iter->second.c_str(), "PhotoEditorExtensionAbility");
    }
    GTEST_LOG_(INFO) << "PhotoEditorExtensionModuleLoader_0300 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
