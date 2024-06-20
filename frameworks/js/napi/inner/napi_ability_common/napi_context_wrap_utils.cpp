/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "napi_context_wrap_utils.h"

#include "ability_process.h"
#include "directory_ex.h"
#include "file_ex.h"
#include "hilog_tag_wrapper.h"
#include "napi_common_ability_wrap_utils.h"

namespace OHOS {
namespace AppExecFwk {
const std::string NAPI_CONTEXT_FILE_SEPARATOR = std::string("/");
const std::string NAPI_CONTEXT_DATABASE = std::string("database");
const std::string NAPI_CONTEXT_PREFERENCES = std::string("preferences");

#ifdef SUPPORT_GRAPHICS
Ability* GetJSAbilityObject(napi_env env)
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
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    ShowOnLockScreenCB *showOnLockScreenCB = static_cast<ShowOnLockScreenCB *>(data);
    if (showOnLockScreenCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input param is nullptr");
        return;
    }

    showOnLockScreenCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (showOnLockScreenCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input param is nullptr");
        showOnLockScreenCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
    } else {
#ifdef SUPPORT_SCREEN
        showOnLockScreenCB->cbBase.ability->SetShowOnLockScreen(showOnLockScreenCB->isShow);
#endif
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

    TAG_LOGD(AAFwkTag::JSNAPI, "called end");
}

napi_value SetShowOnLockScreenAsync(napi_env env, napi_value *args, ShowOnLockScreenCB *showOnLockScreenCB)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    if (showOnLockScreenCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input param is nullptr");
        return nullptr;
    }

    napi_valuetype valuetypeParam1 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetypeParam1));
    if (valuetypeParam1 != napi_function) {
        TAG_LOGE(AAFwkTag::JSNAPI, "error, params is error type");
        return nullptr;
    }

    NAPI_CALL(env, napi_create_reference(env, args[PARAM1], 1, &showOnLockScreenCB->cbBase.cbInfo.callback));

    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName,
            [](napi_env env, void *data) {
                TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_SetShowOnLockScreen, worker pool thread execute.");
            },
            SetShowOnLockScreenAsyncCompleteCB,
            static_cast<void *>(showOnLockScreenCB),
            &showOnLockScreenCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, showOnLockScreenCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));

    TAG_LOGI(AAFwkTag::JSNAPI, "called end");
    return result;
}

napi_value SetShowOnLockScreenPromise(napi_env env, ShowOnLockScreenCB *cbData)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "promise.");
    if (cbData == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "param == nullptr.");
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
        [](napi_env env, void *data) {
            TAG_LOGI(AAFwkTag::JSNAPI, "SetShowOnLockScreenPromise, worker pool thread execute.");
        },
        [](napi_env env, napi_status status, void *data) {
            ShowOnLockScreenCB *showOnLockScreenCB = static_cast<ShowOnLockScreenCB *>(data);
            showOnLockScreenCB->cbBase.errCode = NO_ERROR;
            if (showOnLockScreenCB->cbBase.ability == nullptr) {
                TAG_LOGE(AAFwkTag::JSNAPI, "input param is nullptr");
                showOnLockScreenCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
            } else {
#ifdef SUPPORT_SCREEN
                showOnLockScreenCB->cbBase.ability->SetShowOnLockScreen(showOnLockScreenCB->isShow);
#endif
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
            TAG_LOGI(AAFwkTag::JSNAPI, "SetShowOnLockScreenPromise, main event thread complete end.");
        },
        static_cast<void *>(cbData),
        &cbData->cbBase.asyncWork);
    napi_queue_async_work_with_qos(env, cbData->cbBase.asyncWork, napi_qos_user_initiated);
    TAG_LOGI(AAFwkTag::JSNAPI, "promise end.");
    return promise;
}

void SetDisplayOrientationExecuteCallbackWork(napi_env env, void *data)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called.");
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "called. asyncCallbackInfo is null");
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }
#ifdef SUPPORT_SCREEN
    int orientation = asyncCallbackInfo->param.paramArgs.GetIntValue("orientation");
    asyncCallbackInfo->ability->SetDisplayOrientation(orientation);
