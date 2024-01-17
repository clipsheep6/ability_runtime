/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "js_ability_manager_utils.h"

#include <cstdint>

#include "ability_state.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value CreateJSToken(napi_env env, const sptr<IRemoteObject> target)
{
    napi_value tokenClass = nullptr;
    auto constructorcb = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar = nullptr;
        napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
        return thisVar;
    };
    napi_define_class(
        env, "TokenClass", NAPI_AUTO_LENGTH, constructorcb, nullptr, 0, nullptr, &tokenClass);
    napi_value jsToken = nullptr;
    napi_new_instance(env, tokenClass, 0, nullptr, &jsToken);
    auto finalizecb = [](napi_env env, void *data, void *hint) {};
    napi_wrap(env, jsToken, static_cast<void *>(target.GetRefPtr()), finalizecb, nullptr, nullptr);
    return jsToken;
}

napi_value CreateJsAbilityRunningInfoArray(napi_env env, const std::vector<AAFwk::AbilityRunningInfo> &infos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, infos.size(), &arrayValue);
    uint32_t index = 0;
    for (const auto &runningInfo : infos) {
        napi_set_element(env, arrayValue, index++, CreateJsAbilityRunningInfo(env, runningInfo));
    }
    return arrayValue;
}

napi_value CreateJsElementName(napi_env env, const AppExecFwk::ElementName &elementName)
{
    return OHOS::AppExecFwk::WrapElementName(env, elementName);
}

napi_value CreateJsExtensionRunningInfoArray(napi_env env, const std::vector<AAFwk::ExtensionRunningInfo> &infos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, infos.size(), &arrayValue);
    uint32_t index = 0;
    for (const auto &runningInfo : infos) {
        napi_set_element(env, arrayValue, index++, CreateJsExtensionRunningInfo(env, runningInfo));
    }
    return arrayValue;
}

napi_value CreateJsAbilityRunningInfo(napi_env env, const AAFwk::AbilityRunningInfo &info)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env, info.ability);
    napi_set_named_property(env, objValue, "ability", napiElementName);
    napi_set_named_property(env, objValue, "pid", CreateJsValue(env, info.pid));
    napi_set_named_property(env, objValue, "uid", CreateJsValue(env, info.uid));
    napi_set_named_property(env, objValue, "processName", CreateJsValue(env, info.processName));
    napi_set_named_property(env, objValue, "startTime", CreateJsValue(env, info.startTime));
    napi_set_named_property(env, objValue, "abilityState", CreateJsValue(env, info.abilityState));
    return objValue;
}

napi_value CreateJsExtensionRunningInfo(napi_env env, const AAFwk::ExtensionRunningInfo &info)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env, info.extension);
    napi_set_named_property(env, objValue, "extension", napiElementName);
    napi_set_named_property(env, objValue, "pid", CreateJsValue(env, info.pid));
    napi_set_named_property(env, objValue, "uid", CreateJsValue(env, info.uid));
    napi_set_named_property(env, objValue, "type", CreateJsValue(env, info.type));
    napi_set_named_property(env, objValue, "processName", CreateJsValue(env, info.processName));
    napi_set_named_property(env, objValue, "startTime", CreateJsValue(env, info.startTime));
    napi_set_named_property(env, objValue, "clientPackage", CreateNativeArray(env, info.clientPackage));
    return objValue;
}

napi_value AbilityStateInit(napi_env env)
{
    HILOG_DEBUG("enter");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "INITIAL", CreateJsValue(env, AAFwk::AbilityState::INITIAL));
    napi_set_named_property(env, objValue, "FOCUS", CreateJsValue(env, AAFwk::AbilityState::ACTIVE));
    napi_set_named_property(env, objValue, "FOREGROUND", CreateJsValue(env, AAFwk::AbilityState::FOREGROUND));
    napi_set_named_property(env, objValue, "BACKGROUND", CreateJsValue(env, AAFwk::AbilityState::BACKGROUND));
    napi_set_named_property(env, objValue, "FOREGROUNDING", CreateJsValue(env, AAFwk::AbilityState::FOREGROUNDING));
    napi_set_named_property(env, objValue, "BACKGROUNDING", CreateJsValue(env, AAFwk::AbilityState::BACKGROUNDING));
    return objValue;
}

napi_value CreateJsAbilityStateData(napi_env env, const AbilityStateData &abilityStateData)
{
    HILOG_DEBUG("Called.");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("ObjValue nullptr.");
        return nullptr;
    }
    napi_set_named_property(env, object, "bundleName", CreateJsValue(env, abilityStateData.bundleName));
    napi_set_named_property(env, object, "moduleName", CreateJsValue(env, abilityStateData.moduleName));
    napi_set_named_property(env, object, "abilityName", CreateJsValue(env, abilityStateData.abilityName));
    napi_set_named_property(env, object, "pid", CreateJsValue(env, abilityStateData.pid));
    napi_set_named_property(env, object, "uid", CreateJsValue(env, abilityStateData.uid));
    napi_set_named_property(env, object, "state", CreateJsValue(env, abilityStateData.abilityState));
    napi_set_named_property(env, object, "abilityType", CreateJsValue(env, abilityStateData.abilityType));
    return object;
}

napi_value CreateJsAbilityStateDataArray(
    napi_env env, const std::vector<AppExecFwk::AbilityStateData> &abilityStateDatas)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, abilityStateDatas.size(), &arrayValue);
    uint32_t index = 0;
    for (const auto &abilityStateData : abilityStateDatas) {
        napi_set_element(env, arrayValue, index++, CreateJsAbilityStateData(env, abilityStateData));
    }
    return arrayValue;
}
} // namespace AbilityRuntime
} // namespace OHOS
