/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "child_main_thread.h"

#include "bundle_mgr_proxy.h"
#include "child_process_manager.h"
#include "constants.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AbilityBase::Constants;
using OHOS::AbilityRuntime::ChildProcessManager;
ChildMainThread::ChildMainThread()
{
    TAG_LOGD(AAFwkTag::APPKIT, "ChildMainThread constructor called");
}

ChildMainThread::~ChildMainThread()
{
    TAG_LOGD(AAFwkTag::APPKIT, "ChildMainThread deconstructor called");
}

void ChildMainThread::Start(const ChildProcessInfo &processInfo)
{
    TAG_LOGI(AAFwkTag::APPKIT, "ChildMainThread start.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    if (thread == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "New ChildMainThread failed.");
        return;
    }
    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    if (runner == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "runner is null.");
        return;
    }
    if (!thread->Init(runner, processInfo)) {
        TAG_LOGE(AAFwkTag::APPKIT, "ChildMainThread Init failed.");
        return;
    }
    if (!thread->Attach()) {
        TAG_LOGE(AAFwkTag::APPKIT, "ChildMainThread Attach failed.");
        return;
    }

    auto ret = runner->Run();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ChildMainThread runner->Run failed ret = %{public}d", ret);
    }

    TAG_LOGD(AAFwkTag::APPKIT, "ChildMainThread end.");
}

bool ChildMainThread::Init(const std::shared_ptr<EventRunner> &runner, const ChildProcessInfo &processInfo)
{
    TAG_LOGD(AAFwkTag::APPKIT, "ChildMainThread:Init called.");
    if (runner == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "runner is null.");
        return false;
    }
    processInfo_ = std::make_shared<ChildProcessInfo>(processInfo);
    mainHandler_ = std::make_shared<EventHandler>(runner);
    BundleInfo bundleInfo;
    if (!ChildProcessManager::GetInstance().GetBundleInfo(bundleInfo)) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetBundleInfo failed!.");
        return false;
    }
    bundleInfo_ = std::make_shared<BundleInfo>(bundleInfo);
    InitNativeLib(bundleInfo);
    return true;
}

bool ChildMainThread::Attach()
{
    TAG_LOGD(AAFwkTag::APPKIT, "ChildMainThread::Attach called.");
    auto sysMrgClient = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (sysMrgClient == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get SysMrgClient.");
        return false;
    }
    auto object = sysMrgClient->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetAppMgr failed.");
        return false;
    }
    appMgr_ = iface_cast<IAppMgr>(object);
    if (appMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to iface_cast object to appMgr_");
        return false;
    }
    appMgr_->AttachChildProcess(this);
    return true;
}

bool ChildMainThread::ScheduleLoadJs()
{
    TAG_LOGI(AAFwkTag::APPKIT, "ScheduleLoadJs called.");
    if (mainHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "mainHandler_ is null");
        return false;
    }
    wptr<ChildMainThread> weak = this;
    auto task = [weak]() {
        auto childMainThread = weak.promote();
        if (childMainThread == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "childMainThread is nullptr, ScheduleLoadJs failed.");
            return;
        }
        childMainThread->HandleLoadJs();
    };
    if (!mainHandler_->PostTask(task, "ChildMainThread::HandleLoadJs")) {
        TAG_LOGE(AAFwkTag::APPKIT, "ChildMainThread::ScheduleLoadJs PostTask task failed.");
        return false;
    }
    return true;
}

void ChildMainThread::HandleLoadJs()
{
    TAG_LOGD(AAFwkTag::APPKIT, "called.");
    if (!processInfo_ || !bundleInfo_) {
        TAG_LOGE(AAFwkTag::APPKIT, "processInfo or bundleInfo_ is null.");
        return;
    }
    ChildProcessManager &childProcessManager = ChildProcessManager::GetInstance();
    HapModuleInfo hapModuleInfo;
    BundleInfo bundleInfoCopy = *bundleInfo_;
    if (!childProcessManager.GetHapModuleInfo(bundleInfoCopy, hapModuleInfo)) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetHapModuleInfo failed.");
        return;
    }

    runtime_ = childProcessManager.CreateRuntime(bundleInfoCopy, hapModuleInfo, true, processInfo_->jitEnabled);
    if (!runtime_) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create child process runtime");
        return;
    }
    AbilityRuntime::Runtime::DebugOption debugOption;
    debugOption.isStartWithDebug = processInfo_->isStartWithDebug;
    debugOption.processName = processInfo_->processName;
    debugOption.isDebugApp = processInfo_->isDebugApp;
    debugOption.isStartWithNative = processInfo_->isStartWithNative;
    TAG_LOGD(AAFwkTag::APPKIT, "StartDebugMode, isStartWithDebug is %{public}d, processName is %{public}s, "
        "isDebugApp is %{public}d, isStartWithNative is %{public}d.", processInfo_->isStartWithDebug,
        processInfo_->processName.c_str(), processInfo_->isDebugApp, processInfo_->isStartWithNative);
    runtime_->StartDebugMode(debugOption);
    childProcessManager.LoadJsFile(processInfo_->srcEntry, hapModuleInfo, runtime_);
    TAG_LOGD(AAFwkTag::APPKIT, "ChildMainThread::HandleLoadJs end.");
    ExitProcessSafely();
}

