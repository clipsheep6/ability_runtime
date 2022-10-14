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
#include "js_data_ability_helper.h"

#include <cstring>
#include <uv.h>
#include <vector>
#include <string>

#include "data_ability_helper.h"
#include "data_ability_observer_interface.h"
#include "uri.h"

#include "../inner/napi_common/napi_common_ability.h"
#include "data_ability_operation.h"
#include "data_ability_result.h"
#include "hilog_wrapper.h"
#include "message_parcel.h"
#include "napi_base_context.h"
#include "napi_data_ability_operation.h"
#include "napi_data_ability_predicates.h"
#include "napi_rdb_predicates.h"
#include "napi_result_set.h"
#include "securec.h"

#ifndef SUPPORT_GRAPHICS
#define DBL_MIN ((double)2.22507385850720138309e-308L)
#define DBL_MAX ((double)2.22507385850720138309e-308L)
#endif

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AbilityRuntime {
std::list<std::shared_ptr<DataAbilityHelper>> g_dataAbilityHelperList;
std::vector<DAHelperOnOffCB *> g_registerInstances;

static void OnChangeJSThreadWorker(uv_work_t *work, int status)
{
    HILOG_INFO("OnChange, uv_queue_work.");
    if (work == nullptr) {
        HILOG_ERROR("OnChange, uv_queue_work input work is nullptr.");
        return;
    }
    DAHelperOnOffCB *onCB = (DAHelperOnOffCB *)work->data;
    if (onCB == nullptr) {
        HILOG_ERROR("OnChange, uv_queue_work onCB is nullptr.");
        delete work;
        work = nullptr;
        return;
    }

    if (onCB->observer != nullptr) {
        onCB->observer->CallJsMethod();
    }

    delete onCB;
    onCB = nullptr;
    delete work;
    work = nullptr;
    HILOG_INFO("OnChange, uv_queue_work end.");
}

void NAPIDataAbilityObserver::ReleaseJSCallback()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (ref_ == nullptr) {
        HILOG_ERROR("ref_ is nullptr.");
        return;
    }

    if (isCallingback_) {
        needRelease_ = true;
        HILOG_WARN("ref_ is calling back.");
        return;
    }

    SafeReleaseJSCallback();
    HILOG_INFO("End.");
}

void NAPIDataAbilityObserver::SafeReleaseJSCallback()
{
    uv_loop_s* loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("loop is nullptr.");
        return;
    }

    struct DelRefCallbackInfo {
        napi_env env_;
        napi_ref ref_;
    };

    DelRefCallbackInfo* delRefCallbackInfo = new DelRefCallbackInfo {
        .env_ = env_,
        .ref_ = ref_,
    };

    uv_work_t* work = new uv_work_t;
    work->data = (void*)delRefCallbackInfo;
    int ret = uv_queue_work(
        loop, work, [](uv_work_t* work) {},
        [](uv_work_t* work, int status) {
            // JS Thread
            if (work == nullptr) {
                HILOG_ERROR("work is nullptr.");
                return;
            }
            auto delRefCallbackInfo =  reinterpret_cast<DelRefCallbackInfo*>(work->data);
            if (delRefCallbackInfo == nullptr) {
                HILOG_ERROR("delRefCallbackInfo is nullptr.");
                delete work;
                work = nullptr;
                return;
            }

            napi_delete_reference(delRefCallbackInfo->env_, delRefCallbackInfo->ref_);
            delete delRefCallbackInfo;
            delRefCallbackInfo = nullptr;
            delete work;
            work = nullptr;
        });
    if (ret != 0) {
        if (delRefCallbackInfo != nullptr) {
            delete delRefCallbackInfo;
            delRefCallbackInfo = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
    ref_ = nullptr;
}

void NAPIDataAbilityObserver::SetEnv(const napi_env &env)
{
    env_ = env;
    HILOG_INFO("End.");
}

void NAPIDataAbilityObserver::SetCallbackRef(const napi_ref &ref)
{
    ref_ = ref;
    HILOG_INFO("End.");
}

void NAPIDataAbilityObserver::CallJsMethod()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (ref_ == nullptr || env_ == nullptr) {
            HILOG_WARN("ref_ or env_ is nullptr.");
            return;
        }
        isCallingback_ = true;
    }
    napi_value result[ARGS_TWO] = {0};
    result[PARAM0] = GetCallbackErrorValue(env_, NO_ERROR);
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_get_undefined(env_, &undefined);
    napi_value callResult = 0;
    napi_get_reference_value(env_, ref_, &callback);
    napi_call_function(env_, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (needRelease_ && ref_ != nullptr) {
            HILOG_INFO("Delete callback.");
            napi_delete_reference(env_, ref_);
            ref_ = nullptr;
            needRelease_ = false;
        }
        isCallingback_ = false;
    }
}

