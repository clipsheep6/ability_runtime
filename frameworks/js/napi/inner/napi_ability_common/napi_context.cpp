/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "napi_common_ability.h"
#include "hilog_tag_wrapper.h"
#include "napi_context_wrap_utils.h"
#include "napi_js_context.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {

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
napi_value NAPI_SetDisplayOrientationWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called.");
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapSetDisplayOrientation(env, argc, args, asyncCallbackInfo)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "called. Invoke UnwrapSetDisplayOrientation fail");
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        asyncParamEx.resource = "NAPI_SetDisplayOrientationCallback";
        asyncParamEx.execute = SetDisplayOrientationExecuteCallbackWork;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        asyncParamEx.resource = "NAPI_SetDisplayOrientationPromise";
        asyncParamEx.execute = SetDisplayOrientationExecuteCallbackWork;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}
#endif

napi_value NAPI_SetShowOnLockScreen(napi_env env, napi_callback_info info)
{
#ifdef SUPPORT_GRAPHICS
    TAG_LOGI(AAFwkTag::JSNAPI, "called");

    size_t argc = 2;
    const size_t argcAsync = 2, argcPromise = 1;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc != argcAsync && argc != argcPromise) {
        TAG_LOGE(AAFwkTag::JSNAPI, "error, wrong argument count.");
        return nullptr;
    }

    napi_valuetype valuetypeParam0 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetypeParam0));
    if (valuetypeParam0 != napi_boolean) {
        TAG_LOGE(AAFwkTag::JSNAPI, "error, params is error type");
        return nullptr;
    }

    ShowOnLockScreenCB *showOnLockScreenCB = new ShowOnLockScreenCB();
    showOnLockScreenCB->cbBase.cbInfo.env = env;
    showOnLockScreenCB->cbBase.abilityType = AbilityType::PAGE;
    if (!UnwrapBoolFromJS2(env, args[PARAM0], showOnLockScreenCB->isShow)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "error, unwrapBoolFromJS2 error");
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
        TAG_LOGE(AAFwkTag::JSNAPI, "SetShowOnLockScreenWrap failed.");
        delete showOnLockScreenCB;
        showOnLockScreenCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    return ret;
#else
   return nullptr;
#endif
}

napi_value NAPI_VerifySelfPermissionWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called.");
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamVerifySelfPermission(env, argc, args, asyncCallbackInfo)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "called. Invoke UnwrapParamVerifySelfPermission fail");
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        asyncParamEx.resource = "NAPI_VerifySelfPermissionCallback";
        asyncParamEx.execute = VerifySelfPermissionExecuteCallbackWork;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        asyncParamEx.resource = "NAPI_VerifySelfPermissionPromise";
        asyncParamEx.execute = VerifySelfPermissionExecuteCallbackWork;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_VerifySelfPermission(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called.");

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

napi_value NAPI_RequestPermissionsFromUserWrap(
    napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called.");
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapRequestPermissionsFromUser(env, argc, args, asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "called. Invoke UnwrapRequestPermissionsFromUser failed.");
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        asyncParamEx.resource = "NAPI_RequestPermissionsFromUserCallback";
        asyncParamEx.execute = RequestPermissionsFromUserExecuteCallbackWork;
        asyncParamEx.complete = RequestPermissionsFromUserCompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
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

        NAPI_CALL(env, napi_queue_async_work_with_qos(env, asyncCallbackInfo->asyncWork, napi_qos_user_initiated));

        return promise;
    }
}

napi_value NAPI_RequestPermissionsFromUser(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called.");

    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "called. Invoke CreateAsyncJSCallbackInfo failed.");
        return WrapVoidToJS(env);
    }

    napi_value rev = NAPI_RequestPermissionsFromUserWrap(env, info, asyncCallbackInfo);
    if (rev == nullptr) {
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        rev = WrapVoidToJS(env);
    }
    return rev;
}

struct OnRequestPermissionsData {
    int requestCode = 0;
    std::vector<std::string> permissions;
    std::vector<int> grantResults;
    uv_work_t uvWork{};
    NapiAsyncTask *napiAsyncTask = nullptr;
    napi_env env = nullptr;

    ~OnRequestPermissionsData()
    {
        if (napiAsyncTask) {
            delete napiAsyncTask;
        }
    }

    static void WorkCallback(uv_work_t* work)
    {
        TAG_LOGI(AAFwkTag::JSNAPI, "called env");
    }

