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

#include "util/logger.h"

#include <cstdio>

namespace OHOS {
namespace Idl {
int Logger::level_ = DEBUG;

void Logger::D(const char* tag, const char* format, ...)
{
    if (level_ > DEBUG) return;

    va_list args;
    va_start(args, format);
    Log(tag, format, args);
    va_end(args);
}

void Logger::E(const char* tag, const char* format, ...)
{
    if (level_ > ERROR) return;

    va_list args;
    va_start(args, format);
    Err(tag, format, args);
    va_end(args);
}

void Logger::V(const char* tag, const char* format, ...)
{
    if (level_ > VERBOSE) return;

    va_list args;
    va_start(args, format);
    Log(tag, format, args);
    va_end(args);
}

void Logger::Log(const char* tag, const char* format, va_list args)
{
    printf("[%s]: ", tag);
    vprintf(format, args);
    printf("\n");
}

void Logger::Err(const char* tag, const char* format, va_list args)
{
    fprintf(stderr, "[%s]: ", tag);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}
}
}
