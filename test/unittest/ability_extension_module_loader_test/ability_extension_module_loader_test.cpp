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
#include "request_info.h"

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

/**
 * @tc.number: RequestInfo_GetToken_0100
 * @tc.name: GetToken
 * @tc.desc: GetToken
 */
HWTEST_F(AbilityExtensionModuleLoaderTest, RequestInfo_GetToken_0100, TestSize.Level1)
{
    HILOG_INFO("RequestInfo_GetToken_0100 start");
    sptr<IRemoteObject> token = nullptr;
    int32_t left = 0, top = 0, width = 0, height = 0;
    auto requestInfo = std::make_shared<RequestInfo>(token, left, top, width, height);
    EXPECT_EQ(requestInfo->GetToken(), nullptr);
    HILOG_INFO("RequestInfo_GetToken_0100 end");
}

/**
 * @tc.number: RequestInfo_CreateJsWindowRect_0100
 * @tc.name: CreateJsWindowRect
 * @tc.desc: CreateJsWindowRect
 */
HWTEST_F(AbilityExtensionModuleLoaderTest, RequestInfo_CreateJsWindowRect_0100, TestSize.Level1)
{
    HILOG_INFO("RequestInfo_CreateJsWindowRect_0100 start");
    sptr<IRemoteObject> token = nullptr;
    int32_t left = 0, top = 0, width = 0, height = 0;
    auto requestInfo = std::make_shared<RequestInfo>(token, left, top, width, height);
    EXPECT_EQ(requestInfo->CreateJsWindowRect(nullptr, left, top, width, height), nullptr);
    HILOG_INFO("RequestInfo_CreateJsWindowRect_0100 end");
}

/**
 * @tc.number: RequestInfo_WrapRequestInfo_0100
 * @tc.name: WrapRequestInfo
 * @tc.desc: WrapRequestInfo
 */
HWTEST_F(AbilityExtensionModuleLoaderTest, RequestInfo_WrapRequestInfo_0100, TestSize.Level1)
{
    HILOG_INFO("RequestInfo_WrapRequestInfo_0100 start");
    sptr<IRemoteObject> token = nullptr;
    int32_t left = 0, top = 0, width = 0, height = 0;
    auto requestInfo = std::make_shared<RequestInfo>(token, left, top, width, height);
    EXPECT_EQ(requestInfo->WrapRequestInfo(nullptr, nullptr), nullptr);
    HILOG_INFO("RequestInfo_WrapRequestInfo_0100 end");
}

/**
 * @tc.number: RequestInfo_UnwrapRequestInfo_0100
 * @tc.name: UnwrapRequestInfo
 * @tc.desc: UnwrapRequestInfo
 */
HWTEST_F(AbilityExtensionModuleLoaderTest, RequestInfo_UnwrapRequestInfo_0100, TestSize.Level1)
{
    HILOG_INFO("RequestInfo_UnwrapRequestInfo_0100 start");
    sptr<IRemoteObject> token = nullptr;
    int32_t left = 0, top = 0, width = 0, height = 0;
    auto requestInfo = std::make_shared<RequestInfo>(token, left, top, width, height);
    EXPECT_EQ(requestInfo->UnwrapRequestInfo(nullptr, nullptr), nullptr);
    HILOG_INFO("RequestInfo_UnwrapRequestInfo_0100 end");
}
}  // namespace AbilityRuntime
}  // namespace OHOS
