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
 
#ifndef OHOS_ABILITY_RUNTIME_CJ_TEST_RUNNER_OBJECT_H
#define OHOS_ABILITY_RUNTIME_CJ_TEST_RUNNER_OBJECT_H

#include <memory>

#ifdef WINDOWS_PLATFORM
#define CJ_EXPORT __declspec(dllexport)
#else
#define CJ_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
struct CJTestRunnerFuncs {
    int64_t (*cjTestRunnerCreate)(const char* name);
    void (*cjTestRunnerRelease)(int64_t id);
    void (*cjTestRunnerOnRun)(int64_t id);
    void (*cjTestRunnerOnPrepare)(int64_t id);
};

CJ_EXPORT void RegisterCJTestRunnerFuncs(void (*registerFunc)(CJTestRunnerFuncs*));
}

namespace OHOS {
namespace RunnerRuntime {
class CJTestRunnerObject {
public:
    static std::shared_ptr<CJTestRunnerObject> LoadModule(const std::string& name);
    explicit CJTestRunnerObject(int64_t id) : id_(id) {}
    ~CJTestRunnerObject();
    void OnRun() const;
    void OnPrepare() const;
private:
    int64_t id_ = -1;
};
} // namespace RunnerRuntime
} // namespace OHOS
 
#endif // OHOS_ABILITY_RUNTIME_CJ_TEST_RUNNER_OBJECT_H