#endif
    asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
}

bool UnwrapSetDisplayOrientation(napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called, argc=%{public}zu", argc);

    const size_t argcMax = 2;
    if (argc > argcMax || argc < argcMax - 1) {
        TAG_LOGE(AAFwkTag::JSNAPI, "called, Params is invalid.");
        return false;
    }

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM1], asyncCallbackInfo)) {
            TAG_LOGD(AAFwkTag::JSNAPI, "called, the second parameter is invalid.");
            return false;
        }
    }

    int orientation = 0;
    if (!UnwrapInt32FromJS2(env, argv[PARAM0], orientation)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "called, the parameter is invalid.");
        return false;
    }

    int maxRange = 3;
    if (orientation < 0 || orientation > maxRange) {
        TAG_LOGE(AAFwkTag::JSNAPI, "called, wrong parameter range.");
        return false;
    }

    asyncCallbackInfo->param.paramArgs.PutIntValue("orientation", orientation);
    return true;
}

static void SetWakeUpScreenAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    SetWakeUpScreenCB *setWakeUpScreenCB = static_cast<SetWakeUpScreenCB *>(data);
    if (setWakeUpScreenCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, input param is nullptr", __func__);
        return;
    }

    do {
        setWakeUpScreenCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
        if (setWakeUpScreenCB->cbBase.ability == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, input param is nullptr", __func__);
            setWakeUpScreenCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
            break;
        }
#ifdef SUPPORT_SCREEN
        setWakeUpScreenCB->cbBase.ability->SetWakeUpScreen(setWakeUpScreenCB->wakeUp);
#endif
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
}

static napi_value SetWakeUpScreenAsync(napi_env env, napi_value *args, SetWakeUpScreenCB *cbData)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    if (cbData == nullptr || args == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input param is nullptr");
        return nullptr;
    }

    napi_valuetype valuetypeParam0 = napi_undefined;
    napi_valuetype valuetypeParam1 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetypeParam0));
    NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetypeParam1));
    if (valuetypeParam0 != napi_boolean || valuetypeParam1 != napi_function) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Params is error type");
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
            [](napi_env env, void *data) {
                TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_SetWakeUpScreenScreen, worker pool thread execute.");
            },
            SetWakeUpScreenAsyncCompleteCB,
            static_cast<void *>(cbData),
            &cbData->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, cbData->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

static napi_value SetWakeUpScreenPromise(napi_env env, SetWakeUpScreenCB *cbData)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "promise.");
    if (cbData == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "param == nullptr.");
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
        [](napi_env env, void *data) {
            TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_SetWakeUpScreenScreen, worker pool thread execute.");
        },
        [](napi_env env, napi_status status, void *data) {
            TAG_LOGI(AAFwkTag::JSNAPI, "SetWakeUpScreenPromise, main event thread complete.");
            SetWakeUpScreenCB *setWakeUpScreenCB = static_cast<SetWakeUpScreenCB *>(data);
            setWakeUpScreenCB->cbBase.errCode = NO_ERROR;
            if (setWakeUpScreenCB->cbBase.ability == nullptr) {
                TAG_LOGE(AAFwkTag::JSNAPI, "input param is nullptr");
                setWakeUpScreenCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
            } else {
#ifdef SUPPORT_SCREEN
                setWakeUpScreenCB->cbBase.ability->SetWakeUpScreen(setWakeUpScreenCB->wakeUp);
#endif
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
            TAG_LOGI(AAFwkTag::JSNAPI, "SetWakeUpScreenPromise, main event thread complete end.");
        },
        static_cast<void *>(cbData),
        &cbData->cbBase.asyncWork);
    napi_queue_async_work(env, cbData->cbBase.asyncWork);
    return promise;
}

napi_value SetWakeUpScreenWrap(napi_env env, napi_callback_info info, SetWakeUpScreenCB *cbData)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    if (cbData == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input param cbData is nullptr");
        return nullptr;
    }

    size_t argcAsync = 2;
    const size_t argStdValue = 2;
    const size_t argPromise = 1;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync != argStdValue && argcAsync != argPromise) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument count.");
        return nullptr;
    }

    if (!UnwrapBoolFromJS2(env, args[PARAM0], cbData->wakeUp)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "UnwrapBoolFromJS2(wakeUp) run error");
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
    return ret;
}
#endif