    static void AfterWorkCallback(uv_work_t* work, int status)
    {
        TAG_LOGD(AAFwkTag::JSNAPI, "OnRequestPermissionsFromUserResult called");
        if (work == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, work is nullptr.", __func__);
            return;
        }
        if (work->data == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, work data is nullptr.", __func__);
            return;
        }
        std::unique_ptr<OnRequestPermissionsData> data{static_cast<OnRequestPermissionsData *>(work->data)};
        auto env = data->env;
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);
        napi_value object = nullptr;
        napi_create_object(env, &object);
        napi_set_named_property(env, object, "requestCode", CreateJsValue(env, data->requestCode));
        napi_set_named_property(env, object, "permissions", CreateNativeArray(env, data->permissions));
        napi_set_named_property(env, object, "authResults", CreateNativeArray(env, data->grantResults));
        data->napiAsyncTask->Resolve(env, object);
        napi_close_handle_scope(env, scope);
    }
};

EXTERN_C_START
void CallOnRequestPermissionsFromUserResult(int requestCode, const std::vector<std::string> &permissions,
    const std::vector<int> &grantResults, CallbackInfo callbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s,called env", __func__);
    if (permissions.empty()) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, the string vector permissions is empty.", __func__);
        return;
    }
    if (permissions.size() != grantResults.size()) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, the size of permissions not equal the size of grantResults.", __func__);
        return;
    }
    if (callbackInfo.env == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CallOnRequestPermissionsFromUserResult callbackInfo.env is nullptr.");
        return;
    }
    if (callbackInfo.napiAsyncTask == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CallOnRequestPermissionsFromUserResult callbackInfo.napiAsyncTask is nullptr.");
        return;
    }

    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo.env, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CallOnRequestPermissionsFromUserResult loop is nullptr.");
        return;
    }

    auto reqData = std::make_unique<OnRequestPermissionsData>();
    reqData->permissions = permissions;
    reqData->grantResults = grantResults;
    reqData->requestCode = requestCode;
    reqData->env = callbackInfo.env;
    reqData->napiAsyncTask = callbackInfo.napiAsyncTask;
    reqData->uvWork.data = static_cast<void *>(reqData.get());

    int rev = uv_queue_work_with_qos(loop, &(reqData->uvWork),
        OnRequestPermissionsData::WorkCallback, OnRequestPermissionsData::AfterWorkCallback, uv_qos_user_initiated);
    if (rev == 0) {
        (void)reqData.release();
    }
}
EXTERN_C_END

