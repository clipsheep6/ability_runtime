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

#ifndef RUNNER_RUNTIME_JS_TEST_RUNNER_H
#define RUNNER_RUNTIME_JS_TEST_RUNNER_H

#include "bundle_info.h"
#include "js_runtime.h"
#include "test_runner.h"

class NativeReference;
class NativeValue;

namespace OHOS {
namespace RunnerRuntime {
using namespace AppExecFwk;
using namespace AbilityRuntime;

class JsTestRunner : public TestRunner {
public:
    static std::unique_ptr<TestRunner> Create(const std::unique_ptr<Runtime> &runtime,
        const std::shared_ptr<AbilityDelegatorArgs> &args, const AppExecFwk::BundleInfo &bundleInfo);

    ~JsTestRunner() override;

    void Prepare() override;
    void Run() override;

private:
    JsTestRunner(JsRuntime &jsRuntime,
        const std::shared_ptr<AbilityDelegatorArgs> &args, const AppExecFwk::BundleInfo &bundleInfo);

    void CallObjectMethod(const char *name, NativeValue *const *argv = nullptr, size_t argc = 0);
    void ReportFinished(const std::string &msg);
    void ReportStatus(const std::string &msg);

    JsRuntime &jsRuntime_;
    std::unique_ptr<NativeReference> jsTestRunnerObj_;
    std::string srcPath_;
};
}  // namespace RunnerRuntime
}  // namespace OHOS
#endif  // RUNNER_RUNTIME_JS_TEST_RUNNER_H