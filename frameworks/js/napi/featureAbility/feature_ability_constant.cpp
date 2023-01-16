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
#include "feature_ability_constant.h"

#include <cstring>
#include <vector>

#include "hilog_wrapper.h"
#include "securec.h"
#include "data_ability_predicates.h"
#include "napi_common_want.h"
#include "napi_data_ability_helper.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AbilityRuntime;
/**
 * @brief FeatureAbilityConstantInit NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
NativeValue* FAConstantInitForAbilityStartSetting(NativeEngine *engine)
{
    HILOG_DEBUG("enter");
    if (engine == nullptr) {
        HILOG_ERROR("Invalid input parameters");
        return nullptr;
    }
    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }
    object->SetProperty("BOUNDS_KEY", CreateJsValue(*engine, "abilityBounds"));
    object->SetProperty("WINDOW_MODE_KEY", CreateJsValue(*engine, "windowMode"));
    object->SetProperty("DISPLAY_ID_KEY", CreateJsValue(*engine, "displayId"));

    return objValue;
}

NativeValue* FAConstantInitForAbilityWindowConfiguration(NativeEngine *engine)
{
    HILOG_DEBUG("enter");
    if (engine == nullptr) {
        HILOG_ERROR("Invalid input parameters");
        return nullptr;
    }
    const int Window_Configuration_Zero = 100;
    const int Window_Configuration_One = 101;
    const int Window_Configuration_Two = 102;

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }
    object->SetProperty("WINDOW_MODE_UNDEFINED", CreateJsValue(*engine, 0));
    object->SetProperty("WINDOW_MODE_FULLSCREEN", CreateJsValue(*engine, 1));
    object->SetProperty("WINDOW_MODE_SPLIT_PRIMARY", CreateJsValue(*engine, Window_Configuration_Zero));
    object->SetProperty("WINDOW_MODE_SPLIT_SECONDARY", CreateJsValue(*engine, Window_Configuration_One));
    object->SetProperty("WINDOW_MODE_FLOATING", CreateJsValue(*engine, Window_Configuration_Two));

    return objValue;
}

NativeValue* FAConstantInitForErrorCode(NativeEngine *engine)
{
    HILOG_DEBUG("enter");
    if (engine == nullptr) {
        HILOG_ERROR("Invalid input parameters");
        return nullptr;
    }
    const int NO_ERROR = 0;
    const int INVALID_PARAMETER = -1;
    const int ABILITY_NOT_FOUND = -2;
    const int PERMISSION_DENY = -3;

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }
    object->SetProperty("NO_ERROR", CreateJsValue(*engine, NO_ERROR));
    object->SetProperty("INVALID_PARAMETER", CreateJsValue(*engine, INVALID_PARAMETER));
    object->SetProperty("ABILITY_NOT_FOUND", CreateJsValue(*engine, ABILITY_NOT_FOUND));
    object->SetProperty("PERMISSION_DENY", CreateJsValue(*engine, PERMISSION_DENY));

    return objValue;
}

void SetNamedProperty(napi_env env, napi_value dstObj, const char *objName, const char *propName)
{
    napi_value prop = nullptr;
    napi_create_string_utf8(env, objName, NAPI_AUTO_LENGTH, &prop);
    napi_set_named_property(env, dstObj, propName, prop);
}

void SetNamedProperty(napi_env env, napi_value dstObj, const int32_t objValue, const char *propName)
{
    napi_value prop = nullptr;
    napi_create_int32(env, objValue, &prop);
    napi_set_named_property(env, dstObj, propName, prop);
}
}  // namespace AppExecFwk
}  // namespace OHOS
