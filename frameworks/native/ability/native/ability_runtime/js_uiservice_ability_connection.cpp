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

#include "js_uiservice_ability_connection.h"

#include "ability_business_error.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "js_ui_service_proxy.h"
#include "napi_common_want.h"
#include "ui_ability_servicehost_stub_impl.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr size_t ARGC_ONE = 1;

namespace UIServiceConnection {
static std::map<ConnectionKey, sptr<JSUIServiceExtAbilityConnection>, KeyCompare> g_uiServiceExtensionConnects;
static std::recursive_mutex g_uiServiceExtensionConnectsLock_;
static int64_t g_uiServiceExtensionSerialNumber = 0;

// This function has to be called from engine thread
void RemoveUIServiceAbilityConnection(int64_t connectId)
{
    std::lock_guard<std::recursive_mutex> lock(g_uiServiceExtensionConnectsLock_);
    auto item = std::find_if(g_uiServiceExtensionConnects.begin(), g_uiServiceExtensionConnects.end(),
    [&connectId](const auto &obj) {
        return connectId == obj.first.id;
    });
    if (item != g_uiServiceExtensionConnects.end()) {
        TAG_LOGD(AAFwkTag::CONTEXT, "exist, remove");
        if (item->second) {
            item->second->RemoveConnectionObject();
            item->second->SetProxyObject(nullptr);
        }
        g_uiServiceExtensionConnects.erase(item);
    } else {
        TAG_LOGD(AAFwkTag::CONTEXT, "not exist");
    }
    TAG_LOGI(AAFwkTag::CONTEXT, "Connects new size:%{public}zu", g_uiServiceExtensionConnects.size());
}

int64_t InsertUIServiceAbilityConnection(sptr<JSUIServiceExtAbilityConnection> connection, const AAFwk::Want &want)
{
    std::lock_guard<std::recursive_mutex> lock(g_uiServiceExtensionConnectsLock_);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "connection null");
        return -1;
    }
    int64_t connectId = g_uiServiceExtensionSerialNumber;
    ConnectionKey key;
    key.id = g_uiServiceExtensionSerialNumber;
    key.want = want;
    key.accountId = 0;
    connection->SetConnectionId(key.id);
    g_uiServiceExtensionConnects.emplace(key, connection);
    if (g_uiServiceExtensionSerialNumber < INT32_MAX) {
        g_uiServiceExtensionSerialNumber++;
    } else {
        g_uiServiceExtensionSerialNumber = 0;
    }
    return connectId;
}

void FindUIServiceAbilityConnection(const int64_t& connectId, AAFwk::Want& want,
    sptr<JSUIServiceExtAbilityConnection>& connection)
{
    std::lock_guard<std::recursive_mutex> lock(g_uiServiceExtensionConnectsLock_);
    TAG_LOGI(AAFwkTag::UI_EXT, "connection:%{public}d", static_cast<int32_t>(connectId));
    auto item = std::find_if(g_uiServiceExtensionConnects.begin(), g_uiServiceExtensionConnects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != g_uiServiceExtensionConnects.end()) {
        want = item->first.want;
        connection = item->second;
        TAG_LOGI(AAFwkTag::UI_EXT, "found");
    } else {
        TAG_LOGI(AAFwkTag::UI_EXT, "not found");
    }
}

void FindUIServiceAbilityConnection(napi_env env, AAFwk::Want& want, napi_value callback,
    sptr<JSUIServiceExtAbilityConnection>& connection)
{
    std::lock_guard<std::recursive_mutex> lock(g_uiServiceExtensionConnectsLock_);
    auto item = std::find_if(g_uiServiceExtensionConnects.begin(), g_uiServiceExtensionConnects.end(),
        [&want, env, callback](const auto &obj) {
        bool wantEquals = (obj.first.want.GetElement() == want.GetElement());
        std::unique_ptr<NativeReference>& tempCallbackPtr = obj.second->GetJsConnectionObject();
        bool callbackObjectEquals =
            JSUIServiceExtAbilityConnection::IsJsCallbackObjectEquals(env, tempCallbackPtr, callback);
        return wantEquals && callbackObjectEquals;
    });
    if (item == g_uiServiceExtensionConnects.end()) {
        return;
    }
    connection = item->second;
}
}

JSUIServiceExtAbilityConnection::JSUIServiceExtAbilityConnection(napi_env env) : JSAbilityConnection(env)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "JSUIServiceExtAbilityConnection");
    wptr<JSUIServiceExtAbilityConnection> weakthis = this;
    serviceHostStub_ = sptr<UIAbilityServiceHostStubImpl>::MakeSptr(weakthis);
}

