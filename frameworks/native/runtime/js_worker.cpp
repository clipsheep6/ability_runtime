/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include <unistd.h>
#include <vector>

#include "bundle_mgr_helper.h"
#include "connect_server_manager.h"
#include "commonlibrary/c_utils/base/include/refbase.h"
#ifdef SUPPORT_GRAPHICS
#include "core/common/container_scope.h"
#endif
#include "declarative_module_preloader.h"
#include "extractor.h"
#include "file_mapper.h"
#include "foundation/bundlemanager/bundle_framework/interfaces/inner_api/appexecfwk_base/include/bundle_info.h"
#include "foundation/bundlemanager/bundle_framework/interfaces/inner_api/appexecfwk_core/include/bundlemgr/bundle_mgr_proxy.h"
#include "foundation/systemabilitymgr/samgr/interfaces/innerkits/samgr_proxy/include/iservice_registry.h"
#include "foundation/communication/ipc/interfaces/innerkits/ipc_core/include/iremote_object.h"
#include "singleton.h"
#include "system_ability_definition.h"
#include "hilog_tag_wrapper.h"
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
constexpr int32_t API8 = 8;
constexpr int32_t API12 = 12;
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
bool g_nativeStart = false;
std::mutex g_mutex;
}

void InitWorkerFunc(NativeEngine* nativeEngine)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    if (nativeEngine == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Input nativeEngine is nullptr");
        return;
    }

    napi_value globalObj = nullptr;
    napi_get_global(reinterpret_cast<napi_env>(nativeEngine), &globalObj);
    if (globalObj == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Failed to get global object");
        return;
    }

    OHOS::JsSysModule::Console::InitConsoleModule(reinterpret_cast<napi_env>(nativeEngine));
    OHOS::Ace::DeclarativeModulePreloader::PreloadWorker(*nativeEngine);

    auto arkNativeEngine = static_cast<ArkNativeEngine*>(nativeEngine);
    // load jsfwk
    if (g_jsFramework && !arkNativeEngine->ExecuteJsBin("/system/etc/strip.native.min.abc")) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Failed to load js framework!");
    }

    if (g_debugMode) {
        auto instanceId = gettid();
        std::string instanceName = "workerThread_" + std::to_string(instanceId);
        bool needBreakPoint = ConnectServerManager::Get().AddInstance(instanceId, instanceId, instanceName);
        if (g_nativeStart) {
            TAG_LOGD(AAFwkTag::APPMGR, "native is true, set needBreakPoint = false.");
            needBreakPoint = false;
        }
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
    TAG_LOGD(AAFwkTag::JSRUNTIME, "OffWorkerFunc called");
    if (nativeEngine == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Input nativeEngine is nullptr");
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
using FileMapper = AbilityBase::FileMapper;
using FileMapperType = AbilityBase::FileMapperType;
using IBundleMgr = AppExecFwk::IBundleMgr;

std::string AssetHelper::NormalizedFileName(const std::string& fileName) const
{
    std::string normalizedFilePath;
    size_t index = 0;
    index = fileName.find_last_of(".");
    // 1.1 end with file name
    // 1.2 end with file name and file type
    if (index == std::string::npos) {
        TAG_LOGD(AAFwkTag::JSRUNTIME, "uri end without file type");
        normalizedFilePath = fileName + ".abc";
    } else {
        TAG_LOGD(AAFwkTag::JSRUNTIME, "uri end with file type");
        normalizedFilePath = fileName.substr(0, index) + ".abc";
    }
    return normalizedFilePath;
}

AssetHelper::~AssetHelper()
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "destroyed.");
    if (fd_ != -1) {
        close(fd_);
        fd_ = -1;
    }
}

