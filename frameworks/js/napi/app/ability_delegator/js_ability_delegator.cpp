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

#include "js_ability_delegator.h"

#include <mutex>
#include "ability_delegator_registry.h"
#include "hilog_wrapper.h"
#include "js_ability_delegator_utils.h"
#include "js_context_utils.h"
#include "js_error_utils.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "napi_remote_object.h"
#include "shell_cmd_result.h"

namespace OHOS {
namespace AbilityDelegatorJs {
struct AbilityObjectBox {
    std::weak_ptr<NativeReference> object_;
};
struct AbilityStageObjBox {
    std::weak_ptr<NativeReference> object_;
};

struct ShellCmdResultBox {
    std::unique_ptr<ShellCmdResult> shellCmdResult_;
};

constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;

using namespace OHOS::AbilityRuntime;
std::map<std::shared_ptr<NativeReference>, std::shared_ptr<AbilityMonitor>> g_monitorRecord;
std::map<std::shared_ptr<NativeReference>, std::shared_ptr<AbilityStageMonitor>> g_stageMonitorRecord;
std::map<std::weak_ptr<NativeReference>, sptr<IRemoteObject>, std::owner_less<>> g_abilityRecord;
std::mutex g_mutexAbilityRecord;
std::mutex g_mtxStageMonitorRecord;

enum ERROR_CODE {
    INCORRECT_PARAMETERS    = 401,
};

std::unordered_map<int32_t, std::string> errorMap = {
    {INCORRECT_PARAMETERS,  "Incorrect parameters."},
};

#ifdef ENABLE_ERRCODE
constexpr int COMMON_FAILED = 16000100;
#else
constexpr int COMMON_FAILED = -1;
#endif

napi_value ThrowJsError(napi_env env, int32_t errCode)
{
#ifdef ENABLE_ERRCODE
    napi_value error = CreateJsError(env, errCode, errorMap[errCode]);
    napi_throw(env, error);
#endif
    return CreateJsUndefined(env);
}

void ResolveWithNoError(napi_env env, NapiAsyncTask &task, napi_value value = nullptr)
{
#ifdef ENABLE_ERRCODE
    if (value == nullptr) {
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    } else {
        task.ResolveWithNoError(env, value);
    }
#else
    if (value == nullptr) {
        task.Resolve(env, CreateJsNull(env));
    } else {
        task.Resolve(env, value);
    }
#endif
}

napi_value AttachAppContext(napi_env env, void *value, void *)
{
    HILOG_INFO("AttachAppContext");
    if (value == nullptr) {
        HILOG_WARN("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityRuntime::Context>*>(value)->lock();
    if (ptr == nullptr) {
        HILOG_WARN("invalid context.");
        return nullptr;
    }

    napi_value object = CreateJsBaseContext(env, ptr, true);
    napi_coerce_to_native_binding_object(env, object, DetachCallbackFunc, AttachAppContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Context>(ptr);
    napi_wrap(env, object, workContext,
        [](napi_env, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr app context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context> *>(data);
        }, nullptr, nullptr);
    return object;
}

JSAbilityDelegator::JSAbilityDelegator()
{
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        auto clearFunc = [](const std::shared_ptr<ADelegatorAbilityProperty> &property) {
            HILOG_INFO("Clear function is called");
            if (!property) {
                HILOG_ERROR("Invalid property");
                return;
            }

            std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
            for (auto it = g_abilityRecord.begin(); it != g_abilityRecord.end();) {
                if (it->second == property->token_) {
                    it = g_abilityRecord.erase(it);
                    continue;
                }
                ++it;
            }
        };

        delegator->RegisterClearFunc(clearFunc);
    }
}

void JSAbilityDelegator::Finalizer(napi_env env, void *data, void *hint)
{
    HILOG_INFO("enter");
    std::unique_ptr<JSAbilityDelegator>(static_cast<JSAbilityDelegator *>(data));
}

napi_value JSAbilityDelegator::AddAbilityMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnAddAbilityMonitor);
}

napi_value JSAbilityDelegator::AddAbilityMonitorSync(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnAddAbilityMonitorSync);
}

napi_value JSAbilityDelegator::RemoveAbilityMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnRemoveAbilityMonitor);
}

napi_value JSAbilityDelegator::RemoveAbilityMonitorSync(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnRemoveAbilityMonitorSync);
}

napi_value JSAbilityDelegator::WaitAbilityMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnWaitAbilityMonitor);
}

napi_value JSAbilityDelegator::AddAbilityStageMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnAddAbilityStageMonitor);
}

napi_value JSAbilityDelegator::AddAbilityStageMonitorSync(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnAddAbilityStageMonitorSync);
}

napi_value JSAbilityDelegator::RemoveAbilityStageMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnRemoveAbilityStageMonitor);
}

napi_value JSAbilityDelegator::RemoveAbilityStageMonitorSync(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnRemoveAbilityStageMonitorSync);
}

