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

#include "napi_common_ability.h"

#include <chrono>
#include <dlfcn.h>
#include <memory>
#include <uv.h>

#include "ability_util.h"
#include "hilog_tag_wrapper.h"
#include "js_napi_common_ability.h"
#include "js_runtime_utils.h"
#include "napi_common_ability_execute_utils.h"
#include "napi_common_ability_wrap_utils.h"
#include "napi_common_util.h"
#include "napi_context.h"
#include "napi_base_context.h"
#include "napi_remote_object.h"
#include "securec.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AppExecFwk {
napi_ref thread_local g_contextObject = nullptr;
napi_ref thread_local g_dataAbilityHelper = nullptr;
bool thread_local g_dataAbilityHelperStatus = false;

using NAPICreateJsRemoteObject = napi_value (*)(napi_env env, const sptr<IRemoteObject> target);

napi_status SetGlobalClassContext(napi_env env, napi_value constructor)
{
    return napi_create_reference(env, constructor, 1, &g_contextObject);
}

napi_value GetGlobalClassContext(napi_env env)
{
    napi_value constructor;
    NAPI_CALL(env, napi_get_reference_value(env, g_contextObject, &constructor));
    return constructor;
}

napi_status SaveGlobalDataAbilityHelper(napi_env env, napi_value constructor)
{
    return napi_create_reference(env, constructor, 1, &g_dataAbilityHelper);
}

napi_value GetGlobalDataAbilityHelper(napi_env env)
{
    napi_value constructor;
    NAPI_CALL(env, napi_get_reference_value(env, g_dataAbilityHelper, &constructor));
    return constructor;
}

bool& GetDataAbilityHelperStatus()
{
    return g_dataAbilityHelperStatus;
}

napi_value NAPI_GetFilesDirWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "start");
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid args[PARAM0]");
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
        asyncParamEx.resource = "NAPI_GetFilesDirCallback";
        asyncParamEx.execute = GetFilesDirExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "promise");
        asyncParamEx.resource = "NAPI_GetFilesDirPromise";
        asyncParamEx.execute = GetFilesDirExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_GetFilesDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null asyncCallbackInfo");
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetFilesDirWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }

    return ret;
}

