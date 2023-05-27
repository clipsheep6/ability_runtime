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

#ifndef OHOS_ABILITY_RUNTIME_JS_WORKER_H
#define OHOS_ABILITY_RUNTIME_JS_WORKER_H

#include <string>
#include "bundle_mgr_proxy.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
void StartDebuggerInWorkerModule();
void InitWorkerFunc(NativeEngine* nativeEngine);
void OffWorkerFunc(NativeEngine* nativeEngine);
int32_t GetContainerId();
void UpdateContainerScope(int32_t id);
void RestoreContainerScope(int32_t id);

class AssetHelper final {
public:    
    explicit AssetHelper(const std::string& codePath, bool isDebugVersion, bool isBundle)
        : codePath_(codePath), isDebugVersion_(isDebugVersion), isBundle_(isBundle)
    {
        if (!codePath_.empty() && codePath.back() != '/') {
            codePath_.append("/");
        }
    }

    std::string NormalizedFileName(const std::string& fileName) const;

    void operator()(const std::string& uri, std::vector<uint8_t>& content, std::string &ami);

    sptr<AppExecFwk::IBundleMgr> GetBundleMgrProxy();

    bool ReadAmiData(const std::string& ami, std::vector<uint8_t>& content) const;

    bool ReadFilePathData(const std::string& filePath, std::vector<uint8_t>& content);

    std::string codePath_;
    bool isDebugVersion_ = false;
    bool isBundle_ = true;

private:
    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy_ = nullptr;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_WORKER_H