JSUIServiceExtAbilityConnection::~JSUIServiceExtAbilityConnection()
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "~JSUIServiceExtAbilityConnection");
    serviceHostStub_ = nullptr;
    napiAsyncTask_ = nullptr;
    ReleaseNativeReference(serviceProxyObject_.release());
}

void JSUIServiceExtAbilityConnection::HandleOnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    if (napiAsyncTask_ != nullptr) {
        TAG_LOGI(AAFwkTag::UISERVC_EXT, "HandleOnAbilityConnectDone, CreateJsUIServiceProxy");
        sptr<UIAbilityServiceHostStubImpl> hostStub = GetServiceHostStub();
        sptr<IRemoteObject> hostProxy = nullptr;
        if (hostStub != nullptr) {
            hostProxy = hostStub->AsObject();
        }
        napi_value proxy = AAFwk::JsUIServiceProxy::CreateJsUIServiceProxy(env_, remoteObject,
            connectionId_, hostProxy);
        SetProxyObject(proxy);
        napiAsyncTask_->ResolveWithNoError(env_, proxy);

        ResolveDuplicatedPendingTask(env_, proxy);
    } else {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "HandleOnAbilityConnectDone, napiAsyncTask_ null");
    }
    napiAsyncTask_ = nullptr;
}

void JSUIServiceExtAbilityConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "HandleOnAbilityDisconnectDone");
    if (napiAsyncTask_ != nullptr) {
        napi_value innerError = CreateJsError(env_, AbilityErrorCode::ERROR_CODE_INNER);
        napiAsyncTask_->Reject(env_, innerError);
        RejectDuplicatedPendingTask(env_, innerError);
        napiAsyncTask_ = nullptr;
    }

    CallJsOnDisconnect();
    UIServiceConnection::RemoveUIServiceAbilityConnection(connectionId_);
}

void JSUIServiceExtAbilityConnection::SetNapiAsyncTask(std::shared_ptr<NapiAsyncTask>& task)
{
    napiAsyncTask_ = task;
}

void JSUIServiceExtAbilityConnection::AddDuplicatedPendingTask(std::unique_ptr<NapiAsyncTask>& task)
{
    duplicatedPendingTaskList_.push_back(std::move(task));
}

void JSUIServiceExtAbilityConnection::ResolveDuplicatedPendingTask(napi_env env, napi_value proxy)
{
    for (auto &task : duplicatedPendingTaskList_) {
        if (task != nullptr) {
            task->ResolveWithNoError(env, proxy);
        }
    }
    duplicatedPendingTaskList_.clear();
}

void JSUIServiceExtAbilityConnection::RejectDuplicatedPendingTask(napi_env env, napi_value error)
{
    for (auto &task : duplicatedPendingTaskList_) {
        if (task != nullptr) {
            task->Reject(env, error);
        }
    }
    duplicatedPendingTaskList_.clear();
}

void JSUIServiceExtAbilityConnection::SetProxyObject(napi_value proxy)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "SetProxyObject");
    serviceProxyObject_.reset();
    if (proxy != nullptr) {
        napi_ref ref = nullptr;
        napi_create_reference(env_, proxy, 1, &ref);
        serviceProxyObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
    }
}

napi_value JSUIServiceExtAbilityConnection::GetProxyObject()
{
    if (serviceProxyObject_ == nullptr) {
        return nullptr;
    }
    return serviceProxyObject_->GetNapiValue();
}

int32_t JSUIServiceExtAbilityConnection::OnSendData(OHOS::AAFwk::WantParams &data)
{
    wptr<JSUIServiceExtAbilityConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, wantParams = data](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSUIServiceExtAbilityConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                TAG_LOGE(AAFwkTag::UISERVC_EXT, "connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnSendData(wantParams);
        });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSUIServiceExtAbilityConnection::SendData",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));

    return static_cast<int32_t>(AbilityErrorCode::ERROR_OK);
}

void JSUIServiceExtAbilityConnection::HandleOnSendData(const OHOS::AAFwk::WantParams &data)
{
    napi_value argv[] = { AppExecFwk::CreateJsWantParams(env_, data) };
    CallObjectMethod("onData", argv, ARGC_ONE);
}

void JSUIServiceExtAbilityConnection::CallJsOnDisconnect()
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "called");
    CallObjectMethod("onDisconnect", nullptr, 0);
}

bool JSUIServiceExtAbilityConnection::IsJsCallbackObjectEquals(napi_env env,
    std::unique_ptr<NativeReference> &callback, napi_value value)
{
    if (value == nullptr || callback == nullptr) {
        return callback.get() == reinterpret_cast<NativeReference*>(value);
    }
    auto object = callback->GetNapiValue();
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get object.");
        return false;
    }
    bool result = false;
    if (napi_strict_equals(env, object, value, &result) != napi_ok) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Object does not match value.");
        return false;
    }
    return result;
}

}
}
