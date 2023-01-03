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

#ifndef MOCK_JS_RUNTIME_H
#define MOCK_JS_RUNTIME_H

#include <gtest/gtest.h>

#include "js_runtime.h"
#include "mock_js_native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
class MockJsRuntime : public JsRuntime {
public:
    MockJsRuntime() = default;
    ~MockJsRuntime() = default;

    void StartDebugMode(bool needBreakPoint)
    {}
    void FinishPreload()
    {
        GTEST_LOG_(INFO) << "MockJsRuntime::FinishPreload called";
    }
    bool LoadRepairPatch(const std::string& patchFile, const std::string& baseFile)
    {
        return true;
    }
    bool NotifyHotReloadPage()
    {
        return true;
    }
    bool UnLoadRepairPatch(const std::string& patchFile)
    {
        return true;
    }
    bool RunScript(const std::string& path, const std::string& hapPath, bool useCommonChunk = false)
    {
        GTEST_LOG_(INFO) << "MockJsRuntime::RunScript called";
        return true;
    }
    bool Initialize(const Options& options)
    {
        GTEST_LOG_(INFO) << "MockJsRuntime::Initialize called";
        return true;
    }
    void Deinitialize()
    {}
    NativeValue* LoadJsBundle(const std::string& path, const std::string& hapPath, bool useCommonChunk = false)
    {
        GTEST_LOG_(INFO) << "MockJsRuntime::LoadJsBundle called";
        return nullptr;
    }
    NativeValue* LoadJsModule(const std::string& path, const std::string& hapPath)
    {
        GTEST_LOG_(INFO) << "MockJsRuntime::LoadJsModule called";
        return nullptr;
    }
    void PreloadSystemModule(const std::string& moduleName)
    {
        GTEST_LOG_(INFO) << "MockJsRuntime::PreloadSystemModule called";
    }
    std::unique_ptr<NativeReference> LoadModule(
        const std::string& moduleName, const std::string& modulePath, const std::string& hapPath, bool esmodule = false)
    {
        GTEST_LOG_(INFO) << "MockJsRuntime::LoadModule called";
        return nullptr;
    }
    std::unique_ptr<NativeReference> LoadSystemModule(
        const std::string& moduleName, NativeValue* const* argv = nullptr, size_t argc = 0)
    {
        GTEST_LOG_(INFO) << "MockJsRuntime::LoadSystemModule called";
        return nullptr;
    }
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // MOCK_JS_RUNTIME_H
