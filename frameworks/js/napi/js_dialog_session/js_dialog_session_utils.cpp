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

#include "js_dialog_session_utils.h"

#include "hilog_wrapper.h"
#include "json/json.h"
#include "napi_common_ability.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "napi/native_api.h"
#include "napi_remote_object.h"
#include "want.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
napi_value WrapArrayDialogAbilityInfoToJS(napi_env env, const std::vector<DialogAbilityInfo> &value)
{
    napi_value jsArray = nullptr;
    napi_value jsValue = nullptr;
    uint32_t index = 0;

    NAPI_CALL(env, napi_create_array(env, &jsArray));
    for (uint32_t i = 0; i < value.size(); i++) {
        jsValue = WrapDialogAbilityInfo(env, value[i]);
        if (jsValue && napi_set_element(env, jsArray, index, jsValue) == napi_ok) {
            index++;
        }
    }
    return jsArray;
}

napi_value WrapDialogAbilityInfo(napi_env env, const AAFwk::DialogAbilityInfo &dialogAbilityInfo)
{
    napi_value jsObject = nullptr;
    napi_value jsValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    jsValue = nullptr;
    jsValue = WrapStringToJS(env, dialogAbilityInfo.bundleName);
    SetPropertyValueByPropertyName(env, jsObject, "bundleName", jsValue);
    jsValue = WrapStringToJS(env, dialogAbilityInfo.moduleName);
    SetPropertyValueByPropertyName(env, jsObject, "moduleName", jsValue);
    jsValue = WrapStringToJS(env, dialogAbilityInfo.abilityName);
    SetPropertyValueByPropertyName(env, jsObject, "abilityName", jsValue);

    jsValue = nullptr;
    jsValue = WrapInt32ToJS(env, dialogAbilityInfo.bundleIconId);
    SetPropertyValueByPropertyName(env, jsObject, "bundleIconId", jsValue);
    jsValue = WrapInt32ToJS(env, dialogAbilityInfo.bundleLabelId);
    SetPropertyValueByPropertyName(env, jsObject, "bundleLabelId", jsValue);
    jsValue = WrapInt32ToJS(env, dialogAbilityInfo.abilityIconId);
    SetPropertyValueByPropertyName(env, jsObject, "abilityIconId", jsValue);
    jsValue = WrapInt32ToJS(env, dialogAbilityInfo.abilityLabelId);
    SetPropertyValueByPropertyName(env, jsObject, "abilityLabelId", jsValue);

    return jsObject;
}

napi_value WrapDialogSessionInfo(napi_env env, const AAFwk::DialogSessionInfo &dialogSessionInfo)
{
    napi_value jsObject = nullptr;
    napi_value jsValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));
    jsValue = WrapDialogAbilityInfo(env, dialogSessionInfo.callerAbilityInfo);
    SetPropertyValueByPropertyName(env, jsObject, "callerAbilityInfo", jsValue);

    jsValue = nullptr;
    jsValue = WrapArrayDialogAbilityInfoToJS(env, dialogSessionInfo.targetAbilityInfos);
    SetPropertyValueByPropertyName(env, jsObject, "targetAbilityInfos", jsValue);

    jsValue = nullptr;
    jsValue = AppExecFwk::WrapWantParams(env, dialogSessionInfo.parameters);
    SetPropertyValueByPropertyName(env, jsObject, "parameters", jsValue);

    return jsObject;
}
} // namespace AbilityRuntime
} // nampspace OHOS
