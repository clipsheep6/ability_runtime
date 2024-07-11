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

#include "napi_js_context.h"

#include "ability_process.h"
#include "ability_util.h"
#include "directory_ex.h"
#include "file_ex.h"
#include "hilog_tag_wrapper.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
void NapiJsContext::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called.");
    std::unique_ptr<NapiJsContext>(static_cast<NapiJsContext*>(data));
}

napi_value NapiJsContext::JsRequestPermissionsFromUser(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->OnRequestPermissionsFromUser(env, info);
}

napi_value NapiJsContext::JsGetBundleName(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->OnGetBundleName(env, info);
}

napi_value NapiJsContext::JsVerifyPermission(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->OnVerifyPermission(env, info);
}

napi_value NapiJsContext::JsGetApplicationInfo(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters engine is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->OnGetApplicationInfo(env, info);
}

napi_value NapiJsContext::JsGetProcessInfo(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->OnGetProcessInfo(env, info);
}

napi_value NapiJsContext::JsGetElementName(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->OnGetElementName(env, info);
}

napi_value NapiJsContext::JsGetProcessName(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->OnGetProcessName(env, info);
}

napi_value NapiJsContext::JsGetCallingBundle(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->OnGetCallingBundle(env, info);
}

napi_value NapiJsContext::JsGetOrCreateLocalDir(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->OnGetOrCreateLocalDir(env, info);
}

napi_value NapiJsContext::JsGetFilesDir(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetFilesDir(env, info, AbilityType::PAGE);
}

napi_value NapiJsContext::JsIsUpdatingConfigurations(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsIsUpdatingConfigurations(env, info, AbilityType::PAGE);
}

napi_value NapiJsContext::JsPrintDrawnCompleted(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsPrintDrawnCompleted(env, info, AbilityType::PAGE);
}

napi_value NapiJsContext::JsGetCacheDir(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetCacheDir(env, info, AbilityType::PAGE);
}

napi_value NapiJsContext::JsGetCtxAppType(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetCtxAppType(env, info, AbilityType::PAGE);
}

napi_value NapiJsContext::JsGetCtxHapModuleInfo(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetCtxHapModuleInfo(env, info, AbilityType::PAGE);
}

napi_value NapiJsContext::JsGetAppVersionInfo(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetAppVersionInfo(env, info, AbilityType::PAGE);
}

napi_value NapiJsContext::JsGetApplicationContext(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetContext(env, info, AbilityType::PAGE);
}

napi_value NapiJsContext::JsGetCtxAbilityInfo(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetCtxAbilityInfo(env, info, AbilityType::PAGE);
}

napi_value NapiJsContext::JsSetShowOnLockScreen(napi_env env, napi_callback_info info)
{
#ifdef SUPPORT_GRAPHICS
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->OnSetShowOnLockScreen(env, info);
#else
   return nullptr;
#endif
}

napi_value NapiJsContext::JsGetOrCreateDistributedDir(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetOrCreateDistributedDir(env, info, AbilityType::PAGE);
}

napi_value NapiJsContext::JsSetWakeUpScreen(napi_env env, napi_callback_info info)
{
#ifdef SUPPORT_GRAPHICS
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->OnSetWakeUpScreen(env, info);
#else
   return nullptr;
#endif
}

napi_value NapiJsContext::JsSetDisplayOrientation(napi_env env, napi_callback_info info)
{
#ifdef SUPPORT_GRAPHICS
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->OnSetDisplayOrientation(env, info);
#else
   return nullptr;
#endif
}

napi_value NapiJsContext::JsGetDisplayOrientation(napi_env env, napi_callback_info info)
{
#ifdef SUPPORT_GRAPHICS
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetDisplayOrientation(env, info, AbilityType::PAGE);
#else
   return nullptr;
#endif
}