bool UnwrapParamVerifySelfPermission(
    napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called, argc=%{public}zu", argc);

    const size_t argcMax = 2;
    if (argc > argcMax || argc < argcMax - 1) {
        TAG_LOGI(AAFwkTag::JSNAPI, "called, Params is invalid.");
        return false;
    }

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM1], asyncCallbackInfo)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "called, the second parameter is invalid.");
            return false;
        }
    }

    std::string permission("");
    if (!UnwrapStringFromJS2(env, argv[PARAM0], permission)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "called, the first parameter is invalid.");
        return false;
    }

    asyncCallbackInfo->param.paramArgs.PutStringValue("permission", permission);
    return true;
}

void VerifySelfPermissionExecuteCallbackWork(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "start.");

    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "called. asyncCallbackInfo is nullptr");
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

bool UnwrapRequestPermissionsFromUser(
    napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called, argc=%{public}zu", argc);

    const size_t argcMax = 3;
    if (argc > argcMax || argc < argcMax - 1) {
        TAG_LOGI(AAFwkTag::JSNAPI, "called, parameters is invalid");
        return false;
    }

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM2], asyncCallbackInfo)) {
            TAG_LOGD(AAFwkTag::JSNAPI, "called, the third parameter is invalid.");
            return false;
        }
    }

    std::vector<std::string> permissionList;
    if (!UnwrapArrayStringFromJS(env, argv[PARAM0], permissionList)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "called, the first parameter is invalid.");
        return false;
    }

    int requestCode = 0;
    if (!UnwrapInt32FromJS2(env, argv[PARAM1], requestCode)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "called, the second parameter is invalid.");
        return false;
    }

    asyncCallbackInfo->param.paramArgs.PutIntValue("requestCode", requestCode);
    asyncCallbackInfo->param.paramArgs.PutStringValueArray("permissionList", permissionList);
    return true;
}

void RequestPermissionsFromUserExecuteCallbackWork(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called.");
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "called. asyncCallbackInfo is null");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "called.");

    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "called, asyncCallbackInfo is null");
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

static bool UnwrapVerifyPermissionOptions(napi_env env, napi_value argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called, asyncCallbackInfo is null", __func__);
        return false;
    }

    if (!IsTypeForNapiValue(env, argv, napi_object)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called, type of parameter is error.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called, argc=%{public}zu", __func__, argc);

    const size_t argcMax = ARGS_THREE;
    if (argc > argcMax || argc < 1) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called, Params is invalid.", __func__);
        return false;
    }

    std::string permission("");
    if (!UnwrapStringFromJS2(env, argv[PARAM0], permission)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called, the first parameter is invalid.", __func__);
        return false;
    }
    asyncCallbackInfo->param.paramArgs.PutStringValue("permission", permission);

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM2], asyncCallbackInfo)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called, the second parameter is invalid.", __func__);
            return false;
        }

        if (!UnwrapVerifyPermissionOptions(env, argv[PARAM1], asyncCallbackInfo)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called, the second parameter is invalid.", __func__);
            return false;
        }
    } else if (argc == ARGS_TWO) {
        if (!CreateAsyncCallback(env, argv[PARAM1], asyncCallbackInfo)) {
            if (!UnwrapVerifyPermissionOptions(env, argv[PARAM1], asyncCallbackInfo)) {
                TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called, the second parameter is invalid.", __func__);
                return false;
            }
        }
    } else if (argc == ARGS_ONE) {
        asyncCallbackInfo->cbInfo.callback = nullptr;
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called, the parameter is invalid.", __func__);
        return false;
    }
    return true;
}

