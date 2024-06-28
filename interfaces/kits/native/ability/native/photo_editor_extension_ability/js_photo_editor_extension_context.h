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

#ifndef OHOS_ABILITY_RUNTIME_JS_PHOTO_EDITOR_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_JS_PHOTO_EDITOR_EXTENSION_CONTEXT_H

#include "photo_editor_extension_context.h"
#include "native_engine/native_engine.h"
#include "image_packer.h"

namespace OHOS {
namespace AbilityRuntime {
struct NapiCallbackInfo;

class JsPhotoEditorExtensionContext {
public:
    explicit JsPhotoEditorExtensionContext(const std::shared_ptr<PhotoEditorExtensionContext> &context)
        : context_(context)
    {}

    virtual ~JsPhotoEditorExtensionContext() = default;

    static void Finalizer(napi_env env, void *data, void *hint);

    static napi_value CreateJsPhotoEditorExtensionContext(napi_env env,
                                                          std::shared_ptr<PhotoEditorExtensionContext> context);

    static napi_value SaveEditedContentWithUri(napi_env env, napi_callback_info info);

    static napi_value SaveEditedContentWithImage(napi_env env, napi_callback_info info);

private:
    napi_value OnSaveEditedContentWithUri(napi_env env, NapiCallbackInfo &info);

    napi_value OnSaveEditedContentWithImage(napi_env env, NapiCallbackInfo &info);

    bool UnwrapPackOption(napi_env env, napi_value jsOption, Media::PackOption &packOption);

private:
    std::weak_ptr<PhotoEditorExtensionContext> context_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_PHOTO_EDITOR_EXTENSION_CONTEXT_H