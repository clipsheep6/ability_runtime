/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "form_runtime/js_form_extension_context.h"

#include <cinttypes>
#include <cstdint>

#include "hilog_wrapper.h"
#include "form_mgr_errors.h"
#include "js_extension_context.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_ability.h"
#include "napi_common_start_options.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "napi_form_util.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
const int UPDATE_FORM_PARAMS_SIZE = 2;

std::map<ConnectionKey, sptr<JSFormExtensionConnection>, key_compare> g_connects;
int64_t g_serialNumber = 0;

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
class JsFormExtensionContext final {
public:
    explicit JsFormExtensionContext(const std::shared_ptr<FormExtensionContext>& context) : context_(context) {}
    ~JsFormExtensionContext() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("called.");
        std::unique_ptr<JsFormExtensionContext>(static_cast<JsFormExtensionContext*>(data));
    }

    static NativeValue* UpdateForm(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsFormExtensionContext* me = CheckParamsAndGetThis<JsFormExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnUpdateForm(*engine, *info) : nullptr;
    }

    static NativeValue* StartAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsFormExtensionContext* me = CheckParamsAndGetThis<JsFormExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnStartAbility(*engine, *info) : nullptr;
    }

    static NativeValue* ConnectAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsFormExtensionContext* me = CheckParamsAndGetThis<JsFormExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnConnectAbility(*engine, *info) : nullptr;
    }

    static NativeValue* DisconnectAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsFormExtensionContext* me = CheckParamsAndGetThis<JsFormExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnDisconnectAbility(*engine, *info) : nullptr;
    }

private:
    std::weak_ptr<FormExtensionContext> context_;

    NativeValue* OnUpdateForm(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("called.");
        if (info.argc < UPDATE_FORM_PARAMS_SIZE) {
            HILOG_ERROR("Not enough params, not enough params");
            return engine.CreateUndefined();
        }

        std::string strFormId;
        ConvertFromJsValue(engine, info.argv[0], strFormId);
        int64_t formId = strFormId.empty() ? -1 : std::stoll(strFormId);

        AppExecFwk::FormProviderData formProviderData;
        std::string formDataStr = "{}";
        NativeObject* nativeObject = ConvertNativeValueTo<NativeObject>(info.argv[1]);
        if (nativeObject != nullptr) {
            NativeValue* nativeDataValue = nativeObject->GetProperty("data");
            if (nativeDataValue == nullptr || !ConvertFromJsValue(engine, nativeDataValue, formDataStr)) {
                HILOG_ERROR("NativeDataValue is nullptr or ConvertFromJsValue failed.");
            }
        } else {
            HILOG_ERROR("NativeObject is nullptr.");
        }

        formProviderData = AppExecFwk::FormProviderData(formDataStr);
        AsyncTask::CompleteCallback complete =
            [weak = context_, formId, formProviderData](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, 1, "Context is released"));
                    return;
                }
                auto errcode = context->UpdateForm(formId, formProviderData);
                if (errcode == ERR_OK) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "update form failed."));
                }
            };

        NativeValue* lastParam =
            (info.argc == UPDATE_FORM_PARAMS_SIZE) ? nullptr : info.argv[info.argc - 1];
        NativeValue* result = nullptr;
        AsyncTask::ScheduleHighQos("JsFormExtensionContext::OnUpdateForm",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnStartAbility(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnStartAbility is called");
        // only support one or two params
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("Not enough params");
            NapiFormUtil::ThrowParamNumError(engine, std::to_string(info.argc), "1 or 2");
            return engine.CreateUndefined();
        }

        decltype(info.argc) unwrapArgc = 0;
        AAFwk::Want want;
        bool unwrapResult = OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), want);
        if (!unwrapResult) {
            HILOG_ERROR("Failed to unwrap want.");
            NapiFormUtil::ThrowParamTypeError(engine, "want", "Want");
            return engine.CreateUndefined();
        }
        HILOG_INFO("Start ability, bundleName: %{public}s abilityName: %{public}s.",
            want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        unwrapArgc++;

        AsyncTask::CompleteCallback complete =
            [weak = context_, want](NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("startAbility begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine,
                        NapiFormUtil::CreateErrorByInternalErrorCode(engine, ERR_APPEXECFWK_FORM_COMMON_CODE));
                    return;
                }

                // entry to the core functionality.
                ErrCode innerErrorCode = context->StartAbility(want);
                if (innerErrorCode == ERR_OK) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    HILOG_ERROR("Failed to StartAbility, errorCode: %{public}d.", innerErrorCode);
                    task.Reject(engine,
                        NapiFormUtil::CreateErrorByInternalErrorCode(engine, innerErrorCode));
                }
            };

        NativeValue* lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
        NativeValue* result = nullptr;
        AsyncTask::ScheduleHighQos("JsFormExtensionContext::OnStartAbility",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnConnectAbility(NativeEngine& engine, NativeCallbackInfo& info)
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
        sptr<JSFormExtensionConnection> connection = new JSFormExtensionConnection(engine);
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
                HILOG_DEBUG("ConnectAbility connection:%{public}d", static_cast<int32_t>(connectId));
                auto innerErrorCode = context->ConnectAbility(want, connection);
                int32_t errcode = static_cast<int32_t>(AbilityRuntime::GetJsErrorCodeByNativeError(innerErrorCode));
                if (errcode) {
                    connection->CallJsFailed(errcode);
                    RemoveConnection(connectId);
                }
                task.Resolve(engine, engine.CreateUndefined());
            };
        NativeValue* result = nullptr;
        AsyncTask::ScheduleHighQos("JSFormExtensionConnection::OnConnectAbility",
            engine, CreateAsyncTaskWithLastParam(engine, nullptr, nullptr, std::move(complete), &result));
        return engine.CreateNumber(connectId);
    }

    NativeValue* OnDisconnectAbility(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("DisconnectAbility");
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
        sptr<JSFormExtensionConnection> connection = nullptr;
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
        AsyncTask::Schedule("JSFormExtensionConnection::OnDisconnectAbility",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
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
        sptr<JSFormExtensionConnection>& connection, AAFwk::Want& want) const
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
        AAFwk::Want& want, sptr<JSFormExtensionConnection>& connection, int64_t& connectId) const
    {
        HILOG_INFO("Disconnect ability begin, connection:%{public}d.", static_cast<int32_t>(connectId));
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
};
} // namespace

