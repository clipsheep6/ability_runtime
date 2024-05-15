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

#include "cj_environment.h"

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

#ifdef WITH_EVENT_HANDLER
static std::shared_ptr<AppExecFwk::EventHandler> g_handler = nullptr;
#endif

bool LoadSymbolInitCJRuntime(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, INIT_CJRUNTIME_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", INIT_CJRUNTIME_SYMBOL_NAME);
        return false;
    }
    apis.InitCJRuntime = reinterpret_cast<InitCJRuntimeType>(symbol);
    return true;
}

bool LoadSymbolInitUIScheduler(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, INIT_UISCHEDULER_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", INIT_UISCHEDULER_SYMBOL_NAME);
        return false;
    }
    apis.InitUIScheduler = reinterpret_cast<InitUISchedulerType>(symbol);
    return true;
}

bool LoadSymbolRunUIScheduler(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, RUN_UISCHEDULER_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", RUN_UISCHEDULER_SYMBOL_NAME);
        return false;
    }
    apis.RunUIScheduler = reinterpret_cast<RunUISchedulerType>(symbol);
    return true;
}

bool LoadSymbolFiniCJRuntime(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, FINI_CJRUNTIME_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", FINI_CJRUNTIME_SYMBOL_NAME);
        return false;
    }
    apis.FiniCJRuntime = reinterpret_cast<FiniCJRuntimeType>(symbol);
    return true;
}

bool LoadSymbolInitCJLibrary(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, INIT_CJLIBRARY_SYMBOL_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", INIT_CJLIBRARY_SYMBOL_NAME);
        return false;
    }
    apis.InitCJLibrary = reinterpret_cast<InitCJLibraryType>(symbol);
    return true;
}

bool LoadSymbolRegisterEventHandlerCallbacks(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, REGISTER_EVENTHANDLER_CALLBACKS_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", REGISTER_EVENTHANDLER_CALLBACKS_NAME);
        return false;
    }
    apis.RegisterEventHandlerCallbacks = reinterpret_cast<RegisterEventHandlerType>(symbol);
    return true;
}

#ifdef __OHOS__
bool LoadSymbolRegisterCJUncaughtExceptionHandler(void* handle, CJRuntimeAPI& apis)
{
    auto symbol = DynamicFindSymbol(handle, REGISTER_UNCAUGHT_EXCEPTION_NAME);
    if (symbol == nullptr) {
        LOGE("runtime api not found: %{public}s", REGISTER_UNCAUGHT_EXCEPTION_NAME);
        return false;
    }
    apis.RegisterCJUncaughtExceptionHandler = reinterpret_cast<RegisterUncaughtExceptionType>(symbol);
    return true;
}
#endif

bool PostTaskWrapper(void* func)
{
    return CJEnvironment::GetInstance()->PostTask(reinterpret_cast<TaskFuncType>(func));
}

bool HasHigherPriorityTaskWrapper()
{
    return CJEnvironment::GetInstance()->HasHigherPriorityTask();
}
} // namespace

const char *CJEnvironment::cjAppNSName = "cj_app";
const char *CJEnvironment::cjSDKNSName = "cj_sdk";
const char *CJEnvironment::cjSysNSName = "cj_system";
const char *CJEnvironment::cjChipSDKNSName = "cj_chipsdk";

CJRuntimeAPI CJEnvironment::lazyApis_ {};

CJEnvironment* CJEnvironment::GetInstance()
{
    static CJEnvironment cjEnv;
#ifdef WITH_EVENT_HANDLER
    if (g_handler == nullptr) {
        g_handler = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    }
#endif
    return &cjEnv;
}

bool CJEnvironment::LoadRuntimeApis()
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
    dlns_get(CJEnvironment::cjSDKNSName, &ns);
    auto dso = DynamicLoadLibrary(&ns, RTLIB_NAME, 1);
#else
    auto dso = DynamicLoadLibrary(RTLIB_NAME, 1);
#endif
    if (!dso) {
        LOGE("load library failed: %{public}s", RTLIB_NAME);
        return false;
    }
#undef RTLIB_NAME
    if (!LoadSymbolInitCJRuntime(dso, lazyApis_) ||
        !LoadSymbolInitUIScheduler(dso, lazyApis_) ||
        !LoadSymbolRunUIScheduler(dso, lazyApis_) ||
        !LoadSymbolFiniCJRuntime(dso, lazyApis_) ||
        !LoadSymbolInitCJLibrary(dso, lazyApis_) ||
        !LoadSymbolRegisterEventHandlerCallbacks(dso, lazyApis_)) {
        LOGE("load symbol failed");
        return false;
    }
#ifdef __OHOS__
    if (!LoadSymbolRegisterCJUncaughtExceptionHandler(dso, lazyApis_)) {
        LOGE("load symbol RegisterCJUncaughtExceptionHandler failed");
        return false;
    }
#endif
    isRuntimeApiLoaded = true;
    return true;
}

void CJEnvironment::RegisterCJUncaughtExceptionHandler(const CJUncaughtExceptionInfo& handle)
{
    lazyApis_.RegisterCJUncaughtExceptionHandler(handle);
}

