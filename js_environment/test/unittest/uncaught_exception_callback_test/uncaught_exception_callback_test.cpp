/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "js_environment.h"
#include "js_runtime.h"
#include "uncaught_exception_callback.h"

namespace OHOS {
namespace JsEnv {
using namespace testing::ext;
class NapiUncaughtExceptionCallbackTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void NapiUncaughtExceptionCallbackTest::SetUpTestCase(void)
{
}

void NapiUncaughtExceptionCallbackTest::TearDownTestCase(void)
{
}

void NapiUncaughtExceptionCallbackTest::SetUp(void)
{
}

void NapiUncaughtExceptionCallbackTest::TearDown(void)
{
}

/**
 * @tc.name: NapiUncaughtExceptionCallbackTest_0100
 * @tc.type: FUNC
 * @tc.desc: Test NapiNapiUncaughtExceptionCallback GetNativeStrFromJsTaggedObj.
 * @tc.require: #I6T4K1
 */
HWTEST_F(NapiUncaughtExceptionCallbackTest, NapiUncaughtExceptionCallbackTest_0100, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    options.preload = false;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);
    auto env = jsRuntime->GetNapiEnv();
    EXPECT_NE(env, nullptr);

    // Test with null object
    auto task = [](std::string summary, const JsEnv::ErrorObject errorObj) {
        summary += "test";
    };
    NapiUncaughtExceptionCallback callback(task, nullptr, env);
    ASSERT_EQ(callback.GetNativeStrFromJsTaggedObj(nullptr, "key"), "");

    // Test with invalid object
    napi_value object = nullptr;
    napi_create_object(env, &object);
    NapiUncaughtExceptionCallback callback2(task, nullptr, env);
    ASSERT_EQ(callback2.GetNativeStrFromJsTaggedObj(object, "key"), "");

    // Test with valid object
    std::string errorMsg = "This is an error message.";
    std::string errorName = "TypeError";
    std::string errorStack = "TypeError: This is a stack trace.";
    napi_value nativeErrorMsg = nullptr;
    napi_value nativeErrorName = nullptr;
    napi_value nativeErrorStack = nullptr;
    napi_create_string_utf8(env, errorMsg.c_str(), errorMsg.length(), &nativeErrorMsg);
    napi_create_string_utf8(env, errorName.c_str(), errorName.length(), &nativeErrorName);
    napi_create_string_utf8(env, errorStack.c_str(), errorStack.length(), &nativeErrorStack);

    napi_set_named_property(env, object, "message", nativeErrorMsg);
    napi_set_named_property(env, object, "name", nativeErrorName);
    napi_set_named_property(env, object, "stack", nativeErrorStack);
    NapiUncaughtExceptionCallback callback3(task, nullptr, env);
    ASSERT_EQ(callback3.GetNativeStrFromJsTaggedObj(object, "message"), errorMsg);
    ASSERT_EQ(callback3.GetNativeStrFromJsTaggedObj(object, "name"), errorName);
    ASSERT_EQ(callback3.GetNativeStrFromJsTaggedObj(object, "stack"), errorStack);
}

/**
 * @tc.name: NapiUncaughtExceptionCallbackTest_0200
 * @tc.type: FUNC
 * @tc.desc: Test NapiUncaughtExceptionCallback operator().
 * @tc.require: #I6T4K1
 */
HWTEST_F(NapiUncaughtExceptionCallbackTest, NapiUncaughtExceptionCallbackTest_0200, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    options.preload = false;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);
    auto env = jsRuntime->GetNapiEnv();
    EXPECT_NE(env, nullptr);
    // Test with null object
    auto task = [](std::string summary, const JsEnv::ErrorObject errorObj) {
        summary += "test";
    };
    napi_value nullValue = nullptr;
    napi_get_undefined(env, &nullValue);
    NapiUncaughtExceptionCallback callback(task, nullptr, env);
    callback(nullValue);

    // Test with valid code, and errorStack is empty
    napi_value object = nullptr;
    napi_create_object(env, &object);

    std::string errorCode = "This is an error code.";
    napi_value nativeErrorCode =nullptr;
    napi_create_string_utf8(env, errorCode.c_str(), errorCode.length(), &nativeErrorCode);
    napi_set_named_property(env, object, "code", nativeErrorCode);

    NapiUncaughtExceptionCallback callback1(task, nullptr, env);
    callback1(object);
}

/**
 * @tc.name: NapiUncaughtExceptionCallbackTest_0300
 * @tc.type: FUNC
 * @tc.desc: Test NapiUncaughtExceptionCallback operator().
 * @tc.require: #I6T4K1
 */
HWTEST_F(NapiUncaughtExceptionCallbackTest, NapiUncaughtExceptionCallbackTest_0300, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    options.preload = false;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);
    auto env = jsRuntime->GetNapiEnv();
    EXPECT_NE(env, nullptr);

    // Test with valid code, and errorStack is not empty
    napi_value object = nullptr;
    napi_create_object(env, &object);

    std::string errorCode = "This is an error code.";
    std::string errorStack = "TypeError: This is a stack trace.";
    napi_value nativeErrorCode = nullptr;
    napi_value nativeErrorStack = nullptr;
    napi_create_string_utf8(env, errorCode.c_str(), errorCode.length(), &nativeErrorCode);
    napi_create_string_utf8(env, errorStack.c_str(), errorStack.length(), &nativeErrorStack);
    napi_set_named_property(env, object, "code", nativeErrorCode);
    napi_set_named_property(env, object, "stack", nativeErrorStack);
    auto task = [](std::string summary, const JsEnv::ErrorObject errorObj) {
        summary += "test";
    };
    NapiUncaughtExceptionCallback callback(task, nullptr, env);
    callback(object);
}

/**
 * @tc.name: NapiUncaughtExceptionCallbackTest_0400
 * @tc.type: FUNC
 * @tc.desc: Test NapiUncaughtExceptionCallback operator().
 * @tc.require: #I6T4K1
 */
HWTEST_F(NapiUncaughtExceptionCallbackTest, NapiUncaughtExceptionCallbackTest_0400, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    options.preload = false;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);
    auto env = jsRuntime->GetNapiEnv();
    EXPECT_NE(env, nullptr);

    // Test with valid code, errorStack is not empty, fuc is not empty.
    napi_value object = nullptr;
    napi_create_object(env, &object);

    std::string errorCode = "This is an error code.";
    std::string errorStack = "TypeError: This is a stack trace.";
    std::string errorFunc = "This is an error func.";
    napi_callback func = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar = nullptr;
        napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
        return thisVar;
    };

    auto task = [](std::string summary, const JsEnv::ErrorObject errorObj) {
        summary += "test";
    };
    napi_value nativeErrorCode = nullptr;
    napi_value nativeErrorStack = nullptr;
    napi_value nativeErrorFunc = nullptr;
    napi_create_string_utf8(env, errorCode.c_str(), errorCode.length(), &nativeErrorCode);
    napi_create_string_utf8(env, errorStack.c_str(), errorStack.length(), &nativeErrorStack);
    napi_create_function(env, errorFunc.c_str(), errorFunc.length(), func, nullptr, &nativeErrorFunc);
    napi_set_named_property(env, object, "code", nativeErrorCode);
    napi_set_named_property(env, object, "stack", nativeErrorStack);
    napi_set_named_property(env, object, "errorfunc", nativeErrorFunc);

    NapiUncaughtExceptionCallback callback(task, nullptr, env);
    callback(object);
}
} // namespace AppExecFwk
} // namespace OHOS
