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

#include "runner_runtime/cj_test_runner.h"

#include <regex>

#include "ability_delegator_registry.h"
#include "hilog_wrapper.h"
#include "runner_runtime/cj_test_runner_object.h"

namespace OHOS {
namespace RunnerRuntime {

std::unique_ptr<TestRunner> CJTestRunner::Create(const std::unique_ptr<Runtime> &runtime,
    const std::shared_ptr<AbilityDelegatorArgs> &args, const AppExecFwk::BundleInfo &bundleInfo)
{
    HILOG_INFO("CJTestRunner::Create start.");
    if (!runtime) {
        HILOG_ERROR("Invalid runtime");
        return nullptr;
    }

    auto cjRuntime = static_cast<CJRuntime*>(runtime.get());
    if (!cjRuntime->IsAppLibLoaded()) {
        HILOG_ERROR("CJTestRunner: AppLib Not Loaded");
        return nullptr;
    }

    if (!args) {
        HILOG_ERROR("Invalid ability delegator args");
        return nullptr;
    }

    auto pTestRunner = new (std::nothrow) CJTestRunner(*cjRuntime, args, bundleInfo);
    if (!pTestRunner) {
        HILOG_ERROR("Failed to create test runner");
        return nullptr;
    }

    return std::unique_ptr<CJTestRunner>(pTestRunner);
}

CJTestRunner::CJTestRunner(CJRuntime &cjRuntime, const std::shared_ptr<AbilityDelegatorArgs> &args,
    const AppExecFwk::BundleInfo &bundleInfo) : cjRuntime_(cjRuntime)
{
    std::string moduleName = args->GetTestRunnerClassName();
    cjTestRunnerObj_ = CJTestRunnerObject::LoadModule(moduleName);
}

CJTestRunner::~CJTestRunner() = default;

bool CJTestRunner::Initialize()
{
    if (!cjRuntime_.IsAppLibLoaded()) {
        HILOG_ERROR("CJTestRunner: AppLib Not Loaded");
        return false;
    }
    if (!cjTestRunnerObj_) {
        HILOG_ERROR("CJTestRunnerObj does not exist, Initialize failed.");
        return false;
    }
    return true;
}

void CJTestRunner::Prepare()
{
    HILOG_INFO("Enter");
    TestRunner::Prepare();
    if (!cjTestRunnerObj_) {
        HILOG_ERROR("CJTestRunnerObj does not exist, Prepare failed.");
        return;
    }
    cjTestRunnerObj_->OnPrepare();
    HILOG_INFO("End");
}

void CJTestRunner::Run()
{
    HILOG_INFO("Enter");
    TestRunner::Run();
    if (!cjTestRunnerObj_) {
        HILOG_ERROR("CJTestRunnerObj does not exist, OnRun failed.");
        return;
    }
    cjTestRunnerObj_->OnRun();
    HILOG_INFO("End");
}

void CJTestRunner::ReportFinished(const std::string &msg)
{
    HILOG_INFO("Enter");
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null.");
        return;
    }

    delegator->FinishUserTest(msg, -1);
}

void CJTestRunner::ReportStatus(const std::string &msg)
{
    HILOG_INFO("Enter");
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null.");
        return;
    }

    delegator->Print(msg);
}
}  // namespace RunnerRuntime
}  // namespace OHOS
