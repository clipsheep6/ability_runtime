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

#ifndef OHOS_ABILITY_RUNTIME_CJ_RUNTIME_H
#define OHOS_ABILITY_RUNTIME_CJ_RUNTIME_H

#include <functional>

#ifdef WINDOWS_PLATFORM
#define CJ_EXPORT __declspec(dllexport)
#else
#define CJ_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
struct CJRuntimeAPI;

struct CJErrorObject {
    const char* name;
    const char* message;
    const char* stack;
};

struct CJUncaughtExceptionInfo {
    const char* hapPath;
    std::function<void(const char* summary, const CJErrorObject errorObj)> uncaughtTask;
};

class CJ_EXPORT CJRuntime final {
public:
    static bool IsRuntimeStarted()
    {
        return isRuntimeStarted_;
    }
    static void InitCJAppNS(const char *path);
    static void InitCJSDKNS(const char *path);
    static void InitCJSysNS(const char *path);
    static void InitCJChipSDKNS(const char *path);
    static bool StartRuntime();
    static void StopRuntime();
    static void RegisterCJUncaughtExceptionHandler(const CJUncaughtExceptionInfo& handle);
    static bool IsUISchedulerStarted()
    {
        return isUISchedulerStarted_;
    }
    static bool StartUIScheduler();
    static void StopUIScheduler();
    enum LibraryKind {
        SYSTEM,
        SDK,
        APP,
    };
    static void* LoadCJLibrary(const char* dlName);
    static void* LoadCJLibrary(LibraryKind kind, const char* dlName);
    static void* GetUIScheduler()
    {
        if (!isUISchedulerStarted_) {
            return nullptr;
        }
        return uiScheduler_;
    }
    static void* GetSymbol(void* dso, const char* symbol);
    static bool StartDebugger();
    static bool PostTask(void* func);
    static bool HasHigherPriorityTask();

    static const char *cjAppNSName_;
    static const char *cjSDKNSName_;
    static const char *cjSysNSName_;
    static const char *cjChipSDKNSName_;
private:
    static CJRuntimeAPI lazyApis_;
    static bool isRuntimeStarted_;
    static bool isUISchedulerStarted_;
    static void* uiScheduler_;
    static bool LoadRuntimeApis();
};
}

#endif //OHOS_ABILITY_RUNTIME_CJ_RUNTIME_H
