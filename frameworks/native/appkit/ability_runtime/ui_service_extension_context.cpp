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

#include "ui_service_extension_context.h"

#include "ability_connection.h"
#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include <native_engine/native_engine.h>
#include "ui_content.h"
#include "connection_manager.h"
#include "hilog_wrapper.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t UIServiceExtensionContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("UIServiceExtensionContext"));
const std::string UIEXTENSION_TARGET_TYPE_KEY = "ability.want.params.uiExtensionTargetType";
const std::string FLAG_AUTH_READ_URI_PERMISSION = "ability.want.params.uriPermissionFlag";

int32_t UIServiceExtensionContext::ILLEGAL_REQUEST_CODE(-1);

ErrCode UIServiceExtensionContext::StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "Start ability begin, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_,
        ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "UIServiceExtensionContext::StartAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode UIServiceExtensionContext::TerminateSelf()
{
    TAG_LOGI(AAFwkTag::APPKIT, "begin.");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "UIServiceExtensionContext::TerminateAbility is failed %{public}d", err);
    }
    TAG_LOGI(AAFwkTag::APPKIT, "%{public}s end.", __func__);
    return err;
}

sptr<Rosen::Window> UIServiceExtensionContext::GetWindow()
{
    return window_;
}

Ace::UIContent* UIServiceExtensionContext::GetUIContent()
{
    TAG_LOGI(AAFwkTag::UI_EXT, "called");
    if (window_ == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "window_ is nullptr");
        return nullptr;
    }
    return window_->GetUIContent();
}

ErrCode UIServiceExtensionContext::StartAbilityByType(const std::string &type,
    AAFwk::WantParams &wantParam, const std::shared_ptr<JsUIExtensionCallback> &uiExtensionCallbacks)
{
    TAG_LOGD(AAFwkTag::APPKIT, "StartAbilityByType begin.");
    if (uiExtensionCallbacks == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "uiExtensionCallbacks is nullptr.");
        return ERR_INVALID_VALUE;
    }
    auto uiContent = GetUIContent();
    if (uiContent == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "uiContent is nullptr.");
        return ERR_INVALID_VALUE;
    }
    wantParam.SetParam(UIEXTENSION_TARGET_TYPE_KEY, AAFwk::String::Box(type));
    AAFwk::Want want;
    want.SetParams(wantParam);
    if (wantParam.HasParam(FLAG_AUTH_READ_URI_PERMISSION)) {
        int32_t flag = wantParam.GetIntParam(FLAG_AUTH_READ_URI_PERMISSION, 0);
        want.SetFlags(flag);
        wantParam.Remove(FLAG_AUTH_READ_URI_PERMISSION);
    }
    Ace::ModalUIExtensionCallbacks callback;
    Ace::ModalUIExtensionConfig config;
    SetModalUIExtensionCallbackFunc(callback, uiExtensionCallbacks);
    int32_t sessionId = uiContent->CreateModalUIExtension(want, callback, config);
    if (sessionId == 0) {
        TAG_LOGD(AAFwkTag::APPKIT, "CreateModalUIExtension is failed");
        return ERR_INVALID_VALUE;
    }
    uiExtensionCallbacks->SetUIContent(uiContent);
    uiExtensionCallbacks->SetSessionId(sessionId);
    return ERR_OK;
}

void UIServiceExtensionContext::SetModalUIExtensionCallbackFunc(Ace::ModalUIExtensionCallbacks &callback,
    const std::shared_ptr<JsUIExtensionCallback> &uiExtensionCallbacks)
{
    callback.onError = [uiExtensionCallbacks](int32_t number) {
        if (uiExtensionCallbacks->env_ == nullptr) {
            return;
        }
        std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
            ([uiExtensionCallbacks, number](napi_env env, NapiAsyncTask &task, int32_t status) {
                uiExtensionCallbacks->CallJsError(number);
            });
        napi_ref innerCallback = nullptr;
        std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
        NapiAsyncTask::Schedule("JsUIExtensionCallback::OnError:", uiExtensionCallbacks->env_,
            std::make_unique<NapiAsyncTask>(innerCallback, std::move(execute), std::move(complete)));
        if (uiExtensionCallbacks->uiContent_ == nullptr) {
            return;
        }
        uiExtensionCallbacks->uiContent_->CloseModalUIExtension(uiExtensionCallbacks->sessionId_);
    }
    callback.onRelease = [uiExtensionCallbacks](int32_t code) {
        if (uiExtensionCallbacks->uiContent_ == nullptr) {
            return;
        }
        uiExtensionCallbacks->uiContent_->CloseModalUIExtension(uiExtensionCallbacks->sessionId_);
    }
    callback.onResult = [uiExtensionCallbacks](int32_t resultCode, const AAFwk::Want &want) {
        if (uiExtensionCallbacks->env_ == nullptr) {
            return;
        }
        std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
            ([uiExtensionCallbacks, resultCode, want](napi_env env, NapiAsyncTask &task, int32_t status) {
                    uiExtensionCallbacks->CallJsResult(resultCode, want);
            });
        napi_ref innerCallback = nullptr;
        std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
        NapiAsyncTask::Schedule("JsUIExtensionCallback::OnResult:", uiExtensionCallbacks->env_,
            std::make_unique<NapiAsyncTask>(innerCallback, std::move(execute), std::move(complete)));
        if (uiExtensionCallbacks->uiContent_ == nullptr) {
            return;
        }
        uiExtensionCallbacks->uiContent_->CloseModalUIExtension(uiExtensionCallbacks->sessionId_);
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS
