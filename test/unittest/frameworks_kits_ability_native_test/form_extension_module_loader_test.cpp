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
#include "form_extension_module_loader.h"
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
#ifdef APP_USE_ARM
constexpr char FORM_EXTENSION_MODULE_LIB_PATH[] = "/system/lib/extensionability/libform_extension_module.z.so";
#else
constexpr char FORM_EXTENSION_MODULE_LIB_PATH[] = "/system/lib64/extensionability/libform_extension_module.z.so";
#endif

class FormExtensionModuleLoaderTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void FormExtensionModuleLoaderTest::SetUpTestCase(void)
{}

void FormExtensionModuleLoaderTest::TearDownTestCase(void)
{}

void FormExtensionModuleLoaderTest::SetUp()
{}

void FormExtensionModuleLoaderTest::TearDown()
{}

/**
 * @tc.name: FormExtensionModuleLoader_0100
 * @tc.desc: GetInstance
 * @tc.type: FUNC
 * @tc.require: issueI5Z8AZ
 */
HWTEST_F(FormExtensionModuleLoaderTest, FormExtensionModuleLoader_0100, TestSize.Level1)
{
    HILOG_INFO("start");
    void *handle = dlopen(FORM_EXTENSION_MODULE_LIB_PATH, RTLD_LAZY);
    if (handle != nullptr) {
        auto object = reinterpret_cast<FormExtensionModuleLoader*>(
            dlsym(handle, "OHOS_EXTENSION_GetExtensionModule"));
        EXPECT_NE(object, nullptr);
        dlclose(handle);
    }
    HILOG_INFO("end");
}

/**
 * @tc.name: FormExtensionModuleLoader_0200
 * @tc.desc: Create
 * @tc.type: FUNC
 * @tc.require: issueI5Z8AZ
 */
HWTEST_F(FormExtensionModuleLoaderTest, FormExtensionModuleLoader_0200, TestSize.Level1)
{
    HILOG_INFO("start");
    std::unique_ptr<Runtime> runtime;
    auto extension = FormExtensionModuleLoader::GetInstance().Create(runtime);
    EXPECT_NE(extension, nullptr);
    HILOG_INFO("end");
}

/**
 * @tc.name: FormExtensionModuleLoader_0300
 * @tc.desc: GetParams
 * @tc.type: FUNC
 * @tc.require: issueI5Z8AZ
 */
HWTEST_F(FormExtensionModuleLoaderTest, FormExtensionModuleLoader_0300, TestSize.Level1)
{
    HILOG_INFO("start");
    auto params = FormExtensionModuleLoader::GetInstance().GetParams();

    std::string key = "type";
    auto finder = params.find(key);
    if (finder != params.end()) {
        EXPECT_STREQ(finder->second.c_str(), "0");
    }

    key = "name";
    auto iter = params.find(key);
    if (iter != params.end()) {
        EXPECT_STREQ(iter->second.c_str(), "FormExtension");
    }
    HILOG_INFO("end");
}
} // namespace AbilityRuntime
} // namespace OHOS
