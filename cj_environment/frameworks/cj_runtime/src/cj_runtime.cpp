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

#include "cj_runtime.h"

#include <string>

#include "cj_hilog.h"
#include "cj_invoker.h"
#ifdef __OHOS__
#include <dlfcn.h>
#endif
#include "dynamic_loader.h"
#ifdef WITH_EVENT_HANDLER
#include "event_handler.h"
#endif

using namespace OHOS;

namespace {
const char DEBUGGER_LIBNAME[] = "libcj_debugger.z.so";
const char DEBUGGER_SYMBOL_NAME[] = "StartDebuggerServer";
const char INIT_CJRUNTIME_SYMBOL_NAME[] = "InitCJRuntime";
const char INIT_UISCHEDULER_SYMBOL_NAME[] = "InitUIScheduler";
const char RUN_UISCHEDULER_SYMBOL_NAME[] = "RunUIScheduler";
const char FINI_CJRUNTIME_SYMBOL_NAME[] = "FiniCJRuntime";
const char INIT_CJLIBRARY_SYMBOL_NAME[] = "InitCJLibrary";
const char REGISTER_EVENTHANDLER_CALLBACKS_NAME[] = "RegisterEventHandlerCallbacks";

using InitCJRuntimeType = int(*)(const struct RuntimeParam*);;
using InitUISchedulerType = void*(*)();
using RunUISchedulerType = int(*)(unsigned long long);
using FiniCJRuntimeType = int(*)();
using InitCJLibraryType = int(*)(const char*);
using RegisterEventHandlerType = void(*)(PostTaskType, HasHigherPriorityType);

#ifdef __OHOS__
const char REGISTER_UNCAUGHT_EXCEPTION_NAME[] = "RegisterUncaughtExceptionHandler";
using RegisterUncaughtExceptionType = void (*)(const CJUncaughtExceptionInfo& handle);
#endif
} // namespace

bool CJRuntime::isRuntimeStarted_ {false};
bool CJRuntime::isUISchedulerStarted_ {false};
CJRuntimeAPI CJRuntime::lazyApis_ {};
void* CJRuntime::uiScheduler_ {nullptr};

const char *CJRuntime::cjAppNSName_ = "cj_app";
const char *CJRuntime::cjSDKNSName_ = "cj_sdk";
const char *CJRuntime::cjSysNSName_ = "cj_system";
const char *CJRuntime::cjChipSDKNSName_ = "cj_chipsdk";

bool CJRuntime::LoadRuntimeApis()
{
    static bool isRuntimeApiLoaded {false};
    if (isRuntimeApiLoaded) {
        return true;
    }

#ifdef __WINDOWS__
#define RTLIB_NAME "libcangjie-runtime.dll"
#else
#define RTLIB_NAME "libcangjie-runtime.so"
#endif
#ifdef __OHOS__
    Dl_namespace ns;
    dlns_get(CJRuntime::cjSDKNSName_, &ns);
    auto dso = DynamicLoadLibrary(&ns, RTLIB_NAME, 1);
#else
    auto dso = DynamicLoadLibrary(RTLIB_NAME, 1);
#endif
    if (!dso) {
        LOGE("load library failed: %{public}s", RTLIB_NAME);
        return false;
    }
#undef RTLIB_NAME
    void* symbol;
    {
        symbol = DynamicFindSymbol(dso, INIT_CJRUNTIME_SYMBOL_NAME);
        if (symbol == nullptr) {
            LOGE("runtime api not found: %{public}s", INIT_CJRUNTIME_SYMBOL_NAME);
            return false;
        }
        lazyApis_.InitCJRuntime = reinterpret_cast<InitCJRuntimeType>(symbol);
    }
    {
        symbol = DynamicFindSymbol(dso, INIT_UISCHEDULER_SYMBOL_NAME);
        if (symbol == nullptr) {
            LOGE("runtime api not found: %{public}s", INIT_UISCHEDULER_SYMBOL_NAME);
            return false;
        }
        lazyApis_.InitUIScheduler = reinterpret_cast<InitUISchedulerType>(symbol);
    }
    {
        symbol = DynamicFindSymbol(dso, RUN_UISCHEDULER_SYMBOL_NAME);
        if (symbol == nullptr) {
            LOGE("runtime api not found: %{public}s", RUN_UISCHEDULER_SYMBOL_NAME);
            return false;
        }
        lazyApis_.RunUIScheduler = reinterpret_cast<RunUISchedulerType>(symbol);
    }
    {
        symbol = DynamicFindSymbol(dso, FINI_CJRUNTIME_SYMBOL_NAME);
        if (symbol == nullptr) {
            LOGE("runtime api not found: %{public}s", FINI_CJRUNTIME_SYMBOL_NAME);
            return false;
        }
        lazyApis_.FiniCJRuntime = reinterpret_cast<FiniCJRuntimeType>(symbol);
    }
    {
        symbol = DynamicFindSymbol(dso, INIT_CJLIBRARY_SYMBOL_NAME);
        if (symbol == nullptr) {
            LOGE("runtime api not found: %{public}s", INIT_CJLIBRARY_SYMBOL_NAME);
            return false;
        }
        lazyApis_.InitCJLibrary = reinterpret_cast<InitCJLibraryType>(symbol);
    }
    {
        symbol = DynamicFindSymbol(dso, REGISTER_EVENTHANDLER_CALLBACKS_NAME);
        if (symbol == nullptr) {
            LOGE("runtime api not found: %{public}s", REGISTER_EVENTHANDLER_CALLBACKS_NAME);
            return false;
        }
        lazyApis_.RegisterEventHandlerCallbacks = reinterpret_cast<RegisterEventHandlerType>(symbol);
    }
#ifdef __OHOS__
    {
        symbol = DynamicFindSymbol(dso, REGISTER_UNCAUGHT_EXCEPTION_NAME);
        if (symbol == nullptr) {
            LOGE("runtime api not found: %{public}s", REGISTER_UNCAUGHT_EXCEPTION_NAME);
            return false;
        }
        lazyApis_.RegisterCJUncaughtExceptionHandler = reinterpret_cast<RegisterUncaughtExceptionType>(symbol);
    }
#endif
    isRuntimeApiLoaded = true;
    return true;
}

