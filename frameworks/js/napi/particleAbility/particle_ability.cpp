/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "particle_ability.h"

#include <cstring>
#include <uv.h>
#include <vector>

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "napi_common_ability.h"
#include "napi/native_api.h"
#include "securec.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Obtains the type of this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAGetAppType(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetAppTypeCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief Obtains information about the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAGetAbilityInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetAbilityInfoCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief Obtains the HapModuleInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAGetHapModuleInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetHapModuleInfoCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief Get context.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAGetContext(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetContextCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief Get want.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAGetWant(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetWantCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief Obtains the class name in this ability name, without the prefixed bundle name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAGetAbilityName(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetAbilityNameCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief ParticleAbility NAPI method : stopAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAStopAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_StopAbilityCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief FeatureAbility NAPI method : acquireDataAbilityHelper.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAAcquireDataAbilityHelper(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    return NAPI_AcquireDataAbilityHelperCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief ParticleAbility NAPI method : startBackgroundRunning.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAStartBackgroundRunning(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    return NAPI_StartBackgroundRunningCommon(env, info);
}

/**
 * @brief ParticleAbility NAPI method : cancelBackgroundRunning.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PACancelBackgroundRunning(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    return NAPI_CancelBackgroundRunningCommon(env, info);
}

/**
 * @brief ParticleAbility NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value ParticleAbilityInit(napi_env env, napi_value exports)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getAppType", NAPI_PAGetAppType),
        DECLARE_NAPI_FUNCTION("getAbilityInfo", NAPI_PAGetAbilityInfo),
        DECLARE_NAPI_FUNCTION("getHapModuleInfo", NAPI_PAGetHapModuleInfo),
        DECLARE_NAPI_FUNCTION("getContext", NAPI_PAGetContext),
        DECLARE_NAPI_FUNCTION("getWant", NAPI_PAGetWant),
        DECLARE_NAPI_FUNCTION("getAbilityName", NAPI_PAGetAbilityName),
        DECLARE_NAPI_FUNCTION("stopAbility", NAPI_PAStopAbility),
        DECLARE_NAPI_FUNCTION("startBackgroundRunning", NAPI_PAStartBackgroundRunning),
        DECLARE_NAPI_FUNCTION("cancelBackgroundRunning", NAPI_PACancelBackgroundRunning),
    };
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);

    return reinterpret_cast<napi_value>(JsParticleAbilityInit(reinterpret_cast<NativeEngine*>(env),
        reinterpret_cast<NativeValue*>(exports)));
}

void JsParticleAbility::Finalizer(NativeEngine *engine, void *data, void *hint)
{
    HILOG_INFO("JsWantAgent::Finalizer is called");
    std::unique_ptr<JsParticleAbility>(static_cast<JsParticleAbility*>(data));
}

NativeValue* JsParticleAbility::PAConnectAbility(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsParticleAbility *me = CheckParamsAndGetThis<JsParticleAbility>(engine, info);
    return (me != nullptr) ? me->JsConnectAbility(*engine, *info, AbilityType::UNKNOWN) : nullptr;
}

NativeValue* JsParticleAbility::PADisConnectAbility(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsParticleAbility *me = CheckParamsAndGetThis<JsParticleAbility>(engine, info);
    return (me != nullptr) ? me->JsDisConnectAbility(*engine, *info, AbilityType::UNKNOWN) : nullptr;
}

NativeValue* JsParticleAbility::PAStartAbility(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsParticleAbility *me = CheckParamsAndGetThis<JsParticleAbility>(engine, info);
    return (me != nullptr) ? me->JsStartAbility(*engine, *info, AbilityType::UNKNOWN) : nullptr;
}

NativeValue* JsParticleAbility::PATerminateAbility(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsParticleAbility *me = CheckParamsAndGetThis<JsParticleAbility>(engine, info);
    return (me != nullptr) ? me->JsTerminateAbility(*engine, *info) : nullptr;
}

NativeValue* JsParticleAbility::AcquireDataAbilityHelper(NativeEngine *engine, NativeCallbackInfo *info)
{
    auto *me = CheckParamsAndGetThis<JsParticleAbility>(engine, info);
    return (me != nullptr) ? me->JsAcquireDataAbility(*engine, *info, AbilityType::UNKNOWN) : nullptr;
}

Ability* JsParticleAbility::GetAbility(napi_env env)
{
    napi_status ret;
    napi_value global = nullptr;
    const napi_extended_error_info *errorInfo = nullptr;
    ret = napi_get_global(env, &global);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        HILOG_ERROR("JsParticleAbility::GetAbility, get_global=%{public}d err:%{public}s",
            ret, errorInfo->error_message);
        return nullptr;
    }
    napi_value abilityObj = nullptr;
    ret = napi_get_named_property(env, global, "ability", &abilityObj);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        HILOG_ERROR("JsParticleAbility::GetAbility, get_named_property=%{public}d err:%{public}s",
            ret, errorInfo->error_message);
        return nullptr;
    }
    Ability* ability = nullptr;
    ret = napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability));
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        HILOG_ERROR("JsParticleAbility::GetAbility, get_value_external=%{public}d err:%{public}s",
            ret, errorInfo->error_message);
        return nullptr;
    }
    return ability;
}

NativeValue* JsParticleAbilityInit(NativeEngine *engine, NativeValue *exportObj)
{
    HILOG_DEBUG("JsParticleAbility is called");

    if (engine == nullptr || exportObj == nullptr) {
        HILOG_ERROR("engine or exportObj null");
        return nullptr;
    }

    NativeObject *object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_ERROR("object null");
        return nullptr;
    }

    std::unique_ptr<JsParticleAbility> jsParticleAbility = std::make_unique<JsParticleAbility>();
    jsParticleAbility->ability_ = jsParticleAbility->GetAbility(reinterpret_cast<napi_env>(engine));
    object->SetNativePointer(jsParticleAbility.release(), JsParticleAbility::Finalizer, nullptr);

    HILOG_DEBUG("JsParticleAbility BindNativeFunction called");
    const char *moduleName = "JsParticleAbility";
    BindNativeFunction(*engine, *object, "connectAbility", moduleName, JsParticleAbility::PAConnectAbility);
    BindNativeFunction(*engine, *object, "disConnectAbility", moduleName, JsParticleAbility::PADisConnectAbility);
    BindNativeFunction(*engine, *object, "startAbility", moduleName, JsParticleAbility::PAStartAbility);
    BindNativeFunction(*engine, *object, "terminateSelf", moduleName, JsParticleAbility::PATerminateAbility);
    BindNativeFunction(*engine, *object, "acquireDataAbilityHelper",
        moduleName, JsParticleAbility::AcquireDataAbilityHelper);

    HILOG_DEBUG("JsParticleAbility end");
    return exportObj;
}
}  // namespace AppExecFwk
}  // namespace OHOS
