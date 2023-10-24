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
#include "js_runtime_utils.h"
#include "runner_runtime/js_test_runner.h"

extern const char _binary_delegator_mgmt_abc_start[];
extern const char _binary_delegator_mgmt_abc_end[];
namespace OHOS {
namespace RunnerRuntime {
namespace {
const std::string CAPITALTESTRUNNER = "/ets/TestRunner/";
const std::string LOWERCASETESTRUNNER = "/ets/testrunner/";
}  // namespace
    
std::unique_ptr<TestRunner> JsTestRunner::Create(const std::unique_ptr<Runtime> &runtime,
    const std::shared_ptr<AbilityDelegatorArgs> &args, const AppExecFwk::BundleInfo &bundleInfo, bool isFaJsModel)
{
    if (!runtime) {
        HILOG_ERROR("Invalid runtime");
        return nullptr;
    }

    if (!args) {
        HILOG_ERROR("Invalid ability delegator args");
        return nullptr;
    }

    auto pTestRunner = new (std::nothrow) JsTestRunner(static_cast<JsRuntime &>(*runtime), args, bundleInfo,
        isFaJsModel);
    if (!pTestRunner) {
        HILOG_ERROR("Failed to create test runner");
        return nullptr;
    }

    return std::unique_ptr<JsTestRunner>(pTestRunner);
}

JsTestRunner::JsTestRunner(
    JsRuntime &jsRuntime, const std::shared_ptr<AbilityDelegatorArgs> &args, const AppExecFwk::BundleInfo &bundleInfo,
    bool isFaJsModel)
    : jsRuntime_(jsRuntime), isFaJsModel_(isFaJsModel)
{
    std::string moduleName;
    if (args) {
        std::string srcPath;
        if (bundleInfo.hapModuleInfos.back().isModuleJson) {
            srcPath.append(args->GetTestModuleName());
            if (args->GetTestRunnerClassName().find("/") == std::string::npos) {
                srcPath.append(LOWERCASETESTRUNNER);
            }
            moduleName = args->GetTestModuleName();
        } else {
            srcPath.append(args->GetTestPackageName());
            srcPath.append("/assets/js/TestRunner/");
            moduleName = args->GetTestPackageName();
        }
        srcPath.append(args->GetTestRunnerClassName());
        srcPath.append(".abc");
        srcPath_ = srcPath;
    }
    HILOG_DEBUG("JsTestRunner srcPath is %{public}s", srcPath_.c_str());

    if (!moduleName.empty()) {
        for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
            if ((hapModuleInfo.isModuleJson && hapModuleInfo.name == moduleName) ||
                hapModuleInfo.package == moduleName) {
                hapPath_ = hapModuleInfo.hapPath;
                break;
            }
        }
    } else {
        hapPath_ = bundleInfo.hapModuleInfos.back().hapPath;
    }
    HILOG_DEBUG("JsTestRunner hapPath is %{public}s", hapPath_.c_str());

    if (isFaJsModel) {
        return;
    }

