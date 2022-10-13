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

#include "js_service_extension_context.h"

#include <cstdint>

#include "ability_runtime/js_caller_complex.h"
#include "hilog_wrapper.h"
#include "js_extension_context.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_ability.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "napi_remote_object.h"
#include "napi_common_start_options.h"
#include "start_options.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr int32_t ERROR_CODE_TWO = 2;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t ARGC_FOUR = 4;
constexpr int32_t ERR_NOT_OK = -1;

class StartAbilityByCallParameters {
public:
    int err = 0;
    sptr<IRemoteObject> remoteCallee = nullptr;
    std::shared_ptr<CallerCallBack> callerCallBack = nullptr;
    std::mutex mutexlock;
    std::condition_variable condition;
};

class JsServiceExtensionContext final {
public:
    explicit JsServiceExtensionContext(const std::shared_ptr<ServiceExtensionContext>& context) : context_(context) {}
    ~JsServiceExtensionContext() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsAbilityContext::Finalizer is called");
        std::unique_ptr<JsServiceExtensionContext>(static_cast<JsServiceExtensionContext*>(data));
    }

    static NativeValue* StartAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsServiceExtensionContext* me = CheckParamsAndGetThis<JsServiceExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnStartAbility(*engine, *info) : nullptr;
    }

    static NativeValue* StartAbilityByCall(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsServiceExtensionContext* me = CheckParamsAndGetThis<JsServiceExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnStartAbilityByCall(*engine, *info) : nullptr;
    }

    static NativeValue* StartAbilityWithAccount(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsServiceExtensionContext* me = CheckParamsAndGetThis<JsServiceExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnStartAbilityWithAccount(*engine, *info) : nullptr;
    }

    static NativeValue* ConnectAbilityWithAccount(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsServiceExtensionContext* me = CheckParamsAndGetThis<JsServiceExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnConnectAbilityWithAccount(*engine, *info) : nullptr;
    }

    static NativeValue* TerminateAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsServiceExtensionContext* me = CheckParamsAndGetThis<JsServiceExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnTerminateAbility(*engine, *info) : nullptr;
    }

    static NativeValue* ConnectAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsServiceExtensionContext* me = CheckParamsAndGetThis<JsServiceExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnConnectAbility(*engine, *info) : nullptr;
    }

    static NativeValue* DisconnectAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsServiceExtensionContext* me = CheckParamsAndGetThis<JsServiceExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnDisconnectAbility(*engine, *info) : nullptr;
    }

    static NativeValue* StartServiceExtensionAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsServiceExtensionContext* me = CheckParamsAndGetThis<JsServiceExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnStartExtensionAbility(*engine, *info) : nullptr;
    }

    static NativeValue* StartServiceExtensionAbilityWithAccount(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsServiceExtensionContext* me = CheckParamsAndGetThis<JsServiceExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnStartExtensionAbilityWithAccount(*engine, *info) : nullptr;
    }

    static NativeValue* StopServiceExtensionAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsServiceExtensionContext* me = CheckParamsAndGetThis<JsServiceExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnStopExtensionAbility(*engine, *info) : nullptr;
    }

    static NativeValue* StopServiceExtensionAbilityWithAccount(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsServiceExtensionContext* me = CheckParamsAndGetThis<JsServiceExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnStopExtensionAbilityWithAccount(*engine, *info) : nullptr;
    }

