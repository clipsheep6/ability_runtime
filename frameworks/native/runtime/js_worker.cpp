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

#include "js_worker.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <unistd.h>

#include "connect_server_manager.h"
#include "commonlibrary/c_utils/base/include/refbase.h"
#ifdef SUPPORT_GRAPHICS
#include "core/common/container_scope.h"
#endif
#include "extractor.h"
#include "foundation/bundlemanager/bundle_framework/interfaces/inner_api/appexecfwk_base/include/bundle_info.h"
#include "foundation/bundlemanager/bundle_framework/interfaces/inner_api/appexecfwk_core/include/bundlemgr/bundle_mgr_proxy.h"
#include "foundation/systemabilitymgr/samgr/interfaces/innerkits/samgr_proxy/include/iservice_registry.h"
#include "foundation/communication/ipc/interfaces/innerkits/ipc_core/include/iremote_object.h"
#include "system_ability_definition.h"
#include "hilog_wrapper.h"
#include "js_console_log.h"
#include "js_runtime_utils.h"
#include "native_engine/impl/ark/ark_native_engine.h"

#ifdef SUPPORT_GRAPHICS
using OHOS::Ace::ContainerScope;
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int64_t ASSET_FILE_MAX_SIZE = 32 * 1024 * 1024;
#if defined(_ARM64_)
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib64/libark_debugger.z.so";
#else
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib/libark_debugger.z.so";
#endif

bool g_debugMode = false;

void InitWorkerFunc(NativeEngine* nativeEngine)
{
    HILOG_INFO("InitWorkerFunc called");
    if (nativeEngine == nullptr) {
        HILOG_ERROR("Input nativeEngine is nullptr");
        return;
    }

    NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine->GetGlobal());
    if (globalObj == nullptr) {
        HILOG_ERROR("Failed to get global object");
        return;
    }

    InitConsoleLogModule(*nativeEngine, *globalObj);

    if (g_debugMode) {
        auto instanceId = gettid();
        std::string instanceName = "workerThread_" + std::to_string(instanceId);
        bool needBreakPoint = ConnectServerManager::Get().AddInstance(instanceId, instanceName);
        auto arkNativeEngine = static_cast<ArkNativeEngine*>(nativeEngine);
        auto vm = const_cast<EcmaVM*>(arkNativeEngine->GetEcmaVm());
        auto workerPostTask = [nativeEngine](std::function<void()>&& callback) {
            nativeEngine->CallDebuggerPostTaskFunc(std::move(callback));
        };
        panda::JSNApi::StartDebugger(ARK_DEBUGGER_LIB_PATH, vm, needBreakPoint, instanceId, workerPostTask);
    }
}

void OffWorkerFunc(NativeEngine* nativeEngine)
{
    HILOG_INFO("OffWorkerFunc called");
    if (nativeEngine == nullptr) {
        HILOG_ERROR("Input nativeEngine is nullptr");
        return;
    }

    if (g_debugMode) {
        auto instanceId = gettid();
        ConnectServerManager::Get().RemoveInstance(instanceId);
        auto arkNativeEngine = static_cast<ArkNativeEngine*>(nativeEngine);
        auto vm = const_cast<EcmaVM*>(arkNativeEngine->GetEcmaVm());
        panda::JSNApi::StopDebugger(vm);
    }
}

struct AssetHelper final {
    using Extractor = AbilityBase::Extractor;
    using ExtractorUtil = AbilityBase::ExtractorUtil;
    using BundleMgrProxy = AppExecFwk::BundleMgrProxy;
    explicit AssetHelper(const std::string& codePath, bool isDebugVersion, const std::string& bundleName)
        : codePath_(codePath), isDebugVersion_(isDebugVersion), bundleName_(bundleName)
    {
        if (!codePath_.empty() && codePath.back() != '/') {
            codePath_.append("/");
        }
    }

    void operator()(const std::string& uri, std::vector<uint8_t>& content, std::string &ami) const
    {
        if (uri.empty()) {
            HILOG_ERROR("Uri is empty.");
            return;
        }

        HILOG_INFO("RegisterAssetFunc called, uri: %{private}s", uri.c_str());
        size_t index = uri.find_last_of(".");
        if (index == std::string::npos) {
            HILOG_ERROR("Invalid uri");
            return;
        }

        std::string filePath = uri.substr(0, index) + ".abc";
        ami = codePath_ + filePath;
        HILOG_INFO("Get asset, ami: %{private}s", ami.c_str());
        if (!ReadAssetData(filePath, content)) {
            HILOG_ERROR("Get asset content failed.");
            return;
        }
    }