napi_value JSAbilityDelegator::WaitAbilityStageMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnWaitAbilityStageMonitor);
}

napi_value JSAbilityDelegator::GetAppContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnGetAppContext);
}

napi_value JSAbilityDelegator::GetAbilityState(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnGetAbilityState);
}

napi_value JSAbilityDelegator::GetCurrentTopAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnGetCurrentTopAbility);
}

napi_value JSAbilityDelegator::StartAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnStartAbility);
}

napi_value JSAbilityDelegator::DoAbilityForeground(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnDoAbilityForeground);
}

napi_value JSAbilityDelegator::DoAbilityBackground(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnDoAbilityBackground);
}

napi_value JSAbilityDelegator::Print(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnPrint);
}

napi_value JSAbilityDelegator::PrintSync(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnPrintSync);
}

napi_value JSAbilityDelegator::ExecuteShellCommand(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnExecuteShellCommand);
}

napi_value JSAbilityDelegator::FinishTest(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnFinishTest);
}

napi_value JSAbilityDelegator::SetMockList(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnSetMockList);
}

napi_value JSAbilityDelegator::OnAddAbilityMonitor(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseAbilityMonitorPara(env, info, monitor, false)) {
        HILOG_ERROR("Parse addAbilityMonitor parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete = [monitor](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnAddAbilityMonitor NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "addAbilityMonitor failed."));
            return;
        }
        delegator->AddAbilityMonitor(monitor);
        ResolveWithNoError(env, task);
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnAddAbilityMonitor",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnAddAbilityMonitorSync(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseAbilityMonitorPara(env, info, monitor, true)) {
        HILOG_ERROR("Parse addAbilityMonitorSync parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        delegator->AddAbilityMonitor(monitor);
    } else {
        ThrowError(env, COMMON_FAILED, "addAbilityMonitor failed.");
    }
    return CreateJsUndefined(env);
}