/**
 * @brief GetFilesDir processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NAPI_GetOrCreateDistributedDirWrap(
    napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "start");
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid args[PARAM0]");
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
        asyncParamEx.resource = "NAPI_GetFilesDirCallback";
        asyncParamEx.execute = GetOrCreateDistributedDirExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "promise");
        asyncParamEx.resource = "NAPI_GetFilesDirPromise";
        asyncParamEx.execute = GetOrCreateDistributedDirExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_GetOrCreateDistributedDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null asyncCallback");
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetOrCreateDistributedDirWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }

    return ret;
}

/**
 * @brief NAPI_GetCacheDirWrap processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NAPI_GetCacheDirWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "begin");
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid args[PARAM0]");
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
        asyncParamEx.resource = "NAPI_GetCacheDirCallback";
        asyncParamEx.execute = GetCacheDirExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "promise");
        asyncParamEx.resource = "NAPI_GetCacheDirPromise";
        asyncParamEx.execute = GetCacheDirExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_GetCacheDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null asyncCallbackInfo");
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetCacheDirWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }

    return ret;
}

/**
 * @brief NAPI_GetExternalCacheDirWrap processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NAPI_GetExternalCacheDirWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "begin");
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "args[PARAM0] invalid");
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
        asyncParamEx.resource = "NAPI_GetExternalCacheDirCallback";
        asyncParamEx.execute = GetExternalCacheDirExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "promise");
        asyncParamEx.resource = "NAPI_GetExternalCacheDirPromise";
        asyncParamEx.execute = GetExternalCacheDirExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_GetExternalCacheDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "create callbackInfo failed");
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetExternalCacheDirWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }

    return ret;
}

napi_value NAPI_IsUpdatingConfigurationsWrap(
    napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        TAG_LOGI(AAFwkTag::JSNAPI, "invalid argc");
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "args[PARAM0] invalid");
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
        asyncParamEx.resource = "NAPI_IsUpdatingConfigurationsCallback";
        asyncParamEx.execute = IsUpdatingConfigurationsExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "promise");
        asyncParamEx.resource = "NAPI_IsUpdatingConfigurationsPromise";
        asyncParamEx.execute = IsUpdatingConfigurationsExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_PrintDrawnCompletedWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "begin");
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "args[PARAM0] invalid");
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
        asyncParamEx.resource = "NAPI_PrintDrawnCompletedCallback";
        asyncParamEx.execute = PrintDrawnCompletedExecuteCallback;
        asyncParamEx.complete = CompleteAsyncVoidCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "promise");
        asyncParamEx.resource = "NAPI_PrintDrawnCompletedPromise";
        asyncParamEx.execute = PrintDrawnCompletedExecuteCallback;
        asyncParamEx.complete = CompletePromiseVoidCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_IsUpdatingConfigurationsCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "create callbackInfo failed");
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_IsUpdatingConfigurationsWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }

    return ret;
}

napi_value NAPI_PrintDrawnCompletedCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "create callbackInfo failed");
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_PrintDrawnCompletedWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }

    return ret;
}

/**
 * @brief Obtains the type of this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAppTypeCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AppTypeCB *appTypeCB = CreateAppTypeCBInfo(env);
    if (appTypeCB == nullptr) {
        return WrapVoidToJS(env);
    }

    appTypeCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    appTypeCB->cbBase.abilityType = abilityType;
    napi_value ret = GetAppTypeWrap(env, info, appTypeCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        if (appTypeCB != nullptr) {
            delete appTypeCB;
            appTypeCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }

    return ret;
}

#ifdef SUPPORT_GRAPHICS
napi_value GetDisplayOrientationWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamGetDisplayOrientationWrap(env, argc, args, asyncCallbackInfo)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "unwrapParamGetDisplayOrientationWrap fail");
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
        asyncParamEx.resource = "NAPI_GetDisplayOrientationWrapCallback";
        asyncParamEx.execute = GetDisplayOrientationExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "promise");
        asyncParamEx.resource = "NAPI_GetDisplayOrientationWrapPromise";
        asyncParamEx.execute = GetDisplayOrientationExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

void GetDisplayOrientationExecuteCallback(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null asyncCallbackInfo");
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;

    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid type");
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_INT32;
    asyncCallbackInfo->native_data.int32_value = asyncCallbackInfo->ability->GetDisplayOrientation();
}

bool UnwrapParamGetDisplayOrientationWrap(napi_env env, size_t argc, napi_value *argv,
    AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "argc=%{public}zu", argc);
    const size_t argcMax = 1;
    if (argc > argcMax || argc < argcMax - 1) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
        return false;
    }

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM0], asyncCallbackInfo)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "args[PARAM0] invalid");
            return false;
        }
    }

    return true;
}

napi_value NAPI_GetDisplayOrientationCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "create callbackInfo failed");
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = GetDisplayOrientationWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
    return ret;
}
#endif

/**
 * @brief Obtains information about the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AbilityInfoCB *abilityInfoCB = CreateAbilityInfoCBInfo(env);
    if (abilityInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    abilityInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    abilityInfoCB->cbBase.abilityType = abilityType;
    napi_value ret = GetAbilityInfoWrap(env, info, abilityInfoCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        if (abilityInfoCB != nullptr) {
            delete abilityInfoCB;
            abilityInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }

    return ret;
}

/**
 * @brief Obtains the HapModuleInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetHapModuleInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    HapModuleInfoCB *hapModuleInfoCB = CreateHapModuleInfoCBInfo(env);
    if (hapModuleInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    hapModuleInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    hapModuleInfoCB->cbBase.abilityType = abilityType;
    napi_value ret = GetHapModuleInfoWrap(env, info, hapModuleInfoCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        if (hapModuleInfoCB != nullptr) {
            delete hapModuleInfoCB;
            hapModuleInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }

    return ret;
}

/**
 * @brief Obtains the AppVersionInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAppVersionInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AppVersionInfoCB *appVersionInfoCB = CreateAppVersionInfoCBInfo(env);
    if (appVersionInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    appVersionInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    appVersionInfoCB->cbBase.abilityType = abilityType;
    napi_value ret = GetAppVersionInfoWrap(env, info, appVersionInfoCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        if (appVersionInfoCB != nullptr) {
            delete appVersionInfoCB;
            appVersionInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }

    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AsyncCallbackInfo on success, nullptr on failure
 */
