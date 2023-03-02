/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_RUNTIME_H
#define OHOS_ABILITY_RUNTIME_JS_RUNTIME_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "native_engine/native_engine.h"
#include "runtime.h"
#include "source_map.h"

namespace OHOS {
namespace AppExecFwk {
class EventHandler;
} // namespace AppExecFwk

namespace AbilityBase {
class Extractor;
} // namespace AbilityBase

namespace AbilityRuntime {
class TimerTask;
class ModSourceMap;

inline void *DetachCallbackFunc(NativeEngine *engine, void *value, void *)
{
    return value;
}

class JsRuntime : public Runtime {
public:
    static std::unique_ptr<Runtime> Create(const Options& options);

    static std::unique_ptr<NativeReference> LoadSystemModuleByEngine(NativeEngine* engine,
        const std::string& moduleName, NativeValue* const* argv, size_t argc);

    ~JsRuntime() override = default;

    NativeEngine& GetNativeEngine() const
    {
        return *nativeEngine_;
    }

    ModSourceMap& GetSourceMap() const
    {
        return *bindSourceMaps_;
    }

    Language GetLanguage() const override
    {
        return Language::JS;
    }

    std::unique_ptr<NativeReference> LoadModule(const std::string& moduleName, const std::string& modulePath,
        const std::string& hapPath, bool esmodule = false, bool useCommonChunk = false);
    std::unique_ptr<NativeReference> LoadSystemModule(
        const std::string& moduleName, NativeValue* const* argv = nullptr, size_t argc = 0);
    void PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime);
    void RemoveTask(const std::string& name);
    void DumpHeapSnapshot(bool isPrivate) override;
    bool BuildJsStackInfoList(uint32_t tid, std::vector<JsFrames>& jsFrames) override;
    void NotifyApplicationState(bool isBackground) override;

    bool RunSandboxScript(const std::string& path, const std::string& hapPath);
    virtual bool RunScript(const std::string& path, const std::string& hapPath, bool useCommonChunk = false) = 0;

    void PreloadSystemModule(const std::string& moduleName) override;
    void UpdateExtensionType(int32_t extensionType) override;
    void AllowCrossThreadExecution() const;

protected:
    JsRuntime() = default;

    virtual bool Initialize(const Options& options);
    void Deinitialize();

    virtual NativeValue* LoadJsBundle(const std::string& path, const std::string& hapPath, bool useCommonChunk = false);
    virtual NativeValue* LoadJsModule(const std::string& path, const std::string& hapPath) = 0;

    bool isArkEngine_ = false;
    bool debugMode_ = false;
    bool preloaded_ = false;
    bool isBundle_ = true;
    std::unique_ptr<NativeEngine> nativeEngine_;
    std::unique_ptr<ModSourceMap> bindSourceMaps_;
    std::string codePath_;
    std::string moduleName_;
    std::unique_ptr<NativeReference> methodRequireNapiRef_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    std::unordered_map<std::string, NativeReference*> modules_;

private:
    bool GetFileBuffer(const std::string& filePath, std::string& fileFullName, std::vector<uint8_t>& buffer);
};
}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_JS_RUNTIME_H