void VerifyPermissionExecuteCallback(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. asyncCallbackInfo is null", __func__);
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

static void GetAppInfoExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, worker pool thread execute.");
    AppInfoCB *appInfoCB = static_cast<AppInfoCB *>(data);
    appInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;

    if (appInfoCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, ability == nullptr");
        appInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    std::shared_ptr<ApplicationInfo> appInfoPtr = appInfoCB->cbBase.ability->GetApplicationInfo();
    if (appInfoPtr != nullptr) {
        appInfoCB->appInfo = *appInfoPtr;
    } else {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, appInfoPtr == nullptr");
        appInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, worker pool thread execute end.");
}

static void GetAppInfoAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, main event thread complete end.");
}

static napi_value GetApplicationInfoAsync(napi_env env, napi_value *args, const size_t argCallback, AppInfoCB *appInfoCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || appInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, appInfoCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
    return result;
}

static void GetAppInfoPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, main event thread complete.");
    AppInfoCB *appInfoCB = static_cast<AppInfoCB *>(data);
    if (appInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, appInfoCB == nullptr");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, main event thread complete end.");
}

static napi_value GetApplicationInfoPromise(napi_env env, AppInfoCB *appInfoCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise.", __func__);
    if (appInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, appInfoCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetApplicationInfoWrap(napi_env env, napi_callback_info info, AppInfoCB *appInfoCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (appInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, appInfoCB == null.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, Wrong parameter count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetApplicationInfoAsync(env, args, 0, appInfoCB);
    } else {
        ret = GetApplicationInfoPromise(env, appInfoCB);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
    return ret;
}

AppInfoCB *CreateAppInfoCBInfo(napi_env env)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    AppInfoCB *appInfoCB = new (std::nothrow) AppInfoCB;
    if (appInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, appInfoCB == nullptr.", __func__);
        return nullptr;
    }
    appInfoCB->cbBase.cbInfo.env = env;
    appInfoCB->cbBase.asyncWork = nullptr;
    appInfoCB->cbBase.deferred = nullptr;
    appInfoCB->cbBase.ability = ability;
    appInfoCB->cbBase.abilityType = AbilityType::UNKNOWN;
    appInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    return appInfoCB;
}

void GetBundleNameExecuteCallback(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ability == nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_STRING;
    asyncCallbackInfo->native_data.str_value = asyncCallbackInfo->ability->GetBundleName();
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end. bundleName=%{public}s", __func__,
             asyncCallbackInfo->native_data.str_value.c_str());
}

static napi_value WrapProcessInfo(napi_env env, ProcessInfoCB *processInfoCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    if (processInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s Invalid param(processInfoCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_create_int32(env, processInfoCB->pid, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "pid", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, processInfoCB->processName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "processName", proValue));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end", __func__);
    return result;
}

static void GetProcessInfoExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetProcessInfo, worker pool thread execute.");
    ProcessInfoCB *processInfoCB = static_cast<ProcessInfoCB *>(data);
    if (processInfoCB == nullptr) {
        return;
    }

    processInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (processInfoCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetProcessInfo, ability == nullptr");
        processInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    std::shared_ptr<ProcessInfo> processInfoPtr = processInfoCB->cbBase.ability->GetProcessInfo();
    if (processInfoPtr != nullptr) {
        processInfoCB->processName = processInfoPtr->GetProcessName();
        processInfoCB->pid = processInfoPtr->GetPid();
    } else {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetProcessInfo, processInfoPtr == nullptr");
        processInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetProcessInfo, worker pool thread execute end.");
}

static void GetProcessInfoAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetProcessInfo, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetProcessInfo, main event thread complete end.");
}

static napi_value GetProcessInfoAsync(napi_env env, napi_value *args, const size_t argCallback, ProcessInfoCB *processInfoCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || processInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, processInfoCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
    return result;
}

static void GetProcessInfoPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetProcessInfo, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetProcessInfo, main event thread complete end.");
}

static napi_value GetProcessInfoPromise(napi_env env, ProcessInfoCB *processInfoCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise.", __func__);
    if (processInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, processInfoCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetProcessInfoWrap(napi_env env, napi_callback_info info, ProcessInfoCB *processInfoCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (processInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, processInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, argument count fail.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetProcessInfoAsync(env, args, 0, processInfoCB);
    } else {
        ret = GetProcessInfoPromise(env, processInfoCB);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
    return ret;
}

ProcessInfoCB *CreateProcessInfoCBInfo(napi_env env)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    ProcessInfoCB *processInfoCB = new (std::nothrow) ProcessInfoCB;
    if (processInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, processInfoCB == nullptr.", __func__);
        return nullptr;
    }
    processInfoCB->cbBase.cbInfo.env = env;
    processInfoCB->cbBase.asyncWork = nullptr;
    processInfoCB->cbBase.deferred = nullptr;
    processInfoCB->cbBase.ability = ability;

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    return processInfoCB;
}

ElementNameCB *CreateElementNameCBInfo(napi_env env)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    ElementNameCB *elementNameCB = new (std::nothrow) ElementNameCB;
    if (elementNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, elementNameCB == nullptr.", __func__);
        return nullptr;
    }
    elementNameCB->cbBase.cbInfo.env = env;
    elementNameCB->cbBase.asyncWork = nullptr;
    elementNameCB->cbBase.deferred = nullptr;
    elementNameCB->cbBase.ability = ability;

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    return elementNameCB;
}

static napi_value WrapElementName(napi_env env, const ElementNameCB *elementNameCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
    if (elementNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s,Invalid param(appInfoCB = nullptr)", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    return result;
}

static void GetElementNameExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetElementName, worker pool thread execute.");
    if (data == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, data == nullptr.", __func__);
        return;
    }
    ElementNameCB *elementNameCB = static_cast<ElementNameCB *>(data);
    if (elementNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetElementName, elementNameCB == nullptr");
        return;
    }

    elementNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (elementNameCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetElementName, ability == nullptr");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetElementName, worker pool thread execute end.");
}

static void GetElementNameAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetElementName, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetElementName, main event thread complete end.");
}

static void GetElementNamePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetElementName, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetElementName, main event thread complete end.");
}

