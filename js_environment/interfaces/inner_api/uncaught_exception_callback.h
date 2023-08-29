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

#ifndef OHOS_ABILITY_UNCAUGHT_EXCEPTION_CALLBACK_H
#define OHOS_ABILITY_UNCAUGHT_EXCEPTION_CALLBACK_H

#include <string>

#include "native_engine/native_engine.h"
#include "source_map.h"
#include "source_map_operator.h"

namespace OHOS {
namespace JsEnv {
struct ErrorObject {
    std::string name;
    std::string message;
    std::string stack;
};

struct UncaughtExceptionInfo {
    std::string hapPath;
    std::function<void(std::string summary, const JsEnv::ErrorObject errorObj)> uncaughtTask;
};

template<class T>
inline T* ConvertNativeValueTo(NativeValue* value)
{
    return (value != nullptr) ? static_cast<T*>(value->GetInterface(T::INTERFACE_ID)) : nullptr;
}

class UncaughtExceptionCallback final {
public:
    UncaughtExceptionCallback(
        std::function<void(const std::string summary, const JsEnv::ErrorObject errorObj)> uncaughtTask,
        std::shared_ptr<SourceMapOperator> sourceMapOperator)
        : uncaughtTask_(uncaughtTask), sourceMapOperator_(sourceMapOperator)
    {}

    ~UncaughtExceptionCallback() = default;

    void operator()(NativeValue* value);

    std::string GetNativeStrFromJsTaggedObj(NativeObject* obj, const char* key);

private:
    std::function<void(std::string summary, const JsEnv::ErrorObject errorObj)> uncaughtTask_;
    std::shared_ptr<SourceMapOperator> sourceMapOperator_ = nullptr;
};
} // namespace JsEnv
} // namespace OHOS
#endif // OHOS_ABILITY_UNCAUGHT_EXCEPTION_CALLBACK_H
