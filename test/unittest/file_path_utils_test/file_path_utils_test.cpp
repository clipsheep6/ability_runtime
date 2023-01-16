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

#include "file_path_utils.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityBase;

namespace OHOS {
namespace AbilityRuntime {
class FilePathUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void FilePathUtilsTest::SetUpTestCase()
{}

void FilePathUtilsTest::TearDownTestCase()
{}

void FilePathUtilsTest::SetUp()
{}

void FilePathUtilsTest::TearDown()
{}

/**
 * @tc.name: StringStartWith_0100
 * @tc.desc: StringStartWith Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, StringStartWith_0100, TestSize.Level0)
{
    std::string longStr = "abcde";
    const char *shortStr = "abc";
    size_t startStrLenInvalid1 = 20;
    EXPECT_FALSE(StringStartWith(longStr, shortStr, startStrLenInvalid1));
    size_t startStrLenInvalid2 = 0;
    EXPECT_FALSE(StringStartWith(longStr, shortStr, startStrLenInvalid2));
    size_t startStrLen = 3;
    EXPECT_TRUE(StringStartWith(longStr, shortStr, startStrLen));
}

/**
 * @tc.name: StringEndWith_0100
 * @tc.desc: StringEndWith Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, StringEndWith_0100, TestSize.Level0)
{
    std::string longStr = "abcde";
    const char *shortStr = "de";
    size_t endStrLenInvalid1 = 20;
    EXPECT_FALSE(StringEndWith(longStr, shortStr, endStrLenInvalid1));
    size_t endStrLenInvalid2 = 0;
    EXPECT_FALSE(StringEndWith(longStr, shortStr, endStrLenInvalid2));
    size_t endStrLen = 2;
    EXPECT_TRUE(StringEndWith(longStr, shortStr, endStrLen));
}

/**
 * @tc.name: SplitString_0100
 * @tc.desc: SplitString Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, SplitString_0100, TestSize.Level0)
{
    std::string longStr = "";
    std::vector<std::string> strVector;
    size_t pos = 0;
    const char* seps = "a";
    SplitString(longStr, strVector, pos, seps);
    EXPECT_TRUE(strVector.size() == 0);
}

/**
 * @tc.name: SplitString_0200
 * @tc.desc: SplitString Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, SplitString_0200, TestSize.Level0)
{
    std::string longStr = "a";
    std::vector<std::string> strVector;
    size_t pos = 6;
    const char* seps = "a";
    SplitString(longStr, strVector, pos, seps);
    EXPECT_TRUE(strVector.size() == 0);
}

/**
 * @tc.name: SplitString_0300
 * @tc.desc: SplitString Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, SplitString_0300, TestSize.Level0)
{
    std::string longStr = "abc:abc";
    std::vector<std::string> strVector;
    size_t pos = 0;
    const char* seps = "|";
    SplitString(longStr, strVector, pos, seps);
    EXPECT_TRUE(strVector.size() == 1);
}

/**
 * @tc.name: SplitString_0400
 * @tc.desc: SplitString Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, SplitString_0400, TestSize.Level0)
{
    std::string longStr = "abc:abc";
    std::vector<std::string> strVector;
    size_t pos = 0;
    const char* seps = ":";
    SplitString(longStr, strVector, pos, seps);
    EXPECT_TRUE(strVector.size() == 2);
}

/**
 * @tc.name: JoinString_0100
 * @tc.desc: JoinString Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, JoinString_0100, TestSize.Level0)
{
    std::vector<std::string> strVector{"a", "b", "c", "d", "e"};
    char sep = ':';
    size_t startIndex = 0;
    std::string result = JoinString(strVector, sep, startIndex);
    EXPECT_TRUE(result == "a:b:c:d:e");
}

/**
 * @tc.name: JoinString_0200
 * @tc.desc: JoinString Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, JoinString_0200, TestSize.Level0)
{
    std::vector<std::string> strVector{"a", "b", "c", "d", ""};
    char sep = ':';
    size_t startIndex = 0;
    std::string result = JoinString(strVector, sep, startIndex);
    EXPECT_TRUE(result == "a:b:c:d");
}

/**
 * @tc.name: JoinString_0300
 * @tc.desc: JoinString Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, JoinString_0300, TestSize.Level0)
{
    std::vector<std::string> strVector{""};
    char sep = ':';
    size_t startIndex = 0;
    std::string result = JoinString(strVector, sep, startIndex);
    EXPECT_TRUE(result == "");
}

/**
 * @tc.name: StripString_0100
 * @tc.desc: StripString Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, StripString_0100, TestSize.Level0)
{
    std::string str = "abc";
    const char *charSet = "123";
    std::string result = StripString(str, charSet);
    EXPECT_TRUE(result == str);

    std::string str1 = "123abc";
    std::string result1 = StripString(str, charSet);
    EXPECT_TRUE(result1 == str);
}

/**
 * @tc.name: FixExtName_0100
 * @tc.desc: FixExtName Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, FixExtName_0100, TestSize.Level0)
{
    std::string path = "";
    FixExtName(path);
    EXPECT_TRUE(path == "");

    std::string path1 = "123.abc";
    FixExtName(path1);
    EXPECT_TRUE(path1 == "123.abc");

    std::string path2 = "123.ets";
    FixExtName(path2);
    EXPECT_TRUE(path2 == "123.abc");

    std::string path3 = "123.ts";
    FixExtName(path3);
    EXPECT_TRUE(path3 == "123.abc");

    std::string path4 = "123.js";
    FixExtName(path4);
    EXPECT_TRUE(path4 == "123.abc");
}

/**
 * @tc.name: GetInstallPath_0100
 * @tc.desc: GetInstallPath Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, GetInstallPath_0100, TestSize.Level0)
{
    const std::string& curJsModulePath = "/data/storage/el1/bundle/curJsModulePath";
    bool module = false;
    std::string newJsModulePath = GetInstallPath(curJsModulePath, module);
    EXPECT_EQ(newJsModulePath, "/data/storage/el1/bundle/");
}

/**
 * @tc.name: GetInstallPath_0200
 * @tc.desc: GetInstallPath Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, GetInstallPath_0200, TestSize.Level0)
{
    const std::string& curJsModulePath = "/data/bundle";
    bool module = false;
    std::string newJsModulePath = GetInstallPath(curJsModulePath, module);
    EXPECT_EQ(newJsModulePath, std::string());
}

/**
 * @tc.name: GetInstallPath_0300
 * @tc.desc: GetInstallPath Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, GetInstallPath_0300, TestSize.Level0)
{
    const std::string& curJsModulePath = "/data/bundlescurJsModulePath";
    bool module = false;
    std::string newJsModulePath = GetInstallPath(curJsModulePath, module);
    EXPECT_EQ(newJsModulePath, std::string());
}

/**
 * @tc.name: GetInstallPath_0400
 * @tc.desc: GetInstallPath Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, GetInstallPath_0400, TestSize.Level0)
{
    const std::string& curJsModulePath = "/data/bundles/curJsModulePath/module";
    bool module = false;
    std::string newJsModulePath = GetInstallPath(curJsModulePath, module);
    EXPECT_EQ(newJsModulePath, "/data/bundles/curJsModulePath/");
}

/**
 * @tc.name: GetInstallPath_0500
 * @tc.desc: GetInstallPath Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, GetInstallPath_0500, TestSize.Level0)
{
    const std::string& curJsModulePath = "/data/bundles/curJsModulePath/module";
    bool module = true;
    std::string newJsModulePath = GetInstallPath(curJsModulePath, module);
    EXPECT_EQ(newJsModulePath, std::string());
}

/**
 * @tc.name: GetInstallPath_0600
 * @tc.desc: GetInstallPath Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, GetInstallPath_0600, TestSize.Level0)
{
    const std::string& curJsModulePath = "/data/storage/el1/bundle/module/curJsModulePath";
    bool module = true;
    std::string newJsModulePath = GetInstallPath(curJsModulePath, module);
    EXPECT_EQ(newJsModulePath, "/data/storage/el1/bundle/module/");
}

/**
 * @tc.name: MakeNewJsModulePath_0100
 * @tc.desc: MakeNewJsModulePath Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, MakeNewJsModulePath_0100, TestSize.Level0)
{
    const std::string& curJsModulePath = "/data/bundles/curJsModulePath/module";
    const std::string& newJsModuleUri = "";
    std::string newJsModulePath = MakeNewJsModulePath(curJsModulePath, newJsModuleUri);
    EXPECT_EQ(newJsModulePath, std::string());
}

/**
 * @tc.name: MakeNewJsModulePath_0200
 * @tc.desc: MakeNewJsModulePath Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, MakeNewJsModulePath_0200, TestSize.Level0)
{
    const std::string& curJsModulePath = "/data/storage/el1/bundle/module/";
    const std::string& newJsModuleUri = "";
    std::string newJsModulePath = MakeNewJsModulePath(curJsModulePath, newJsModuleUri);
    EXPECT_EQ(newJsModulePath, std::string());
}

/**
 * @tc.name: FindNpmPackageInPath_0100
 * @tc.desc: FindNpmPackageInPath Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, FindNpmPackageInPath_0100, TestSize.Level0)
{
    std::string lengthPath(PATH_MAX, 'a');
    const std::string& npmPath = lengthPath;
    std::string newJsModulePath = FindNpmPackageInPath(npmPath);
    EXPECT_EQ(newJsModulePath, std::string());
}

/**
 * @tc.name: FindNpmPackageInPath_0200
 * @tc.desc: FindNpmPackageInPath Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, FindNpmPackageInPath_0200, TestSize.Level0)
{
    const std::string& npmPath = "npmPath";
    std::string newJsModulePath = FindNpmPackageInPath(npmPath);
    EXPECT_EQ(newJsModulePath, std::string());
}

/**
 * @tc.name: FindNpmPackageInTopLevel_0100
 * @tc.desc: FindNpmPackageInTopLevel Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, FindNpmPackageInTopLevel_0100, TestSize.Level0)
{
    const std::string& moduleInstallPath = "";
    const std::string& npmPackage = "";
    size_t start = 2;
    std::string newJsModulePath = FindNpmPackageInTopLevel(moduleInstallPath, npmPackage, start);
    EXPECT_EQ(newJsModulePath, std::string());
}

/**
 * @tc.name: ParseOhmUri_0100
 * @tc.desc: ParseOhmUri Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, ParseOhmUri_0100, TestSize.Level0)
{
    const std::string& originBundleName = "";
    const std::string& curJsModulePath = "";
    const std::string& newJsModuleUri = "@bundle:originBundleName\bundleName";
    std::string newJsModulePath = ParseOhmUri(originBundleName, curJsModulePath, newJsModuleUri);
    EXPECT_EQ(newJsModulePath, std::string());
}

/**
 * @tc.name: ParseOhmUri_0200
 * @tc.desc: ParseOhmUri Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, ParseOhmUri_0200, TestSize.Level0)
{
    const std::string& originBundleName = "bundleName1";
    const std::string& curJsModulePath = "/data/storage/el1/bundle/curJsModulePath";
    const std::string& newJsModuleUri = "@bundle:originBundleName/bundleName1/bundleName2/bundleName3/bundleName4";
    std::string newJsModulePath = ParseOhmUri(originBundleName, curJsModulePath, newJsModuleUri);
    EXPECT_EQ(newJsModulePath, "/data/bundles/originBundleName/bundleName1/bundleName2/bundleName3/bundleName4");
}

/**
 * @tc.name: ParseOhmUri_0300
 * @tc.desc: ParseOhmUri Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, ParseOhmUri_0300, TestSize.Level0)
{
    const std::string& originBundleName = "";
    const std::string& curJsModulePath = "";
    const std::string& newJsModuleUri = "@module:originBundleName\bundleName";
    std::string newJsModulePath = ParseOhmUri(originBundleName, curJsModulePath, newJsModuleUri);
    EXPECT_EQ(newJsModulePath, std::string());
}

/**
 * @tc.name: ParseOhmUri_0400
 * @tc.desc: ParseOhmUri Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, ParseOhmUri_0400, TestSize.Level0)
{
    const std::string& originBundleName = "";
    const std::string& curJsModulePath = "";
    const std::string& newJsModuleUri = "@module:originBundleName\bundleName1\bundleName2";
    std::string newJsModulePath = ParseOhmUri(originBundleName, curJsModulePath, newJsModuleUri);
    EXPECT_EQ(newJsModulePath, std::string());
}

/**
 * @tc.name: ParseOhmUri_0500
 * @tc.desc: ParseOhmUri Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, ParseOhmUri_0500, TestSize.Level0)
{
    const std::string& originBundleName = "";
    const std::string& curJsModulePath = "/data/storage/el1/bundle/module/curJsModulePath";
    const std::string& newJsModuleUri = "@module:originBundleName/bundleName1/bundleName2/bundleName3";
    std::string newJsModulePath = ParseOhmUri(originBundleName, curJsModulePath, newJsModuleUri);
    EXPECT_EQ(newJsModulePath, "/data/storage/el1/bundle/originBundleName/bundleName1/bundleName2/bundleName3");
}

/**
 * @tc.name: ParseOhmUri_0600
 * @tc.desc: ParseOhmUri Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, ParseOhmUri_0600, TestSize.Level0)
{
    const std::string& originBundleName = "";
    const std::string& curJsModulePath = "";
    const std::string& newJsModuleUri = "@local:originBundleName";
    std::string newJsModulePath = ParseOhmUri(originBundleName, curJsModulePath, newJsModuleUri);
    EXPECT_EQ(newJsModulePath, std::string());
}

/**
 * @tc.name: ParseOhmUri_0700
 * @tc.desc: ParseOhmUri Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, ParseOhmUri_0700, TestSize.Level0)
{
    const std::string& originBundleName = "";
    const std::string& curJsModulePath = "/data/bundles/curJsModulePath/module";
    const std::string& newJsModuleUri = "@local:originBundleName";
    std::string newJsModulePath = ParseOhmUri(originBundleName, curJsModulePath, newJsModuleUri);
    EXPECT_EQ(newJsModulePath, std::string());
}

/**
 * @tc.name: ParseOhmUri_0800
 * @tc.desc: ParseOhmUri Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, ParseOhmUri_0800, TestSize.Level0)
{
    const std::string& originBundleName = "";
    const std::string& curJsModulePath = "";
    const std::string& newJsModuleUri = "@other:originBundleName\bundleName";
    std::string newJsModulePath = ParseOhmUri(originBundleName, curJsModulePath, newJsModuleUri);
    EXPECT_EQ(newJsModulePath, std::string());
}

/**
 * @tc.name: NormalizeUri_0100
 * @tc.desc: NormalizeUri Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, NormalizeUri_0100, TestSize.Level0)
{
    const std::string& bundleName = "";
    const std::string& curJsModulePath = "";
    const std::string& newJsModuleUri = "";
    std::string newJsModulePath = NormalizeUri(bundleName, curJsModulePath, newJsModuleUri);
    EXPECT_EQ(newJsModulePath, "");
}

/**
 * @tc.name: NormalizeUri_0200
 * @tc.desc: NormalizeUri Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, NormalizeUri_0200, TestSize.Level0)
{
    const std::string& bundleName = "";
    const std::string& curJsModulePath = "";
    const std::string& newJsModuleUri = "a";
    std::string newJsModulePath = NormalizeUri(bundleName, curJsModulePath, newJsModuleUri);
    EXPECT_EQ(newJsModulePath, "");
}

/**
 * @tc.name: NormalizeUri_0300
 * @tc.desc: NormalizeUri Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, NormalizeUri_0300, TestSize.Level0)
{
    const std::string& bundleName = "";
    const std::string& curJsModulePath = "a";
    const std::string& newJsModuleUri = "";
    std::string newJsModulePath = NormalizeUri(bundleName, curJsModulePath, newJsModuleUri);
    EXPECT_EQ(newJsModulePath, "");
}

/**
 * @tc.name: MakeFilePath_0100
 * @tc.desc: MakeFilePath Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, MakeFilePath_0100, TestSize.Level0)
{
    std::string bundleName(PATH_MAX, 'a');
    const std::string& codePath = bundleName;
    const std::string& modulePath = "";
    std::string fileName = "";
    bool newJsModulePath = MakeFilePath(codePath, modulePath, fileName);
    EXPECT_FALSE(newJsModulePath);
}

/**
 * @tc.name: MakeFilePath_0200
 * @tc.desc: MakeFilePath Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, MakeFilePath_0200, TestSize.Level0)
{
    const std::string& codePath = "codePath";
    const std::string& modulePath = "";
    std::string fileName = "";
    bool newJsModulePath = MakeFilePath(codePath, modulePath, fileName);
    EXPECT_FALSE(newJsModulePath);
}

/**
 * @tc.name: MakeFilePath_0300
 * @tc.desc: MakeFilePath Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(FilePathUtilsTest, MakeFilePath_0300, TestSize.Level0)
{
    const std::string& codePath = "../codePath";
    const std::string& modulePath = "";
    std::string fileName = "";
    bool newJsModulePath = MakeFilePath(codePath, modulePath, fileName);
    EXPECT_FALSE(newJsModulePath);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