static napi_value GetElementNamePromise(napi_env env, ElementNameCB *elementNameCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise.", __func__);
    if (elementNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, elementNameCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise end.", __func__);
    return promise;
}

static napi_value GetElementNameAsync(napi_env env, napi_value *args, const size_t argCallback, ElementNameCB *elementNameCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || elementNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, elementNameCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value GetElementNameWrap(napi_env env, napi_callback_info info, ElementNameCB *elementNameCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (elementNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, appInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, argument count fail.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetElementNameAsync(env, args, 0, elementNameCB);
    } else {
        ret = GetElementNamePromise(env, elementNameCB);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
    return ret;
}

ProcessNameCB *CreateProcessNameCBInfo(napi_env env)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    ProcessNameCB *processNameCB = new (std::nothrow) ProcessNameCB;
    if (processNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, processNameCB == nullptr.", __func__);
        return nullptr;
    }
    processNameCB->cbBase.cbInfo.env = env;
    processNameCB->cbBase.asyncWork = nullptr;
    processNameCB->cbBase.deferred = nullptr;
    processNameCB->cbBase.ability = ability;

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return processNameCB;
}

static void GetProcessNameExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetProcessName, worker pool thread execute.");
    ProcessNameCB *processNameCB = static_cast<ProcessNameCB *>(data);
    if (processNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetProcessName, processNameCB == nullptr");
        return;
    }

    processNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (processNameCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetProcessName, ability == nullptr");
        processNameCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    processNameCB->processName = processNameCB->cbBase.ability->GetProcessName();
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetProcessName, worker pool thread execute end.");
}

static napi_value WrapProcessName(napi_env env, const ProcessNameCB *processNameCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
    if (processNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, Invalid param(processNameCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, processNameCB->processName.c_str(), NAPI_AUTO_LENGTH, &result));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    return result;
}

static void GetProcessNameAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetProcessName, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetProcessName, main event thread complete end.");
}

static void GetProcessNamePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetProcessName, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetProcessName, main event thread complete end.");
}

static napi_value GetProcessNameAsync(napi_env env, napi_value *args, const size_t argCallback, ProcessNameCB *processNameCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || processNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, processNameCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
    return result;
}

