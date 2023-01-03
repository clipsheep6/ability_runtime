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

#include "accessibility_ability_utils.h"

using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::Accessibility;
using namespace testing::ext;

class AccessibilityAbilityUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AccessibilityAbilityUtilsTest::SetUpTestCase()
{}

void AccessibilityAbilityUtilsTest::TearDownTestCase()
{}

void AccessibilityAbilityUtilsTest::SetUp()
{}

void AccessibilityAbilityUtilsTest::TearDown()
{}

/**
 * @tc.number: GetStaticCapabilityNames_0100
 * @tc.name: GetStaticCapabilityNames
 * @tc.desc: Get StaticCapabilityNames Failed
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetStaticCapabilityNames_0100, Function | MediumTest | Level1)
{
    AccessibilityAbilityInitParams params;
    AccessibilityAbilityInfo abilityInfo(params);
    auto result = AccessibilityUtils::GetStaticCapabilityNames(abilityInfo);
    EXPECT_EQ(result, "");
}

/**
 * @tc.number: GetStaticCapabilityNames_0200
 * @tc.name: GetStaticCapabilityNames
 * @tc.desc: Get StaticCapabilityNames Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetStaticCapabilityNames_0200, Function | MediumTest | Level1)
{
    AccessibilityAbilityInitParams params;
    params.staticCapabilities = 0x0001;
    AccessibilityAbilityInfo abilityInfo(params);
    auto result = AccessibilityUtils::GetStaticCapabilityNames(abilityInfo);
    EXPECT_EQ(result, "r");
}

/**
 * @tc.number: GetStaticCapabilityNames_0300
 * @tc.name: GetStaticCapabilityNames
 * @tc.desc: Get StaticCapabilityNames Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetStaticCapabilityNames_0300, Function | MediumTest | Level1)
{
    AccessibilityAbilityInitParams params;
    params.staticCapabilities = 0x0010;
    AccessibilityAbilityInfo abilityInfo(params);
    auto result = AccessibilityUtils::GetStaticCapabilityNames(abilityInfo);
    EXPECT_EQ(result, "z");
}

/**
 * @tc.number: FormatAbilityInfos_0100
 * @tc.name: FormatAbilityInfos
 * @tc.desc: Format AbilityInfos Failed
 */
HWTEST_F(AccessibilityAbilityUtilsTest, FormatAbilityInfos_0100, Function | MediumTest | Level1)
{
    std::vector<Accessibility::AccessibilityAbilityInfo> installedAbilities;
    std::stringstream headerStream;
    headerStream << std::left << std::setw(10) << "NO"
        << std::left << std::setw(50) << "bundleName"
        << std::left << std::setw(30) << "abilityName"
        << std::left << std::setw(20) << "capabilities-abbr" << std::endl;
    auto result = AccessibilityUtils::FormatAbilityInfos(installedAbilities);
    EXPECT_EQ(result, headerStream.str());
}

