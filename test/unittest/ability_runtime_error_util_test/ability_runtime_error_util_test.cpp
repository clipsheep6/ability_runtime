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
#include "ecmascript/napi/include/jsnapi.h"
#include "errors.h"
#include "hilog_wrapper.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "native_engine/native_engine.h"

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

    napi_env env_ = nullptr;
    panda::ecmascript::EcmaVM* vm_ = nullptr;
};

void AbilityRuntimeErrorUtilTest::SetUpTestCase()
{}

void AbilityRuntimeErrorUtilTest::TearDownTestCase()
{}

void AbilityRuntimeErrorUtilTest::SetUp()
{
    panda::RuntimeOption pandaOption;
    vm_ = panda::JSNApi::CreateJSVM(pandaOption);
    if (vm_ == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "Create vm failed.");
        return;
    }

    env_ = reinterpret_cast<napi_env>(new ArkNativeEngine(vm_, nullptr));
}

void AbilityRuntimeErrorUtilTest::TearDown()
{
    if (env_ != nullptr) {
        delete reinterpret_cast<NativeEngine*>(env_);
        env_ = nullptr;
    }

    if (vm_ != nullptr) {
        panda::JSNApi::DestroyJSVM(vm_);
        vm_ = nullptr;
    }
}

/**
 * @tc.name: ThrowByInternalErrCode_0200
 * @tc.desc: ThrowByInternalErrCode_0200 Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(AbilityRuntimeErrorUtilTest, ThrowByInternalErrCode_0200, TestSize.Level0)
{
    ASSERT_NE(env_, nullptr);
    bool result = AbilityRuntimeErrorUtil::ThrowByInternalErrCode(env_, 1);
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
    ASSERT_NE(env_, nullptr);
    napi_value result = AbilityRuntimeErrorUtil::CreateErrorByInternalErrCode(env_, ERR_OK);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name: CreateErrorByInternalErrCode_0200
 * @tc.desc: CreateErrorByInternalErrCode_0200 Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(AbilityRuntimeErrorUtilTest, CreateErrorByInternalErrCode_0200, TestSize.Level0)
{
    ASSERT_NE(env_, nullptr);
    napi_value result = AbilityRuntimeErrorUtil::CreateErrorByInternalErrCode(env_, 1);
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

/**
 * @tc.name: Throw_0100
 * @tc.desc: Throw_0100 Test
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeErrorUtilTest, Throw_0100, TestSize.Level0)
{
    ASSERT_NE(env_, nullptr);
    std::string errMessage = nullptr;
    bool result = AbilityRuntimeErrorUtil::Throw(env_, 1, errMessage);
    EXPECT_FALSE(result);
}
}  // namespace AAFwk
}  // namespace OHOS
