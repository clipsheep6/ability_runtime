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

#include "napi_ability_connection.h"

#include "hilog_tag_wrapper.h"
#include "napi_remote_object.h"

namespace OHOS {
namespace AppExecFwk {

void NAPIAbilityConnection::AddConnectionCallback(std::shared_ptr<ConnectionCallback> callback)
{
    std::lock_guard<std::mutex> guard(lock_);
    callbacks_.emplace_back(callback);
}

int NAPIAbilityConnection::GetConnectionState() const
{
    std::lock_guard<std::mutex> guard(lock_);
    return connectionState_;
}

void NAPIAbilityConnection::SetConnectionState(int connectionState)
{
    std::lock_guard<std::mutex> guard(lock_);
    connectionState_ = connectionState;
}

size_t NAPIAbilityConnection::GetCallbackSize()
{
    std::lock_guard<std::mutex> guard(lock_);
    return callbacks_.size();
}

size_t NAPIAbilityConnection::RemoveAllCallbacks(ConnectRemoveKeyType key)
{
    size_t result = 0;
    std::lock_guard<std::mutex> guard(lock_);
    for (auto it = callbacks_.begin(); it != callbacks_.end();) {
        auto callback = *it;
        if (callback && callback->removeKey == key) {
            it = callbacks_.erase(it);
            result++;
        } else {
            ++it;
        }
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "RemoveAllCallbacks removed size:%{public}zu, left size:%{public}zu", result,
             callbacks_.size());
    return result;
}

void UvWorkOnAbilityConnectDone(uv_work_t *work, int status)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "UvWorkOnAbilityConnectDone, uv_queue_work");
    std::unique_ptr<uv_work_t> managedWork(work);
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "UvWorkOnAbilityConnectDone, work is null");
        return;
    }
    // JS Thread
    std::unique_ptr<ConnectAbilityCB> connectAbilityCB(static_cast<ConnectAbilityCB *>(work->data));
    if (connectAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "UvWorkOnAbilityConnectDone, connectAbilityCB is null");
        return;
    }
    CallbackInfo &cbInfo = connectAbilityCB->cbBase.cbInfo;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cbInfo.env, &scope);
    if (scope == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "napi_open_handle_scope failed");
        return;
    }

    napi_value globalValue;
    napi_get_global(cbInfo.env, &globalValue);
    napi_value func;
    napi_get_named_property(cbInfo.env, globalValue, "requireNapi", &func);

    napi_value rpcInfo;
    napi_create_string_utf8(cbInfo.env, "rpc", NAPI_AUTO_LENGTH, &rpcInfo);
    napi_value funcArgv[1] = { rpcInfo };
    napi_value returnValue;
    napi_call_function(cbInfo.env, globalValue, func, 1, funcArgv, &returnValue);

    napi_value result[ARGS_TWO] = {nullptr};
    result[PARAM0] =
        WrapElementName(cbInfo.env, connectAbilityCB->abilityConnectionCB.elementName);
    napi_value jsRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(
        cbInfo.env, connectAbilityCB->abilityConnectionCB.connection);
    result[PARAM1] = jsRemoteObject;

    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(cbInfo.env, &undefined);
    napi_value callResult = nullptr;
    napi_get_reference_value(cbInfo.env, cbInfo.callback, &callback);

    napi_call_function(
        cbInfo.env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);
    if (cbInfo.callback != nullptr) {
        napi_delete_reference(cbInfo.env, cbInfo.callback);
    }
    napi_close_handle_scope(cbInfo.env, scope);
    TAG_LOGI(AAFwkTag::JSNAPI, "UvWorkOnAbilityConnectDone, uv_queue_work end");
}

