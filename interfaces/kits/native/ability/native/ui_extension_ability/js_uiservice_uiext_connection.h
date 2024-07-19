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
#ifndef OHOS_ABILITY_RUNTIME_UISERVICE_UIEXT_CONNECTION_H
#define OHOS_ABILITY_RUNTIME_UISERVICE_UIEXT_CONNECTION_H

#include "js_ui_extension_context.h"

#include "ui_service_host_stub.h"

namespace OHOS {
namespace AbilityRuntime {
namespace UIServiceConnection {
void AddUIServiceExtensionConnection(AAFwk::Want& want, sptr<JSUIServiceUIExtConnection>& connection);
void RemoveUIServiceExtensionConnection(const int64_t& connectId);
void FindUIServiceExtensionConnection(const int64_t& connectId, AAFwk::Want& want,
    sptr<JSUIServiceUIExtConnection>& connection);
void FindUIServiceExtensionConnection(napi_env env, AAFwk::Want& want, napi_value callback,
    sptr<JSUIServiceUIExtConnection>& connection);
}

class UIExtensionServiceHostStubImpl;
class JSUIServiceUIExtConnection : public JSUIExtensionConnection {
public:
    JSUIServiceUIExtConnection(napi_env env);
    ~JSUIServiceUIExtConnection();
    virtual void HandleOnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    virtual void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    sptr<UIExtensionServiceHostStubImpl> GetServiceHostStub() { return serviceHostStub_; }
    void SetProxyObject(napi_value proxy);
    napi_value GetProxyObject();
    void SetNapiAsyncTask(std::shared_ptr<NapiAsyncTask>& task);
    void AddDuplicatedPendingTask(std::unique_ptr<NapiAsyncTask>& task);
    void ResolveDuplicatedPendingTask(napi_env env, napi_value proxy);
    void RejectDuplicatedPendingTask(napi_env env, napi_value error);
    int32_t OnSendData(OHOS::AAFwk::WantParams &data);
    void HandleOnSendData(const OHOS::AAFwk::WantParams &data);
    void CallJsOnDisconnect();
    static bool IsJsCallbackObjectEquals(napi_env env, std::unique_ptr<NativeReference>& callback, napi_value value);

private:
    sptr<UIExtensionServiceHostStubImpl> serviceHostStub_;
    std::unique_ptr<NativeReference> serviceProxyObject_;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask_;
    std::vector<std::unique_ptr<NapiAsyncTask>> duplicatedPendingTaskList_;
};

}
}
#endif
