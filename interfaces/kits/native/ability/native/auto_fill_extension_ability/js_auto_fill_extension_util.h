/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_UTIL_H
#define OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_UTIL_H
#include "js_extension_common.h"
#include "native_engine/native_engine.h"
#include "session_info.h"
#include "view_data.h"

namespace OHOS {
namespace AbilityRuntime {
using Want = OHOS::AAFwk::Want;
class JsRuntime;
struct FillResponse {
    AbilityBase::ViewData viewData;
};
struct PopupSize {
    int32_t width = 0;
    int32_t height = 0;
};

enum AutoFillCommand {
    NONE,
    FILL,
    SAVE,
    UPDATE,
    RESIZE,
    INPUT,
    RELOAD_IN_MODAL
};

struct CustomData {
    AAFwk::WantParams data;
};
/**
 * @brief Js autofill extension base.
 */
class JsAutoFillExtensionUtil {
public:
    explicit JsAutoFillExtensionUtil(const std::unique_ptr<Runtime> &runtime);
    virtual ~JsAutoFillExtensionUtil();
    static napi_value WrapFillRequest(const AAFwk::Want &want, const napi_env env);
    static napi_value WrapUpdateRequest(const AAFwk::WantParams &wantParams, const napi_env env);
    static napi_value WrapViewData(const napi_env env, const AbilityBase::ViewData &viewData);
    static napi_value WrapPageNodeInfo(const napi_env env, const AbilityBase::PageNodeInfo &pageNodeInfo);
    static napi_value WrapRectData(const napi_env env, const AbilityBase::Rect &rect);
    static napi_value WrapCustomData(const napi_env env, const AAFwk::WantParams &param);
    static void UnwrapViewData(const napi_env env, const napi_value value, AbilityBase::ViewData &viewData);
    static void UnwrapPageNodeInfo(const napi_env env, const napi_value jsProValue, AbilityBase::PageNodeInfo &node);
    static void UnwrapRectData(const napi_env env, const napi_value value, AbilityBase::Rect &rect);
    static void UnwrapFillResponse(const napi_env env, const napi_value value, FillResponse &response);
    static void UnwrapPopupSize(const napi_env env, const napi_value value, PopupSize &popupSize);

    enum AutoFillResultCode {
        CALLBACK_SUCESS = 0,
        CALLBACK_FAILED,
        CALLBACK_CANCEL,
        CALLBACK_REMOVE_TIME_OUT,
        CALLBACK_FAILED_INVALID_PARAM,
    };
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_UTIL_H
