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
#include "hilog_wrapper.h"
#include "static_subscriber_extension_module_loader.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
#ifdef APP_USE_ARM
constexpr char STATIC_SUBSCRIBER_EXTENSION_MODULE_LIB_PATH[] =
    "/system/lib/extensionability/libstatic_subscriber_extension_module.z.so";
#else
constexpr char STATIC_SUBSCRIBER_EXTENSION_MODULE_LIB_PATH[] =
    "/system/lib64/extensionability/libstatic_subscriber_extension_module.z.so";
#endif

class StaticSubscriberExtensionModuleLoaderTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void StaticSubscriberExtensionModuleLoaderTest::SetUpTestCase(void)
{}

void StaticSubscriberExtensionModuleLoaderTest::TearDownTestCase(void)
{}

void StaticSubscriberExtensionModuleLoaderTest::SetUp()
{}

void StaticSubscriberExtensionModuleLoaderTest::TearDown()
{}

/**
 * @tc.name: StaticSubscriberExtensionModuleLoader_0100
 * @tc.desc: GetInstance
 * @tc.type: FUNC
 * @tc.require: issueI5Z8AZ
 */
HWTEST_F(StaticSubscriberExtensionModuleLoaderTest, StaticSubscriberExtensionModuleLoader_0100, TestSize.Level1)
{
    HILOG_INFO("start");
    void *handle = dlopen(STATIC_SUBSCRIBER_EXTENSION_MODULE_LIB_PATH, RTLD_LAZY);
    if (handle != nullptr) {
        auto object = reinterpret_cast<StaticSubscriberExtensionModuleLoader*>(
            dlsym(handle, "OHOS_EXTENSION_GetExtensionModule"));
        EXPECT_NE(object, nullptr);
        dlclose(handle);
    }
    HILOG_INFO("end");
}

/**
 * @tc.name: StaticSubscriberExtensionModuleLoader_0200
 * @tc.desc: Create
 * @tc.type: FUNC
 * @tc.require: issueI5Z8AZ
 */
HWTEST_F(StaticSubscriberExtensionModuleLoaderTest, StaticSubscriberExtensionModuleLoader_0200, TestSize.Level1)
{
    HILOG_INFO("start");
    std::unique_ptr<Runtime> runtime;
    auto extension = StaticSubscriberExtensionModuleLoader::GetInstance().Create(runtime);
    EXPECT_NE(extension, nullptr);
    HILOG_INFO("end");
}

/**
 * @tc.name: StaticSubscriberExtensionModuleLoader_0300
 * @tc.desc: GetParams
 * @tc.type: FUNC
 * @tc.require: issueI5Z8AZ
 */
HWTEST_F(StaticSubscriberExtensionModuleLoaderTest, StaticSubscriberExtensionModuleLoader_0300, TestSize.Level1)
{
    HILOG_INFO("start");
    auto params = StaticSubscriberExtensionModuleLoader::GetInstance().GetParams();

    std::string key = "type";
    auto finder = params.find(key);
    if (finder != params.end()) {
        EXPECT_STREQ(finder->second.c_str(), "7");
    }

    key = "name";
    auto iter = params.find(key);
    if (iter != params.end()) {
        EXPECT_STREQ(iter->second.c_str(), "StaticSubscriberExtension");
    }
    HILOG_INFO("end");
}
} // namespace AbilityRuntime
} // namespace OHOS
