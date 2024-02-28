/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "bundle_mgr_helper.h"
#include "connect_server_manager.h"
#include "commonlibrary/c_utils/base/include/refbase.h"
#ifdef SUPPORT_GRAPHICS
#include "core/common/container_scope.h"
#endif
#include "declarative_module_preloader.h"
#include "extractor.h"
#include "foundation/bundlemanager/bundle_framework/interfaces/inner_api/appexecfwk_base/include/bundle_info.h"
#include "foundation/bundlemanager/bundle_framework/interfaces/inner_api/appexecfwk_core/include/bundlemgr/bundle_mgr_proxy.h"
#include "foundation/systemabilitymgr/samgr/interfaces/innerkits/samgr_proxy/include/iservice_registry.h"
#include "foundation/communication/ipc/interfaces/innerkits/ipc_core/include/iremote_object.h"
#include "singleton.h"
#include "system_ability_definition.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "commonlibrary/ets_utils/js_sys_module/console/console.h"
#ifdef SUPPORT_GRAPHICS
using OHOS::Ace::ContainerScope;
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int64_t ASSET_FILE_MAX_SIZE = 32 * 1024 * 1024;
const std::string BUNDLE_NAME_FLAG = "@bundle:";
const std::string CACHE_DIRECTORY = "el2";
const int PATH_THREE = 3;
#ifdef APP_USE_ARM
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib/platformsdk/libark_debugger.z.so";
#elif defined(APP_USE_X86_64)
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib64/platformsdk/libark_debugger.z.so";
#else
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib64/platformsdk/libark_debugger.z.so";
#endif

bool g_debugMode = false;
bool g_debugApp = false;
bool g_jsFramework = false;
std::mutex g_mutex;
}

void InitWorkerFunc(NativeEngine* nativeEngine)
{
    HILOG_DEBUG("called");
    if (nativeEngine == nullptr) {
        HILOG_ERROR("Input nativeEngine is nullptr");
        return;
    }

    napi_value globalObj = nullptr;
    napi_get_global(reinterpret_cast<napi_env>(nativeEngine), &globalObj);
    if (globalObj == nullptr) {
        HILOG_ERROR("Failed to get global object");
        return;
    }

    OHOS::JsSysModule::Console::InitConsoleModule(reinterpret_cast<napi_env>(nativeEngine));
    OHOS::Ace::DeclarativeModulePreloader::PreloadWorker(*nativeEngine);

    auto arkNativeEngine = static_cast<ArkNativeEngine*>(nativeEngine);
    // load jsfwk
    if (g_jsFramework && !arkNativeEngine->ExecuteJsBin("/system/etc/strip.native.min.abc")) {
        HILOG_ERROR("Failed to load js framework!");
    }

    if (g_debugMode) {
        auto instanceId = gettid();
        std::string instanceName = "workerThread_" + std::to_string(instanceId);
        bool needBreakPoint = ConnectServerManager::Get().AddInstance(instanceId, instanceId, instanceName);
        auto workerPostTask = [nativeEngine](std::function<void()>&& callback) {
            nativeEngine->CallDebuggerPostTaskFunc(std::move(callback));
        };
        panda::JSNApi::DebugOption debugOption = {ARK_DEBUGGER_LIB_PATH, needBreakPoint};
        auto vm = const_cast<EcmaVM*>(arkNativeEngine->GetEcmaVm());
        ConnectServerManager::Get().StoreDebuggerInfo(
            instanceId, reinterpret_cast<void*>(vm), debugOption, workerPostTask, g_debugApp);

        panda::JSNApi::NotifyDebugMode(instanceId, vm, debugOption, instanceId, workerPostTask, g_debugApp);
    }
}