    sptr<BundleMgrProxy> GetBundleMgrProxy() const
    {
        auto systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityManager) {
            HILOG_ERROR("fail to get system ability mgr.");
            return nullptr;
        }

        auto remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (!remoteObject) {
            HILOG_ERROR("fail to get bundle manager proxy.");
            return nullptr;
        }

        HILOG_INFO("get bundle manager proxy success.");
        return iface_cast<BundleMgrProxy>(remoteObject);
    }

    bool ReadAssetData(const std::string& filePath, std::vector<uint8_t>& content) const
    {
        bool newCreate = false;
        size_t fileLen = 0;
        size_t pos = filePath.find('/');

        if (bundleName_.empty()) {
            HILOG_ERROR("BundleName is nullptr.");
            return false;
        }
        auto bundleMgrProxy = GetBundleMgrProxy();
        if (!bundleMgrProxy) {
            HILOG_ERROR("bundle mgr proxy is nullptr.");
            return false;
        }

        AppExecFwk::BundleInfo bundleInfo;
        auto getInfoResult = bundleMgrProxy->GetBundleInfoV9(bundleName_,
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE), bundleInfo, 100);
        if (getInfoResult != 0) {
            HILOG_ERROR("GetBundleInfo failed through %{private}s.", bundleName_.c_str());
            return false;
        }
        if (bundleInfo.hapModuleInfos.size() == 0) {
            HILOG_ERROR("get hapModuleInfo of bundleInfo failed.");
            return false;
        }
        std::string newHapPath;
        for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
            if (hapModuleInfo.moduleName == filePath.substr(0, pos)) {
                newHapPath = hapModuleInfo.hapPath;
                break;
            }
        }

        std::string loadPath = ExtractorUtil::GetLoadFilePath(newHapPath);
        std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(loadPath, newCreate);
        if (extractor == nullptr) {
            HILOG_ERROR("loadPath %{private}s GetExtractor failed", loadPath.c_str());
            return false;
        }
        std::unique_ptr<uint8_t[]> dataPtr = nullptr;
        std::string realfilePath = filePath.substr(pos + 1);
        HILOG_INFO("Get asset, realfilePath: %{private}s", realfilePath.c_str());
        if (!extractor->ExtractToBufByName(realfilePath, dataPtr, fileLen)) {
            HILOG_ERROR("get mergeAbc fileBuffer failed");
            return false;
        }
        if (!isDebugVersion_ && fileLen > ASSET_FILE_MAX_SIZE) {
            HILOG_ERROR("ReadAssetData failed, file is too large");
            return false;
        }
        content.assign(dataPtr.get(), dataPtr.get() + fileLen);
        return true;
    }

    std::string codePath_;
    bool isDebugVersion_ = false;
    std::string bundleName_;
};

int32_t GetContainerId()
{
#ifdef SUPPORT_GRAPHICS
    int32_t scopeId = ContainerScope::CurrentId();
    return scopeId;
#else
    constexpr int32_t containerScopeDefaultId = 0;
    return containerScopeDefaultId;
#endif
}
void UpdateContainerScope(int32_t id)
{
#ifdef SUPPORT_GRAPHICS
ContainerScope::UpdateCurrent(id);
#endif
}
void RestoreContainerScope(int32_t id)
{
#ifdef SUPPORT_GRAPHICS
ContainerScope::UpdateCurrent(-1);
#endif
}
}

void InitWorkerModule(NativeEngine& engine, const std::string& codePath,
    bool isDebugVersion, const std::string& bundleName)
{
    engine.SetInitWorkerFunc(InitWorkerFunc);
    engine.SetOffWorkerFunc(OffWorkerFunc);
    engine.SetGetAssetFunc(AssetHelper(codePath, isDebugVersion, bundleName));

    engine.SetGetContainerScopeIdFunc(GetContainerId);
    engine.SetInitContainerScopeFunc(UpdateContainerScope);
    engine.SetFinishContainerScopeFunc(RestoreContainerScope);
}

void StartDebuggerInWorkerModule()
{
    g_debugMode = true;
}
} // namespace AbilityRuntime
} // namespace OHOS