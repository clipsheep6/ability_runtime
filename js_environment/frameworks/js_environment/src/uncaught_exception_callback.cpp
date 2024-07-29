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
#include <sstream>

#include "hilog_tag_wrapper.h"
#include "native_engine/native_engine.h"
#ifdef SUPPORT_GRAPHICS
#include "ui_content.h"
#endif // SUPPORT_GRAPHICS
#include "unwinder.h"

namespace OHOS {
namespace JsEnv {
constexpr char BACKTRACE[] = "=====================Backtrace========================";

std::string NapiUncaughtExceptionCallback::GetNativeStrFromJsTaggedObj(napi_value obj, const char* key)
{
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Failed to get value from key");
        return "";
    }

    napi_value valueStr = nullptr;
    napi_get_named_property(env_, obj, key, &valueStr);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env_, valueStr, &valueType);
    if (valueType != napi_string) {
        TAG_LOGD(AAFwkTag::JSENV, "Failed to convert value from key");
        return "";
    }

    size_t valueStrBufLength = 0;
    napi_get_value_string_utf8(env_, valueStr, nullptr, 0, &valueStrBufLength);
    auto valueCStr = std::make_unique<char[]>(valueStrBufLength + 1);
    size_t valueStrLength = 0;
    napi_get_value_string_utf8(env_, valueStr, valueCStr.get(), valueStrBufLength + 1, &valueStrLength);
    std::string ret(valueCStr.get(), valueStrLength);
    TAG_LOGD(AAFwkTag::JSENV, "GetNativeStrFromJsTaggedObj Success");
    return ret;
}

void NapiUncaughtExceptionCallback::operator()(napi_value obj)
{
    std::string errorMsg = GetNativeStrFromJsTaggedObj(obj, "message");
    std::string errorName = GetNativeStrFromJsTaggedObj(obj, "name");
    std::string errorStack = GetNativeStrFromJsTaggedObj(obj, "stack");
    std::string topStack = GetNativeStrFromJsTaggedObj(obj, "topstack");
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
        TAG_LOGE(AAFwkTag::JSENV, "errorStack is empty");
        return;
    }
    auto errorPos = SourceMap::GetErrorPos(topStack);
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
    if (errorStack.find(BACKTRACE) != std::string::npos) {
        summary += error + "Stacktrace:\n" + GetBuildId(errorStack);
    } else {
        summary += error + "Stacktrace:\n" + errorStack;
    }
#ifdef SUPPORT_GRAPHICS
    std::string str = Ace::UIContent::GetCurrentUIStackInfo();
    if (!str.empty()) {
        summary.append(str);
    }
#endif // SUPPORT_GRAPHICS
    if (uncaughtTask_) {
        uncaughtTask_(summary, errorObj);
    }
}

std::string NapiUncaughtExceptionCallback::GetBuildId(std::string nativeStack)
{
    std::stringstream ss(nativeStack);
    std::string tempStr;
    std::string addBuildId;
    int i = 0;
    while (std::getline(ss, tempStr)) {
        auto spitlPos = tempStr.rfind(" ");
        if (spitlPos != std::string::npos) {
            auto elfFile = std::make_shared<HiviewDFX::DfxElf>(tempStr.substr(spitlPos + 1));
            std::string buildId = elfFile->GetBuildId();
            if (i != 0 && !buildId.empty()) {
                addBuildId += tempStr + "(" + buildId + ")" + "\n";
            } else {
                addBuildId += tempStr + "\n";
            }
        }
        i++;
    }
    return addBuildId;
}
} // namespace JsEnv
} // namespace OHOS
