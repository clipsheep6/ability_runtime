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

#include <dlfcn.h>
#include <unistd.h>
#include <filesystem>
#include <regex>

#include "cj_envsetup.h"
#include "hilog_tag_wrapper.h"
#include "hdc_register.h"
#include "connect_server_manager.h"

using namespace OHOS::AbilityRuntime;

#ifdef APP_USE_ARM64
#define APP_LIB_NAME "arm64"
#elif defined(APP_USE_ARM)
#define APP_LIB_NAME "arm"
#elif defined(APP_USE_X86_64)
#define APP_LIB_NAME "x86_64"
#else
#error unsupported platform
#endif

namespace {
const std::string DEBUGGER = "@Debugger";
const std::string SANDBOX_LIB_PATH = "/data/storage/el1/bundle/libs/" APP_LIB_NAME;
const std::string CJ_RT_PATH = SANDBOX_LIB_PATH + "/runtime";
const std::string CJ_LIB_PATH = SANDBOX_LIB_PATH + "/ohos";
const std::string CJ_SYSLIB_PATH = "/system/lib64:/system/lib64/platformsdk:/system/lib64/module:/system/lib64/ndk";
const std::string CJ_CHIPSDK_PATH = "/system/lib64/chipset-pub-sdk";
} // namespace

#define LIB_NAME "libcj_environment.z.so"
#define GET_ENV_INS_NAME "OHOS_GetCJEnvInstance"

namespace OHOS {
CJEnvMethods* CJEnv::LoadInstance()
{
    auto handle = dlopen(LIB_NAME, RTLD_NOW);
    if (!handle) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "dlopen failed %{public}s, %{public}s", LIB_NAME, dlerror());
        return nullptr;
    }
    auto symbol = dlsym(handle, GET_ENV_INS_NAME);
    if (!symbol) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "dlsym failed %{public}s, %{public}s", GET_ENV_INS_NAME, dlerror());
        dlclose(handle);
        return nullptr;
    }
    auto func = reinterpret_cast<CJEnvMethods* (*)()>(symbol);
    return func();
}
}
AppLibPathVec CJRuntime::appLibPaths_;

std::unique_ptr<CJRuntime> CJRuntime::Create(const Options& options)
{
    auto instance = std::make_unique<CJRuntime>();
    if (!instance || !instance->Initialize(options)) {
        return nullptr;
    }
    return instance;
}

void CJRuntime::SetAppLibPath(const AppLibPathMap& appLibPaths)
{
    std::string appPath = "";
    for (const auto& kv : appLibPaths) {
        for (const auto& libPath : kv.second) {
            TAG_LOGD(AAFwkTag::CJRUNTIME, "SetCJAppLibPath: %{public}s.", libPath.c_str());
            CJRuntime::appLibPaths_.emplace_back(libPath);
            appPath += appPath.empty() ? libPath : ":" + libPath;
        }
    }
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "CJEnv LoadInstance failed.");
        return;
    }
    cjEnv->initCJChipSDKNS(CJ_CHIPSDK_PATH);
    cjEnv->initCJAppNS(appPath);
    cjEnv->initCJSDKNS(CJ_RT_PATH + ":" + CJ_LIB_PATH);
    cjEnv->initCJSysNS(CJ_SYSLIB_PATH);
}

bool CJRuntime::Initialize(const Options& options)
{
    if (options.lang != GetLanguage()) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "CJRuntime Initialize fail, language mismatch");
        return false;
    }
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "CJEnv LoadInstance failed.");
        return false;
    }
    if (!cjEnv->startRuntime()) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "start cj runtime failed");
        return false;
    }
    if (!cjEnv->startUIScheduler()) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "start cj ui context failed");
        return false;
    }
    if (!LoadCJAppLibrary(CJRuntime::appLibPaths_)) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "CJRuntime::Initialize fail, load app library fail.");
        return false;
    }
    bundleName_ = options.bundleName;
    instanceId_ = static_cast<uint32_t>(getproctid());
    return true;
}

