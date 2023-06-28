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

#ifndef OHOS_ABILITY_RUNTIME_JS_API_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_API_UTILS_H

#include "want.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace JsApiUtils {
bool UnWrapAbilityResult(NativeEngine &engine, NativeValue* argv, int &resultCode, AAFwk::Want &want);
bool UnWrapWant(NativeEngine &engine, NativeValue* param, AAFwk::Want &want);
bool UnwrapWantParams(NativeEngine &engine, NativeValue* param, AAFwk::WantParams &wantParams);

template<typename numtype>
bool UnwrapNumberValue(NativeValue* param, numtype &value)
{
    if (param == nullptr) {
        return false;
    }
    if (param->TypeOf() != NativeValueType::NATIVE_NUMBER) {
        return false;
    }
    value = *ConvertNativeValueTo<NativeNumber>(param);
    return true;
}


bool UnwrapStringValue(NativeValue* param, std::string &value);
bool UnwrapArrayStringValue(NativeValue* param, std::vector<std::string> &value);
bool IsNarmalObject(NativeValue* value);

void HandleTooFewParam(NativeEngine &engine, AsyncTask::CompleteCallback &completeCb);
void HandleInvalidParam(NativeEngine &engine, AsyncTask::CompleteCallback &completeCb, const std::string &paramName);
void HandleTaskResolve(NativeEngine &engine, AsyncTask &task, NativeValue* value = nullptr);
void HandleTaskReject(NativeEngine &engine, AsyncTask &task, int errcode);
}
}  // namespace AbilityRuntime
}  // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_API_UTILS_H