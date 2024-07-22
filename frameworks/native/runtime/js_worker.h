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

#ifndef OHOS_ABILITY_RUNTIME_JS_WORKER_H
#define OHOS_ABILITY_RUNTIME_JS_WORKER_H

#include <string>
#include "bundle_mgr_proxy.h"
#include "js_environment_impl.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
void StartDebuggerInWorkerModule(bool isDebugApp, bool isNativeStart);
void InitWorkerFunc(NativeEngine* nativeEngine);
void OffWorkerFunc(NativeEngine* nativeEngine);
int32_t GetContainerId();
void UpdateContainerScope(int32_t id);
void RestoreContainerScope(int32_t id);
void SetJsFramework();

class AssetHelper final {
public:
    explicit AssetHelper(std::shared_ptr<JsEnv::WorkerInfo> workerInfo) : workerInfo_(workerInfo)
    {
        panda::panda_file::StringPacProtect codePath = panda::panda_file::StringPacProtect(workerInfo_->codePath);
        if (!(codePath.GetOriginString()).empty() && (codePath.GetOriginString()).back() != '/') {
            (workerInfo_->codePath).Append('/');
        }
    }

    virtual ~AssetHelper();

    void operator()(const std::string& uri, uint8_t** buff, size_t* buffSize, std::vector<uint8_t>& content,
        std::string& ami, bool& useSecureMem, bool isRestricted = false);

private:
    std::string NormalizedFileName(const std::string& fileName) const;

    bool ReadAmiData(const std::string& ami, uint8_t** buff, size_t* buffSize, std::vector<uint8_t>& content,
        bool& useSecureMem, bool isRestricted);

    bool ReadFilePathData(const std::string& filePath, uint8_t** buff, size_t* buffSize, std::vector<uint8_t>& content,
        bool& useSecureMem, bool isRestricted);

    void GetAmi(std::string& ami, const std::string& filePath);

    bool GetSafeData(const std::string& ami, uint8_t** buff, size_t* buffSize);
    
    bool GetIsStageModel();

    std::shared_ptr<JsEnv::WorkerInfo> workerInfo_ = nullptr;
    int fd_ = -1;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_WORKER_H