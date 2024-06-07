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

#include "js_ui_service_host_proxy.h"
#include "ability_business_error.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "ipc_skeleton.h"
#include "napi_common_want.h"
#include "js_error_utils.h"
#include "js_ui_service_host_proxy.h"
#include "permission_constants.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AAFwk {
using namespace AbilityRuntime;

static constexpr int32_t INDEX_ZERO = 0;

napi_value JsUIServiceHostProxy::CreateJsUIServiceHostProxy(napi_env env, const sptr<IRemoteObject>& impl)
{
    TAG_LOGE(AAFwkTag::UISERVC_EXT, "JsUIServiceHostProxy::CreateJsUIServiceHostProxy");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "object is null");
        return CreateJsUndefined(env);
    }

    std::unique_ptr<JsUIServiceHostProxy> proxy = std::make_unique<JsUIServiceHostProxy>(impl);
    napi_wrap(env, object, proxy.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "JsUIServiceHostProxy";
    BindNativeFunction(env, object, "sendData", moduleName, SendData);
    return object;
}

void JsUIServiceHostProxy::Finalizer(napi_env env, void* data, void* hint)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "JsUIServiceHostProxy Finalizer");
    std::unique_ptr<JsUIServiceHostProxy>(static_cast<JsUIServiceHostProxy*>(data));
}

JsUIServiceHostProxy::JsUIServiceHostProxy(const sptr<IRemoteObject>& impl)
{
    TAG_LOGE(AAFwkTag::UISERVC_EXT, "JsUIServiceHostProxy::JsUIServiceHostProxy");
    proxy_ = std::make_unique<UIServiceHostProxy>(impl);
}

JsUIServiceHostProxy::~JsUIServiceHostProxy()
{
    TAG_LOGE(AAFwkTag::UISERVC_EXT, "JsUIServiceHostProxy::~JsUIServiceHostProxy");
}

bool JsUIServiceHostProxy::CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Current app is not system app, not allow.");
        return false;
    }
    return true;
}

napi_value JsUIServiceHostProxy::SendData(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIServiceHostProxy, OnSendData);
}

napi_value JsUIServiceHostProxy::OnSendData(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGE(AAFwkTag::UISERVC_EXT, "JsUIServiceHostProxy::OnSendData");
    if (!CheckCallerIsSystemApp()) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);        
    }
    if (proxy_ == nullptr) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    AAFwk::WantParams params;
    bool result = AppExecFwk::UnwrapWantParams(env, info.argv[INDEX_ZERO], params);
    if (!result) {
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), "Parameter verification failed");
        return CreateJsUndefined(env);
    }

    int32_t ret = proxy_->SendData(params);
    if (ret != static_cast<int32_t>(AbilityErrorCode::ERROR_OK)) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
    return CreateJsUndefined(env);
}

}
}
