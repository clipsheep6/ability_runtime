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

#ifndef OHOS_ABILITY_RUNTIME_TIME_UTIL_H
#define OHOS_ABILITY_RUNTIME_TIME_UTIL_H

#include <cinttypes>
#include <sys/stat.h>
#include <time.h>

namespace OHOS::AbilityRuntime {
namespace TimeUtil {
// NANOSECONDS mean 10^9 nano second
constexpr int64_t NANOSECONDS = 1000000000;
// MICROSECONDS mean 10^6 milli second
constexpr int64_t MICROSECONDS = 1000000;
constexpr int64_t SEC_TO_MILLISEC = 1000;
constexpr int64_t MAX_TIME_BUFF = 64; // 64 : for example 2021-05-27-01-01-01

[[maybe_unused]] static int64_t SystemTimeMillisecond()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS;
}

[[maybe_unused]] static std::string FormatTime(const std::string &format)
{
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    auto timestamp = millisecs.count();
    std::time_t tt = static_cast<std::time_t>(timestamp / SEC_TO_MILLISEC);
    std::tm t = *std::localtime(&tt);
    char buffer[MAX_TIME_BUFF] = {0};
    std::strftime(buffer, sizeof(buffer), format.c_str(), &t);
    return std::string(buffer);
}
}  // namespace TimeUtil
}  // namespace OHOS::AbilityRuntime
#endif  // OHOS_ABILITY_RUNTIME_TIME_UTIL_H