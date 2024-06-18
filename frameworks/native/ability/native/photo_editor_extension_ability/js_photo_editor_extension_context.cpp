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

#include "js_photo_editor_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_extension_context.h"
#include "napi/native_api.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "pixel_map_napi.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_TWO = 2;
constexpr const char *ERR_MSG_PARAMS_ERROR = "Params error";
constexpr const char *ERR_MSG_INTERNAL_ERROR = "Internal error";
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
} // namespace

void JsPhotoEditorExtensionContext::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtensionContext finalizer is called");
    std::unique_ptr<JsPhotoEditorExtensionContext>(static_cast<JsPhotoEditorExtensionContext *>(data));
}

napi_value JsPhotoEditorExtensionContext::CreateJsPhotoEditorExtensionContext(
    napi_env env, std::shared_ptr<PhotoEditorExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "CreateJsPhotoEditorExtensionContext begin.");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }

    napi_value objValue = JsUIExtensionContext::CreateJsUIExtensionContext(env, context);
    std::unique_ptr<JsPhotoEditorExtensionContext> jsContext = std::make_unique<JsPhotoEditorExtensionContext>(context);
    napi_status status = napi_wrap(env, objValue, jsContext.release(), Finalizer, nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to do napi wrap.");
    }

    const char *moduleName = "JsPhotoEditorExtensionContext";
    BindNativeFunction(env, objValue, "saveEditedContentWithUri", moduleName, SaveEditedContentWithUri);
    BindNativeFunction(env, objValue, "saveEditedContentWithImage", moduleName, SaveEditedContentWithImage);

    return objValue;
}

napi_value JsPhotoEditorExtensionContext::SaveEditedContentWithUri(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "SaveEditedContentWithUri is called.");
    GET_NAPI_INFO_AND_CALL(env, info, JsPhotoEditorExtensionContext, OnSaveEditedContentWithUri);
}

napi_value JsPhotoEditorExtensionContext::SaveEditedContentWithImage(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "SaveEditedContentWithImage is called.");
    GET_NAPI_INFO_AND_CALL(env, info, JsPhotoEditorExtensionContext, OnSaveEditedContentWithImage);
}

napi_value JsPhotoEditorExtensionContext::OnSaveEditedContentWithUri(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnSaveEditedContentWithUri is called: param size: %{public}d.",
             static_cast<int32_t>(info.argc));

    if (info.argc != ARGC_TWO) {
        ThrowError(env, static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_PARAM_ERROR), ERR_MSG_PARAMS_ERROR);
        return CreateJsUndefined(env);
    }

    std::string uri = AppExecFwk::UnwrapStringFromJS(env, info.argv[INDEX_ZERO]);
    TAG_LOGD(AAFwkTag::UI_EXT, "Uri: %{public}s.", uri.c_str());

    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Context is released.");
        ThrowError(env, static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_INTERNAL_ERROR), ERR_MSG_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete = [weak = context_, uri](napi_env env, NapiAsyncTask &task,
                                                                      int32_t status) {
        TAG_LOGD(AAFwkTag::UI_EXT, "OnSaveEditedContentWithUri begin.");
        auto context = weak.lock();
        if (!context) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Context is released.");
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_INTERNAL_ERROR)));
            return;
        }

        AAFwk::Want newWant;
        PhotoEditorErrorCode errCode = context->SaveEditedContent(uri, newWant);
        napi_value abilityResult = AppExecFwk::WrapAbilityResult(env, static_cast<int>(errCode), newWant);
        if (abilityResult == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Wrap abilityResult failed.");
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_INTERNAL_ERROR)));
            return;
        }

        task.Resolve(env, abilityResult);
    };

    napi_value lastParam = (info.argc > INDEX_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsPhotoEditorExtensionContext OnSaveEditedContentWithUri", env,
                                   CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsPhotoEditorExtensionContext::OnSaveEditedContentWithImage(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnSaveEditedContentWithImage is called: param size: %{public}d.",
        static_cast<int32_t>(info.argc));

    auto image = Media::PixelMapNapi::GetPixelMap(env, info.argv[INDEX_ZERO]);
    if (!image) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Get edited image fail.");
        ThrowError(env, static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_PARAM_ERROR), ERR_MSG_PARAMS_ERROR);
        return CreateJsUndefined(env);
    }

    Media::PackOption packOption;
    if (!UnwrapPackOption(env, info.argv[INDEX_ONE], packOption)) {
        return CreateJsUndefined(env);
    }

    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Context is released.");
        ThrowError(env, static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_INTERNAL_ERROR), ERR_MSG_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete = [weak = context_, image, packOption = std::move(packOption)](
                                                   napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGD(AAFwkTag::UI_EXT, "OnSaveEditedContentWithImage begin.");
        auto context = weak.lock();
        if (!context) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Context is released.");
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_INTERNAL_ERROR)));
            return;
        }

        AAFwk::Want newWant;
        PhotoEditorErrorCode errCode = context->SaveEditedContent(image, packOption, newWant);
        napi_value abilityResult = AppExecFwk::WrapAbilityResult(env, static_cast<int>(errCode), newWant);
        if (abilityResult == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Wrap abilityResult failed.");
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_INTERNAL_ERROR)));
            return;
        }

        task.Resolve(env, abilityResult);
    };

    napi_value lastParam = nullptr;
    if (AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_TWO], napi_function)) {
        lastParam = info.argv[INDEX_TWO];
    }
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsPhotoEditorExtensionContext OnSaveEditedContentWithImage", env,
                                   CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

bool JsPhotoEditorExtensionContext::UnwrapPackOption(napi_env env, napi_value jsOption, Media::PackOption &packOption)
{
    if (!AppExecFwk::IsTypeForNapiValue(env, jsOption, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Packing option error, not object");
        ThrowError(env, static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_PARAM_ERROR), ERR_MSG_PARAMS_ERROR);
        return false;
    }

    napi_value jsFormat = AppExecFwk::GetPropertyValueByPropertyName(env, jsOption, "format", napi_string);
    std::string format = AppExecFwk::UnwrapStringFromJS(env, jsFormat, "");
    if (format == "") {
        ThrowError(env, static_cast<int32_t>(PhotoEditorErrorCode::ERROR_CODE_PARAM_ERROR), ERR_MSG_PARAMS_ERROR);
        return false;
    }
    napi_value jsQuality = AppExecFwk::GetPropertyValueByPropertyName(env, jsOption, "quality", napi_number);
    int quality = AppExecFwk::UnwrapInt32FromJS(env, jsQuality, 100);
    TAG_LOGD(AAFwkTag::UI_EXT, "Unwrap pack option result, format=%{public}s, quality=%{public}d.", format.c_str(),
             quality);
    packOption.format = format;
    packOption.quality = static_cast<uint8_t>(quality);
    return true;
}

} // namespace AbilityRuntime
} // namespace OHOS