void AssetHelper::operator()(const std::string& uri, uint8_t** buff, size_t* buffSize, std::vector<uint8_t>& content,
    std::string& ami, bool& useSecureMem, bool isRestricted)
{
    if (uri.empty() || buff == nullptr || buffSize == nullptr || workerInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Input params invalid.");
        return;
    }

    TAG_LOGD(AAFwkTag::JSRUNTIME, "RegisterAssetFunc called, uri: %{private}s", uri.c_str());
    std::string realPath;
    std::string filePath;
    useSecureMem = false;

    // 1. compilemode is jsbundle
    // 2. compilemode is esmodule
    if (workerInfo_->isBundle) {
        // the @bundle:bundlename/modulename only exist in esmodule.
        // 1.1 start with /modulename
        // 1.2 start with ../
        // 1.3 start with @namespace [not support]
        // 1.4 start with modulename
        TAG_LOGD(AAFwkTag::JSRUNTIME, "The application is packaged using jsbundle mode.");
        if (uri.find_first_of("/") == 0) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "uri start with /modulename");
            realPath = uri.substr(1);
        } else if (uri.find("../") == 0 && !workerInfo_->isStageModel) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "uri start with ../");
            realPath = uri.substr(PATH_THREE);
        } else if (uri.find_first_of("@") == 0) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "uri start with @namespace");
            realPath = uri.substr(uri.find_first_of("/") + 1);
        } else {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "uri start with modulename");
            realPath = uri;
        }

        filePath = NormalizedFileName(realPath);
        TAG_LOGI(AAFwkTag::JSRUNTIME, "filePath %{private}s", filePath.c_str());

        if (!workerInfo_->isStageModel) {
            GetAmi(ami, filePath);
        } else {
            ami = workerInfo_->codePath + filePath;
        }

        TAG_LOGD(AAFwkTag::JSRUNTIME, "Get asset, ami: %{private}s", ami.c_str());
        if (ami.find(CACHE_DIRECTORY) != std::string::npos) {
            if (!ReadAmiData(ami, buff, buffSize, content, useSecureMem, isRestricted)) {
                TAG_LOGE(AAFwkTag::JSRUNTIME, "Get buffer by ami failed.");
            }
        } else if (!ReadFilePathData(filePath, buff, buffSize, content, useSecureMem, isRestricted)) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "Get buffer by filepath failed.");
        }
    } else {
        // 2.1 start with @bundle:bundlename/modulename
        // 2.2 start with /modulename
        // 2.3 start with @namespace
        // 2.4 start with modulename
        TAG_LOGD(AAFwkTag::JSRUNTIME, "The application is packaged using esmodule mode.");
        if (uri.find(BUNDLE_NAME_FLAG) == 0) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "uri start with @bundle:");
            size_t fileNamePos = uri.find_last_of("/");
            realPath = uri.substr(fileNamePos + 1);
            if (realPath.find_last_of(".") != std::string::npos) {
                ami = NormalizedFileName(uri);
            } else {
                ami = uri;
            }
            TAG_LOGD(AAFwkTag::JSRUNTIME, "Get asset, ami: %{private}s", ami.c_str());
            return;
        } else if (uri.find_first_of("/") == 0) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "uri start with /modulename");
            realPath = uri.substr(1);
        } else if (uri.find_first_of("@") == 0) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "uri start with @namespace");
            realPath = workerInfo_->moduleName + uri;
        } else {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "uri start with modulename");
            realPath = uri;
        }

        filePath = NormalizedFileName(realPath);
        ami = workerInfo_->codePath + filePath;
        TAG_LOGD(AAFwkTag::JSRUNTIME, "Get asset, ami: %{private}s", ami.c_str());
        if (ami.find(CACHE_DIRECTORY) != std::string::npos) {
            if (!ReadAmiData(ami, buff, buffSize, content, useSecureMem, isRestricted)) {
                TAG_LOGE(AAFwkTag::JSRUNTIME, "Get buffer by ami failed.");
            }
        } else if (!ReadFilePathData(filePath, buff, buffSize, content, useSecureMem, isRestricted)) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "Get buffer by filepath failed.");
        }
    }
}

bool AssetHelper::GetSafeData(const std::string& ami, uint8_t** buff, size_t* buffSize)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "Get secure mem.");
    std::string resolvedPath;
    resolvedPath.reserve(PATH_MAX);
    resolvedPath.resize(PATH_MAX - 1);
    if (realpath(ami.c_str(), &(resolvedPath[0])) == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Realpath file %{private}s caught error: %{public}d.", ami.c_str(), errno);
        return false;
    }

    int fd = open(resolvedPath.c_str(), O_RDONLY);
    if (fd < 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Open file %{private}s caught error: %{public}d.", resolvedPath.c_str(), errno);
        return false;
    }

    struct stat statbuf;
    if (fstat(fd, &statbuf) < 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Get fstat of file %{private}s caught error: %{public}d.", resolvedPath.c_str(),
            errno);
        close(fd);
        return false;
    }

    std::unique_ptr<FileMapper> fileMapper = std::make_unique<FileMapper>();
    if (fileMapper == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Create file mapper failed.");
        close(fd);
        return false;
    }

    auto result = fileMapper->CreateFileMapper(resolvedPath, false, fd, 0, statbuf.st_size, FileMapperType::SAFE_ABC);
    if (!result) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Create file %{private}s mapper failed.", resolvedPath.c_str());
        close(fd);
        return false;
    }

    *buff = fileMapper->GetDataPtr();
    *buffSize = fileMapper->GetDataLen();
    fd_ = fd;
    return true;
}