napi_value NapiJsContext::JsGetExternalCacheDir(napi_env env, napi_callback_info info)
{
    CHECK_POINTER_AND_RETURN_LOG(env, nullptr, "but input parameters env is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(info, nullptr, "but input parameters info is nullptr");

    auto object = CheckParamsAndGetThis<NapiJsContext>(env, info);
    CHECK_POINTER_AND_RETURN_LOG(object, CreateJsUndefined(env), "CheckParamsAndGetThis return nullptr");

    return object->JsNapiCommon::JsGetExternalCacheDir(env, info, AbilityType::PAGE);
}

bool NapiJsContext::DataInit(napi_env env)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    napi_value global = nullptr;
    napi_value abilityObj = nullptr;
    TAG_LOGI(AAFwkTag::JSNAPI, "Get Ability to start");
    NAPI_CALL_BASE(env, napi_get_global(env, &global), false);
    NAPI_CALL_BASE(env, napi_get_named_property(env, global, "ability", &abilityObj), false);
    napi_status status = napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability_));
    if (status != napi_ok) {
        TAG_LOGW(AAFwkTag::JSNAPI, "Failed to get external ability info.");
    }
    TAG_LOGD(AAFwkTag::JSNAPI, "end");

    return true;
}

napi_value NapiJsContext::OnRequestPermissionsFromUser(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "OnRequestPermissionsFromUser called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc == ARGS_ZERO || argc > ARGS_THREE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }
    CallAbilityPermissionParam permissionParam;
    if (!GetStringsValue(env, argv[PARAM0], permissionParam.permission_list)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params string error");
        return CreateJsUndefined(env);
    }

    if (!ConvertFromJsValue(env, argv[PARAM1], permissionParam.requestCode)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params int error");
        return CreateJsUndefined(env);
    }

    auto callback = argc == ARGS_THREE ? argv[PARAM2] : nullptr;
    napi_value result = nullptr;
    auto napiAsyncTask =
        AbilityRuntime::CreateAsyncTaskWithLastParam(env, callback, nullptr, nullptr, &result).release();

    int32_t errorCode = NAPI_ERR_NO_ERROR;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "OnRequestPermissionsFromUser ability is nullptr.");
        errorCode = NAPI_ERR_ACE_ABILITY;
    }

    if (permissionParam.permission_list.size() == 0) {
        TAG_LOGE(AAFwkTag::JSNAPI, "OnRequestPermissionsFromUser permission_list size is 0");
        errorCode = NAPI_ERR_PARAM_INVALID;
    }

    if (errorCode != NAPI_ERR_NO_ERROR) {
        napi_value errorValue = CreateJsError(env, errorCode, ConvertErrorCode(errorCode));
        napiAsyncTask->Reject(env, errorValue);
        delete napiAsyncTask;
        napiAsyncTask = nullptr;
    } else {
        CallbackInfo callbackInfo;
        callbackInfo.env = env;
        callbackInfo.napiAsyncTask = napiAsyncTask;
        AbilityProcess::GetInstance()->RequestPermissionsFromUser(ability_, permissionParam, callbackInfo);
    }

    return result;
}