private:
    std::weak_ptr<ServiceExtensionContext> context_;
    NativeValue* OnStartAbility(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
        HILOG_INFO("OnStartAbility is called");
        // only support one or two or three params
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO && info.argc != ARGC_THREE) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }

        decltype(info.argc) unwrapArgc = 0;
        AAFwk::Want want;
        OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), want);
        HILOG_INFO("Start ability begin, bundle:%{public}s, ability:%{public}s",
            want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        unwrapArgc++;

        AAFwk::StartOptions startOptions;
        if (info.argc > ARGC_ONE && info.argv[INDEX_ONE]->TypeOf() == NATIVE_OBJECT) {
            HILOG_INFO("OnStartAbility start options is used.");
            AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[INDEX_ONE]), startOptions);
            unwrapArgc++;
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_, want, startOptions, unwrapArgc](NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("startAbility begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "Context is released"));
                    return;
                }

                ErrCode innerErrorCode = ERR_OK;
                (unwrapArgc == 1) ? innerErrorCode = context->StartAbility(want) :
                    innerErrorCode = context->StartAbility(want, startOptions);
                ErrCode errcode = AppExecFwk::GetStartAbilityErrorCode(innerErrorCode);
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Start Ability failed."));
                }
            };

        NativeValue* lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionContext::OnStartAbility",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnStartAbilityByCall(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnStartAbilityByCall is called.");
        constexpr size_t ARGC_ONE = 1;
        constexpr size_t ARGC_TWO = 2;
        if (info.argc < ARGC_ONE || info.argv[0]->TypeOf() != NATIVE_OBJECT) {
            HILOG_ERROR("int put params count error");
            return engine.CreateUndefined();
        }

        AAFwk::Want want;
        OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[0]), want);

        std::shared_ptr<StartAbilityByCallParameters> calls = std::make_shared<StartAbilityByCallParameters>();
        if (calls == nullptr) {
            HILOG_ERROR("calls create error");
            return engine.CreateUndefined();
        }

        NativeValue* lastParam = ((info.argc == ARGC_TWO) ? info.argv[ARGC_ONE] : nullptr);
        NativeValue* retsult = nullptr;

        calls->callerCallBack = std::make_shared<CallerCallBack>();
        calls->callerCallBack->SetCallBack(GetCallBackDone(calls));
        calls->callerCallBack->SetOnRelease(GetReleaseListen());

        auto context = context_.lock();
        if (context == nullptr) {
            HILOG_ERROR("OnStartAbilityByCall context is nullptr");
            return engine.CreateUndefined();
        }

        if (context->StartAbilityByCall(want, calls->callerCallBack) != 0) {
            HILOG_ERROR("OnStartAbilityByCall StartAbility is failed");
            return engine.CreateUndefined();
        }

        if (calls->remoteCallee == nullptr) {
            HILOG_INFO("OnStartAbilityByCall async wait execute");
            AsyncTask::Schedule("JsAbilityContext::OnStartAbilityByCall", engine,
                CreateAsyncTaskWithLastParam(
                    engine, lastParam, GetCallExecute(calls), GetCallComplete(calls), &retsult));
        } else {
            HILOG_INFO("OnStartAbilityByCall promiss return result execute");
            AsyncTask::Schedule("JSServiceExtensionContext::OnStartAbilityByCall", engine,
                CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, GetCallComplete(calls), &retsult));
        }
        return retsult;
    }

    AsyncTask::CompleteCallback GetCallComplete(std::shared_ptr<StartAbilityByCallParameters> calls)
    {
        auto callComplete = [weak = context_, calldata = calls] (
            NativeEngine& engine, AsyncTask& task, int32_t) {
            if (calldata->err != 0) {
                HILOG_ERROR("OnStartAbilityByCall callComplete err is %{public}d", calldata->err);
                task.Reject(engine, CreateJsError(engine, calldata->err, "callComplete err."));
                return;
            }

            auto context = weak.lock();
            if (context != nullptr && calldata->callerCallBack != nullptr && calldata->remoteCallee != nullptr) {
                auto releaseCallFunc = [weak] (
                    const std::shared_ptr<CallerCallBack> &callback) -> ErrCode {
                    auto contextForRelease = weak.lock();
                    if (contextForRelease == nullptr) {
                        HILOG_ERROR("releaseCallFunction, context is nullptr");
                        return -1;
                    }
                    return contextForRelease->ReleaseCall(callback);
                };
                task.Resolve(engine,
                    CreateJsCallerComplex(
                        engine, releaseCallFunc, calldata->remoteCallee, calldata->callerCallBack));
            } else {
                HILOG_ERROR("OnStartAbilityByCall callComplete params error %{public}s is nullptr",
                    context == nullptr ? "context" :
                        (calldata->remoteCallee == nullptr ? "remoteCallee" : "callerCallBack"));
                task.Reject(engine, CreateJsError(engine, -1, "Create Call Failed."));
            }

            HILOG_DEBUG("OnStartAbilityByCall callComplete end");
        };
        return callComplete;
    }

    AsyncTask::ExecuteCallback GetCallExecute(std::shared_ptr<StartAbilityByCallParameters> calls)
    {
        auto callExecute = [calldata = calls] () {
            constexpr int callerTimeOut = 10; // 10s
            std::unique_lock<std::mutex> lock(calldata->mutexlock);
            if (calldata->remoteCallee != nullptr) {
                HILOG_INFO("OnStartAbilityByCall callExecute callee isn`t nullptr");
                return;
            }

            if (calldata->condition.wait_for(lock, std::chrono::seconds(callerTimeOut)) == std::cv_status::timeout) {
                HILOG_ERROR("OnStartAbilityByCall callExecute waiting callee timeout");
                calldata->err = -1;
            }
            HILOG_DEBUG("OnStartAbilityByCall callExecute end");
        };
        return callExecute;
    }

    CallerCallBack::CallBackClosure GetCallBackDone(std::shared_ptr<StartAbilityByCallParameters> calls)
    {
        auto callBackDone = [calldata = calls] (const sptr<IRemoteObject> &obj) {
            HILOG_DEBUG("OnStartAbilityByCall callBackDone mutexlock");
            std::unique_lock<std::mutex> lock(calldata->mutexlock);
            HILOG_DEBUG("OnStartAbilityByCall callBackDone remoteCallee assignment");
            calldata->remoteCallee = obj;
            calldata->condition.notify_all();
            HILOG_INFO("OnStartAbilityByCall callBackDone is called end");
        };
        return callBackDone;
    }

    CallerCallBack::OnReleaseClosure GetReleaseListen()
    {
        auto releaseListen = [](const std::string &str) {
            HILOG_INFO("OnStartAbilityByCall releaseListen is called %{public}s", str.c_str());
        };
        return releaseListen;
    }

    NativeValue* OnStartAbilityWithAccount(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnStartAbilityWithAccount is called");
        // only support two or three or four params
        if (info.argc != ARGC_TWO && info.argc != ARGC_THREE && info.argc != ARGC_FOUR) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }

        decltype(info.argc) unwrapArgc = 0;
        AAFwk::Want want;
        OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), want);
        HILOG_INFO("%{public}s bundlename:%{public}s abilityname:%{public}s",
            __func__,
            want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        unwrapArgc++;

        int32_t accountId = 0;
        if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_ONE]), accountId)) {
            HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
            return engine.CreateUndefined();
        }
        HILOG_INFO("%{public}d accountId:", accountId);
        unwrapArgc++;

        AAFwk::StartOptions startOptions;
        if (info.argc > ARGC_TWO && info.argv[INDEX_TWO]->TypeOf() == NATIVE_OBJECT) {
            HILOG_INFO("OnStartAbilityWithAccount start options is used.");
            AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[INDEX_TWO]), startOptions);
            unwrapArgc++;
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_, want, accountId, startOptions, unwrapArgc](
                NativeEngine& engine, AsyncTask& task, int32_t status) {
                    HILOG_INFO("startAbility begin");
                    auto context = weak.lock();
                    if (!context) {
                        HILOG_WARN("context is released");
                        task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "Context is released"));
                        return;
                    }

                    ErrCode innerErrorCode = ERR_OK;
                    (unwrapArgc == ARGC_TWO) ? innerErrorCode = context->StartAbilityWithAccount(want, accountId) :
                        innerErrorCode = context->StartAbilityWithAccount(want, accountId, startOptions);
                    ErrCode errcode = AppExecFwk::GetStartAbilityErrorCode(innerErrorCode);
                    if (errcode == 0) {
                        task.Resolve(engine, engine.CreateUndefined());
                    } else {
                        task.Reject(engine, CreateJsError(engine, errcode, "Start Ability failed."));
                    }
                };

        NativeValue* lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionContext::OnStartAbilityWithAccount",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnTerminateAbility(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnTerminateAbility is called");
        // only support one or zero params
        if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
            HILOG_ERROR("Not enough params");
            Throw(engine, ERROR_CODE_PARAM_INVALID, ERROR_MSG_PARAM_INVALID);
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_](NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("TerminateAbility begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_CONTEXT_EMPTY, "Context is released"));
                    return;
                }

                auto errcode = context->TerminateAbility();
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, GetErrorCode(errcode), "Terminate Ability failed."));
                }
            };

        NativeValue* lastParam = (info.argc == ARGC_ZERO) ? nullptr : info.argv[INDEX_ZERO];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionContext::OnTerminateAbility",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnConnectAbility(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnConnectAbility is called");
        // only support two params
        if (info.argc != ARGC_TWO) {
            HILOG_ERROR("Not enough params");
            Throw(engine, ERROR_CODE_PARAM_INVALID, ERROR_MSG_PARAM_INVALID);
            return engine.CreateUndefined();
        }

        // unwrap want
        AAFwk::Want want;
        OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), want);
        HILOG_INFO("%{public}s bundlename:%{public}s abilityname:%{public}s",
            __func__,
            want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        // unwarp connection
        sptr<JSServiceExtensionConnection> connection = new JSServiceExtensionConnection(engine);
        connection->SetJsConnectionObject(info.argv[1]);
        int64_t connectId = serialNumber_;
        ConnecttionKey key;
        key.id = serialNumber_;
        key.want = want;
        connection->SetConnectionId(key.id);
        connects_.emplace(key, connection);
        if (serialNumber_ < INT32_MAX) {
            serialNumber_++;
        } else {
            serialNumber_ = 0;
        }
        HILOG_INFO("%{public}s not find connection, make new one", __func__);
        AsyncTask::CompleteCallback complete =
            [weak = context_, want, connection, connectId](NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("OnConnectAbility begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_CONTEXT_EMPTY, "Context is released"));
                    return;
                }
                HILOG_INFO("context->ConnectAbility connection:%{public}d", (int32_t)connectId);
                if (!context->ConnectAbility(want, connection)) {
                    connection->CallJsFailed(ERROR_CODE_CONNECTION_INVALID_STATE);
                }
                task.Resolve(engine, engine.CreateUndefined());
            };
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionConnection::OnConnectAbility",
            engine, CreateAsyncTaskWithLastParam(engine, nullptr, nullptr, std::move(complete), &result));
        return engine.CreateNumber(connectId);
    }

    NativeValue* OnConnectAbilityWithAccount(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnConnectAbilityWithAccount is called");
        // only support three params
        if (info.argc != ARGC_THREE) {
            HILOG_ERROR("Not enough params");
            Throw(engine, ERROR_CODE_PARAM_INVALID, ERROR_MSG_PARAM_INVALID);
            return engine.CreateUndefined();
        }

        // unwrap want
        AAFwk::Want want;
        OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), want);
        HILOG_INFO("%{public}s bundlename:%{public}s abilityname:%{public}s",
            __func__,
            want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());

        int32_t accountId = 0;
        if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_ONE]), accountId)) {
            HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
            return engine.CreateUndefined();
        }

        // unwarp connection
        sptr<JSServiceExtensionConnection> connection = new JSServiceExtensionConnection(engine);
        connection->SetJsConnectionObject(info.argv[1]);
        int64_t connectId = serialNumber_;
        ConnecttionKey key;
        key.id = serialNumber_;
        key.want = want;
        connection->SetConnectionId(key.id);
        connects_.emplace(key, connection);
        if (serialNumber_ < INT32_MAX) {
            serialNumber_++;
        } else {
            serialNumber_ = 0;
        }
        HILOG_INFO("%{public}s not find connection, make new one", __func__);
        AsyncTask::CompleteCallback complete =
            [weak = context_, want, accountId, connection, connectId](
                NativeEngine& engine, AsyncTask& task, int32_t status) {
                    HILOG_INFO("OnConnectAbilityWithAccount begin");
                    auto context = weak.lock();
                    if (!context) {
                        HILOG_WARN("context is released");
                        task.Reject(engine, CreateJsError(engine, ERROR_CODE_CONTEXT_EMPTY, "Context is released"));
                        return;
                    }
                    HILOG_INFO("context->ConnectAbilityWithAccount connection:%{public}d", (int32_t)connectId);
                    if (!context->ConnectAbilityWithAccount(want, accountId, connection)) {
                        connection->CallJsFailed(ERROR_CODE_CONNECTION_INVALID_STATE);
                    }
                    task.Resolve(engine, engine.CreateUndefined());
                };
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionConnection::OnConnectAbilityWithAccount",
            engine, CreateAsyncTaskWithLastParam(engine, nullptr, nullptr, std::move(complete), &result));
        return engine.CreateNumber(connectId);
    }

    NativeValue* OnDisconnectAbility(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnDisconnectAbility is called");
        // only support one or two params
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("Not enough params");
            Throw(engine, ERROR_CODE_PARAM_INVALID, ERROR_MSG_PARAM_INVALID);
            return engine.CreateUndefined();
        }

        // unwrap want
        AAFwk::Want want;
        // unwrap connectId
        int64_t connectId = -1;
        sptr<JSServiceExtensionConnection> connection = nullptr;
        napi_get_value_int64(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), &connectId);
        HILOG_INFO("OnDisconnectAbility connection:%{public}d", static_cast<int32_t>(connectId));
        auto item = std::find_if(connects_.begin(), connects_.end(),
            [&connectId](
                const std::map<ConnecttionKey, sptr<JSServiceExtensionConnection>>::value_type &obj) {
                    return connectId == obj.first.id;
            });
        if (item != connects_.end()) {
            // match id
            want = item->first.want;
            connection = item->second;
            HILOG_INFO("%{public}s find conn ability exist", __func__);
        } else {
            HILOG_INFO("%{public}s not find conn exist.", __func__);
        }
        // begin disconnect
        AsyncTask::CompleteCallback complete =
            [weak = context_, want, connection](
                NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("OnDisconnectAbility begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_CONTEXT_EMPTY, "Context is released"));
                    return;
                }
                if (connection == nullptr) {
                    HILOG_WARN("connection nullptr");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_CONNECTION_NOT_EXISTED, "not found connection"));
                    return;
                }
                HILOG_INFO("context->DisconnectAbility");
                auto errcode = context->DisconnectAbility(want, connection);
                errcode == 0 ? task.Resolve(engine, engine.CreateUndefined()) :
                    task.Reject(engine, CreateJsError(engine, GetErrorCode(errcode), "Disconnect Ability failed."));
            };

        NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionConnection::OnDisconnectAbility",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnStartExtensionAbility(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnStartExtensionAbility is called.");
        AAFwk::Want want;
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("Invalid params");
            Throw(engine, ERROR_CODE_PARAM_INVALID, ERROR_MSG_PARAM_INVALID);
            return engine.CreateUndefined();
        } else {
            OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[0]), want);
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_, want](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_CONTEXT_EMPTY, "Context is released"));
                    return;
                }
                auto errcode = context->StartServiceExtensionAbility(want);
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, GetErrorCode(errcode), "Start extensionAbility failed."));
                }
            };

        NativeValue* lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[ARGC_ONE];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionContext::OnStartExtensionAbility",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnStartExtensionAbilityWithAccount(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnStartExtensionAbilityWithAccount is called.");
        AAFwk::Want want;
        int32_t accountId = -1;
        if (info.argc != ARGC_TWO && info.argc != ARGC_THREE) {
            HILOG_ERROR("Invalid params");
            Throw(engine, ERROR_CODE_PARAM_INVALID, ERROR_MSG_PARAM_INVALID);
            return engine.CreateUndefined();
        } else {
            OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[0]), want);

            if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[1]), accountId)) {
                HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
                Throw(engine, ERROR_CODE_PARAM_INVALID, ERROR_MSG_PARAM_INVALID);
                return engine.CreateUndefined();
            }
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_, want, accountId](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_CONTEXT_EMPTY, "Context is released"));
                    return;
                }
                auto errcode = context->StartServiceExtensionAbility(want, accountId);
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, GetErrorCode(errcode), "Start extensionAbility failed."));
                }
            };

        NativeValue* lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[ARGC_TWO];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionContext::OnStartExtensionAbilityWithAccount",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnStopExtensionAbility(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnStopExtensionAbility is called.");
        AAFwk::Want want;

        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("Invalid params");
            Throw(engine, ERROR_CODE_PARAM_INVALID, ERROR_MSG_PARAM_INVALID);
            return engine.CreateUndefined()
        } else {
            OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[0]), want);
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_, want](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, 1, "Context is released"));
                    return;
                }
                auto errcode = context->StopServiceExtensionAbility(want);
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, GetErrorCode(errcode), "stop extensionAbility failed."));
                }
            };

        NativeValue* lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[ARGC_ONE];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionContext::OnStopExtensionAbility",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnStopExtensionAbilityWithAccount(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnStopExtensionAbilityWithAccount is called.");
        AAFwk::Want want;
        int32_t accountId = -1;
        if (info.argc != ARGC_TWO && info.argc != ARGC_THREE) {
            HILOG_ERROR("Invalid params");
            Throw(engine, ERROR_CODE_PARAM_INVALID, ERROR_MSG_PARAM_INVALID);
            return engine.CreateUndefined()
        } else {
            OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[0]), want);

            if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[1]), accountId)) {
                HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
                Throw(engine, ERROR_CODE_PARAM_INVALID, ERROR_MSG_PARAM_INVALID);
                return engine.CreateUndefined()
            }
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_, want, accountId](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_CONTEXT_EMPTY, "Context is released"));
                    return;
                }
                auto errcode = context->StopServiceExtensionAbility(want, accountId);
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, GetErrorCode(errcode), "Stop extensionAbility failed."));
                }
            };

        NativeValue* lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[ARGC_TWO];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionContext::OnStopExtensionAbilityWithAccount",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};
} // namespace

