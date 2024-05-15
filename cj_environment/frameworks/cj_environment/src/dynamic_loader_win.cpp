/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "dynamic_loader.h"

#include <cstdio>
#include <libloaderapi.h>
#include <processenv.h>

constexpr auto ERROR_BUF_SIZE = 255;
static char g_dlError[ERROR_BUF_SIZE] {0};

void* DynamicLoadLibrary(const char* dlPath, int mode)
{
    return LoadLibraryA(dlPath);
}

void* DynamicFindSymbol(void* so, const char* symbol)
{
    return (void*)GetProcAddress((HMODULE)so, symbol);
}

void DynamicFreeLibrary(void* so)
{
    (void)FreeLibrary((HMODULE)so);
}

const char* DynamicGetError()
{
    return g_dlError;
}
