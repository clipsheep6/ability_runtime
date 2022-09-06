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

#include "napi_want.h"
#include "want.h"
#include "hilog_wrapper.h"

namespace OHOS {

napi_value JS_Constructor(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    AAFwk::Want* want = new AAFwk::Want();

    napi_wrap(
        env, thisVar, want,
        [](napi_env env, void* data, void* hint) {
            AAFwk::Want* want = (AAFwk::Want*)data;
            delete want;
        },
        nullptr, nullptr);

    return thisVar;
}
   
napi_value WantInit(napi_env env, napi_value exports)
{
    HILOG_INFO("napi_moudule Init start...");
    napi_property_descriptor desc[] = {
    };

    napi_value want = nullptr;
    napi_define_class(env, 
        "want", 
        NAPI_AUTO_LENGTH,
        JS_Constructor,
        nullptr,
        0,
        nullptr,
        &want);
    napi_set_named_property(env, exports, "want", want);

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    HILOG_INFO("napi_moudule Init end...");
    return exports;
}
}