AsyncCallbackInfo *CreateAsyncCallbackInfo(napi_env env)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    if (env == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "null env");
        return nullptr;
    }

    napi_status ret;
    napi_value global = nullptr;
    const napi_extended_error_info *errorInfo = nullptr;
    ret = napi_get_global(env, &global);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        TAG_LOGE(AAFwkTag::JSNAPI, "get_global=%{public}d err:%{public}s", ret,
                 errorInfo->error_message);
    }

    napi_value abilityObj = nullptr;
    ret = napi_get_named_property(env, global, "ability", &abilityObj);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        TAG_LOGE(AAFwkTag::JSNAPI, "get_named_property=%{public}d err:%{public}s", ret,
                 errorInfo->error_message);
    }

    Ability *ability = nullptr;
    ret = napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability));
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        TAG_LOGE(AAFwkTag::JSNAPI, "get_value_external=%{public}d err:%{public}s", ret,
                 errorInfo->error_message);
    }

    AsyncCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo;
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null asyncCallbackInfo");
        return nullptr;
    }
    asyncCallbackInfo->cbInfo.env = env;
    asyncCallbackInfo->asyncWork = nullptr;
    asyncCallbackInfo->deferred = nullptr;
    asyncCallbackInfo->ability = ability;
    asyncCallbackInfo->native_result = false;
    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = AbilityType::UNKNOWN;

    return asyncCallbackInfo;
}

void GetContextAsyncExecuteCB(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "execute");
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null asyncCallbackInfo");
        return;
    }
    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "wrong ability type");
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "execute end");
}

napi_value GetContextAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null params");
        return nullptr;
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argCallback], &valuetype);
    if (valuetype == napi_function) {
        TAG_LOGD(AAFwkTag::JSNAPI, "napi_create_reference");
        napi_create_reference(env, args[argCallback], 1, &asyncCallbackInfo->cbInfo.callback);
    }
    napi_create_async_work(env, nullptr, resourceName, GetContextAsyncExecuteCB,
        [](napi_env env, napi_status, void *data) {
            TAG_LOGI(AAFwkTag::JSNAPI, "complete");
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            napi_value callback = nullptr;
            napi_value undefined = nullptr;
            napi_value result[ARGS_TWO] = {nullptr};
            napi_value callResult = nullptr;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
            if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
                napi_new_instance(env, GetGlobalClassContext(env), 0, nullptr, &result[PARAM1]);
            } else {
                result[PARAM1] = WrapUndefinedToJS(env);
            }
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                TAG_LOGD(AAFwkTag::JSNAPI, "Delete GetContextAsync callback reference");
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
            TAG_LOGI(AAFwkTag::JSNAPI, "complete end");
        }, static_cast<void *>(asyncCallbackInfo), &asyncCallbackInfo->asyncWork);
    napi_queue_async_work_with_qos(env, asyncCallbackInfo->asyncWork, napi_qos_user_initiated);
    napi_value result = nullptr;
    napi_get_null(env, &result);
    TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback end");
    return result;
}

napi_value GetContextPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "promise");
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null param");
        return nullptr;
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);
    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        GetContextAsyncExecuteCB,
        [](napi_env env, napi_status, void *data) {
            TAG_LOGI(AAFwkTag::JSNAPI, "complete");
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            napi_value result = nullptr;
            if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
                napi_new_instance(env, GetGlobalClassContext(env), 0, nullptr, &result);
                napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            } else {
                result = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
                napi_reject_deferred(env, asyncCallbackInfo->deferred, result);
            }

            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
            TAG_LOGI(AAFwkTag::JSNAPI, "complete end");
        },
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work_with_qos(env, asyncCallbackInfo->asyncWork, napi_qos_user_initiated);
    TAG_LOGI(AAFwkTag::JSNAPI, "promise end");
    return promise;
}

