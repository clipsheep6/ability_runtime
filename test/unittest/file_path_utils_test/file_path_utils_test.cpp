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
}  // namespace AbilityRuntime
}  // namespace OHOS
