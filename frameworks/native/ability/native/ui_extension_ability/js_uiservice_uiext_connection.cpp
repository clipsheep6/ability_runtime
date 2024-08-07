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

#include "js_uiservice_uiext_connection.h"

#include "ability_business_error.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "js_ui_service_proxy.h"
#include "napi_common_want.h"
#include "ui_extension_servicehost_stub_impl.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr size_t ARGC_ONE = 1;

namespace UIServiceConnection {
static std::map<UIExtensionConnectionKey, sptr<JSUIServiceUIExtConnection>, key_compare> gUiServiceExtConnects;
static std::recursive_mutex gUiServiceExtConnectsLock;
static int64_t gUiServiceExtConnectSn = 0;

void AddUIServiceExtensionConnection(AAFwk::Want& want, sptr<JSUIServiceUIExtConnection>& connection)
{
    std::lock_guard<std::recursive_mutex> lock(gUiServiceExtConnectsLock);
    UIExtensionConnectionKey key;
    key.id = gUiServiceExtConnectSn;
    key.want = want;
    connection->SetConnectionId(key.id);
    gUiServiceExtConnects.emplace(key, connection);
    if (gUiServiceExtConnectSn < INT32_MAX) {
        gUiServiceExtConnectSn++;
    } else {
        gUiServiceExtConnectSn = 0;
    }
}

void RemoveUIServiceExtensionConnection(const int64_t& connectId)
{
    std::lock_guard<std::recursive_mutex> lock(gUiServiceExtConnectsLock);
    auto item = std::find_if(gUiServiceExtConnects.begin(), gUiServiceExtConnects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != gUiServiceExtConnects.end()) {
        TAG_LOGI(AAFwkTag::UI_EXT, "found, erase");
        gUiServiceExtConnects.erase(item);
    } else {
        TAG_LOGI(AAFwkTag::UI_EXT, "not found");
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "connects new size:%{public}zu", gUiServiceExtConnects.size());
}

void FindUIServiceExtensionConnection(const int64_t& connectId, AAFwk::Want& want,
    sptr<JSUIServiceUIExtConnection>& connection)
{
    std::lock_guard<std::recursive_mutex> lock(gUiServiceExtConnectsLock);
    TAG_LOGI(AAFwkTag::UI_EXT, "connection:%{public}d", static_cast<int32_t>(connectId));
    auto item = std::find_if(gUiServiceExtConnects.begin(), gUiServiceExtConnects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != gUiServiceExtConnects.end()) {
        want = item->first.want;
        connection = item->second;
        TAG_LOGI(AAFwkTag::UI_EXT, "found");
    } else {
        TAG_LOGI(AAFwkTag::UI_EXT, "not found");
    }
}

void FindUIServiceExtensionConnection(napi_env env, AAFwk::Want& want, napi_value callback,
    sptr<JSUIServiceUIExtConnection>& connection)
{
    std::lock_guard<std::recursive_mutex> lock(gUiServiceExtConnectsLock);
    auto item = std::find_if(gUiServiceExtConnects.begin(), gUiServiceExtConnects.end(),
        [&want, env, callback](const auto &obj) {
        bool wantEquals = (obj.first.want.GetElement() == want.GetElement());
        std::unique_ptr<NativeReference>& tempCallbackPtr = obj.second->GetJsConnectionObject();
        bool callbackObjectEquals =
            JSUIServiceUIExtConnection::IsJsCallbackObjectEquals(env, tempCallbackPtr, callback);
        return wantEquals && callbackObjectEquals;
    });
    if (item == gUiServiceExtConnects.end()) {
        return;
    }
    connection = item->second;
}
}

JSUIServiceUIExtConnection::JSUIServiceUIExtConnection(napi_env env) : JSUIExtensionConnection(env)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "JSUIServiceUIExtConnection");
    wptr<JSUIServiceUIExtConnection> weakthis = this;
    serviceHostStub_ = sptr<UIExtensionServiceHostStubImpl>::MakeSptr(weakthis);
}

