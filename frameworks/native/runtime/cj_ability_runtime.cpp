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

#include "cj_ability_runtime.h"

#include <dlfcn.h>
#include <unistd.h>
#include <filesystem>

#include "cj_runtime.h"
#include "hilog_wrapper.h"
#include "hdc_register.h"
#include "connect_server_manager.h"

using namespace OHOS::AbilityRuntime;

namespace {
const std::string DEBUGGER = "@Debugger";
} // namespace

AppLibPathVec CJAbilityRuntime::appLibPaths_;

std::unique_ptr<CJAbilityRuntime> CJAbilityRuntime::Create(const Options& options)
{
    auto instance = std::make_unique<CJAbilityRuntime>();
    if (!instance || !instance->Initialize(options)) {
        return nullptr;
    }
    return instance;
}

void CJAbilityRuntime::SetAppLibPath(const AppLibPathMap& appLibPaths)
{
    std::string appPath = "";
    for (const auto& kv : appLibPaths) {
        for (const auto& libPath : kv.second) {
            HILOG_INFO("SetCJAppLibPath: %{public}s.", libPath.c_str());
            CJAbilityRuntime::appLibPaths_.emplace_back(libPath);
            appPath += appPath.empty() ? libPath : ":" + libPath;
        }
    }
    CJRuntime::InitCJChipSDKNS("/system/lib64/chipset-pub-sdk");
    CJRuntime::InitCJAppNS(appPath.c_str());
    CJRuntime::InitCJSDKNS("/data/storage/el1/bundle/libs/arm64/ohos:/data/storage/el1/bundle/libs/arm64/runtime");
    CJRuntime::InitCJSysNS("/system/lib64:/system/lib64/platformsdk:/system/lib64/module:/system/lib64/ndk");
}

bool CJAbilityRuntime::Initialize(const Options& options)
{
    if (options.lang != GetLanguage()) {
        HILOG_ERROR("CJAbilityRuntime Initialize fail, language mismatch");
        return false;
    }
    if (!OHOS::CJRuntime::StartRuntime()) {
        HILOG_ERROR("start cj runtime failed");
        return false;
    }
    if (!OHOS::CJRuntime::StartUIScheduler()) {
        HILOG_ERROR("start cj ui context failed");
        return false;
    }
    if (!LoadCJAppLibrary(CJAbilityRuntime::appLibPaths_)) {
        HILOG_ERROR("CJAbilityRuntime::Initialize fail, load app library fail.");
        return false;
    }
    bundleName_ = options.bundleName;
    instanceId_ = static_cast<uint32_t>(getproctid());
    return true;
}

void CJAbilityRuntime::RegisterUncaughtExceptionHandler(const CJUncaughtExceptionInfo& uncaughtExceptionInfo)
{
    HILOG_INFO("RegisterUncaughtExceptionHandler not support yet");
}

bool CJAbilityRuntime::LoadCJAppLibrary(const AppLibPathVec& appLibPaths)
{
    void* handle = nullptr;
    for (const auto& libPath : appLibPaths) {
        for (auto& itor : std::filesystem::directory_iterator(libPath)) {
            // According to the convention, the names of Cangjie generated products must contain the following keywords
            if (itor.path().string().find("ohos_app_cangjie") == std::string::npos) {
                continue;
            }
            handle = OHOS::CJRuntime::LoadCJLibrary(itor.path().c_str());
            if (handle == nullptr) {
                HILOG_ERROR(
                    "Failed to load cj app library %{public}s : reason: %{public}s.", itor.path().c_str(), dlerror());
                return false;
            }
        }
    }
    appLibLoaded_ = true;
    return true;
}

void CJAbilityRuntime::StartDebugMode(const DebugOption dOption)
{
    if (debugModel_) {
        HILOG_INFO("Already in debug mode");
        return;
    }

    bool isStartWithDebug = dOption.isStartWithDebug;
    bool isDebugApp = dOption.isDebugApp;
    const std::string bundleName = bundleName_;
    uint32_t instanceId = instanceId_;
    std::string inputProcessName = bundleName_ != dOption.processName ? dOption.processName : "";

    HILOG_INFO("StartDebugMode %{public}s", bundleName_.c_str());

    HdcRegister::Get().StartHdcRegister(bundleName_, inputProcessName, isDebugApp,
        [bundleName, isStartWithDebug, isDebugApp](int socketFd, std::string option) {
            HILOG_INFO("HdcRegister callback is call, socket fd is %{public}d, option is %{public}s.",
                       socketFd, option.c_str());
            if (option.find(DEBUGGER) == std::string::npos) {
                if (!isDebugApp) {
                    ConnectServerManager::Get().StopConnectServer(false);
                }
                ConnectServerManager::Get().SendDebuggerInfo(isStartWithDebug, isDebugApp);
                ConnectServerManager::Get().StartConnectServer(bundleName, socketFd, false);
            } else {
                HILOG_ERROR("debugger service unexpected option: %{public}s", option.c_str());
            }
        });
    if (isDebugApp) {
        ConnectServerManager::Get().StartConnectServer(bundleName_, -1, true);
    }
    ConnectServerManager::Get().AddInstance(instanceId_, instanceId_);

    debugModel_ = StartDebugger();
}

bool CJAbilityRuntime::StartDebugger()
{
    return CJRuntime::StartDebugger();
}

void CJAbilityRuntime::UnLoadCJAppLibrary()
{
    HILOG_INFO("UnLoadCJAppLibrary not support yet");
}