NativeValue* CreateJsFormExtensionContext(NativeEngine& engine, std::shared_ptr<FormExtensionContext> context)
{
    HILOG_DEBUG("Create js form extension context.");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }
    NativeValue* objValue = CreateJsExtensionContext(engine, context, abilityInfo);
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsFormExtensionContext> jsContext = std::make_unique<JsFormExtensionContext>(context);
    object->SetNativePointer(jsContext.release(), JsFormExtensionContext::Finalizer, nullptr);

    const char *moduleName = "JsFormExtensionContext";
    BindNativeFunction(engine, *object, "updateForm", moduleName, JsFormExtensionContext::UpdateForm);
    BindNativeFunction(engine, *object, "startAbility", moduleName, JsFormExtensionContext::StartAbility);
    BindNativeFunction(
        engine, *object, "connectServiceExtensionAbility", moduleName, JsFormExtensionContext::ConnectAbility);
    BindNativeFunction(engine, *object, "disconnectServiceExtensionAbility",
        moduleName, JsFormExtensionContext::DisconnectAbility);

    HILOG_DEBUG("Create finished.");
    return objValue;
}

JSFormExtensionConnection::JSFormExtensionConnection(NativeEngine& engine) : engine_(engine) {}

JSFormExtensionConnection::~JSFormExtensionConnection()
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

void JSFormExtensionConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

int64_t JSFormExtensionConnection::GetConnectionId()
{
    return connectionId_;
}

void JSFormExtensionConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_DEBUG("OnAbilityConnectDone, resultCode:%{public}d", resultCode);
    wptr<JSFormExtensionConnection> connection = this;
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([connection, element, remoteObject, resultCode](NativeEngine &engine, AsyncTask &task, int32_t status) {
            sptr<JSFormExtensionConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                HILOG_ERROR("connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
        });

    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JSFormExtensionConnection::OnAbilityConnectDone",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSFormExtensionConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("HandleOnAbilityConnectDone, resultCode:%{public}d", resultCode);
    // wrap ElementName
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(reinterpret_cast<napi_env>(&engine_), element);
    NativeValue* nativeElementName = reinterpret_cast<NativeValue*>(napiElementName);

    // wrap RemoteObject
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
    engine_.CallFunction(value, methodOnConnect, argv, ARGC_TWO);
}

void JSFormExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    HILOG_DEBUG("OnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    wptr<JSFormExtensionConnection> connection = this;
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([connection, element, resultCode](NativeEngine &engine, AsyncTask &task, int32_t status) {
            sptr<JSFormExtensionConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                HILOG_INFO("connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JSFormExtensionConnection::OnAbilityDisconnectDone",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSFormExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    HILOG_INFO("HandleOnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
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
    HILOG_DEBUG("OnAbilityDisconnectDone g_connects.size:%{public}zu", g_connects.size());
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    auto item = std::find_if(g_connects.begin(),
        g_connects.end(),
        [bundleName, abilityName, connectionId = connectionId_](
            const auto &obj) {
            return (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName()) &&
                   connectionId == obj.first.id;
        });
    if (item != g_connects.end()) {
        // match bundleName && abilityName
        g_connects.erase(item);
        HILOG_DEBUG("OnAbilityDisconnectDone erase g_connects.size:%{public}zu", g_connects.size());
    }
    engine_.CallFunction(value, method, argv, ARGC_ONE);
}

void JSFormExtensionConnection::SetJsConnectionObject(NativeValue* jsConnectionObject)
{
    jsConnectionObject_ = std::unique_ptr<NativeReference>(engine_.CreateReference(jsConnectionObject, 1));
}

void JSFormExtensionConnection::RemoveConnectionObject()
{
    jsConnectionObject_.reset();
}

void JSFormExtensionConnection::CallJsFailed(int32_t errorCode)
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
} // namespace AbilityRuntime
} // namespace OHOS