NativeValue* CreateJsMetadata(NativeEngine& engine, const AppExecFwk::Metadata &Info)
{
    HILOG_INFO("CreateJsMetadata");
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    object->SetProperty("name", CreateJsValue(engine, Info.name));
    object->SetProperty("value", CreateJsValue(engine, Info.value));
    object->SetProperty("resource", CreateJsValue(engine, Info.resource));
    return objValue;
}

NativeValue* CreateJsMetadataArray(NativeEngine& engine, const std::vector<AppExecFwk::Metadata> &info)
{
    HILOG_INFO("CreateJsMetadataArray");
    NativeValue* arrayValue = engine.CreateArray(info.size());
    NativeArray* array = ConvertNativeValueTo<NativeArray>(arrayValue);
    uint32_t index = 0;
    for (const auto& item : info) {
        array->SetElement(index++, CreateJsMetadata(engine, item));
    }
    return arrayValue;
}

NativeValue* CreateJsServiceExtensionContext(NativeEngine& engine, std::shared_ptr<ServiceExtensionContext> context,
                                             DetachCallback detach, AttachCallback attach)
{
    HILOG_INFO("CreateJsServiceExtensionContext begin");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }
    NativeValue* objValue = CreateJsExtensionContext(engine, context, abilityInfo, detach, attach);
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsServiceExtensionContext> jsContext = std::make_unique<JsServiceExtensionContext>(context);
    object->SetNativePointer(jsContext.release(), JsServiceExtensionContext::Finalizer, nullptr);

    // make handler
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());

    const char *moduleName = "JsServiceExtensionContext";
    BindNativeFunction(engine, *object, "startAbility", moduleName, JsServiceExtensionContext::StartAbility);
    BindNativeFunction(engine, *object, "terminateSelf", moduleName, JsServiceExtensionContext::TerminateAbility);
    BindNativeFunction(engine, *object, "connectAbility", moduleName, JsServiceExtensionContext::ConnectAbility);
    BindNativeFunction(engine, *object, "disconnectAbility",
        moduleName, JsServiceExtensionContext::DisconnectAbility);
    BindNativeFunction(engine, *object, "startAbilityWithAccount",
        moduleName, JsServiceExtensionContext::StartAbilityWithAccount);
    BindNativeFunction(engine, *object, "startAbilityByCall",
        moduleName, JsServiceExtensionContext::StartAbilityByCall);
    BindNativeFunction(
        engine, *object, "connectAbilityWithAccount", moduleName, JsServiceExtensionContext::ConnectAbilityWithAccount);
    BindNativeFunction(engine, *object, "startServiceExtensionAbility", moduleName,
        JsServiceExtensionContext::StartServiceExtensionAbility);
    BindNativeFunction(engine, *object, "startServiceExtensionAbilityWithAccount", moduleName,
        JsServiceExtensionContext::StartServiceExtensionAbilityWithAccount);
    BindNativeFunction(engine, *object, "stopServiceExtensionAbility", moduleName,
        JsServiceExtensionContext::StopServiceExtensionAbility);
    BindNativeFunction(engine, *object, "stopServiceExtensionAbilityWithAccount", moduleName,
        JsServiceExtensionContext::StopServiceExtensionAbilityWithAccount);

    return objValue;
}

