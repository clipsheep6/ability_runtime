/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "frameworks/bridge/declarative_frontend/engine/functions/js_webview_function.h"

#include "base/log/log.h"

namespace OHOS::Ace::Framework {
void JsWebViewFunction::Execute(const std::string& result)
{
    LOGD("JsWebViewFunction execute result[%{public}s]", result.c_str());
    JSRef<JSVal> params[1];
    params[0] = JSRef<JSVal>::Make(ToJSValue(result));
    JsFunction::ExecuteJS(1, params);
}
} // namespace OHOS::Ace::Framework
