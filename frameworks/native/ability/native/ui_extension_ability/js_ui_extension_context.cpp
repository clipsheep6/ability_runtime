/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "js_ui_extension_context.h"

#include <cstdint>

#include "event_handler.h"
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
#include "napi_common_start_options.h"
#include "napi_remote_object.h"
#include "start_options.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
} // namespace

static std::map<UIExtensionConnectionKey, sptr<JSUIExtensionConnection>, key_compare> g_connects;
static int64_t g_serialNumber = 0;
void RemoveConnection(int64_t connectId)
{
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
    [&connectId](const auto &obj) {
        return connectId == obj.first.id;
    });
    if (item != g_connects.end()) {
        HILOG_DEBUG("remove conn ability exist");
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_connects.erase(item);
    } else {
        HILOG_DEBUG("remove conn ability not exist");
    }
}

void FindConnection(AAFwk::Want& want, sptr<JSUIExtensionConnection>& connection, int64_t& connectId)
{
    HILOG_DEBUG("Disconnect ability enter, connection:%{public}d.", static_cast<int32_t>(connectId));
    auto item = std::find_if(g_connects.begin(),
        g_connects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != g_connects.end()) {
        // match id
        want = item->first.want;
        connection = item->second;
        HILOG_DEBUG("find conn ability exist");
    }
    return;
}

bool CheckConnectionParam(napi_env env, napi_value value, sptr<JSUIExtensionConnection>& connection, AAFwk::Want& want)
{
    if (!CheckTypeForNapiValue(env, value, napi_object)) {
        HILOG_ERROR("Failed to get connection object");
        return false;
    }
    connection->SetJsConnectionObject(value);
    UIExtensionConnectionKey key;
    key.id = g_serialNumber;
    key.want = want;
    connection->SetConnectionId(key.id);
    g_connects.emplace(key, connection);
    if (g_serialNumber < INT32_MAX) {
        g_serialNumber++;
    } else {
        g_serialNumber = 0;
    }
    HILOG_DEBUG("not find connection, create a new connection");
    return true;
}

void JsUIExtensionContext::Finalizer(napi_env env, void* data, void* hint)
{
    HILOG_DEBUG("JsUIExtensionContext Finalizer is called");
    std::unique_ptr<JsUIExtensionContext>(static_cast<JsUIExtensionContext*>(data));
}

napi_value JsUIExtensionContext::StartAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnStartAbility);
}

napi_value JsUIExtensionContext::TerminateSelf(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnTerminateSelf);
}

napi_value JsUIExtensionContext::StartAbilityForResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnStartAbilityForResult);
}

napi_value JsUIExtensionContext::TerminateSelfWithResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnTerminateSelfWithResult);
}

napi_value JsUIExtensionContext::ConnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnConnectAbility);
}

napi_value JsUIExtensionContext::DisconnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnDisconnectAbility);
}