void NAPIAbilityConnection::HandleOnAbilityConnectDone(ConnectionCallback &callback, int resultCode)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callback.env, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, loop == null.", __func__);
        return;
    }

    uv_work_t *work = new(std::nothrow) uv_work_t;
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, work == null.", __func__);
        return;
    }

    ConnectAbilityCB *connectAbilityCB = new (std::nothrow) ConnectAbilityCB;
    if (connectAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, connectAbilityCB == null.", __func__);
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
        return;
    }
    connectAbilityCB->cbBase.cbInfo.env = callback.env;
    connectAbilityCB->cbBase.cbInfo.callback = callback.connectCallbackRef;
    callback.connectCallbackRef = nullptr;
    connectAbilityCB->abilityConnectionCB.elementName = element_;
    connectAbilityCB->abilityConnectionCB.resultCode = resultCode;
    connectAbilityCB->abilityConnectionCB.connection = serviceRemoteObject_;
    work->data = static_cast<void *>(connectAbilityCB);

    int rev = uv_queue_work_with_qos(
        loop, work, [](uv_work_t *work) {}, UvWorkOnAbilityConnectDone, uv_qos_user_initiated);
    if (rev != 0) {
        if (connectAbilityCB != nullptr) {
            delete connectAbilityCB;
            connectAbilityCB = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
}

void NAPIAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s bundleName:%{public}s abilityName:%{public}s, resultCode:%{public}d",
             __func__, element.GetBundleName().c_str(), element.GetAbilityName().c_str(), resultCode);
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, remoteObject == nullptr.", __func__);
        return;
    }
    std::lock_guard<std::mutex> guard(lock_);
    element_ = element;
    serviceRemoteObject_ = remoteObject;
    for (const auto &callback : callbacks_) {
        HandleOnAbilityConnectDone(*callback, resultCode);
    }
    connectionState_ = CONNECTION_STATE_CONNECTED;
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
}

void UvWorkOnAbilityDisconnectDone(uv_work_t *work, int status)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "UvWorkOnAbilityDisconnectDone, uv_queue_work");
    std::unique_ptr<uv_work_t> managedWork(work);
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "UvWorkOnAbilityDisconnectDone, work is null");
        return;
    }
    // JS Thread
    std::unique_ptr<ConnectAbilityCB> connectAbilityCB(static_cast<ConnectAbilityCB *>(work->data));
    if (connectAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "UvWorkOnAbilityDisconnectDone, connectAbilityCB is null");
        return;
    }
    CallbackInfo &cbInfo = connectAbilityCB->cbBase.cbInfo;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cbInfo.env, &scope);
    if (scope == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "napi_open_handle_scope failed");
        return;
    }
    napi_value result = WrapElementName(cbInfo.env, connectAbilityCB->abilityConnectionCB.elementName);
    if (cbInfo.callback != nullptr) {
        napi_value callback = nullptr;
        napi_value callResult = nullptr;
        napi_value undefined = nullptr;
        napi_get_undefined(cbInfo.env, &undefined);
        napi_get_reference_value(cbInfo.env, cbInfo.callback, &callback);
        napi_call_function(cbInfo.env, undefined, callback, ARGS_ONE, &result, &callResult);
        napi_delete_reference(cbInfo.env, cbInfo.callback);
        cbInfo.callback = nullptr;
    }
    napi_close_handle_scope(cbInfo.env, scope);

    // release connect
    std::lock_guard<std::mutex> lock(g_connectionsLock_);
    TAG_LOGI(AAFwkTag::JSNAPI, "UvWorkOnAbilityDisconnectDone connects_.size:%{public}zu", connects_.size());
    std::string deviceId = connectAbilityCB->abilityConnectionCB.elementName.GetDeviceID();
    std::string bundleName = connectAbilityCB->abilityConnectionCB.elementName.GetBundleName();
    std::string abilityName = connectAbilityCB->abilityConnectionCB.elementName.GetAbilityName();
    auto item = std::find_if(connects_.begin(), connects_.end(),
        [deviceId, bundleName, abilityName](const std::map<ConnectionKey,
            sptr<NAPIAbilityConnection>>::value_type &obj) {
            return (deviceId == obj.first.want.GetDeviceId()) &&
                   (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName());
        });
    if (item != connects_.end()) {
        // match deviceid & bundlename && abilityname
        connects_.erase(item);
        TAG_LOGI(AAFwkTag::JSNAPI, "UvWorkOnAbilityDisconnectDone erase connects_.size:%{public}zu", connects_.size());
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "UvWorkOnAbilityDisconnectDone, uv_queue_work end");
}