void CJRuntime::RegisterCJUncaughtExceptionHandler(const CJUncaughtExceptionInfo& handle)
{
    lazyApis_.RegisterCJUncaughtExceptionHandler(handle);
}

bool CJRuntime::PostTask(void* func)
{
#ifdef WITH_EVENT_HANDLER
    if (func == nullptr) {
        return false;
    }
    void(*task)() = reinterpret_cast<void(*)()>(func);
    static auto handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    bool postDone = handler_->PostTask(task, "spawn-main-task-from-cj", 0, AppExecFwk::EventQueue::Priority::HIGH);
    if (!postDone) {
        LOGE("event handler support cj ui scheduler");
        return false;
    }
 
    return true;
#endif
    return true;
}
 
bool CJRuntime::HasHigherPriorityTask()
{
#ifdef WITH_EVENT_HANDLER
    static auto handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    return handler_->HasPreferEvent(static_cast<int>(AppExecFwk::EventQueue::Priority::HIGH));
#endif
    return false;
}

void CJRuntime::InitCJChipSDKNS(const char *path)
{
#ifdef __OHOS__
    LOGI("InitCJChipSDKNS: %{public}s", path);
    Dl_namespace chip_ndk;
    DynamicInitNamespace(&chip_ndk, nullptr, path, CJRuntime::cjChipSDKNSName_);

    Dl_namespace ndk, current;
    dlns_get(nullptr, &current);
    dlns_get("ndk", &ndk);
    dlns_inherit(&chip_ndk, &ndk, "allow_all_shared_libs");
    dlns_inherit(&chip_ndk, &current, "allow_all_shared_libs");
#endif
}

// Init app namespace
void CJRuntime::InitCJAppNS(const char *path)
{
#ifdef __OHOS__
    LOGI("InitCJAppNS: %{public}s", path);
    Dl_namespace ndk, ns;
    DynamicInitNamespace(&ns, nullptr, path, CJRuntime::cjAppNSName_);
    dlns_get("ndk", &ndk);
    dlns_inherit(&ns, &ndk, "allow_all_shared_libs");
    Dl_namespace current;
    dlns_get(nullptr, &current);
    dlns_inherit(&ndk, &current, "allow_all_shared_libs");
    dlns_inherit(&current, &ndk, "allow_all_shared_libs");
#endif
}

// Init cj sdk namespace
void CJRuntime::InitCJSDKNS(const char *path)
{
#ifdef __OHOS__
    LOGI("InitCJSDKNS: %{public}s", path);
    Dl_namespace cj_app, ns;
    dlns_get(CJRuntime::cjAppNSName_, &cj_app);
    DynamicInitNamespace(&ns, &cj_app, path, CJRuntime::cjSDKNSName_);
#endif
}

// Init cj system namespace
void CJRuntime::InitCJSysNS(const char *path)
{
#ifdef __OHOS__
    LOGI("InitCJSysNS: %{public}s", path);
    Dl_namespace cj_sdk, ndk, ns;
    dlns_get(CJRuntime::cjSDKNSName_, &cj_sdk);
    DynamicInitNamespace(&ns, &cj_sdk, path, CJRuntime::cjSysNSName_);
    dlns_get("ndk", &ndk);
    dlns_inherit(&ns, &ndk, "allow_all_shared_libs");
#endif
}