napi_value JsUIExtensionContext::OnStartAbility(napi_env env, NapiCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("OnStartAbility is called");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Start ability failed, not enough params.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    size_t unwrapArgc = 0;
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    if (!CheckStartAbilityInputParam(env, info, want, startOptions, unwrapArgc)) {
        HILOG_DEBUG("Failed, input param type invalid");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    if (screenMode_ == want.GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE)) {
        HILOG_ERROR("Not support half screen pulling up half screen");
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, startOptions, unwrapArgc](napi_env env, NapiAsyncTask& task, int32_t status) {
            HILOG_DEBUG("startAbility begin");
            auto context = weak.lock();
            if (!context) {
                HILOG_ERROR("context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            ErrCode innerErrorCode = ERR_OK;
            (unwrapArgc == 1) ? innerErrorCode = context->StartAbility(want) :
                innerErrorCode = context->StartAbility(want, startOptions);
            if (innerErrorCode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
            }
        };

    napi_value lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSUIExtensionContext OnStartAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsUIExtensionContext::OnTerminateSelf(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("OnTerminateSelf is called");
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_ERROR("context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            ErrCode innerErrorCode = context->TerminateSelf();
            if (innerErrorCode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
            }
        };

    napi_value lastParam = (info.argc == ARGC_ZERO) ? nullptr : info.argv[INDEX_ZERO];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSUIExtensionContext OnTerminateSelf",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsUIExtensionContext::OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("OnStartAbilityForResult called.");
    if (info.argc == ARGC_ZERO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    size_t unwrapArgc = 0;
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    if (!CheckStartAbilityInputParam(env, info, want, startOptions, unwrapArgc)) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        HILOG_DEBUG("input param type invalid");
        return CreateJsUndefined(env);
    }
    napi_value lastParam = info.argc > unwrapArgc ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> uasyncTask = CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);
    RuntimeTask task = [env, asyncTask](int resultCode, const AAFwk::Want &want, bool isInner) {
        HILOG_INFO("async callback is called.");
        napi_value abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            HILOG_WARN("wrap abilityResult failed.");
            asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
        if (isInner) {
            asyncTask->Reject(env, CreateJsErrorByNativeErr(env, resultCode));
            return;
        }
        asyncTask->Resolve(env, abilityResult);
    };
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_WARN("context is released.");
        asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        return result;
    }
    want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
    int curRequestCode = context->GenerateCurRequestCode();
    (unwrapArgc == INDEX_ONE) ? context->StartAbilityForResult(want, curRequestCode, std::move(task))
                              : context->StartAbilityForResult(want, startOptions, curRequestCode, std::move(task));
    HILOG_DEBUG("OnStartAbilityForResult end.");
    return result;
}

napi_value JsUIExtensionContext::OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("OnTerminateSelfWithResult is start");

    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    int resultCode = 0;
    AAFwk::Want want;
    if (!AppExecFwk::UnWrapAbilityResult(env, info.argv[0], resultCode, want)) {
        HILOG_ERROR("OnTerminateSelfWithResult Failed to parse ability result!");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, resultCode](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            auto errorCode = context->TerminateSelf();
            if (errorCode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errorCode));
            }
        };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[1] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsUIExtensionContext::OnTerminateSelfWithResult",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    HILOG_DEBUG("OnTerminateSelfWithResult is called end");
    return result;
}

napi_value JsUIExtensionContext::OnConnectAbility(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("ConnectAbility called.");
    // Check params count
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("Connect ability failed, not enough params.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    // Unwrap want and connection
    AAFwk::Want want;
    sptr<JSUIExtensionConnection> connection = new JSUIExtensionConnection(env);
    if (!AppExecFwk::UnwrapWant(env, info.argv[0], want) ||
        !CheckConnectionParam(env, info.argv[1], connection, want)) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }
    int64_t connectId = connection->GetConnectionId();
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, connection, connectId](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_ERROR("context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                RemoveConnection(connectId);
                return;
            }
            HILOG_DEBUG("ConnectAbility connection:%{public}d.", static_cast<int32_t>(connectId));
            auto innerErrorCode = context->ConnectAbility(want, connection);
            int32_t errcode = static_cast<int32_t>(AbilityRuntime::GetJsErrorCodeByNativeError(innerErrorCode));
            if (errcode) {
                connection->CallJsFailed(errcode);
                RemoveConnection(connectId);
            }
            task.Resolve(env, CreateJsUndefined(env));
        };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSUIExtensionConnection::OnConnectAbility",
        env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
    return CreateJsValue(env, connectId);
}

