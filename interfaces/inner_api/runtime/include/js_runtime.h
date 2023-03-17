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

namespace JsEnv {
class JsEnvironment;
} // namespace JsEnv

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

    JsRuntime();
    ~JsRuntime() override;

    NativeEngine& GetNativeEngine() const;

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
    bool RunScript(const std::string& path, const std::string& hapPath, bool useCommonChunk = false);

    void PreloadSystemModule(const std::string& moduleName) override;
    void UpdateExtensionType(int32_t extensionType) override;
    void StartDebugMode(bool needBreakPoint) override;
    bool LoadRepairPatch(const std::string& hqfFile, const std::string& hapPath) override;
    bool UnLoadRepairPatch(const std::string& hqfFile) override;
    bool NotifyHotReloadPage() override;

    NativeEngine* GetNativeEnginePointer() const;
    panda::ecmascript::EcmaVM* GetEcmaVm() const;

private:
    void FinishPreload() override;

    bool Initialize(const Options& options);
    void Deinitialize();

    NativeValue* LoadJsBundle(const std::string& path, const std::string& hapPath, bool useCommonChunk = false);
    NativeValue* LoadJsModule(const std::string& path, const std::string& hapPath);

    bool debugMode_ = false;
    bool preloaded_ = false;
    bool isBundle_ = true;
    std::unique_ptr<ModSourceMap> bindSourceMaps_;
    std::string codePath_;
    std::string moduleName_;
    std::unique_ptr<NativeReference> methodRequireNapiRef_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    std::unordered_map<std::string, NativeReference*> modules_;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv_ = nullptr;

    std::string bundleName_;
    uint32_t instanceId_ = 0;
    int32_t extensionType_ = 255; // default unspecified type.

    static std::atomic<bool> hasInstance;

private:
    bool GetFileBuffer(const std::string& filePath, std::string& fileFullName, std::vector<uint8_t>& buffer);
    bool CreateJsEnv(const Options& options);
    void PreloadAce(const Options& options);
    void InitSourceMap(const Options& options);
    void SetAppLibPath(const std::map<std::string, std::vector<std::string>>& appLibPaths);
    bool InitLoop(const std::shared_ptr<AppExecFwk::EventRunner>& eventRunner);
    inline bool IsUseAbilityRuntime(const Options& options) const;
};
}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_JS_RUNTIME_H
