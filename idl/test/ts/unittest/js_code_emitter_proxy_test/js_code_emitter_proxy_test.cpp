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
#include <iostream>

using namespace testing::ext;
using namespace OHOS::Idl::TestCommon;
namespace OHOS {
namespace Idl {
namespace UnitTest {
class JsCodeEmitterProxyTest : public testing::Test, public IdlCommon {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void JsCodeEmitterProxyTest::SetUpTestCase(void)
{}
void JsCodeEmitterProxyTest::TearDownTestCase(void)
{}
void JsCodeEmitterProxyTest::SetUp()
{}
void JsCodeEmitterProxyTest::TearDown()
{}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitWriteArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitWriteArrayVariable_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = jsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_IN) != 0) {
                MetaType* innerType = jsCodeGen_->metaComponent_->types_[mt->nestedTypeIndexes_[0]];
                jsCodeGen_->EmitWriteArrayVariable("data", mp->name_, innerType,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeSequenceableArray"), ERR_FAIL);
    EXPECT_TRUE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitWriteArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitWriteArrayVariable_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = jsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_IN) != 0) {
                MetaType* innerType = jsCodeGen_->metaComponent_->types_[mt->nestedTypeIndexes_[0]];
                jsCodeGen_->EmitWriteArrayVariable("data", mp->name_, innerType,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceableArray"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitWriteArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitWriteArrayVariable_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = jsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_IN) != 0) {
                MetaType* innerType = jsCodeGen_->metaComponent_->types_[mt->nestedTypeIndexes_[0]];
                jsCodeGen_->EmitWriteArrayVariable("data", mp->name_, innerType,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceableArray"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitWriteArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitWriteArrayVariable_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = jsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_IN) != 0) {
                MetaType* innerType = jsCodeGen_->metaComponent_->types_[mt->nestedTypeIndexes_[0]];
                jsCodeGen_->EmitWriteArrayVariable("data", mp->name_, innerType,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("writeSequenceableArray"), ERR_FAIL);
    EXPECT_TRUE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitWriteArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitWriteArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitWriteArrayVariable_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    int count = 0;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = jsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_IN) != 0) {
                count++;
                MetaType* innerType = jsCodeGen_->metaComponent_->types_[mt->nestedTypeIndexes_[0]];
                jsCodeGen_->EmitWriteArrayVariable("data", mp->name_, innerType,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    const int size = 18;
    EXPECT_TRUE(count == size);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("Int8Array"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("writeSequenceableArray"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadOutArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitReadOutArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitReadOutArrayVariable_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = jsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_MASK) == ATTR_OUT) {
                jsCodeGen_->EmitReadOutArrayVariable("data", mp->name_, mt,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceableArray"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadOutArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitReadOutArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitReadOutArrayVariable_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = jsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_MASK) == ATTR_OUT) {
                jsCodeGen_->EmitReadOutArrayVariable("data", mp->name_, mt,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_EQ(stringBuilder.ToString().IndexOf("readSequenceableArray"), ERR_FAIL);
    EXPECT_TRUE(stringBuilder.ToString().IsEmpty());
}

/*
 * Feature: idl
 * Function: EmitReadOutArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitReadOutArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitReadOutArrayVariable_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = jsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_MASK) == ATTR_OUT) {
                jsCodeGen_->EmitReadOutArrayVariable("data", mp->name_, mt,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceableArray"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadOutArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitReadOutArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitReadOutArrayVariable_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = jsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_MASK) == ATTR_OUT) {
                jsCodeGen_->EmitReadOutArrayVariable("data", mp->name_, mt,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceableArray"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadOutArrayVariable
 * SubFunction: NA
 * FunctionPoints: EmitReadOutArrayVariable validates the array type
 * EnvConditions: NA
 * CaseDescription: Array type keyword generated successfully.
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitReadOutArrayVariable_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(),
        NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./idl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-ts", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    int count = 0;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int j = 0; j < metaMethod->parameterNumber_; j++) {
            MetaParameter* mp = metaMethod->parameters_[j];
            MetaType* mt = jsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array && (mp->attributes_ & ATTR_MASK) == ATTR_OUT) {
                count++;
                jsCodeGen_->EmitReadOutArrayVariable("data", mp->name_, mt,
                    stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    const int size = 18;
    EXPECT_TRUE(count == size);
    EXPECT_FALSE(stringBuilder.ToString().IsEmpty());
    EXPECT_NE(stringBuilder.ToString().IndexOf("readBooleanArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readByteArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readShortArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readIntArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readLongArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readFloatArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readDoubleArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readStringArray"), ERR_FAIL);
    EXPECT_NE(stringBuilder.ToString().IndexOf("readSequenceableArray"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitWriteMethodParameter
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitWriteMethodParameter_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; index < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                jsCodeGen_->EmitWriteMethodParameter(mp, "reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString());
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("writeInt"), ERR_FAIL);
    EXPECT_NE(data.find("writeLong"), ERR_FAIL);
    EXPECT_NE(data.find("writeFloat"), ERR_FAIL);
    EXPECT_NE(data.find("writeDouble"), ERR_FAIL);
    EXPECT_NE(data.find("writeString"), ERR_FAIL);
    EXPECT_NE(data.find("writeSequenceable"), ERR_FAIL);
    EXPECT_NE(data.find("writeRemoteObject"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitWriteMethodParameter
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitWriteMethodParameter_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; index < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                jsCodeGen_->EmitWriteMethodParameter(mp, "reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString());
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("writeInt"), ERR_FAIL);
    EXPECT_NE(data.find("writeLong"), ERR_FAIL);
    EXPECT_NE(data.find("writeFloat"), ERR_FAIL);
    EXPECT_NE(data.find("writeDouble"), ERR_FAIL);
    EXPECT_NE(data.find("writeString"), ERR_FAIL);
    EXPECT_NE(data.find("writeSequenceable"), ERR_FAIL);
    EXPECT_NE(data.find("writeRemoteObject"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitWriteMethodParameter
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitWriteMethodParameter_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; index < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                jsCodeGen_->EmitWriteMethodParameter(mp, "reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString());
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("writeInt"), ERR_FAIL);
    EXPECT_NE(data.find("writeLong"), ERR_FAIL);
    EXPECT_NE(data.find("writeFloat"), ERR_FAIL);
    EXPECT_NE(data.find("writeDouble"), ERR_FAIL);
    EXPECT_NE(data.find("writeString"), ERR_FAIL);
    EXPECT_NE(data.find("writeSequenceable"), ERR_FAIL);
    EXPECT_NE(data.find("writeRemoteObject"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitWriteMethodParameter
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitWriteMethodParameter_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; index < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                jsCodeGen_->EmitWriteMethodParameter(mp, "reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString());
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("writeInt"), ERR_FAIL);
    EXPECT_NE(data.find("writeLong"), ERR_FAIL);
    EXPECT_NE(data.find("writeFloat"), ERR_FAIL);
    EXPECT_NE(data.find("writeDouble"), ERR_FAIL);
    EXPECT_NE(data.find("writeString"), ERR_FAIL);
    EXPECT_NE(data.find("writeSequenceable"), ERR_FAIL);
    EXPECT_NE(data.find("writeRemoteObject"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitWriteMethodParameter
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitWriteMethodParameter_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; index < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                jsCodeGen_->EmitWriteMethodParameter(mp, "reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString());
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("writeInt"), ERR_FAIL);
    EXPECT_NE(data.find("writeLong"), ERR_FAIL);
    EXPECT_NE(data.find("writeFloat"), ERR_FAIL);
    EXPECT_NE(data.find("writeDouble"), ERR_FAIL);
    EXPECT_NE(data.find("writeString"), ERR_FAIL);
    EXPECT_NE(data.find("writeSequenceable"), ERR_FAIL);
    EXPECT_NE(data.find("writeRemoteObject"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadMethodParameter
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitReadMethodParameter_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; index < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                jsCodeGen_->EmitReadMethodParameter(mp, "result.reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString());
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("readInt"), ERR_FAIL);
    EXPECT_NE(data.find("readLong"), ERR_FAIL);
    EXPECT_NE(data.find("readFloat"), ERR_FAIL);
    EXPECT_NE(data.find("readDouble"), ERR_FAIL);
    EXPECT_NE(data.find("readString"), ERR_FAIL);
    EXPECT_NE(data.find("readSequenceable"), ERR_FAIL);
    EXPECT_NE(data.find("readRemoteObject"), ERR_FAIL);
    EXPECT_NE(data.find("new Map()"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadMethodParameter
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitReadMethodParameter_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; index < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                jsCodeGen_->EmitReadMethodParameter(mp, "result.reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString());
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("readInt"), ERR_FAIL);
    EXPECT_NE(data.find("readLong"), ERR_FAIL);
    EXPECT_NE(data.find("readFloat"), ERR_FAIL);
    EXPECT_NE(data.find("readDouble"), ERR_FAIL);
    EXPECT_NE(data.find("readString"), ERR_FAIL);
    EXPECT_NE(data.find("readSequenceable"), ERR_FAIL);
    EXPECT_NE(data.find("readRemoteObject"), ERR_FAIL);
    EXPECT_NE(data.find("new Map()"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadMethodParameter
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitReadMethodParameter_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; index < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                jsCodeGen_->EmitReadMethodParameter(mp, "result.reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString());
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("readInt"), ERR_FAIL);
    EXPECT_NE(data.find("readLong"), ERR_FAIL);
    EXPECT_NE(data.find("readFloat"), ERR_FAIL);
    EXPECT_NE(data.find("readDouble"), ERR_FAIL);
    EXPECT_NE(data.find("readString"), ERR_FAIL);
    EXPECT_NE(data.find("readSequenceable"), ERR_FAIL);
    EXPECT_NE(data.find("readRemoteObject"), ERR_FAIL);
    EXPECT_NE(data.find("new Map()"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadMethodParameter
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitReadMethodParameter_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; index < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                jsCodeGen_->EmitReadMethodParameter(mp, "result.reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString());
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("readInt"), ERR_FAIL);
    EXPECT_NE(data.find("readLong"), ERR_FAIL);
    EXPECT_NE(data.find("readFloat"), ERR_FAIL);
    EXPECT_NE(data.find("readDouble"), ERR_FAIL);
    EXPECT_NE(data.find("readString"), ERR_FAIL);
    EXPECT_NE(data.find("readSequenceable"), ERR_FAIL);
    EXPECT_NE(data.find("readRemoteObject"), ERR_FAIL);
    EXPECT_NE(data.find("new Map()"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadMethodParameter
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitReadMethodParameter_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; index < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                jsCodeGen_->EmitReadMethodParameter(mp, "result.reply", stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString());
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("readInt"), ERR_FAIL);
    EXPECT_NE(data.find("readLong"), ERR_FAIL);
    EXPECT_NE(data.find("readFloat"), ERR_FAIL);
    EXPECT_NE(data.find("readDouble"), ERR_FAIL);
    EXPECT_NE(data.find("readString"), ERR_FAIL);
    EXPECT_NE(data.find("readSequenceable"), ERR_FAIL);
    EXPECT_NE(data.find("readRemoteObject"), ERR_FAIL);
    EXPECT_NE(data.find("new Map()"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadOutVariable
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitReadOutVariable_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; index < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            MetaType* mt = jsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                jsCodeGen_->EmitReadOutVariable("result.reply", mp->name_, mt, stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString());
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("readInt"), ERR_FAIL);
    EXPECT_NE(data.find("readLong"), ERR_FAIL);
    EXPECT_NE(data.find("readFloat"), ERR_FAIL);
    EXPECT_NE(data.find("readDouble"), ERR_FAIL);
    EXPECT_NE(data.find("readString"), ERR_FAIL);
    EXPECT_NE(data.find("readSequenceable"), ERR_FAIL);
    EXPECT_NE(data.find("readRemoteObject"), ERR_FAIL);
    EXPECT_NE(data.find("new Map()"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadOutVariable
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitReadOutVariable_002, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; index < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            MetaType* mt = jsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                jsCodeGen_->EmitReadOutVariable("result.reply", mp->name_, mt, stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString());
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("readInt"), ERR_FAIL);
    EXPECT_NE(data.find("readLong"), ERR_FAIL);
    EXPECT_NE(data.find("readFloat"), ERR_FAIL);
    EXPECT_NE(data.find("readDouble"), ERR_FAIL);
    EXPECT_NE(data.find("readString"), ERR_FAIL);
    EXPECT_NE(data.find("readSequenceable"), ERR_FAIL);
    EXPECT_NE(data.find("readRemoteObject"), ERR_FAIL);
    EXPECT_NE(data.find("new Map()"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadOutVariable
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitReadOutVariable_003, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_AND_OUT_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; index < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            MetaType* mt = jsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                jsCodeGen_->EmitReadOutVariable("result.reply", mp->name_, mt, stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString());
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("readInt"), ERR_FAIL);
    EXPECT_NE(data.find("readLong"), ERR_FAIL);
    EXPECT_NE(data.find("readFloat"), ERR_FAIL);
    EXPECT_NE(data.find("readDouble"), ERR_FAIL);
    EXPECT_NE(data.find("readString"), ERR_FAIL);
    EXPECT_NE(data.find("readSequenceable"), ERR_FAIL);
    EXPECT_NE(data.find("readRemoteObject"), ERR_FAIL);
    EXPECT_NE(data.find("new Map()"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadOutVariable
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitReadOutVariable_004, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_RETURN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; index < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            MetaType* mt = jsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                jsCodeGen_->EmitReadOutVariable("result.reply", mp->name_, mt, stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString());
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("readInt"), ERR_FAIL);
    EXPECT_NE(data.find("readLong"), ERR_FAIL);
    EXPECT_NE(data.find("readFloat"), ERR_FAIL);
    EXPECT_NE(data.find("readDouble"), ERR_FAIL);
    EXPECT_NE(data.find("readString"), ERR_FAIL);
    EXPECT_NE(data.find("readSequenceable"), ERR_FAIL);
    EXPECT_NE(data.find("readRemoteObject"), ERR_FAIL);
    EXPECT_NE(data.find("new Map()"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitReadOutVariable
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitReadOutVariable_005, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_COMBINATION_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; index < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            MetaType* mt = jsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                jsCodeGen_->EmitReadOutVariable("result.reply", mp->name_, mt, stringBuilder, CodeEmitter::TAB);
            }
        }
    }
    std::string data(stringBuilder.ToString());
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("readInt"), ERR_FAIL);
    EXPECT_NE(data.find("readLong"), ERR_FAIL);
    EXPECT_NE(data.find("readFloat"), ERR_FAIL);
    EXPECT_NE(data.find("readDouble"), ERR_FAIL);
    EXPECT_NE(data.find("readString"), ERR_FAIL);
    EXPECT_NE(data.find("readSequenceable"), ERR_FAIL);
    EXPECT_NE(data.find("readRemoteObject"), ERR_FAIL);
    EXPECT_NE(data.find("new Map()"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitLicense
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitLicense_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    jsCodeGen_->EmitLicense(stringBuilder);

    std::string data(stringBuilder.ToString());
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("Ltd."), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: EmitLicense
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitLicense_002, TestSize.Level1)
{
    std::string fileLicense;
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str(), fileLicense), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    StringBuilder stringBuilder;
    jsCodeGen_->EmitLicense(stringBuilder);

    std::string data(stringBuilder.ToString());
    EXPECT_TRUE(data.length());
}

/*
 * Feature: idl
 * Function: EmitType
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, EmitType_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    String result;
    for (int index = 0; index < jsCodeGen_->metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = jsCodeGen_->metaInterface_->methods_[index];
        for (int parameterIndex = 0; index < metaMethod->parameterNumber_; parameterIndex++) {
            MetaParameter* mp = metaMethod->parameters_[parameterIndex];
            MetaType* mt = jsCodeGen_->metaComponent_->types_[mp->typeIndex_];
            result += jsCodeGen_->EmitType(mt);
            result += "_";
        }
    }
    std::string data = result.string();
    EXPECT_FALSE(data.length());
    EXPECT_NE(data.find("void"), ERR_FAIL);
    EXPECT_NE(data.find("number"), ERR_FAIL);
    EXPECT_NE(data.find("string"), ERR_FAIL);
    EXPECT_NE(data.find("MySequenceable"), ERR_FAIL);
    EXPECT_NE(data.find("IInterfaceDeclare"), ERR_FAIL);
    EXPECT_NE(data.find("Map<String, MySequenceable>"), ERR_FAIL);
    EXPECT_NE(data.find("boolean[]"), ERR_FAIL);
    EXPECT_NE(data.find("String[]"), ERR_FAIL);
    EXPECT_NE(data.find("number[]"), ERR_FAIL);
    EXPECT_NE(data.find("MySequenceable[]"), ERR_FAIL);
}

/*
 * Feature: idl
 * Function: FileName
 * SubFunction: NA
 * FunctionPoints: 
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(JsCodeEmitterProxyTest, FileName_001, TestSize.Level1)
{
    EXPECT_EQ(PrepareIdlFile(UNKNOW_TYPE_IDL_NAME.c_str(), NORMAL_TYPE_IN_IDL_CONTENT.c_str()), ERR_OK);
    int argc = 6;
    const char* argvArray[] = {"./zidl", "-c", UNKNOW_TYPE_IDL_NAME.c_str(), "-gen-js", "-d", "."};
    char** argv = (char**)argvArray;
    EXPECT_EQ(Ready(argc, argv), ERR_OK);
    EXPECT_NE(jsCodeGen_, nullptr);
    String param1;
    EXPECT_TRUE(jsCodeGen_->FileName(param1).Equals(param1));
    String param2("testFileName");
    EXPECT_TRUE(jsCodeGen_->FileName(param1).Equals("test_file_name"));
    String param3("app.data.testFileName");
    EXPECT_TRUE(jsCodeGen_->FileName(param1).Equals("app/data/test_file_name"));
    String param4("testfile");
    EXPECT_TRUE(jsCodeGen_->FileName(param1).Equals("testfile"));
}
}
}
}
