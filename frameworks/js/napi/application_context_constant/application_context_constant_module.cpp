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
#include "napi/native_api.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AAFwk {
static napi_status SetEnumItem(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_status status;
    napi_value itemName;
    napi_value itemValue;

    NAPI_CALL_BASE(env, status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &itemName), status);
    NAPI_CALL_BASE(env, status = napi_create_int32(env, value, &itemValue), status);

    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemName, itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemValue, itemName), status);

    return napi_ok;
}

static napi_value InitAreaModeObject(napi_env env)
{
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "EL1", 0));
    NAPI_CALL(env, SetEnumItem(env, object, "EL2", 1));

    return object;
}

/*
 * The module initialization.
 */
static napi_value ApplicationContextConstantInit(napi_env env, napi_value exports)
{
    napi_value areaMode = InitAreaModeObject(env);
    NAPI_ASSERT(env, areaMode != nullptr, "failed to create AreaMode object");

    napi_property_descriptor exportObjs[] = {
        DECLARE_NAPI_PROPERTY("AreaMode", areaMode),
    };

    napi_status status = napi_define_properties(env, exports, sizeof(exportObjs) / sizeof(exportObjs[0]), exportObjs);
    NAPI_ASSERT(env, status == napi_ok, "failed to define properties for exports");

    return exports;
}

/*
 * The module definition.
 */
static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ApplicationContextConstantInit,
    .nm_modname = "app.ability.contextConstant",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

/*
 * The module registration.
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
}  // namespace AAFwk
}  // namespace OHOS