JSServiceExtensionConnection::JSServiceExtensionConnection(NativeEngine& engine) : engine_(engine) {}

JSServiceExtensionConnection::~JSServiceExtensionConnection() = default;

void JSServiceExtensionConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

void JSServiceExtensionConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("OnAbilityConnectDone begin, resultCode:%{public}d", resultCode);
    if (handler_ == nullptr) {
        HILOG_INFO("handler_ nullptr");
        return;
    }
    wptr<JSServiceExtensionConnection> connection = this;
    auto task = [connection, element, remoteObject, resultCode]() {
        sptr<JSServiceExtensionConnection> connectionSptr = connection.promote();
        if (!connectionSptr) {
            HILOG_INFO("connectionSptr nullptr");
            return;
        }
        connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
    };
    handler_->PostTask(task, "OnAbilityConnectDone");
}

void JSServiceExtensionConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("HandleOnAbilityConnectDone begin, resultCode:%{public}d", resultCode);
    // wrap ElementName
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(reinterpret_cast<napi_env>(&engine_), element);
    NativeValue* nativeElementName = reinterpret_cast<NativeValue*>(napiElementName);

    // wrap RemoteObject
    HILOG_INFO("OnAbilityConnectDone begin NAPI_ohos_rpc_CreateJsRemoteObject");
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(
        reinterpret_cast<napi_env>(&engine_), remoteObject);
    NativeValue* nativeRemoteObject = reinterpret_cast<NativeValue*>(napiRemoteObject);
    NativeValue* argv[] = {nativeElementName, nativeRemoteObject};
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }
    NativeValue* value = jsConnectionObject_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }
    NativeValue* methodOnConnect = obj->GetProperty("onConnect");
    if (methodOnConnect == nullptr) {
        HILOG_ERROR("Failed to get onConnect from object");
        return;
    }
    HILOG_INFO("JSServiceExtensionConnection::CallFunction onConnect, success");
    engine_.CallFunction(value, methodOnConnect, argv, ARGC_TWO);
    HILOG_INFO("OnAbilityConnectDone end");
}

void JSServiceExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    HILOG_INFO("OnAbilityDisconnectDone begin, resultCode:%{public}d", resultCode);
    if (handler_ == nullptr) {
        HILOG_INFO("handler_ nullptr");
        return;
    }
    wptr<JSServiceExtensionConnection> connection = this;
    auto task = [connection, element, resultCode]() {
        sptr<JSServiceExtensionConnection> connectionSptr = connection.promote();
        if (!connectionSptr) {
            HILOG_INFO("connectionSptr nullptr");
            return;
        }
        connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
    };
    handler_->PostTask(task, "OnAbilityDisconnectDone");
}

void JSServiceExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    HILOG_INFO("HandleOnAbilityDisconnectDone begin, resultCode:%{public}d", resultCode);
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(reinterpret_cast<napi_env>(&engine_), element);
    NativeValue* nativeElementName = reinterpret_cast<NativeValue*>(napiElementName);
    NativeValue* argv[] = {nativeElementName};
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }
    NativeValue* value = jsConnectionObject_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue* method = obj->GetProperty("onDisconnect");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onDisconnect from object");
        return;
    }

    // release connect
    HILOG_INFO("OnAbilityDisconnectDone connects_.size:%{public}zu", connects_.size());
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    auto item = std::find_if(connects_.begin(),
        connects_.end(),
        [bundleName, abilityName, connectionId = connectionId_](
            const std::map<ConnecttionKey, sptr<JSServiceExtensionConnection>>::value_type &obj) {
            return (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName()) &&
                   connectionId == obj.first.id;
        });
    if (item != connects_.end()) {
        // match bundlename && abilityname
        connects_.erase(item);
        HILOG_INFO("OnAbilityDisconnectDone erase connects_.size:%{public}zu", connects_.size());
    }
    HILOG_INFO("OnAbilityDisconnectDone CallFunction success");
    engine_.CallFunction(value, method, argv, ARGC_ONE);
}

void JSServiceExtensionConnection::SetJsConnectionObject(NativeValue* jsConnectionObject)
{
    jsConnectionObject_ = std::unique_ptr<NativeReference>(engine_.CreateReference(jsConnectionObject, 1));
}

void JSServiceExtensionConnection::CallJsFailed(int32_t errorCode)
{
    HILOG_INFO("CallJsFailed begin");
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }
    NativeValue* value = jsConnectionObject_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue* method = obj->GetProperty("onFailed");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onFailed from object");
        return;
    }
    NativeValue* argv[] = {engine_.CreateNumber(errorCode)};
    HILOG_INFO("CallJsFailed CallFunction success");
    engine_.CallFunction(value, method, argv, ARGC_ONE);
    HILOG_INFO("CallJsFailed end");
}
}  // namespace AbilityRuntime
}  // namespace OHOS
