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

#include "js_ability_auto_startup_manager_utils.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AbilityRuntime {
bool UnwrapAutoStartupInfo(napi_env env, napi_value param, AutoStartupInfo &info)
{
    if (!IsNormalObject(env, param)) {
        HILOG_ERROR("param is invalid.");
        return false;
    }

    if (!AppExecFwk::UnwrapStringByPropertyName(env, param, "bundleName", info.bundleName)) {
        HILOG_ERROR("Convert bundle name failed.");
        return false;
    }

    if (!AppExecFwk::UnwrapStringByPropertyName(env, param, "abilityName", info.abilityName)) {
        HILOG_ERROR("Convert ability name failed.");
        return false;
    }

    AppExecFwk::UnwrapStringByPropertyName(env, param, "moduleName", info.moduleName);
    return true;
}

bool IsNormalObject(napi_env env, napi_value value)
{
    if (value == nullptr) {
        HILOG_ERROR("value is nullptr.");
        return false;
    }
    napi_valuetype type;
    napi_typeof(env, value, &type);
    if (type == napi_undefined || type == napi_null) {
        HILOG_ERROR("value is invalid type.");
        return false;
    }
    if (type != napi_object) {
        HILOG_ERROR("Invalid type.");
        return false;
    }
    return true;
}

napi_value CreateJsAutoStartupInfoArray(napi_env env, const std::vector<AutoStartupInfo> &infoList)
{
    HILOG_DEBUG("Called.");
    napi_value arrayObj = nullptr;
    napi_create_array(env, &arrayObj);
    for (size_t i = 0; i < infoList.size(); ++i) {
        auto object = CreateJsAutoStartupInfo(env, infoList.at(i));
        if (object == nullptr) {
            HILOG_ERROR("Convert object failed.");
            return nullptr;
        }

        if (napi_set_element(env, arrayObj, i, object) != napi_ok) {
            HILOG_ERROR("Inster object to array failed.");
            return nullptr;
        }
    }

    return arrayObj;
}

napi_value CreateJsAutoStartupInfo(napi_env env, const AutoStartupInfo &info)
{
    HILOG_DEBUG("Called.");
    napi_value object = AppExecFwk::CreateJSObject(env);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr.");
        return nullptr;
    }

    napi_value bundleName = AppExecFwk::WrapStringToJS(env, info.bundleName);
    if (bundleName == nullptr) {
        HILOG_ERROR("Convert bundle name failed.");
        return nullptr;
    }

    napi_value abilityName = AppExecFwk::WrapStringToJS(env, info.abilityName);
    if (abilityName == nullptr) {
        HILOG_ERROR("Convert ability name failed.");
        return nullptr;
    }

    napi_value moduleName = AppExecFwk::WrapStringToJS(env, info.moduleName);
    if (moduleName == nullptr) {
        HILOG_ERROR("Convert module name failed.");
        return nullptr;
    }

    napi_value abilityTypeName = AppExecFwk::WrapStringToJS(env, info.abilityTypeName);
    if (abilityTypeName == nullptr) {
        HILOG_ERROR("Convert ability type name failed.");
        return nullptr;
    }

    if (!(AppExecFwk::SetPropertyValueByPropertyName(env, object, "bundleName", bundleName) &&
        AppExecFwk::SetPropertyValueByPropertyName(env, object, "abilityName", abilityName) &&
        AppExecFwk::SetPropertyValueByPropertyName(env, object, "moduleName", moduleName) &&
        AppExecFwk::SetPropertyValueByPropertyName(env, object, "abilityTypeName", abilityTypeName))) {
        HILOG_ERROR("Create js AutoStartupInfo failed.");
        return nullptr;
    }
    return object;
}
} // namespace AbilityRuntime
} // namespace OHOS