napi_value NapiJsContext::OnGetBundleName(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsNull(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsBundleName> bundleName = std::make_shared<JsBundleName>();
    auto execute = [obj = this, name = bundleName, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute wrong, the ability is nullptr");
            return;
        }
        if (name == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute wrong, name is nullptr");
            return;
        }
        name->name = obj->ability_->GetBundleName();
    };
    auto complete = [obj = this, name = bundleName, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || name == nullptr) {
            auto ecode = name == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            TAG_LOGD(AAFwkTag::JSNAPI, "task execute error, name is nullptr or NAPI_ERR_ABILITY_CALL_INVALID");
            return;
        }
        task.Resolve(env, CreateJsValue(env, name->name));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("NapiJsContext::OnGetBundleName",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value NapiJsContext::OnVerifyPermission(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc == ARGS_ZERO || argc > ARGS_THREE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsNull(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::string permission("");
    if (!ConvertFromJsValue(env, argv[PARAM0], permission)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params string error");
        return CreateJsNull(env);
    }
    JsPermissionOptions options;
    bool flagCall = UnwrapVerifyPermissionParams(env, info, options);
    auto execute = [obj = this, permission, options, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability is nullptr");
            return;
        }
        if (options.uidFlag) {
            *value = obj->ability_->VerifyPermission(permission, options.pid, options.uid);
        } else {
            *value = obj->ability_->VerifySelfPermission(permission);
        }
    };
    auto complete = [obj = this, value = errorVal] (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value == static_cast<int32_t>(NAPI_ERR_ACE_ABILITY)) {
            task.Reject(env, CreateJsError(env, *value, obj->ConvertErrorCode(*value)));
            return;
        }
        task.Resolve(env, CreateJsValue(env, *value));
    };

    auto callback = flagCall ? ((argc == ARGS_TWO) ? argv[PARAM1] : argv[PARAM2]) : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("NapiJsContext::OnGetBundleName",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value NapiJsContext::OnGetApplicationInfo(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "OnGetApplicationInfo called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsApplicationInfo> infoData = std::make_shared<JsApplicationInfo>();
    auto execute = [obj = this, info = infoData, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability is nullptr");
            return;
        }
        auto getInfo = obj->ability_->GetApplicationInfo();
        if (getInfo != nullptr && info != nullptr) {
            info->appInfo = *getInfo;
        } else {
            TAG_LOGE(AAFwkTag::JSNAPI, "GetApplicationInfo return nullptr");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
        }
    };
    auto complete = [obj = this, info = infoData, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || info == nullptr) {
            TAG_LOGD(AAFwkTag::JSNAPI, "errorVal is 0 or JsHapModuleInfo is null");
            auto ecode = info == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, obj->CreateAppInfo(env, info->appInfo));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("NapiJsContext::OnGetApplicationInfo",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value NapiJsContext::OnGetProcessInfo(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "OnGetProcessInfo called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsProcessInfo> processInfo = std::make_shared<JsProcessInfo>();
    auto execute = [obj = this, data = processInfo, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability is nullptr");
            return;
        }
        auto getInfo = obj->ability_->GetProcessInfo();
        if (getInfo != nullptr && data != nullptr) {
            data->processName = getInfo->GetProcessName();
            data->pid = getInfo->GetPid();
        } else {
            TAG_LOGE(AAFwkTag::JSNAPI, "GetProcessInfo return nullptr");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
        }
    };
    auto complete = [obj = this, info = processInfo, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || info == nullptr) {
            auto ecode = info == nullptr ? (NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, obj->CreateProcessInfo(env, info));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("NapiJsContext::OnGetProcessInfo",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value NapiJsContext::OnGetElementName(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "OnGetElementName called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsElementName> elementName = std::make_shared<JsElementName>();
    auto execute = [obj = this, data = elementName, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability is nullptr");
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
            TAG_LOGE(AAFwkTag::JSNAPI, "GetElementName return nullptr");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
        }
    };
    auto complete = [obj = this, ename = elementName, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || ename == nullptr) {
            auto ecode = ename == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, obj->CreateElementName(env, ename));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("NapiJsContext::OnGetElementName",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value NapiJsContext::OnGetProcessName(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "OnGetProcessName called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsProcessName> processName = std::make_shared<JsProcessName>();
    auto execute = [obj = this, name = processName, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability is null");
            return;
        }
        if (name == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, name is null");
            return;
        }
        name->name = obj->ability_->GetProcessName();
    };
    auto complete = [obj = this, name = processName, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || name == nullptr) {
            auto ecode = name == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            TAG_LOGD(AAFwkTag::JSNAPI, "task execute error, name is nullptr or NAPI_ERR_ABILITY_CALL_INVALID.");
            return;
        }
        task.Resolve(env, CreateJsValue(env, name->name));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("NapiJsContext::OnGetProcessName",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value NapiJsContext::OnGetCallingBundle(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "OnGetCallingBundle called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsCallingBundleName> callingBundleName = std::make_shared<JsCallingBundleName>();
    auto execute = [obj = this, name = callingBundleName, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability is nullptr");
            return;
        }
        if (name == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, name is nullptr");
            return;
        }
        name->name = obj->ability_->GetCallingBundle();
    };
    auto complete = [obj = this, name = callingBundleName, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || name == nullptr) {
            auto ecode = name == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, CreateJsValue(env, name->name));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("NapiJsContext::OnGetCallingBundle",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value NapiJsContext::OnGetOrCreateLocalDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "OnGetOrCreateLocalDir called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsOrCreateLocalDir> createDir = std::make_shared<JsOrCreateLocalDir>();
    auto execute = [obj = this, dir = createDir, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability is nullptr");
            return;
        }
        auto context = obj->ability_->GetAbilityContext();
        if (context == nullptr || dir == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability context is nullptr");
            return;
        }
        dir->name = context->GetBaseDir();
        if (!OHOS::FileExists(dir->name)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "dir is not exits, create dir.");
            OHOS::ForceCreateDirectory(dir->name);
            OHOS::ChangeModeDirectory(dir->name, MODE);
        }
    };
    auto complete = [obj = this, dir = createDir, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || dir == nullptr) {
            TAG_LOGD(AAFwkTag::JSNAPI, "errorVal is error or JsCacheDir is nullptr");
            auto ecode = dir == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, CreateJsValue(env, dir->name));
    };
    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("NapiJsContext::OnGetOrCreateLocalDir",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value NapiJsContext::OnSetShowOnLockScreen(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc == ARGS_ZERO || argc > ARGS_TWO) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    bool isShow = false;
    if (!ConvertFromJsValue(env, argv[PARAM0], isShow)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params int error");
        return CreateJsUndefined(env);
    }
    auto complete = [obj = this, isShow, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (obj->ability_ == nullptr) {
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(NAPI_ERR_ACE_ABILITY), "get ability error"));
            return;
        }
#ifdef SUPPORT_SCREEN
        obj->ability_->SetShowOnLockScreen(isShow);
#endif
        task.Resolve(env, CreateJsUndefined(env));
    };

    auto callback = argc == ARGS_ONE ? nullptr : argv[PARAM1];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("NapiJsContext::OnSetShowOnLockScreen",
        env, CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));

    return result;
}

napi_value NapiJsContext::OnSetWakeUpScreen(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc == ARGS_ZERO || argc > ARGS_TWO) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    bool wakeUp = false;
    if (!ConvertFromJsValue(env, argv[PARAM0], wakeUp)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params int error");
        return CreateJsUndefined(env);
    }
    auto complete = [obj = this, wakeUp]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (obj->ability_ == nullptr) {
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(NAPI_ERR_ACE_ABILITY), "get ability error"));
            return;
        }
#ifdef SUPPORT_SCREEN
        obj->ability_->SetWakeUpScreen(wakeUp);
#endif
        task.Resolve(env, CreateJsUndefined(env));
    };

    auto callback = argc == ARGS_ONE ? nullptr : argv[PARAM1];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("NapiJsContext::OnSetWakeUpScreen",
        env, CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));

    return result;
}

napi_value NapiJsContext::OnSetDisplayOrientation(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc == ARGS_ZERO || argc > ARGS_TWO) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    int32_t orientation = 0;
    if (!ConvertFromJsValue(env, argv[PARAM0], orientation)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params int error");
        return CreateJsUndefined(env);
    }

    int32_t maxRange = 3;
    if (orientation < 0 || orientation > maxRange) {
        TAG_LOGE(AAFwkTag::JSNAPI, "wrong parameter orientation : %{public}d", orientation);
        return CreateJsNull(env);
    }
    auto complete = [obj = this, orientationData = orientation]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (obj->ability_ == nullptr) {
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(NAPI_ERR_ACE_ABILITY), "get ability error"));
            return;
        }
#ifdef SUPPORT_SCREEN
        obj->ability_->SetDisplayOrientation(orientationData);
#endif
        task.Resolve(env, CreateJsUndefined(env));
    };

    auto callback = argc == ARGS_ONE ? nullptr : argv[PARAM1];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("NapiJsContext::SetDisplayOrientation",
        env, CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));

    return result;
}
}
}