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

#include "hilog_wrapper.h"
#include "js_console_log.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int64_t ASSET_FILE_MAX_SIZE = 32 * 1024 * 1024;

void InitWorkerFunc(NativeEngine* nativeEngine)
{
    HILOG_INFO("RegisterInitWorkerFunc called");
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
}

bool ReadAssetData(const std::string& filePath, std::vector<uint8_t>& content, bool isDebugVersion)
{
    char path[PATH_MAX];
    if (realpath(filePath.c_str(), path) == nullptr) {
        HILOG_ERROR("ReadAssetData realpath(%{private}s) failed, errno = %{public}d", filePath.c_str(), errno);
        return false;
    }

    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream.is_open()) {
        HILOG_ERROR("ReadAssetData failed to open file %{private}s", filePath.c_str());
        return false;
    }

    auto fileLen = stream.tellg();
    if (!isDebugVersion && fileLen > ASSET_FILE_MAX_SIZE) {
        HILOG_ERROR("ReadAssetData failed, file is too large");
        return false;
    }

    content.resize(fileLen);

    stream.seekg(0);
    stream.read(reinterpret_cast<char*>(content.data()), content.size());
    return true;
}

struct AssetHelper final {
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
        size_t index = uri.find_last_of(".");
        if (index == std::string::npos) {
            HILOG_ERROR("Invalid uri");
            return;
        }

        ami = codePath_ + uri.substr(0, index) + ".abc";
        HILOG_INFO("Get asset, ami: %{private}s", ami.c_str());
        if (!ReadAssetData(ami, content, isDebugVersion_)) {
            HILOG_ERROR("Get asset content failed.");
            return;
        }
    }

    std::string codePath_;
    bool isDebugVersion_ = false;
};
}

void InitWorkerModule(NativeEngine& engine, const std::string& codePath, bool isDebugVersion)
{
    engine.SetInitWorkerFunc(InitWorkerFunc);
    engine.SetGetAssetFunc(AssetHelper(codePath, isDebugVersion));
}
} // namespace AbilityRuntime
} // namespace OHOS