static napi_value GetProcessNamePromise(napi_env env, ProcessNameCB *processNameCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise.", __func__);
    if (processNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, processNameCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetProcessNameWrap(napi_env env, napi_callback_info info, ProcessNameCB *processNameCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (processNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, processNameCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, parameter count error.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetProcessNameAsync(env, args, 0, processNameCB);
    } else {
        ret = GetProcessNamePromise(env, processNameCB);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
    return ret;
}

CallingBundleCB *CreateCallingBundleCBInfo(napi_env env)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    CallingBundleCB *callingBundleCB = new (std::nothrow) CallingBundleCB;
    if (callingBundleCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, callingBundleCB == nullptr.", __func__);
        return nullptr;
    }
    callingBundleCB->cbBase.cbInfo.env = env;
    callingBundleCB->cbBase.asyncWork = nullptr;
    callingBundleCB->cbBase.deferred = nullptr;
    callingBundleCB->cbBase.ability = ability;

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return callingBundleCB;
}

static void GetCallingBundleExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetCallingBundle, worker pool thread execute.");
    CallingBundleCB *callingBundleCB = static_cast<CallingBundleCB *>(data);
    if (callingBundleCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetCallingBundle, callingBundleCB == nullptr");
        return;
    }

    callingBundleCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (callingBundleCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetCallingBundle, ability == nullptr");
        callingBundleCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    callingBundleCB->callingBundleName = callingBundleCB->cbBase.ability->GetCallingBundle();
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetCallingBundle, worker pool thread execute end.");
}

static napi_value WrapCallingBundle(napi_env env, const CallingBundleCB *callingBundleCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
    if (callingBundleCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s,Invalid param(callingBundleCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, callingBundleCB->callingBundleName.c_str(), NAPI_AUTO_LENGTH, &result));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    return result;
}

static void GetCallingBundleAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetCallingBundle, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetCallingBundle, main event thread complete end.");
}

static void GetCallingBundlePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetCallingBundle, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetCallingBundle, main event thread complete end.");
}

static napi_value GetCallingBundleAsync(
    napi_env env, napi_value *args, const size_t argCallback, CallingBundleCB *callingBundleCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || callingBundleCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, callingBundleCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
    return result;
}

static napi_value GetCallingBundlePromise(napi_env env, CallingBundleCB *callingBundleCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise.", __func__);
    if (callingBundleCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, callingBundleCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetCallingBundleWrap(napi_env env, napi_callback_info info, CallingBundleCB *callingBundleCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (callingBundleCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, callingBundleCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, parameter count fail.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetCallingBundleAsync(env, args, 0, callingBundleCB);
    } else {
        ret = GetCallingBundlePromise(env, callingBundleCB);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
    return ret;
}

GetOrCreateLocalDirCB *CreateGetOrCreateLocalDirCBInfo(napi_env env)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    GetOrCreateLocalDirCB *getOrCreateLocalDirCB = new (std::nothrow) GetOrCreateLocalDirCB;
    if (getOrCreateLocalDirCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, getOrCreateLocalDirCB == nullptr.", __func__);
        return nullptr;
    }
    getOrCreateLocalDirCB->cbBase.cbInfo.env = env;
    getOrCreateLocalDirCB->cbBase.asyncWork = nullptr;
    getOrCreateLocalDirCB->cbBase.deferred = nullptr;
    getOrCreateLocalDirCB->cbBase.ability = ability;

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return getOrCreateLocalDirCB;
}

static void GetOrCreateLocalDirExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetOrCreateLocalDir, worker pool thread execute.");
    GetOrCreateLocalDirCB *getOrCreateLocalDirCB = static_cast<GetOrCreateLocalDirCB *>(data);
    if (getOrCreateLocalDirCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetOrCreateLocalDir, callingBundleCB == nullptr");
        return;
    }

    getOrCreateLocalDirCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (getOrCreateLocalDirCB->cbBase.ability == nullptr ||
        getOrCreateLocalDirCB->cbBase.ability->GetAbilityContext() == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetOrCreateLocalDir, ability or abilityContext is nullptr");
        getOrCreateLocalDirCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    getOrCreateLocalDirCB->rootDir = getOrCreateLocalDirCB->cbBase.ability->GetAbilityContext()->GetBaseDir();
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetOrCreateLocalDir, GetDir rootDir:%{public}s",
             getOrCreateLocalDirCB->rootDir.c_str());
    if (!OHOS::FileExists(getOrCreateLocalDirCB->rootDir)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetOrCreateLocalDir dir is not exits, create dir.");
        OHOS::ForceCreateDirectory(getOrCreateLocalDirCB->rootDir);
        OHOS::ChangeModeDirectory(getOrCreateLocalDirCB->rootDir, MODE);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetOrCreateLocalDir, worker pool thread execute end.");
}

static napi_value WrapGetOrCreateLocalDir(napi_env env, const GetOrCreateLocalDirCB *getOrCreateLocalDirCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
    if (getOrCreateLocalDirCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s,Invalid param(getOrCreateLocalDirCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, getOrCreateLocalDirCB->rootDir.c_str(), NAPI_AUTO_LENGTH, &result));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    return result;
}

static void GetOrCreateLocalDirAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetOrCreateLocalDir, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetOrCreateLocalDir, main event thread complete end.");
}

static void GetOrCreateLocalDirPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetOrCreateLocalDir, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetOrCreateLocalDir, main event thread complete end.");
}

