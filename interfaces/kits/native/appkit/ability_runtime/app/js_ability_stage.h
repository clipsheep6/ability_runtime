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

#ifndef OHOS_ABILITY_RUNTIME_JS_ABILITY_STAGE_H
#define OHOS_ABILITY_RUNTIME_JS_ABILITY_STAGE_H

#include "ability_delegator_infos.h"

#include <memory>
#include <vector>

#include "ability_stage.h"
#include "configuration.h"
#include "js_startup_task.h"
#include "resource_manager.h"
#include "nlohmann/json.hpp"
#include "native_engine/native_value.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
class JsRuntime;
class JsAbilityStage : public AbilityStage {
public:
    static std::shared_ptr<AbilityStage> Create(
        const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo);

    JsAbilityStage(JsRuntime& jsRuntime, std::unique_ptr<NativeReference>&& jsAbilityStageObj);
    ~JsAbilityStage() override;

    void Init(const std::shared_ptr<Context> &context) override;

    void OnCreate(const AAFwk::Want &want) const override;

    std::string OnAcceptWant(const AAFwk::Want &want) override;

    std::string OnNewProcessRequest(const AAFwk::Want &want) override;

    void OnConfigurationUpdated(const AppExecFwk::Configuration& configuration) override;

    void OnMemoryLevel(int32_t level) override;

    int32_t RunAutoStartupTask(bool &waitingForStartup) override;

private:
    napi_value CallObjectMethod(const char* name, napi_value const * argv = nullptr, size_t argc = 0);

    std::shared_ptr<AppExecFwk::DelegatorAbilityStageProperty> CreateStageProperty() const;

    std::string GetHapModuleProp(const std::string &propName) const;

    static bool UseCommonChunk(const AppExecFwk::HapModuleInfo& hapModuleInfo);

    int32_t RunAutoStartupTaskInner(bool &waitingForStartup);

    int32_t RegisterStartupTaskFromProfile(std::vector<JsStartupTask> &jsStartupTasks);
    
    bool GetProfileInfoFromResourceManager(std::vector<std::string> &profileInfo);
    
    bool AnalyzeProfileInfoAndRegisterStartupTask(
        std::vector<std::string> &profileInfo,
        std::vector<JsStartupTask> &jsStartupTasks);
    
    void SetOptionalParameters(const nlohmann::json &module, JsStartupTask &jsStartupTask);
    
    std::unique_ptr<NativeReference> LoadJsStartupTask(const std::string &srcEntry);
    
    bool GetResFromResMgr(
        const std::string &resName,
        const std::shared_ptr<Global::Resource::ResourceManager> &resMgr,
        bool isCompressed, std::vector<std::string> &profileInfo);
        
    bool IsFileExisted(const std::string &filePath);
    
    bool TransformFileToJsonString(const std::string &resPath, std::string &profile);

    JsRuntime& jsRuntime_;
    std::shared_ptr<NativeReference> jsAbilityStageObj_;
    std::shared_ptr<NativeReference> shellContextRef_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_ABILITY_STAGE_H
