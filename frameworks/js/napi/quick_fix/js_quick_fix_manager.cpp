/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "js_quick_fix_manager.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_application_quick_fix_info.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"
#include "quick_fix_error_utils.h"
#include "quick_fix_manager_client.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
const char *QUICK_FIX_MANAGER_NAME = "JsQuickFixMgr";
} // namespace

class JsQuickFixManager {
public:
    JsQuickFixManager() = default;
    ~JsQuickFixManager() = default;

    static void Finalizer(napi_env env, void *data, void *hint)
    {
        TAG_LOGD(AAFwkTag::QUICKFIX, "function called.");
        std::unique_ptr<JsQuickFixManager>(static_cast<JsQuickFixManager*>(data));
    }

    static napi_value ApplyQuickFix(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsQuickFixManager, OnApplyQuickFix);
    }

    static napi_value GetApplyedQuickFixInfo(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsQuickFixManager, OnGetApplyedQuickFixInfo);
    }

    static napi_value RevokeQuickFix(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsQuickFixManager, OnRevokeQuickFix);
    }

    static void Throw(napi_env env, int32_t errCode)
    {
        auto externalErrCode = AAFwk::QuickFixErrorUtil::GetErrorCode(errCode);
        auto errMsg = AAFwk::QuickFixErrorUtil::GetErrorMessage(errCode);
        napi_value error = CreateJsError(env, externalErrCode, errMsg);
        napi_throw(env, error);
    }

    static napi_value CreateJsErrorByErrorCode(napi_env env, int32_t errCode)
    {
        auto externalErrCode = AAFwk::QuickFixErrorUtil::GetErrorCode(errCode);
        auto errMsg = AAFwk::QuickFixErrorUtil::GetErrorMessage(errCode);
        return CreateJsError(env, externalErrCode, errMsg);
    }

private:
    napi_value OnGetApplyedQuickFixInfo(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::QUICKFIX, "function called.");
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "The number of parameter is invalid.");
            Throw(env, AAFwk::ERR_QUICKFIX_PARAM_INVALID);
            return CreateJsUndefined(env);
        }

        std::string bundleName;
        if (!OHOS::AppExecFwk::UnwrapStringFromJS2(env, info.argv[0], bundleName)) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "The bundleName is invalid.");
            Throw(env, AAFwk::ERR_QUICKFIX_PARAM_INVALID);
            return CreateJsUndefined(env);
        }

        auto complete = [bundleName](napi_env env, NapiAsyncTask &task, int32_t status) {
            AppExecFwk::ApplicationQuickFixInfo quickFixInfo;
            auto errCode = DelayedSingleton<AAFwk::QuickFixManagerClient>::GetInstance()->GetApplyedQuickFixInfo(
                bundleName, quickFixInfo);
            if (errCode == 0) {
                task.ResolveWithNoError(env, CreateJsApplicationQuickFixInfo(env, quickFixInfo));
            } else {
                task.Reject(env, CreateJsErrorByErrorCode(env, errCode));
            }
        };

        napi_value lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[1];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsQuickFixManager::OnGetApplyedQuickFixInfo", env,
            CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        TAG_LOGD(AAFwkTag::QUICKFIX, "function finished.");
        return result;
    }

    napi_value OnApplyQuickFix(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::QUICKFIX, "function called.");
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "The number of parameter is invalid.");
            Throw(env, AAFwk::ERR_QUICKFIX_PARAM_INVALID);
            return CreateJsUndefined(env);
        }

        std::vector<std::string> hapQuickFixFiles;
        if (!OHOS::AppExecFwk::UnwrapArrayStringFromJS(env, info.argv[0], hapQuickFixFiles)) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "Hap quick fix files is invalid.");
            Throw(env, AAFwk::ERR_QUICKFIX_PARAM_INVALID);
            return CreateJsUndefined(env);
        }

        auto complete = [hapQuickFixFiles](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto errcode = DelayedSingleton<AAFwk::QuickFixManagerClient>::GetInstance()->ApplyQuickFix(
                hapQuickFixFiles);
            if (errcode == 0) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByErrorCode(env, errcode));
            }
        };

        napi_value lastParam = (info.argc == 1) ? nullptr : info.argv[1];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsQuickFixManager::OnApplyQuickFix", env,
            CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        TAG_LOGD(AAFwkTag::QUICKFIX, "function finished.");
        return result;
    }

    napi_value OnRevokeQuickFix(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::QUICKFIX, "called.");
        if (info.argc == ARGC_ZERO) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "The number of parameter is invalid.");
            Throw(env, AAFwk::ERR_QUICKFIX_PARAM_INVALID);
            return CreateJsUndefined(env);
        }

        std::string bundleName;
        if (!ConvertFromJsValue(env, info.argv[ARGC_ZERO], bundleName)) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "The bundleName is invalid.");
            Throw(env, AAFwk::ERR_QUICKFIX_PARAM_INVALID);
            return CreateJsUndefined(env);
        }

        std::shared_ptr<int32_t> errCode = std::make_shared<int32_t>(AAFwk::ERR_OK);
        auto execute = [retval = errCode, bundleName] () {
            auto quickFixMgr = DelayedSingleton<AAFwk::QuickFixManagerClient>::GetInstance();
            if (quickFixMgr == nullptr) {
                *retval = AAFwk::ERR_QUICKFIX_INTERNAL_ERROR;
                TAG_LOGE(AAFwkTag::QUICKFIX, "Get quick fix mgr is nullptr.");
                return;
            }

            *retval = quickFixMgr->RevokeQuickFix(bundleName);
            TAG_LOGD(AAFwkTag::QUICKFIX, "Revoke quick fix execute retval is {%{public}d}.", *retval);
        };

        auto complete = [retval = errCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            TAG_LOGD(AAFwkTag::QUICKFIX, "Revoke quick fix complete called.");
            if (*retval != AAFwk::ERR_OK) {
                TAG_LOGE(AAFwkTag::QUICKFIX, "Revoke quick fix execution failed. retval is %{public}d", *retval);
                task.Reject(env, CreateJsErrorByErrorCode(env, *retval));
                return;
            }
            TAG_LOGD(AAFwkTag::QUICKFIX, "Revoke quick fix complete called ok.");
            task.ResolveWithNoError(env, CreateJsUndefined(env));
        };

        napi_value lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[ARGC_ONE];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsQuickFixManager::OnRevokeQuickFix", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        TAG_LOGD(AAFwkTag::QUICKFIX, "Function finished.");
        return result;
    }
};

napi_value CreateJsQuickFixManager(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::QUICKFIX, "function called.");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Input parameter is invalid.");
        return nullptr;
    }

    std::unique_ptr<JsQuickFixManager> quickFixManager = std::make_unique<JsQuickFixManager>();
    napi_wrap(env, exportObj, quickFixManager.release(), JsQuickFixManager::Finalizer, nullptr, nullptr);

    BindNativeFunction(env, exportObj, "applyQuickFix", QUICK_FIX_MANAGER_NAME, JsQuickFixManager::ApplyQuickFix);
    BindNativeFunction(env, exportObj, "getApplicationQuickFixInfo", QUICK_FIX_MANAGER_NAME,
        JsQuickFixManager::GetApplyedQuickFixInfo);
    BindNativeFunction(env, exportObj, "revokeQuickFix", QUICK_FIX_MANAGER_NAME, JsQuickFixManager::RevokeQuickFix);
    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // namespace OHOS