void NAPIDataAbilityObserver::OnChange()
{
    if (ref_ == nullptr) {
        HILOG_ERROR("ref_ is nullptr.");
        return;
    }
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("loop is nullptr.");
        return;
    }

    uv_work_t *work = new uv_work_t;
    DAHelperOnOffCB *onCB = new DAHelperOnOffCB;
    onCB->observer = this;
    work->data = (void *)onCB;
    int rev = uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        OnChangeJSThreadWorker);
    if (rev != 0) {
        if (onCB != nullptr) {
            delete onCB;
            onCB = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
    HILOG_INFO("End.");
}

/**
 * @brief DataAbilityHelper NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value DataAbilityHelperInit(napi_env env, napi_value exports)
{
    HILOG_INFO("Enter.");
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("insert", NAPI_Insert),
        DECLARE_NAPI_FUNCTION("notifyChange", NAPI_NotifyChange),
        DECLARE_NAPI_FUNCTION("on", NAPI_Register),
        DECLARE_NAPI_FUNCTION("off", NAPI_UnRegister),
        DECLARE_NAPI_FUNCTION("delete", NAPI_Delete),
        DECLARE_NAPI_FUNCTION("query", NAPI_Query),
        DECLARE_NAPI_FUNCTION("update", NAPI_Update),
        DECLARE_NAPI_FUNCTION("batchInsert", NAPI_BatchInsert),
        DECLARE_NAPI_FUNCTION("openFile", NAPI_OpenFile),
        DECLARE_NAPI_FUNCTION("getType", NAPI_GetType),
        DECLARE_NAPI_FUNCTION("getFileTypes", NAPI_GetFileTypes),
        DECLARE_NAPI_FUNCTION("normalizeUri", NAPI_NormalizeUri),
        DECLARE_NAPI_FUNCTION("denormalizeUri", NAPI_DenormalizeUri),
        DECLARE_NAPI_FUNCTION("executeBatch", NAPI_ExecuteBatch),
        DECLARE_NAPI_FUNCTION("call", NAPI_Call),
    };

    napi_value constructor;
    NAPI_CALL(env,
        napi_define_class(env,
            "dataAbilityHelper",
            NAPI_AUTO_LENGTH,
            DataAbilityHelperConstructor,
            nullptr,
            sizeof(properties) / sizeof(*properties),
            properties,
            &constructor));
    NAPI_CALL(env, SaveGlobalDataAbilityHelper(env, constructor));
    g_dataAbilityHelperList.clear();
    return exports;
}

napi_value DataAbilityHelperConstructor(napi_env env, napi_callback_info info)
{
    HILOG_INFO("Enter.");
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    auto& dataAbilityHelperStatus = GetDataAbilityHelperStatus();
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");

    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = nullptr;
    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, argv[0], stageMode);
    if (status != napi_ok) {
        HILOG_INFO("argv[0] is not a context.");
        auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
        if (ability == nullptr) {
            HILOG_ERROR("Failed to get native context instance.");
            return nullptr;
        }
        std::string strUri = NapiValueToStringUtf8(env, argv[0]);
        HILOG_INFO("FA Model: strUri = %{public}s.", strUri.c_str());
        dataAbilityHelper = DataAbilityHelper::Creator(ability->GetContext(), std::make_shared<Uri>(strUri));
    } else {
        HILOG_INFO("argv[0] is a context.");
        if (stageMode) {
            auto context = OHOS::AbilityRuntime::GetStageModeContext(env, argv[0]);
            if (context == nullptr) {
                HILOG_ERROR("Failed to get native context instance.");
                return nullptr;
            }
            std::string strUri = NapiValueToStringUtf8(env, argv[PARAM1]);
            HILOG_INFO("Stage Model: strUri = %{public}s.", strUri.c_str());
            dataAbilityHelper = DataAbilityHelper::Creator(context, std::make_shared<Uri>(strUri));
        } else {
            auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
            if (ability == nullptr) {
                HILOG_ERROR("Failed to get native context instance.");
                return nullptr;
            }
            std::string strUri = NapiValueToStringUtf8(env, argv[PARAM1]);
            HILOG_INFO("FA Model: strUri = %{public}s.", strUri.c_str());
            dataAbilityHelper = DataAbilityHelper::Creator(ability->GetContext(), std::make_shared<Uri>(strUri));
        }
    }

    if (dataAbilityHelper == nullptr) {
        HILOG_INFO("dataAbilityHelper is nullptr.");
        dataAbilityHelperStatus = false;
        return nullptr;
    }
    dataAbilityHelper->SetCallFromJs();
    g_dataAbilityHelperList.emplace_back(dataAbilityHelper);
    HILOG_INFO("dataAbilityHelperList size = %{public}zu.", g_dataAbilityHelperList.size());

    napi_wrap(
        env,
        thisVar,
        dataAbilityHelper.get(),
        [](napi_env env, void *data, void *hint) {
            DataAbilityHelper *objectInfo = static_cast<DataAbilityHelper *>(data);
            HILOG_INFO("g_registerInstances size = %{public}zu.", g_registerInstances.size());
            for (auto iter = g_registerInstances.begin(); iter != g_registerInstances.end();) {
                if (!NeedErase(iter, objectInfo)) {
                    iter = g_registerInstances.erase(iter);
                }
            }
            HILOG_INFO("g_registerInstances size = %{public}zu.", g_registerInstances.size());
            g_dataAbilityHelperList.remove_if(
                [objectInfo](const std::shared_ptr<DataAbilityHelper> &dataAbilityHelper) {
                    return objectInfo == dataAbilityHelper.get();
                });
            HILOG_INFO("g_dataAbilityHelperList size = %{public}zu.", g_dataAbilityHelperList.size());
        },
        nullptr,
        nullptr);

    dataAbilityHelperStatus = true;
    HILOG_INFO("End.");
    return thisVar;
}

bool CheckArgCount(const napi_env &env, uint32_t argCount, uint32_t promiseArgCount)
{
    if (argCount < promiseArgCount || argCount > ARGS_MAX_COUNT) {
        HILOG_ERROR("Wrong argument count.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }
    return true;
}

bool CheckStringParam(const napi_env &env, const napi_value &arg, std::string &stringParam)
{
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, arg, &valuetype);
    if (valuetype != napi_string) {
        HILOG_ERROR("Wrong argument type.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }
    stringParam = NapiValueToStringUtf8(env, arg);
    HILOG_INFO("StringParam = %{public}s.", stringParam.c_str());
    return true;
}

bool CheckValuesBucket(const napi_env &env, const napi_value &arg, NativeRdb::ValuesBucket &valuesBucket)
{
    if (!AnalysisValuesBucket(valuesBucket, env, arg)) {
        HILOG_ERROR("Unwrap vaules bucket from JS failed.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }
    return true;
}

bool CheckNotifyType(const napi_env &env, const napi_value &arg)
{
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, arg, &valuetype);
    if (valuetype != napi_string) {
        HILOG_ERROR("Wrong argument type.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }

    std::string type = NapiValueToStringUtf8(env, arg);
    if (type != "dataChange") {
        HILOG_ERROR("Wrong argument type, type: %{public}s.", type.c_str());
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }
    return true;
}

bool CheckUnregisterParam(const napi_env &env, napi_value *args, uint32_t argCount, DAHelperOnOffCB *offCB)
{
    // check param1
    offCB->uri = "";
    napi_valuetype valuetype = napi_undefined;
    if (argCount > ARGS_TWO) {
        // parse uri and callback
        napi_typeof(env, args[PARAM1], &valuetype);
        if (valuetype == napi_string) {
            offCB->uri = NapiValueToStringUtf8(env, args[PARAM1]);
            HILOG_INFO("Unregister uri = %{public}s.", offCB->uri.c_str());
        } else {
            HILOG_ERROR("Wrong argument type.");
            ThrowException(env, JS_ERR_PARAM_INVALID);
            return false;
        }
        napi_typeof(env, args[PARAM2], &valuetype);
        if (valuetype == napi_function) {
            napi_create_reference(env, args[PARAM2], 1, &offCB->cbBase.cbInfo.callback);
        } else {
            HILOG_ERROR("Wrong argument type.");
            ThrowException(env, JS_ERR_PARAM_INVALID);
            return false;
        }
        return true;
    } 

    // parse uri or callback
    napi_typeof(env, args[PARAM1], &valuetype);
    if (valuetype == napi_string) {
        offCB->uri = NapiValueToStringUtf8(env, args[PARAM1]);
        HILOG_INFO("Unregister uri = %{public}s.", offCB->uri.c_str());
    } else if (valuetype == napi_function) {
        napi_create_reference(env, args[PARAM1], 1, &offCB->cbBase.cbInfo.callback);
    } else {
        HILOG_ERROR("Wrong argument type.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }
    return true;
}

bool CheckArrayStringParam(napi_env env, napi_value param, std::vector<std::string> &result)
{
    if (!NapiValueToArrayStringUtf8(env, param, result)) {
        HILOG_ERROR("Unwrap array string from JS failed.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }
    return true;
}

bool CheckArrayValuesBucket(const napi_env &env, const napi_value &param, std::vector<NativeRdb::ValuesBucket> &result)
{
    if (!UnwrapArrayObjectFromJS(env, param, result)) {
        HILOG_ERROR("Unwrap array values bucket JS failed.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }
    return true;
}

bool CheckArrayOperation(napi_env env, napi_callback_info info, napi_value param,
    std::vector<std::shared_ptr<DataAbilityOperation>> &result)
{
    if (UnwrapArrayOperationFromJS(env, info, param, result)) {
        HILOG_ERROR("Unwrap array operation from JS failed.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }
    return true;
}

int32_t TransferErrorToExternal(int32_t errCode)
{
    int32_t ExternalCode = JS_ERR_INTERNAL_ERROR;
    switch (errCode) {
        default:
            ExternalCode = JS_ERR_INTERNAL_ERROR;
    }

    HILOG_DEBUG("Internal errorCode[%{public}d] to external errorCode[%{public}d].", errCode, ExternalCode);
    return ExternalCode;
}

napi_value CreateErrorValue(napi_env env, int32_t errCode)
{
    napi_value code = nullptr;
    napi_create_int32(env, errCode, &code);

    auto iter = ERR_CODE_MSG.find(errCode);
    std::string errMsg = iter != ERR_CODE_MSG.end() ? iter->second : "";
    napi_value message = nullptr;
    napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &message);

    napi_value error = nullptr;
    napi_create_error(env, nullptr, message, &error);
    napi_set_named_property(env, error, "code", code);
    return error;
}

void ThrowException(napi_env env, int32_t errCode)
{
    HILOG_DEBUG("Enter.");
    napi_throw(env, CreateErrorValue(env, errCode));
}

void CreateCallBackValue(const napi_env &env, const napi_ref &callbackIn, int32_t errorCode, const napi_value &result)
{
    HILOG_DEBUG("Enter.");
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value jsResults[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;

    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, callbackIn, &callback));

    if (errorCode != 0) {
        // native function executes failed, return (error)
        jsResults[PARAM0] = CreateErrorValue(env, TransferErrorToExternal(errorCode));
        NAPI_CALL_RETURN_VOID(env,
            napi_call_function(env, undefined, callback, ARGS_ONE, &jsResults[PARAM0], &callResult));
    } else {
        // native function executes successfully, return (null, data)
        jsResults[PARAM0] = WrapVoidToJS(env);
        jsResults[PARAM1] = result;
        NAPI_CALL_RETURN_VOID(env,
            napi_call_function(env, undefined, callback, ARGS_TWO, &jsResults[PARAM0], &callResult));
    }
    HILOG_DEBUG("End.");
}

void CreateCallBackValue(const napi_env &env, const napi_ref &callbackIn, int32_t errorCode)
{
    HILOG_DEBUG("Enter.");
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value jsResults[ARGS_ONE] = {nullptr};
    napi_value callResult = nullptr;

    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, callbackIn, &callback));

    if (errorCode != NO_ERROR) {
        // native function executes failed, return (error)
        jsResults[PARAM0] = CreateErrorValue(env, TransferErrorToExternal(errorCode));
    } else {
        // native function executes successfully, return (null)
        jsResults[PARAM0] = WrapVoidToJS(env);
    }

    NAPI_CALL_RETURN_VOID(env,
        napi_call_function(env, undefined, callback, ARGS_ONE, &jsResults[PARAM0], &callResult));
    HILOG_DEBUG("End.");
}

void CreatePromiseValue(const napi_env &env,
    const napi_deferred &deferred, int32_t errorCode, const napi_value &result)
{
    HILOG_DEBUG("Enter");
    if (errorCode != NO_ERROR) {
        NAPI_CALL_RETURN_VOID(env,
            napi_reject_deferred(env, deferred, CreateErrorValue(env, TransferErrorToExternal(errorCode))));
    } else {
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, deferred, result));
    }
    HILOG_DEBUG("End.");
}
}  // namespace AppExecFwk
}  // namespace OHOS

