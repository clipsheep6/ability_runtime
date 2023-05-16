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
class UncaughtExceptionCallbackTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UncaughtExceptionCallbackTest::SetUpTestCase(void)
{
}

void UncaughtExceptionCallbackTest::TearDownTestCase(void)
{
}

void UncaughtExceptionCallbackTest::SetUp(void)
{
}

void UncaughtExceptionCallbackTest::TearDown(void)
{
}

/**
 * @tc.name: UncaughtExceptionCallbackTest_0100
 * @tc.type: FUNC
 * @tc.desc: Test UncaughtExceptionCallback GetNativeStrFromJsTaggedObj.
 * @tc.require: #I6T4K1
 */
HWTEST_F(UncaughtExceptionCallbackTest, UncaughtExceptionCallbackTest_0100, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    options.preload = false;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);
    // Test with null object
    auto task = [](std::string summary, const JsEnv::ErrorObject errorObj) {
        summary += "test";
    };
    UncaughtExceptionCallback callback(task, nullptr);
    ASSERT_EQ(callback.GetNativeStrFromJsTaggedObj(nullptr, "key"), "");

    // Test with invalid object
    auto engine = jsRuntime->GetNativeEnginePointer();
    EXPECT_NE(engine, nullptr);
    NativeValue* objValue = engine->CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    UncaughtExceptionCallback callback2(task, nullptr);
    ASSERT_EQ(callback2.GetNativeStrFromJsTaggedObj(ConvertNativeValueTo<NativeObject>(objValue), "key"), "");

    // Test with valid object
    std::string errorMsg = "This is an error message.";
    std::string errorName = "TypeError";
    std::string errorStack = "TypeError: This is a stack trace.";
    NativeValue* nativeErrorMsg = engine->CreateString(errorMsg.c_str(), errorMsg.length());
    NativeValue* nativeErrorName = engine->CreateString(errorName.c_str(), errorName.length());
    NativeValue* nativeErrorStack = engine->CreateString(errorStack.c_str(), errorStack.length());

    object->SetProperty("message", nativeErrorMsg);
    object->SetProperty("name", nativeErrorName);
    object->SetProperty("stack", nativeErrorStack);
    UncaughtExceptionCallback callback3(task, nullptr);
    ASSERT_EQ(callback3.GetNativeStrFromJsTaggedObj(object, "message"), errorMsg);
    ASSERT_EQ(callback3.GetNativeStrFromJsTaggedObj(object, "name"), errorName);
    ASSERT_EQ(callback3.GetNativeStrFromJsTaggedObj(object, "stack"), errorStack);
}

/**
 * @tc.name: UncaughtExceptionCallbackTest_0200
 * @tc.type: FUNC
 * @tc.desc: Test UncaughtExceptionCallback operator().
 * @tc.require: #I6T4K1
 */
HWTEST_F(UncaughtExceptionCallbackTest, UncaughtExceptionCallbackTest_0200, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    options.preload = false;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    auto engine = jsRuntime->GetNativeEnginePointer();

    ASSERT_NE(jsRuntime, nullptr);
    // Test with null object
    auto task = [](std::string summary, const JsEnv::ErrorObject errorObj) {
        summary += "test";
    };
    NativeValue* nullValue = engine->CreateUndefined();
    UncaughtExceptionCallback callback(task, nullptr);
    callback(nullValue);

    // Test with valid code, and errorStack is empty
    EXPECT_NE(engine, nullptr);
    NativeValue* objValue = engine->CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::string errorCode = "This is an error code.";
    NativeValue* nativeErrorCode = engine->CreateString(errorCode.c_str(), errorCode.length());
    object->SetProperty("code", nativeErrorCode);

    UncaughtExceptionCallback callback1(task, nullptr);
    callback1(objValue);
}

/**
 * @tc.name: UncaughtExceptionCallbackTest_0300
 * @tc.type: FUNC
 * @tc.desc: Test UncaughtExceptionCallback operator().
 * @tc.require: #I6T4K1
 */
HWTEST_F(UncaughtExceptionCallbackTest, UncaughtExceptionCallbackTest_0300, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    options.preload = false;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);

    // Test with valid code, and errorStack is not empty
    auto engine = jsRuntime->GetNativeEnginePointer();
    EXPECT_NE(engine, nullptr);
    NativeValue* objValue = engine->CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::string errorCode = "This is an error code.";
    std::string errorStack = "TypeError: This is a stack trace.";
    NativeValue* nativeErrorCode = engine->CreateString(errorCode.c_str(), errorCode.length());
    NativeValue* nativeErrorStack = engine->CreateString(errorStack.c_str(), errorStack.length());
    object->SetProperty("code", nativeErrorCode);
    object->SetProperty("stack", nativeErrorStack);
    auto task = [](std::string summary, const JsEnv::ErrorObject errorObj) {
        summary += "test";
    };
    UncaughtExceptionCallback callback(task, nullptr);
    callback(objValue);
}

/**
 * @tc.name: UncaughtExceptionCallbackTest_0400
 * @tc.type: FUNC
 * @tc.desc: Test UncaughtExceptionCallback operator().
 * @tc.require: #I6T4K1
 */
HWTEST_F(UncaughtExceptionCallbackTest, UncaughtExceptionCallbackTest_0400, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    options.preload = false;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);

    // Test with valid code, errorStack is not empty, fuc is not empty.
    auto engine = jsRuntime->GetNativeEnginePointer();
    EXPECT_NE(engine, nullptr);
    NativeValue* objValue = engine->CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::string errorCode = "This is an error code.";
    std::string errorStack = "TypeError: This is a stack trace.";
    std::string errorFunc = "This is an error func.";
    NativeCallback func = [](NativeEngine* engine, NativeCallbackInfo* info) -> NativeValue* {
        return info->thisVar;
    };

    auto task = [](std::string summary, const JsEnv::ErrorObject errorObj) {
        summary += "test";
    };
    NativeValue* nativeErrorCode = engine->CreateString(errorCode.c_str(), errorCode.length());
    NativeValue* nativeErrorStack = engine->CreateString(errorStack.c_str(), errorStack.length());
    NativeValue* nativeErrorFunc = engine->CreateFunction(errorFunc.c_str(), errorFunc.length(), func, nullptr);
    object->SetProperty("code", nativeErrorCode);
    object->SetProperty("stack", nativeErrorStack);
    object->SetProperty("errorfunc", nativeErrorFunc);

    UncaughtExceptionCallback callback(task, nullptr);
    callback(objValue);
}
} // namespace AppExecFwk
} // namespace OHOS
