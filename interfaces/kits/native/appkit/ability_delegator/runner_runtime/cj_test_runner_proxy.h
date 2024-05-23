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
 
#ifndef FOUNDATION_APPEXECFWK_OHOS_CJ_TEST_RUNNER_PROXY_H
#define FOUNDATION_APPEXECFWK_OHOS_CJ_TEST_RUNNER_PROXY_H
 
#include <memory>
 
#include "cj_runtime.h"
 
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
class CJTestRunnerProxy {
public:
    static std::shared_ptr<CJTestRunnerProxy> LoadModule(const std::string& name);
    explicit CJTestRunnerProxy(int64_t id) : id_(id) {}
    ~CJTestRunnerProxy();
    void OnRun() const;
    void OnPrepare() const;
private:
    int64_t id_;
};
} // namespace RunnerRuntime
} // namespace OHOS
 
#endif // FOUNDATION_APPEXECFWK_OHOS_CJ_TEST_RUNNER_PROXY_H