napi_value NAPI_GetFilesDir(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    return NAPI_GetFilesDirCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_GetOrCreateDistributedDir(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    return NAPI_GetOrCreateDistributedDirCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_GetCacheDir(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    return NAPI_GetCacheDirCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_GetCtxAppType(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    return NAPI_GetAppTypeCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_GetCtxHapModuleInfo(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    return NAPI_GetHapModuleInfoCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_GetAppVersionInfo(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    return NAPI_GetAppVersionInfoCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_GetApplicationContext(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    return NAPI_GetContextCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_GetCtxAbilityInfo(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    return NAPI_GetAbilityInfoCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_VerifyPermissionWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamVerifyPermission(env, argc, args, asyncCallbackInfo)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. Invoke UnwrapParamVerifyPermission fail", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_VerifyPermissionCallback";
        asyncParamEx.execute = VerifyPermissionExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_VerifyPermissionPromise";
        asyncParamEx.execute = VerifyPermissionExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_VerifyPermission(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    napi_value rev = NAPI_VerifyPermissionWrap(env, info, asyncCallbackInfo);
    if (rev == nullptr) {
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        rev = WrapVoidToJS(env);
    }
    return rev;
}

napi_value NAPI_GetBundleNameWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called, parameters is invalid", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called, the first parameter is invalid", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. asyncCallback", __func__);
        asyncParamEx.resource = "NAPI_GetBundleNameCallback";
        asyncParamEx.execute = GetBundleNameExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetBundleNamePromise";
        asyncParamEx.execute = GetBundleNameExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_GetBundleName(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    napi_value ret = NAPI_GetBundleNameWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end", __func__);
    return ret;
}

napi_value NAPI_GetApplicationInfo(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s,called", __func__);
    AppInfoCB *appInfoCB = CreateAppInfoCBInfo(env);
    if (appInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    napi_value ret = GetApplicationInfoWrap(env, info, appInfoCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s,ret == nullptr", __func__);
        if (appInfoCB != nullptr) {
            delete appInfoCB;
            appInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s,end", __func__);
    return ret;
}

napi_value NAPI_GetProcessInfo(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    ProcessInfoCB *processInfoCB = CreateProcessInfoCBInfo(env);
    if (processInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    processInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetProcessInfoWrap(env, info, processInfoCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, ret == nullptr.", __func__);
        if (processInfoCB != nullptr) {
            delete processInfoCB;
            processInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    return ret;
}

napi_value NAPI_GetElementName(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    ElementNameCB *elementNameCB = CreateElementNameCBInfo(env);
    if (elementNameCB == nullptr) {
        return WrapVoidToJS(env);
    }

    elementNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetElementNameWrap(env, info, elementNameCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr.", __func__);
        if (elementNameCB != nullptr) {
            delete elementNameCB;
            elementNameCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return ret;
}

napi_value NAPI_GetProcessName(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    ProcessNameCB *processNameCB = CreateProcessNameCBInfo(env);
    if (processNameCB == nullptr) {
        return WrapVoidToJS(env);
    }

    processNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetProcessNameWrap(env, info, processNameCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr.", __func__);
        if (processNameCB != nullptr) {
            delete processNameCB;
            processNameCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return ret;
}

napi_value NAPI_GetCallingBundle(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    CallingBundleCB *callingBundleCB = CreateCallingBundleCBInfo(env);
    if (callingBundleCB == nullptr) {
        return WrapVoidToJS(env);
    }

    callingBundleCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetCallingBundleWrap(env, info, callingBundleCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr", __func__);
        if (callingBundleCB != nullptr) {
            delete callingBundleCB;
            callingBundleCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return ret;
}

napi_value NAPI_GetOrCreateLocalDir(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    GetOrCreateLocalDirCB *getOrCreateLocalDirCB = CreateGetOrCreateLocalDirCBInfo(env);
    if (getOrCreateLocalDirCB == nullptr) {
        return WrapVoidToJS(env);
    }

    getOrCreateLocalDirCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetOrCreateLocalDirWrap(env, info, getOrCreateLocalDirCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr", __func__);
        if (getOrCreateLocalDirCB != nullptr) {
            delete getOrCreateLocalDirCB;
            getOrCreateLocalDirCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return ret;
}

napi_value NAPI_GetDatabaseDirSync(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
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
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr", __func__);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end", __func__);
    }
    return ret;
}

napi_value NAPI_GetPreferencesDirSync(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
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
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr", __func__);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    }
    return ret;
}

napi_value NAPI_IsUpdatingConfigurations(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    return NAPI_IsUpdatingConfigurationsCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_GetExternalCacheDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    return NAPI_GetExternalCacheDirCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_PrintDrawnCompleted(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    return NAPI_PrintDrawnCompletedCommon(env, info, AbilityType::PAGE);
}

napi_value NAPI_SetDisplayOrientation(napi_env env, napi_callback_info info)
{
#ifdef SUPPORT_GRAPHICS
    TAG_LOGD(AAFwkTag::JSNAPI, "%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGW(AAFwkTag::JSNAPI, "%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
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
    TAG_LOGD(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    return NAPI_GetDisplayOrientationCommon(env, info, AbilityType::PAGE);
#else
   return 0;
#endif
}

napi_value ContextPermissionInit(napi_env env, napi_value exports)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "Context::ContextPermissionInit called.");

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
    napi_value constructor;
    NAPI_CALL(env,
        napi_define_class(env,
            "context",
            NAPI_AUTO_LENGTH,
            ContextConstructor,
            nullptr,
            sizeof(properties) / sizeof(*properties),
            properties,
            &constructor));
    NAPI_CALL(env, SetGlobalClassContext(env, constructor));
    return exports;
}

napi_value NAPI_SetWakeUpScreen(napi_env env, napi_callback_info info)
{
#ifdef SUPPORT_GRAPHICS
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s,called", __func__);
    SetWakeUpScreenCB *setWakeUpScreenCB = new (std::nothrow) SetWakeUpScreenCB;
    if (setWakeUpScreenCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, SetWakeUpScreenCB new failed", __func__);
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
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, setWakeUpScreenCB run failed, delete resource", __func__);
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s,called end", __func__);
    return ret;
#else
   return nullptr;
#endif
}

static bool BindNapiJSContextFunction(napi_env env, napi_value object)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params error");
        return false;
    }
    const char* moduleName = "context";
    BindNativeFunction(
        env, object, "requestPermissionsFromUser", moduleName, NapiJsContext::JsRequestPermissionsFromUser);
    BindNativeFunction(env, object, "getBundleName", moduleName, NapiJsContext::JsGetBundleName);
    BindNativeFunction(env, object, "verifyPermission", moduleName, NapiJsContext::JsVerifyPermission);
    BindNativeFunction(env, object, "getApplicationInfo", moduleName, NapiJsContext::JsGetApplicationInfo);
    BindNativeFunction(env, object, "getProcessInfo", moduleName, NapiJsContext::JsGetProcessInfo);
    BindNativeFunction(env, object, "getElementName", moduleName, NapiJsContext::JsGetElementName);
    BindNativeFunction(env, object, "getProcessName", moduleName, NapiJsContext::JsGetProcessName);
    BindNativeFunction(env, object, "getCallingBundle", moduleName, NapiJsContext::JsGetCallingBundle);
    BindNativeFunction(env, object, "getOrCreateLocalDir", moduleName, NapiJsContext::JsGetOrCreateLocalDir);
    BindNativeFunction(env, object, "getFilesDir", moduleName, NapiJsContext::JsGetFilesDir);
    BindNativeFunction(env, object, "isUpdatingConfigurations", moduleName, NapiJsContext::JsIsUpdatingConfigurations);
    BindNativeFunction(env, object, "printDrawnCompleted", moduleName, NapiJsContext::JsPrintDrawnCompleted);
    BindNativeFunction(env, object, "getCacheDir", moduleName, NapiJsContext::JsGetCacheDir);
    BindNativeFunction(env, object, "getAppType", moduleName, NapiJsContext::JsGetCtxAppType);
    BindNativeFunction(env, object, "getHapModuleInfo", moduleName, NapiJsContext::JsGetCtxHapModuleInfo);
    BindNativeFunction(env, object, "getAppVersionInfo", moduleName, NapiJsContext::JsGetAppVersionInfo);
    BindNativeFunction(env, object, "getApplicationContext", moduleName, NapiJsContext::JsGetApplicationContext);
    BindNativeFunction(env, object, "getAbilityInfo", moduleName, NapiJsContext::JsGetCtxAbilityInfo);
    BindNativeFunction(env, object, "setShowOnLockScreen", moduleName, NapiJsContext::JsSetShowOnLockScreen);
    BindNativeFunction(env, object, "getOrCreateDistributedDir", moduleName,
        NapiJsContext::JsGetOrCreateDistributedDir);
    BindNativeFunction(env, object, "setWakeUpScreen", moduleName, NapiJsContext::JsSetWakeUpScreen);
    BindNativeFunction(env, object, "setDisplayOrientation", moduleName, NapiJsContext::JsSetDisplayOrientation);
    BindNativeFunction(env, object, "getDisplayOrientation", moduleName, NapiJsContext::JsGetDisplayOrientation);
    BindNativeFunction(env, object, "getExternalCacheDir", moduleName, NapiJsContext::JsGetExternalCacheDir);

    return true;
}

static napi_value ConstructNapiJSContext(napi_env env)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    napi_value objContext = nullptr;
    napi_create_object(env, &objContext);
    if (objContext == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CreateObject failed");
        return nullptr;
    }
    auto jsClass = std::make_unique<NapiJsContext>();
    if (!jsClass->DataInit(env)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NapiJsContext init failed");
        return nullptr;
    }
    napi_wrap(env, objContext, jsClass.release(), NapiJsContext::Finalizer, nullptr, nullptr);
    napi_set_named_property(env, objContext, "stageMode", CreateJsValue(env, false));
    if (!BindNapiJSContextFunction(env, objContext)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "bind func failed");
        return nullptr;
    }

    return objContext;
}

napi_value CreateNapiJSContext(napi_env env)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    auto jsObj = ConstructNapiJSContext(env);
    if (jsObj == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Construct Context failed");
        return CreateJsUndefined(env);
    }

    return jsObj;
}
}  // namespace AppExecFwk
}  // namespace OHOS