void OffWorkerFunc(NativeEngine* nativeEngine)
{
    HILOG_DEBUG("OffWorkerFunc called");
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


using Extractor = AbilityBase::Extractor;
using ExtractorUtil = AbilityBase::ExtractorUtil;
using IBundleMgr = AppExecFwk::IBundleMgr;

std::string AssetHelper::NormalizedFileName(const std::string& fileName) const
{
    std::string normalizedFilePath;
    size_t index = 0;
    index = fileName.find_last_of(".");
    // 1.1 end with file name
    // 1.2 end with file name and file type
    if (index == std::string::npos) {
        HILOG_DEBUG("uri end without file type");
        normalizedFilePath = fileName + ".abc";
    } else {
        HILOG_DEBUG("uri end with file type");
        normalizedFilePath = fileName.substr(0, index) + ".abc";
    }
    return normalizedFilePath;
}

void AssetHelper::operator()(const std::string& uri, std::vector<uint8_t>& content, std::string &ami)
{
    if (uri.empty() || workerInfo_ == nullptr) {
        HILOG_ERROR("Uri is empty.");
        return;
    }

    HILOG_DEBUG("RegisterAssetFunc called, uri: %{private}s", uri.c_str());
    std::string realPath;
    std::string filePath;

    // 1. compilemode is jsbundle
    // 2. compilemode is esmodule
    if (workerInfo_->isBundle) {
        // the @bundle:bundlename/modulename only exist in esmodule.
        // 1.1 start with /modulename
        // 1.2 start with ../
        // 1.3 start with @namespace [not support]
        // 1.4 start with modulename
        HILOG_DEBUG("The application is packaged using jsbundle mode.");
        if (uri.find_first_of("/") == 0) {
            HILOG_DEBUG("uri start with /modulename");
            realPath = uri.substr(1);
        } else if (uri.find("../") == 0 && !workerInfo_->isStageModel) {
            HILOG_DEBUG("uri start with ../");
            realPath = uri.substr(PATH_THREE);
        } else if (uri.find_first_of("@") == 0) {
            HILOG_DEBUG("uri start with @namespace");
            realPath = uri.substr(uri.find_first_of("/") + 1);
        } else {
            HILOG_DEBUG("uri start with modulename");
            realPath = uri;
        }

        filePath = NormalizedFileName(realPath);
        HILOG_INFO("filePath %{private}s", filePath.c_str());

        if (!workerInfo_->isStageModel) {
            GetAmi(ami, filePath);
        } else {
            ami = workerInfo_->codePath + filePath;
        }

        HILOG_DEBUG("Get asset, ami: %{private}s", ami.c_str());
        if (ami.find(CACHE_DIRECTORY) != std::string::npos) {
            if (!ReadAmiData(ami, content)) {
                HILOG_ERROR("Get asset content by ami failed.");
            }
        } else if (!ReadFilePathData(filePath, content)) {
            HILOG_ERROR("Get asset content by filepath failed.");
        }
    } else {
        // 2.1 start with @bundle:bundlename/modulename
        // 2.2 start with /modulename
        // 2.3 start with @namespace
        // 2.4 start with modulename
        HILOG_DEBUG("The application is packaged using esmodule mode.");
        if (uri.find(BUNDLE_NAME_FLAG) == 0) {
            HILOG_DEBUG("uri start with @bundle:");
            size_t fileNamePos = uri.find_last_of("/");
            realPath = uri.substr(fileNamePos + 1);
            if (realPath.find_last_of(".") != std::string::npos) {
                ami = NormalizedFileName(uri);
            } else {
                ami = uri;
            }
            HILOG_DEBUG("Get asset, ami: %{private}s", ami.c_str());
            return;
        } else if (uri.find_first_of("/") == 0) {
            HILOG_DEBUG("uri start with /modulename");
            realPath = uri.substr(1);
        } else if (uri.find_first_of("@") == 0) {
            HILOG_DEBUG("uri start with @namespace");
            realPath = workerInfo_->moduleName + uri;
        } else {
            HILOG_DEBUG("uri start with modulename");
            realPath = uri;
        }

        filePath = NormalizedFileName(realPath);
        ami = workerInfo_->codePath + filePath;
        HILOG_DEBUG("Get asset, ami: %{private}s", ami.c_str());
        if (ami.find(CACHE_DIRECTORY) != std::string::npos) {
            if (!ReadAmiData(ami, content)) {
                HILOG_ERROR("Get asset content by ami failed.");
            }
        } else if (!ReadFilePathData(filePath, content)) {
            HILOG_ERROR("Get asset content by filepath failed.");
        }
    }
}

bool AssetHelper::ReadAmiData(const std::string& ami, std::vector<uint8_t>& content) const
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
    if (!workerInfo_->isDebugVersion && fileLen > ASSET_FILE_MAX_SIZE) {
        HILOG_ERROR("ReadAmiData failed, file is too large");
        return false;
    }

    content.resize(fileLen);

    stream.seekg(0);
    stream.read(reinterpret_cast<char*>(content.data()), content.size());
    return true;
}

