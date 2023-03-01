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

#include "js_ui_extension_context.h"

#include <cstdint>

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
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr int32_t ERROR_CODE_TWO = 2;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;

struct ConnecttionKey {
    AAFwk::Want want;
    int64_t id;
};

struct key_compare {
    bool operator()(const ConnecttionKey &key1, const ConnecttionKey &key2) const
    {
        if (key1.id < key2.id) {
            return true;
        }
        return false;
    }
};
} // namespace

static std::map<ConnecttionKey, sptr<JSUIExtensionConnection>, key_compare> connects_;
static int64_t serialNumber_ = 0;
static std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;

void JsUIExtensionContext::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_DEBUG("JsUIExtensionContext Finalizer is called");
    std::unique_ptr<JsUIExtensionContext>(static_cast<JsUIExtensionContext*>(data));
}

NativeValue *JsUIExtensionContext::StartAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContext* me = CheckParamsAndGetThis<JsUIExtensionContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbility(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContext::TerminateAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContext* me = CheckParamsAndGetThis<JsUIExtensionContext>(engine, info);
    return (me != nullptr) ? me->OnTerminateAbility(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContext::ConnectExtensionAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContext* me = CheckParamsAndGetThis<JsUIExtensionContext>(engine, info);
    return (me != nullptr) ? me->OnConnectExtensionAbility(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContext::DisconnectExtensionAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContext* me = CheckParamsAndGetThis<JsUIExtensionContext>(engine, info);
    return (me != nullptr) ? me->OnDisconnectExtensionAbility(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContext::StartUIExtensionAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContext* me = CheckParamsAndGetThis<JsUIExtensionContext>(engine, info);
    return (me != nullptr) ? me->OnStartUIExtensionAbility(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContext::OnStartAbility(NativeEngine& engine, NativeCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("OnStartAbility is called");
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

    AsyncTask::CompleteCallback complete =
        [weak = context_, want, startOptions, unwrapArgc](NativeEngine& engine, AsyncTask& task, int32_t status) {
            HILOG_DEBUG("startAbility begin");
            auto context = weak.lock();
            if (!context) {
                HILOG_ERROR("context is released");
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
    AsyncTask::Schedule("JSUIExtensionContext OnStartAbility",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

bool JsUIExtensionContext::CheckStartAbilityInputParam(NativeEngine& engine, NativeCallbackInfo& info,
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
        AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[1]), startOptions);
        unwrapArgc++;
    }
    return true;
}

NativeValue *JsUIExtensionContext::OnTerminateAbility(NativeEngine& engine, const NativeCallbackInfo& info)
{
    HILOG_DEBUG("OnTerminateAbility is called");
    AsyncTask::CompleteCallback complete =
        [weak = context_](NativeEngine& engine, AsyncTask& task, int32_t status) {
            HILOG_DEBUG("TerminateAbility begin");
            auto context = weak.lock();
            if (!context) {
                HILOG_ERROR("context is released");
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
    AsyncTask::Schedule("JSUIExtensionContext OnTerminateAbility",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsUIExtensionContext::OnConnectExtensionAbility(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("OnConnectExtensionAbility is called");
    // Check params count
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("Connect extension ability failed, not enough params.");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }
    // Unwrap want and connection
    AAFwk::Want want;
    sptr<JSUIExtensionConnection> connection = new JSUIExtensionConnection(engine);
    if (!CheckWantParam(engine, info.argv[0], want) ||
        !CheckConnectionParam(engine, info.argv[1], connection, want)) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    int64_t connectId = connection->GetConnectionId();
    AsyncTask::CompleteCallback complete =
        [weak = context_, want, connection, connectId](NativeEngine& engine, AsyncTask& task, int32_t status) {
            HILOG_DEBUG("OnConnectExtensionAbility begin");
            auto context = weak.lock();
            if (!context) {
                HILOG_ERROR("context is released");
                task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "Context is released"));
                return;
            }
            HILOG_DEBUG("ConnectExtensionAbility connection:%{public}d", static_cast<int32_t>(connectId));
            auto innerErrorCode = context->ConnectExtensionAbility(want, connection);
            int32_t errcode = static_cast<int32_t>(AbilityRuntime::GetJsErrorCodeByNativeError(innerErrorCode));
            if (errcode) {
                connection->CallJsFailed(errcode);
            }
            task.Resolve(engine, engine.CreateUndefined());
        };
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JSUIExtensionConnection OnConnectExtensionAbility",
        engine, CreateAsyncTaskWithLastParam(engine, nullptr, nullptr, std::move(complete), &result));
    return engine.CreateNumber(connectId);
}

bool JsUIExtensionContext::CheckWantParam(NativeEngine& engine, NativeValue* value, AAFwk::Want& want) const
{
    if (!OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(value), want)) {
        HILOG_ERROR("The input want is invalid.");
        return false;
    }
    HILOG_DEBUG("UnwrapWant, BundleName: %{public}s, AbilityName: %{public}s.", want.GetBundle().c_str(),
        want.GetElement().GetAbilityName().c_str());
    return true;
}

bool JsUIExtensionContext::CheckConnectionParam(NativeEngine& engine, NativeValue* value,
    sptr<JSUIExtensionConnection>& connection, AAFwk::Want& want) const
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
    return true;
}

NativeValue *JsUIExtensionContext::OnDisconnectExtensionAbility(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("OnDisconnectExtensionAbility is called");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Disconnect extension ability failed, not enough params.");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }
    int64_t connectId = -1;
    if (!CheckOnDisconnectExtensionAbilityParam(engine, info, connectId)) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AAFwk::Want want;
    sptr<JSUIExtensionConnection> connection = nullptr;
    FindConnection(engine, info, want, connection, connectId);
    HILOG_DEBUG("DisconnectExtensionAbility connection:%{public}d", static_cast<int32_t>(connectId));
    // begin disconnect
    AsyncTask::CompleteCallback complete =
        [weak = context_, want, connection](
            NativeEngine& engine, AsyncTask& task, int32_t status) {
            HILOG_DEBUG("OnDisconnectExtensionAbility begin");
            auto context = weak.lock();
            if (!context) {
                HILOG_ERROR("context is released");
                task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "Context is released"));
                return;
            }
            if (connection == nullptr) {
                HILOG_ERROR("connection is nullptr");
                task.Reject(engine, CreateJsError(engine, ERROR_CODE_TWO, "not found connection"));
                return;
            }
            auto innerErrorCode = context->DisconnectExtensionAbility(want, connection);
            if (innerErrorCode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
            }
        };

    NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JSUIExtensionConnection OnDisconnectExtensionAbility",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

bool JsUIExtensionContext::CheckOnDisconnectExtensionAbilityParam(NativeEngine& engine, NativeCallbackInfo& info,
    int64_t& connectId) const
{
    // Check input connection is number type
    if (!AppExecFwk::UnwrapInt64FromJS2(
        reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), connectId)) {
        HILOG_ERROR("The input connection id is not number type.");
        return false;
    }
    return true;
}

void JsUIExtensionContext::FindConnection(NativeEngine& engine, NativeCallbackInfo& info, AAFwk::Want& want,
    sptr<JSUIExtensionConnection>& connection, int64_t& connectId) const
{
    HILOG_DEBUG("FindConnection begin, connection:%{public}d.", static_cast<int32_t>(connectId));
    auto item = std::find_if(connects_.begin(), connects_.end(),
        [&connectId](const std::map<ConnecttionKey, sptr<JSUIExtensionConnection>>::value_type &obj) {
            return connectId == obj.first.id;
        });
    if (item != connects_.end()) {
        // match id
        want = item->first.want;
        connection = item->second;
        HILOG_DEBUG("FindConnection find conn ability exist");
    } else {
        HILOG_WARN("FindConnection can not find conn.");
    }
    return;
}

NativeValue *JsUIExtensionContext::OnStartUIExtensionAbility(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("OnStartUIExtensionAbility is called.");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Start ui extension ability failed, not enough params.");
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
                HILOG_ERROR("context is released");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto innerErrorCode = context->StartUIExtensionAbility(want);
            if (innerErrorCode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
            }
        };

    NativeValue* lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[ARGC_ONE];
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JSUIExtensionContext OnStartUIExtensionAbility",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsUIExtensionContext::CreateJsUIExtensionContext(NativeEngine& engine, std::shared_ptr<UIExtensionContext> context)
{
    HILOG_DEBUG("CreateJsUIExtensionContext begin");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }
    NativeValue* objValue = CreateJsExtensionContext(engine, context, abilityInfo);
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsUIExtensionContext> jsContext = std::make_unique<JsUIExtensionContext>(context);
    object->SetNativePointer(jsContext.release(), Finalizer, nullptr);

    // make handler
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());

    const char *moduleName = "JsUIExtensionContext";
    BindNativeFunction(engine, *object, "startAbility", moduleName, StartAbility);
    BindNativeFunction(engine, *object, "terminateSelf", moduleName, TerminateAbility);
    BindNativeFunction( engine, *object, "connectExtensionAbility", moduleName, ConnectExtensionAbility);
    BindNativeFunction(engine, *object, "disconnectExtensionAbility", moduleName, DisconnectExtensionAbility);
    BindNativeFunction(engine, *object, "startUIExtensionAbility", moduleName, StartUIExtensionAbility);

    return objValue;
}

