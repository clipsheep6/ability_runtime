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
#include "js_auto_fill_manager.h"

#include "ability_business_error.h"
#include "view_data.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "permission_constants.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AAFwk;
namespace {
constexpr size_t ARGC_TWO = 2;
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr const char *AUTO_FILL_TYPE_SYSTEM = "systemAutoFill";
} // namespace

void JsAutoFillManager::Finalizer(napi_env env, void *data, void *hint)
{
    HILOG_DEBUG("Called.");
    std::unique_ptr<JsAutoFillManager>(static_cast<JsAutoFillManager *>(data));
}

napi_value JsAutoFillManager::RequestAutoSave(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAutoFillManager, OnRequestAutoSave);
}

bool JsAutoFillManager::CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        HILOG_ERROR("Current app is not system app, not allow.");
        return false;
    }
    return true;
}

napi_value JsAutoFillManager::OnRequestAutoSave(napi_env env, NapiCallbackInfo &info)
{
    HILOG_DEBUG("Called.");
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string type;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], type) || type != AUTO_FILL_TYPE_SYSTEM) {
        HILOG_ERROR("Failed to parse type.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    if (!CheckCallerIsSystemApp()) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    if (jsSaveRequestCallback_ == nullptr) {
        jsSaveRequestCallback_ = std::make_shared<JsSaveRequestCallback>(env);
        if (jsSaveRequestCallback_ == nullptr) {
            HILOG_ERROR("jsSaveRequestCallback_ is nullptr.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
    }
    jsSaveRequestCallback_->Register(info.argv[INDEX_ONE]);

    // TODO Debug
    // autoFillType 
    // instanceid = Container::CurrentId()
    // auto uiContent = UIContent::GetUIContent(instanceid);
    // auto viewData = uiContent->GetViewData();
    // auto ret =
    //     AutoFillManager::GetInstance()->RequestAutoSave(autoFillType, uiContent, viewdata, jsSaveRequestCallback_);
    // if (ret != ERR_OK) {
    //     jsSaveRequestCallback_ = nullptr;
    //     HILOG_ERROR("RequestAutoSave error[%{public}d].", ret);
    //     ThrowError(env, GetJsErrorCodeByNativeError(ret));
    //     return CreateJsUndefined(env);
    // }

    return CreateJsUndefined(env);
}

napi_value JsAutoFillManagerInit(napi_env env, napi_value exportObj)
{
    HILOG_DEBUG("Called.");
    if (env == nullptr || exportObj == nullptr) {
        HILOG_ERROR("Env or exportObj nullptr.");
        return nullptr;
    }

    auto jsAbilityAutoFillManager = std::make_unique<JsAutoFillManager>();
    napi_wrap(env, exportObj, jsAbilityAutoFillManager.release(),
        JsAutoFillManager::Finalizer, nullptr, nullptr);

    HILOG_DEBUG("End.");
    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // namespace OHOS