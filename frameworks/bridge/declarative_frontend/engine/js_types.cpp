/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "frameworks/bridge/declarative_frontend/engine/js_types.h"

namespace OHOS::Ace::Framework {

#ifdef USE_ARK_ENGINE
Local<JSValueRef> JsStopPropagation(panda::JsiRuntimeCallInfo *info)
{
    Local<JSValueRef> thisObj = info->GetThisRef();
    auto eventInfo = static_cast<BaseEventInfo*>(panda::Local<panda::ObjectRef>(thisObj)->GetNativePointerField(0));
    if (eventInfo) {
        LOGD("JsStopPropagation is trigger");
        eventInfo->SetStopPropagation(true);
    }
    return JSValueRef::Undefined(info->GetVM());
}
#endif

} // namespace OHOS::Ace::Framework
