/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ABILITY_STAGE_PROXY_H
#define OHOS_ABILITY_RUNTIME_CJ_ABILITY_STAGE_PROXY_H

#include <memory>

#include "cj_ability_runtime.h"
#include "configuration.h"
#include "want.h"

#ifdef WINDOWS_PLATFORM
#define CJ_EXPORT __declspec(dllexport)
#else
#define CJ_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
struct CJConfiguration {
    int32_t colorMode;
    int32_t direction;
    int32_t displayId;
    int32_t screenDensity;
    bool isValid;
    const char* language;
};

struct CJAbilityStageFuncs {
    int64_t (*LoadAbilityStage)(const char* moduleName);
    void (*ReleaseAbilityStage)(int64_t handle);
    void (*AbilityStageOnCreate)(int64_t handle);
    char* (*AbilityStageOnAcceptWant)(int64_t handle, OHOS::AAFwk::Want* want);
    void (*AbilityStageOnConfigurationUpdated)(int64_t id, CJConfiguration configuration);
    void (*AbilityStageOnMemoryLevel)(int64_t id, int32_t level);
};

CJ_EXPORT void RegisterCJAbilityStageFuncs(void (*registerFunc)(CJAbilityStageFuncs* result));
}

namespace OHOS {
namespace AbilityRuntime {
class CJAbilityStageProxy {
public:
    static std::shared_ptr<CJAbilityStageProxy> LoadModule(const std::string& moduleName);

    explicit CJAbilityStageProxy(int64_t id) : id_(id) {}
    ~CJAbilityStageProxy();

    void OnCreate() const;
    std::string OnAcceptWant(const AAFwk::Want& want) const;
    void OnConfigurationUpdated(const std::shared_ptr<AppExecFwk::Configuration>& configuration) const;
    void OnMemoryLevel(int32_t level) const;

private:
    int64_t id_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CJ_ABILITY_STAGE_PROXY_H