bool AssetHelper::ReadFilePathData(const std::string& filePath, std::vector<uint8_t>& content)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        HILOG_ERROR("The bundleMgrHelper is nullptr.");
        return false;
    }

    AppExecFwk::BundleInfo bundleInfo;
    auto getInfoResult = bundleMgrHelper->GetBundleInfoForSelf(
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE), bundleInfo);
    if (getInfoResult != 0) {
        HILOG_ERROR("GetBundleInfoForSelf failed.");
        return false;
    }
    if (bundleInfo.hapModuleInfos.size() == 0) {
        HILOG_ERROR("Get hapModuleInfo of bundleInfo failed.");
        return false;
    }

    std::string newHapPath;
    size_t pos = filePath.find('/');
    if (!workerInfo_->isStageModel) {
        newHapPath = workerInfo_->hapPath;
    } else {
        for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
            if (hapModuleInfo.moduleName == filePath.substr(0, pos)) {
                newHapPath = hapModuleInfo.hapPath;
                break;
            }
        }
    }
    HILOG_DEBUG("HapPath: %{private}s", newHapPath.c_str());
    bool newCreate = false;
    std::string loadPath = ExtractorUtil::GetLoadFilePath(newHapPath);
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(loadPath, newCreate);
    if (extractor == nullptr) {
        HILOG_ERROR("LoadPath %{private}s GetExtractor failed", loadPath.c_str());
        return false;
    }
    std::unique_ptr<uint8_t[]> dataPtr = nullptr;
    std::string realfilePath;
    size_t fileLen = 0;
    if (!workerInfo_->isStageModel) {
        bool flag = false;
        for (const auto& basePath : workerInfo_->assetBasePathStr) {
            realfilePath = basePath + filePath;
            HILOG_DEBUG("realfilePath: %{private}s", realfilePath.c_str());
            if (extractor->ExtractToBufByName(realfilePath, dataPtr, fileLen)) {
                flag = true;
                break;
            }
        }
        if (!flag) {
            HILOG_ERROR("ExtractToBufByName error");
            return flag;
        }
    } else {
        realfilePath = filePath.substr(pos + 1);
        HILOG_DEBUG("realfilePath: %{private}s", realfilePath.c_str());
        if (!extractor->ExtractToBufByName(realfilePath, dataPtr, fileLen)) {
            HILOG_ERROR("get mergeAbc fileBuffer failed");
            return false;
        }
    }

    if (!workerInfo_->isDebugVersion && fileLen > ASSET_FILE_MAX_SIZE) {
        HILOG_ERROR("ReadFilePathData failed, file is too large");
        return false;
    }
    content.assign(dataPtr.get(), dataPtr.get() + fileLen);
    return true;
}

void AssetHelper::GetAmi(std::string& ami, const std::string& filePath)
{
    size_t slashPos = filePath.find_last_of("/");
    std::string fileName = filePath.substr(slashPos + 1);
    std::string path = filePath.substr(0, slashPos + 1);

    std::string loadPath = ExtractorUtil::GetLoadFilePath(workerInfo_->hapPath);
    bool newCreate = false;
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(loadPath, newCreate);
    if (extractor == nullptr) {
        HILOG_ERROR("loadPath %{private}s GetExtractor failed", loadPath.c_str());
        return;
    }
    std::vector<std::string> files;
    for (const auto& basePath : workerInfo_->assetBasePathStr) {
        std::string assetPath = basePath + path;
        HILOG_INFO("assetPath: %{private}s", assetPath.c_str());
        bool res = extractor->IsDirExist(assetPath);
        if (!res) {
            continue;
        }
        res = extractor->GetFileList(assetPath, files);
        if (!res) {
            continue;
        }
    }

    std::string targetFilePath;
    bool flag = false;
    for (const auto& file : files) {
        size_t filePos = file.find_last_of("/");
        if (filePos != std::string::npos) {
            if (file.substr(filePos + 1) == fileName) {
                targetFilePath = path + fileName;
                flag = true;
                break;
            }
        }
    }

    HILOG_INFO("targetFilePath %{public}s", targetFilePath.c_str());

    if (!flag) {
        HILOG_ERROR("get targetFilePath failed!");
        return;
    }

    for (const auto& basePath : workerInfo_->assetBasePathStr) {
        std::string filePathName = basePath + targetFilePath;
        bool hasFile = extractor->HasEntry(filePathName);
        if (hasFile) {
            ami = workerInfo_->hapPath + "/" + filePathName;
            return;
        }
    }
}

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

void StartDebuggerInWorkerModule()
{
    g_debugMode = true;
}

void SetDebuggerApp(bool isDebugApp)
{
    g_debugApp = isDebugApp;
}

void SetJsFramework()
{
    g_jsFramework = true;
}
} // namespace AbilityRuntime
} // namespace OHOS
