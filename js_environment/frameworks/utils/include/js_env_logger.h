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

#ifndef OHOS_ABILITY_JS_ENVIRONMENT_JS_ENV_LOGGER_H
#define OHOS_ABILITY_JS_ENVIRONMENT_JS_ENV_LOGGER_H

namespace OHOS {
namespace JsEnv {
enum class JsEnvLogLevel {
    DEBUG = 0,
    INFO,
    WARN,
    ERROR,
    FATAL
};

struct JsEnvLogger final {
    static void(*logger)(JsEnvLogLevel level, const char* fileName, const char* functionName, int line,
        const char* fmt, ...);
};

void(*JsEnvLogger::logger)(JsEnvLogLevel level, const char* fileName, const char* functionName, int line,
        const char* fmt, ...) = nullptr;

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define JSENV_LOG_D(fmt, ...) \
    JsEnvLogger::logger(JsEnvLogLevel::DEBUG, __FILENAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define JSENV_LOG_I(fmt, ...) \
    JsEnvLogger::logger(JsEnvLogLevel::INFO, __FILENAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define JSENV_LOG_W(fmt, ...) \
    JsEnvLogger::logger(JsEnvLogLevel::WARN, __FILENAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define JSENV_LOG_E(fmt, ...) \
    JsEnvLogger::logger(JsEnvLogLevel::ERROR, __FILENAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define JSENV_LOG_F(fmt, ...) \
    JsEnvLogger::logger(JsEnvLogLevel::FATAL, __FILENAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
} // namespace JsEnv
} // namespace OHOS
#endif // OHOS_ABILITY_JS_ENVIRONMENT_JS_ENV_LOGGER_H