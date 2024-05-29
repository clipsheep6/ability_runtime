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
static char* g_sharedLibsSonames = nullptr;
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

static void InitSharedLibsSonames()
{
    if (g_sharedLibsSonames != nullptr) {
        return;
    }
    const char* allowList[] = {
        "libc.so",
        "libdl.so",
        "libm.so",
        "libz.so",
        "libclang_rt.asan.so",
        "libclang_rt.tsan.so",
        // z library
        "libace_napi.z.so",
        "libace_ndk.z.so",
        "libbundle_ndk.z.so",
        "libdeviceinfo_ndk.z.so",
        "libEGL.so",
        "libGLESv3.so",
        "libhiappevent_ndk.z.so",
        "libhuks_ndk.z.so",
        "libhukssdk.z.so",
        "libnative_drawing.so",
        "libnative_window.so",
        "libnative_buffer.so",
        "libnative_vsync.so",
        "libOpenSLES.so",
        "libpixelmap_ndk.z.so",
        "libimage_ndk.z.so",
        "libimage_receiver_ndk.z.so",
        "libimage_source_ndk.z.so",
        "librawfile.z.so",
        "libuv.so",
        "libhilog.so",
        "libnative_image.so",
        "libnative_media_adec.so",
        "libnative_media_aenc.so",
        "libnative_media_codecbase.so",
        "libnative_media_core.so",
        "libnative_media_vdec.so",
        "libnative_media_venc.so",
        "libnative_media_avmuxer.so",
        "libnative_media_avdemuxer.so",
        "libnative_media_avsource.so",
        "libnative_avscreen_capture.so",
        "libavplayer.so",
        // adaptor library
        "libohosadaptor.so",
        "libusb_ndk.z.so",
        "libvulkan.so",
    };

    size_t allowListLength = sizeof(allowList) / sizeof(char*);
    int32_t sharedLibsSonamesLength = 1;
    for (size_t i = 0; i < allowListLength; i++) {
        sharedLibsSonamesLength += strlen(allowList[i]) + 1;
    }
    g_sharedLibsSonames = new char[sharedLibsSonamesLength];
    int32_t cursor = 0;
    for (size_t i = 0; i < allowListLength; i++) {
        if (sprintf_s(g_sharedLibsSonames + cursor, sharedLibsSonamesLength - cursor, "%s:", allowList[i]) == -1) {
            delete[] g_sharedLibsSonames;
            g_sharedLibsSonames = nullptr;
            return;
        }
        cursor += strlen(allowList[i]) + 1;
    }
    g_sharedLibsSonames[cursor] = '\0';
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
        if (sprintf_s(g_dlError, sizeof(g_dlError), errMsg.c_str()) == -1) {
            LOGE("Fail to generate error msg.");
            return;
        }
        return;
    }
    if (parent) {
        dlns_inherit((Dl_namespace*)parent, ns, "allow_all_shared_libs");
    }
    Dl_namespace current;
    dlns_get(nullptr, &current);
    if (strcmp(name, "cj_app") != 0) {
        dlns_inherit(ns, &current, "allow_all_shared_libs");
    } else {
        InitSharedLibsSonames();
        dlns_inherit(ns, &current, g_sharedLibsSonames);
        if (g_sharedLibsSonames != nullptr) {
            delete[] g_sharedLibsSonames;
            g_sharedLibsSonames = nullptr;
        }
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
