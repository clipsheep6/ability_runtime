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

#include "js_ui_service_proxy.h"
#include "hilog_tag_wrapper.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AAFwk {
using namespace AbilityRuntime;

static constexpr int32_t INDEX_ZERO = 0;

napi_value JsUIServiceProxy::CreateJsUIServiceProxy(napi_env env, const sptr<IRemoteObject>& impl)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "JsUIServiceProxy::CreateJsUIServiceProxy");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "object is null");
        return CreateJsUndefined(env);
    }

    std::unique_ptr<JsUIServiceProxy> proxy = std::make_unique<JsUIServiceProxy>(impl);
    napi_wrap(env, object, proxy.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "UIServiceProxy";
    BindNativeFunction(env, object, "sendData", moduleName, SendData);
    return object;
}

void JsUIServiceProxy::Finalizer(napi_env env, void* data, void* hint)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "JsUIServiceProxy::Finalizer");
    std::unique_ptr<JsUIServiceProxy>(static_cast<JsUIServiceProxy*>(data));
}

JsUIServiceProxy::JsUIServiceProxy(const sptr<IRemoteObject>& impl)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "JsUIServiceProxy::JsUIServiceProxy");
    proxy_ = std::make_unique<UIServiceProxy>(impl);
}

JsUIServiceProxy::~JsUIServiceProxy()
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "JsUIServiceProxy::~JsUIServiceProxy");
}

napi_value JsUIServiceProxy::SendData(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIServiceProxy, OnSendData);
}

napi_value JsUIServiceProxy::OnSendData(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "JsUIServiceProxy::OnSendData");
    if (proxy_ == nullptr) {
        return CreateJsUndefined(env);
    }

    AAFwk::WantParams params;
    bool result = AppExecFwk::UnwrapWantParams(env, info.argv[INDEX_ZERO], params);
    if (result) {
        proxy_->SendData(params);
    }
    return CreateJsUndefined(env);
}

}
}
