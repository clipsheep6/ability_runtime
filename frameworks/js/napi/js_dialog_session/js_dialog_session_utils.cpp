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

#include <array>

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
namespace {
constexpr size_t DIALOG_SESSION_INFO_PROPERTY_NUM = 3;
constexpr size_t DIALOG_ABILITY_INFO_PROPERTY_NUM = 7;

constexpr std::array<const char*, DIALOG_SESSION_INFO_PROPERTY_NUM> dialogSessionInfoProperty{
    "callerAbilityInfo",
    "targetAbilityInfos",
    "parameters"
};
constexpr std::array<const char*, DIALOG_ABILITY_INFO_PROPERTY_NUM> dialogAbilityInfoProperty{
    "bundleName",
    "moduleName",
    "abilityName",
    "bundleIconId",
    "bundleLabelId",
    "abilityIconId",
    "abilityLabelId"
};
}

napi_value WrapArrayDialogAbilityInfoToJS(napi_env env, const std::vector<DialogAbilityInfo> &value)
{
    napi_value jsArray = nullptr;
    napi_value jsValue = nullptr;
    uint32_t index = 0;

    NAPI_CALL(env, napi_create_array(env, &jsArray));
    for (const auto& dialogAbilityInfo : value) {
        jsValue = WrapDialogAbilityInfo(env, dialogAbilityInfo);
        if (jsValue && napi_set_element(env, jsArray, index, jsValue) == napi_ok) {
            index++;
        }
    }
    return jsArray;
}

napi_value WrapDialogSessionInfo(napi_env env, const AAFwk::DialogSessionInfo &dialogSessionInfo)
{
    napi_value jsObject = nullptr;
    napi_value jsValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    napi_value propertyJsValue[DIALOG_SESSION_INFO_PROPERTY_NUM] = {
        WrapDialogAbilityInfo(env, dialogSessionInfo.callerAbilityInfo),
        WrapArrayDialogAbilityInfoToJS(env, dialogSessionInfo.targetAbilityInfos),
        AppExecFwk::WrapWantParams(env, dialogSessionInfo.parameters)
    };

    for (size_t i = 0; i < DIALOG_SESSION_INFO_PROPERTY_NUM; i++) {
        SetPropertyValueByPropertyName(env, jsObject, dialogSessionInfoProperty[i], propertyJsValue[i]);
    }
    return jsObject;
}

napi_value WrapDialogAbilityInfo(napi_env env, const AAFwk::DialogAbilityInfo &dialogAbilityInfo)
{
    napi_value jsObject = nullptr;
    napi_value jsValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    napi_value propertyJsValue[DIALOG_ABILITY_INFO_PROPERTY_NUM] = {
        WrapStringToJS(env, dialogAbilityInfo.bundleName),
        WrapStringToJS(env, dialogAbilityInfo.moduleName),
        WrapStringToJS(env, dialogAbilityInfo.abilityName),
        WrapInt32ToJS(env, dialogAbilityInfo.bundleIconId),
        WrapInt32ToJS(env, dialogAbilityInfo.bundleLabelId),
        WrapInt32ToJS(env, dialogAbilityInfo.abilityIconId),
        WrapInt32ToJS(env, dialogAbilityInfo.abilityLabelId),
    };

    for (size_t i = 0; i < DIALOG_ABILITY_INFO_PROPERTY_NUM; i++) {
        SetPropertyValueByPropertyName(env, jsObject, dialogAbilityInfoProperty[i], propertyJsValue[i]);
    }
    return jsObject;
}
} // namespace AppExecFwk
} // nampspace OHOS
