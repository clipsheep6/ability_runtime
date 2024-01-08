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
#include "uncaught_exception_callback.h"

#include <string>

#include "js_env_logger.h"
#include "native_engine/native_engine.h"
#include "ui_content.h"

namespace OHOS {
namespace JsEnv {
std::string NapiUncaughtExceptionCallback::GetNativeStrFromJsTaggedObj(napi_value obj, const char* key)
{
    if (obj == nullptr) {
        JSENV_LOG_E("Failed to get value from key.");
        return "";
    }

    napi_value valueStr = nullptr;
    napi_get_named_property(env_, obj, key, &valueStr);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env_, valueStr, &valueType);
    if (valueType != napi_string) {
        JSENV_LOG_E("Failed to convert value from key.");
        return "";
    }

    size_t valueStrBufLength = 0;
    napi_get_value_string_utf8(env_, valueStr, nullptr, 0, &valueStrBufLength);
    auto valueCStr = std::make_unique<char[]>(valueStrBufLength + 1);
    size_t valueStrLength = 0;
    napi_get_value_string_utf8(env_, valueStr, valueCStr.get(), valueStrBufLength + 1, &valueStrLength);
    std::string ret(valueCStr.get(), valueStrLength);
    JSENV_LOG_D("GetNativeStrFromJsTaggedObj Success.");
    return ret;
}

void NapiUncaughtExceptionCallback::operator()(napi_value obj)
{
    std::string errorMsg = GetNativeStrFromJsTaggedObj(obj, "message");
    std::string errorName = GetNativeStrFromJsTaggedObj(obj, "name");
    std::string errorStack = GetNativeStrFromJsTaggedObj(obj, "stack");
    std::string originalStack = GetNativeStrFromJsTaggedObj(obj, "originalStack");
    std::string summary = "Error name:" + errorName + "\n";
    summary += "Error message:" + errorMsg + "\n";
    const JsEnv::ErrorObject errorObj = {
        .name = errorName,
        .message = errorMsg,
        .stack = errorStack
    };
    bool hasProperty = false;
    napi_has_named_property(env_, obj, "code", &hasProperty);
    if (hasProperty) {
        std::string errorCode = GetNativeStrFromJsTaggedObj(obj, "code");
        summary += "Error code:" + errorCode + "\n";
    }
    if (errorStack.empty()) {
        JSENV_LOG_E("errorStack is empty");
        return;
    }
    auto errorPos = SourceMap::GetErrorPos(originalStack);
    std::string error;
    if (obj != nullptr) {
        napi_value fuc = nullptr;
        napi_get_named_property(env_, obj, "errorfunc", &fuc);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env_, fuc, &valueType);
        if (valueType == napi_function) {
            error = reinterpret_cast<NativeEngine*>(env_)->GetSourceCodeInfo(fuc, errorPos);
        }
    }
    if (sourceMapOperator_ == nullptr) {
        JSENV_LOG_E("sourceMapOperator_ is empty");
        return;
    }
    summary += error + "Stacktrace:\n" + errorStack;
    std::string str = Ace::UIContent::GetCurrentUIStackInfo();
    if (!str.empty()) {
        summary.append(str);
    }
    if (uncaughtTask_) {
        uncaughtTask_(summary, errorObj);
    }
}
} // namespace JsEnv
} // namespace OHOS
