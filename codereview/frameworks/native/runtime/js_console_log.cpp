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

#include "js_console_log.h"

#include <string>

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr uint32_t JS_CONSOLE_LOG_DOMAIN = 0xFEFE;
constexpr char JS_CONSOLE_LOG_TAG[] = "JsApp";

std::string MakeLogContent(NativeCallbackInfo& info)
{
    std::string content;

    for (size_t i = 0; i < info.argc; i++) {
        NativeValue* value = info.argv[i];
        if (value->TypeOf() != NATIVE_STRING) {
            value = value->ToString();
        }

        NativeString* str = ConvertNativeValueTo<NativeString>(value);
        if (str == nullptr) {
            HILOG_ERROR("Failed to convert to string object");
            continue;
        }

        size_t bufferLen = str->GetLength();
        auto buff = new (std::nothrow) char[bufferLen + 1];
        if (buff == nullptr) {
            HILOG_ERROR("Failed to allocate buffer, size = %zu", bufferLen + 1);
            break;
        }

        size_t strLen = 0;
        str->GetCString(buff, bufferLen + 1, &strLen);
        if (!content.empty()) {
            content.append(" ");
        }
        content.append(buff);
        delete [] buff;
    }

    return content;
}

template<LogLevel LEVEL>
NativeValue* ConsoleLog(NativeEngine* engine, NativeCallbackInfo* info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("engine or callback info is nullptr");
        return nullptr;
    }

    std::string content = MakeLogContent(*info);
    HiLogPrint(LOG_APP, LEVEL, JS_CONSOLE_LOG_DOMAIN, JS_CONSOLE_LOG_TAG, "%{public}s", content.c_str());

    return engine->CreateUndefined();
}
}

void InitConsoleLogModule(NativeEngine& engine, NativeObject& globalObject)
{
    NativeValue* consoleValue = engine.CreateObject();
    NativeObject* consoleObj = ConvertNativeValueTo<NativeObject>(consoleValue);
    if (consoleObj == nullptr) {
        HILOG_ERROR("Failed to create console object");
        return;
    }
    const char *moduleName = "console";
    BindNativeFunction(engine, *consoleObj, "log", moduleName, ConsoleLog<LOG_INFO>);
    BindNativeFunction(engine, *consoleObj, "debug", moduleName, ConsoleLog<LOG_DEBUG>);
    BindNativeFunction(engine, *consoleObj, "info", moduleName, ConsoleLog<LOG_INFO>);
    BindNativeFunction(engine, *consoleObj, "warn", moduleName, ConsoleLog<LOG_WARN>);
    BindNativeFunction(engine, *consoleObj, "error", moduleName, ConsoleLog<LOG_ERROR>);
    BindNativeFunction(engine, *consoleObj, "fatal", moduleName, ConsoleLog<LOG_FATAL>);

    globalObject.SetProperty("console", consoleValue);
}
} // namespace AbilityRuntime
} // namespace OHOS