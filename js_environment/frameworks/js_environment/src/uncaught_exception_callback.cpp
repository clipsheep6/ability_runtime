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

namespace OHOS {
namespace JsEnv {
std::string UncaughtExceptionCallback::GetNativeStrFromJsTaggedObj(NativeObject* obj, const char* key)
{
    if (obj == nullptr) {
        JSENV_LOG_E("Failed to get value from key.");
        return "";
    }

    NativeValue* value = obj->GetProperty(key);
    NativeString* valueStr = JsEnv::ConvertNativeValueTo<NativeString>(value);
    if (valueStr == nullptr) {
        JSENV_LOG_E("Failed to convert value from key.");
        return "";
    }

    size_t valueStrBufLength = valueStr->GetLength();
    size_t valueStrLength = 0;
    auto valueCStr = std::make_unique<char[]>(valueStrBufLength + 1);

    valueStr->GetCString(valueCStr.get(), valueStrBufLength + 1, &valueStrLength);
    std::string ret(valueCStr.get(), valueStrLength);
    JSENV_LOG_D("GetNativeStrFromJsTaggedObj Success.");
    return ret;
}

void UncaughtExceptionCallback::operator()(NativeValue* value)
{
    NativeObject* obj = JsEnv::ConvertNativeValueTo<NativeObject>(value);
    std::string errorMsg = GetNativeStrFromJsTaggedObj(obj, "message");
    std::string errorName = GetNativeStrFromJsTaggedObj(obj, "name");
    std::string errorStack = GetNativeStrFromJsTaggedObj(obj, "stack");
    std::string summary = "Error message:" + errorMsg + "\n";
    const JsEnv::ErrorObject errorObj = {
        .name = errorName,
        .message = errorMsg,
        .stack = errorStack
    };
    if (obj != nullptr && obj->HasProperty("code")) {
        std::string errorCode = GetNativeStrFromJsTaggedObj(obj, "code");
        summary += "Error code:" + errorCode + "\n";
    }
    if (errorStack.empty()) {
        JSENV_LOG_E("errorStack is empty");
        return;
    }
    auto errorPos = SourceMap::GetErrorPos(errorStack);
    std::string error;
    if (obj != nullptr) {
        NativeValue* value = obj->GetProperty("errorfunc");
        NativeFunction* fuc = JsEnv::ConvertNativeValueTo<NativeFunction>(value);
        if (fuc != nullptr) {
            error = fuc->GetSourceCodeInfo(errorPos);
        }
    }
    if (sourceMapOperator_ == nullptr) {
        JSENV_LOG_E("sourceMapOperator_ is empty");
        return;
    }
    summary += error + "Stacktrace:\n" + sourceMapOperator_->TranslateBySourceMap(errorStack);
    if (uncaughtTask_) {
        uncaughtTask_(summary, errorObj);
    }
}
} // namespace JsEnv
} // namespace OHOS
