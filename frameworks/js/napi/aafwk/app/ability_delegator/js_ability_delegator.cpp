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
#include "ability_runtime/js_ability.h"
#include "hilog_wrapper.h"
#include "js_ability_delegator_utils.h"
#include "js_context_utils.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "shell_cmd_result.h"

namespace OHOS {
namespace AbilityDelegatorJs {
struct AbilityObjectBox {
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
constexpr int ERROR = -1;

using namespace OHOS::AbilityRuntime;
std::map<std::shared_ptr<NativeReference>, std::shared_ptr<AbilityMonitor>> monitorRecord_;
std::map<std::weak_ptr<NativeReference>, sptr<IRemoteObject>, std::owner_less<>> ablityRecord_;
std::mutex mutexAblityRecord_;

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

            std::unique_lock<std::mutex> lck(mutexAblityRecord_);
            for (auto it = ablityRecord_.begin(); it != ablityRecord_.end();) {
                if (it->second == property->token_) {
                    it = ablityRecord_.erase(it);
                    continue;
                }
                ++it;
            }
        };

        delegator->RegisterClearFunc(clearFunc);
    }
}

void JSAbilityDelegator::Finalizer(NativeEngine *engine, void *data, void *hint)
{
    HILOG_INFO("enter");
    std::unique_ptr<JSAbilityDelegator>(static_cast<JSAbilityDelegator *>(data));
}

