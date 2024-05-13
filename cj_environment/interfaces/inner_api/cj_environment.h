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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ENVIRONMENT_H
#define OHOS_ABILITY_RUNTIME_CJ_ENVIRONMENT_H

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

using TaskFuncType = void(*)();

class CJ_EXPORT CJEnvironment final {
public:
    static CJEnvironment* GetInstance();

    bool IsRuntimeStarted()
    {
        return isRuntimeStarted_;
    }
    void InitCJAppNS(const std::string& path);
    void InitCJSDKNS(const std::string& path);
    void InitCJSysNS(const std::string& path);
    void InitCJChipSDKNS(const std::string& path);
    bool StartRuntime();
    void StopRuntime();
    void RegisterCJUncaughtExceptionHandler(const CJUncaughtExceptionInfo& handle);
    bool IsUISchedulerStarted()
    {
        return isUISchedulerStarted_;
    }
    bool StartUIScheduler();
    void StopUIScheduler();
    enum LibraryKind {
        SYSTEM,
        SDK,
        APP,
    };
    void* LoadCJLibrary(const char* dlName);
    void* LoadCJLibrary(LibraryKind kind, const char* dlName);
    void UnLoadCJLibrary(void* handle);
    void* GetUIScheduler()
    {
        if (!isUISchedulerStarted_) {
            return nullptr;
        }
        return uiScheduler_;
    }
    void* GetSymbol(void* dso, const char* symbol);
    bool StartDebugger();
    bool PostTask(TaskFuncType task);
    bool HasHigherPriorityTask();

    static const char *cjAppNSName;
    static const char *cjSDKNSName;
    static const char *cjSysNSName;
    static const char *cjChipSDKNSName;
private:
    bool LoadRuntimeApis();
    static CJRuntimeAPI lazyApis_;
    bool isRuntimeStarted_{false};
    bool isUISchedulerStarted_{false};
    void* uiScheduler_ {nullptr};
};
}

#endif //OHOS_ABILITY_RUNTIME_CJ_ENVIRONMENT_H