napi_value JSAbilityDelegator::OnAddAbilityStageMonitor(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    bool isExisted = false;
    std::shared_ptr<AbilityStageMonitor> monitor = nullptr;
    if (!ParseAbilityStageMonitorPara(env, info, monitor, isExisted, false)) {
        HILOG_ERROR("Parse addAbilityStageMonitor parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete = [monitor](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnAddAbilityStageMonitor NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "addAbilityStageMonitor failed."));
            return;
        }
        delegator->AddAbilityStageMonitor(monitor);
        ResolveWithNoError(env, task);
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnAddAbilityStageMonitor",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));

    if (!isExisted) {
        AddStageMonitorRecord(env, info.argv[INDEX_ZERO], monitor);
    }
    return result;
}

napi_value JSAbilityDelegator::OnAddAbilityStageMonitorSync(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    bool isExisted = false;
    std::shared_ptr<AbilityStageMonitor> monitor = nullptr;
    if (!ParseAbilityStageMonitorPara(env, info, monitor, isExisted, true)) {
        HILOG_ERROR("Parse addAbilityStageMonitorSync parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        ThrowError(env, COMMON_FAILED, "addAbilityStageMonitor failed.");
        return CreateJsUndefined(env);
    }
    delegator->AddAbilityStageMonitor(monitor);
    if (!isExisted) {
        AddStageMonitorRecord(env, info.argv[INDEX_ZERO], monitor);
    }
    return CreateJsUndefined(env);
}

napi_value JSAbilityDelegator::OnRemoveAbilityMonitor(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseAbilityMonitorPara(env, info, monitor, false)) {
        HILOG_ERROR("Parse removeAbilityMonitor parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete =
        [monitor](napi_env env, NapiAsyncTask &task, int32_t status) mutable {
        HILOG_INFO("OnRemoveAbilityMonitor NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "removeAbilityMonitor failed."));
            return;
        }
        delegator->RemoveAbilityMonitor(monitor);
        ResolveWithNoError(env, task);
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnRemoveAbilityMonitor",
        env, CreateAsyncTaskWithLastParam(env,
        lastParam, nullptr, std::move(complete), &result));

    if (AbilityDelegatorRegistry::GetAbilityDelegator()) {
        for (auto iter = g_monitorRecord.begin(); iter != g_monitorRecord.end(); ++iter) {
            std::shared_ptr<NativeReference> jsMonitor = iter->first;
            bool isEquals = false;
            napi_strict_equals(env, (info.argv[INDEX_ZERO]), jsMonitor->GetNapiValue(), &isEquals);
            if (isEquals) {
                g_monitorRecord.erase(iter);
                break;
            }
        }
    }
    return result;
}


napi_value JSAbilityDelegator::OnRemoveAbilityMonitorSync(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseAbilityMonitorPara(env, info, monitor, true)) {
        HILOG_ERROR("Parse removeAbilityMonitorSync parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        ThrowError(env, COMMON_FAILED, "removeAbilityMonitor failed.");
        return CreateJsUndefined(env);
    }
    delegator->RemoveAbilityMonitor(monitor);
    for (auto iter = g_monitorRecord.begin(); iter != g_monitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        bool isEquals = false;
        napi_strict_equals(env, (info.argv[INDEX_ZERO]), jsMonitor->GetNapiValue(), &isEquals);
        if (isEquals) {
            g_monitorRecord.erase(iter);
            break;
        }
    }
    return CreateJsUndefined(env);
}

napi_value JSAbilityDelegator::OnRemoveAbilityStageMonitor(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    bool isExisted = false;
    std::shared_ptr<AbilityStageMonitor> monitor = nullptr;
    if (!ParseAbilityStageMonitorPara(env, info, monitor, isExisted, false)) {
        HILOG_ERROR("Parse removeAbilityStageMonitor parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete =
        [monitor](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnRemoveAbilityStageMonitor NapiAsyncTask is called");

        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "removeAbilityStageMonitor failed."));
            return;
        }
        delegator->RemoveAbilityStageMonitor(monitor);
        ResolveWithNoError(env, task);
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnRemoveAbilityStageMonitor", env,
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));

    if (isExisted) {
        RemoveStageMonitorRecord(env, info.argv[INDEX_ZERO]);
    }
    return result;
}

napi_value JSAbilityDelegator::OnRemoveAbilityStageMonitorSync(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    bool isExisted = false;
    std::shared_ptr<AbilityStageMonitor> monitor = nullptr;
    if (!ParseAbilityStageMonitorPara(env, info, monitor, isExisted, true)) {
        HILOG_ERROR("Parse removeAbilityStageMonitorSync parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        ThrowError(env, COMMON_FAILED, "removeAbilityStageMonitor failed.");
        return CreateJsUndefined(env);
    }
    delegator->RemoveAbilityStageMonitor(monitor);
    if (isExisted) {
        RemoveStageMonitorRecord(env, info.argv[INDEX_ZERO]);
    }
    return CreateJsUndefined(env);
}

napi_value JSAbilityDelegator::OnWaitAbilityMonitor(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    TimeoutCallback opt {false, false};
    int64_t timeout = 0;
    if (!ParseWaitAbilityMonitorPara(env, info, monitor, opt, timeout)) {
        HILOG_ERROR("Parse waitAbilityMonitor parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    auto abilityObjectBox = std::make_shared<AbilityObjectBox>();
    NapiAsyncTask::ExecuteCallback execute = [monitor, timeout, opt, abilityObjectBox]() {
        HILOG_INFO("OnWaitAbilityMonitor NapiAsyncTask ExecuteCallback is called");
        if (!abilityObjectBox) {
            HILOG_ERROR("OnWaitAbilityMonitor NapiAsyncTask ExecuteCallback, Invalid abilityObjectBox");
            return;
        }

        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            HILOG_ERROR("OnWaitAbilityMonitor NapiAsyncTask ExecuteCallback, Invalid delegator");
            return;
        }

        std::shared_ptr<ADelegatorAbilityProperty> property = opt.hasTimeoutPara ?
            delegator->WaitAbilityMonitor(monitor, timeout) : delegator->WaitAbilityMonitor(monitor);
        if (!property || property->object_.expired()) {
            HILOG_ERROR("Invalid property");
            return;
        }

        abilityObjectBox->object_ = property->object_;
        std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
        g_abilityRecord.emplace(property->object_, property->token_);
    };

    NapiAsyncTask::CompleteCallback complete = [abilityObjectBox](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnWaitAbilityMonitor NapiAsyncTask CompleteCallback is called");
        if (abilityObjectBox && !abilityObjectBox->object_.expired()) {
            ResolveWithNoError(env, task, abilityObjectBox->object_.lock()->GetNapiValue());
        } else {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "waitAbilityMonitor failed."));
        }
    };

    napi_value lastParam = nullptr;
    if (opt.hasCallbackPara) {
        lastParam = opt.hasTimeoutPara ? info.argv[INDEX_TWO] : info.argv[INDEX_ONE];
    }

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnWaitAbilityMonitor",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnWaitAbilityStageMonitor(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityStageMonitor> monitor = nullptr;
    TimeoutCallback opt {false, false};
    int64_t timeout = 0;
    if (!ParseWaitAbilityStageMonitorPara(env, info, monitor, opt, timeout)) {
        HILOG_ERROR("Parse waitAbilityStageMonitor parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    auto abilityStageObjBox = std::make_shared<AbilityStageObjBox>();
    NapiAsyncTask::ExecuteCallback execute = [monitor, timeout, opt, abilityStageObjBox]() {
        HILOG_INFO("OnWaitAbilityStageMonitor NapiAsyncTask ExecuteCallback is called");
        if (!abilityStageObjBox) {
            HILOG_ERROR("OnWaitAbilityStageMonitor NapiAsyncTask ExecuteCallback, Invalid abilityStageObjBox");
            return;
        }
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            HILOG_ERROR("OnWaitAbilityMonitor NapiAsyncTask ExecuteCallback, Invalid delegator");
            return;
        }
        std::shared_ptr<DelegatorAbilityStageProperty> result;
        result = opt.hasTimeoutPara ?
            delegator->WaitAbilityStageMonitor(monitor, timeout) : delegator->WaitAbilityStageMonitor(monitor);
        if (!result || result->object_.expired()) {
            HILOG_ERROR("WaitAbilityStageMonitor failed");
            return;
        }
        abilityStageObjBox->object_ = result->object_;
    };

    NapiAsyncTask::CompleteCallback complete = [abilityStageObjBox](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnWaitAbilityMonitor NapiAsyncTask CompleteCallback is called");
        if (abilityStageObjBox && !abilityStageObjBox->object_.expired()) {
            ResolveWithNoError(env, task, abilityStageObjBox->object_.lock()->GetNapiValue());
        } else {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "waitAbilityStageMonitor failed."));
        }
    };
    napi_value lastParam = nullptr;
    if (opt.hasCallbackPara) {
        lastParam = opt.hasTimeoutPara ? info.argv[INDEX_TWO] : info.argv[INDEX_ONE];
    }
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnWaitAbilityStageMonitor",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnPrint(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string msg;
    if (!ParsePrintPara(env, info, msg)) {
        HILOG_ERROR("Parse print parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete = [msg](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnPrint NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "print failed."));
            return;
        }
        delegator->Print(msg);
        ResolveWithNoError(env, task);
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnPrint",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnPrintSync(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string msg;
    if (!ParsePrintPara(env, info, msg)) {
        HILOG_ERROR("Parse print parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("Invalid delegator");
        return CreateJsUndefined(env);
    }

    delegator->Print(msg);
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::OnExecuteShellCommand(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string cmd;
    TimeoutCallback opt {false, false};
    int64_t timeoutSecs = 0;
    if (!ParseExecuteShellCommandPara(env, info, cmd, opt, timeoutSecs)) {
        HILOG_ERROR("Parse executeShellCommand parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    auto shellCmdResultBox = std::make_shared<ShellCmdResultBox>();
    NapiAsyncTask::ExecuteCallback execute = [cmd, timeoutSecs, shellCmdResultBox]() {
        HILOG_INFO("OnExecuteShellCommand NapiAsyncTask ExecuteCallback is called");
        if (!shellCmdResultBox) {
            HILOG_ERROR("OnExecuteShellCommand NapiAsyncTask ExecuteCallback, Invalid shellCmdResultBox");
            return;
        }

        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            HILOG_ERROR("OnExecuteShellCommand NapiAsyncTask ExecuteCallback, Invalid delegator");
            return;
        }

        shellCmdResultBox->shellCmdResult_ = delegator->ExecuteShellCommand(cmd, timeoutSecs);
    };

    NapiAsyncTask::CompleteCallback complete = [shellCmdResultBox](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnExecuteShellCommand NapiAsyncTask CompleteCallback is called");
        if (!shellCmdResultBox) {
            HILOG_ERROR("OnExecuteShellCommand NapiAsyncTask CompleteCallback, Invalid shellCmdResultBox");
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "executeShellCommand failed."));
            return;
        }

        napi_value result = CreateJsShellCmdResult(env, shellCmdResultBox->shellCmdResult_);
        if (result) {
            ResolveWithNoError(env, task, result);
        } else {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "executeShellCommand failed."));
        }
    };

    napi_value lastParam = nullptr;
    if (opt.hasCallbackPara) {
        lastParam = opt.hasTimeoutPara ? info.argv[INDEX_TWO] : info.argv[INDEX_ONE];
    }

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnExecuteShellCommand:" + cmd,
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnGetAppContext(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null");
        return CreateJsNull(env);
    }
    std::shared_ptr<AbilityRuntime::Context> context = delegator->GetAppContext();
    if (!context) {
        HILOG_ERROR("context is null");
        return CreateJsNull(env);
    }
    napi_value value = CreateJsBaseContext(env, context, false);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Context>(context);
    if (workContext == nullptr) {
        HILOG_ERROR("invalid workContext");
        return CreateJsNull(env);
    }
    napi_coerce_to_native_binding_object(env, value, DetachCallbackFunc, AttachAppContext, workContext, nullptr);
    napi_wrap(env, value, workContext,
        [](napi_env, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr app context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context> *>(data);
        }, nullptr, nullptr);
    return value;
}

