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

#define private public
#define protected public
#include "source_map.h"
#undef private
#undef protected

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

class SourceMapTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SourceMapTest::SetUpTestCase(void)
{
}

void SourceMapTest::TearDownTestCase(void)
{
}

void SourceMapTest::SetUp(void)
{
}

void SourceMapTest::TearDown(void)
{
}

/**
 * @tc.number: AaFwk_SourceMap_0100
 * @tc.name: Base64CharToInt
 * @tc.desc: Verify int values from A to Z.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0100 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();
    char charCode = 'A';
    uint32_t value = modSourceMap->Base64CharToInt(charCode);
    EXPECT_EQ(static_cast<int32_t>(value), 0);

    charCode = 'Z';
    value = modSourceMap->Base64CharToInt(charCode);
    EXPECT_EQ(static_cast<int32_t>(value), 25);

    charCode = 'C';
    value = modSourceMap->Base64CharToInt(charCode);
    EXPECT_EQ(static_cast<int32_t>(value), 2);
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0100 end";
}

/**
 * @tc.number: AaFwk_SourceMap_0200
 * @tc.name: Base64CharToInt
 * @tc.desc: Verify int values from a to z.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0200 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();
    char charCode = 'a';
    uint32_t value = modSourceMap->Base64CharToInt(charCode);
    EXPECT_EQ(static_cast<int32_t>(value), 26);

    charCode = 'z';
    value = modSourceMap->Base64CharToInt(charCode);
    EXPECT_EQ(static_cast<int32_t>(value), 51);

    charCode = 'c';
    value = modSourceMap->Base64CharToInt(charCode);
    EXPECT_EQ(static_cast<int32_t>(value), 28);
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0200 end";
}

/**
 * @tc.number: AaFwk_SourceMap_0300
 * @tc.name: Base64CharToInt
 * @tc.desc: Verify int values from 0 to 9.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0300 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();
    char charCode = '0';
    uint32_t value = modSourceMap->Base64CharToInt(charCode);
    EXPECT_EQ(static_cast<int32_t>(value), 52);

    charCode = '9';
    value = modSourceMap->Base64CharToInt(charCode);
    EXPECT_EQ(static_cast<int32_t>(value), 61);

    charCode = '2';
    value = modSourceMap->Base64CharToInt(charCode);
    EXPECT_EQ(static_cast<int32_t>(value), 54);
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0300 end";
}

/**
 * @tc.number: AaFwk_SourceMap_0400
 * @tc.name: Base64CharToInt
 * @tc.desc: Verify int values for + and / or other symbols.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0400 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();
    char charCode = '+';
    uint32_t value = modSourceMap->Base64CharToInt(charCode);
    EXPECT_EQ(static_cast<int32_t>(value), 62);

    charCode = '/';
    value = modSourceMap->Base64CharToInt(charCode);
    EXPECT_EQ(static_cast<int32_t>(value), 63);

    charCode = '&';
    value = modSourceMap->Base64CharToInt(charCode);
    EXPECT_EQ(static_cast<int32_t>(value), 64);

    charCode = '@';
    value = modSourceMap->Base64CharToInt(charCode);
    EXPECT_EQ(static_cast<int32_t>(value), 64);
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0400 end";
}

/**
 * @tc.number: AaFwk_SourceMap_0500
 * @tc.name: GetErrorPos
 * @tc.desc: Verifying GetErrorPos succeeded.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0500 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();
    std::string rawStack = "at AssertException (/mnt/assets/ets/TestAbility/TestAbility_.js:5779:5)\n";
    auto pos = modSourceMap->GetErrorPos(rawStack);
    EXPECT_EQ(pos.first, 5779);
    EXPECT_EQ(pos.second, 5);
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0500 end";
}

/**
 * @tc.number: AaFwk_SourceMap_0600
 * @tc.name: GetErrorPos
 * @tc.desc: Verifying GetErrorPos succeeded.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0600 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();
    std::string rawStack = "\n";
    auto pos = modSourceMap->GetErrorPos(rawStack);
    EXPECT_EQ(pos.first, 0);
    EXPECT_EQ(pos.second, 0);
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0600 end";
}

/**
 * @tc.number: AaFwk_SourceMap_0700
 * @tc.name: GetErrorPos
 * @tc.desc: Verifying GetErrorPos succeeded.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0700 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();
    std::string rawStack = "?\n";
    auto pos = modSourceMap->GetErrorPos(rawStack);
    EXPECT_EQ(pos.first, 0);
    EXPECT_EQ(pos.second, 0);
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0700 end";
}

/**
 * @tc.number: AaFwk_SourceMap_0800
 * @tc.name: ReadSourceMapData
 * @tc.desc: Verifying ReadSourceMapData Failed.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_0800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0800 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();
    std::string filePath = "./source_map_test";
    std::string context;
    EXPECT_FALSE(modSourceMap->ReadSourceMapData(filePath, context));
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0800 end";
}

/**
 * @tc.number: AaFwk_SourceMap_0900
 * @tc.name: ReadSourceMapData
 * @tc.desc: Verifying ReadSourceMapData succeeded.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_0900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0900 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();
    std::string filePath = "./abc.map";
    std::string context;
    modSourceMap->ReadSourceMapData(filePath, context);
    EXPECT_TRUE(context.empty());
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_0900 end";
}

/**
 * @tc.number: AaFwk_SourceMap_1000
 * @tc.name: Find
 * @tc.desc: Verifying Find succeeded.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_1000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_1000 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();
    int32_t row = 0;
    int32_t col = 1;
    SourceMapData targetMap;
    std::string key = "";
    auto info = modSourceMap->Find(row, col, targetMap, key);
    EXPECT_TRUE(info.sources.empty());

    row = 1;
    col = 0;
    info = modSourceMap->Find(row, col, targetMap, key);
    EXPECT_TRUE(info.sources.empty());
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_1000 end";
}

/**
 * @tc.number: AaFwk_SourceMap_1100
 * @tc.name: Find
 * @tc.desc: Verifying Find succeeded.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_1100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_1100 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();
    int32_t row = 0;
    int32_t col = 1;
    SourceMapData targetMap;
    std::string key = "";
    auto info = modSourceMap->Find(row, col, targetMap, key);
    EXPECT_TRUE(info.sources.empty());

    row = 1;
    col = 0;
    info = modSourceMap->Find(row, col, targetMap, key);
    EXPECT_TRUE(info.sources.empty());
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_1100 end";
}

/**
 * @tc.number: AaFwk_SourceMap_1200
 * @tc.name: Find
 * @tc.desc: Verifying Find succeeded.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_1200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_1200 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();
    int32_t row = 3;
    int32_t col = 3;
    SourceMapData targetMap;
    targetMap.files_.emplace_back("file");

    SourceMapInfo mapInfo;
    mapInfo.beforeRow = 0;
    mapInfo.beforeColumn = 0;
    mapInfo.afterRow = 1;
    mapInfo.afterColumn = 0;
    mapInfo.sourcesVal = 0;
    mapInfo.namesVal = 0;
    targetMap.afterPos_.emplace_back(mapInfo);
    std::string key = "";
    auto info = modSourceMap->Find(row, col, targetMap, key);
    EXPECT_STREQ(info.sources.c_str(), "file");
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_1200 end";
}

/**
 * @tc.number: AaFwk_SourceMap_1300
 * @tc.name: Find
 * @tc.desc: Verify binary search.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_1300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_1300 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();
    int32_t row = 3;
    int32_t col = 3;
    SourceMapData targetMap;
    targetMap.files_.emplace_back("file");

    for (int32_t i = 0; i < 10; i++) {
        for (int32_t j = 0; j < 5; j++) {
            SourceMapInfo mapInfo;
            mapInfo.beforeRow = 0;
            mapInfo.beforeColumn = 0;
            mapInfo.afterRow = i;
            mapInfo.afterColumn = j;
            targetMap.afterPos_.emplace_back(mapInfo);
        }
    }

    std::string key = "";
    auto info = modSourceMap->Find(row, col, targetMap, key);
    EXPECT_EQ(info.row, 1);
    EXPECT_EQ(info.col, 1);
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_1300 end";
}

/**
 * @tc.number: AaFwk_SourceMap_1400
 * @tc.name: Find
 * @tc.desc: Verify binary search.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_1400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_1400 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();
    int32_t row = 7;
    int32_t col = 1;
    SourceMapData targetMap;
    targetMap.files_.emplace_back("file");

    for (int32_t i = 0; i < 10; i++) {
        SourceMapInfo mapInfo;
        mapInfo.beforeRow = 0;
        mapInfo.beforeColumn = 0;
        mapInfo.afterRow = i;
        mapInfo.afterColumn = 1;
        targetMap.afterPos_.emplace_back(mapInfo);
    }

    std::string key = "aaawebpack:///bbb";
    auto info = modSourceMap->Find(row, col, targetMap, key);
    EXPECT_EQ(info.row, 1);
    EXPECT_EQ(info.col, 1);
    EXPECT_STREQ(info.sources.c_str(), "aaabbb");
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_1400 end";
}

/**
 * @tc.number: AaFwk_SourceMap_1500
 * @tc.name: GetPosInfo
 * @tc.desc: Verifying GetPosInfo succeeded.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_1500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_1500 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();

    std::string temp = "TestAbility.js:5779:5";
    int32_t start = 22;
    std::string line;
    std::string column;
    modSourceMap->GetPosInfo(temp, start, line, column);
    EXPECT_STREQ(line.c_str(), "5779");
    EXPECT_STREQ(column.c_str(), "5");
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_1500 end";
}

/**
 * @tc.number: AaFwk_SourceMap_1600
 * @tc.name: StringToInt
 * @tc.desc: Verifying StringToInt succeeded.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_1600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_1600 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();

    std::string value = "2030300 This is test";
    auto res = modSourceMap->StringToInt(value);
    EXPECT_EQ(res, 2030300);

    value = "2147483648 This is test";
    res = modSourceMap->StringToInt(value);
    EXPECT_EQ(res, 0);

    value = "";
    res = modSourceMap->StringToInt(value);
    EXPECT_EQ(res, 0);

    value = "-2147483649 This is test";
    res = modSourceMap->StringToInt(value);
    EXPECT_EQ(res, 0);
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_1600 end";
}

/**
 * @tc.number: AaFwk_SourceMap_1700
 * @tc.name: GetRelativePath
 * @tc.desc: Verifying GetRelativePath succeeded.
 */
HWTEST_F(SourceMapTest, AaFwk_SourceMap_1700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_1700 start";
    auto modSourceMap = std::make_shared<AbilityRuntime::ModSourceMap>();

    std::string sources = "TEST:/data/app/MainAbility.js";
    auto res = modSourceMap->GetRelativePath(sources);
    EXPECT_STREQ(res.c_str(), "/data/app/MainAbility.js");
    GTEST_LOG_(INFO) << "AaFwk_SourceMap_1700 end";
}
} // namespace AppExecFwk
} // namespace OHOS
