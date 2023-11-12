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

#ifndef OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_BASE_H
#define OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_BASE_H
#include "js_extension_common.h"
#include "native_engine/native_engine.h"
#include "session_info.h"
#include "view_data.h"

namespace OHOS {
namespace AbilityRuntime {
using Want = OHOS::AAFwk::Want;
class JsRuntime;
/**
 * @brief Js autofill extension base.
 */
class JsAutoFillExtensionBase {
public:
    explicit JsAutoFillExtensionBase(const std::unique_ptr<Runtime> &runtime);
    virtual ~JsAutoFillExtensionBase();
    static napi_value WrapViewData(const napi_env env, std::unique_ptr<AbilityBase::ViewData> viewData);
    static napi_value WrapPageNodeInfo(const napi_env env, const AbilityBase::PageNodeInfo &pageNodeInfo);
    static std::string UnwrapViewData(const napi_env env, const napi_value value);
    static void UnwrapPageNodeInfo(const napi_env env, const napi_value jsProValue, AbilityBase::PageNodeInfo &node);

    enum AutoFillResultCode {
        CALLBACK_SUCESS = 0,
        CALLBACK_FAILED,
        CALLBACK_CANCEL,
        CALLBACK_FAILED_INVALID_PARAM,
    };
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_BASE_H
