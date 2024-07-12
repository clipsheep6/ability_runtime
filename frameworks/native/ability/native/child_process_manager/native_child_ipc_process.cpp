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

#include "native_child_ipc_process.h"
#include <dlfcn.h>
#include <thread>
#include <chrono>
#include "hilog_tag_wrapper.h"
#include "child_process_manager_error_utils.h"

namespace OHOS {
namespace AbilityRuntime {

std::shared_ptr<ChildProcess> NativeChildIpcProcess::Create()
{
    return std::make_shared<NativeChildIpcProcess>();
}

NativeChildIpcProcess::~NativeChildIpcProcess()
{
    UnloadNativeLib();
}
    
bool NativeChildIpcProcess::Init(const std::shared_ptr<ChildProcessStartInfo> &info)
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "init");
    if (info == nullptr || info->ipcObj == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "info or ipc callback is null");
        return false;
    }

    if (!ChildProcess::Init(info)) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "init failed");
        return false;
    }

    auto iNotify = iface_cast<OHOS::AppExecFwk::INativeChildNotify>(info->ipcObj);
    if (iNotify == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null iNotify");
        return false;
    }

    if (!LoadNativeLib(info)) {
        iNotify->OnError(static_cast<int32_t>(ChildProcessManagerErrorCode::ERR_LIB_LOADING_FAILED));
        return false;
    }

    mainProcessCb_ = iNotify;
    return true;
}

void NativeChildIpcProcess::OnStart()
{
    if (funcNativeLibOnConnect_ == nullptr || funcNativeLibMainProc_ == nullptr || mainProcessCb_ == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "No init");
        return;
    }
    
    ChildProcess::OnStart();
    OHIPCRemoteStub *ipcStub = funcNativeLibOnConnect_();
    if (ipcStub == nullptr || ipcStub->remote == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null ipctub");
        mainProcessCb_->OnError(static_cast<int32_t>(ChildProcessManagerErrorCode::ERR_CONNECTION_FAILED));
        return;
    }

    std::thread cbThread([this, childIpcStub = std::move(ipcStub->remote)] () -> void {
        // Wait MainProc run first
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        TAG_LOGI(AAFwkTag::PROCESSMGR, "Notify native child process started");
        mainProcessCb_->OnNativeChildStarted(childIpcStub);
    });

    TAG_LOGI(AAFwkTag::PROCESSMGR, "Enter MainProc");
    funcNativeLibMainProc_();
    TAG_LOGI(AAFwkTag::PROCESSMGR, "MainProc returned");

    if (cbThread.joinable()) {
        cbThread.join();
    }
}

bool NativeChildIpcProcess::LoadNativeLib(const std::shared_ptr<ChildProcessStartInfo> &info)
{
    if (nativeLibHandle_ != nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Native lib already loaded");
        return false;
    }

    if (info->moduleName.empty()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "empty module name");
        return false;
    }

    Dl_namespace dlnsApp;
    std::string appDlNameSpace = "moduleNs_" + info->moduleName;
    int ret = dlns_get(appDlNameSpace.c_str(), &dlnsApp);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Get dlns(%{private}s) err:%{public}d",
            appDlNameSpace.c_str(), ret);
        return false;
    }

    void *libHandle = dlopen_ns(&dlnsApp, info->srcEntry.c_str(), RTLD_LAZY);
    if (libHandle == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Load lib file %{private}s err %{public}s",
            info->srcEntry.c_str(), dlerror());
        return false;
    }

    do {
        NativeChildProcess_OnConnect funcOnConnect =
            reinterpret_cast<NativeChildProcess_OnConnect>(dlsym(libHandle, "NativeChildProcess_OnConnect"));
        if (funcOnConnect == nullptr) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "null funcOnConnect, err %{public}s", dlerror());
            break;
        }

        NativeChildProcess_MainProc funcMainProc =
            reinterpret_cast<NativeChildProcess_MainProc>(dlsym(libHandle, "NativeChildProcess_MainProc"));
        if (funcMainProc == nullptr) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "null funcMainProc, err %{public}s", dlerror());
            break;
        }

        funcNativeLibOnConnect_ = funcOnConnect;
        funcNativeLibMainProc_ = funcMainProc;
        nativeLibHandle_ = libHandle;
        return true;
    } while (false);

    dlclose(libHandle);
    return false;
}

void NativeChildIpcProcess::UnloadNativeLib()
{
    if (nativeLibHandle_ != nullptr) {
        dlclose(nativeLibHandle_);
        nativeLibHandle_ = nullptr;
        funcNativeLibOnConnect_ = nullptr;
        funcNativeLibMainProc_ = nullptr;
    }
}

} // namespace AbilityRuntime
} // namespace OHOS
