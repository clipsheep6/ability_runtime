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

#ifndef MOCK_RUNTIME_H
#define MOCK_RUNTIME_H

#include <gtest/gtest.h>

#include "js_runtime.h"

namespace OHOS {
namespace AbilityRuntime {
class MockRuntime : public Runtime {
public:
    MockRuntime() = default;
    ~MockRuntime() = default;

    Language GetLanguage() const override
    {
        return static_cast<Runtime::Language>(100);
    }

    bool BuildJsStackInfoList(uint32_t tid, std::vector<JsFrames>& jsFrames) override
    {
        GTEST_LOG_(INFO) << "MockRuntime::BuildJsStackInfoList called";
        return true;
    }
    void StartDebugMode(bool needBreakPoint) override {}
    void FinishPreload() override {}
    bool LoadRepairPatch(const std::string& patchFile, const std::string& baseFile) override
    {
        return true;
    }
    bool NotifyHotReloadPage() override
    {
        return true;
    }
    bool UnLoadRepairPatch(const std::string& patchFile) override
    {
        return true;
    }
    void DumpHeapSnapshot(bool isPrivate) override
    {
        return;
    }
    void NotifyApplicationState(bool isBackground) override
    {
        return;
    }
    void PreloadSystemModule(const std::string& moduleName) override
    {
        return;
    }
    void UpdateExtensionType(int32_t extensionType) override
    {
        return;
    }
    bool RunScript(const std::string& path, const std::string& hapPath, bool useCommonChunk = false)
    {
        return true;
    }
    bool Initialize(const Options& options)
    {
        return true;
    }
    void Deinitialize() {}
    NativeValue* LoadJsBundle(const std::string& path, const std::string& hapPath, bool useCommonChunk = false)
    {
        return nullptr;
    }
    NativeValue* LoadJsModule(const std::string& path, const std::string& hapPath)
    {
        return nullptr;
    }
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // MOCK_RUNTIME_H
