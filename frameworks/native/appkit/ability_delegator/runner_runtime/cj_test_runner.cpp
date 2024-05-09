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
 
#include <regex>
 
#include "ability_delegator_registry.h"
#include "hilog_wrapper.h"
#include "runner_runtime/cj_test_runner.h"
#include "runner_runtime/cj_test_runner_proxy.h"
 
namespace OHOS {
namespace RunnerRuntime {
namespace {
const std::string CAPITALTESTRUNNER = "/ets/TestRunner/";
const std::string LOWERCASETESTRUNNER = "/libs/arm64/";
}  // namespace
    
std::unique_ptr<TestRunner> CJTestRunner::Create(const std::unique_ptr<Runtime> &runtime,
    const std::shared_ptr<AbilityDelegatorArgs> &args, const AppExecFwk::BundleInfo &bundleInfo)
{
    HILOG_INFO("CJTestRunner::Create start.");
    if (!runtime) {
        HILOG_ERROR("Invalid runtime");
        return nullptr;
    }
 
    if (!args) {
        HILOG_ERROR("Invalid ability delegator args");
        return nullptr;
    }
 
    auto pTestRunner = new (std::nothrow) CJTestRunner(static_cast<CJRuntime &>(*runtime), args, bundleInfo);
    if (!pTestRunner) {
        HILOG_ERROR("Failed to create test runner");
        return nullptr;
    }
 
    return std::unique_ptr<CJTestRunner>(pTestRunner);
}
 
CJTestRunner::CJTestRunner(CJRuntime &cjAbilityRuntime, const std::shared_ptr<AbilityDelegatorArgs> &args,
    const AppExecFwk::BundleInfo &bundleInfo) : cjAbilityRuntime_(cjAbilityRuntime)
{
    if (!cjAbilityRuntime_.IsAppLibLoaded()) {
        HILOG_ERROR("CJTestRunner: AppLib Not Loaded");
        return;
    }
    std::string moduleName = args->GetTestRunnerClassName();
    cjTestRunnerObj_ = CJTestRunnerProxy::LoadModule(moduleName);
    if (!cjTestRunnerObj_) {
        HILOG_ERROR("Not found %{public}s", moduleName.c_str());
        return;
    }
}
 
CJTestRunner::~CJTestRunner() = default;
 
bool CJTestRunner::Initialize()
{
    return true;
}
 
void CJTestRunner::Prepare()
{
    HILOG_INFO("Enter");
    TestRunner::Prepare();
    cjTestRunnerObj_->OnPrepare();
    HILOG_INFO("End");
}
 
void CJTestRunner::Run()
{
    HILOG_INFO("Enter");
    TestRunner::Run();
    cjTestRunnerObj_->OnRun();
    HILOG_INFO("End");
}
 
void CJTestRunner::ReportFinished(const std::string &msg)
{
    HILOG_INFO("Enter");
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null");
        return;
    }
 
    delegator->FinishUserTest(msg, -1);
}
 
void CJTestRunner::ReportStatus(const std::string &msg)
{
    HILOG_INFO("Enter");
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null");
        return;
    }
 
    delegator->Print(msg);
}
}  // namespace RunnerRuntime
}  // namespace OHOS