napi_value JSAbilityDelegator::OnGetAbilityState(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return CreateJsUndefined(env);
    }

    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    if (!ParseAbilityPara(env, info.argv[INDEX_ZERO], remoteObject)) {
        HILOG_ERROR("Parse ability parameter failed");
        return CreateJsUndefined(env);
    }

    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null");
        return CreateJsNull(env);
    }
    AbilityDelegator::AbilityState lifeState = delegator->GetAbilityState(remoteObject);
    AbilityLifecycleState abilityLifeState = AbilityLifecycleState::UNINITIALIZED;
    AbilityLifecycleStateToJs(lifeState, abilityLifeState);
    return CreateJsValue(env, static_cast<int>(abilityLifeState));
}

napi_value JSAbilityDelegator::OnGetCurrentTopAbility(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    if (info.argc >= ARGC_ONE && !AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_function)) {
        HILOG_ERROR("Parse getCurrentTopAbility parameter failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete = [this](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnGetCurrentTopAbility NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            HILOG_ERROR("Invalid delegator");
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "getCurrentTopAbility failed."));
            return;
        }

        auto property = delegator->GetCurrentTopAbility();
        if (!property || property->object_.expired()) {
            HILOG_ERROR("Invalid property");
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "getCurrentTopAbility failed."));
        } else {
            {
                std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
                g_abilityRecord.emplace(property->object_, property->token_);
            }
            ResolveWithNoError(env, task, property->object_.lock()->GetNapiValue());
        }
    };

    napi_value lastParam = (info.argc >= ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSAbilityDelegator::OnGetCurrentTopAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnStartAbility(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    AAFwk::Want want;
    if (!ParseStartAbilityPara(env, info, want)) {
        HILOG_ERROR("Parse startAbility parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete = [want](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnStartAbility NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "startAbility failed."));
            return;
        }
        int result = delegator->StartAbility(want);
        if (result) {
            task.Reject(env, CreateJsError(env, result, "startAbility failed."));
        } else {
            ResolveWithNoError(env, task);
        }
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSAbilityDelegator::OnStartAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnDoAbilityForeground(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    if (!ParseAbilityCommonPara(env, info, remoteObject)) {
        HILOG_ERROR("Parse doAbilityForeground parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete = [remoteObject](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnDoAbilityForeground NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "doAbilityForeground failed."));
            return;
        }
        if (delegator->DoAbilityForeground(remoteObject)) {
            ResolveWithNoError(env, task, CreateJsNull(env));
        } else {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "doAbilityForeground failed."));
        }
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSAbilityDelegator::OnDoAbilityForeground",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnDoAbilityBackground(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    if (!ParseAbilityCommonPara(env, info, remoteObject)) {
        HILOG_ERROR("Parse doAbilityBackground parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete = [remoteObject](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnDoAbilityBackground NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "doAbilityBackground failed."));
            return;
        }
        if (delegator->DoAbilityBackground(remoteObject)) {
            ResolveWithNoError(env, task, CreateJsNull(env));
        } else {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "doAbilityBackground failed."));
        }
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSAbilityDelegator::OnDoAbilityBackground",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnFinishTest(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string msg;
    int64_t code = 0;
    if (!ParseFinishTestPara(env, info, msg, code)) {
        HILOG_ERROR("Parse finishTest parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete = [msg, code](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnFinishTest NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "finishTest failed."));
            return;
        }
        delegator->FinishUserTest(msg, code);
        ResolveWithNoError(env, task);
    };
    napi_value lastParam = (info.argc > ARGC_TWO) ? info.argv[INDEX_TWO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnFinishTest",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnSetMockList(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::map<std::string, std::string> mockList;
    if (!ParseMockListPara(env, info, mockList)) {
        HILOG_ERROR("Parse setMockList parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }
    auto engine = reinterpret_cast<NativeEngine*>(env);
    engine->SetMockModuleList(mockList);
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseMonitorPara(
    napi_env env, napi_value value, std::shared_ptr<AbilityMonitor> &monitor)
{
    HILOG_INFO("enter, monitorRecord size = %{public}zu", g_monitorRecord.size());

    for (auto iter = g_monitorRecord.begin(); iter != g_monitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        bool isEquals = false;
        napi_strict_equals(env, value, jsMonitor->GetNapiValue(), &isEquals);
        if (isEquals) {
            HILOG_ERROR("monitor existed");
            monitor = iter->second;
            return monitor ? CreateJsNull(env) : nullptr;
        }
    }

    napi_value abilityNameValue = nullptr;
    napi_get_named_property(env, value, "abilityName", &abilityNameValue);
    if (abilityNameValue == nullptr) {
        HILOG_ERROR("Failed to get property abilityName");
        return nullptr;
    }

    std::string abilityName;
    if (!ConvertFromJsValue(env, abilityNameValue, abilityName)) {
        return nullptr;
    }

    std::string moduleName = "";
    napi_value moduleNameValue = nullptr;
    napi_get_named_property(env, value, "moduleName", &moduleNameValue);
    if (moduleNameValue != nullptr && !ConvertFromJsValue(env, moduleNameValue, moduleName)) {
        HILOG_WARN("Failed to get property moduleName");
        moduleName = "";
    }

    std::shared_ptr<JSAbilityMonitor> abilityMonitor = nullptr;
    if (moduleName.empty()) {
        abilityMonitor = std::make_shared<JSAbilityMonitor>(abilityName);
        abilityMonitor->SetJsAbilityMonitorEnv(env);
        abilityMonitor->SetJsAbilityMonitor(value);
        monitor = std::make_shared<AbilityMonitor>(abilityName, abilityMonitor);
    } else {
        abilityMonitor = std::make_shared<JSAbilityMonitor>(abilityName, moduleName);
        abilityMonitor->SetJsAbilityMonitorEnv(env);
        abilityMonitor->SetJsAbilityMonitor(value);
        monitor = std::make_shared<AbilityMonitor>(abilityName, moduleName, abilityMonitor);
    }

    std::shared_ptr<NativeReference> reference = nullptr;
    napi_ref ref = nullptr;
    napi_create_reference(env, value, 1, &ref);
    reference.reset(reinterpret_cast<NativeReference*>(ref));
    g_monitorRecord.emplace(reference, monitor);

    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseStageMonitorPara(
    napi_env env, napi_value value, std::shared_ptr<AbilityStageMonitor> &monitor, bool &isExisted)
{
    HILOG_INFO("enter, stageMonitorRecord size = %{public}zu", g_stageMonitorRecord.size());

    isExisted = false;
    for (auto iter = g_stageMonitorRecord.begin(); iter != g_stageMonitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        bool isEquals = false;
        napi_strict_equals(env, value, jsMonitor->GetNapiValue(), &isEquals);
        if (isEquals) {
            HILOG_WARN("AbilityStage monitor existed");
            isExisted = true;
            monitor = iter->second;
            return monitor ? CreateJsNull(env) : nullptr;
        }
    }

    napi_value moduleNameValue = nullptr;
    napi_get_named_property(env, value, "moduleName", &moduleNameValue);
    if (moduleNameValue == nullptr) {
        HILOG_ERROR("Failed to get property moduleName");
        return nullptr;
    }
    std::string moduleName;
    if (!ConvertFromJsValue(env, moduleNameValue, moduleName)) {
        HILOG_ERROR("Failed to get moduleName from JsValue");
        return nullptr;
    }
    napi_value srcEntranceValue = nullptr;
    napi_get_named_property(env, value, "srcEntrance", &srcEntranceValue);
    if (srcEntranceValue == nullptr) {
        HILOG_ERROR("Failed to get property srcEntrance");
        return nullptr;
    }
    std::string srcEntrance;
    if (!ConvertFromJsValue(env, srcEntranceValue, srcEntrance)) {
        HILOG_ERROR("Failed to get srcEntrance from JsValue");
        return nullptr;
    }

    monitor = std::make_shared<AbilityStageMonitor>(moduleName, srcEntrance);
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseAbilityPara(
    napi_env env, napi_value value, sptr<OHOS::IRemoteObject> &remoteObject)
{
    HILOG_INFO("enter");

    std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
    for (auto iter = g_abilityRecord.begin(); iter != g_abilityRecord.end();) {
        if (iter->first.expired()) {
            iter = g_abilityRecord.erase(iter);
            continue;
        }

        bool isEquals = false;
        napi_strict_equals(env, value, iter->first.lock()->GetNapiValue(), &isEquals);
        if (isEquals) {
            remoteObject = iter->second;
            HILOG_INFO("Ability exist");
            return remoteObject ? CreateJsNull(env) : nullptr;
        }

        ++iter;
    }

    HILOG_ERROR("Ability doesn't exist");
    remoteObject = nullptr;
    return nullptr;
}

napi_value JSAbilityDelegator::CreateAbilityObject(napi_env env, const sptr<IRemoteObject> &remoteObject)
{
    HILOG_INFO("enter");

    if (!remoteObject) {
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    std::shared_ptr<NativeReference> reference = nullptr;
    napi_ref ref = nullptr;
    napi_create_reference(env, objValue, 1, &ref);
    reference.reset(reinterpret_cast<NativeReference*>(ref));

    std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
    g_abilityRecord[reference] = remoteObject;
    return objValue;
}

void JSAbilityDelegator::AbilityLifecycleStateToJs(
    const AbilityDelegator::AbilityState &lifeState, AbilityLifecycleState &abilityLifeState)
{
    HILOG_INFO("enter and lifeState = %{public}d", static_cast<int32_t>(lifeState));
    switch (lifeState) {
        case AbilityDelegator::AbilityState::STARTED:
            abilityLifeState = AbilityLifecycleState::CREATE;
            break;
        case AbilityDelegator::AbilityState::FOREGROUND:
            abilityLifeState = AbilityLifecycleState::FOREGROUND;
            break;
        case AbilityDelegator::AbilityState::BACKGROUND:
            abilityLifeState = AbilityLifecycleState::BACKGROUND;
            break;
        case AbilityDelegator::AbilityState::STOPPED:
            abilityLifeState = AbilityLifecycleState::DESTROY;
            break;
        default:
            abilityLifeState = AbilityLifecycleState::UNINITIALIZED;
            break;
    }
}

napi_value JSAbilityDelegator::ParseAbilityMonitorPara(
    napi_env env, NapiCallbackInfo& info, std::shared_ptr<AbilityMonitor> &monitor, bool isSync)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ParseMonitorPara(env, info.argv[INDEX_ZERO], monitor)) {
        HILOG_ERROR("Parse monitor parameters failed");
        return nullptr;
    }
    
    if (isSync) {
        return CreateJsNull(env);
    }

    if (info.argc > ARGC_ONE) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
            HILOG_ERROR("ParseAbilityMonitorPara, Parse callback parameters failed");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseAbilityStageMonitorPara(napi_env env, NapiCallbackInfo& info,
    std::shared_ptr<AbilityStageMonitor> &monitor, bool &isExisted, bool isSync)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ParseStageMonitorPara(env, info.argv[INDEX_ZERO], monitor, isExisted)) {
        HILOG_ERROR("Parse stage monitor parameters failed");
        return nullptr;
    }
    
    if (isSync) {
        return CreateJsNull(env);
    }

    if (info.argc > ARGC_ONE) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
            HILOG_ERROR("ParseAbilityStageMonitorPara, Parse callback parameters failed");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseWaitAbilityMonitorPara(napi_env env, NapiCallbackInfo& info,
    std::shared_ptr<AbilityMonitor> &monitor, TimeoutCallback &opt, int64_t &timeout)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ParseMonitorPara(env, info.argv[INDEX_ZERO], monitor)) {
        HILOG_ERROR("Monitor parse parameters failed");
        return nullptr;
    }

    if (!ParseTimeoutCallbackPara(env, info, opt, timeout)) {
        HILOG_ERROR("TimeoutCallback parse parameters failed");
        return nullptr;
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseWaitAbilityStageMonitorPara(napi_env env, NapiCallbackInfo& info,
    std::shared_ptr<AbilityStageMonitor> &monitor, TimeoutCallback &opt, int64_t &timeout)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    bool isExisted = false;
    if (!ParseStageMonitorPara(env, info.argv[INDEX_ZERO], monitor, isExisted)) {
        HILOG_ERROR("Stage monitor parse parameters failed");
        return nullptr;
    }
    if (!ParseTimeoutCallbackPara(env, info, opt, timeout)) {
        HILOG_ERROR("TimeoutCallback parse parameters failed");
        return nullptr;
    }
    if (!isExisted) {
        AddStageMonitorRecord(env, info.argv[INDEX_ZERO], monitor);
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseTimeoutCallbackPara(
    napi_env env, NapiCallbackInfo& info, TimeoutCallback &opt, int64_t &timeout)
{
    HILOG_INFO("enter");

    opt.hasCallbackPara = false;
    opt.hasTimeoutPara = false;

    if (info.argc >= ARGC_TWO) {
        if (ConvertFromJsValue(env, info.argv[INDEX_ONE], timeout)) {
            opt.hasTimeoutPara = true;
        } else {
            if (info.argv[INDEX_ONE] == nullptr) {
                HILOG_WARN("info.argv[1] is null");
            } else if (AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
                opt.hasCallbackPara = true;
                return CreateJsNull(env);
            } else {
                return nullptr;
            }
        }

        if (info.argc > ARGC_TWO) {
            if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_TWO], napi_function)) {
                if (info.argv[INDEX_TWO] == nullptr) {
                    HILOG_WARN("info.argv[2] is null");
                    return CreateJsNull(env);
                }
                return nullptr;
            }
            opt.hasCallbackPara = true;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParsePrintPara(napi_env env, NapiCallbackInfo& info, std::string &msg)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], msg)) {
        HILOG_ERROR("Parse msg parameter failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
            HILOG_ERROR("Parse callback parameter failed");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseExecuteShellCommandPara(
    napi_env env, NapiCallbackInfo& info, std::string &cmd, TimeoutCallback &opt, int64_t &timeout)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], cmd)) {
        HILOG_ERROR("Parse cmd parameter failed");
        return nullptr;
    }
    if (!ParseTimeoutCallbackPara(env, info, opt, timeout)) {
        HILOG_ERROR("Parse timeOut callback parameters failed");
        return nullptr;
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseAbilityCommonPara(
    napi_env env, NapiCallbackInfo& info, sptr<OHOS::IRemoteObject> &remoteObject)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ParseAbilityPara(env, info.argv[INDEX_ZERO], remoteObject)) {
        HILOG_ERROR("Parse ability parameter failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
            HILOG_ERROR("Parse ability callback parameters failed");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseStartAbilityPara(
    napi_env env, NapiCallbackInfo& info, AAFwk::Want &want)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        HILOG_ERROR("Parse want parameter failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
            HILOG_ERROR("Parse StartAbility callback parameters failed");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseFinishTestPara(
    napi_env env, NapiCallbackInfo& info, std::string &msg, int64_t &code)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], msg)) {
        HILOG_ERROR("Parse msg parameter failed");
        return nullptr;
    }

    if (!ConvertFromJsValue(env, info.argv[INDEX_ONE], code)) {
        HILOG_ERROR("Parse code para parameter failed");
        return nullptr;
    }

    if (info.argc > ARGC_TWO) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_TWO], napi_function)) {
            HILOG_ERROR("Incorrect Callback Function type");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

bool JSAbilityDelegator::ParseMockListPara(
    napi_env env, NapiCallbackInfo& info, std::map<std::string, std::string> &mockList)
{
    HILOG_DEBUG("enter");
    if (info.argc != ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return false;
    }

    napi_value value = info.argv[INDEX_ZERO];
    if (value == nullptr) {
        HILOG_ERROR("The arg[0] is nullptr.");
        return false;
    }

    if (!CheckTypeForNapiValue(env, value, napi_object)) {
        HILOG_ERROR("The type of arg[0] is not napi_object.");
        return false;
    }

    std::vector<std::string> propNames;
    napi_value array = nullptr;
    napi_get_property_names(env, value, &array);
    if (!ParseArrayStringValue(env, array, propNames)) {
        HILOG_ERROR("Failed to property names.");
        return false;
    }

    for (const auto &propName : propNames) {
        napi_value prop = nullptr;
        napi_get_named_property(env, value, propName.c_str(), &prop);
        if (prop == nullptr) {
            HILOG_WARN("Prop is null: %{public}s", propName.c_str());
            continue;
        }
        if (!CheckTypeForNapiValue(env, prop, napi_string)) {
            HILOG_WARN("Prop is not string: %{public}s", propName.c_str());
            continue;
        }
        std::string valName;
        if (!ConvertFromJsValue(env, prop, valName)) {
            HILOG_WARN("Failed to ConvertFromJsValue: %{public}s", propName.c_str());
            continue;
        }
        HILOG_DEBUG("add mock list: key: %{public}s, value: %{public}s", propName.c_str(), valName.c_str());
        mockList.emplace(propName, valName);
    }
    return true;
}

bool JSAbilityDelegator::ParseArrayStringValue(
    napi_env env, napi_value array, std::vector<std::string> &vector)
{
    if (array == nullptr) {
        HILOG_ERROR("array is nullptr!");
        return false;
    }
    bool isArray = false;
    if (napi_is_array(env, array, &isArray) != napi_ok || isArray == false) {
        HILOG_ERROR("not array!");
        return false;
    }

    uint32_t arrayLen = 0;
    napi_get_array_length(env, array, &arrayLen);
    if (arrayLen == 0) {
        return true;
    }
    vector.reserve(arrayLen);
    for (uint32_t i = 0; i < arrayLen; i++) {
        std::string strItem;
        napi_value jsValue = nullptr;
        napi_get_element(env, array, i, &jsValue);
        if (!ConvertFromJsValue(env, jsValue, strItem)) {
            HILOG_WARN("Failed to ConvertFromJsValue, index: %{public}u", i);
            continue;
        }
        vector.emplace_back(std::move(strItem));
    }
    return true;
}

void JSAbilityDelegator::AddStageMonitorRecord(
    napi_env env, napi_value value, const std::shared_ptr<AbilityStageMonitor> &monitor)
{
    if (!value) {
        HILOG_ERROR("UpdateStageMonitorRecord value is empty");
        return;
    }
    if (!AbilityDelegatorRegistry::GetAbilityDelegator()) {
        HILOG_ERROR("AbilityDelegator is null");
        return;
    }
    std::shared_ptr<NativeReference> reference = nullptr;
    napi_ref ref = nullptr;
    napi_create_reference(env, value, 1, &ref);
    reference.reset(reinterpret_cast<NativeReference*>(ref));
    {
        std::unique_lock<std::mutex> lck(g_mtxStageMonitorRecord);
        g_stageMonitorRecord.emplace(reference, monitor);
    }
    HILOG_INFO("g_stageMonitorRecord added, size = %{public}zu", g_stageMonitorRecord.size());
}

void JSAbilityDelegator::RemoveStageMonitorRecord(napi_env env, napi_value value)
{
    if (!value) {
        HILOG_ERROR("UpdateStageMonitorRecord value is empty");
        return;
    }
    if (!AbilityDelegatorRegistry::GetAbilityDelegator()) {
        HILOG_ERROR("AbilityDelegator is null");
        return;
    }
    std::unique_lock<std::mutex> lck(g_mtxStageMonitorRecord);
    for (auto iter = g_stageMonitorRecord.begin(); iter != g_stageMonitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        bool isEquals = false;
        napi_strict_equals(env, value, jsMonitor->GetNapiValue(), &isEquals);
        if (isEquals) {
            g_stageMonitorRecord.erase(iter);
            HILOG_INFO("g_stageMonitorRecord removed, size = %{public}zu", g_stageMonitorRecord.size());
            break;
        }
    }
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