    moduleName.append("::").append("TestRunner");
    jsTestRunnerObj_ = jsRuntime_.LoadModule(moduleName, srcPath_, hapPath_,
        bundleInfo.hapModuleInfos.back().compileMode == AppExecFwk::CompileMode::ES_MODULE);
    if (!jsTestRunnerObj_ && srcPath_.find(LOWERCASETESTRUNNER) != std::string::npos) {
        HILOG_DEBUG("Not found %{public}s , retry load capital address", srcPath_.c_str());
        std::regex src_pattern(LOWERCASETESTRUNNER);
        srcPath_ = std::regex_replace(srcPath_, src_pattern, CAPITALTESTRUNNER);
        HILOG_DEBUG("Capital address is %{public}s", srcPath_.c_str());
        jsTestRunnerObj_ = jsRuntime_.LoadModule(moduleName, srcPath_, hapPath_,
            bundleInfo.hapModuleInfos.back().compileMode == AppExecFwk::CompileMode::ES_MODULE);
    }
}

JsTestRunner::~JsTestRunner() = default;

bool JsTestRunner::Initialize()
{
    if (isFaJsModel_) {
        if (!jsRuntime_.RunScript("/system/etc/strip.native.min.abc", "")) {
            HILOG_ERROR("RunScript err");
            return false;
        }
        std::vector<uint8_t> buffer((uint8_t*)_binary_delegator_mgmt_abc_start,
            (uint8_t*)_binary_delegator_mgmt_abc_end);
        napi_env env = jsRuntime_.GetNapiEnv();
        napi_value mgmtResult = nullptr;
        napi_run_buffer_script(env, buffer, &mgmtResult);
        if (mgmtResult == nullptr) {
            HILOG_ERROR("mgmtResult init error");
            return false;
        }
        if (!jsRuntime_.RunSandboxScript(srcPath_, hapPath_)) {
            HILOG_ERROR("RunScript srcPath_ err");
            return false;
        }
        napi_value object = nullptr;
        napi_get_global(env, &object);
        if (object == nullptr) {
            HILOG_ERROR("Failed to get global object");
            return false;
        }
        napi_value mainEntryFunc = nullptr;
        napi_get_named_property(env, object, "___mainEntry___", &mainEntryFunc);
        if (mainEntryFunc == nullptr) {
            HILOG_ERROR("Failed to get mainEntryFunc");
            return false;
        }
        napi_value value = nullptr;
        napi_get_global(env, &value);
        if (value == nullptr) {
            HILOG_ERROR("Failed to get global");
            return false;
        }
        napi_call_function(env, value, mainEntryFunc, 1, &value, nullptr);
    }
    return true;
}

void JsTestRunner::Prepare()
{
    HILOG_INFO("Enter");
    TestRunner::Prepare();
    CallObjectMethod("onPrepare");
    HILOG_INFO("End");
}

void JsTestRunner::Run()
{
    HILOG_INFO("Enter");
    TestRunner::Run();
    CallObjectMethod("onRun");
    HILOG_INFO("End");
}

void JsTestRunner::CallObjectMethod(const char *name, napi_value const *argv, size_t argc)
{
    HILOG_INFO("JsTestRunner::CallObjectMethod(%{public}s)", name);
    auto env = jsRuntime_.GetNapiEnv();
    if (isFaJsModel_) {
        napi_value global = nullptr;
        napi_get_global(env, &global);
        napi_value exportObject = nullptr;
        napi_get_named_property(env, global, "exports", &exportObject);
        if (!CheckTypeForNapiValue(env, exportObject, napi_object)) {
            HILOG_ERROR("Failed to get exportObject");
            return;
        }

        napi_value defaultObject = nullptr;
        napi_get_named_property(env, exportObject, "default", &defaultObject);
        if (!CheckTypeForNapiValue(env, defaultObject, napi_object)) {
            HILOG_ERROR("Failed to get defaultObject");
            return;
        }

        napi_value func = nullptr;
        napi_get_named_property(env, defaultObject, name, &func);
        if (!CheckTypeForNapiValue(env, func, napi_function)) {
            HILOG_ERROR("CallRequest func is %{public}s", func == nullptr ? "nullptr" : "not func");
            return;
        }
        napi_call_function(env, CreateJsUndefined(env), func, argc, argv, nullptr);
        return;
    }

    if (!jsTestRunnerObj_) {
        HILOG_ERROR("Not found %{public}s", srcPath_.c_str());
        ReportFinished("Not found " + srcPath_);
        return;
    }

    HandleScope handleScope(jsRuntime_);
    napi_value obj = jsTestRunnerObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get Test Runner object");
        ReportFinished("Failed to get Test Runner object");
        return;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, name, &methodOnCreate);
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from Test Runner object", name);
        ReportStatus("Failed to get " + std::string(name) + " from Test Runner object");
        return;
    }
    napi_call_function(env, obj, methodOnCreate, argc, argv, nullptr);
}

void JsTestRunner::ReportFinished(const std::string &msg)
{
    HILOG_INFO("Enter");
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null");
        return;
    }

    delegator->FinishUserTest(msg, -1);
}

void JsTestRunner::ReportStatus(const std::string &msg)
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