napi_value JsUIExtensionContext::OnDisconnectAbility(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("DisconnectAbility start");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Disconnect ability error, not enough params.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    int64_t connectId = -1;
    if (!AppExecFwk::UnwrapInt64FromJS2(env, info.argv[INDEX_ZERO], connectId)) {
        HILOG_ERROR("Invalid connectId");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    sptr<JSUIExtensionConnection> connection = nullptr;
    FindConnection(want, connection, connectId);
    // begin disconnect
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, connection](
            napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released.");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            if (connection == nullptr) {
                HILOG_WARN("connection nullptr");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            HILOG_DEBUG("context->DisconnectAbility.");
            auto innerErrorCode = context->DisconnectAbility(want, connection);
            if (innerErrorCode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
            }
        };

    napi_value lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSUIExtensionConnection::OnDisconnectAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsUIExtensionContext::CreateJsUIExtensionContext(napi_env env,
    std::shared_ptr<UIExtensionContext> context)
{
    HILOG_DEBUG("CreateJsUIExtensionContext begin");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }
    napi_value objValue = CreateJsExtensionContext(env, context, abilityInfo);

    std::unique_ptr<JsUIExtensionContext> jsContext = std::make_unique<JsUIExtensionContext>(context);
    napi_wrap(env, objValue, jsContext.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "JsUIExtensionContext";
    BindNativeFunction(env, objValue, "startAbility", moduleName, StartAbility);
    BindNativeFunction(env, objValue, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(env, objValue, "startAbilityForResult", moduleName, StartAbilityForResult);
    BindNativeFunction(env, objValue, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);
    BindNativeFunction(env, objValue, "connectServiceExtensionAbility", moduleName, ConnectAbility);
    BindNativeFunction(env, objValue, "disconnectServiceExtensionAbility", moduleName, DisconnectAbility);

    return objValue;
}

bool JsUIExtensionContext::CheckStartAbilityInputParam(napi_env env, NapiCallbackInfo& info,
    AAFwk::Want& want, AAFwk::StartOptions& startOptions, size_t& unwrapArgc) const
{
    if (info.argc < ARGC_ONE) {
        return false;
    }
    unwrapArgc = ARGC_ZERO;
    // Check input want
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        return false;
    }
    if (!want.HasParameter(Want::PARAM_BACK_TO_OTHER_MISSION_STACK)) {
        want.SetParam(Want::PARAM_BACK_TO_OTHER_MISSION_STACK, true);
    }
    ++unwrapArgc;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[1], napi_object)) {
        AppExecFwk::UnwrapStartOptions(env, info.argv[1], startOptions);
        unwrapArgc++;
    }
    return true;
}

JSUIExtensionConnection::JSUIExtensionConnection(napi_env env) : env_(env) {}

JSUIExtensionConnection::~JSUIExtensionConnection()
{
    if (jsConnectionObject_ == nullptr) {
        return;
    }

    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
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

void JSUIExtensionConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

int64_t JSUIExtensionConnection::GetConnectionId()
{
    return connectionId_;
}

void JSUIExtensionConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_DEBUG("OnAbilityConnectDone, resultCode:%{public}d", resultCode);
    wptr<JSUIExtensionConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, remoteObject, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSUIExtensionConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                HILOG_ERROR("connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
        });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSUIExtensionConnection::OnAbilityConnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSUIExtensionConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_DEBUG("HandleOnAbilityConnectDone start, resultCode:%{public}d", resultCode);
    // wrap ElementName
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);

    // wrap RemoteObject
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(env_, remoteObject);
    napi_value argv[] = {napiElementName, napiRemoteObject};
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ null");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        HILOG_ERROR("Failed to get object");
        return;
    }
    napi_value methodOnConnect = nullptr;
    napi_get_named_property(env_, obj, "onConnect", &methodOnConnect);
    if (methodOnConnect == nullptr) {
        HILOG_ERROR("Failed to get onConnect from object.");
        return;
    }
    napi_call_function(env_, obj, methodOnConnect, ARGC_TWO, argv, nullptr);
}

void JSUIExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    HILOG_DEBUG("OnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    wptr<JSUIExtensionConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSUIExtensionConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                HILOG_INFO("connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSUIExtensionConnection::OnAbilityDisconnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSUIExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    HILOG_DEBUG("HandleOnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);
    napi_value argv[] = {napiElementName};
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        HILOG_ERROR("Failed to get object");
        return;
    }
    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onDisconnect", &method);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onDisconnect from object");
        return;
    }

    // release connect
    RemoveConnection(connectionId_);
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
}

void JSUIExtensionConnection::SetJsConnectionObject(napi_value jsConnectionObject)
{
    napi_ref value = nullptr;
    napi_create_reference(env_, jsConnectionObject, 1, &value);
    jsConnectionObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(value));
}

void JSUIExtensionConnection::RemoveConnectionObject()
{
    jsConnectionObject_.reset();
}

void JSUIExtensionConnection::CallJsFailed(int32_t errorCode)
{
    HILOG_DEBUG("CallJsFailed enter");
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr.");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        HILOG_ERROR("wrong to get object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onFailed", &method);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onFailed from object");
        return;
    }
    napi_value argv[] = { CreateJsValue(env_, errorCode) };
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
    HILOG_DEBUG("CallJsFailed end");
}

}  // namespace AbilityRuntime
}  // namespace OHOS