/**
 * @tc.number: FormatAbilityInfos_0200
 * @tc.name: FormatAbilityInfos
 * @tc.desc: Format AbilityInfos Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, FormatAbilityInfos_0200, Function | MediumTest | Level1)
{
    std::vector<AccessibilityAbilityInfo> installedAbilities;
    AccessibilityAbilityInitParams params;
    params.name = "myapplication";
    params.staticCapabilities = 10;
    AccessibilityAbilityInfo abilityInfo(params);
    std::string bundleName = "com.example.myapplication";
    abilityInfo.SetPackageName(bundleName);
    installedAbilities.emplace_back(abilityInfo);
    auto result = AccessibilityUtils::FormatAbilityInfos(installedAbilities);
    std::string resultstring = "";
    std::stringstream headerStream;
    headerStream << std::left << std::setw(10) << "NO"
        << std::left << std::setw(50) << "bundleName"
        << std::left << std::setw(30) << "abilityName"
        << std::left << std::setw(20) << "capabilities-abbr" << std::endl;
    resultstring.append(headerStream.str());

    std::stringstream lineStream;
    std::string capabilityNames = AccessibilityUtils::GetStaticCapabilityNames(abilityInfo);
    lineStream << std::left << std::setw(10) << std::to_string(1)
        << std::left << std::setw(50) << bundleName
        << std::left << std::setw(30) << params.name
        << std::left << capabilityNames << std::endl;
    resultstring.append(lineStream.str());
    EXPECT_EQ(result, resultstring);
}

/**
 * @tc.number: GetCapabilityValue_0100
 * @tc.name: GetCapabilityValue
 * @tc.desc: Get Capability Value Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetCapabilityValue_0100, Function | MediumTest | Level1)
{
    auto result = AccessibilityUtils::GetCapabilityValue("r");
    EXPECT_EQ(result, 0x0001);
}

/**
 * @tc.number: GetCapabilityValue_0200
 * @tc.name: GetCapabilityValue
 * @tc.desc: Get Capability Value Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetCapabilityValue_0200, Function | MediumTest | Level1)
{
    auto result = AccessibilityUtils::GetCapabilityValue("k");
    EXPECT_EQ(result, 0x0008);
}

/**
 * @tc.number: GetCapabilityValue_0300
 * @tc.name: GetCapabilityValue
 * @tc.desc: Get Capability Value Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetCapabilityValue_0300, Function | MediumTest | Level1)
{
    auto result = AccessibilityUtils::GetCapabilityValue("z");
    EXPECT_EQ(result, 0x0010);
}

/**
 * @tc.number: GetCapabilityValue_0400
 * @tc.name: GetCapabilityValue
 * @tc.desc: Get Capability Value Failed
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetCapabilityValue_0400, Function | MediumTest | Level1)
{
    auto result = AccessibilityUtils::GetCapabilityValue("a");
    std::uint32_t i = 0;
    EXPECT_EQ(result, i);
}

/**
 * @tc.number: GetCapabilityValue_0500
 * @tc.name: GetCapabilityValue
 * @tc.desc: Get CapabilityValue Failed
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetCapabilityValue_0500, Function | MediumTest | Level1)
{
    auto result = AccessibilityUtils::GetCapabilityValue("");
    std::uint32_t i = 0;
    EXPECT_EQ(result, i);
}

/**
 * @tc.number: GetCapabilityValue_0600
 * @tc.name: GetCapabilityValue
 * @tc.desc: Get Capability Value Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetCapabilityValue_0600, Function | MediumTest | Level1)
{
    auto result = AccessibilityUtils::GetCapabilityValue("zr");
    EXPECT_EQ(result, 0x0001|0x0010);
}

/**
 * @tc.number: GetCapabilityValue_0700
 * @tc.name: GetCapabilityValue
 * @tc.desc: Get Capability Value Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetCapabilityValue_0700, Function | MediumTest | Level1)
{
    auto result = AccessibilityUtils::GetCapabilityValue("rtz");
    EXPECT_EQ(result, 0x0001|0x0002|0x0010);
}

/**
 * @tc.number: GetInvalidCapabilityNames_0100
 * @tc.name: GetInvalidCapabilityNames
 * @tc.desc: Get Invalid Capability Names Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetInvalidCapabilityNames_0100, Function | MediumTest | Level1)
{
    auto result = AccessibilityUtils::GetInvalidCapabilityNames("asdfg", "zxcvb");
    EXPECT_EQ(result, "asdfg");
}

/**
 * @tc.number: GetInvalidCapabilityNames_0200
 * @tc.name: GetInvalidCapabilityNames
 * @tc.desc: Get Invalid Capability Names Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetInvalidCapabilityNames_0200, Function | MediumTest | Level1)
{
    auto result = AccessibilityUtils::GetInvalidCapabilityNames("asdfg", "axcvb");
    EXPECT_EQ(result, "sdfg");
}

/**
 * @tc.number: GetInvalidCapabilityNames_0300
 * @tc.name: GetInvalidCapabilityNames
 * @tc.desc: Get Invalid Capability Names Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetInvalidCapabilityNames_0300, Function | MediumTest | Level1)
{
    auto result = AccessibilityUtils::GetInvalidCapabilityNames("asdfg", "axdvb");
    EXPECT_EQ(result, "sfg");
}

/**
 * @tc.number: GetInvalidCapabilityNames_0400
 * @tc.name: GetInvalidCapabilityNames
 * @tc.desc: Get Invalid Capability Names Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetInvalidCapabilityNames_0400, Function | MediumTest | Level1)
{
    auto result = AccessibilityUtils::GetInvalidCapabilityNames("asdfb", "axdvb");
    EXPECT_EQ(result, "sf");
}

/**
 * @tc.number: GetInvalidCapabilityNames_0500
 * @tc.name: GetInvalidCapabilityNames
 * @tc.desc: Get Invalid Capability Names Failed
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetInvalidCapabilityNames_0500, Function | MediumTest | Level1)
{
    auto result = AccessibilityUtils::GetInvalidCapabilityNames("asdfb", "asdfb");
    EXPECT_EQ(result, "");
}

/**
 * @tc.number: GetUnknownArgumentsMsg_0100
 * @tc.name: GetUnknownArgumentsMsg
 * @tc.desc: Get UnknownArguments Msg Failed
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetUnknownArgumentsMsg_0100, Function | MediumTest | Level1)
{
    std::vector<std::string> unknownArguments;
    auto result = AccessibilityUtils::GetUnknownArgumentsMsg(unknownArguments);
    EXPECT_EQ(result, "");
}

/**
 * @tc.number: GetUnknownArgumentsMsg_0200
 * @tc.name: GetUnknownArgumentsMsg
 * @tc.desc: Get UnknownArguments Msg Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetUnknownArgumentsMsg_0200, Function | MediumTest | Level1)
{
    std::vector<std::string> unknownArguments;
    unknownArguments.emplace_back("abilityInfo");
    auto result = AccessibilityUtils::GetUnknownArgumentsMsg(unknownArguments);
    EXPECT_EQ(result, "abilityInfo ");
}

/**
 * @tc.number: GetUnknownArgumentsMsg_0300
 * @tc.name: GetUnknownArgumentsMsg
 * @tc.desc: Get UnknownArguments Msg Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, GetUnknownArgumentsMsg_0300, Function | MediumTest | Level1)
{
    std::vector<std::string> unknownArguments;
    unknownArguments.emplace_back("abilityInfo");
    unknownArguments.emplace_back("bundleInfo");
    auto result = AccessibilityUtils::GetUnknownArgumentsMsg(unknownArguments);
    EXPECT_EQ(result,"abilityInfo bundleInfo ");
}

/**
 * @tc.number: IsValidStateString_0100
 * @tc.name: IsValidStateString
 * @tc.desc: Is Valid State String
 */