JSUIExtensionConnection::~JSUIExtensionConnection()
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
    HILOG_DEBUG("OnAbilityConnectDone begin, resultCode:%{public}d", resultCode);
    if (handler_ == nullptr) {
        HILOG_INFO("handler is nullptr");
        return;
    }
    wptr<JSUIExtensionConnection> connection = this;
    auto task = [connection, element, remoteObject, resultCode]() {
        sptr<JSUIExtensionConnection> connectionSptr = connection.promote();
        if (!connectionSptr) {
            HILOG_INFO("connection is nullptr");
            return;
        }
        connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
    };
    handler_->PostTask(task, "OnAbilityConnectDone");
}

void JSUIExtensionConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_DEBUG("HandleOnAbilityConnectDone begin, resultCode:%{public}d", resultCode);
    // wrap ElementName
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(reinterpret_cast<napi_env>(&engine_), element);
    NativeValue* nativeElementName = reinterpret_cast<NativeValue*>(napiElementName);

    // wrap RemoteObject
    HILOG_DEBUG("OnAbilityConnectDone begin NAPI_ohos_rpc_CreateJsRemoteObject");
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(reinterpret_cast<napi_env>(&engine_),
        remoteObject);
    NativeValue* nativeRemoteObject = reinterpret_cast<NativeValue*>(napiRemoteObject);
    NativeValue* argv[] = {nativeElementName, nativeRemoteObject};
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject is nullptr");
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
    engine_.CallFunction(value, methodOnConnect, argv, ARGC_TWO);
    HILOG_DEBUG("OnAbilityConnectDone end");
}

void JSUIExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    HILOG_DEBUG("OnAbilityDisconnectDone begin, resultCode:%{public}d", resultCode);
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr");
        return;
    }
    wptr<JSUIExtensionConnection> connection = this;
    auto task = [connection, element, resultCode]() {
        sptr<JSUIExtensionConnection> connectionSptr = connection.promote();
        if (!connectionSptr) {
            HILOG_ERROR("connection is nullptr");
            return;
        }
        connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
    };
    handler_->PostTask(task, "OnAbilityDisconnectDone");
}

void JSUIExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    HILOG_DEBUG("HandleOnAbilityDisconnectDone begin, resultCode:%{public}d", resultCode);
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(reinterpret_cast<napi_env>(&engine_), element);
    NativeValue* nativeElementName = reinterpret_cast<NativeValue*>(napiElementName);
    NativeValue* argv[] = {nativeElementName};
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject is nullptr");
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
    HILOG_DEBUG("HandleOnAbilityDisconnectDone connects_.size:%{public}zu", connects_.size());
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    auto item = std::find_if(connects_.begin(),
        connects_.end(),
        [bundleName, abilityName, connectionId = connectionId_](
            const std::map<ConnecttionKey, sptr<JSUIExtensionConnection>>::value_type &obj) {
            return (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName()) &&
                   connectionId == obj.first.id;
        });
    if (item != connects_.end()) {
        // match bundlename && abilityname
        connects_.erase(item);
        HILOG_DEBUG("HandleOnAbilityDisconnectDone erase connects_.size:%{public}zu", connects_.size());
    }
    engine_.CallFunction(value, method, argv, ARGC_ONE);
    HILOG_DEBUG("HandleOnAbilityDisconnectDone end.");
}

void JSUIExtensionConnection::SetJsConnectionObject(NativeValue* jsConnectionObject)
{
    jsConnectionObject_ = std::unique_ptr<NativeReference>(engine_.CreateReference(jsConnectionObject, 1));
}

void JSUIExtensionConnection::CallJsFailed(int32_t errorCode)
{
    HILOG_DEBUG("CallJsFailed begin");
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject is nullptr");
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
    engine_.CallFunction(value, method, argv, ARGC_ONE);
    HILOG_DEBUG("CallJsFailed end");
}
}  // namespace AbilityRuntime
}  // namespace OHOS