NativeValue *JSAbilityDelegator::AddAbilityMonitor(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnAddAbilityMonitor(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::RemoveAbilityMonitor(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnRemoveAbilityMonitor(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::WaitAbilityMonitor(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnWaitAbilityMonitor(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::GetAppContext(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnGetAppContext(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::GetAbilityState(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnGetAbilityState(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::GetCurrentTopAbility(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnGetCurrentTopAbility(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::StartAbility(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnStartAbility(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::DoAbilityForeground(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnDoAbilityForeground(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::DoAbilityBackground(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnDoAbilityBackground(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::Print(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnPrint(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::PrintSync(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnPrintSync(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::ExecuteShellCommand(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnExecuteShellCommand(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::FinishTest(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnFinishTest(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::OnAddAbilityMonitor(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseAbilityMonitorPara(engine, info, monitor)) {
        HILOG_ERROR("Parse addAbilityMonitor parameters failed");
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [monitor](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnAddAbilityMonitor AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(engine, CreateJsError(engine, ERROR, "addAbilityMonitor failed."));
            return;
        }
        delegator->AddAbilityMonitor(monitor);
        task.Resolve(engine, engine.CreateNull());
    };

    NativeValue *lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnRemoveAbilityMonitor(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseAbilityMonitorPara(engine, info, monitor)) {
        HILOG_ERROR("Parse removeAbilityMonitor parameters failed");
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [monitor](NativeEngine &engine, AsyncTask &task, int32_t status) mutable {
        HILOG_INFO("OnRemoveAbilityMonitor AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(engine, CreateJsError(engine, ERROR, "removeAbilityMonitor failed."));
            return;
        }
        delegator->RemoveAbilityMonitor(monitor);
        task.Resolve(engine, engine.CreateNull());
    };

    NativeValue *lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));

    if (AbilityDelegatorRegistry::GetAbilityDelegator()) {
        for (auto iter = monitorRecord_.begin(); iter != monitorRecord_.end(); ++iter) {
            std::shared_ptr<NativeReference> jsMonitor = iter->first;
            if ((info.argv[INDEX_ZERO])->StrictEquals(jsMonitor->Get())) {
                monitorRecord_.erase(iter);
                break;
            }
        }
    }
    return result;
}

NativeValue *JSAbilityDelegator::OnWaitAbilityMonitor(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    TimeoutCallback opt {false, false};
    int64_t timeout = 0;
    if (!ParseWaitAbilityMonitorPara(engine, info, monitor, opt, timeout)) {
        HILOG_ERROR("Parse waitAbilityMonitor parameters failed");
        return engine.CreateUndefined();
    }

    auto abilityObjectBox = std::make_shared<AbilityObjectBox>();
    AsyncTask::ExecuteCallback execute = [monitor, timeout, opt, abilityObjectBox]() {
        HILOG_INFO("OnWaitAbilityMonitor AsyncTask ExecuteCallback is called");
        if (!abilityObjectBox) {
            HILOG_ERROR("OnWaitAbilityMonitor AsyncTask ExecuteCallback, Invalid abilityObjectBox");
            return;
        }

        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            HILOG_ERROR("OnWaitAbilityMonitor AsyncTask ExecuteCallback, Invalid delegator");
            return;
        }

        std::shared_ptr<ADelegatorAbilityProperty> property;
        if (opt.hasTimeoutPara) {
            property = delegator->WaitAbilityMonitor(monitor, timeout);
        } else {
            property = delegator->WaitAbilityMonitor(monitor);
        }

        if (!property || property->object_.expired()) {
            HILOG_ERROR("Invalid property");
            return;
        }

        abilityObjectBox->object_ = property->object_;

        std::unique_lock<std::mutex> lck(mutexAblityRecord_);
        ablityRecord_.emplace(property->object_, property->token_);
    };

    AsyncTask::CompleteCallback complete = [abilityObjectBox](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnWaitAbilityMonitor AsyncTask CompleteCallback is called");
        if (abilityObjectBox && !abilityObjectBox->object_.expired()) {
            task.Resolve(engine, abilityObjectBox->object_.lock()->Get());
        } else {
            task.Reject(engine, CreateJsError(engine, ERROR, "waitAbilityMonitor failed."));
        }
    };

    NativeValue *lastParam = nullptr;
    if (opt.hasCallbackPara) {
        lastParam = opt.hasTimeoutPara ? info.argv[INDEX_TWO] : info.argv[INDEX_ONE];
    }

    NativeValue *result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnPrint(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string msg;
    if (!ParsePrintPara(engine, info, msg)) {
        HILOG_ERROR("Parse print parameters failed");
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [msg](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnPrint AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(engine, CreateJsError(engine, ERROR, "print failed."));
            return;
        }
        delegator->Print(msg);
        task.Resolve(engine, engine.CreateNull());
    };

    NativeValue *lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnPrintSync(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string msg;
    if (!ParsePrintPara(engine, info, msg)) {
        HILOG_ERROR("Parse print parameters failed");
        return engine.CreateNull();
    }

    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("Invalid delegator");
        return engine.CreateNull();
    }

    delegator->Print(msg);
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::OnExecuteShellCommand(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string cmd;
    TimeoutCallback opt {false, false};
    int64_t timeoutSecs = 0;
    if (!ParseExecuteShellCommandPara(engine, info, cmd, opt, timeoutSecs)) {
        HILOG_ERROR("Parse executeShellCommand parameters failed");
        return engine.CreateUndefined();
    }

    auto shellCmdResultBox = std::make_shared<ShellCmdResultBox>();
    AsyncTask::ExecuteCallback execute = [cmd, timeoutSecs, shellCmdResultBox]() {
        HILOG_INFO("OnExecuteShellCommand AsyncTask ExecuteCallback is called");
        if (!shellCmdResultBox) {
            HILOG_ERROR("OnExecuteShellCommand AsyncTask ExecuteCallback, Invalid shellCmdResultBox");
            return;
        }

        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            HILOG_ERROR("OnExecuteShellCommand AsyncTask ExecuteCallback, Invalid delegator");
            return;
        }

        shellCmdResultBox->shellCmdResult_ = delegator->ExecuteShellCommand(cmd, timeoutSecs);
    };

    AsyncTask::CompleteCallback complete = [shellCmdResultBox](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnExecuteShellCommand AsyncTask CompleteCallback is called");
        if (!shellCmdResultBox) {
            HILOG_ERROR("OnExecuteShellCommand AsyncTask CompleteCallback, Invalid shellCmdResultBox");
            task.Reject(engine, CreateJsError(engine, ERROR, "executeShellCommand failed."));
            return;
        }

        NativeValue *result = CreateJsShellCmdResult(engine, shellCmdResultBox->shellCmdResult_);
        if (result) {
            task.Resolve(engine, result);
        } else {
            task.Reject(engine, CreateJsError(engine, ERROR, "executeShellCommand failed."));
        }
    };

    NativeValue *lastParam = nullptr;
    if (opt.hasCallbackPara) {
        lastParam = opt.hasTimeoutPara ? info.argv[INDEX_TWO] : info.argv[INDEX_ONE];
    }

    NativeValue *result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnGetAppContext(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null");
        return engine.CreateNull();
    }
    std::shared_ptr<AbilityRuntime::Context> context = delegator->GetAppContext();
    if (!context) {
        HILOG_ERROR("context is null");
        return engine.CreateNull();
    }
    return CreateJsBaseContext(engine, context, false);
}

NativeValue *JSAbilityDelegator::OnGetAbilityState(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return engine.CreateUndefined();
    }

    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    if (!ParseAbilityPara(engine, info.argv[INDEX_ZERO], remoteObject)) {
        HILOG_ERROR("Parse ability parameter failed");
        return engine.CreateUndefined();
    }

    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null");
        return engine.CreateNull();
    }
    AbilityDelegator::AbilityState lifeState = delegator->GetAbilityState(remoteObject);
    AbilityLifecycleState abilityLifeState = AbilityLifecycleState::UNINITIALIZED;
    AbilityLifecycleStateToJs(lifeState, abilityLifeState);
    return engine.CreateNumber(static_cast<int>(abilityLifeState));
}

NativeValue *JSAbilityDelegator::OnGetCurrentTopAbility(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    if (info.argc >= ARGC_ONE) {
        if (info.argv[INDEX_ZERO]->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
            HILOG_ERROR("Parse getCurrentTopAbility parameter failed");
            return engine.CreateUndefined();
        }
    }

    AsyncTask::CompleteCallback complete = [this](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnGetCurrentTopAbility AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            HILOG_ERROR("Invalid delegator");
            task.Reject(engine, CreateJsError(engine, ERROR, "getCurrentTopAbility failed."));
            return;
        }

        auto property = delegator->GetCurrentTopAbility();
        if (!property || property->object_.expired()) {
            HILOG_ERROR("Invalid property");
            task.Reject(engine, CreateJsError(engine, ERROR, "getCurrentTopAbility failed."));
        } else {
            {
                std::unique_lock<std::mutex> lck(mutexAblityRecord_);
                ablityRecord_.emplace(property->object_, property->token_);
            }
            task.Resolve(engine, property->object_.lock()->Get());
        }
    };

    NativeValue *lastParam = (info.argc >= ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnStartAbility(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    AAFwk::Want want;
    if (!ParseStartAbilityPara(engine, info, want)) {
        HILOG_ERROR("Parse startAbility parameters failed");
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [want](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnStartAbility AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(engine, CreateJsError(engine, ERROR, "startAbility failed."));
            return;
        }
        int result = delegator->StartAbility(want);
        if (result) {
            task.Reject(engine, CreateJsError(engine, result, "startAbility failed."));
        } else {
            task.Resolve(engine, engine.CreateNull());
        }
    };

    NativeValue *lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnDoAbilityForeground(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    if (!ParseAbilityCommonPara(engine, info, remoteObject)) {
        HILOG_ERROR("Parse doAbilityForeground parameters failed");
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [remoteObject](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnDoAbilityForeground AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(engine, CreateJsError(engine, ERROR, "doAbilityForeground failed."));
            return;
        }
        bool ret = delegator->DoAbilityForeground(remoteObject);
        task.Resolve(engine, engine.CreateBoolean(ret));
    };

    NativeValue *lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnDoAbilityBackground(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    if (!ParseAbilityCommonPara(engine, info, remoteObject)) {
        HILOG_ERROR("Parse doAbilityBackground parameters failed");
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [remoteObject](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnDoAbilityBackground AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(engine, CreateJsError(engine, ERROR, "doAbilityBackground failed."));
            return;
        }
        bool ret = delegator->DoAbilityBackground(remoteObject);
        task.Resolve(engine, engine.CreateBoolean(ret));
    };

    NativeValue *lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnFinishTest(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string msg;
    int64_t code = 0;
    if (!ParseFinishTestPara(engine, info, msg, code)) {
        HILOG_ERROR("Parse finishTest parameters failed");
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [msg, code](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnFinishTest AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(engine, CreateJsError(engine, ERROR, "finishTest failed."));
            return;
        }
        delegator->FinishUserTest(msg, code);
        task.Resolve(engine, engine.CreateNull());
    };
    NativeValue *lastParam = (info.argc > ARGC_TWO) ? info.argv[INDEX_TWO] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::ParseMonitorPara(
    NativeEngine &engine, NativeValue *value, std::shared_ptr<AbilityMonitor> &monitor)
{
    HILOG_INFO("enter, monitorRecord size = %{public}zu", monitorRecord_.size());

    for (auto iter = monitorRecord_.begin(); iter != monitorRecord_.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        if (value->StrictEquals(jsMonitor->Get())) {
            HILOG_ERROR("monitor existed");
            monitor = iter->second;
            return monitor ? engine.CreateNull() : nullptr;
        }
    }

    NativeObject *object = ConvertNativeValueTo<NativeObject>(value);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    auto abilityNameValue = object->GetProperty("abilityName");
    if (abilityNameValue == nullptr) {
        HILOG_ERROR("Failed to get property abilityName");
        return nullptr;
    }
    std::string abilityName;
    if (!ConvertFromJsValue(engine, abilityNameValue, abilityName)) {
        return nullptr;
    }
    std::shared_ptr<JSAbilityMonitor> abilityMonitor = std::make_shared<JSAbilityMonitor>(abilityName);
    abilityMonitor->SetJsAbilityMonitorEnv(&engine);
    abilityMonitor->SetJsAbilityMonitor(value);

    monitor = std::make_shared<AbilityMonitor>(abilityName, abilityMonitor);
    if (!monitor) {
        HILOG_ERROR("Failed to create monitor");
        return nullptr;
    }

    std::shared_ptr<NativeReference> reference = nullptr;
    reference.reset(engine.CreateReference(value, 1));
    monitorRecord_.emplace(reference, monitor);

    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseAbilityPara(
    NativeEngine &engine, NativeValue *value, sptr<OHOS::IRemoteObject> &remoteObject)
{
    HILOG_INFO("enter");

    std::unique_lock<std::mutex> lck(mutexAblityRecord_);
    for (auto iter = ablityRecord_.begin(); iter != ablityRecord_.end();) {
        if (iter->first.expired()) {
            iter = ablityRecord_.erase(iter);
            continue;
        }

        if (value->StrictEquals(iter->first.lock()->Get())) {
            remoteObject = iter->second;
            HILOG_INFO("Ablity exist");
            return remoteObject ? engine.CreateNull() : nullptr;
        }

        ++iter;
    }

    HILOG_ERROR("Ablity doesn't exist");
    remoteObject = nullptr;
    return nullptr;
}

NativeValue *JSAbilityDelegator::CreateAbilityObject(NativeEngine &engine, const sptr<IRemoteObject> &remoteObject)
{
    HILOG_INFO("enter");

    if (!remoteObject) {
        return nullptr;
    }

    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    std::shared_ptr<NativeReference> refence = nullptr;
    refence.reset(engine.CreateReference(objValue, 1));

    std::unique_lock<std::mutex> lck(mutexAblityRecord_);
    ablityRecord_[refence] = remoteObject;
    return objValue;
}

void JSAbilityDelegator::AbilityLifecycleStateToJs(
    const AbilityDelegator::AbilityState &lifeState, AbilityLifecycleState &abilityLifeState)
{
    HILOG_INFO("enter and lifeState = %{public}d", (int32_t)lifeState);
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

NativeValue *JSAbilityDelegator::ParseAbilityMonitorPara(
    NativeEngine &engine, NativeCallbackInfo &info, std::shared_ptr<AbilityMonitor> &monitor)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ParseMonitorPara(engine, info.argv[INDEX_ZERO], monitor)) {
        HILOG_ERROR("Parse monitor parameters failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (info.argv[INDEX_ONE]->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
            HILOG_ERROR("Parse callback parameters failed");
            return nullptr;
        }
    }
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseWaitAbilityMonitorPara(NativeEngine &engine, NativeCallbackInfo &info,
    std::shared_ptr<AbilityMonitor> &monitor, TimeoutCallback &opt, int64_t &timeout)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ParseMonitorPara(engine, info.argv[INDEX_ZERO], monitor)) {
        HILOG_ERROR("Monitor parse parameters failed");
        return nullptr;
    }

    if (!ParseTimeoutCallbackPara(engine, info, opt, timeout)) {
        HILOG_ERROR("TimeoutCallback parse parameters failed");
        return nullptr;
    }
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseTimeoutCallbackPara(
    NativeEngine &engine, NativeCallbackInfo &info, TimeoutCallback &opt, int64_t &timeout)
{
    HILOG_INFO("enter");

    opt.hasCallbackPara = false;
    opt.hasTimeoutPara = false;

    if (info.argc >= ARGC_TWO) {
        if (!ConvertFromJsValue(engine, info.argv[INDEX_ONE], timeout)) {
            if (info.argv[INDEX_ONE] == nullptr || info.argv[INDEX_ONE]->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
                HILOG_ERROR("Parse parameter argv[1] failed");
                return nullptr;
            }
            opt.hasCallbackPara = true;
            return engine.CreateNull();
        }
        opt.hasTimeoutPara = true;

        if (info.argc > ARGC_TWO) {
            if (info.argv[INDEX_TWO]->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
                HILOG_ERROR("Parse parameter argv[2] failed");
                return nullptr;
            }
            opt.hasCallbackPara = true;
        }
    }
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParsePrintPara(NativeEngine &engine, NativeCallbackInfo &info, std::string &msg)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], msg)) {
        HILOG_ERROR("Parse msg parameter failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (info.argv[INDEX_ONE]->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
            HILOG_ERROR("Parse callback parameter failed");
            return nullptr;
        }
    }
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseExecuteShellCommandPara(
    NativeEngine &engine, NativeCallbackInfo &info, std::string &cmd, TimeoutCallback &opt, int64_t &timeout)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], cmd)) {
        HILOG_ERROR("Parse cmd parameter failed");
        return nullptr;
    }
    if (!ParseTimeoutCallbackPara(engine, info, opt, timeout)) {
        HILOG_ERROR("Parse timeOut callback parameters failed");
        return nullptr;
    }
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseAbilityCommonPara(
    NativeEngine &engine, NativeCallbackInfo &info, sptr<OHOS::IRemoteObject> &remoteObject)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ParseAbilityPara(engine, info.argv[INDEX_ZERO], remoteObject)) {
        HILOG_ERROR("Parse ability parameter failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (info.argv[INDEX_ONE]->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
            HILOG_ERROR("Parse ability callback parameters failed");
            return nullptr;
        }
    }
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseStartAbilityPara(
    NativeEngine &engine, NativeCallbackInfo &info, AAFwk::Want &want)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!OHOS::AppExecFwk::UnwrapWant(
        reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), want)) {
        HILOG_ERROR("Parse want parameter failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (info.argv[INDEX_ONE]->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
            HILOG_ERROR("Parse StartAbility callback parameters failed");
            return nullptr;
        }
    }
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseFinishTestPara(
    NativeEngine &engine, NativeCallbackInfo &info, std::string &msg, int64_t &code)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], msg)) {
        HILOG_ERROR("Parse msg parameter failed");
        return nullptr;
    }

    if (!ConvertFromJsValue(engine, info.argv[INDEX_ONE], code)) {
        HILOG_ERROR("Parse code para parameter failed");
        return nullptr;
    }

    if (info.argc > ARGC_TWO) {
        if ((info.argv[INDEX_TWO])->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
            HILOG_ERROR("Incorrect Callback Function type");
            return nullptr;
        }
    }
    return engine.CreateNull();
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