bool AssetHelper::ReadAmiData(const std::string& ami, uint8_t** buff, size_t* buffSize, std::vector<uint8_t>& content,
    bool& useSecureMem, bool isRestricted)
{
    // Current function is a private, validity of workerInfo_ has been checked by caller.
    bool apiSatisfy = workerInfo_->apiTargetVersion == 0 || workerInfo_->apiTargetVersion > API8;
    if (workerInfo_->isStageModel && !isRestricted && apiSatisfy) {
        if (workerInfo_->apiTargetVersion >= API12) {
            useSecureMem = true;
            return GetSafeData(ami, buff, buffSize);
        } else if (GetSafeData(ami, buff, buffSize)) {
            useSecureMem = true;
            return true;
        } else {
            // If api version less than 12 and get secure mem failed, try get normal mem.
            TAG_LOGW(AAFwkTag::JSRUNTIME, "Get secure mem failed, file %{private}s.", ami.c_str());
        }
    }

    char path[PATH_MAX];
    if (realpath(ami.c_str(), path) == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Realpath file %{private}s caught error: %{public}d.", ami.c_str(), errno);
        return false;
    }

    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream.is_open()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Failed to open file %{private}s.", ami.c_str());
        return false;
    }

    auto fileLen = stream.tellg();
    if (!workerInfo_->isDebugVersion && fileLen > ASSET_FILE_MAX_SIZE) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "File is too large.");
        return false;
    }

    if (fileLen <= 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Invalid file length.");
        return false;
    }

    content.resize(fileLen);
    stream.seekg(0);
    stream.read(reinterpret_cast<char*>(content.data()), content.size());
    return true;
}

bool AssetHelper::ReadFilePathData(const std::string& filePath, uint8_t** buff, size_t* buffSize,
    std::vector<uint8_t>& content, bool& useSecureMem, bool isRestricted)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "The bundleMgrHelper is nullptr.");
        return false;
    }

    AppExecFwk::BundleInfo bundleInfo;
    auto getInfoResult = bundleMgrHelper->GetBundleInfoForSelf(
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE), bundleInfo);
    if (getInfoResult != 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "GetBundleInfoForSelf failed.");
        return false;
    }
    if (bundleInfo.hapModuleInfos.size() == 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Get hapModuleInfo of bundleInfo failed.");
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
    TAG_LOGD(AAFwkTag::JSRUNTIME, "HapPath: %{private}s", newHapPath.c_str());
    bool newCreate = false;
    std::string loadPath = ExtractorUtil::GetLoadFilePath(newHapPath);
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(loadPath, newCreate);
    if (extractor == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "LoadPath %{private}s GetExtractor failed", loadPath.c_str());
        return false;
    }
    std::unique_ptr<uint8_t[]> dataPtr = nullptr;
    std::string realfilePath;
    size_t fileLen = 0;
    if (!workerInfo_->isStageModel) {
        bool flag = false;
        for (const auto& basePath : workerInfo_->assetBasePathStr) {
            realfilePath = basePath + filePath;
            TAG_LOGD(AAFwkTag::JSRUNTIME, "realfilePath: %{private}s", realfilePath.c_str());
            if (extractor->ExtractToBufByName(realfilePath, dataPtr, fileLen)) {
                flag = true;
                break;
            }
        }
        if (!flag) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "ExtractToBufByName error");
            return flag;
        }
    } else {
        realfilePath = filePath.substr(pos + 1);
        TAG_LOGD(AAFwkTag::JSRUNTIME, "realfilePath: %{private}s", realfilePath.c_str());
        bool apiSatisfy = workerInfo_->apiTargetVersion == 0 || workerInfo_->apiTargetVersion > API8;
        if (workerInfo_->isStageModel && !isRestricted && apiSatisfy && !extractor->IsHapCompress(realfilePath)) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "Use secure mem.");
            auto safeData = extractor->GetSafeData(realfilePath);
            if (workerInfo_->apiTargetVersion >= API12) {
                useSecureMem = true;
                if (safeData == nullptr) {
                    TAG_LOGE(AAFwkTag::JSRUNTIME, "Get secure mem failed, file %{private}s.", filePath.c_str());
                    return false;
                }
                *buff = safeData->GetDataPtr();
                *buffSize = safeData->GetDataLen();
                return true;
            } else if (safeData != nullptr) {
                useSecureMem = true;
                *buff = safeData->GetDataPtr();
                *buffSize = safeData->GetDataLen();
                return true;
            } else {
                // If api version less than 12 and get secure mem failed, try get normal mem.
                TAG_LOGW(AAFwkTag::JSRUNTIME, "Get secure mem failed, file %{private}s.", filePath.c_str());
            }
        }
        if (!extractor->ExtractToBufByName(realfilePath, dataPtr, fileLen)) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "get mergeAbc fileBuffer failed");
            return false;
        }
    }

    if (!workerInfo_->isDebugVersion && fileLen > ASSET_FILE_MAX_SIZE) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "ReadFilePathData failed, file is too large");
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
        TAG_LOGE(AAFwkTag::JSRUNTIME, "loadPath %{private}s GetExtractor failed", loadPath.c_str());
        return;
    }
    std::vector<std::string> files;
    for (const auto& basePath : workerInfo_->assetBasePathStr) {
        std::string assetPath = basePath + path;
        TAG_LOGI(AAFwkTag::JSRUNTIME, "assetPath: %{private}s", assetPath.c_str());
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

    TAG_LOGI(AAFwkTag::JSRUNTIME, "targetFilePath %{public}s", targetFilePath.c_str());

    if (!flag) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "get targetFilePath failed!");
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

void SetNativeStart(bool isNativeStart)
{
    g_nativeStart = isNativeStart;
}
} // namespace AbilityRuntime
} // namespace OHOS
