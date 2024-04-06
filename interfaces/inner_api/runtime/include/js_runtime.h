/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

namespace panda::ecmascript {
class EcmaVM;
} // namespace panda::ecmascript
namespace panda {
struct HmsMap;
}
namespace OHOS {
namespace AppExecFwk {
class EventHandler;
} // namespace AppExecFwk

namespace AbilityBase {
class Extractor;
} // namespace AbilityBase

namespace JsEnv {
class JsEnvironment;
class SourceMapOperator;
struct UncaughtExceptionInfo;
} // namespace JsEnv

using AppLibPathMap = std::map<std::string, std::vector<std::string>>;

namespace AbilityRuntime {
class TimerTask;

inline void *DetachCallbackFunc(napi_env env, void *value, void *)
{
    return value;
}

class JsRuntime : public Runtime {
public:
    static std::unique_ptr<JsRuntime> Create(const Options& options);

    static void SetAppLibPath(const AppLibPathMap& appLibPaths, const bool& isSystemApp = false);

    static bool ReadSourceMapData(const std::string& hapPath, const std::string& sourceMapPath, std::string& content);

    static std::shared_ptr<Options> GetChildOptions();
    JsRuntime();
    ~JsRuntime() override;

    NativeEngine& GetNativeEngine() const;
    napi_env GetNapiEnv() const;

    Language GetLanguage() const override
    {
        return Language::JS;
    }

    void PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime);
    void PostSyncTask(const std::function<void()>& task, const std::string& name);
    void RemoveTask(const std::string& name);
    void DumpHeapSnapshot(bool isPrivate) override;
    void DumpCpuProfile(bool isPrivate) override;
    void DestroyHeapProfiler() override;
    void ForceFullGC() override;
    void ForceFullGC(uint32_t tid) override;
    void DumpHeapSnapshot(uint32_t tid, bool isFullGC) override;
    void AllowCrossThreadExecution() override;
    void GetHeapPrepare() override;
    bool BuildJsStackInfoList(uint32_t tid, std::vector<JsFrames>& jsFrames) override;
    void NotifyApplicationState(bool isBackground) override;
    bool SuspendVM(uint32_t tid) override;
    void ResumeVM(uint32_t tid) override;

    bool RunSandboxScript(const std::string& path, const std::string& hapPath);
    bool RunScript(const std::string& path, const std::string& hapPath, bool useCommonChunk = false);

    void PreloadSystemModule(const std::string& moduleName) override;

    void StartDebugMode(bool needBreakPoint, const std::string &processName, bool isDebug = true,
        bool isNativeStart = false) override;
    void StopDebugMode();
    bool LoadRepairPatch(const std::string& hqfFile, const std::string& hapPath) override;
    bool UnLoadRepairPatch(const std::string& hqfFile) override;
    bool NotifyHotReloadPage() override;
    void RegisterUncaughtExceptionHandler(const JsEnv::UncaughtExceptionInfo& uncaughtExceptionInfo);
    bool LoadScript(const std::string& path, std::vector<uint8_t>* buffer = nullptr, bool isBundle = false);
    bool LoadScript(const std::string& path, uint8_t* buffer, size_t len, bool isBundle);
    bool StartDebugger(bool needBreakPoint, uint32_t instanceId);
    void StopDebugger();

    NativeEngine* GetNativeEnginePointer() const;
    panda::ecmascript::EcmaVM* GetEcmaVm() const;

    void UpdateModuleNameAndAssetPath(const std::string& moduleName);
    void RegisterQuickFixQueryFunc(const std::map<std::string, std::string>& moduleAndPath) override;
    static bool GetFileBuffer(const std::string& filePath, std::string& fileFullName, std::vector<uint8_t>& buffer);

    void InitSourceMap(const std::shared_ptr<JsEnv::SourceMapOperator> operatorImpl);
    void FreeNativeReference(std::unique_ptr<NativeReference> reference);
    void FreeNativeReference(std::shared_ptr<NativeReference>&& reference);
    void StartProfiler(const std::string &perfCmd, bool needBreakPoint, const std::string &processName,
        bool isDebug = true, bool isNativeStart = false) override;

    void ReloadFormComponent(); // Reload ArkTS-Card component
    void DoCleanWorkAfterStageCleaned() override;
    void SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate>& moduleCheckerDelegate) const override;

    static std::unique_ptr<NativeReference> LoadSystemModuleByEngine(napi_env env,
        const std::string& moduleName, const napi_value* argv, size_t argc);
    std::unique_ptr<NativeReference> LoadModule(const std::string& moduleName, const std::string& modulePath,
        const std::string& hapPath, bool esmodule = false, bool useCommonChunk = false);
    std::unique_ptr<NativeReference> LoadSystemModule(
        const std::string& moduleName, const napi_value* argv = nullptr, size_t argc = 0);
    void SetDeviceDisconnectCallback(const std::function<bool()> &cb) override;

private:
    void FinishPreload() override;

    bool Initialize(const Options& options);
    void Deinitialize();
    static void SetChildOptions(const Options& options);

    int32_t JsperfProfilerCommandParse(const std::string &command, int32_t defaultValue);

    napi_value LoadJsBundle(const std::string& path, const std::string& hapPath, bool useCommonChunk = false);
    napi_value LoadJsModule(const std::string& path, const std::string& hapPath);

    bool preloaded_ = false;
    bool isBundle_ = true;
    std::string codePath_;
    std::string moduleName_;
    std::unique_ptr<NativeReference> methodRequireNapiRef_;
    std::unordered_map<std::string, NativeReference*> modules_;
    std::shared_ptr<JsEnv::JsEnvironment> jsEnv_ = nullptr;
    uint32_t instanceId_ = 0;
    std::string bundleName_;
    int32_t apiTargetVersion_ = 0;

    static std::atomic<bool> hasInstance;

    static std::shared_ptr<Options> childOptions_;
    
private:
    bool CreateJsEnv(const Options& options);
    void PreloadAce(const Options& options);
    bool InitLoop();
    inline bool IsUseAbilityRuntime(const Options& options) const;
    void FreeNativeReference(std::unique_ptr<NativeReference> uniqueNativeRef,
        std::shared_ptr<NativeReference>&& sharedNativeRef);
    void InitConsoleModule();
    void InitTimerModule();
    void InitWorkerModule(const Options& options);
    void ReInitJsEnvImpl(const Options& options);
    void PostPreload(const Options& options);
    void LoadAotFile(const Options& options);
    void SetRequestAotCallback();

    std::vector<panda::HmsMap> GetSystemKitsMap(uint32_t version);

    void GetPkgContextInfoListMap(const std::map<std::string, std::string> &contextInfoMap,
        std::map<std::string, std::vector<std::vector<std::string>>> &pkgContextInfoMap,
        std::map<std::string, std::string> &pkgAliasMap);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_RUNTIME_H
