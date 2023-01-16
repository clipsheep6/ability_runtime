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

#include "napi_context.h"

#include <cstring>
#include <uv.h>

#include "../inner/napi_common/napi_common_ability.h"
#include "ability_util.h"
#include "ability_process.h"
#include "directory_ex.h"
#include "feature_ability_common.h"
#include "file_ex.h"
#include "hilog_wrapper.h"
#include "js_napi_common_ability.h"
#include "permission_list_state.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AppExecFwk {
const std::string NAPI_CONTEXT_FILE_SEPARATOR = std::string("/");
const std::string NAPI_CONTEXT_DATABASE = std::string("database");
const std::string NAPI_CONTEXT_PREFERENCES = std::string("preferences");

napi_value ContextConstructor(napi_env env, napi_callback_info info)
{
    napi_value jsthis = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr));

    napi_value value = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, false, &value));

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_PROPERTY("stageMode", value),
    };
    NAPI_CALL(env, napi_define_properties(env, jsthis, sizeof(desc) / sizeof(desc[0]), desc));

    return jsthis;
}

#ifdef SUPPORT_GRAPHICS
static Ability* GetJSAbilityObject(napi_env env)
{
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));
    return ability;
}

static void SetShowOnLockScreenAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s,called", __func__);
    ShowOnLockScreenCB *showOnLockScreenCB = static_cast<ShowOnLockScreenCB *>(data);
    if (showOnLockScreenCB == nullptr) {
        HILOG_ERROR("%{public}s, input param is nullptr", __func__);
        return;
    }

    showOnLockScreenCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (showOnLockScreenCB->cbBase.ability == nullptr) {
        HILOG_ERROR("%{public}s, input param is nullptr", __func__);
        showOnLockScreenCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
    } else {
        showOnLockScreenCB->cbBase.ability->SetShowOnLockScreen(showOnLockScreenCB->isShow);
    }

    napi_value callback = nullptr, undefined = nullptr, callResult = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_get_undefined(env, &undefined);
    result[PARAM0] = GetCallbackErrorValue(env, showOnLockScreenCB->cbBase.errCode);
    napi_get_null(env, &result[PARAM1]);
    napi_get_reference_value(env, showOnLockScreenCB->cbBase.cbInfo.callback, &callback);
    napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

    if (showOnLockScreenCB->cbBase.cbInfo.callback != nullptr) {
        napi_delete_reference(env, showOnLockScreenCB->cbBase.cbInfo.callback);
    }
    napi_delete_async_work(env, showOnLockScreenCB->cbBase.asyncWork);
    delete showOnLockScreenCB;
    showOnLockScreenCB = nullptr;

    HILOG_INFO("%{public}s,called end", __func__);
}

static napi_value SetShowOnLockScreenAsync(napi_env env, napi_value *args, ShowOnLockScreenCB *showOnLockScreenCB)
{
    HILOG_INFO("%{public}s,called", __func__);
    if (showOnLockScreenCB == nullptr) {
        HILOG_ERROR("%{public}s, input param is nullptr", __func__);
        return nullptr;
    }

    napi_valuetype valuetypeParam1 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetypeParam1));
    if (valuetypeParam1 != napi_function) {
        HILOG_ERROR("%{public}s error, params is error type", __func__);
        return nullptr;
    }

    NAPI_CALL(env, napi_create_reference(env, args[PARAM1], 1, &showOnLockScreenCB->cbBase.cbInfo.callback));

    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName,
            [](napi_env env, void *data) { HILOG_INFO("NAPI_SetShowOnLockScreen, worker pool thread execute."); },
            SetShowOnLockScreenAsyncCompleteCB,
            static_cast<void *>(showOnLockScreenCB),
            &showOnLockScreenCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, showOnLockScreenCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));

    HILOG_INFO("%{public}s,called end", __func__);
    return result;
}

napi_value SetShowOnLockScreenPromise(napi_env env, ShowOnLockScreenCB *cbData)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (cbData == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_deferred deferred;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);
    cbData->cbBase.deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) { HILOG_INFO("SetShowOnLockScreenPromise, worker pool thread execute."); },
        [](napi_env env, napi_status status, void *data) {
            ShowOnLockScreenCB *showOnLockScreenCB = static_cast<ShowOnLockScreenCB *>(data);
            showOnLockScreenCB->cbBase.errCode = NO_ERROR;
            if (showOnLockScreenCB->cbBase.ability == nullptr) {
                HILOG_ERROR("%{public}s, input param is nullptr", __func__);
                showOnLockScreenCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
            } else {
                showOnLockScreenCB->cbBase.ability->SetShowOnLockScreen(showOnLockScreenCB->isShow);
            }

            napi_value result = GetCallbackErrorValue(env, showOnLockScreenCB->cbBase.errCode);
            if (showOnLockScreenCB->cbBase.errCode == NO_ERROR) {
                napi_resolve_deferred(env, showOnLockScreenCB->cbBase.deferred, result);
            } else {
                napi_reject_deferred(env, showOnLockScreenCB->cbBase.deferred, result);
            }

            napi_delete_async_work(env, showOnLockScreenCB->cbBase.asyncWork);
            delete showOnLockScreenCB;
            showOnLockScreenCB = nullptr;
            HILOG_INFO("SetShowOnLockScreenPromise, main event thread complete end.");
        },
        static_cast<void *>(cbData),
        &cbData->cbBase.asyncWork);
    napi_queue_async_work(env, cbData->cbBase.asyncWork);
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

napi_value NAPI_SetDisplayOrientationWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapSetDisplayOrientation(env, argc, args, asyncCallbackInfo)) {
        HILOG_INFO("%{public}s called. Invoke UnwrapSetDisplayOrientation fail", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_SetDisplayOrientationCallback";
        asyncParamEx.execute = SetDisplayOrientationExecuteCallbackWork;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_SetDisplayOrientationPromise";
        asyncParamEx.execute = SetDisplayOrientationExecuteCallbackWork;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

void SetDisplayOrientationExecuteCallbackWork(napi_env env, void *data)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    int orientation = asyncCallbackInfo->param.paramArgs.GetIntValue("orientation");
    asyncCallbackInfo->ability->SetDisplayOrientation(orientation);
    asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
}

bool UnwrapSetDisplayOrientation(napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_DEBUG("%{public}s called, argc=%{public}zu", __func__, argc);

    const size_t argcMax = 2;
    if (argc > argcMax || argc < argcMax - 1) {
        HILOG_ERROR("%{public}s called, Params is invalid.", __func__);
        return false;
    }

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM1], asyncCallbackInfo)) {
            HILOG_DEBUG("%{public}s called, the second parameter is invalid.", __func__);
            return false;
        }
    }

    int orientation = 0;
    if (!UnwrapInt32FromJS2(env, argv[PARAM0], orientation)) {
        HILOG_ERROR("%{public}s called, the parameter is invalid.", __func__);
        return false;
    }

    int maxRange = 3;
    if (orientation < 0 || orientation > maxRange) {
        HILOG_ERROR("%{public}s called, wrong parameter range.", __func__);
        return false;
    }

    asyncCallbackInfo->param.paramArgs.PutIntValue("orientation", orientation);
    return true;
}

static void SetWakeUpScreenAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s,called", __func__);
    SetWakeUpScreenCB *setWakeUpScreenCB = static_cast<SetWakeUpScreenCB *>(data);
    if (setWakeUpScreenCB == nullptr) {
        HILOG_ERROR("%{public}s, input param is nullptr", __func__);
        return;
    }

    do {
        setWakeUpScreenCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
        if (setWakeUpScreenCB->cbBase.ability == nullptr) {
            HILOG_ERROR("%{public}s, input param is nullptr", __func__);
            setWakeUpScreenCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
            break;
        }

        setWakeUpScreenCB->cbBase.ability->SetWakeUpScreen(setWakeUpScreenCB->wakeUp);
    } while (false);

    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_get_undefined(env, &undefined);
    result[PARAM0] = GetCallbackErrorValue(env, setWakeUpScreenCB->cbBase.errCode);
    napi_get_null(env, &result[PARAM1]);
    napi_get_reference_value(env, setWakeUpScreenCB->cbBase.cbInfo.callback, &callback);
    napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

    if (setWakeUpScreenCB->cbBase.cbInfo.callback != nullptr) {
        napi_delete_reference(env, setWakeUpScreenCB->cbBase.cbInfo.callback);
    }
    napi_delete_async_work(env, setWakeUpScreenCB->cbBase.asyncWork);
    delete setWakeUpScreenCB;
    setWakeUpScreenCB = nullptr;

    HILOG_INFO("%{public}s,called end", __func__);
}

static napi_value SetWakeUpScreenAsync(napi_env env, napi_value *args, SetWakeUpScreenCB *cbData)
{
    HILOG_INFO("%{public}s,called", __func__);
    if (cbData == nullptr || args == nullptr) {
        HILOG_ERROR("%{public}s, input param is nullptr", __func__);
        return nullptr;
    }

    napi_valuetype valuetypeParam0 = napi_undefined;
    napi_valuetype valuetypeParam1 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetypeParam0));
    NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetypeParam1));
    if (valuetypeParam0 != napi_boolean || valuetypeParam1 != napi_function) {
        HILOG_ERROR("%{public}s, Params is error type", __func__);
        return nullptr;
    }
    NAPI_CALL(env, napi_create_reference(env, args[PARAM1], 1, &cbData->cbBase.cbInfo.callback));

    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    NAPI_CALL(env,
        napi_create_async_work(
            env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) { HILOG_INFO("NAPI_SetWakeUpScreenScreen, worker pool thread execute."); },
            SetWakeUpScreenAsyncCompleteCB,
            static_cast<void *>(cbData),
            &cbData->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, cbData->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));

    HILOG_INFO("%{public}s,called end", __func__);
    return result;
}

napi_value SetWakeUpScreenPromise(napi_env env, SetWakeUpScreenCB *cbData)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (cbData == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);
    cbData->cbBase.deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) { HILOG_INFO("NAPI_SetWakeUpScreenScreen, worker pool thread execute."); },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("SetWakeUpScreenPromise, main event thread complete.");
            SetWakeUpScreenCB *setWakeUpScreenCB = static_cast<SetWakeUpScreenCB *>(data);
            setWakeUpScreenCB->cbBase.errCode = NO_ERROR;
            if (setWakeUpScreenCB->cbBase.ability == nullptr) {
                HILOG_ERROR("%{public}s, input param is nullptr", __func__);
                setWakeUpScreenCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
            } else {
                setWakeUpScreenCB->cbBase.ability->SetWakeUpScreen(setWakeUpScreenCB->wakeUp);
            }
            napi_value result = GetCallbackErrorValue(env, setWakeUpScreenCB->cbBase.errCode);
            if (setWakeUpScreenCB->cbBase.errCode == NO_ERROR) {
                napi_resolve_deferred(env, setWakeUpScreenCB->cbBase.deferred, result);
            } else {
                napi_reject_deferred(env, setWakeUpScreenCB->cbBase.deferred, result);
            }

            napi_delete_async_work(env, setWakeUpScreenCB->cbBase.asyncWork);
            delete setWakeUpScreenCB;
            setWakeUpScreenCB = nullptr;
            HILOG_INFO("SetWakeUpScreenPromise, main event thread complete end.");
        },
        static_cast<void *>(cbData),
        &cbData->cbBase.asyncWork);
    napi_queue_async_work(env, cbData->cbBase.asyncWork);
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

static napi_value SetWakeUpScreenWrap(napi_env env, napi_callback_info info, SetWakeUpScreenCB *cbData)
{
    HILOG_INFO("%{public}s,called", __func__);
    if (cbData == nullptr) {
        HILOG_ERROR("%{public}s, input param cbData is nullptr", __func__);
        return nullptr;
    }

    size_t argcAsync = 2;
    const size_t argStdValue = 2;
    const size_t argPromise = 1;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync != argStdValue && argcAsync != argPromise) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (!UnwrapBoolFromJS2(env, args[PARAM0], cbData->wakeUp)) {
        HILOG_ERROR("%{public}s, UnwrapBoolFromJS2(wakeUp) run error", __func__);
        return nullptr;
    }

    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    cbData->cbBase.ability = ability;
    napi_value ret = nullptr;
    if (argcAsync == argStdValue) {
        ret = SetWakeUpScreenAsync(env, args, cbData);
    } else {
        ret = SetWakeUpScreenPromise(env, cbData);
    }
    HILOG_INFO("%{public}s,called end", __func__);
    return ret;
}
#endif

napi_value NAPI_SetShowOnLockScreen(napi_env env, napi_callback_info info)
{
#ifdef SUPPORT_GRAPHICS
    HILOG_INFO("%{public}s called", __func__);

    size_t argc = 2;
    const size_t argcAsync = 2, argcPromise = 1;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc != argcAsync && argc != argcPromise) {
        HILOG_ERROR("%{public}s error, wrong argument count.", __func__);
        return nullptr;
    }

    napi_valuetype valuetypeParam0 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetypeParam0));
    if (valuetypeParam0 != napi_boolean) {
        HILOG_ERROR("%{public}s error, params is error type", __func__);
        return nullptr;
    }

    ShowOnLockScreenCB *showOnLockScreenCB = new ShowOnLockScreenCB();
    showOnLockScreenCB->cbBase.cbInfo.env = env;
    showOnLockScreenCB->cbBase.abilityType = AbilityType::PAGE;
    if (!UnwrapBoolFromJS2(env, args[PARAM0], showOnLockScreenCB->isShow)) {
        HILOG_ERROR("%{public}s error, unwrapBoolFromJS2 error", __func__);
        delete showOnLockScreenCB;
        showOnLockScreenCB = nullptr;
        return nullptr;
    }

    showOnLockScreenCB->cbBase.ability = GetJSAbilityObject(env);
    napi_value ret = nullptr;
    if (argc == argcAsync) {
        ret = SetShowOnLockScreenAsync(env, args, showOnLockScreenCB);
    } else {
        ret = SetShowOnLockScreenPromise(env, showOnLockScreenCB);
    }

    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, SetShowOnLockScreenWrap failed.", __func__);
        delete showOnLockScreenCB;
        showOnLockScreenCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    return ret;
#else
   return nullptr;
#endif
}

bool UnwrapParamVerifySelfPermission(
    napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called, argc=%{public}zu", __func__, argc);

    const size_t argcMax = 2;
    if (argc > argcMax || argc < argcMax - 1) {
        HILOG_INFO("%{public}s called, Params is invalid.", __func__);
        return false;
    }

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM1], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
            return false;
        }
    }

    std::string permission("");
    if (!UnwrapStringFromJS2(env, argv[PARAM0], permission)) {
        HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
        return false;
    }

    asyncCallbackInfo->param.paramArgs.PutStringValue("permission", permission);
    return true;
}

void VerifySelfPermissionExecuteCallbackWork(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;

    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_INT32;
    asyncCallbackInfo->native_data.int32_value = asyncCallbackInfo->ability->VerifySelfPermission(
        asyncCallbackInfo->param.paramArgs.GetStringValue("permission"));
}

napi_value NAPI_VerifySelfPermissionWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamVerifySelfPermission(env, argc, args, asyncCallbackInfo)) {
        HILOG_INFO("%{public}s called. Invoke UnwrapParamVerifySelfPermission fail", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_VerifySelfPermissionCallback";
        asyncParamEx.execute = VerifySelfPermissionExecuteCallbackWork;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_VerifySelfPermissionPromise";
        asyncParamEx.execute = VerifySelfPermissionExecuteCallbackWork;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_VerifySelfPermission(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }

    napi_value rev = NAPI_VerifySelfPermissionWrap(env, info, asyncCallbackInfo);
    if (rev == nullptr) {
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        rev = WrapVoidToJS(env);
    }
    return rev;
}

bool UnwrapRequestPermissionsFromUser(
    napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called, argc=%{public}zu", __func__, argc);

    const size_t argcMax = 3;
    if (argc > argcMax || argc < argcMax - 1) {
        HILOG_INFO("%{public}s called, parameters is invalid", __func__);
        return false;
    }

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM2], asyncCallbackInfo)) {
            HILOG_DEBUG("%{public}s called, the third parameter is invalid.", __func__);
            return false;
        }
    }

    std::vector<std::string> permissionList;
    if (!UnwrapArrayStringFromJS(env, argv[PARAM0], permissionList)) {
        HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
        return false;
    }

    int requestCode = 0;
    if (!UnwrapInt32FromJS2(env, argv[PARAM1], requestCode)) {
        HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
        return false;
    }

    asyncCallbackInfo->param.paramArgs.PutIntValue("requestCode", requestCode);
    asyncCallbackInfo->param.paramArgs.PutStringValueArray("permissionList", permissionList);
    return true;
}

void RequestPermissionsFromUserExecuteCallbackWork(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    CallAbilityPermissionParam permissionParam;
    permissionParam.requestCode = asyncCallbackInfo->param.paramArgs.GetIntValue("requestCode");
    asyncCallbackInfo->param.paramArgs.GetStringValueArray("permissionList", permissionParam.permission_list);
    if (permissionParam.permission_list.size() == 0) {
        asyncCallbackInfo->error_code = NAPI_ERR_PARAM_INVALID;
        return;
    }

    AbilityProcess::GetInstance()->RequestPermissionsFromUser(
        asyncCallbackInfo->ability, permissionParam, asyncCallbackInfo->cbInfo);
}

void RequestPermissionsFromUserCompleteAsyncCallbackWork(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called, asyncCallbackInfo is null", __func__);
        return;
    }

    if (asyncCallbackInfo->error_code != NAPI_ERR_NO_ERROR) {
        napi_value callback = nullptr;
        napi_value undefined = nullptr;
        napi_get_undefined(env, &undefined);
        napi_value callResult = nullptr;
        napi_value revParam[ARGS_TWO] = {nullptr};

        revParam[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->error_code);
        revParam[PARAM1] = WrapVoidToJS(env);

        if (asyncCallbackInfo->cbInfo.callback != nullptr) {
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS_TWO, revParam, &callResult);
            napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
        } else if (asyncCallbackInfo->cbInfo.deferred != nullptr) {
            napi_reject_deferred(env, asyncCallbackInfo->cbInfo.deferred, revParam[PARAM0]);
        }
    }

    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;
}

napi_value NAPI_RequestPermissionsFromUserWrap(
    napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapRequestPermissionsFromUser(env, argc, args, asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s called. Invoke UnwrapRequestPermissionsFromUser failed.", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_DEBUG("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_RequestPermissionsFromUserCallback";
        asyncParamEx.execute = RequestPermissionsFromUserExecuteCallbackWork;
        asyncParamEx.complete = RequestPermissionsFromUserCompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_DEBUG("%{public}s called. promise.", __func__);
        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->cbInfo.deferred = deferred;

        napi_value resourceName = nullptr;
        NAPI_CALL(env, napi_create_string_latin1(env, "NAPI_RequestPermissionsFromUserPromise",
            NAPI_AUTO_LENGTH, &resourceName));
        NAPI_CALL(env,
            napi_create_async_work(env,
                nullptr,
                resourceName,
                RequestPermissionsFromUserExecuteCallbackWork,
                RequestPermissionsFromUserCompleteAsyncCallbackWork,
                static_cast<void *>(asyncCallbackInfo),
                &asyncCallbackInfo->asyncWork));

        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

        return promise;
    }
}

napi_value NAPI_RequestPermissionsFromUser(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    napi_value rev = NAPI_RequestPermissionsFromUserWrap(env, info, asyncCallbackInfo);
    if (rev == nullptr) {
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        rev = WrapVoidToJS(env);
    }
    return rev;
}