void CJRuntime::RegisterUncaughtExceptionHandler(const CJUncaughtExceptionInfo& uncaughtExceptionInfo)
{
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "CJEnv LoadInstance failed.");
        return;
    }
    cjEnv->registerCJUncaughtExceptionHandler(uncaughtExceptionInfo);
}

bool CJRuntime::IsCJAbility(const std::string& info)
{
    // in cj application, the srcEntry format should be packageName.AbilityClassName.
    std::string pattern = "^([a-zA-Z0-9_]+\\.)+[a-zA-Z0-9_]+$";
    return std::regex_match(info, std::regex(pattern));
}

bool CJRuntime::LoadCJAppLibrary(const AppLibPathVec& appLibPaths)
{
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "CJEnv LoadInstance failed.");
        return false;
    }
    void* handle = nullptr;
    for (const auto& libPath : appLibPaths) {
        for (auto& itor : std::filesystem::directory_iterator(libPath)) {
            // According to the convention, the names of cj generated products must contain the following keywords
            if (itor.path().string().find("ohos_app_cangjie") == std::string::npos) {
                continue;
            }
            handle = cjEnv->loadCJLibrary(itor.path().c_str());
            if (handle == nullptr) {
                char* errMsg = dlerror();
                TAG_LOGE(AAFwkTag::CJRUNTIME,
                    "Failed to load %{public}s : reason: %{public}s.", itor.path().c_str(), errMsg ? errMsg : "null");
                return false;
            }
        }
    }
    appLibLoaded_ = true;
    return true;
}

void CJRuntime::SetAsanVersion()
{
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "CJEnv LoadInstance failed.");
        return;
    }
    cjEnv->setSanitizerKindRuntimeVersion(SanitizerKind::ASAN);
}

void CJRuntime::SetTsanVersion()
{
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "CJEnv LoadInstance failed.");
        return;
    }
    cjEnv->setSanitizerKindRuntimeVersion(SanitizerKind::TSAN);
}

void CJRuntime::SetHWAsanVersion()
{
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "CJEnv LoadInstance failed.");
        return;
    }
    cjEnv->setSanitizerKindRuntimeVersion(SanitizerKind::HWASAN);
}

void CJRuntime::StartDebugMode(const DebugOption dOption)
{
    if (debugModel_) {
        TAG_LOGI(AAFwkTag::CJRUNTIME, "Already in debug mode");
        return;
    }

    bool isStartWithDebug = dOption.isStartWithDebug;
    bool isDebugApp = dOption.isDebugApp;
    const std::string bundleName = bundleName_;
    std::string inputProcessName = bundleName_ != dOption.processName ? dOption.processName : "";

    TAG_LOGI(AAFwkTag::CJRUNTIME, "StartDebugMode %{public}s", bundleName_.c_str());

    HdcRegister::Get().StartHdcRegister(bundleName_, inputProcessName, isDebugApp,
        [bundleName, isStartWithDebug, isDebugApp](int socketFd, std::string option) {
            TAG_LOGI(AAFwkTag::CJRUNTIME,
                "HdcRegister callback is call, socket fd is %{public}d, option is %{public}s.",
                socketFd, option.c_str());
            if (option.find(DEBUGGER) == std::string::npos) {
                if (!isDebugApp) {
                    ConnectServerManager::Get().StopConnectServer(false);
                }
                ConnectServerManager::Get().SendDebuggerInfo(isStartWithDebug, isDebugApp);
                ConnectServerManager::Get().StartConnectServer(bundleName, socketFd, false);
            } else {
                TAG_LOGE(AAFwkTag::CJRUNTIME, "debugger service unexpected option: %{public}s", option.c_str());
            }
        });
    if (isDebugApp) {
        ConnectServerManager::Get().StartConnectServer(bundleName_, -1, true);
    }
    ConnectServerManager::Get().AddInstance(instanceId_, instanceId_);

    debugModel_ = StartDebugger();
}

bool CJRuntime::StartDebugger()
{
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CJRUNTIME, "CJEnv LoadInstance failed.");
        return false;
    }
    return cjEnv->startDebugger();
}

void CJRuntime::UnLoadCJAppLibrary()
{
    TAG_LOGI(AAFwkTag::CJRUNTIME, "UnLoadCJAppLibrary not support yet");
}