bool CJEnvironment::PostTask(TaskFuncType task)
{
#ifdef WITH_EVENT_HANDLER
    if (task == nullptr) {
        LOGE("null task could not be posted");
        return false;
    }

    bool postDone = g_handler->PostTask(task, "spawn-main-task-from-cj", 0, AppExecFwk::EventQueue::Priority::HIGH);
    if (!postDone) {
        LOGE("event handler support cj ui scheduler");
        return false;
    }
    return true;
#endif
    return true;
}

bool CJEnvironment::HasHigherPriorityTask()
{
#ifdef WITH_EVENT_HANDLER
    return g_handler->HasPreferEvent(static_cast<int>(AppExecFwk::EventQueue::Priority::HIGH));
#endif
    return false;
}

void CJEnvironment::InitCJChipSDKNS(const std::string& path)
{
#ifdef __OHOS__
    LOGI("InitCJChipSDKNS: %{public}s", path.c_str());
    Dl_namespace chip_ndk;
    DynamicInitNamespace(&chip_ndk, nullptr, path.c_str(), CJEnvironment::cjChipSDKNSName);

    Dl_namespace ndk;
    Dl_namespace current;
    dlns_get(nullptr, &current);
    dlns_get("ndk", &ndk);
    dlns_inherit(&chip_ndk, &ndk, "allow_all_shared_libs");
    dlns_inherit(&chip_ndk, &current, "allow_all_shared_libs");
#endif
}

// Init app namespace
void CJEnvironment::InitCJAppNS(const std::string& path)
{
#ifdef __OHOS__
    LOGI("InitCJAppNS: %{public}s", path.c_str());
    Dl_namespace ndk;
    Dl_namespace ns;
    DynamicInitNamespace(&ns, nullptr, path.c_str(), CJEnvironment::cjAppNSName);
    dlns_get("ndk", &ndk);
    dlns_inherit(&ns, &ndk, "allow_all_shared_libs");
    Dl_namespace current;
    dlns_get(nullptr, &current);
    dlns_inherit(&ndk, &current, "allow_all_shared_libs");
    dlns_inherit(&current, &ndk, "allow_all_shared_libs");
#endif
}

// Init cj sdk namespace
void CJEnvironment::InitCJSDKNS(const std::string& path)
{
#ifdef __OHOS__
    LOGI("InitCJSDKNS: %{public}s", path.c_str());
    Dl_namespace cj_app;
    Dl_namespace ns;
    dlns_get(CJEnvironment::cjAppNSName, &cj_app);
    DynamicInitNamespace(&ns, &cj_app, path.c_str(), CJEnvironment::cjSDKNSName);
#endif
}

// Init cj system namespace
void CJEnvironment::InitCJSysNS(const std::string& path)
{
#ifdef __OHOS__
    LOGI("InitCJSysNS: %{public}s", path.c_str());
    Dl_namespace cj_sdk;
    Dl_namespace ndk;
    Dl_namespace ns;
    dlns_get(CJEnvironment::cjSDKNSName, &cj_sdk);
    DynamicInitNamespace(&ns, &cj_sdk, path.c_str(), CJEnvironment::cjSysNSName);
    dlns_get("ndk", &ndk);
    dlns_inherit(&ns, &ndk, "allow_all_shared_libs");
#endif
}

bool CJEnvironment::StartRuntime()
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

    lazyApis_.RegisterEventHandlerCallbacks(PostTaskWrapper, HasHigherPriorityTaskWrapper);

    isRuntimeStarted_ = true;
    return true;
}

void CJEnvironment::StopRuntime()
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

bool CJEnvironment::StartUIScheduler()
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

void CJEnvironment::StopUIScheduler()
{
    isUISchedulerStarted_ = false;
}

void* CJEnvironment::LoadCJLibrary(const char* dlName)
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
        UnLoadCJLibrary(handle);
        return nullptr;
    }

    return handle;
}

void* CJEnvironment::LoadCJLibrary(OHOS::CJEnvironment::LibraryKind kind, const char* dlName)
{
#ifdef __OHOS__
    Dl_namespace ns;
    switch (kind) {
        case APP:
            dlns_get(CJEnvironment::cjAppNSName, &ns);
            break;
        case SYSTEM:
            dlns_get(CJEnvironment::cjSysNSName, &ns);
            break;
        case SDK:
            dlns_get(CJEnvironment::cjSDKNSName, &ns);
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

void CJEnvironment::UnLoadCJLibrary(void* handle)
{
    DynamicFreeLibrary(handle);
}

void* CJEnvironment::GetSymbol(void* dso, const char* symbol)
{
    return DynamicFindSymbol(dso, symbol);
}

bool CJEnvironment::StartDebugger()
{
#ifdef __OHOS__
    Dl_namespace ns;
    dlns_get(CJEnvironment::cjSysNSName, &ns);
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
        DynamicFreeLibrary(handle);
        return false;
    }
    auto func = reinterpret_cast<bool (*)(int, const std::string&)>(symbol);
    std::string name = "PandaDebugger";
    func(0, name);
    return true;
}

