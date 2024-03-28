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

#include "js_auto_fill_extension_context.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_extension_context.h"
#include "napi/native_api.h"
#include "napi_common_want.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AbilityRuntime {
void JsAutoFillExtensionContext::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Called.");
    std::unique_ptr<JsAutoFillExtensionContext>(static_cast<JsAutoFillExtensionContext*>(data));
}

napi_value JsAutoFillExtensionContext::CreateJsAutoFillExtensionContext(
    napi_env env, const std::shared_ptr<AutoFillExtensionContext> &context)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Called.");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context != nullptr) {
        abilityInfo = context->GetAbilityInfo();
    }
    napi_value objValue = CreateJsExtensionContext(env, context, abilityInfo);

    auto jsContext = std::make_unique<JsAutoFillExtensionContext>();
    napi_wrap(env, objValue, jsContext.release(), Finalizer, nullptr, nullptr);

    return objValue;
}
} // namespace AbilityRuntime
} // namespace OHOS
