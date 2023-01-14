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
#include "js_error_utils.h"
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

    static NativeValue* StartRecentAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsServiceExtensionContext* me = CheckParamsAndGetThis<JsServiceExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnStartAbility(*engine, *info, true) : nullptr;
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
    NativeValue* OnStartAbility(NativeEngine& engine, NativeCallbackInfo& info, bool isStartRecent = false)
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
        HILOG_INFO("OnStartAbility is called");
        if (info.argc < ARGC_ONE) {
            HILOG_ERROR("Start ability failed, not enough params.");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }

        size_t unwrapArgc = 0;
        AAFwk::Want want;
        AAFwk::StartOptions startOptions;
        if (!CheckStartAbilityInputParam(engine, info, want, startOptions, unwrapArgc)) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        if (isStartRecent) {
            HILOG_INFO("OnStartRecentAbility is called");
            want.SetParam(Want::PARAM_RESV_START_RECENT, true);
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_, want, startOptions, unwrapArgc](NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("startAbility begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    return;
                }

                ErrCode innerErrorCode = ERR_OK;
                (unwrapArgc == 1) ? innerErrorCode = context->StartAbility(want) :
                    innerErrorCode = context->StartAbility(want, startOptions);
                if (innerErrorCode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
                }
            };

        NativeValue* lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionContext::OnStartAbility",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    bool CheckStartAbilityInputParam(
        NativeEngine& engine, NativeCallbackInfo& info,
        AAFwk::Want& want, AAFwk::StartOptions& startOptions, size_t& unwrapArgc) const
    {
        if (info.argc < ARGC_ONE) {
            return false;
        }
        unwrapArgc = ARGC_ZERO;
        // Check input want
        if (!CheckWantParam(engine, info.argv[INDEX_ZERO], want)) {
            return false;
        }
        ++unwrapArgc;
        if (info.argc > ARGC_ONE && info.argv[1]->TypeOf() == NATIVE_OBJECT) {
            HILOG_INFO("OnStartAbility start options is used.");
            AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[1]), startOptions);
            unwrapArgc++;
        }
        return true;
    }

    NativeValue* OnStartAbilityByCall(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        HILOG_INFO("OnStartAbilityByCall is called.");
        if (info.argc < ARGC_ONE) {
            HILOG_ERROR("Start ability by call failed, not enough params.");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }
        AAFwk::Want want;
        if (!CheckStartAbilityByCallInputParam(engine, info, want)) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        std::shared_ptr<StartAbilityByCallParameters> calls = std::make_shared<StartAbilityByCallParameters>();
        NativeValue* retsult = nullptr;
        calls->callerCallBack = std::make_shared<CallerCallBack>();
        calls->callerCallBack->SetCallBack(GetCallBackDone(calls));
        calls->callerCallBack->SetOnRelease(GetReleaseListen());

        auto context = context_.lock();
        if (context == nullptr) {
            HILOG_ERROR("OnStartAbilityByCall context is nullptr");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
            return engine.CreateUndefined();
        }

        auto ret = context->StartAbilityByCall(want, calls->callerCallBack);
        if (ret) {
            HILOG_ERROR("OnStartAbilityByCall is failed");
            ThrowErrorByNativeErr(engine, ret);
            return engine.CreateUndefined();
        }

        if (calls->remoteCallee == nullptr) {
            HILOG_INFO("OnStartAbilityByCall async wait execute");
            AsyncTask::Schedule("JsAbilityContext::OnStartAbilityByCall", engine,
                CreateAsyncTaskWithLastParam(
                    engine, nullptr, GetCallExecute(calls), GetCallComplete(calls), &retsult));
        } else {
            HILOG_INFO("OnStartAbilityByCall promiss return result execute");
            AsyncTask::Schedule("JSServiceExtensionContext::OnStartAbilityByCall", engine,
                CreateAsyncTaskWithLastParam(engine, nullptr, nullptr, GetCallComplete(calls), &retsult));
        }
        return retsult;
    }

    bool CheckStartAbilityByCallInputParam(NativeEngine& engine, const NativeCallbackInfo& info,
        AAFwk::Want& want) const
    {
        if (!CheckWantParam(engine, info.argv[INDEX_ZERO], want)) {
            return false;
        }
        HILOG_INFO("Connect ability called, callee:%{public}s.%{public}s.",
            want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        return true;
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
        if (info.argc < ARGC_TWO) {
            HILOG_ERROR("Start ability with account failed, not enough params.");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }

        size_t unwrapArgc = 0;
        AAFwk::Want want;
        int32_t accountId = 0;
        if (!CheckStartAbilityWithAccountInputParam(engine, info, want, accountId, unwrapArgc)) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

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
                    if (innerErrorCode == 0) {
                        task.Resolve(engine, engine.CreateUndefined());
                    } else {
                        task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
                    }
                };

        NativeValue* lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionContext::OnStartAbilityWithAccount",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    bool CheckStartAbilityWithAccountInputParam(
        NativeEngine& engine, const NativeCallbackInfo& info,
        AAFwk::Want& want, int32_t& accountId, size_t& unwrapArgc) const
    {
        if (info.argc < ARGC_TWO) {
            return false;
        }
        unwrapArgc = ARGC_ZERO;
        // Check input want
        if (!CheckWantParam(engine, info.argv[INDEX_ZERO], want)) {
            return false;
        }
        ++unwrapArgc;
        if (!CheckAccountIdParam(engine, info.argv[INDEX_ONE], accountId)) {
            return false;
        }
        ++unwrapArgc;
        return true;
    }

    bool CheckAccountIdParam(NativeEngine& engine, NativeValue* value, int32_t& accountId) const
    {
        if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(value), accountId)) {
            HILOG_ERROR("The input accountId is invalid.");
            return false;
        }
        HILOG_INFO("%{public}d accountId:", accountId);
        return true;
    }

    NativeValue* OnTerminateAbility(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        HILOG_INFO("OnTerminateAbility is called");

        AsyncTask::CompleteCallback complete =
            [weak = context_](NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("TerminateAbility begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "Context is released"));
                    return;
                }

                ErrCode innerErrorCode = context->TerminateAbility();
                if (innerErrorCode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
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
        HILOG_INFO("Connect ability called.");
        // Check params count
        if (info.argc < ARGC_TWO) {
            HILOG_ERROR("Connect ability failed, not enough params.");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }
        // Unwrap want and connection
        AAFwk::Want want;
        sptr<JSServiceExtensionConnection> connection = new JSServiceExtensionConnection(engine);
        if (!CheckWantParam(engine, info.argv[0], want) ||
            !CheckConnectionParam(engine, info.argv[1], connection, want)) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
        int64_t connectId = connection->GetConnectionId();
        AsyncTask::CompleteCallback complete =
            [weak = context_, want, connection, connectId](NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("OnConnectAbility begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "Context is released"));
                    return;
                }
                HILOG_INFO("ConnectAbility connection:%{public}d", static_cast<int32_t>(connectId));
                auto innerErrorCode = context->ConnectAbility(want, connection);
                int32_t errcode = static_cast<int32_t>(AbilityRuntime::GetJsErrorCodeByNativeError(innerErrorCode));
                if (errcode) {
                    connection->CallJsFailed(errcode);
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
        // Check params count
        if (info.argc < ARGC_THREE) {
            HILOG_ERROR("Connect ability failed, not enough params.");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }
        // Unwrap want, accountId and connection
        AAFwk::Want want;
        int32_t accountId = 0;
        sptr<JSServiceExtensionConnection> connection = new JSServiceExtensionConnection(engine);
        if (!CheckWantParam(engine, info.argv[INDEX_ZERO], want) ||
            !CheckAccountIdParam(engine, info.argv[INDEX_ONE], accountId) ||
            !CheckConnectionParam(engine, info.argv[INDEX_TWO], connection, want)) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
        int64_t connectId = connection->GetConnectionId();
        AsyncTask::CompleteCallback complete =
            [weak = context_, want, accountId, connection, connectId](
                NativeEngine& engine, AsyncTask& task, int32_t status) {
                    HILOG_INFO("OnConnectAbilityWithAccount begin");
                    auto context = weak.lock();
                    if (!context) {
                        HILOG_WARN("context is released");
                        task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "Context is released"));
                        return;
                    }
                    HILOG_INFO("ConnectAbilityWithAccount connection:%{public}d", static_cast<int32_t>(connectId));
                    auto innerErrorCode = context->ConnectAbilityWithAccount(want, accountId, connection);
                    int32_t errcode = static_cast<int32_t>(AbilityRuntime::GetJsErrorCodeByNativeError(innerErrorCode));
                    if (errcode) {
                        connection->CallJsFailed(errcode);
                    }
                    task.Resolve(engine, engine.CreateUndefined());
                };
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionConnection::OnConnectAbilityWithAccount",
            engine, CreateAsyncTaskWithLastParam(engine, nullptr, nullptr, std::move(complete), &result));
        return engine.CreateNumber(connectId);
    }

    bool CheckWantParam(NativeEngine& engine, NativeValue* value, AAFwk::Want& want) const
    {
        if (!OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(value), want)) {
            HILOG_ERROR("The input want is invalid.");
            return false;
        }
        HILOG_INFO("UnwrapWant, BundleName: %{public}s, AbilityName: %{public}s.",
            want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        return true;
    }

    bool CheckConnectionParam(
        NativeEngine& engine, NativeValue* value,
        sptr<JSServiceExtensionConnection>& connection, const AAFwk::Want& want) const
    {
        if (ConvertNativeValueTo<NativeObject>(value) == nullptr) {
            HILOG_ERROR("Failed to get connection object");
            return false;
        }
        connection->SetJsConnectionObject(value);
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
        return true;
    }

    NativeValue* OnDisconnectAbility(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnDisconnectAbility is called");
        if (info.argc < ARGC_ONE) {
            HILOG_ERROR("Disconnect ability failed, not enough params.");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }
        int64_t connectId = -1;
        if (!CheckOnDisconnectAbilityParam(engine, info, connectId)) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        AAFwk::Want want;
        sptr<JSServiceExtensionConnection> connection = nullptr;
        FindConnection(engine, info, want, connection, connectId);
        // begin disconnect
        AsyncTask::CompleteCallback complete =
            [weak = context_, want, connection](
                NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("OnDisconnectAbility begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "Context is released"));
                    return;
                }
                if (connection == nullptr) {
                    HILOG_WARN("connection nullptr");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_TWO, "not found connection"));
                    return;
                }
                HILOG_INFO("context->DisconnectAbility");
                auto innerErrorCode = context->DisconnectAbility(want, connection);
                if (innerErrorCode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
                }
            };

        NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionConnection::OnDisconnectAbility",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    bool CheckOnDisconnectAbilityParam(NativeEngine& engine, NativeCallbackInfo& info, int64_t& connectId) const
    {
        // Check input connection is number type
        if (!AppExecFwk::UnwrapInt64FromJS2(
            reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), connectId)) {
            HILOG_ERROR("The input connection id is not number type.");
            return false;
        }
        return true;
    }

    void FindConnection(
        NativeEngine& engine, NativeCallbackInfo& info,
        AAFwk::Want& want, sptr<JSServiceExtensionConnection>& connection, int64_t& connectId) const
    {
        HILOG_INFO("Disconnect ability begin, connection:%{public}d.", static_cast<int32_t>(connectId));
        auto item = std::find_if(connects_.begin(),
            connects_.end(),
            [&connectId](const std::map<ConnecttionKey, sptr<JSServiceExtensionConnection>>::value_type &obj) {
                return connectId == obj.first.id;
            });
        if (item != connects_.end()) {
            // match id
            want = item->first.want;
            connection = item->second;
            HILOG_INFO("%{public}s find conn ability exist", __func__);
        } else {
            HILOG_INFO("%{public}s can not find conn.", __func__);
        }
        return;
    }

    NativeValue* OnStartExtensionAbility(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnStartExtensionAbility is called.");
        if (info.argc < ARGC_ONE) {
            HILOG_ERROR("Start extension failed, not enough params.");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }
        AAFwk::Want want;
        if (!CheckWantParam(engine, info.argv[INDEX_ZERO], want)) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_, want](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    return;
                }
                auto innerErrorCode = context->StartServiceExtensionAbility(want);
                if (innerErrorCode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
                }
            };

        NativeValue* lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[ARGC_ONE];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionContext::OnStartExtensionAbility",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnStartExtensionAbilityWithAccount(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        HILOG_INFO("OnStartExtensionAbilityWithAccount is called.");
        if (info.argc < ARGC_TWO) {
            HILOG_ERROR("Stop extension failed, not enough params.");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }
        AAFwk::Want want;
        int32_t accountId = -1;
        if (!CheckWantParam(engine, info.argv[INDEX_ZERO], want) ||
            !CheckAccountIdParam(engine, info.argv[INDEX_ONE], accountId)) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_, want, accountId](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    return;
                }
                auto innerErrorCode = context->StartServiceExtensionAbility(want, accountId);
                if (innerErrorCode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
                }
            };

        NativeValue* lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[ARGC_TWO];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionContext::OnStartExtensionAbilityWithAccount",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnStopExtensionAbility(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        HILOG_INFO("OnStopExtensionAbility is called.");
        if (info.argc < ARGC_ONE) {
            HILOG_ERROR("Start extension failed, not enough params.");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }
        AAFwk::Want want;
        if (!CheckWantParam(engine, info.argv[INDEX_ZERO], want)) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_, want](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    return;
                }
                auto innerErrorCode = context->StopServiceExtensionAbility(want);
                if (innerErrorCode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
                }
            };

        NativeValue* lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[ARGC_ONE];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSServiceExtensionContext::OnStopExtensionAbility",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnStopExtensionAbilityWithAccount(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        HILOG_INFO("OnStopExtensionAbilityWithAccount is called.");
        if (info.argc < ARGC_TWO) {
            HILOG_ERROR("Stop extension failed, not enough params.");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }
        AAFwk::Want want;
        int32_t accountId = -1;
        if (!CheckWantParam(engine, info.argv[INDEX_ZERO], want) ||
            !CheckAccountIdParam(engine, info.argv[INDEX_ONE], accountId)) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_, want, accountId](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    return;
                }
                auto innerErrorCode = context->StopServiceExtensionAbility(want, accountId);
                if (innerErrorCode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
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

NativeValue* CreateJsMetadata(NativeEngine& engine, const AppExecFwk::Metadata &info)
{
    HILOG_INFO("CreateJsMetadata");
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    object->SetProperty("name", CreateJsValue(engine, info.name));
    object->SetProperty("value", CreateJsValue(engine, info.value));
    object->SetProperty("resource", CreateJsValue(engine, info.resource));
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
    BindNativeFunction(
        engine, *object, "connectServiceExtensionAbility", moduleName, JsServiceExtensionContext::ConnectAbility);
    BindNativeFunction(engine, *object, "disconnectAbility",
        moduleName, JsServiceExtensionContext::DisconnectAbility);
    BindNativeFunction(engine, *object, "disconnectServiceExtensionAbility",
        moduleName, JsServiceExtensionContext::DisconnectAbility);
    BindNativeFunction(engine, *object, "startAbilityWithAccount",
        moduleName, JsServiceExtensionContext::StartAbilityWithAccount);
    BindNativeFunction(engine, *object, "startAbilityByCall",
        moduleName, JsServiceExtensionContext::StartAbilityByCall);
    BindNativeFunction(
        engine, *object, "connectAbilityWithAccount", moduleName, JsServiceExtensionContext::ConnectAbilityWithAccount);
    BindNativeFunction(
        engine, *object,
        "connectServiceExtensionAbilityWithAccount", moduleName, JsServiceExtensionContext::ConnectAbilityWithAccount);
    BindNativeFunction(engine, *object, "startServiceExtensionAbility", moduleName,
        JsServiceExtensionContext::StartServiceExtensionAbility);
    BindNativeFunction(engine, *object, "startServiceExtensionAbilityWithAccount", moduleName,
        JsServiceExtensionContext::StartServiceExtensionAbilityWithAccount);
    BindNativeFunction(engine, *object, "stopServiceExtensionAbility", moduleName,
        JsServiceExtensionContext::StopServiceExtensionAbility);
    BindNativeFunction(engine, *object, "stopServiceExtensionAbilityWithAccount", moduleName,
        JsServiceExtensionContext::StopServiceExtensionAbilityWithAccount);
    BindNativeFunction(engine, *object, "startRecentAbility", moduleName,
        JsServiceExtensionContext::StartRecentAbility);

    return objValue;
}

JSServiceExtensionConnection::JSServiceExtensionConnection(NativeEngine& engine) : engine_(engine) {}

JSServiceExtensionConnection::~JSServiceExtensionConnection()
{
    if (jsConnectionObject_ == nullptr) {
        return;
    }

    uv_loop_t *loop = engine_.GetUVLoop();
    if (loop == nullptr) {
        return;
    }

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        return;
    }
    work->data = reinterpret_cast<void *>(jsConnectionObject_.release());
    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
    [](uv_work_t *work, int status) {
        if (work == nullptr) {
            return;
        }
        if (work->data == nullptr) {
            delete work;
            work = nullptr;
            return;
        }
        delete reinterpret_cast<NativeReference *>(work->data);
        work->data = nullptr;
        delete work;
        work = nullptr;
    });
    if (ret != 0) {
        delete reinterpret_cast<NativeReference *>(work->data);
        work->data = nullptr;
        delete work;
        work = nullptr;
    }
}

void JSServiceExtensionConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

int64_t JSServiceExtensionConnection::GetConnectionId()
{
    return connectionId_;
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
