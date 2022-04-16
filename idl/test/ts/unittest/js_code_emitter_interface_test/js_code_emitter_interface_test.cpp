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
#include "idl_common.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Idl::TestCommon;

namespace OHOS {
namespace Idl {
namespace UnitTest {
class JsCodeEmitterInterfaceTest : public testing::Test, public IdlCommon {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void JsCodeEmitterInterfaceTest::SetUpTestCase()
{}

void JsCodeEmitterInterfaceTest::TearDownTestCase()
{}

void JsCodeEmitterInterfaceTest::SetUp()
{}

void JsCodeEmitterInterfaceTest::TearDown()
{}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .idl(value type).
 */
HWTEST_F(JsCodeEmitterInterfaceTest, CheckInterfaceType_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE1_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = const_cast<char **>(argvArray);
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: list type exists in .idl(value type).
 */
HWTEST_F(JsCodeEmitterInterfaceTest, CheckInterfaceType_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE2_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = const_cast<char **>(argvArray);
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .idl(Map<int, char>).
 */
HWTEST_F(JsCodeEmitterInterfaceTest, CheckInterfaceType_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE3_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = const_cast<char **>(argvArray);
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: list type exists in .idl(Map<int, List<int>>).
 */
HWTEST_F(JsCodeEmitterInterfaceTest, CheckInterfaceType_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE4_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = const_cast<char **>(argvArray);
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .idl(return value).
 */
HWTEST_F(JsCodeEmitterInterfaceTest, CheckInterfaceType_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE5_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = const_cast<char **>(argvArray);
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: list type exists in .idl(return value).
 */
HWTEST_F(JsCodeEmitterInterfaceTest, CheckInterfaceType_006, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE6_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = const_cast<char **>(argvArray);
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .idl(return Map<int, char>).
 */
HWTEST_F(JsCodeEmitterInterfaceTest, CheckInterfaceType_007, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE7_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = const_cast<char **>(argvArray);
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: char type exists in .idl(return Map<int, List<int>>).
 */
HWTEST_F(JsCodeEmitterInterfaceTest, CheckInterfaceType_008, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), UNKNOW_TYPE_CASE8_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = const_cast<char **>(argvArray);
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_FALSE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: CheckInterfaceType
 * SubFunction: NA
 * FunctionPoints: Check whether the interface return value type and parameter type are supported by the tool
 * EnvConditions: NA
 * CaseDescription: normal type exists in .idl(int).
 */
HWTEST_F(JsCodeEmitterInterfaceTest, CheckInterfaceType_009, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = const_cast<char **>(argvArray);
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    EXPECT_TRUE(jsCodeGen_->CheckInterfaceType());
}

/*
 * Feature: idl
 * Function: EmitInterfaceImports
 * SubFunction: NA
 * FunctionPoints: For generating impotr code.
 * EnvConditions: NA
 * CaseDescription: The string in EmitInterfaceImports is not empty.
 */
HWTEST_F(JsCodeEmitterInterfaceTest, EmitInterfaceImports_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = const_cast<char **>(argvArray);
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    jsCodeGen_->EmitInterfaceImports(stringBuilder);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
}
}  // namespace UnitTest
}  // namespace Idl
}  // namespace OHOS