bool CJRuntime::StartRuntime()
{
    if (isRuntimeStarted_) {
        return true;
    }

    if (!LoadRuntimeApis()) {
        LOGE("LoadRuntimeApis failed");
        return false;
    }

    RuntimeParam rtParams {
        .heapParam = {
            .regionSize = 64,
            .heapSize = 64 * 1024,
            .exemptionThreshold= 0.8,
            .heapUtilization = 0.8,
            .heapGrowth = 0.15,
            .allocationRate = 0,
            .allocationWaitTime = 0,
        },
        .gcParam = {
            .gcThreshold = 0,
            .garbageThreshold = 0,
            .gcInterval = 0,
            .backupGCInterval = 0,
            .gcThreads = 0,
        },
        .logParam = {
            .logLevel = RTLOG_ERROR,
        },
        .coParam = {
            .thStackSize = 2 * 1024,
            .coStackSize = 2 * 1024,
            .processorNum = 8,
        }
    };

    auto status = lazyApis_.InitCJRuntime(&rtParams);
    if (status != E_OK) {
        LOGE("init cj runtime failed: %{public}d", status);
        return false;
    }

    lazyApis_.RegisterEventHandlerCallbacks(PostTask, HasHigherPriorityTask);

    isRuntimeStarted_ = true;
    return true;
}

void CJRuntime::StopRuntime()
{
    if (!isRuntimeStarted_) {
        return;
    }

    if (isUISchedulerStarted_) {
        StopUIScheduler();
    }

    auto code = lazyApis_.FiniCJRuntime();
    if (code == E_OK) {
        isRuntimeStarted_ = false;
    }
}

bool CJRuntime::StartUIScheduler()
{
    if (isUISchedulerStarted_) {
        return true;
    }

    uiScheduler_ = lazyApis_.InitUIScheduler();
    if (!uiScheduler_) {
        LOGE("init cj ui scheduler failed");
        return false;
    }

    isUISchedulerStarted_ = true;
    return true;
}

void CJRuntime::StopUIScheduler()
{
    isUISchedulerStarted_ = false;
}

void* CJRuntime::LoadCJLibrary(const char* dlName)
{
    if (!StartRuntime()) {
        LOGE("StartRuntime failed");
        return nullptr;
    }
    auto handle = LoadCJLibrary(APP, dlName);
    if (!handle) {
        LOGE("load cj library failed: %{public}s", DynamicGetError());
        return nullptr;
    }

    LOGI("LoadCJLibrary InitCJLibrary: %{public}s", dlName);
    auto status = lazyApis_.InitCJLibrary(dlName);
    if (status != E_OK) {
        LOGE("InitCJLibrary failed: %{public}s", dlName);
        return nullptr;
    }

    return handle;
}

void* CJRuntime::LoadCJLibrary(OHOS::CJRuntime::LibraryKind kind, const char* dlName)
{
#ifdef __OHOS__
    Dl_namespace ns;
    switch (kind) {
        case APP:
            dlns_get(CJRuntime::cjAppNSName_, &ns);
            break;
        case SYSTEM:
            dlns_get(CJRuntime::cjSysNSName_, &ns);
            break;
        case SDK:
            dlns_get(CJRuntime::cjSDKNSName_, &ns);
            break;
    }
    auto handle = DynamicLoadLibrary(&ns, dlName, 1);
#else
    auto handle = DynamicLoadLibrary(dlName, 1);
#endif
    if (!handle) {
        LOGE("load cj library failed: %{public}s", DynamicGetError());
        return nullptr;
    }
    return handle;
}

void* CJRuntime::GetSymbol(void* dso, const char* symbol)
{
    return DynamicFindSymbol(dso, symbol);
}

bool CJRuntime::StartDebugger()
{
#ifdef __OHOS__
    Dl_namespace ns;
    dlns_get(CJRuntime::cjSysNSName_, &ns);
    auto handle = DynamicLoadLibrary(&ns, DEBUGGER_LIBNAME, 0);
#else
    auto handle = DynamicLoadLibrary(DEBUGGER_LIBNAME, 0);
#endif
    if (!handle) {
        LOGE("failed to load library: %{public}s", DEBUGGER_LIBNAME);
        return false;
    }
    auto symbol = DynamicFindSymbol(handle, DEBUGGER_SYMBOL_NAME);
    if (!symbol) {
        LOGE("failed to find symbol: %{public}s", DEBUGGER_SYMBOL_NAME);
        return false;
    }
    auto func = reinterpret_cast<bool (*)(int, const std::string&)>(symbol);
    std::string name = "PandaDebugger";
    func(0, name);

    return true;
}

