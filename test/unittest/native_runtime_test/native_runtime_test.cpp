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

#include <gtest/gtest.h>

#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_environment.h"
#include "native_runtime_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {

class NativeRuntimeImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void NativeRuntimeImplTest::SetUpTestCase() {}

void NativeRuntimeImplTest::TearDownTestCase() {}

void NativeRuntimeImplTest::SetUp() {}

void NativeRuntimeImplTest::TearDown() {}

/*
 * @tc.number    : NativeRuntimeImplTest_0100
 * @tc.name      : Marshalling/UnMarshalling
 * @tc.desc      : 1.Marshalling/UnMarshalling
 */
HWTEST_F(NativeRuntimeImplTest, NativeRuntimeImplTest_001, TestSize.Level1)
{
    HILOG_INFO("NativeRuntimeImplTest_001 start");
    Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = NativeRuntimeImpl::GetNativeRuntimeImpl().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, NATIVE_RUNTIME_ERR_OK);

    err = NativeRuntimeImpl::GetNativeRuntimeImpl().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    EXPECT_EQ(err, NATIVE_RUNTIME_ERR_OK);
    HILOG_INFO("NativeRuntimeImplTest_001 end");
}

HWTEST_F(NativeRuntimeImplTest, NativeRuntimeImplTest_002, TestSize.Level1)
{
    HILOG_INFO("NativeRuntimeImplTest_002 start");
    Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = NativeRuntimeImpl::GetNativeRuntimeImpl().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, NATIVE_RUNTIME_ERR_OK);

    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv2 = nullptr;
    err = NativeRuntimeImpl::GetNativeRuntimeImpl().CreateJsEnv(options, jsEnv2);
    EXPECT_EQ(err, NATIVE_RUNTIME_THREAD_ONLY_ONE_RUNENV);

    err = NativeRuntimeImpl::GetNativeRuntimeImpl().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    EXPECT_EQ(err, NATIVE_RUNTIME_ERR_OK);

    err = NativeRuntimeImpl::GetNativeRuntimeImpl().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv2->GetNativeEngine()));
    EXPECT_EQ(err, NATIVE_RUNTIME_DESTORY_FAILED);
    HILOG_INFO("NativeRuntimeImplTest_002 end");
}

HWTEST_F(NativeRuntimeImplTest, NativeRuntimeImplTest_003, TestSize.Level1)
{
    HILOG_INFO("NativeRuntimeImplTest_003 start");
    Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = NativeRuntimeImpl::GetNativeRuntimeImpl().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, NATIVE_RUNTIME_ERR_OK);

    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    err = NativeRuntimeImpl::GetNativeRuntimeImpl().Init(options, env);
    EXPECT_EQ(err, NATIVE_RUNTIME_ERR_OK);

    err = NativeRuntimeImpl::GetNativeRuntimeImpl().RemoveJsEnv(env);
    EXPECT_EQ(err, NATIVE_RUNTIME_ERR_OK);
    HILOG_INFO("NativeRuntimeImplTest_003 end");
}
}  // namespace AAFwk
}  // namespace OHOS