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
const std::string MODULE_NAME = "@module:";
const std::string CACHE_DIRECTORY = "el2";
#ifdef APP_USE_ARM
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib/libark_debugger.z.so";
#else
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib64/libark_debugger.z.so";
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
    explicit AssetHelper(const std::string& codePath, bool isDebugVersion)
        : codePath_(codePath), isDebugVersion_(isDebugVersion)
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
        std::string realPath;
        size_t index = 0;
        // 1. start with @module:modulename
        // 2. start with /modulename
        // 3. start with modulename
        if (uri.find(MODULE_NAME) == 0) {
            HILOG_DEBUG("uri start with @module:");
            index = uri.find_first_of(":");
            realPath = uri.substr(index + 1);
        } else if (uri.find_first_of("/") == 0) {
            HILOG_DEBUG("uri start with /modulename");
            realPath = uri.substr(1);
        } else {
            HILOG_DEBUG("uri start with modulename");
            realPath = uri;
        }

        index = realPath.find_last_of(".");
        std::string filePath;
        // 1. end with file name
        // 2. end with file name and file type
        if (index == std::string::npos) {
            HILOG_DEBUG("uri end without file type");
            filePath = realPath + ".abc";
        } else {
            HILOG_DEBUG("uri end with file type");
            filePath = realPath.substr(0, index) + ".abc";
        }

        ami = codePath_ + filePath;
        HILOG_DEBUG("Get asset, ami: %{private}s", ami.c_str());
        if (ami.find(CACHE_DIRECTORY) != std::string::npos) {
            if (!ReadAmiData(ami, content)) {
                HILOG_ERROR("Get asset content by ami failed.");
            }
        } else if (!ReadFilePathData(filePath, content)) {
            HILOG_ERROR("Get asset content by filepath failed.");
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

        HILOG_DEBUG("get bundle manager proxy success.");
        return iface_cast<BundleMgrProxy>(remoteObject);
    }

    bool ReadAmiData(const std::string& ami, std::vector<uint8_t>& content) const
    {
        char path[PATH_MAX];
        if (realpath(ami.c_str(), path) == nullptr) {
            HILOG_ERROR("ReadAmiData realpath(%{private}s) failed, errno = %{public}d", ami.c_str(), errno);
            return false;
        }

        std::ifstream stream(path, std::ios::binary | std::ios::ate);
        if (!stream.is_open()) {
            HILOG_ERROR("ReadAmiData failed to open file %{private}s", ami.c_str());
            return false;
        }

        auto fileLen = stream.tellg();
        if (!isDebugVersion_ && fileLen > ASSET_FILE_MAX_SIZE) {
            HILOG_ERROR("ReadAmiData failed, file is too large");
            return false;
        }

        content.resize(fileLen);

        stream.seekg(0);
        stream.read(reinterpret_cast<char*>(content.data()), content.size());
        return true;
    }

    bool ReadFilePathData(const std::string& filePath, std::vector<uint8_t>& content) const
    {
        auto bundleMgrProxy = GetBundleMgrProxy();
        if (!bundleMgrProxy) {
            HILOG_ERROR("bundle mgr proxy is nullptr.");
            return false;
        }

        AppExecFwk::BundleInfo bundleInfo;
        auto getInfoResult = bundleMgrProxy->GetBundleInfoForSelf(
                     static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE), bundleInfo);
        if (getInfoResult != 0) {
            HILOG_ERROR("GetBundleInfoForSelf failed.");
            return false;
        }
        if (bundleInfo.hapModuleInfos.size() == 0) {
            HILOG_ERROR("get hapModuleInfo of bundleInfo failed.");
            return false;
        }
        std::string newHapPath;
        size_t pos = filePath.find('/');
        for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
            if (hapModuleInfo.moduleName == filePath.substr(0, pos)) {
                newHapPath = hapModuleInfo.hapPath;
                break;
            }
        }

        bool newCreate = false;
        std::string loadPath = ExtractorUtil::GetLoadFilePath(newHapPath);
        std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(loadPath, newCreate);
        if (extractor == nullptr) {
            HILOG_ERROR("loadPath %{private}s GetExtractor failed", loadPath.c_str());
            return false;
        }
        std::unique_ptr<uint8_t[]> dataPtr = nullptr;
        std::string realfilePath = filePath.substr(pos + 1);
        size_t fileLen = 0;
        HILOG_DEBUG("Get asset, realfilePath: %{private}s", realfilePath.c_str());
        if (!extractor->ExtractToBufByName(realfilePath, dataPtr, fileLen)) {
            HILOG_ERROR("get mergeAbc fileBuffer failed");
            return false;
        }
        if (!isDebugVersion_ && fileLen > ASSET_FILE_MAX_SIZE) {
            HILOG_ERROR("ReadFilePathData failed, file is too large");
            return false;
        }
        content.assign(dataPtr.get(), dataPtr.get() + fileLen);
        return true;
    }

    std::string codePath_;
    bool isDebugVersion_ = false;
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

void InitWorkerModule(NativeEngine& engine, const std::string& codePath, bool isDebugVersion)
{
    engine.SetInitWorkerFunc(InitWorkerFunc);
    engine.SetOffWorkerFunc(OffWorkerFunc);
    engine.SetGetAssetFunc(AssetHelper(codePath, isDebugVersion));

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