static napi_value GetOrCreateLocalDirAsync(
    napi_env env, napi_value *args, const size_t argCallback, GetOrCreateLocalDirCB *getOrCreateLocalDirCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || getOrCreateLocalDirCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
    return result;
}

static napi_value GetOrCreateLocalDirPromise(napi_env env, GetOrCreateLocalDirCB *getOrCreateLocalDirCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise.", __func__);
    if (getOrCreateLocalDirCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetOrCreateLocalDirWrap(napi_env env, napi_callback_info info, GetOrCreateLocalDirCB *getOrCreateLocalDirCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (getOrCreateLocalDirCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, getOrCreateLocalDirCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetOrCreateLocalDirAsync(env, args, 0, getOrCreateLocalDirCB);
    } else {
        ret = GetOrCreateLocalDirPromise(env, getOrCreateLocalDirCB);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
    return ret;
}

DatabaseDirCB *CreateGetDatabaseDirCBInfo(napi_env env)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    DatabaseDirCB *getDatabaseDirCB = new (std::nothrow) DatabaseDirCB;
    if (getDatabaseDirCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, getDatabaseDirCB == nullptr.", __func__);
        return nullptr;
    }
    getDatabaseDirCB->cbBase.cbInfo.env = env;
    getDatabaseDirCB->cbBase.asyncWork = nullptr;
    getDatabaseDirCB->cbBase.deferred = nullptr;
    getDatabaseDirCB->cbBase.ability = ability;

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return getDatabaseDirCB;
}

napi_value GetDatabaseDirWrap(napi_env env, napi_callback_info info, DatabaseDirCB *getDatabaseDirCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
    if (getDatabaseDirCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, getDatabaseDirCB == nullptr.", __func__);
        return nullptr;
    }

    getDatabaseDirCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (getDatabaseDirCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetDatabaseDir, ability == nullptr");
        getDatabaseDirCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return nullptr;
    }

    std::string abilityName = getDatabaseDirCB->cbBase.ability->GetAbilityInfo()->name;
    std::string dataDir = getDatabaseDirCB->cbBase.ability->GetAbilityInfo()->applicationInfo.dataDir;
    std::shared_ptr<HapModuleInfo> hap = getDatabaseDirCB->cbBase.ability->GetHapModuleInfo();
    std::string moduleName = (hap != nullptr) ? hap->name : std::string();
    std::string dataDirWithModuleName = dataDir + NAPI_CONTEXT_FILE_SEPARATOR + moduleName;
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, dataDir:%{public}s moduleName:%{public}s abilityName:%{public}s",
        __func__,
        dataDir.c_str(),
        moduleName.c_str(),
        abilityName.c_str());

    // if dataDirWithModuleName is not exits, do nothing and return.
    if (!OHOS::FileExists(dataDirWithModuleName)) {
        getDatabaseDirCB->dataBaseDir = "";
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, dirWithModuleName is not exits:%{public}s, do nothing and return null.",
            __func__,
            dataDirWithModuleName.c_str());
    } else {
        getDatabaseDirCB->dataBaseDir = dataDirWithModuleName + NAPI_CONTEXT_FILE_SEPARATOR + abilityName +
                                        NAPI_CONTEXT_FILE_SEPARATOR + NAPI_CONTEXT_DATABASE;
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, GetDir dataBaseDir:%{public}s", __func__,
                 getDatabaseDirCB->dataBaseDir.c_str());
        if (!OHOS::FileExists(getDatabaseDirCB->dataBaseDir)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetDatabaseDir dir is not exits, create dir.");
            OHOS::ForceCreateDirectory(getDatabaseDirCB->dataBaseDir);
            OHOS::ChangeModeDirectory(getDatabaseDirCB->dataBaseDir, MODE);
        }
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, getDatabaseDirCB->dataBaseDir.c_str(), NAPI_AUTO_LENGTH, &result));

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    return result;
}