EXTERN_C_START
void CallOnRequestPermissionsFromUserResult(int requestCode, const std::vector<std::string> &permissions,
    const std::vector<int> &grantResults, CallbackInfo callbackInfo)
{
    HILOG_INFO("%{public}s,called env", __func__);

    if (permissions.empty()) {
        HILOG_ERROR("%{public}s, the string vector permissions is empty.", __func__);
        return;
    }
    if (permissions.size() != grantResults.size()) {
        HILOG_ERROR("%{public}s, the size of permissions not equal the size of grantResults.", __func__);
        return;
    }
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(callbackInfo.env, &loop);
    if (loop == nullptr) {
        return;
    }

    uv_work_t *work = new uv_work_t;
    OnRequestPermissionsFromUserResultCallback *onRequestPermissionCB = new OnRequestPermissionsFromUserResultCallback;
    onRequestPermissionCB->requestCode = requestCode;
    onRequestPermissionCB->permissions = permissions;
    onRequestPermissionCB->grantResults = grantResults;
    onRequestPermissionCB->cb = callbackInfo;

    work->data = static_cast<void *>(onRequestPermissionCB);

    int rev = uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            if (work == nullptr) {
                HILOG_ERROR("%{public}s, uv_queue_work work is nullptr.", __func__);
                return;
            }
            OnRequestPermissionsFromUserResultCallback *onRequestPermissionCB =
                (OnRequestPermissionsFromUserResultCallback *)work->data;
            if (onRequestPermissionCB == nullptr) {
                delete work;
                work = nullptr;
                return;
            }

            napi_value result[ARGS_TWO] = {0};
            result[PARAM0] = GetCallbackErrorValue(onRequestPermissionCB->cb.env, 0);
            napi_create_object(onRequestPermissionCB->cb.env, &result[PARAM1]);

            // create requestCode
            napi_value jsValue = 0;
            napi_create_int32(onRequestPermissionCB->cb.env, onRequestPermissionCB->requestCode, &jsValue);
            napi_set_named_property(onRequestPermissionCB->cb.env, result[PARAM1], "requestCode", jsValue);

            // create permissions
            napi_value perValue = 0;
            napi_value perArray = 0;
            napi_create_array(onRequestPermissionCB->cb.env, &perArray);

            for (size_t i = 0; i < onRequestPermissionCB->permissions.size(); i++) {
                napi_create_string_utf8(onRequestPermissionCB->cb.env,
                    onRequestPermissionCB->permissions[i].c_str(),
                    NAPI_AUTO_LENGTH,
                    &perValue);
                napi_set_element(onRequestPermissionCB->cb.env, perArray, i, perValue);
            }
            napi_set_named_property(onRequestPermissionCB->cb.env, result[PARAM1], "permissions", perArray);

            // create grantResults
            napi_value grantArray;
            napi_create_array(onRequestPermissionCB->cb.env, &grantArray);

            for (size_t j = 0; j < onRequestPermissionCB->grantResults.size(); j++) {
                napi_create_int32(onRequestPermissionCB->cb.env, onRequestPermissionCB->grantResults[j], &perValue);
                napi_set_element(onRequestPermissionCB->cb.env, grantArray, j, perValue);
            }
            napi_set_named_property(onRequestPermissionCB->cb.env, result[PARAM1], "authResults", grantArray);

            // call CB function
            if (onRequestPermissionCB->cb.callback != nullptr) {
                HILOG_DEBUG("%{public}s call callback function.", __func__);
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_get_undefined(onRequestPermissionCB->cb.env, &undefined);

                napi_value callResult = 0;
                napi_get_reference_value(onRequestPermissionCB->cb.env, onRequestPermissionCB->cb.callback, &callback);
                napi_call_function(
                    onRequestPermissionCB->cb.env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

                if (onRequestPermissionCB->cb.callback != nullptr) {
                    napi_delete_reference(onRequestPermissionCB->cb.env, onRequestPermissionCB->cb.callback);
                }
            } else if (onRequestPermissionCB->cb.deferred != nullptr) { // call promise function
                HILOG_DEBUG("%{public}s call promise function.", __func__);
                napi_resolve_deferred(onRequestPermissionCB->cb.env, onRequestPermissionCB->cb.deferred,
                    result[PARAM1]);
            }

            delete onRequestPermissionCB;
            onRequestPermissionCB = nullptr;
            delete work;
            work = nullptr;
        });
    if (rev != 0) {
        if (onRequestPermissionCB != nullptr) {
            delete onRequestPermissionCB;
            onRequestPermissionCB = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
}
EXTERN_C_END

napi_value NAPI_GetFilesDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called", __func__);
    return NAPI_GetFilesDirCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_GetOrCreateDistributedDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called", __func__);
    return NAPI_GetOrCreateDistributedDirCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_GetCacheDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called", __func__);
    return NAPI_GetCacheDirCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_GetCtxAppType(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetAppTypeCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_GetCtxHapModuleInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetHapModuleInfoCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_GetAppVersionInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetAppVersionInfoCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_GetApplicationContext(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetContextCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_GetCtxAbilityInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetAbilityInfoCommon(env, info, AbilityType::PAGE);
}

bool UnwrapVerifyPermissionOptions(napi_env env, napi_value argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called, asyncCallbackInfo is null", __func__);
        return false;
    }

    if (!IsTypeForNapiValue(env, argv, napi_object)) {
        HILOG_INFO("%{public}s called, type of parameter is error.", __func__);
        return false;
    }

    int value = 0;
    if (UnwrapInt32ByPropertyName(env, argv, "pid", value)) {
        asyncCallbackInfo->param.paramArgs.PutIntValue("pid", value);
    }

    value = 0;
    if (UnwrapInt32ByPropertyName(env, argv, "uid", value)) {
        asyncCallbackInfo->param.paramArgs.PutIntValue("uid", value);
    }
    return true;
}

bool UnwrapParamVerifyPermission(napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called, argc=%{public}zu", __func__, argc);

    const size_t argcMax = ARGS_THREE;
    if (argc > argcMax || argc < 1) {
        HILOG_INFO("%{public}s called, Params is invalid.", __func__);
        return false;
    }

    std::string permission("");
    if (!UnwrapStringFromJS2(env, argv[PARAM0], permission)) {
        HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
        return false;
    }
    asyncCallbackInfo->param.paramArgs.PutStringValue("permission", permission);

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM2], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
            return false;
        }

        if (!UnwrapVerifyPermissionOptions(env, argv[PARAM1], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
            return false;
        }
    } else if (argc == ARGS_TWO) {
        if (!CreateAsyncCallback(env, argv[PARAM1], asyncCallbackInfo)) {
            if (!UnwrapVerifyPermissionOptions(env, argv[PARAM1], asyncCallbackInfo)) {
                HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
                return false;
            }
        }
    } else if (argc == ARGS_ONE) {
        asyncCallbackInfo->cbInfo.callback = nullptr;
    } else {
        HILOG_INFO("%{public}s called, the parameter is invalid.", __func__);
        return false;
    }
    return true;
}

void VerifyPermissionExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;

    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    std::string permission(asyncCallbackInfo->param.paramArgs.GetStringValue("permission").c_str());
    bool hasUid = asyncCallbackInfo->param.paramArgs.HasKey("uid");
    int pid = asyncCallbackInfo->param.paramArgs.GetIntValue("pid");
    int uid = asyncCallbackInfo->param.paramArgs.GetIntValue("uid");

    asyncCallbackInfo->native_data.data_type = NVT_INT32;

    if (hasUid) {
        asyncCallbackInfo->native_data.int32_value = asyncCallbackInfo->ability->VerifyPermission(permission, pid, uid);
    } else {
        asyncCallbackInfo->native_data.int32_value = asyncCallbackInfo->ability->VerifySelfPermission(permission);
    }
}