HWTEST_F(AccessibilityAbilityUtilsTest, IsValidStateString_0100, Function | MediumTest | Level1)
{
    std::string unknownArguments = " 0 ";
    auto result = AccessibilityUtils::IsValidStateString(unknownArguments);
    EXPECT_TRUE(result);
}

/**
 * @tc.number: IsValidStateString_0200
 * @tc.name: IsValidStateString
 * @tc.desc: Is Valid State String
 */
HWTEST_F(AccessibilityAbilityUtilsTest, IsValidStateString_0200, Function | MediumTest | Level1)
{
    std::string unknownArguments = " 1 ";
    auto result = AccessibilityUtils::IsValidStateString(unknownArguments);
    EXPECT_TRUE(result);
}

/**
 * @tc.number: IsValidStateString_0300
 * @tc.name: IsValidStateString
 * @tc.desc: Is Not ValidState String
 */
HWTEST_F(AccessibilityAbilityUtilsTest, IsValidStateString_0300, Function | MediumTest | Level1)
{
    std::string unknownArguments = " a ";
    auto result = AccessibilityUtils::IsValidStateString(unknownArguments);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: IsValidStateString_0400
 * @tc.name: IsValidStateString
 * @tc.desc: Is Not Valid State String
 */
HWTEST_F(AccessibilityAbilityUtilsTest, IsValidStateString_0400, Function | MediumTest | Level1)
{
    std::string unknownArguments;
    auto result = AccessibilityUtils::IsValidStateString(unknownArguments);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: IsValidIntString_0100
 * @tc.name: IsValidIntString
 * @tc.desc: Is Not Valid Int String
 */
HWTEST_F(AccessibilityAbilityUtilsTest, IsValidIntString_0100, Function | MediumTest | Level1)
{
    std::string unknownArguments;
    auto result = AccessibilityUtils::IsValidIntString(unknownArguments, 0, 0);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: IsValidIntString_0200
 * @tc.name: IsValidIntString
 * @tc.desc: Is Not Valid Int String
 */
HWTEST_F(AccessibilityAbilityUtilsTest, IsValidIntString_0200, Function | MediumTest | Level1)
{
    std::string unknownArguments = "-1111";
    auto result = AccessibilityUtils::IsValidIntString(unknownArguments, 0, 0);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: IsValidIntString_0300
 * @tc.name: IsValidIntString
 * @tc.desc: Is Not Valid Int String
 */
HWTEST_F(AccessibilityAbilityUtilsTest, IsValidIntString_0300, Function | MediumTest | Level1)
{
    std::string unknownArguments = "+1111";
    auto result = AccessibilityUtils::IsValidIntString(unknownArguments, 0, 0);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: IsValidIntString_0400
 * @tc.name: IsValidIntString
 * @tc.desc: Is Not Valid Int String
 */
HWTEST_F(AccessibilityAbilityUtilsTest, IsValidIntString_0400, Function | MediumTest | Level1)
{
    std::string unknownArguments = "+3";
    auto result = AccessibilityUtils::IsValidIntString(unknownArguments, 0, 0);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: IsValidIntString_0500
 * @tc.name: IsValidIntString
 * @tc.desc: Is Valid Int String
 */
HWTEST_F(AccessibilityAbilityUtilsTest, IsValidIntString_0500, Function | MediumTest | Level1)
{
    std::string unknownArguments = "+3";
    auto result = AccessibilityUtils::IsValidIntString(unknownArguments, 0, 1000);
    EXPECT_TRUE(result);
}

/**
 * @tc.number: IsValidIntString_0600
 * @tc.name: IsValidIntString
 * @tc.desc: Is Valid Int String
 */
HWTEST_F(AccessibilityAbilityUtilsTest, IsValidIntString_0600, Function | MediumTest | Level1)
{
    std::string unknownArguments = "+";
    auto result = AccessibilityUtils::IsValidIntString(unknownArguments, 0, 1000);
    EXPECT_TRUE(result);
}

/**
 * @tc.number: IsValidIntString_0700
 * @tc.name: IsValidIntString
 * @tc.desc: Is Valid Int String
 */
HWTEST_F(AccessibilityAbilityUtilsTest, IsValidIntString_0700, Function | MediumTest | Level1)
{
    std::string unknownArguments = "-0";
    auto result = AccessibilityUtils::IsValidIntString(unknownArguments, 0, 1000);
    EXPECT_TRUE(result);
}

/**
 * @tc.number: Trim_0100
 * @tc.name: Trim
 * @tc.desc: Trim Null String
 */
HWTEST_F(AccessibilityAbilityUtilsTest, Trim_0100, Function | MediumTest | Level1)
{
    std::string inputStr = "";
    auto result = AccessibilityUtils::Trim(inputStr);
    EXPECT_EQ(result, "");
}

/**
 * @tc.number: Trim_0200
 * @tc.name: Trim
 * @tc.desc: Trim String Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, Trim_0200, Function | MediumTest | Level1)
{
    std::string inputStr = " 12121";
    auto result = AccessibilityUtils::Trim(inputStr);
    EXPECT_EQ(result, "12121");
}

/**
 * @tc.number: Trim_0300
 * @tc.name: Trim
 * @tc.desc: Trim String Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, Trim_0300, Function | MediumTest | Level1)
{
    std::string inputStr = " 12121 ";
    auto result = AccessibilityUtils::Trim(inputStr);
    EXPECT_EQ(result, "12121");
}

/**
 * @tc.number: Trim_0400
 * @tc.name: Trim
 * @tc.desc: Trim String Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, Trim_0400, Function | MediumTest | Level1)
{
    std::string inputStr = " 121 21 ";
    auto result = AccessibilityUtils::Trim(inputStr);
    EXPECT_EQ(result, "121 21");
}

/**
 * @tc.number: AddPermission_0100
 * @tc.name: AddPermission
 * @tc.desc: Add Permission Success
 */
HWTEST_F(AccessibilityAbilityUtilsTest, AddPermission_0100, Function | MediumTest | Level1)
{
    auto result = AccessibilityUtils::AddPermission();
    EXPECT_EQ(result, 0);
}
