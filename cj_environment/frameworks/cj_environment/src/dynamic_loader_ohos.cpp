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

#include "cj_hilog.h"
#include <dlfcn.h>
#include <cstdio>
#include <securec.h>
#include <unordered_set>
#include <string>

namespace {
constexpr auto ERROR_BUF_SIZE = 255;
static char g_dlError[ERROR_BUF_SIZE];
static std::unordered_set<std::string> HasInited;
}

enum ErrorCode {
    OUT_OF_MEMORY = 12,
    FILE_EXISTS = 17,
    INVALID_ARGUMENT = 22,
};

static void ReadDlError()
{
    char* errMsg = dlerror();
    if (!errMsg) {
        return;
    }
    auto ends = sprintf_s(g_dlError, sizeof(g_dlError), "%s", errMsg);
    if (ends >= ERROR_BUF_SIZE) {
        g_dlError[ERROR_BUF_SIZE - 1] = '\0';
    } else {
        g_dlError[ends] = '\0';
    }
}

void DynamicInitNamespace(Dl_namespace* ns, void* parent, const char* entries, const char* name)
{
    if (!ns || !entries || !name) {
        LOGE("Invaild args for init namespace.");
        return;
    }
    if (HasInited.count(std::string(name))) {
        return;
    }
    dlns_init(ns, name);
    auto status = dlns_create2(ns, entries, 0);
    std::string errMsg;
    if (status != 0) {
        switch (status) {
            case FILE_EXISTS:
                errMsg = "dlns_create failed: File exists";
                break;
            case INVALID_ARGUMENT:
                errMsg = "dlns_create failed: Invalid argument";
                break;
            case OUT_OF_MEMORY:
                errMsg = "dlns_create failed: Out of memory";
                break;
            default:
                errMsg = "dlns_create failed, status: " + std::to_string(status);
        }
        (void)sprintf_s(g_dlError, sizeof(g_dlError), errMsg.c_str());
        return;
    }
    if (parent) {
        dlns_inherit((Dl_namespace*)parent, ns, "allow_all_shared_libs");
    }
    if (strcmp(name, "cj_app") != 0) {
        Dl_namespace current;
        dlns_get(nullptr, &current);
        dlns_inherit(ns, &current, "allow_all_shared_libs");
    }
    HasInited.insert(std::string(name));
}

void* DynamicLoadLibrary(Dl_namespace *ns, const char* dlPath, int mode)
{
    if (ns == nullptr) {
        dlns_get("cj_app", ns);
    }

    auto result = dlopen_ns(ns, dlPath, mode | RTLD_GLOBAL | RTLD_NOW);
    if (!result) {
        ReadDlError();
    }
    return result;
}

void* DynamicFindSymbol(void* so, const char* symbol)
{
    return dlsym(so, symbol);
}

void DynamicFreeLibrary(void* so)
{
    (void)dlclose(so);
}

const char* DynamicGetError()
{
    return g_dlError;
}