napi_value NAPI_VerifyPermissionWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamVerifyPermission(env, argc, args, asyncCallbackInfo)) {
        HILOG_INFO("%{public}s called. Invoke UnwrapParamVerifyPermission fail", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_VerifyPermissionCallback";
        asyncParamEx.execute = VerifyPermissionExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_VerifyPermissionPromise";
        asyncParamEx.execute = VerifyPermissionExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_VerifyPermission(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    napi_value rev = NAPI_VerifyPermissionWrap(env, info, asyncCallbackInfo);
    if (rev == nullptr) {
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        rev = WrapVoidToJS(env);
    }
    return rev;
}

void GetAppInfoExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute.");
    AppInfoCB *appInfoCB = static_cast<AppInfoCB *>(data);
    appInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;

    if (appInfoCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetApplicationInfo, ability == nullptr");
        appInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    std::shared_ptr<ApplicationInfo> appInfoPtr = appInfoCB->cbBase.ability->GetApplicationInfo();
    if (appInfoPtr != nullptr) {
        appInfoCB->appInfo = *appInfoPtr;
    } else {
        HILOG_ERROR("NAPI_GetApplicationInfo, appInfoPtr == nullptr");
        appInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute end.");
}

void GetAppInfoAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    AppInfoCB *appInfoCB = static_cast<AppInfoCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, appInfoCB->cbBase.errCode);
    if (appInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapAppInfo(env, appInfoCB->appInfo);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, appInfoCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (appInfoCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, appInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, appInfoCB->cbBase.asyncWork));
    delete appInfoCB;
    appInfoCB = nullptr;
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete end.");
}

napi_value GetApplicationInfoAsync(napi_env env, napi_value *args, const size_t argCallback, AppInfoCB *appInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || appInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &appInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAppInfoExecuteCB,
            GetAppInfoAsyncCompleteCB,
            static_cast<void *>(appInfoCB),
            &appInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, appInfoCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

void GetAppInfoPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    AppInfoCB *appInfoCB = static_cast<AppInfoCB *>(data);
    if (appInfoCB == nullptr) {
        HILOG_ERROR("NAPI_GetApplicationInfo, appInfoCB == nullptr");
        return;
    }

    napi_value result = nullptr;
    if (appInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapAppInfo(env, appInfoCB->appInfo);
        napi_resolve_deferred(env, appInfoCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, appInfoCB->cbBase.errCode);
        napi_reject_deferred(env, appInfoCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, appInfoCB->cbBase.asyncWork);
    delete appInfoCB;
    appInfoCB = nullptr;
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete end.");
}

napi_value GetApplicationInfoPromise(napi_env env, AppInfoCB *appInfoCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (appInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    appInfoCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAppInfoExecuteCB,
            GetAppInfoPromiseCompleteCB,
            static_cast<void *>(appInfoCB),
            &appInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, appInfoCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetApplicationInfoWrap(napi_env env, napi_callback_info info, AppInfoCB *appInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (appInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, appInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetApplicationInfoAsync(env, args, 0, appInfoCB);
    } else {
        ret = GetApplicationInfoPromise(env, appInfoCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

AppInfoCB *CreateAppInfoCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    AppInfoCB *appInfoCB = new (std::nothrow) AppInfoCB;
    if (appInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, appInfoCB == nullptr.", __func__);
        return nullptr;
    }
    appInfoCB->cbBase.cbInfo.env = env;
    appInfoCB->cbBase.asyncWork = nullptr;
    appInfoCB->cbBase.deferred = nullptr;
    appInfoCB->cbBase.ability = ability;
    appInfoCB->cbBase.abilityType = AbilityType::UNKNOWN;
    appInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;

    HILOG_INFO("%{public}s, end.", __func__);
    return appInfoCB;
}

void GetBundleNameExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_STRING;
    asyncCallbackInfo->native_data.str_value = asyncCallbackInfo->ability->GetBundleName();
    HILOG_INFO("%{public}s end. bundleName=%{public}s", __func__, asyncCallbackInfo->native_data.str_value.c_str());
}

napi_value NAPI_GetBundleNameWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        HILOG_INFO("%{public}s called, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetBundleNameCallback";
        asyncParamEx.execute = GetBundleNameExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetBundleNamePromise";
        asyncParamEx.execute = GetBundleNameExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value WrapProcessInfo(napi_env env, ProcessInfoCB *processInfoCB)
{
    HILOG_INFO("%{public}s called", __func__);
    if (processInfoCB == nullptr) {
        HILOG_ERROR("%{public}s Invalid param(processInfoCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_create_int32(env, processInfoCB->pid, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "pid", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, processInfoCB->processName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "processName", proValue));
    HILOG_INFO("%{public}s end", __func__);
    return result;
}

void GetProcessInfoExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetProcessInfo, worker pool thread execute.");
    ProcessInfoCB *processInfoCB = static_cast<ProcessInfoCB *>(data);
    if (processInfoCB == nullptr) {
        return;
    }

    processInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (processInfoCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetProcessInfo, ability == nullptr");
        processInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    std::shared_ptr<ProcessInfo> processInfoPtr = processInfoCB->cbBase.ability->GetProcessInfo();
    if (processInfoPtr != nullptr) {
        processInfoCB->processName = processInfoPtr->GetProcessName();
        processInfoCB->pid = processInfoPtr->GetPid();
    } else {
        HILOG_ERROR("NAPI_GetProcessInfo, processInfoPtr == nullptr");
        processInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("NAPI_GetProcessInfo, worker pool thread execute end.");
}

void GetProcessInfoAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetProcessInfo, main event thread complete.");
    ProcessInfoCB *processInfoCB = static_cast<ProcessInfoCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, processInfoCB->cbBase.errCode);
    if (processInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapProcessInfo(env, processInfoCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }

    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, processInfoCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (processInfoCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, processInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, processInfoCB->cbBase.asyncWork));
    delete processInfoCB;
    processInfoCB = nullptr;
    HILOG_INFO("NAPI_GetProcessInfo, main event thread complete end.");
}

napi_value GetProcessInfoAsync(napi_env env, napi_value *args, const size_t argCallback, ProcessInfoCB *processInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || processInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &processInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetProcessInfoExecuteCB,
            GetProcessInfoAsyncCompleteCB,
            static_cast<void *>(processInfoCB),
            &processInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, processInfoCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

void GetProcessInfoPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetProcessInfo, main event thread complete.");
    ProcessInfoCB *processInfoCB = static_cast<ProcessInfoCB *>(data);
    napi_value result = nullptr;
    if (processInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapProcessInfo(env, processInfoCB);
        napi_resolve_deferred(env, processInfoCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, processInfoCB->cbBase.errCode);
        napi_reject_deferred(env, processInfoCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, processInfoCB->cbBase.asyncWork);
    delete processInfoCB;
    processInfoCB = nullptr;
    HILOG_INFO("NAPI_GetProcessInfo, main event thread complete end.");
}

napi_value GetProcessInfoPromise(napi_env env, ProcessInfoCB *processInfoCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (processInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    processInfoCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetProcessInfoExecuteCB,
            GetProcessInfoPromiseCompleteCB,
            static_cast<void *>(processInfoCB),
            &processInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, processInfoCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetProcessInfoWrap(napi_env env, napi_callback_info info, ProcessInfoCB *processInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (processInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, processInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetProcessInfoAsync(env, args, 0, processInfoCB);
    } else {
        ret = GetProcessInfoPromise(env, processInfoCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

ProcessInfoCB *CreateProcessInfoCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    ProcessInfoCB *processInfoCB = new (std::nothrow) ProcessInfoCB;
    if (processInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, processInfoCB == nullptr.", __func__);
        return nullptr;
    }
    processInfoCB->cbBase.cbInfo.env = env;
    processInfoCB->cbBase.asyncWork = nullptr;
    processInfoCB->cbBase.deferred = nullptr;
    processInfoCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s, end.", __func__);
    return processInfoCB;
}

ElementNameCB *CreateElementNameCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    ElementNameCB *elementNameCB = new (std::nothrow) ElementNameCB;
    if (elementNameCB == nullptr) {
        HILOG_ERROR("%{public}s, elementNameCB == nullptr.", __func__);
        return nullptr;
    }
    elementNameCB->cbBase.cbInfo.env = env;
    elementNameCB->cbBase.asyncWork = nullptr;
    elementNameCB->cbBase.deferred = nullptr;
    elementNameCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s, end.", __func__);
    return elementNameCB;
}

napi_value WrapElementName(napi_env env, const ElementNameCB *elementNameCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (elementNameCB == nullptr) {
        HILOG_ERROR("%{public}s,Invalid param(appInfoCB = nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_create_string_utf8(env, elementNameCB->abilityName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "abilityName", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, elementNameCB->bundleName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "bundleName", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, elementNameCB->deviceId.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "deviceId", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, elementNameCB->shortName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "shortName", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, elementNameCB->uri.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "uri", proValue));
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

void GetElementNameExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetElementName, worker pool thread execute.");
    if (data == nullptr) {
        HILOG_ERROR("%{public}s, data == nullptr.", __func__);
        return;
    }
    ElementNameCB *elementNameCB = static_cast<ElementNameCB *>(data);
    if (elementNameCB == nullptr) {
        HILOG_ERROR("NAPI_GetElementName, elementNameCB == nullptr");
        return;
    }

    elementNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (elementNameCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetElementName, ability == nullptr");
        elementNameCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    std::shared_ptr<ElementName> elementName = elementNameCB->cbBase.ability->GetElementName();
    if (elementName != nullptr) {
        elementNameCB->deviceId = elementName->GetDeviceID();
        elementNameCB->bundleName = elementName->GetBundleName();
        elementNameCB->abilityName = elementName->GetAbilityName();
        elementNameCB->uri = elementNameCB->cbBase.ability->GetWant()->GetUriString();
        elementNameCB->shortName = "";
    } else {
        elementNameCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("NAPI_GetElementName, worker pool thread execute end.");
}

void GetElementNameAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetElementName, main event thread complete.");
    ElementNameCB *elementNameCB = static_cast<ElementNameCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, elementNameCB->cbBase.errCode);
    if (elementNameCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapElementName(env, elementNameCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, elementNameCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (elementNameCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, elementNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, elementNameCB->cbBase.asyncWork));
    delete elementNameCB;
    elementNameCB = nullptr;
    HILOG_INFO("NAPI_GetElementName, main event thread complete end.");
}

void GetElementNamePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetElementName, main event thread complete.");
    ElementNameCB *elementNameCB = static_cast<ElementNameCB *>(data);
    napi_value result = nullptr;
    if (elementNameCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapElementName(env, elementNameCB);
        napi_resolve_deferred(env, elementNameCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, elementNameCB->cbBase.errCode);
        napi_reject_deferred(env, elementNameCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, elementNameCB->cbBase.asyncWork);
    delete elementNameCB;
    elementNameCB = nullptr;
    HILOG_INFO("NAPI_GetElementName, main event thread complete end.");
}

napi_value GetElementNamePromise(napi_env env, ElementNameCB *elementNameCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (elementNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    elementNameCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetElementNameExecuteCB,
            GetElementNamePromiseCompleteCB,
            static_cast<void *>(elementNameCB),
            &elementNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, elementNameCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetElementNameAsync(napi_env env, napi_value *args, const size_t argCallback, ElementNameCB *elementNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || elementNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &elementNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetElementNameExecuteCB,
            GetElementNameAsyncCompleteCB,
            static_cast<void *>(elementNameCB),
            &elementNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, elementNameCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value GetElementNameWrap(napi_env env, napi_callback_info info, ElementNameCB *elementNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (elementNameCB == nullptr) {
        HILOG_ERROR("%{public}s, appInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetElementNameAsync(env, args, 0, elementNameCB);
    } else {
        ret = GetElementNamePromise(env, elementNameCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

ProcessNameCB *CreateProcessNameCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    ProcessNameCB *processNameCB = new (std::nothrow) ProcessNameCB;
    if (processNameCB == nullptr) {
        HILOG_ERROR("%{public}s, processNameCB == nullptr.", __func__);
        return nullptr;
    }
    processNameCB->cbBase.cbInfo.env = env;
    processNameCB->cbBase.asyncWork = nullptr;
    processNameCB->cbBase.deferred = nullptr;
    processNameCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return processNameCB;
}

void GetProcessNameExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetProcessName, worker pool thread execute.");
    ProcessNameCB *processNameCB = static_cast<ProcessNameCB *>(data);
    if (processNameCB == nullptr) {
        HILOG_ERROR("NAPI_GetProcessName, processNameCB == nullptr");
        return;
    }

    processNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (processNameCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetProcessName, ability == nullptr");
        processNameCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    processNameCB->processName = processNameCB->cbBase.ability->GetProcessName();
    HILOG_INFO("NAPI_GetProcessName, worker pool thread execute end.");
}

napi_value WrapProcessName(napi_env env, const ProcessNameCB *processNameCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (processNameCB == nullptr) {
        HILOG_ERROR("%{public}s, Invalid param(processNameCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, processNameCB->processName.c_str(), NAPI_AUTO_LENGTH, &result));
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

void GetProcessNameAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetProcessName, main event thread complete.");
    ProcessNameCB *processNameCB = static_cast<ProcessNameCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, processNameCB->cbBase.errCode);
    if (processNameCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapProcessName(env, processNameCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, processNameCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (processNameCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, processNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, processNameCB->cbBase.asyncWork));
    delete processNameCB;
    processNameCB = nullptr;
    HILOG_INFO("NAPI_GetProcessName, main event thread complete end.");
}

void GetProcessNamePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetProcessName, main event thread complete.");
    ProcessNameCB *processNameCB = static_cast<ProcessNameCB *>(data);
    napi_value result = nullptr;
    if (processNameCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapProcessName(env, processNameCB);
        napi_resolve_deferred(env, processNameCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, processNameCB->cbBase.errCode);
        napi_reject_deferred(env, processNameCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, processNameCB->cbBase.asyncWork);
    delete processNameCB;
    processNameCB = nullptr;
    HILOG_INFO("NAPI_GetProcessName, main event thread complete end.");
}

napi_value GetProcessNameAsync(napi_env env, napi_value *args, const size_t argCallback, ProcessNameCB *processNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || processNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &processNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetProcessNameExecuteCB,
            GetProcessNameAsyncCompleteCB,
            static_cast<void *>(processNameCB),
            &processNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, processNameCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value GetProcessNamePromise(napi_env env, ProcessNameCB *processNameCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (processNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    processNameCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetProcessNameExecuteCB,
            GetProcessNamePromiseCompleteCB,
            static_cast<void *>(processNameCB),
            &processNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, processNameCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetProcessNameWrap(napi_env env, napi_callback_info info, ProcessNameCB *processNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (processNameCB == nullptr) {
        HILOG_ERROR("%{public}s, processNameCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetProcessNameAsync(env, args, 0, processNameCB);
    } else {
        ret = GetProcessNamePromise(env, processNameCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

CallingBundleCB *CreateCallingBundleCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    CallingBundleCB *callingBundleCB = new (std::nothrow) CallingBundleCB;
    if (callingBundleCB == nullptr) {
        HILOG_ERROR("%{public}s, callingBundleCB == nullptr.", __func__);
        return nullptr;
    }
    callingBundleCB->cbBase.cbInfo.env = env;
    callingBundleCB->cbBase.asyncWork = nullptr;
    callingBundleCB->cbBase.deferred = nullptr;
    callingBundleCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return callingBundleCB;
}

void GetCallingBundleExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetCallingBundle, worker pool thread execute.");
    CallingBundleCB *callingBundleCB = static_cast<CallingBundleCB *>(data);
    if (callingBundleCB == nullptr) {
        HILOG_ERROR("NAPI_GetCallingBundle, callingBundleCB == nullptr");
        return;
    }

    callingBundleCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (callingBundleCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetCallingBundle, ability == nullptr");
        callingBundleCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    callingBundleCB->callingBundleName = callingBundleCB->cbBase.ability->GetCallingBundle();
    HILOG_INFO("NAPI_GetCallingBundle, worker pool thread execute end.");
}

napi_value WrapCallingBundle(napi_env env, const CallingBundleCB *callingBundleCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (callingBundleCB == nullptr) {
        HILOG_ERROR("%{public}s,Invalid param(callingBundleCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, callingBundleCB->callingBundleName.c_str(), NAPI_AUTO_LENGTH, &result));
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

void GetCallingBundleAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetCallingBundle, main event thread complete.");
    CallingBundleCB *callingBundleCB = static_cast<CallingBundleCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, callingBundleCB->cbBase.errCode);
    if (callingBundleCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapCallingBundle(env, callingBundleCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, callingBundleCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (callingBundleCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, callingBundleCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, callingBundleCB->cbBase.asyncWork));
    delete callingBundleCB;
    callingBundleCB = nullptr;
    HILOG_INFO("NAPI_GetCallingBundle, main event thread complete end.");
}

void GetCallingBundlePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetCallingBundle, main event thread complete.");
    CallingBundleCB *callingBundleCB = static_cast<CallingBundleCB *>(data);
    napi_value result = nullptr;
    if (callingBundleCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapCallingBundle(env, callingBundleCB);
        napi_resolve_deferred(env, callingBundleCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, callingBundleCB->cbBase.errCode);
        napi_reject_deferred(env, callingBundleCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, callingBundleCB->cbBase.asyncWork);
    delete callingBundleCB;
    callingBundleCB = nullptr;
    HILOG_INFO("NAPI_GetCallingBundle, main event thread complete end.");
}

napi_value GetCallingBundleAsync(
    napi_env env, napi_value *args, const size_t argCallback, CallingBundleCB *callingBundleCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || callingBundleCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &callingBundleCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetCallingBundleExecuteCB,
            GetCallingBundleAsyncCompleteCB,
            static_cast<void *>(callingBundleCB),
            &callingBundleCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, callingBundleCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value GetCallingBundlePromise(napi_env env, CallingBundleCB *callingBundleCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (callingBundleCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    callingBundleCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetCallingBundleExecuteCB,
            GetCallingBundlePromiseCompleteCB,
            static_cast<void *>(callingBundleCB),
            &callingBundleCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, callingBundleCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetCallingBundleWrap(napi_env env, napi_callback_info info, CallingBundleCB *callingBundleCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (callingBundleCB == nullptr) {
        HILOG_ERROR("%{public}s, callingBundleCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetCallingBundleAsync(env, args, 0, callingBundleCB);
    } else {
        ret = GetCallingBundlePromise(env, callingBundleCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

GetOrCreateLocalDirCB *CreateGetOrCreateLocalDirCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    GetOrCreateLocalDirCB *getOrCreateLocalDirCB = new (std::nothrow) GetOrCreateLocalDirCB;
    if (getOrCreateLocalDirCB == nullptr) {
        HILOG_ERROR("%{public}s, getOrCreateLocalDirCB == nullptr.", __func__);
        return nullptr;
    }
    getOrCreateLocalDirCB->cbBase.cbInfo.env = env;
    getOrCreateLocalDirCB->cbBase.asyncWork = nullptr;
    getOrCreateLocalDirCB->cbBase.deferred = nullptr;
    getOrCreateLocalDirCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return getOrCreateLocalDirCB;
}

void GetOrCreateLocalDirExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetOrCreateLocalDir, worker pool thread execute.");
    GetOrCreateLocalDirCB *getOrCreateLocalDirCB = static_cast<GetOrCreateLocalDirCB *>(data);
    if (getOrCreateLocalDirCB == nullptr) {
        HILOG_ERROR("NAPI_GetOrCreateLocalDir, callingBundleCB == nullptr");
        return;
    }

    getOrCreateLocalDirCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (getOrCreateLocalDirCB->cbBase.ability == nullptr ||
        getOrCreateLocalDirCB->cbBase.ability->GetAbilityContext() == nullptr) {
        HILOG_ERROR("NAPI_GetOrCreateLocalDir, ability or abilityContext is nullptr");
        getOrCreateLocalDirCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    getOrCreateLocalDirCB->rootDir = getOrCreateLocalDirCB->cbBase.ability->GetAbilityContext()->GetBaseDir();
    HILOG_INFO("NAPI_GetOrCreateLocalDir, GetDir rootDir:%{public}s", getOrCreateLocalDirCB->rootDir.c_str());
    if (!OHOS::FileExists(getOrCreateLocalDirCB->rootDir)) {
        HILOG_INFO("NAPI_GetOrCreateLocalDir dir is not exits, create dir.");
        OHOS::ForceCreateDirectory(getOrCreateLocalDirCB->rootDir);
        OHOS::ChangeModeDirectory(getOrCreateLocalDirCB->rootDir, MODE);
    }
    HILOG_INFO("NAPI_GetOrCreateLocalDir, worker pool thread execute end.");
}

napi_value WrapGetOrCreateLocalDir(napi_env env, const GetOrCreateLocalDirCB *getOrCreateLocalDirCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (getOrCreateLocalDirCB == nullptr) {
        HILOG_ERROR("%{public}s,Invalid param(getOrCreateLocalDirCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, getOrCreateLocalDirCB->rootDir.c_str(), NAPI_AUTO_LENGTH, &result));
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

void GetOrCreateLocalDirAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetOrCreateLocalDir, main event thread complete.");
    GetOrCreateLocalDirCB *getOrCreateLocalDirCB = static_cast<GetOrCreateLocalDirCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, getOrCreateLocalDirCB->cbBase.errCode);
    if (getOrCreateLocalDirCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapGetOrCreateLocalDir(env, getOrCreateLocalDirCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, getOrCreateLocalDirCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (getOrCreateLocalDirCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, getOrCreateLocalDirCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, getOrCreateLocalDirCB->cbBase.asyncWork));
    delete getOrCreateLocalDirCB;
    getOrCreateLocalDirCB = nullptr;
    HILOG_INFO("NAPI_GetOrCreateLocalDir, main event thread complete end.");
}

void GetOrCreateLocalDirPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetOrCreateLocalDir, main event thread complete.");
    GetOrCreateLocalDirCB *getOrCreateLocalDirCB = static_cast<GetOrCreateLocalDirCB *>(data);
    napi_value result = nullptr;
    if (getOrCreateLocalDirCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapGetOrCreateLocalDir(env, getOrCreateLocalDirCB);
        napi_resolve_deferred(env, getOrCreateLocalDirCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, getOrCreateLocalDirCB->cbBase.errCode);
        napi_reject_deferred(env, getOrCreateLocalDirCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, getOrCreateLocalDirCB->cbBase.asyncWork);
    delete getOrCreateLocalDirCB;
    getOrCreateLocalDirCB = nullptr;
    HILOG_INFO("NAPI_GetOrCreateLocalDir, main event thread complete end.");
}

napi_value GetOrCreateLocalDirAsync(
    napi_env env, napi_value *args, const size_t argCallback, GetOrCreateLocalDirCB *getOrCreateLocalDirCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || getOrCreateLocalDirCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(
            env, napi_create_reference(env, args[argCallback], 1, &getOrCreateLocalDirCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetOrCreateLocalDirExecuteCB,
            GetOrCreateLocalDirAsyncCompleteCB,
            static_cast<void *>(getOrCreateLocalDirCB),
            &getOrCreateLocalDirCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, getOrCreateLocalDirCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value GetOrCreateLocalDirPromise(napi_env env, GetOrCreateLocalDirCB *getOrCreateLocalDirCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (getOrCreateLocalDirCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    getOrCreateLocalDirCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetOrCreateLocalDirExecuteCB,
            GetOrCreateLocalDirPromiseCompleteCB,
            static_cast<void *>(getOrCreateLocalDirCB),
            &getOrCreateLocalDirCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, getOrCreateLocalDirCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetOrCreateLocalDirWrap(napi_env env, napi_callback_info info, GetOrCreateLocalDirCB *getOrCreateLocalDirCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (getOrCreateLocalDirCB == nullptr) {
        HILOG_ERROR("%{public}s, getOrCreateLocalDirCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetOrCreateLocalDirAsync(env, args, 0, getOrCreateLocalDirCB);
    } else {
        ret = GetOrCreateLocalDirPromise(env, getOrCreateLocalDirCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

napi_value NAPI_GetBundleName(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    napi_value ret = NAPI_GetBundleNameWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

napi_value NAPI_GetApplicationInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    AppInfoCB *appInfoCB = CreateAppInfoCBInfo(env);
    if (appInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    napi_value ret = GetApplicationInfoWrap(env, info, appInfoCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s,ret == nullptr", __func__);
        if (appInfoCB != nullptr) {
            delete appInfoCB;
            appInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

napi_value NAPI_GetProcessInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    ProcessInfoCB *processInfoCB = CreateProcessInfoCBInfo(env);
    if (processInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    processInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetProcessInfoWrap(env, info, processInfoCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, ret == nullptr.", __func__);
        if (processInfoCB != nullptr) {
            delete processInfoCB;
            processInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s, end.", __func__);
    return ret;
}

napi_value NAPI_GetElementName(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    ElementNameCB *elementNameCB = CreateElementNameCBInfo(env);
    if (elementNameCB == nullptr) {
        return WrapVoidToJS(env);
    }

    elementNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetElementNameWrap(env, info, elementNameCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr.", __func__);
        if (elementNameCB != nullptr) {
            delete elementNameCB;
            elementNameCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

napi_value NAPI_GetProcessName(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    ProcessNameCB *processNameCB = CreateProcessNameCBInfo(env);
    if (processNameCB == nullptr) {
        return WrapVoidToJS(env);
    }

    processNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetProcessNameWrap(env, info, processNameCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr.", __func__);
        if (processNameCB != nullptr) {
            delete processNameCB;
            processNameCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

napi_value NAPI_GetCallingBundle(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    CallingBundleCB *callingBundleCB = CreateCallingBundleCBInfo(env);
    if (callingBundleCB == nullptr) {
        return WrapVoidToJS(env);
    }

    callingBundleCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetCallingBundleWrap(env, info, callingBundleCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (callingBundleCB != nullptr) {
            delete callingBundleCB;
            callingBundleCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

napi_value NAPI_GetOrCreateLocalDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    GetOrCreateLocalDirCB *getOrCreateLocalDirCB = CreateGetOrCreateLocalDirCBInfo(env);
    if (getOrCreateLocalDirCB == nullptr) {
        return WrapVoidToJS(env);
    }

    getOrCreateLocalDirCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetOrCreateLocalDirWrap(env, info, getOrCreateLocalDirCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (getOrCreateLocalDirCB != nullptr) {
            delete getOrCreateLocalDirCB;
            getOrCreateLocalDirCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

DatabaseDirCB *CreateGetDatabaseDirCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    DatabaseDirCB *getDatabaseDirCB = new (std::nothrow) DatabaseDirCB;
    if (getDatabaseDirCB == nullptr) {
        HILOG_ERROR("%{public}s, getDatabaseDirCB == nullptr.", __func__);
        return nullptr;
    }
    getDatabaseDirCB->cbBase.cbInfo.env = env;
    getDatabaseDirCB->cbBase.asyncWork = nullptr;
    getDatabaseDirCB->cbBase.deferred = nullptr;
    getDatabaseDirCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return getDatabaseDirCB;
}

napi_value GetDatabaseDirWrap(napi_env env, napi_callback_info info, DatabaseDirCB *getDatabaseDirCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (getDatabaseDirCB == nullptr) {
        HILOG_ERROR("%{public}s, getDatabaseDirCB == nullptr.", __func__);
        return nullptr;
    }

    getDatabaseDirCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (getDatabaseDirCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetDatabaseDir, ability == nullptr");
        getDatabaseDirCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return nullptr;
    }

    std::string abilityName = getDatabaseDirCB->cbBase.ability->GetAbilityInfo()->name;
    std::string dataDir = getDatabaseDirCB->cbBase.ability->GetAbilityInfo()->applicationInfo.dataDir;
    std::shared_ptr<HapModuleInfo> hap = getDatabaseDirCB->cbBase.ability->GetHapModuleInfo();
    std::string moduleName = (hap != nullptr) ? hap->name : std::string();
    std::string dataDirWithModuleName = dataDir + NAPI_CONTEXT_FILE_SEPARATOR + moduleName;
    HILOG_INFO("%{public}s, dataDir:%{public}s moduleName:%{public}s abilityName:%{public}s",
        __func__,
        dataDir.c_str(),
        moduleName.c_str(),
        abilityName.c_str());

    // if dataDirWithModuleName is not exits, do nothing and return.
    if (!OHOS::FileExists(dataDirWithModuleName)) {
        getDatabaseDirCB->dataBaseDir = "";
        HILOG_INFO("%{public}s, dirWithModuleName is not exits:%{public}s, do nothing and return null.",
            __func__,
            dataDirWithModuleName.c_str());
    } else {
        getDatabaseDirCB->dataBaseDir = dataDirWithModuleName + NAPI_CONTEXT_FILE_SEPARATOR + abilityName +
                                        NAPI_CONTEXT_FILE_SEPARATOR + NAPI_CONTEXT_DATABASE;
        HILOG_INFO("%{public}s, GetDir dataBaseDir:%{public}s", __func__, getDatabaseDirCB->dataBaseDir.c_str());
        if (!OHOS::FileExists(getDatabaseDirCB->dataBaseDir)) {
            HILOG_INFO("NAPI_GetDatabaseDir dir is not exits, create dir.");
            OHOS::ForceCreateDirectory(getDatabaseDirCB->dataBaseDir);
            OHOS::ChangeModeDirectory(getDatabaseDirCB->dataBaseDir, MODE);
        }
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, getDatabaseDirCB->dataBaseDir.c_str(), NAPI_AUTO_LENGTH, &result));

    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

napi_value NAPI_GetDatabaseDirSync(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    DatabaseDirCB *getDatabaseDirCB = CreateGetDatabaseDirCBInfo(env);
    if (getDatabaseDirCB == nullptr) {
        return WrapVoidToJS(env);
    }

    getDatabaseDirCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetDatabaseDirWrap(env, info, getDatabaseDirCB);

    delete getDatabaseDirCB;
    getDatabaseDirCB = nullptr;

    if (ret == nullptr) {
        ret = WrapVoidToJS(env);
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
    } else {
        HILOG_INFO("%{public}s, end.", __func__);
    }
    return ret;
}

PreferencesDirCB *CreateGetPreferencesDirCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    PreferencesDirCB *getPreferencesDirCB = new (std::nothrow) PreferencesDirCB;
    if (getPreferencesDirCB == nullptr) {
        HILOG_ERROR("%{public}s, getPreferencesDirCB == nullptr.", __func__);
        return nullptr;
    }
    getPreferencesDirCB->cbBase.cbInfo.env = env;
    getPreferencesDirCB->cbBase.asyncWork = nullptr;
    getPreferencesDirCB->cbBase.deferred = nullptr;
    getPreferencesDirCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return getPreferencesDirCB;
}

napi_value GetPreferencesDirWrap(napi_env env, napi_callback_info info, PreferencesDirCB *getPreferencesDirCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (getPreferencesDirCB == nullptr) {
        HILOG_ERROR("%{public}s, getPreferencesDirCB == nullptr.", __func__);
        return nullptr;
    }

    getPreferencesDirCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (getPreferencesDirCB->cbBase.ability == nullptr) {
        HILOG_ERROR("%{public}s, ability == nullptr", __func__);
        getPreferencesDirCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return nullptr;
    }

    std::string abilityName = getPreferencesDirCB->cbBase.ability->GetAbilityInfo()->name;
    std::string dataDir = getPreferencesDirCB->cbBase.ability->GetAbilityInfo()->applicationInfo.dataDir;
    std::shared_ptr<HapModuleInfo> hap = getPreferencesDirCB->cbBase.ability->GetHapModuleInfo();
    std::string moduleName = (hap != nullptr) ? hap->name : std::string();
    std::string dataDirWithModuleName = dataDir + NAPI_CONTEXT_FILE_SEPARATOR + moduleName;
    HILOG_INFO("%{public}s, dataDir:%{public}s moduleName:%{public}s abilityName:%{public}s",
        __func__,
        dataDir.c_str(),
        moduleName.c_str(),
        abilityName.c_str());

    // if dataDirWithModuleName is not exits, do nothing and return.
    if (!OHOS::FileExists(dataDirWithModuleName)) {
        getPreferencesDirCB->preferencesDir = "";
        HILOG_INFO("%{public}s, dirWithModuleName is not exits:%{public}s, do nothing and return null.",
            __func__,
            dataDirWithModuleName.c_str());
    } else {
        getPreferencesDirCB->preferencesDir = dataDirWithModuleName + NAPI_CONTEXT_FILE_SEPARATOR + abilityName +
                                              NAPI_CONTEXT_FILE_SEPARATOR + NAPI_CONTEXT_PREFERENCES;
        HILOG_INFO(
            "%{public}s, GetDir preferencesDir:%{public}s", __func__, getPreferencesDirCB->preferencesDir.c_str());
        if (!OHOS::FileExists(getPreferencesDirCB->preferencesDir)) {
            HILOG_INFO("NAPI_GetPreferencesDir dir is not exits, create dir.");
            OHOS::ForceCreateDirectory(getPreferencesDirCB->preferencesDir);
            OHOS::ChangeModeDirectory(getPreferencesDirCB->preferencesDir, MODE);
        }
    }
    napi_value result = nullptr;
    NAPI_CALL(
        env, napi_create_string_utf8(env, getPreferencesDirCB->preferencesDir.c_str(), NAPI_AUTO_LENGTH, &result));

    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

napi_value NAPI_GetPreferencesDirSync(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    PreferencesDirCB *preferencesDirCB = CreateGetPreferencesDirCBInfo(env);
    if (preferencesDirCB == nullptr) {
        return WrapVoidToJS(env);
    }

    preferencesDirCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetPreferencesDirWrap(env, info, preferencesDirCB);

    delete preferencesDirCB;
    preferencesDirCB = nullptr;

    if (ret == nullptr) {
        ret = WrapVoidToJS(env);
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
    } else {
        HILOG_INFO("%{public}s, end.", __func__);
    }
    return ret;
}

napi_value NAPI_IsUpdatingConfigurations(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called", __func__);
    return NAPI_IsUpdatingConfigurationsCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_GetExternalCacheDir(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("%{public}s called", __func__);
    return NAPI_GetExternalCacheDirCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_PrintDrawnCompleted(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called", __func__);
    return NAPI_PrintDrawnCompletedCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_SetDisplayOrientation(napi_env env, napi_callback_info info)
{
#ifdef SUPPORT_GRAPHICS
    HILOG_DEBUG("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_WARN("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    napi_value rev = NAPI_SetDisplayOrientationWrap(env, info, asyncCallbackInfo);
    if (rev == nullptr) {
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        rev = WrapVoidToJS(env);
    }
    return rev;
#else
   return WrapVoidToJS(env);
#endif
}

napi_value NAPI_GetDisplayOrientation(napi_env env, napi_callback_info info)
{
#ifdef SUPPORT_GRAPHICS
    HILOG_DEBUG("%{public}s called.", __func__);
    return NAPI_GetDisplayOrientationCommon(env, info, AbilityType::PAGE);
#else
   return 0;
#endif
}

napi_value ContextPermissionInit(napi_env env, napi_value exports)
{
    HILOG_INFO("Context::ContextPermissionInit called.");

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("verifySelfPermission", NAPI_VerifySelfPermission),
        DECLARE_NAPI_FUNCTION("requestPermissionsFromUser", NAPI_RequestPermissionsFromUser),
        DECLARE_NAPI_FUNCTION("getBundleName", NAPI_GetBundleName),
        DECLARE_NAPI_FUNCTION("verifyPermission", NAPI_VerifyPermission),
        DECLARE_NAPI_FUNCTION("getApplicationInfo", NAPI_GetApplicationInfo),
        DECLARE_NAPI_FUNCTION("getProcessInfo", NAPI_GetProcessInfo),
        DECLARE_NAPI_FUNCTION("getElementName", NAPI_GetElementName),
        DECLARE_NAPI_FUNCTION("getProcessName", NAPI_GetProcessName),
        DECLARE_NAPI_FUNCTION("getCallingBundle", NAPI_GetCallingBundle),
        DECLARE_NAPI_FUNCTION("getOrCreateLocalDir", NAPI_GetOrCreateLocalDir),
        DECLARE_NAPI_FUNCTION("getFilesDir", NAPI_GetFilesDir),
        DECLARE_NAPI_FUNCTION("isUpdatingConfigurations", NAPI_IsUpdatingConfigurations),
        DECLARE_NAPI_FUNCTION("printDrawnCompleted", NAPI_PrintDrawnCompleted),
        DECLARE_NAPI_FUNCTION("getDatabaseDirSync", NAPI_GetDatabaseDirSync),
        DECLARE_NAPI_FUNCTION("getPreferencesDirSync", NAPI_GetPreferencesDirSync),
        DECLARE_NAPI_FUNCTION("getCacheDir", NAPI_GetCacheDir),
        DECLARE_NAPI_FUNCTION("getAppType", NAPI_GetCtxAppType),
        DECLARE_NAPI_FUNCTION("getHapModuleInfo", NAPI_GetCtxHapModuleInfo),
        DECLARE_NAPI_FUNCTION("getAppVersionInfo", NAPI_GetAppVersionInfo),
        DECLARE_NAPI_FUNCTION("getApplicationContext", NAPI_GetApplicationContext),
        DECLARE_NAPI_FUNCTION("getAbilityInfo", NAPI_GetCtxAbilityInfo),
        DECLARE_NAPI_FUNCTION("setShowOnLockScreen", NAPI_SetShowOnLockScreen),
        DECLARE_NAPI_FUNCTION("getOrCreateDistributedDir", NAPI_GetOrCreateDistributedDir),
        DECLARE_NAPI_FUNCTION("setWakeUpScreen", NAPI_SetWakeUpScreen),
        DECLARE_NAPI_FUNCTION("setDisplayOrientation", NAPI_SetDisplayOrientation),
        DECLARE_NAPI_FUNCTION("getDisplayOrientation", NAPI_GetDisplayOrientation),
        DECLARE_NAPI_FUNCTION("getExternalCacheDir", NAPI_GetExternalCacheDir),
    };

    NAPI_CALL(env,
        napi_define_class(env,
            "context",
            NAPI_AUTO_LENGTH,
            ContextConstructor,
            nullptr,
            sizeof(properties) / sizeof(*properties),
            properties,
            GetGlobalClassContext()));

    return exports;
}

napi_value NAPI_SetWakeUpScreen(napi_env env, napi_callback_info info)
{
#ifdef SUPPORT_GRAPHICS
    HILOG_INFO("%{public}s,called", __func__);
    SetWakeUpScreenCB *setWakeUpScreenCB = new (std::nothrow) SetWakeUpScreenCB;
    if (setWakeUpScreenCB == nullptr) {
        HILOG_ERROR("%{public}s, SetWakeUpScreenCB new failed", __func__);
        return WrapVoidToJS(env);
    }
    setWakeUpScreenCB->cbBase.cbInfo.env = env;
    setWakeUpScreenCB->cbBase.abilityType = AbilityType::PAGE;
    napi_value ret = SetWakeUpScreenWrap(env, info, setWakeUpScreenCB);
    if (ret == nullptr) {
        if (setWakeUpScreenCB != nullptr) {
            delete setWakeUpScreenCB;
            setWakeUpScreenCB = nullptr;
        }
        HILOG_ERROR("%{public}s, setWakeUpScreenCB run failed, delete resource", __func__);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,called end", __func__);
    return ret;
#else
   return nullptr;
#endif
}

class NapiJsContext : public JsNapiCommon {
public:
    NapiJsContext() = default;
    virtual ~NapiJsContext() = default;

    static void Finalizer(NativeEngine *engine, void *data, void *hint)
    {
        HILOG_DEBUG("called.");
        std::unique_ptr<NativeEngine>(static_cast<NativeEngine*>(data));
    };

    static NativeValue* JsRequestPermissionsFromUser(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetBundleName(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsVerifyPermission(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetApplicationInfo(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetProcessInfo(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetElementName(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetProcessName(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetCallingBundle(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetOrCreateLocalDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetFilesDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsIsUpdatingConfigurations(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsPrintDrawnCompleted(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetCacheDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetCtxAppType(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetCtxHapModuleInfo(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetAppVersionInfo(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetApplicationContext(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetCtxAbilityInfo(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsSetShowOnLockScreen(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetOrCreateDistributedDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsSetWakeUpScreen(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsSetDisplayOrientation(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetDisplayOrientation(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetExternalCacheDir(NativeEngine *engine, NativeCallbackInfo *info);

    bool DataInit(NativeEngine &engine);

private:
    NativeValue* OnRequestPermissionsFromUser(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetBundleName(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnVerifyPermission(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetApplicationInfo(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetProcessInfo(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetElementName(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetProcessName(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetCallingBundle(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetOrCreateLocalDir(NativeEngine &engine, NativeCallbackInfo &info);
#ifdef SUPPORT_GRAPHICS
    NativeValue* OnSetShowOnLockScreen(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnSetWakeUpScreen(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnSetDisplayOrientation(NativeEngine &engine, NativeCallbackInfo &info);
#endif
    int curCallbackKey_ = 0;
};

static bool BindNapiJSContextFunction(NativeEngine &engine, NativeObject* object)
{
    HILOG_DEBUG("called");
    if (object == nullptr) {
        HILOG_ERROR("input params error");
        return false;
    }
    const char* moduleName = "context";
    BindNativeFunction(
        engine, *object, "requestPermissionsFromUser", moduleName, NapiJsContext::JsRequestPermissionsFromUser);
    BindNativeFunction(engine, *object, "getBundleName", moduleName, NapiJsContext::JsGetBundleName);
    BindNativeFunction(engine, *object, "verifyPermission", moduleName, NapiJsContext::JsVerifyPermission);
    BindNativeFunction(engine, *object, "getApplicationInfo", moduleName, NapiJsContext::JsGetApplicationInfo);
    BindNativeFunction(engine, *object, "getProcessInfo", moduleName, NapiJsContext::JsGetProcessInfo);
    BindNativeFunction(engine, *object, "getElementName", moduleName, NapiJsContext::JsGetElementName);
    BindNativeFunction(engine, *object, "getProcessName", moduleName, NapiJsContext::JsGetProcessName);
    BindNativeFunction(engine, *object, "getCallingBundle", moduleName, NapiJsContext::JsGetCallingBundle);
    BindNativeFunction(engine, *object, "getOrCreateLocalDir", moduleName, NapiJsContext::JsGetOrCreateLocalDir);
    BindNativeFunction(engine, *object, "getFilesDir", moduleName, NapiJsContext::JsGetFilesDir);
    BindNativeFunction(engine, *object, "isUpdatingConfigurations", moduleName,
        NapiJsContext::JsIsUpdatingConfigurations);
    BindNativeFunction(engine, *object, "printDrawnCompleted", moduleName, NapiJsContext::JsPrintDrawnCompleted);
    BindNativeFunction(engine, *object, "getCacheDir", moduleName, NapiJsContext::JsGetCacheDir);
    BindNativeFunction(engine, *object, "getAppType", moduleName, NapiJsContext::JsGetCtxAppType);
    BindNativeFunction(engine, *object, "getHapModuleInfo", moduleName, NapiJsContext::JsGetCtxHapModuleInfo);
    BindNativeFunction(engine, *object, "getAppVersionInfo", moduleName, NapiJsContext::JsGetAppVersionInfo);
    BindNativeFunction(engine, *object, "getApplicationContext", moduleName, NapiJsContext::JsGetApplicationContext);
    BindNativeFunction(engine, *object, "getAbilityInfo", moduleName, NapiJsContext::JsGetCtxAbilityInfo);
    BindNativeFunction(engine, *object, "setShowOnLockScreen", moduleName, NapiJsContext::JsSetShowOnLockScreen);
    BindNativeFunction(engine, *object, "getOrCreateDistributedDir", moduleName,
        NapiJsContext::JsGetOrCreateDistributedDir);
    BindNativeFunction(engine, *object, "setWakeUpScreen", moduleName, NapiJsContext::JsSetWakeUpScreen);
    BindNativeFunction(engine, *object, "setDisplayOrientation", moduleName, NapiJsContext::JsSetDisplayOrientation);
    BindNativeFunction(engine, *object, "getDisplayOrientation", moduleName, NapiJsContext::JsGetDisplayOrientation);
    BindNativeFunction(engine, *object, "getExternalCacheDir", moduleName, NapiJsContext::JsGetExternalCacheDir);

    return true;
}

static NativeValue* ConstructNapiJSContext(NativeEngine &engine)
{
    HILOG_DEBUG("called");
    auto objContext = engine.CreateObject();
    if (objContext == nullptr) {
        HILOG_ERROR("CreateObject failed");
        return nullptr;
    }
    auto object = ConvertNativeValueTo<NativeObject>(objContext);
    if (object == nullptr) {
        HILOG_ERROR("ConvertNativeValueTo object failed");
        return nullptr;
    }
    auto jsCalss = std::make_unique<NapiJsContext>();
    if (!jsCalss->DataInit(engine)) {
        HILOG_ERROR("NapiJsContext init failed");
        return nullptr;
    }
    object->SetNativePointer(jsCalss.release(), NapiJsContext::Finalizer, nullptr);
    object->SetProperty("stageMode", engine.CreateBoolean(false));
    if (!BindNapiJSContextFunction(engine, object)) {
        HILOG_ERROR("bind func failed");
        return nullptr;
    }

    return objContext;
}

NativeValue* CreateNapiJSContext(NativeEngine &engine)
{
    HILOG_DEBUG("called");
    auto jsObj = ConstructNapiJSContext(engine);
    if (jsObj == nullptr) {
        HILOG_ERROR("Construct Context failed");
        return engine.CreateUndefined();
    }

    return jsObj;
}

NativeValue* NapiJsContext::JsRequestPermissionsFromUser(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->OnRequestPermissionsFromUser(*engine, *info);
}

NativeValue* NapiJsContext::JsGetBundleName(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->OnGetBundleName(*engine, *info);
}

NativeValue* NapiJsContext::JsVerifyPermission(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->OnVerifyPermission(*engine, *info);
}

NativeValue* NapiJsContext::JsGetApplicationInfo(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->OnGetApplicationInfo(*engine, *info);
}

NativeValue* NapiJsContext::JsGetProcessInfo(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->OnGetProcessInfo(*engine, *info);
}

NativeValue* NapiJsContext::JsGetElementName(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->OnGetElementName(*engine, *info);
}

NativeValue* NapiJsContext::JsGetProcessName(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->OnGetProcessName(*engine, *info);
}

NativeValue* NapiJsContext::JsGetCallingBundle(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->OnGetCallingBundle(*engine, *info);
}

NativeValue* NapiJsContext::JsGetOrCreateLocalDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->OnGetOrCreateLocalDir(*engine, *info);
}

NativeValue* NapiJsContext::JsGetFilesDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetFilesDir(*engine, *info, AbilityType::PAGE);
}

NativeValue* NapiJsContext::JsIsUpdatingConfigurations(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsIsUpdatingConfigurations(*engine, *info, AbilityType::PAGE);
}

NativeValue* NapiJsContext::JsPrintDrawnCompleted(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsPrintDrawnCompleted(*engine, *info, AbilityType::PAGE);
}

NativeValue* NapiJsContext::JsGetCacheDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetCacheDir(*engine, *info, AbilityType::PAGE);
}

NativeValue* NapiJsContext::JsGetCtxAppType(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetCtxAppType(*engine, *info, AbilityType::PAGE);
}

NativeValue* NapiJsContext::JsGetCtxHapModuleInfo(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetCtxHapModuleInfo(*engine, *info, AbilityType::PAGE);
}

NativeValue* NapiJsContext::JsGetAppVersionInfo(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetAppVersionInfo(*engine, *info, AbilityType::PAGE);
}

NativeValue* NapiJsContext::JsGetApplicationContext(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetContext(*engine, *info, AbilityType::PAGE);
}

NativeValue* NapiJsContext::JsGetCtxAbilityInfo(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetCtxAbilityInfo(*engine, *info, AbilityType::PAGE);
}

NativeValue* NapiJsContext::JsSetShowOnLockScreen(NativeEngine *engine, NativeCallbackInfo *info)
{
#ifdef SUPPORT_GRAPHICS
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->OnSetShowOnLockScreen(*engine, *info);
#else
   return nullptr;
#endif
}

NativeValue* NapiJsContext::JsGetOrCreateDistributedDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetOrCreateDistributedDir(*engine, *info, AbilityType::PAGE);
}

NativeValue* NapiJsContext::JsSetWakeUpScreen(NativeEngine *engine, NativeCallbackInfo *info)
{
#ifdef SUPPORT_GRAPHICS
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->OnSetWakeUpScreen(*engine, *info);
#else
   return nullptr;
#endif
}

NativeValue* NapiJsContext::JsSetDisplayOrientation(NativeEngine *engine, NativeCallbackInfo *info)
{
#ifdef SUPPORT_GRAPHICS
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->OnSetDisplayOrientation(*engine, *info);
#else
   return nullptr;
#endif
}

NativeValue* NapiJsContext::JsGetDisplayOrientation(NativeEngine *engine, NativeCallbackInfo *info)
{
#ifdef SUPPORT_GRAPHICS
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetDisplayOrientation(*engine, *info, AbilityType::PAGE);
#else
   return nullptr;
#endif
}

NativeValue* NapiJsContext::JsGetExternalCacheDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    CHECK_POINTER_AND_RETURN_LOG(engine, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(engine, info);
    CHECK_POINTER_AND_RETURN_LOG(object, engine->CreateUndefined(), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetExternalCacheDir(*engine, *info, AbilityType::PAGE);
}

bool NapiJsContext::DataInit(NativeEngine &engine)
{
    HILOG_DEBUG("called");
    napi_value global = nullptr;
    napi_value abilityObj = nullptr;
    auto env = reinterpret_cast<napi_env>(&engine);
    HILOG_INFO("Get Ability to start");
    NAPI_CALL_BASE(env, napi_get_global(env, &global), false);
    NAPI_CALL_BASE(env, napi_get_named_property(env, global, "ability", &abilityObj), false);
    NAPI_CALL_BASE(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability_)), false);
    HILOG_INFO("Get Ability to done");

    return true;
}

NativeValue* NapiJsContext::OnRequestPermissionsFromUser(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("OnRequestPermissionsFromUser called");
    if (info.argc == ARGS_ZERO || info.argc > ARGS_THREE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }
    std::vector<std::string> permission_list;
    if (!GetStringsValue(engine, info.argv[PARAM0], permission_list)) {
        HILOG_ERROR("input params string error");
        return engine.CreateUndefined();
    }
    int requestCode = 0;
    if (!ConvertFromJsValue(engine, info.argv[PARAM1], requestCode)) {
        HILOG_ERROR("input params int error");
        return engine.CreateUndefined();
    }

    auto callback = info.argc == ARGS_THREE ? info.argv[PARAM2] : nullptr;
    NativeValue *result = nullptr;

    std::unique_ptr<AbilityRuntime::AsyncTask> uasyncTask =
        AbilityRuntime::CreateAsyncTaskWithLastParam(engine, callback, nullptr, nullptr, &result);
    std::shared_ptr<AbilityRuntime::AsyncTask> asyncTask = std::move(uasyncTask);

    PermissionRequestTask task = [&engine, asyncTask, requestCode]
        (const std::vector<std::string> &permissions, const std::vector<int> &grantResults) {
        HILOG_DEBUG("OnRequestPermissionsFromUser async callback is called");
        NativeValue *objValue = engine.CreateObject();
        NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);

        object->SetProperty("requestCode", CreateJsValue(engine, requestCode));
        object->SetProperty("permissions", CreateNativeArray(engine, permissions));
        object->SetProperty("authResults", CreateNativeArray(engine, grantResults));
        asyncTask->Resolve(engine, objValue);
        HILOG_DEBUG("OnRequestPermissionsFromUser async callback is called end");
    };
    int32_t errorCode = NAPI_ERR_NO_ERROR;
    if (ability_ == nullptr) {
        HILOG_ERROR("OnRequestPermissionsFromUser ability is nullptr.");
        errorCode = NAPI_ERR_ACE_ABILITY;
    }

    if (permission_list.size() == 0) {
        HILOG_ERROR("OnRequestPermissionsFromUser permission_list size is 0");
        errorCode = NAPI_ERR_PARAM_INVALID;
    }

    if (errorCode != NAPI_ERR_NO_ERROR) {
        asyncTask->Reject(engine, CreateJsError(engine, errorCode, ConvertErrorCode(errorCode)));
    }
    curCallbackKey_ = (curCallbackKey_ == INT_MAX) ? 0 : (curCallbackKey_ + 1);
    AbilityProcess::RequestPermissionsParamInfo param;
    param.permissionList = permission_list;
    param.callbackKey = curCallbackKey_;
    AbilityProcess::GetInstance()->RequestPermissionsFromUser(engine, ability_, param, std::move(task));

    return result;
}

NativeValue* NapiJsContext::OnGetBundleName(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateNull();
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsBundleName> bundleName = std::make_shared<JsBundleName>();
    auto execute = [obj = this, name = bundleName, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the ability is nullptr");
            return;
        }
        if (name == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            HILOG_ERROR("task execute error, name is nullptr");
            return;
        }
        name->name = obj->ability_->GetBundleName();
    };
    auto complete = [obj = this, name = bundleName, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || name == nullptr) {
            auto ecode = name == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(engine, CreateJsValue(engine, name->name));
    };

    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("NapiJsContext::OnGetBundleName",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* NapiJsContext::OnVerifyPermission(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("called");
    if (info.argc == ARGS_ZERO || info.argc > ARGS_THREE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateNull();
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::string permission("");
    if (!ConvertFromJsValue(engine, info.argv[PARAM0], permission)) {
        HILOG_ERROR("input params string error");
        return engine.CreateNull();
    }
    JsPermissionOptions options;
    bool flagCall = UnwarpVerifyPermissionParams(engine, info, options);
    auto execute = [obj = this, permission, options, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the ability is nullptr");
            return;
        }
        if (options.uidFlag) {
            *value = obj->ability_->VerifyPermission(permission, options.pid, options.uid);
        } else {
            *value = obj->ability_->VerifySelfPermission(permission);
        }
    };
    auto complete = [obj = this, value = errorVal] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value == static_cast<int32_t>(NAPI_ERR_ACE_ABILITY)) {
            task.Reject(engine, CreateJsError( engine, *value, obj->ConvertErrorCode(*value)));
            return;
        }
        task.Resolve(engine, CreateJsValue(engine, *value));
    };

    auto callback = flagCall ?
        ((info.argc == ARGS_TWO) ? info.argv[PARAM1] : info.argv[PARAM2]) :
        nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("NapiJsContext::OnGetBundleName",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* NapiJsContext::OnGetApplicationInfo(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsApplicationInfo> infoData = std::make_shared<JsApplicationInfo>();
    auto execute = [obj = this, info = infoData, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the ability is nullptr");
            return;
        }
        auto getInfo = obj->ability_->GetApplicationInfo();
        if (getInfo != nullptr && info != nullptr) {
            info->appInfo = *getInfo;
        } else {
            HILOG_ERROR("GetApplicationInfo return nullptr");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
        }
    };
    auto complete = [obj = this, info = infoData, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || info == nullptr) {
            auto ecode = info == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(engine, obj->CreateAppInfo(engine, info->appInfo));
    };

    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("NapiJsContext::OnGetApplicationInfo",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* NapiJsContext::OnGetProcessInfo(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsProcessInfo> processInfo = std::make_shared<JsProcessInfo>();
    auto execute = [obj = this, data = processInfo, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the ability is nullptr");
            return;
        }
        auto getInfo = obj->ability_->GetProcessInfo();
        if (getInfo != nullptr && data != nullptr) {
            data->processName = getInfo->GetProcessName();
            data->pid = getInfo->GetPid();
        } else {
            HILOG_ERROR("GetProcessInfo return nullptr");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
        }
    };
    auto complete = [obj = this, info = processInfo, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || info == nullptr) {
            auto ecode = info == nullptr ? (NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(engine, obj->CreateProcessInfo(engine, info));
    };

    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("NapiJsContext::OnGetProcessInfo",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* NapiJsContext::OnGetElementName(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsElementName> elementName = std::make_shared<JsElementName>();
    auto execute = [obj = this, data = elementName, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the ability is nullptr");
            return;
        }
        auto elementName = obj->ability_->GetElementName();
        if (elementName != nullptr && data != nullptr) {
            data->deviceId = elementName->GetDeviceID();
            data->bundleName = elementName->GetBundleName();
            data->abilityName = elementName->GetAbilityName();
            data->uri = obj->ability_->GetWant()->GetUriString();
            data->shortName = "";
        } else {
            HILOG_ERROR("GetElementName return nullptr");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
        }
    };
    auto complete = [obj = this, ename = elementName, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || ename == nullptr) {
            auto ecode = ename == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(engine, obj->CreateElementName(engine, ename));
    };

    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("NapiJsContext::OnGetElementName",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* NapiJsContext::OnGetProcessName(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsProcessName> processName = std::make_shared<JsProcessName>();
    auto execute = [obj = this, name = processName, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the ability is nullptr");
            return;
        }
        if (name == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            HILOG_ERROR("task execute error, name is nullptr");
            return;
        }
        name->name = obj->ability_->GetProcessName();
    };
    auto complete = [obj = this, name = processName, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || name == nullptr) {
            auto ecode = name == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(engine, CreateJsValue(engine, name->name));
    };

    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("NapiJsContext::OnGetProcessName",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* NapiJsContext::OnGetCallingBundle(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsCallingBundleName> callingBundleName = std::make_shared<JsCallingBundleName>();
    auto execute = [obj = this, name = callingBundleName, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the ability is nullptr");
            return;
        }
        if (name == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            HILOG_ERROR("task execute error, name is nullptr");
            return;
        }
        name->name = obj->ability_->GetCallingBundle();
    };
    auto complete = [obj = this, name = callingBundleName, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || name == nullptr) {
            auto ecode = name == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(engine, CreateJsValue(engine, name->name));
    };

    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("NapiJsContext::OnGetCallingBundle",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* NapiJsContext::OnGetOrCreateLocalDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsOrCreateLocalDir> createDir = std::make_shared<JsOrCreateLocalDir>();
    auto execute = [obj = this, dir = createDir, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the ability is nullptr");
            return;
        }
        auto context = obj->ability_->GetAbilityContext();
        if (context == nullptr || dir == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            HILOG_ERROR("task execute error, the abilitycontext is nullptr");
            return;
        }
        dir->name = context->GetBaseDir();
        if (!OHOS::FileExists(dir->name)) {
            HILOG_INFO("dir is not exits, create dir.");
            OHOS::ForceCreateDirectory(dir->name);
            OHOS::ChangeModeDirectory(dir->name, MODE);
        }
    };
    auto complete = [obj = this, dir = createDir, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || dir == nullptr) {
            auto ecode = dir == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(engine, CreateJsValue(engine, dir->name));
    };
    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("NapiJsContext::OnGetOrCreateLocalDir",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

#ifdef SUPPORT_GRAPHICS
NativeValue* NapiJsContext::OnSetShowOnLockScreen(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("called");
    if (info.argc == ARGS_ZERO || info.argc > ARGS_TWO) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    bool isShow = false;
    if (!ConvertFromJsValue(engine, info.argv[PARAM0], isShow)) {
        HILOG_ERROR("input params int error");
        return engine.CreateUndefined();
    }
    auto complete = [obj = this, isShow, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (obj->ability_ == nullptr) {
            task.Reject(engine,
                CreateJsError(engine, static_cast<int32_t>(NAPI_ERR_ACE_ABILITY), "get ability error"));
            return;
        }
        obj->ability_->SetShowOnLockScreen(isShow);
        task.Resolve(engine, engine.CreateUndefined());
    };

    auto callback = info.argc == ARGS_ONE ? nullptr : info.argv[PARAM1];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("NapiJsContext::OnSetShowOnLockScreen",
        engine, CreateAsyncTaskWithLastParam(engine, callback, nullptr, std::move(complete), &result));

    return result;
}

NativeValue* NapiJsContext::OnSetWakeUpScreen(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("called");
    if (info.argc == ARGS_ZERO || info.argc > ARGS_TWO) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    bool wakeUp = false;
    if (!ConvertFromJsValue(engine, info.argv[PARAM0], wakeUp)) {
        HILOG_ERROR("input params int error");
        return engine.CreateUndefined();
    }
    auto complete = [obj = this, wakeUp]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (obj->ability_ == nullptr) {
            task.Reject(engine,
                CreateJsError(engine, static_cast<int32_t>(NAPI_ERR_ACE_ABILITY), "get ability error"));
            return;
        }
        obj->ability_->SetWakeUpScreen(wakeUp);
        task.Resolve(engine, engine.CreateUndefined());
    };

    auto callback = info.argc == ARGS_ONE ? nullptr : info.argv[PARAM1];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("NapiJsContext::OnSetWakeUpScreen",
        engine, CreateAsyncTaskWithLastParam(engine, callback, nullptr, std::move(complete), &result));

    return result;
}

NativeValue* NapiJsContext::OnSetDisplayOrientation(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("called");
    if (info.argc == ARGS_ZERO || info.argc > ARGS_TWO) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    int32_t orientation = 0;
    if (!ConvertFromJsValue(engine, info.argv[PARAM0], orientation)) {
        HILOG_ERROR("input params int error");
        return engine.CreateUndefined();
    }

    int32_t maxRange = 3;
    if (orientation < 0 || orientation > maxRange) {
        HILOG_ERROR("wrong parameter orientation : %{public}d", orientation);
        return engine.CreateNull();
    }
    auto complete = [obj = this, orientationData = orientation]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (obj->ability_ == nullptr) {
            task.Reject(engine,
                CreateJsError(engine, static_cast<int32_t>(NAPI_ERR_ACE_ABILITY), "get ability error"));
            return;
        }
        obj->ability_->SetDisplayOrientation(orientationData);
        task.Resolve(engine, engine.CreateUndefined());
    };

    auto callback = info.argc == ARGS_ONE ? nullptr : info.argv[PARAM1];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("NapiJsContext::SetDisplayOrientation",
        engine, CreateAsyncTaskWithLastParam(engine, callback, nullptr, std::move(complete), &result));

    return result;
}
#endif
}  // namespace AppExecFwk
}  // namespace OHOS
