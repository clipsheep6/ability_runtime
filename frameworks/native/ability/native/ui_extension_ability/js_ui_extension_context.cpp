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

static std::map<ConnectionKey, sptr<JSUIExtensionConnection>, key_compare> g_connects;
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

bool CheckOnDisconnectAbilityParam(NativeEngine& engine, NativeCallbackInfo& info, int64_t& connectId)
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
    AAFwk::Want& want, sptr<JSUIExtensionConnection>& connection, int64_t& connectId)
{
    HILOG_DEBUG("Disconnect ability begin, connection:%{public}d.", static_cast<int32_t>(connectId));
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

bool CheckConnectionParam(
    NativeEngine& engine, NativeValue* value,
    sptr<JSUIExtensionConnection>& connection, AAFwk::Want& want)
{
    if (ConvertNativeValueTo<NativeObject>(value) == nullptr) {
        HILOG_ERROR("Failed to get connection object");
        return false;
    }
    connection->SetJsConnectionObject(value);
    ConnectionKey key;
    key.id = g_serialNumber;
    key.want = want;
    connection->SetConnectionId(key.id);
    g_connects.emplace(key, connection);
    if (g_serialNumber < INT32_MAX) {
        g_serialNumber++;
    } else {
        g_serialNumber = 0;
    }
    HILOG_DEBUG("not find connection, make new one");
    return true;
}

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

NativeValue *JsUIExtensionContext::TerminateSelf(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContext* me = CheckParamsAndGetThis<JsUIExtensionContext>(engine, info);
    return (me != nullptr) ? me->OnTerminateSelf(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContext::StartAbilityForResult(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsUIExtensionContext *me = CheckParamsAndGetThis<JsUIExtensionContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbilityForResult(*engine, *info) : nullptr;
}

NativeValue* JsUIExtensionContext::TerminateSelfWithResult(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContext* me = CheckParamsAndGetThis<JsUIExtensionContext>(engine, info);
    return (me != nullptr) ? me->OnTerminateSelfWithResult(*engine, *info) : nullptr;
}

NativeValue* JsUIExtensionContext::ConnectAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContext* me = CheckParamsAndGetThis<JsUIExtensionContext>(engine, info);
    return (me != nullptr) ? me->OnConnectAbility(*engine, *info) : nullptr;
}

NativeValue* JsUIExtensionContext::DisconnectAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContext* me = CheckParamsAndGetThis<JsUIExtensionContext>(engine, info);
    return (me != nullptr) ? me->OnDisconnectAbility(*engine, *info) : nullptr;
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
        HILOG_DEBUG("Failed, input param type invalid");
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
    AsyncTask::ScheduleHighQos("JSUIExtensionContext OnStartAbility",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsUIExtensionContext::OnTerminateSelf(NativeEngine& engine, const NativeCallbackInfo& info)
{
    HILOG_DEBUG("OnTerminateSelf is called");
    AsyncTask::CompleteCallback complete =
        [weak = context_](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_ERROR("context is released");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            ErrCode innerErrorCode = context->TerminateSelf();
            if (innerErrorCode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
            }
        };

    NativeValue* lastParam = (info.argc == ARGC_ZERO) ? nullptr : info.argv[INDEX_ZERO];
    NativeValue* result = nullptr;
    AsyncTask::ScheduleHighQos("JSUIExtensionContext OnTerminateSelf",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsUIExtensionContext::OnStartAbilityForResult(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("called.");
    if (info.argc == ARGC_ZERO) {
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }
    size_t unwrapArgc = 0;
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    if (!CheckStartAbilityInputParam(engine, info, want, startOptions, unwrapArgc)) {
        HILOG_DEBUG("input param type invalid");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    NativeValue *lastParam = info.argc > unwrapArgc ? info.argv[unwrapArgc] : nullptr;
    NativeValue *result = nullptr;
    std::unique_ptr<AsyncTask> uasyncTask = CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<AsyncTask> asyncTask = std::move(uasyncTask);
    RuntimeTask task = [&engine, asyncTask](int resultCode, const AAFwk::Want &want, bool isInner) {
        HILOG_INFO("async callback is called.");
        NativeValue *abilityResult = WrapAbilityResult(engine, resultCode, want);
        if (abilityResult == nullptr) {
            HILOG_WARN("wrap abilityResult failed.");
            asyncTask->Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
        if (isInner) {
            asyncTask->Reject(engine, CreateJsErrorByNativeErr(engine, resultCode));
            return;
        }
        asyncTask->Resolve(engine, abilityResult);
    };
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_WARN("context is released.");
        asyncTask->Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        return result;
    }
    want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
    int curRequestCode = context->GenerateCurRequestCode();
    (unwrapArgc == INDEX_ONE) ? context->StartAbilityForResult(want, curRequestCode, std::move(task))
                              : context->StartAbilityForResult(want, startOptions, curRequestCode, std::move(task));
    HILOG_DEBUG("end.");
    return result;
}

NativeValue* JsUIExtensionContext::OnTerminateSelfWithResult(NativeEngine& engine, const NativeCallbackInfo& info)
{
    HILOG_DEBUG("OnTerminateSelfWithResult is called");

    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    int resultCode = 0;
    AAFwk::Want want;
    if (!JsUIExtensionContext::UnWrapAbilityResult(engine, info.argv[0], resultCode, want)) {
        HILOG_ERROR("OnTerminateSelfWithResult Failed to parse ability result!");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [weak = context_, want, resultCode](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            auto errorCode = context->TerminateSelf();
            if (errorCode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, errorCode));
            }
        };

    NativeValue* lastParam = (info.argc > ARGC_ONE) ? info.argv[1] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::ScheduleHighQos("JsUIExtensionContext::OnTerminateSelfWithResult",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    HILOG_DEBUG("OnTerminateSelfWithResult is called end");
    return result;
}

NativeValue* JsUIExtensionContext::OnConnectAbility(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("ConnectAbility called.");
    // Check params count
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("Connect ability failed, not enough params.");
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
            auto context = weak.lock();
            if (!context) {
                HILOG_ERROR("context is released");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
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
            task.Resolve(engine, engine.CreateUndefined());
        };
    NativeValue* result = nullptr;
    AsyncTask::ScheduleHighQos("JSUIExtensionConnection::OnConnectAbility",
        engine, CreateAsyncTaskWithLastParam(engine, nullptr, nullptr, std::move(complete), &result));
    return engine.CreateNumber(connectId);
}

NativeValue* JsUIExtensionContext::OnDisconnectAbility(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("DisconnectAbility");
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
    sptr<JSUIExtensionConnection> connection = nullptr;
    FindConnection(engine, info, want, connection, connectId);
    // begin disconnect
    AsyncTask::CompleteCallback complete =
        [weak = context_, want, connection](
            NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            if (connection == nullptr) {
                HILOG_WARN("connection nullptr");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            HILOG_DEBUG("context->DisconnectAbility");
            auto innerErrorCode = context->DisconnectAbility(want, connection);
            if (innerErrorCode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
            }
        };

    NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JSUIExtensionConnection::OnDisconnectAbility",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsUIExtensionContext::CreateJsUIExtensionContext(NativeEngine& engine,
    std::shared_ptr<UIExtensionContext> context)
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

    const char *moduleName = "JsUIExtensionContext";
    BindNativeFunction(engine, *object, "startAbility", moduleName, StartAbility);
    BindNativeFunction(engine, *object, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(engine, *object, "startAbilityForResult", moduleName, StartAbilityForResult);
    BindNativeFunction(engine, *object, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);
    BindNativeFunction(engine, *object, "connectServiceExtensionAbility", moduleName, ConnectAbility);
    BindNativeFunction(engine, *object, "disconnectServiceExtensionAbility", moduleName, DisconnectAbility);

    return objValue;
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
    if (!want.HasParameter(Want::PARAM_BACK_TO_OTHER_MISSION_STACK)) {
        want.SetParam(Want::PARAM_BACK_TO_OTHER_MISSION_STACK, true);
    }
    ++unwrapArgc;
    if (info.argc > ARGC_ONE && info.argv[1]->TypeOf() == NATIVE_OBJECT) {
        AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[1]), startOptions);
        unwrapArgc++;
    }
    return true;
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

bool JsUIExtensionContext::UnWrapWant(NativeEngine& engine, NativeValue* argv, AAFwk::Want& want)
{
    if (argv == nullptr) {
        HILOG_WARN("UnWrapWant argv == nullptr!");
        return false;
    }
    return AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(argv), want);
}

bool JsUIExtensionContext::UnWrapAbilityResult(NativeEngine& engine, NativeValue* argv, int& resultCode,
    AAFwk::Want& want)
{
    if (argv == nullptr) {
        HILOG_WARN("UnWrapAbilityResult argv == nullptr!");
        return false;
    }
    if (argv->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_WARN("UnWrapAbilityResult invalid type of abilityResult!");
        return false;
    }
    NativeObject* jObj = ConvertNativeValueTo<NativeObject>(argv);
    NativeValue* jResultCode = jObj->GetProperty("resultCode");
    if (jResultCode == nullptr) {
        HILOG_WARN("UnWrapAbilityResult jResultCode == nullptr!");
        return false;
    }
    if (jResultCode->TypeOf() != NativeValueType::NATIVE_NUMBER) {
        HILOG_WARN("UnWrapAbilityResult invalid type of resultCode!");
        return false;
    }
    resultCode = int64_t(*ConvertNativeValueTo<NativeNumber>(jObj->GetProperty("resultCode")));
    NativeValue* jWant = jObj->GetProperty("want");
    if (jWant == nullptr) {
        HILOG_WARN("UnWrapAbilityResult jWant == nullptr!");
        return false;
    }
    if (jWant->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_WARN("UnWrapAbilityResult invalid type of want!");
        return false;
    }
    return JsUIExtensionContext::UnWrapWant(engine, jWant, want);
}

NativeValue *JsUIExtensionContext::WrapAbilityResult(
    NativeEngine &engine, const int &resultCode, const AAFwk::Want &want)
{
    NativeValue *jAbilityResult = engine.CreateObject();
    NativeObject *abilityResult = ConvertNativeValueTo<NativeObject>(jAbilityResult);
    abilityResult->SetProperty("resultCode", engine.CreateNumber(resultCode));
    abilityResult->SetProperty("want", JsUIExtensionContext::WrapWant(engine, want));
    return jAbilityResult;
}

NativeValue *JsUIExtensionContext::WrapWant(NativeEngine &engine, const AAFwk::Want &want)
{
    return reinterpret_cast<NativeValue *>(AppExecFwk::WrapWant(reinterpret_cast<napi_env>(&engine), want));
}

JSUIExtensionConnection::JSUIExtensionConnection(NativeEngine& engine) : engine_(engine) {}

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
    HILOG_DEBUG("OnAbilityConnectDone, resultCode:%{public}d", resultCode);
    wptr<JSUIExtensionConnection> connection = this;
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([connection, element, remoteObject, resultCode](NativeEngine &engine, AsyncTask &task, int32_t status) {
            sptr<JSUIExtensionConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                HILOG_ERROR("connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
        });

    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::ScheduleHighQos("JSUIExtensionConnection::OnAbilityConnectDone",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSUIExtensionConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_DEBUG("HandleOnAbilityConnectDone start, resultCode:%{public}d", resultCode);
    // wrap ElementName
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(reinterpret_cast<napi_env>(&engine_), element);
    NativeValue* nativeElementName = reinterpret_cast<NativeValue*>(napiElementName);

    // wrap RemoteObject
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(
        reinterpret_cast<napi_env>(&engine_), remoteObject);
    NativeValue* nativeRemoteObject = reinterpret_cast<NativeValue*>(napiRemoteObject);
    NativeValue* argv[] = {nativeElementName, nativeRemoteObject};
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ null");
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
}

void JSUIExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    HILOG_DEBUG("OnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    wptr<JSUIExtensionConnection> connection = this;
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([connection, element, resultCode](NativeEngine &engine, AsyncTask &task, int32_t status) {
            sptr<JSUIExtensionConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                HILOG_INFO("connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JSUIExtensionConnection::OnAbilityDisconnectDone",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSUIExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    HILOG_DEBUG("HandleOnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
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
    RemoveConnection(connectionId_);
    engine_.CallFunction(value, method, argv, ARGC_ONE);
}

void JSUIExtensionConnection::SetJsConnectionObject(NativeValue* jsConnectionObject)
{
    jsConnectionObject_ = std::unique_ptr<NativeReference>(engine_.CreateReference(jsConnectionObject, 1));
}

void JSUIExtensionConnection::RemoveConnectionObject()
{
    jsConnectionObject_.reset();
}

void JSUIExtensionConnection::CallJsFailed(int32_t errorCode)
{
    HILOG_DEBUG("CallJsFailed begin");
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
    engine_.CallFunction(value, method, argv, ARGC_ONE);
    HILOG_DEBUG("CallJsFailed end");
}

}  // namespace AbilityRuntime
}  // namespace OHOS
