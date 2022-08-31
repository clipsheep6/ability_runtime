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

#ifndef OHOS_ABILITY_RUNTIME_RUNTIME_H
#define OHOS_ABILITY_RUNTIME_RUNTIME_H

#include <string>

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
        std::string codePath;
        std::string packagePath;
        std::string hapPath;
        std::shared_ptr<AppExecFwk::EventRunner> eventRunner;
        bool loadAce = true;
        bool preload = false;
    };

    static std::unique_ptr<Runtime> Create(const Options& options);
    static void SavePreloaded(std::unique_ptr<Runtime>&& instance);
    static std::unique_ptr<Runtime> GetPreloaded();

    Runtime() = default;
    virtual ~Runtime() = default;

    virtual Language GetLanguage() const = 0;

    virtual void StartDebugMode(bool needBreakPoint) = 0;
    virtual void DumpHeapSnapshot(bool isPrivate) = 0;
    virtual void NotifyApplicationState(bool isBackground) = 0;
    virtual void PreloadSystemModule(const std::string& moduleName) = 0;
    virtual void FinishPreload() = 0;

    Runtime(const Runtime&) = delete;
    Runtime(Runtime&&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    Runtime& operator=(Runtime&&) = delete;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_RUNTIME_H
