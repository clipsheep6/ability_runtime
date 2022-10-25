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

#include "ability_runtime_error_util.h"
#include "errors.h"
#include "mock_native_engine.h"
#include "mock_native_value.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class AbilityRuntimeErrorUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityRuntimeErrorUtilTest::SetUpTestCase()
{}

void AbilityRuntimeErrorUtilTest::TearDownTestCase()
{}

void AbilityRuntimeErrorUtilTest::SetUp()
{}

void AbilityRuntimeErrorUtilTest::TearDown()
{}

/**
 * @tc.name: Throw_0100
 * @tc.desc: Throw_0100 Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(AbilityRuntimeErrorUtilTest, Throw_0100, TestSize.Level0)
{
    MockNativeEngine engine;
    MockNativeValue error;
    EXPECT_CALL(engine, CreateError(_,_)).WillOnce(DoAll(Return(&error)));
    EXPECT_CALL(engine, Throw(_)).WillOnce(DoAll(Return(true)));
    bool result = AbilityRuntimeErrorUtil::Throw(engine, ERR_OK);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: Throw_0200
 * @tc.desc: Throw_0200 Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(AbilityRuntimeErrorUtilTest, Throw_0200, TestSize.Level0)
{
    MockNativeEngine engine;
    EXPECT_CALL(engine, CreateError(_,_)).WillOnce(DoAll(Return(nullptr)));
    bool result = AbilityRuntimeErrorUtil::Throw(engine, ERR_OK);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ThrowByInternalErrCode_0100
 * @tc.desc: ThrowByInternalErrCode_0100 Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(AbilityRuntimeErrorUtilTest, ThrowByInternalErrCode_0100, TestSize.Level0)
{
    MockNativeEngine engine;
    MockNativeValue error;
    EXPECT_CALL(engine, CreateError(_,_)).WillOnce(DoAll(Return(&error)));
    EXPECT_CALL(engine, Throw(_)).WillOnce(DoAll(Return(true)));
    bool result = AbilityRuntimeErrorUtil::ThrowByInternalErrCode(engine, ERR_OK);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: ThrowByInternalErrCode_0200
 * @tc.desc: ThrowByInternalErrCode_0200 Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(AbilityRuntimeErrorUtilTest, ThrowByInternalErrCode_0200, TestSize.Level0)
{
    MockNativeEngine engine;
    bool result = AbilityRuntimeErrorUtil::ThrowByInternalErrCode(engine, 1);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: CreateErrorByInternalErrCode_0100
 * @tc.desc: CreateErrorByInternalErrCode_0100 Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(AbilityRuntimeErrorUtilTest, CreateErrorByInternalErrCode_0100, TestSize.Level0)
{
    MockNativeEngine engine;
    NativeValue *result = AbilityRuntimeErrorUtil::CreateErrorByInternalErrCode(engine, ERR_OK);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: CreateErrorByInternalErrCode_0200
 * @tc.desc: CreateErrorByInternalErrCode_0200 Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(AbilityRuntimeErrorUtilTest, CreateErrorByInternalErrCode_0200, TestSize.Level0)
{
    MockNativeEngine engine;
    NativeValue *result = AbilityRuntimeErrorUtil::CreateErrorByInternalErrCode(engine, 1);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetErrMessage_0100
 * @tc.desc: GetErrMessage_0100 Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(AbilityRuntimeErrorUtilTest, GetErrMessage_0100, TestSize.Level0)
{
    std::string errMsg = AbilityRuntimeErrorUtil::GetErrMessage(ERR_OK);
    EXPECT_EQ(errMsg, "success");
}

/**
 * @tc.name: GetErrMessage_0200
 * @tc.desc: GetErrMessage_0200 Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(AbilityRuntimeErrorUtilTest, GetErrMessage_0200, TestSize.Level0)
{
    std::string errMsg = AbilityRuntimeErrorUtil::GetErrMessage(1);
    EXPECT_EQ(errMsg, "");
}
}  // namespace AAFwk
}  // namespace OHOS
