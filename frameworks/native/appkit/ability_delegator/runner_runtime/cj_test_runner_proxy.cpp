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
 
#include "runner_runtime/cj_test_runner_proxy.h"
 
#include "hilog_wrapper.h"
 
namespace {
// g_cjTestRunnerFuncs is used to save cangjie functions.
// It is assigned by the global variable REGISTER_ABILITY on the cangjie side which invokes RegisterCJTestRunnerFuncs.
// And it is never released.
CJTestRunnerFuncs* g_cjTestRunnerFuncs = nullptr;
} // namespace
 
void RegisterCJTestRunnerFuncs(void (*registerFunc)(CJTestRunnerFuncs*))
{
    HILOG_INFO("RegisterCJTestRunnerFuncs start.");
    if (g_cjTestRunnerFuncs != nullptr) {
        HILOG_ERROR("Repeated registration for cangjie functions of CJTestRunner.");
        return;
    }
 
    if (registerFunc == nullptr) {
        HILOG_ERROR("RegisterCJTestRunnerFuncs failed, registerFunc is nullptr.");
        return;
    }
 
    g_cjTestRunnerFuncs = new CJTestRunnerFuncs();
    registerFunc(g_cjTestRunnerFuncs);
    HILOG_INFO("RegisterCJTestRunnerFuncs end.");
}
 
namespace OHOS {
namespace RunnerRuntime {
std::shared_ptr<CJTestRunnerProxy> CJTestRunnerProxy::LoadModule(const std::string& name)
{
    if (g_cjTestRunnerFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJTestRunner are not registered");
        return nullptr;
    }
    auto id = g_cjTestRunnerFuncs->cjTestRunnerCreate(name.c_str());
    if (id == 0) {
        HILOG_ERROR(
            "Failed to invoke CJTestRunnerProxy::LoadModule. Ability: %{public}s is not registered.", name.c_str());
        return nullptr;
    }
    return std::make_shared<CJTestRunnerProxy>(id);
}
 
CJTestRunnerProxy::~CJTestRunnerProxy()
{
    g_cjTestRunnerFuncs->cjTestRunnerRelease(id_);
    id_ = 0;
}
 
void CJTestRunnerProxy::OnRun() const
{
    if (g_cjTestRunnerFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJTestRunner are not registered");
        return;
    }
    g_cjTestRunnerFuncs->cjTestRunnerOnRun(id_);
}
 
void CJTestRunnerProxy::OnPrepare() const
{
    if (g_cjTestRunnerFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJTestRunner are not registered");
        return;
    }
    g_cjTestRunnerFuncs->cjTestRunnerOnPrepare(id_);
}
} // namespace RunnerRuntime
} // namespace OHOS