void ChildMainThread::InitNativeLib(const BundleInfo &bundleInfo)
{
    AppLibPathMap appLibPaths {};
    GetNativeLibPath(bundleInfo, appLibPaths);
    bool isSystemApp = bundleInfo.applicationInfo.isSystemApp;
    TAG_LOGD(AAFwkTag::APPKIT, "the application isSystemApp: %{public}d", isSystemApp);
    AbilityRuntime::JsRuntime::SetAppLibPath(appLibPaths, isSystemApp);
}

void ChildMainThread::ExitProcessSafely()
{
    if (appMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appMgr_ is null, use exit(0) instead.");
        exit(0);
        return;
    }
    appMgr_->ExitChildProcessSafely();
}

bool ChildMainThread::ScheduleExitProcessSafely()
{
    TAG_LOGD(AAFwkTag::APPKIT, "ScheduleExitProcessSafely");
    if (mainHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "mainHandler_ is null");
        return false;
    }
    wptr<ChildMainThread> weak = this;
    auto task = [weak]() {
        auto childMainThread = weak.promote();
        if (childMainThread == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "childMainThread is nullptr, ScheduleExitProcessSafely failed.");
            return;
        }
        childMainThread->HandleExitProcessSafely();
    };
    if (!mainHandler_->PostTask(task, "ChildMainThread::HandleExitProcessSafely")) {
        TAG_LOGE(AAFwkTag::APPKIT, "ScheduleExitProcessSafely PostTask task failed.");
        return false;
    }
    return true;
}

void ChildMainThread::HandleExitProcessSafely()
{
    TAG_LOGD(AAFwkTag::APPKIT, "HandleExitProcessSafely called start.");
    std::shared_ptr<EventRunner> runner = mainHandler_->GetEventRunner();
    if (runner == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "HandleExitProcessSafely get runner error.");
        return;
    }
    int ret = runner->Stop();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "HandleExitProcessSafely failed. runner->Run failed ret = %{public}d", ret);
    }
}

void ChildMainThread::GetNativeLibPath(const BundleInfo &bundleInfo, AppLibPathMap &appLibPaths)
{
    std::string nativeLibraryPath = bundleInfo.applicationInfo.nativeLibraryPath;
    if (!nativeLibraryPath.empty()) {
        if (nativeLibraryPath.back() == '/') {
            nativeLibraryPath.pop_back();
        }
        std::string libPath = LOCAL_CODE_PATH;
        libPath += (libPath.back() == '/') ? nativeLibraryPath : "/" + nativeLibraryPath;
        TAG_LOGI(AAFwkTag::APPKIT, "napi lib path = %{private}s", libPath.c_str());
        appLibPaths["default"].emplace_back(libPath);
    }

    for (auto &hapInfo : bundleInfo.hapModuleInfos) {
        TAG_LOGD(AAFwkTag::APPKIT,
            "moduleName: %{public}s, isLibIsolated: %{public}d, compressNativeLibs: %{public}d.",
            hapInfo.moduleName.c_str(), hapInfo.isLibIsolated, hapInfo.compressNativeLibs);
        GetHapSoPath(hapInfo, appLibPaths, hapInfo.hapPath.find(ABS_CODE_PATH));
    }
}

void ChildMainThread::GetHapSoPath(const HapModuleInfo &hapInfo, AppLibPathMap &appLibPaths, bool isPreInstallApp)
{
    if (hapInfo.nativeLibraryPath.empty()) {
        TAG_LOGD(AAFwkTag::APPKIT, "Lib path of %{public}s is empty, lib isn't isolated or compressed.",
            hapInfo.moduleName.c_str());
        return;
    }

    std::string appLibPathKey = hapInfo.bundleName + "/" + hapInfo.moduleName;
    std::string libPath = LOCAL_CODE_PATH;
    if (!hapInfo.compressNativeLibs) {
        TAG_LOGD(AAFwkTag::APPKIT, "Lib of %{public}s will not be extracted from hap.", hapInfo.moduleName.c_str());
        libPath = GetLibPath(hapInfo.hapPath, isPreInstallApp);
    }

    libPath += (libPath.back() == '/') ? hapInfo.nativeLibraryPath : "/" + hapInfo.nativeLibraryPath;
    TAG_LOGI(
        AAFwkTag::APPKIT, "appLibPathKey: %{private}s, lib path: %{private}s", appLibPathKey.c_str(), libPath.c_str());
    appLibPaths[appLibPathKey].emplace_back(libPath);
}

std::string ChildMainThread::GetLibPath(const std::string &hapPath, bool isPreInstallApp)
{
    std::string libPath = LOCAL_CODE_PATH;
    if (isPreInstallApp) {
        auto pos = hapPath.rfind("/");
        if (pos != std::string::npos) {
            libPath = hapPath.substr(0, pos);
        }
    }
    return libPath;
}
}  // namespace AppExecFwk
}  // namespace OHOS