PreferencesDirCB *CreateGetPreferencesDirCBInfo(napi_env env)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    PreferencesDirCB *getPreferencesDirCB = new (std::nothrow) PreferencesDirCB;
    if (getPreferencesDirCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, getPreferencesDirCB == nullptr.", __func__);
        return nullptr;
    }
    getPreferencesDirCB->cbBase.cbInfo.env = env;
    getPreferencesDirCB->cbBase.asyncWork = nullptr;
    getPreferencesDirCB->cbBase.deferred = nullptr;
    getPreferencesDirCB->cbBase.ability = ability;

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return getPreferencesDirCB;
}

napi_value GetPreferencesDirWrap(napi_env env, napi_callback_info info, PreferencesDirCB *getPreferencesDirCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
    if (getPreferencesDirCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, getPreferencesDirCB == nullptr.", __func__);
        return nullptr;
    }

    getPreferencesDirCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (getPreferencesDirCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, ability == nullptr", __func__);
        getPreferencesDirCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return nullptr;
    }

    std::string abilityName = getPreferencesDirCB->cbBase.ability->GetAbilityInfo()->name;
    std::string dataDir = getPreferencesDirCB->cbBase.ability->GetAbilityInfo()->applicationInfo.dataDir;
    std::shared_ptr<HapModuleInfo> hap = getPreferencesDirCB->cbBase.ability->GetHapModuleInfo();
    std::string moduleName = (hap != nullptr) ? hap->name : std::string();
    std::string dataDirWithModuleName = dataDir + NAPI_CONTEXT_FILE_SEPARATOR + moduleName;
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, dataDir:%{public}s moduleName:%{public}s abilityName:%{public}s",
        __func__,
        dataDir.c_str(),
        moduleName.c_str(),
        abilityName.c_str());

    // if dataDirWithModuleName is not exits, do nothing and return.
    if (!OHOS::FileExists(dataDirWithModuleName)) {
        getPreferencesDirCB->preferencesDir = "";
        TAG_LOGI(AAFwkTag::JSNAPI,
            "%{public}s, dirWithModuleName is not exits:%{public}s, do nothing and return null.",
            __func__,
            dataDirWithModuleName.c_str());
    } else {
        getPreferencesDirCB->preferencesDir = dataDirWithModuleName + NAPI_CONTEXT_FILE_SEPARATOR + abilityName +
                                              NAPI_CONTEXT_FILE_SEPARATOR + NAPI_CONTEXT_PREFERENCES;
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, GetDir preferencesDir:%{public}s", __func__,
                 getPreferencesDirCB->preferencesDir.c_str());
        if (!OHOS::FileExists(getPreferencesDirCB->preferencesDir)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetPreferencesDir dir is not exits, create dir.");
            OHOS::ForceCreateDirectory(getPreferencesDirCB->preferencesDir);
            OHOS::ChangeModeDirectory(getPreferencesDirCB->preferencesDir, MODE);
        }
    }
    napi_value result = nullptr;
    NAPI_CALL(
        env, napi_create_string_utf8(env, getPreferencesDirCB->preferencesDir.c_str(), NAPI_AUTO_LENGTH, &result));

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    return result;
}
}
}