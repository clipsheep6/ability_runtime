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

#ifndef OHOS_ABILITY_RUNTIME_RUNTIME_H
#define OHOS_ABILITY_RUNTIME_RUNTIME_H

#include <map>
#include <string>
#include <vector>

struct JsFrames {
    std::string functionName;
    std::string fileName;
    std::string pos;
    uintptr_t *nativePointer = nullptr;
};

class ModuleCheckerDelegate;

namespace OHOS {
namespace AppExecFwk {
class EventRunner;
} // namespace AppExecFwk
namespace AbilityRuntime {
class Runtime {
public:
    enum class Language {
        JS = 0,
    };

    struct Options {
        Language lang = Language::JS;
        std::string bundleName;
        std::string moduleName;
        std::string codePath;
        std::string bundleCodeDir;
        std::string hapPath;
        std::string arkNativeFilePath;
        std::string packagePathStr;
        std::vector<std::string> assetBasePathStr;
        std::shared_ptr<AppExecFwk::EventRunner> eventRunner;
        std::map<std::string, std::string> hapModulePath;
        bool loadAce = true;
        bool preload = false;
        bool isBundle = true;
        bool isDebugVersion = false;
        bool isJsFramework = false;
        bool isStageModel = true;
        bool isTestFramework = false;
        int32_t uid = -1;
        // ArkTsCard start
        bool isUnique = false;
        // ArkTsCard end
        std::shared_ptr<ModuleCheckerDelegate> moduleCheckerDelegate;
        int32_t apiTargetVersion = 0;
    };

    static std::unique_ptr<Runtime> Create(const Options& options);
    static void SavePreloaded(std::unique_ptr<Runtime>&& instance);
    static std::unique_ptr<Runtime> GetPreloaded();

    Runtime() = default;
    virtual ~Runtime() = default;

    virtual Language GetLanguage() const = 0;

    virtual void StartDebugMode(bool needBreakPoint, const std::string &processName, bool isDebug = true) = 0;
    virtual bool BuildJsStackInfoList(uint32_t tid, std::vector<JsFrames>& jsFrames) = 0;
    virtual void DumpHeapSnapshot(bool isPrivate) = 0;
    virtual void DestroyHeapProfiler() = 0;
    virtual void ForceFullGC() = 0;
    virtual void AllowCrossThreadExecution() = 0;
    virtual void GetHeapPrepare() = 0;
    virtual void NotifyApplicationState(bool isBackground) = 0;
    virtual bool SuspendVM(uint32_t tid) = 0;
    virtual void ResumeVM(uint32_t tid) = 0;
    virtual void PreloadSystemModule(const std::string& moduleName) = 0;
    virtual void FinishPreload() = 0;
    virtual bool LoadRepairPatch(const std::string& patchFile, const std::string& baseFile) = 0;
    virtual bool NotifyHotReloadPage() = 0;
    virtual bool UnLoadRepairPatch(const std::string& patchFile) = 0;
    virtual void RegisterQuickFixQueryFunc(const std::map<std::string, std::string>& moduleAndPath) = 0;
    virtual void StartProfiler(
        const std::string &perfCmd, bool needBreakPoint, const std::string &processName, bool isDebug = true) = 0;
    virtual void DoCleanWorkAfterStageCleaned() = 0;
    virtual void SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate>& moduleCheckerDelegate) const {}
    virtual void SetDeviceDisconnectCallback(const std::function<bool()> &cb) = 0;
    Runtime(const Runtime&) = delete;
    Runtime(Runtime&&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    Runtime& operator=(Runtime&&) = delete;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_RUNTIME_H