/**
 * @brief GetContext processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetContextWrap(napi_env env, napi_callback_info, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null asyncCallbackInfo");
        return nullptr;
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    if (!CheckAbilityType(asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "wrong ability type");
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return nullptr;
    }

    napi_value result = nullptr;
    napi_new_instance(env, GetGlobalClassContext(env), 0, nullptr, &result);

    return result;
}

/**
 * @brief Get context.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetContextCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null asyncCallbackInfo");
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = GetContextWrap(env, info, asyncCallbackInfo);

    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;

    if (ret == nullptr) {
        ret = WrapVoidToJS(env);
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
    }
    return ret;
}

/**
 * @brief Get want.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetWantCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null asyncCallbackInfo");
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = GetWantWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    return ret;
}

/**
 * @brief Obtains the class name in this ability name, without the prefixed bundle name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityNameCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AbilityNameCB *abilityNameCB = CreateAbilityNameCBInfo(env);
    if (abilityNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null abilityNameCB");
        return WrapVoidToJS(env);
    }

    abilityNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    abilityNameCB->cbBase.abilityType = abilityType;
    napi_value ret = GetAbilityNameWrap(env, info, abilityNameCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        if (abilityNameCB != nullptr) {
            delete abilityNameCB;
            abilityNameCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }

    return ret;
}

/**
 * @brief stopAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_StopAbilityCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "create callbackInfo failed");
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = StopAbilityWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }

    return ret;
}

/**
 * @brief AcquireDataAbilityHelper.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_AcquireDataAbilityHelperCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    DataAbilityHelperCB *dataAbilityHelperCB = new DataAbilityHelperCB;
    dataAbilityHelperCB->cbBase.cbInfo.env = env;
    dataAbilityHelperCB->cbBase.ability = nullptr; // temporary value assignment
    dataAbilityHelperCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    dataAbilityHelperCB->cbBase.abilityType = abilityType;
    napi_value ret = AcquireDataAbilityHelperWrap(env, info, dataAbilityHelperCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        if (dataAbilityHelperCB != nullptr) {
            delete dataAbilityHelperCB;
            dataAbilityHelperCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }

    return ret;
}

/**
 * @brief acquireDataAbilityHelper processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param dataAbilityHelperCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value AcquireDataAbilityHelperWrap(napi_env env, napi_callback_info info, DataAbilityHelperCB *dataAbilityHelperCB)
{
    if (dataAbilityHelperCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null dataAbilityHelperCB");
        return nullptr;
    }

    size_t requireArgc = ARGS_TWO;
    size_t argc = ARGS_TWO;
    napi_value args[ARGS_TWO] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc > requireArgc) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
        return nullptr;
    }

    size_t uriIndex = PARAM0;
    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, args[0], stageMode);
    if (status == napi_ok) {
        uriIndex = PARAM1;
        TAG_LOGI(AAFwkTag::JSNAPI, "argv[0] is a context, Stage Model: %{public}d", stageMode);
    }

    if (!stageMode) {
        auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "Failed to get native context instance");
            return nullptr;
        }
        dataAbilityHelperCB->cbBase.ability = ability;

        if (!CheckAbilityType(&dataAbilityHelperCB->cbBase)) {
            dataAbilityHelperCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
            TAG_LOGE(AAFwkTag::JSNAPI, "ability type invalid");
            return nullptr;
        }
    }
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[uriIndex], &valuetype));
    if (valuetype != napi_string) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type");
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_new_instance(env, GetGlobalDataAbilityHelper(env), uriIndex + 1, &args[PARAM0], &result));

    if (!IsTypeForNapiValue(env, result, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "result not object");
        return nullptr;
    }

    if (IsTypeForNapiValue(env, result, napi_null)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "result null");
        return nullptr;
    }

    if (IsTypeForNapiValue(env, result, napi_undefined)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "result undefined");
        return nullptr;
    }

    if (!GetDataAbilityHelperStatus()) {
        TAG_LOGE(AAFwkTag::JSNAPI, "GetDataAbilityHelperStatus false");
        return nullptr;
    }

    delete dataAbilityHelperCB;
    dataAbilityHelperCB = nullptr;
    return result;
}

napi_value NAPI_StartBackgroundRunningCommon(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null asyncCallbackInfo");
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = StartBackgroundRunningWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "finish");
    return ret;
}

napi_value NAPI_CancelBackgroundRunningCommon(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null asyncCallbackInfo");
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = CancelBackgroundRunningWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ret");
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }

    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
