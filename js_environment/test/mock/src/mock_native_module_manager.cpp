/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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


#include "mock_native_module_manager.h"

#include <gtest/gtest.h>

namespace {
bool g_mockCheckModuleLoadable = false;
LIBHANDLE g_mockLoadModuleLibrary = nullptr;
NativeModule *g_mockFindNativeModuleByDisk = nullptr;
}

void MockFindNativeModuleByDisk(NativeModule *module)
{
    g_mockFindNativeModuleByDisk = module;
}

void MockLoadModuleLibrary(LIBHANDLE handle)
{
    g_mockLoadModuleLibrary = handle;
    GTEST_LOG_(INFO) << g_mockLoadModuleLibrary;
}

void MockCheckModuleLoadable(bool loadable)
{
    g_mockCheckModuleLoadable = loadable;
}

void MockResetModuleManagerState()
{
    g_mockFindNativeModuleByDisk = nullptr;
    g_mockCheckModuleLoadable = false;
    g_mockLoadModuleLibrary = nullptr;
}

NativeModule* NativeModuleManager::FindNativeModuleByDisk(const char* moduleName, const char* path,
    const char* relativePath, bool internal, const bool isAppModule)
{
    GTEST_LOG_(INFO) << g_mockFindNativeModuleByDisk;
    return g_mockFindNativeModuleByDisk;
}

LIBHANDLE NativeModuleManager::LoadModuleLibrary(const char* path, const char* pathKey, const bool isAppModule)
{
    GTEST_LOG_(INFO) << g_mockLoadModuleLibrary;
    return g_mockLoadModuleLibrary;
}

bool ModuleLoadChecker::CheckModuleLoadable(const char* moduleName)
{
    GTEST_LOG_(INFO) << g_mockCheckModuleLoadable;
    return g_mockCheckModuleLoadable;
}