void NAPIAbilityConnection::HandleOnAbilityDisconnectDone(ConnectionCallback &callback, int resultCode)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callback.env, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, loop == nullptr.", __func__);
        return;
    }

    uv_work_t *work = new(std::nothrow) uv_work_t;
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "work == nullptr.");
        return;
    }

    ConnectAbilityCB *connectAbilityCB = new (std::nothrow) ConnectAbilityCB;
    if (connectAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, connectAbilityCB == nullptr.", __func__);
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
        return;
    }

    connectAbilityCB->cbBase.cbInfo.env = callback.env;
    connectAbilityCB->cbBase.cbInfo.callback = callback.disconnectCallbackRef;
    callback.disconnectCallbackRef = nullptr;
    connectAbilityCB->abilityConnectionCB.elementName = element_;
    connectAbilityCB->abilityConnectionCB.resultCode = resultCode;
    work->data = static_cast<void *>(connectAbilityCB);

    int rev = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, UvWorkOnAbilityDisconnectDone);
    if (rev != 0) {
        if (connectAbilityCB != nullptr) {
            delete connectAbilityCB;
            connectAbilityCB = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
}

void NAPIAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s bundleName:%{public}s abilityName:%{public}s, resultCode:%{public}d",
             __func__, element.GetBundleName().c_str(), element.GetAbilityName().c_str(), resultCode);
    std::lock_guard<std::mutex> guard(lock_);
    element_ = element;
    for (const auto &callback : callbacks_) {
        HandleOnAbilityDisconnectDone(*callback, resultCode);
    }
    connectionState_ = CONNECTION_STATE_DISCONNECTED;
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
}

void ClearCallbackWork(uv_work_t* req, int)
{
    std::unique_ptr<uv_work_t> work(req);
    if (!req) {
        TAG_LOGE(AAFwkTag::JSNAPI, "work null");
        return;
    }
    std::unique_ptr<ConnectionCallback> callback(reinterpret_cast<ConnectionCallback*>(req->data));
    if (!callback) {
        TAG_LOGE(AAFwkTag::JSNAPI, "data null");
        return;
    }
    callback->Reset();
}

void ConnectionCallback::Reset()
{
    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (engine == nullptr) {
        removeKey = nullptr;
        return;
    }
    if (pthread_self() == engine->GetTid()) {
        TAG_LOGD(AAFwkTag::JSNAPI, "in-js-thread");
        if (connectCallbackRef) {
            napi_delete_reference(env, connectCallbackRef);
            connectCallbackRef = nullptr;
        }
        if (disconnectCallbackRef) {
            napi_delete_reference(env, disconnectCallbackRef);
            disconnectCallbackRef = nullptr;
        }
        if (failedCallbackRef) {
            napi_delete_reference(env, failedCallbackRef);
            failedCallbackRef = nullptr;
        }
        env = nullptr;
        removeKey = nullptr;
        return;
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "not in-js-thread");
    auto loop = engine->GetUVLoop();
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, loop == nullptr.", __func__);
        env = nullptr;
        removeKey = nullptr;
        return;
    }
    uv_work_t *work = new(std::nothrow) uv_work_t;
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "work == nullptr.");
        return;
    }
    ConnectionCallback *data = new(std::nothrow) ConnectionCallback(std::move(*this));
    work->data = data;
    auto ret = uv_queue_work(loop, work, [](uv_work_t*) {}, ClearCallbackWork);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::JSNAPI, "uv_queue_work failed: %{public}d", ret);
        data->env = nullptr;
        data->removeKey = nullptr;
        delete data;
        delete work;
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS