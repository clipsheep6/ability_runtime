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
#include "feature_ability.h"
#include <cstring>
#include <vector>
#include <uv.h>
#include "securec.h"
#include "ability_process.h"
#include "hilog_wrapper.h"
#include "napi_context.h"
#include "element_name.h"
#include "napi_data_ability_helper.h"
#include "../inner/napi_common/napi_common_ability.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
extern napi_value g_classContext;
extern napi_value g_dataAbilityHelper;

CallbackInfo g_aceCallbackInfo;

/**
 * @brief FeatureAbility NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value FeatureAbilityInit(napi_env env, napi_value exports)
{
    HILOG_INFO("%{public}s,called", __func__);
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("startAbility", NAPI_StartAbility),
        DECLARE_NAPI_FUNCTION("startAbilityForResult", NAPI_StartAbilityForResult),
        DECLARE_NAPI_FUNCTION("finishWithResult", NAPI_SetResult),
        DECLARE_NAPI_FUNCTION("terminateAbility", NAPI_TerminateAbility),
        DECLARE_NAPI_FUNCTION("hasWindowFocus", NAPI_HasWindowFocus),
        DECLARE_NAPI_FUNCTION("getContext", NAPI_GetContext),
        DECLARE_NAPI_FUNCTION("getWant", NAPI_GetWant),
        DECLARE_NAPI_FUNCTION("getApplicationInfo", NAPI_GetApplicationInfo),
        DECLARE_NAPI_FUNCTION("getAppType", NAPI_GetAppType),
        DECLARE_NAPI_FUNCTION("getElementName", NAPI_GetElementName),
        DECLARE_NAPI_FUNCTION("getAbilityName", NAPI_GetAbilityName),
        DECLARE_NAPI_FUNCTION("getProcessInfo", NAPI_GetProcessInfo),
        DECLARE_NAPI_FUNCTION("getProcessName", NAPI_GetProcessName),
        DECLARE_NAPI_FUNCTION("getCallingBundle", NAPI_GetCallingBundle),
        DECLARE_NAPI_FUNCTION("getAbilityInfo", NAPI_GetAbilityInfo),
        DECLARE_NAPI_FUNCTION("getHapModuleInfo", NAPI_GetHapModuleInfo),
        DECLARE_NAPI_FUNCTION("getDataAbilityHelper", NAPI_GetDataAbilityHelper),
        DECLARE_NAPI_FUNCTION("connectAbility", NAPI_FAConnectAbility),
        DECLARE_NAPI_FUNCTION("disconnectAbility", NAPI_FADisConnectAbility),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties));

    return exports;
}

/**
 * @brief FeatureAbility NAPI method : startAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_StartAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_StartAbilityCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief FeatureAbility NAPI method : startAbilityForResult.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_StartAbilityForResult(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called env=%{public}p", __func__, env);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }

    napi_value ret = StartAbilityForResultWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
    }
    return ret;
}

/**
 * @brief StartAbilityForResult processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value StartAbilityForResultWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{

    HILOG_INFO("%{public}s,called", __func__);
    size_t argcAsync = 3;
    const size_t argcPromise = 2;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    CallAbilityParam param;
    if (UnwrapParam(param, env, args[0]) == nullptr) {
        HILOG_ERROR("%{public}s, call UnwrapParam failed.", __func__);
        return nullptr;
    }
    asyncCallbackInfo->param = param;
    asyncCallbackInfo->aceCallback = &g_aceCallbackInfo;

    g_aceCallbackInfo.env = env;
    if (argcAsync > PARAM1) {
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, args[PARAM1], &valuetype);
        if (valuetype == napi_function) {
            // resultCallback: AsyncCallback<StartAbilityResult>
            napi_create_reference(env, args[PARAM1], 1, &g_aceCallbackInfo.callback);
        }
    }

    if (argcAsync > argcPromise) {
        ret = StartAbilityForResultAsync(env, args, argcAsync, argcPromise, asyncCallbackInfo);
    } else {
        ret = StartAbilityForResultPromise(env, asyncCallbackInfo);
    }

    return ret;
}

napi_value StartAbilityForResultAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    g_aceCallbackInfo.env = env;
    if (argcAsync > PARAM2) {
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, args[PARAM2], &valuetype);
        if (valuetype == napi_function) {
            // callback: AsyncCallback<number>
            napi_create_reference(env, args[PARAM2], 1, &asyncCallbackInfo->cbInfo.callback);
        }
    }
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_StartAbilityForResult, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo != nullptr) {
                AbilityProcess::GetInstance()->StartAbility(
                    asyncCallbackInfo->ability, asyncCallbackInfo->param, *asyncCallbackInfo->aceCallback);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_StartAbilityForResult, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value result[ARGS_TWO] = {0};
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
            napi_create_int32(env, 0, &result[PARAM1]);
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = 0;
    napi_get_null(env, &result);
    return result;
}

napi_value StartAbilityForResultPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);
    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_StartAbilityForResult, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo != nullptr) {
                AbilityProcess::GetInstance()->StartAbility(
                    asyncCallbackInfo->ability, asyncCallbackInfo->param, *asyncCallbackInfo->aceCallback);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_StartAbilityForResult,  main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value result = 0;
            napi_create_int32(env, 0, &result);
            napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return promise;
}

/**
 * @brief FeatureAbility NAPI method : setResult.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_SetResult(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }

    napi_value ret = SetResultWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
    }
    return ret;
}

/**
 * @brief SetResult processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value SetResultWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{

    HILOG_INFO("%{public}s,called", __func__);
    size_t argcAsync = 2;
    const size_t argcPromise = 1;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    CallAbilityParam param;
    if (UnwrapAbilityResult(param, env, args[0]) == nullptr) {
        HILOG_ERROR("%{public}s, call unwrapWant failed.", __func__);
        return nullptr;
    }
    asyncCallbackInfo->param = param;

    if (argcAsync > argcPromise) {
        ret = SetResultAsync(env, args, argcAsync, argcPromise, asyncCallbackInfo);
    } else {
        ret = SetResultPromise(env, asyncCallbackInfo);
    }

    return ret;
}

napi_value SetResultAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argcPromise], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argcPromise], 1, &asyncCallbackInfo->cbInfo.callback);
    }

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_SetResult, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->ability->SetResult(
                    asyncCallbackInfo->param.requestCode, asyncCallbackInfo->param.want);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_SetResult, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value result[ARGS_TWO] = {0};
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
            napi_get_null(env, &result[PARAM1]);
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = 0;
    napi_get_null(env, &result);
    return result;
}

napi_value SetResultPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);
    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_SetResult, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->ability->SetResult(
                    asyncCallbackInfo->param.requestCode, asyncCallbackInfo->param.want);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_SetResult,  main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value result = 0;
            napi_get_null(env, &result);
            napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return promise;
}

/**
 * @brief FeatureAbility NAPI method : terminateAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_TerminateAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }

    napi_value ret = TerminateAbilityWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
    }
    return ret;
}

/**
 * @brief TerminateAbility processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value TerminateAbilityWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncCallbackInfo == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = TerminateAbilityAsync(env, args, argcAsync, argcPromise, asyncCallbackInfo);
    } else {
        ret = TerminateAbilityPromise(env, asyncCallbackInfo);
    }

    return ret;
}

napi_value TerminateAbilityAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argcPromise], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argcPromise], 1, &asyncCallbackInfo->cbInfo.callback);
    }

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_TerminateAbility, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->ability->TerminateAbility();
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_TerminateAbility, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value result[ARGS_TWO] = {0};
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
            napi_get_null(env, &result[PARAM1]);
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = 0;
    napi_get_null(env, &result);
    return result;
}

napi_value TerminateAbilityPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);

    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_TerminateAbility, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->ability->TerminateAbility();
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_TerminateAbility,  main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value result = 0;
            napi_get_null(env, &result);
            napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return promise;
}

/**
 * @brief Checks whether the main window of this ability has window focus.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_HasWindowFocus(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }

    napi_value ret = HasWindowFocusWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
    }
    return ret;
}

/**
 * @brief Get context.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetContext(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetContextCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief HasWindowFocus processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value HasWindowFocusWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncCallbackInfo == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = HasWindowFocusAsync(env, args, argcAsync, argcPromise, asyncCallbackInfo);
    } else {
        ret = HasWindowFocusPromise(env, asyncCallbackInfo);
    }

    return ret;
}

napi_value HasWindowFocusAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argcPromise], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argcPromise], 1, &asyncCallbackInfo->cbInfo.callback);
    }
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_HasWindowFocus, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->native_result = asyncCallbackInfo->ability->HasWindowFocus();
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_HasWindowFocus, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value result[ARGS_TWO] = {0};
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
            napi_get_boolean(env, asyncCallbackInfo->native_result, &result[PARAM1]);
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = 0;
    napi_get_null(env, &result);
    return result;
}

napi_value HasWindowFocusPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);
    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_HasWindowFocus, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->native_result = asyncCallbackInfo->ability->HasWindowFocus();
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_HasWindowFocus, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value result = 0;
            napi_get_boolean(env, asyncCallbackInfo->native_result, &result);
            napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

    return promise;
}

/**
 * @brief The interface of onAbilityResult provided for ACE to call back to JS.
 *
 * @param requestCode Indicates the request code returned after the ability is started.
 * @param resultCode Indicates the result code returned after the ability is started.
 * @param resultData Indicates the data returned after the ability is started.
 * @param cb The environment and call back info that the Node-API call is invoked under.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
void CallOnAbilityResult(int requestCode, int resultCode, const Want &resultData, CallbackInfo cb)
{
    HILOG_INFO("%{public}s,called env=%{public}p", __func__, cb.env);

    uv_loop_s *loop = nullptr;

#if NAPI_VERSION >= 2
    napi_get_uv_event_loop(cb.env, &loop);
#endif  // NAPI_VERSION >= 2

    uv_work_t *work = new uv_work_t;
    OnAbilityCallback *onAbilityCB = new (std::nothrow) OnAbilityCallback{
        .requestCode = requestCode,
        .resultCode = resultCode,
        .resultData = resultData,
        .cb = cb,
    };
    work->data = (void *)onAbilityCB;

    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            HILOG_INFO("CallOnAbilityResult, uv_queue_work");
            // JS Thread
            OnAbilityCallback *event = (OnAbilityCallback *)work->data;
            napi_value result[ARGS_TWO] = {0};
            result[PARAM0] = GetCallbackErrorValue(event->cb.env, NO_ERROR);

            napi_create_object(event->cb.env, &result[PARAM1]);
            // create requestCode
            napi_value jsValue = 0;
            napi_create_int32(event->cb.env, event->requestCode, &jsValue);
            napi_set_named_property(event->cb.env, result[PARAM1], "requestCode", jsValue);
            // create resultCode
            napi_create_int32(event->cb.env, event->resultCode, &jsValue);
            napi_set_named_property(event->cb.env, result[PARAM1], "resultCode", jsValue);
            // create want
            napi_value jsWant = WrapWant(event->cb.env, event->resultData);
            napi_set_named_property(event->cb.env, result[PARAM1], "want", jsWant);
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_get_undefined(event->cb.env, &undefined);
            napi_value callResult = 0;
            napi_get_reference_value(event->cb.env, event->cb.callback, &callback);

            napi_call_function(event->cb.env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);
            if (event->cb.callback != nullptr) {
                napi_delete_reference(event->cb.env, event->cb.callback);
            }
            delete event;
            delete work;
        });

    return;
}

bool InnerUnwrapWant(napi_env env, napi_value args, Want &want)
{
    HILOG_INFO("%{public}s called", __func__);
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, args, &valueType), false);
    if (valueType != napi_object) {
        return false;
    }

    napi_value jsWant = GetPropertyValueByPropertyName(env, args, "want", napi_object);
    if (jsWant == nullptr) {
        return false;
    }

    return UnwrapWant(env, jsWant, want);
}

/**
 * @brief Parse the parameters.
 *
 * @param param Indicates the parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapParam(CallAbilityParam &param, napi_env env, napi_value args)
{
    HILOG_INFO("%{public}s,called", __func__);
    // unwrap the param
    napi_valuetype valueType = napi_undefined;

    // unwrap the param : want object
    InnerUnwrapWant(env, args, param.want);

    // unwrap the param : requestCode (optional)
    napi_value requestCodeProp = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, args, "requestCode", &requestCodeProp));
    NAPI_CALL(env, napi_typeof(env, requestCodeProp, &valueType));
    // there is requestCode in param, set forResultOption = true
    if (valueType == napi_number) {
        NAPI_CALL(env, napi_get_value_int32(env, requestCodeProp, &param.requestCode));
        param.forResultOption = true;
    } else {
        param.forResultOption = false;
    }
    HILOG_INFO("%{public}s, reqCode=%{public}d forResultOption=%{public}d.",
        __func__,
        param.requestCode,
        param.forResultOption);

    // unwrap the param : abilityStartSetting (optional)
    napi_value abilityStartSettingProp = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, args, "abilityStartSetting", &abilityStartSettingProp));
    NAPI_CALL(env, napi_typeof(env, abilityStartSettingProp, &valueType));
    if (valueType == napi_object) {
        param.setting = AbilityStartSetting::GetEmptySetting();
        HILOG_INFO("%{public}s, abilityStartSetting=%{public}p.", __func__, param.setting.get());
    }

    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    return result;
}

/**
 * @brief Parse the abilityResult parameters.
 *
 * @param param Indicates the want parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapAbilityResult(CallAbilityParam &param, napi_env env, napi_value args)
{
    HILOG_INFO("%{public}s,called", __func__);
    // unwrap the param
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args, &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "param type mismatch!");
    // get resultCode property
    napi_value property = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, args, "resultCode", &property));
    NAPI_CALL(env, napi_typeof(env, property, &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "property type mismatch!");
    NAPI_CALL(env, napi_get_value_int32(env, property, &param.requestCode));
    HILOG_INFO("%{public}s, requestCode=%{public}d.", __func__, param.requestCode);

    // unwrap the param : want object
    InnerUnwrapWant(env, args, param.want);

    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    return result;
}

/**
 * @brief Get want.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetWant(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetWantCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains information about the current application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetApplicationInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    return NAPI_GetApplicationInfoCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains the type of this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAppType(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetAppTypeCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains the elementName object of the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetElementName(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetElementNameCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains the class name in this ability name, without the prefixed bundle name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityName(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetAbilityNameCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains the process Info this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetProcessInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetProcessInfoCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains the name of the current process.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetProcessName(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetProcessNameCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains the bundle name of the ability that called the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetCallingBundle(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetCallingBundleCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains information about the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetAbilityInfoCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains the HapModuleInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetHapModuleInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetHapModuleInfoCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief FeatureAbility NAPI method : getDataAbilityHelper.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetDataAbilityHelper(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    DataAbilityHelperCB *dataAbilityHelperCB = new (std::nothrow) DataAbilityHelperCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = nullptr,
        .result = nullptr,
    };

    napi_value ret = GetDataAbilityHelperWrap(env, info, dataAbilityHelperCB);
    if (ret == nullptr) {
        if (dataAbilityHelperCB != nullptr) {
            delete dataAbilityHelperCB;
            dataAbilityHelperCB = nullptr;
        }
    }
    return ret;
}

/**
 * @brief getDataAbilityHelper processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param dataAbilityHelperCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetDataAbilityHelperWrap(napi_env env, napi_callback_info info, DataAbilityHelperCB *dataAbilityHelperCB)
{

    HILOG_INFO("%{public}s,called", __func__);
    size_t argcAsync = 2;
    const size_t argcPromise = 1;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        NAPI_CALL(env, napi_create_reference(env, args[PARAM0], 1, &dataAbilityHelperCB->uri));
    }

    if (argcAsync > argcPromise) {
        ret = GetDataAbilityHelperAsync(env, args, argcAsync, argcPromise, dataAbilityHelperCB);
    } else {
        ret = GetDataAbilityHelperPromise(env, dataAbilityHelperCB);
    }

    return ret;
}

napi_value GetDataAbilityHelperAsync(napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise,
    DataAbilityHelperCB *dataAbilityHelperCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || dataAbilityHelperCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &dataAbilityHelperCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(
            env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) { HILOG_INFO("NAPI_GetDataAbilityHelper, worker pool thread execute."); },
            GetDataAbilityHelperAsyncCompleteCB,
            (void *)dataAbilityHelperCB,
            &dataAbilityHelperCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, dataAbilityHelperCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

napi_value GetDataAbilityHelperPromise(napi_env env, DataAbilityHelperCB *dataAbilityHelperCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (dataAbilityHelperCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    dataAbilityHelperCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(
            env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) { HILOG_INFO("NAPI_GetDataAbilityHelper, worker pool thread execute."); },
            GetDataAbilityHelperPromiseCompleteCB,
            (void *)dataAbilityHelperCB,
            &dataAbilityHelperCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, dataAbilityHelperCB->cbBase.asyncWork));
    return promise;
}

void GetDataAbilityHelperAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetDataAbilityHelper, main event thread complete.");
    DataAbilityHelperCB *dataAbilityHelperCB = (DataAbilityHelperCB *)data;
    napi_value uri = nullptr;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, dataAbilityHelperCB->uri, &uri));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, dataAbilityHelperCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_new_instance(env, g_dataAbilityHelper, 1, &uri, &dataAbilityHelperCB->result));

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    result[PARAM1] = dataAbilityHelperCB->result;
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (dataAbilityHelperCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, dataAbilityHelperCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, dataAbilityHelperCB->cbBase.asyncWork));
    delete dataAbilityHelperCB;
    dataAbilityHelperCB = nullptr;
}

void GetDataAbilityHelperPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetDataAbilityHelper,  main event thread complete.");
    DataAbilityHelperCB *dataAbilityHelperCB = (DataAbilityHelperCB *)data;
    napi_value uri = nullptr;
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, dataAbilityHelperCB->uri, &uri));
    NAPI_CALL_RETURN_VOID(env, napi_new_instance(env, g_dataAbilityHelper, 1, &uri, &dataAbilityHelperCB->result));
    result = dataAbilityHelperCB->result;

    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, dataAbilityHelperCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, dataAbilityHelperCB->cbBase.asyncWork));
    delete dataAbilityHelperCB;
    dataAbilityHelperCB = nullptr;
}

/**
 * @brief FeatureAbility NAPI method : connectAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_FAConnectAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_ConnectAbilityCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief FeatureAbility NAPI method : disConnectAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_FADisConnectAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_DisConnectAbilityCommon(env, info, AbilityType::PAGE);
}
}  // namespace AppExecFwk
}  // namespace OHOS