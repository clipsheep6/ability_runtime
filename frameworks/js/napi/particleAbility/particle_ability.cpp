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

constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_TWO = 2;

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
 * @brief ParticleAbility NAPI method : startAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAStartAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_StartAbilityCommon(env, info, AbilityType::UNKNOWN);
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
 * @brief ParticleAbility NAPI method : connectAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAConnectAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_ConnectAbilityCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief ParticleAbility NAPI method : disconnectAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PADisConnectAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_DisConnectAbilityCommon(env, info, AbilityType::UNKNOWN);
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

napi_value NAPI_PATerminateAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    return NAPI_TerminateAbilityCommon(env, info);
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
        DECLARE_NAPI_FUNCTION("startAbility", NAPI_PAStartAbility),
        DECLARE_NAPI_FUNCTION("stopAbility", NAPI_PAStopAbility),
        DECLARE_NAPI_FUNCTION("connectAbility", NAPI_PAConnectAbility),
        DECLARE_NAPI_FUNCTION("disconnectAbility", NAPI_PADisConnectAbility),
        DECLARE_NAPI_FUNCTION("acquireDataAbilityHelper", NAPI_PAAcquireDataAbilityHelper),
        DECLARE_NAPI_FUNCTION("terminateSelf", NAPI_PATerminateAbility),
    };
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);

    return reinterpret_cast<napi_value>(JsParticleAbilityInit(reinterpret_cast<NativeEngine*>(env),
        reinterpret_cast<NativeValue*>(exports)));
}

napi_value UnwrapParamForWantAgent(napi_env &env, napi_value &args, AbilityRuntime::WantAgent::WantAgent *&wantAgent)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
    napi_value wantAgentParam = nullptr;
    napi_value result = nullptr;

    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, args, "wantAgent", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, args, "wantAgent", &wantAgentParam);
        NAPI_CALL(env, napi_typeof(env, wantAgentParam, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
        napi_unwrap(env, wantAgentParam, (void **)&wantAgent);
    }

    napi_get_null(env, &result);
    return result;
}

void JsParticleAbility::Finalizer(NativeEngine *engine, void *data, void *hint)
{
    HILOG_INFO("JsWantAgent::Finalizer is called");
    std::unique_ptr<JsParticleAbility>(static_cast<JsParticleAbility*>(data));
}

NativeValue* JsParticleAbility::PAStartBackgroundRunning(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsParticleAbility *me = CheckParamsAndGetThis<JsParticleAbility>(engine, info);
    return (me != nullptr) ? me->OnPAStartBackgroundRunning(*engine, *info) : nullptr;
}

NativeValue* JsParticleAbility::PACancelBackgroundRunning(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsParticleAbility *me = CheckParamsAndGetThis<JsParticleAbility>(engine, info);
    return (me != nullptr) ? me->OnPACancelBackgroundRunning(*engine, *info) : nullptr;
}

Ability* JsParticleAbility::GetAbility(napi_env env)
{
    napi_status ret;
    napi_value global = 0;
    const napi_extended_error_info *errorInfo = nullptr;
    ret = napi_get_global(env, &global);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        HILOG_ERROR("JsParticleAbility::GetAbility, get_global=%{public}d err:%{public}s",
            ret, errorInfo->error_message);
        return nullptr;
    }
    napi_value abilityObj = 0;
    ret = napi_get_named_property(env, global, "ability", &abilityObj);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        HILOG_ERROR("JsParticleAbility::GetAbility, get_named_property=%{public}d err:%{public}s",
            ret, errorInfo->error_message);
        return nullptr;
    }
    Ability* ability = nullptr;
    ret = napi_get_value_external(env, abilityObj, (void **)&ability);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        HILOG_ERROR("JsParticleAbility::GetAbility, get_value_external=%{public}d err:%{public}s",
            ret, errorInfo->error_message);
        return nullptr;
    }
    return ability;
}

NativeValue* JsParticleAbility::OnPAStartBackgroundRunning(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s is called", __FUNCTION__);
    if (info.argc < ARGC_TWO || info.argc > ARGC_THREE) {
        HILOG_ERROR("Wrong argument count");
        return engine.CreateUndefined();
    }
    auto errorVal = static_cast<int32_t>(NAPI_ERR_NO_ERROR);
    auto env = reinterpret_cast<napi_env>(&engine);
    auto arg1 = reinterpret_cast<napi_value>(info.argv[1]);
    AbilityRuntime::WantAgent::WantAgent *wantAgent = nullptr;
    if (UnwrapParamForWantAgent(env, arg1, wantAgent) == nullptr) {
        errorVal = static_cast<int32_t>(NAPI_ERR_PARAM_INVALID);
    }

    if (ability_ == nullptr) {
        errorVal = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
    }

    const std::shared_ptr<AbilityInfo> abilityinfo = ability_->GetAbilityInfo();
    if (abilityinfo == nullptr) {
        HILOG_ERROR("abilityinfo is null");
        errorVal = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
    }

    AbilityRuntime::WantAgent::WantAgent wantAgentObj;
    if(!wantAgent) {
        HILOG_WARN("input param without wantAgent");
        wantAgentObj = AbilityRuntime::WantAgent::WantAgent();
    } else {
        wantAgentObj = *wantAgent;
    }
    AsyncTask::CompleteCallback complete =
        [wantAgentObj, obj = this, errorVal] (NativeEngine &engine, AsyncTask &task, int32_t status) {
            if (errorVal != static_cast<int32_t>(NAPI_ERR_NO_ERROR)) {
                task.Reject(engine, CreateJsError(engine, errorVal, "calling interface failed."));
                return;
            }
            auto ret = obj->ability_->StartBackgroundRunning(wantAgentObj);;
            if(ret != NAPI_ERR_NO_ERROR){
                task.Reject(engine, CreateJsError(engine, errorVal, "StartBackgroundRunning failed."));
                return;
            }
            task.Resolve(engine, CreateJsValue(engine, ret));
        };

    NativeValue *lastParam = (info.argc >= ARGC_THREE) ? info.argv[INDEX_TWO] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsParticleAbility::OnPAStartBackgroundRunning",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsParticleAbility::OnPACancelBackgroundRunning(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("%{public}s is called", __FUNCTION__);
    if (info.argc > ARGC_ONE) {
        HILOG_ERROR("Wrong argument count");
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [obj = this](NativeEngine &engine, AsyncTask &task, int32_t status) {
            if (obj->ability_ == nullptr) {
                HILOG_ERROR("task execute error, the ability is nullptr.");
                task.Reject(engine, CreateJsError(engine, NAPI_ERR_ACE_ABILITY, "StopBackgroundRunning failed."));
                return;
            }
            obj->ability_->StopBackgroundRunning();
            task.Resolve(engine, engine.CreateUndefined());
        };

    NativeValue *lastParam = (info.argc >= ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsParticleAbility::OnPACancelBackgroundRunning",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
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
    BindNativeFunction(*engine, *object, "startBackgroundRunning", moduleName,
        JsParticleAbility::PAStartBackgroundRunning);
    BindNativeFunction(*engine, *object, "cancelBackgroundRunning", moduleName,
        JsParticleAbility::PACancelBackgroundRunning);

    HILOG_DEBUG("JsParticleAbility end");
    return exportObj;
}

}  // namespace AppExecFwk
}  // namespace OHOS
