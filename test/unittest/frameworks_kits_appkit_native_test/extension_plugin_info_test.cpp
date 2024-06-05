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
#include "extension_plugin_info.h"
#include "file_ex.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AppExecFwk {
class ExtensionPluginInfoTest : public testing::Test {
public:
    ExtensionPluginInfoTest()
    {}
    ~ExtensionPluginInfoTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ExtensionPluginInfoTest::SetUpTestCase(void)
{}

void ExtensionPluginInfoTest::TearDownTestCase(void)
{}

void ExtensionPluginInfoTest::SetUp(void)
{}

void ExtensionPluginInfoTest::TearDown(void)
{}

/**
 * @tc.number: Preload_0100
 * @tc.name: Preload
 * @tc.desc: Test whether Preload and are called normally.
 */
HWTEST_F(ExtensionPluginInfoTest, Preload_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionPluginInfoTest Preload_0100 start";
    ExtensionPluginInfo::GetInstance().Preload();

    GTEST_LOG_(INFO) << "ExtensionPluginInfoTest Preload_0100 end";
}

/**
 * @tc.number: GetExtensionPlugins_0100
 * @tc.name: GetExtensionPlugins
 * @tc.desc: Test whether GetExtensionPlugins and are called normally.
 */
HWTEST_F(ExtensionPluginInfoTest, GetExtensionPlugins_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionPluginInfoTest GetExtensionPlugins_0100 start";
    ExtensionPluginInfo::GetInstance().extensionPlugins_.clear();
    auto extensionPlugins = ExtensionPluginInfo::GetInstance().GetExtensionPlugins();
    EXPECT_EQ(extensionPlugins.size(), 0);

    GTEST_LOG_(INFO) << "ExtensionPluginInfoTest GetExtensionPlugins_0100 end";
}

/**
 * @tc.number: ScanExtensions_0100
 * @tc.name: ScanExtensions
 * @tc.desc: Test whether ScanExtensions and are called normally.
 */
HWTEST_F(ExtensionPluginInfoTest, ScanExtensions_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionPluginInfoTest ScanExtensions_0100 start";
    std::vector<std::string> files;
    bool res = ExtensionPluginInfo::GetInstance().ScanExtensions(files);
    if (OHOS::FileExists("system/lib/extensionability")) {
        EXPECT_EQ(res, true);
        EXPECT_NE(files.size(), 0);
    } else {
        EXPECT_EQ(res, false);
        EXPECT_EQ(files.size(), 0);
    }

    GTEST_LOG_(INFO) << "ExtensionPluginInfoTest ScanExtensions_0100 end";
}

/**
 * @tc.number: ParseExtensions_0100
 * @tc.name: ParseExtensions
 * @tc.desc: Test whether ParseExtensions and are called normally.
 */
HWTEST_F(ExtensionPluginInfoTest, ParseExtensions_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionPluginInfoTest ParseExtensions_0100 start";
    std::vector<std::string> files;
    bool res = ExtensionPluginInfo::GetInstance().ScanExtensions(files);
    if (OHOS::FileExists("system/lib/extensionability")) {
        EXPECT_EQ(res, true);
        EXPECT_NE(files.size(), 0);
    } else {
        EXPECT_EQ(res, false);
        EXPECT_EQ(files.size(), 0);
    }
    ExtensionPluginInfo::GetInstance().ParseExtensions(files);
    GTEST_LOG_(INFO) << "ExtensionPluginInfoTest ParseExtensions_0100 end";
}

/**
 * @tc.number: CheckFileType_0100
 * @tc.name: CheckFileType
 * @tc.desc: Test whether CheckFileType and are called normally.
 */
HWTEST_F(ExtensionPluginInfoTest, CheckFileType_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionPluginInfoTest CheckFileType_0100 start";
    std::string fileName = "";
    bool res = ExtensionPluginInfo::GetInstance().CheckFileType(fileName, ".so");
    EXPECT_EQ(res, false);

    fileName = "ability";
    res = ExtensionPluginInfo::GetInstance().CheckFileType(fileName, ".so");
    EXPECT_EQ(res, false);

    fileName = "ability.so";
    res = ExtensionPluginInfo::GetInstance().CheckFileType(fileName, ".so");
    EXPECT_EQ(res, true);

    GTEST_LOG_(INFO) << "ExtensionPluginInfoTest CheckFileType_0100 end";
}
}
}