JSUIServiceUIExtConnection::~JSUIServiceUIExtConnection()
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "~JSUIServiceUIExtConnection");
    serviceHostStub_ = nullptr;
    napiAsyncTask_.reset();
    ReleaseNativeReference(serviceProxyObject_.release());
}

void JSUIServiceUIExtConnection::HandleOnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    if (napiAsyncTask_ != nullptr) {
        TAG_LOGI(AAFwkTag::UISERVC_EXT, "HandleOnAbilityConnectDone, CreateJsUIServiceProxy");
        sptr<UIExtensionServiceHostStubImpl> hostStub = GetServiceHostStub();
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
    napiAsyncTask_.reset();
}

void JSUIServiceUIExtConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    if (napiAsyncTask_ != nullptr) {
        napi_value innerError = CreateJsError(env_, AbilityErrorCode::ERROR_CODE_INNER);
        napiAsyncTask_->Reject(env_, innerError);
        RejectDuplicatedPendingTask(env_, innerError);
        napiAsyncTask_ = nullptr;
    }
    CallJsOnDisconnect();
    SetProxyObject(nullptr);
    RemoveConnectionObject();
    duplicatedPendingTaskList_.clear();
    UIServiceConnection::RemoveUIServiceExtensionConnection(connectionId_);
}

void JSUIServiceUIExtConnection::SetNapiAsyncTask(std::shared_ptr<NapiAsyncTask>& task)
{
    napiAsyncTask_ = task;
}

void JSUIServiceUIExtConnection::AddDuplicatedPendingTask(std::unique_ptr<NapiAsyncTask>& task)
{
    duplicatedPendingTaskList_.push_back(std::move(task));
}

void JSUIServiceUIExtConnection::ResolveDuplicatedPendingTask(napi_env env, napi_value proxy)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "called, size %{public}zu", duplicatedPendingTaskList_.size());
    for (auto &task : duplicatedPendingTaskList_) {
        if (task != nullptr) {
            task->ResolveWithNoError(env, proxy);
        }
    }
    duplicatedPendingTaskList_.clear();
}

void JSUIServiceUIExtConnection::RejectDuplicatedPendingTask(napi_env env, napi_value error)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "called, size %{public}zu", duplicatedPendingTaskList_.size());
    for (auto &task : duplicatedPendingTaskList_) {
        if (task != nullptr) {
            task->Reject(env, error);
        }
    }
    duplicatedPendingTaskList_.clear();
}

void JSUIServiceUIExtConnection::SetProxyObject(napi_value proxy)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "SetProxyObject");
    serviceProxyObject_.reset();
    if (proxy != nullptr) {
        napi_ref ref = nullptr;
        napi_create_reference(env_, proxy, 1, &ref);
        serviceProxyObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
    }
}

napi_value JSUIServiceUIExtConnection::GetProxyObject()
{
    if (serviceProxyObject_ == nullptr) {
        return nullptr;
    }
    return serviceProxyObject_->GetNapiValue();
}

int32_t JSUIServiceUIExtConnection::OnSendData(OHOS::AAFwk::WantParams &data)
{
    wptr<JSUIServiceUIExtConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, wantParams = data](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSUIServiceUIExtConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                TAG_LOGE(AAFwkTag::UISERVC_EXT, "connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnSendData(wantParams);
        });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSUIServiceUIExtConnection::SendData",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));

    return static_cast<int32_t>(AbilityErrorCode::ERROR_OK);
}

void JSUIServiceUIExtConnection::HandleOnSendData(const OHOS::AAFwk::WantParams &data)
{
    napi_value argv[] = { AppExecFwk::CreateJsWantParams(env_, data) };
    CallObjectMethod("onData", argv, ARGC_ONE);
}

void JSUIServiceUIExtConnection::CallJsOnDisconnect()
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "called");
    CallObjectMethod("onDisconnect", nullptr, 0);
}

bool JSUIServiceUIExtConnection::IsJsCallbackObjectEquals(napi_env env,
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
