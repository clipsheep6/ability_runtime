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
#include <string>
#include <set>

#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "ability_runtime/js_caller_complex.h"

namespace OHOS {
namespace AbilityRuntime {
namespace { // nameless
static std::map<NativeValueType, std::string> logcast = {
    { NATIVE_UNDEFINED, std::string("NATIVE_UNDEFINED") },
    { NATIVE_NULL, std::string("NATIVE_NULL") },
    { NATIVE_BOOLEAN, std::string("NATIVE_BOOLEAN") },
    { NATIVE_NUMBER, std::string("NATIVE_NUMBER") },
    { NATIVE_STRING, std::string("NATIVE_STRING") },
    { NATIVE_SYMBOL, std::string("NATIVE_SYMBOL") },
    { NATIVE_OBJECT, std::string("NATIVE_OBJECT") },
    { NATIVE_FUNCTION, std::string("NATIVE_FUNCTION") },
    { NATIVE_EXTERNAL, std::string("NATIVE_EXTERNAL") },
    { NATIVE_BIGINT, std::string("NATIVE_BIGINT") },
};

class JsCallerComplex {
public:
    enum class OBJSTATE {
        OBJ_NORMAL,
        OBJ_EXECUTION,
        OBJ_RELEASE
    };

    explicit JsCallerComplex(
        napi_env env, ReleaseCallFunc releaseCallFunc, sptr<IRemoteObject> callee,
        std::shared_ptr<CallerCallBack> callerCallBack) : releaseCallFunc_(releaseCallFunc),
        callee_(callee), releaseCallBackEngine_(env), remoteStateChanegdEngine_(env),
        callerCallBackObj_(callerCallBack), jsReleaseCallBackObj_(nullptr), jsRemoteStateChangedObj_(nullptr)
    {
        AddJsCallerComplex(this);
        handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
        currentState_ = OBJSTATE::OBJ_NORMAL;
    };
    virtual~JsCallerComplex()
    {
        RemoveJsCallerComplex(this);
    };

    static bool ReleaseObject(JsCallerComplex* data)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "ReleaseObject begin");
        if (data == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "ReleaseObject begin, but input parameters is nullptr");
            return false;
        }

        if (!data->ChangeCurrentState(OBJSTATE::OBJ_RELEASE)) {
            auto handler = data->GetEventHandler();
            if (handler == nullptr) {
                TAG_LOGE(AAFwkTag::DEFAULT, "ReleaseObject error end, Get eventHandler failed");
                return false;
            }
            auto releaseObjTask = [pdata = data] () {
                if (!FindJsCallerComplex(pdata)) {
                    TAG_LOGE(AAFwkTag::DEFAULT, "ReleaseObject error end, but input parameters does not found");
                    return;
                }
                ReleaseObject(pdata);
            };

            handler->PostTask(releaseObjTask, "FinalizerRelease");
            return false;
        } else {
            // when the object is about to be destroyed, does not reset state
            std::unique_ptr<JsCallerComplex> delObj(data);
        }
        TAG_LOGD(AAFwkTag::DEFAULT, "ReleaseObject success end");
        return true;
    }

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s begin.", __func__);
        if (data == nullptr) {
            TAG_LOGE(
                AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s is called, but input parameters is nullptr", __func__);
            return;
        }

        auto ptr = static_cast<JsCallerComplex*>(data);
        if (!FindJsCallerComplex(ptr)) {
            TAG_LOGE(AAFwkTag::DEFAULT,
                "JsCallerComplex::%{public}s is called, but input parameters does not found", __func__);
            return;
        }

        ReleaseObject(ptr);
        TAG_LOGD(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s end.", __func__);
    }

    static napi_value JsReleaseCall(napi_env env, napi_callback_info info)
    {
        if (env == nullptr || info == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT,
                "JsCallerComplex::%{public}s is called, but input parameters %{public}s is nullptr", __func__,
                ((env == nullptr) ? "env" : "info"));
            return nullptr;
        }
        GET_NAPI_INFO_AND_CALL(env, info, JsCallerComplex, ReleaseCallInner);
    }

    static napi_value JsSetOnReleaseCallBack(napi_env env, napi_callback_info info)
    {
        if (env == nullptr || info == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT,
                "JsCallerComplex::%{public}s is called, but input parameters %{public}s is nullptr", __func__,
                ((env == nullptr) ? "env" : "info"));
            return nullptr;
        }
        GET_NAPI_INFO_AND_CALL(env, info, JsCallerComplex, SetOnReleaseCallBackInner);
    }

    static napi_value JsSetOnRemoteStateChanged(napi_env env, napi_callback_info info)
    {
        if (env == nullptr || info == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT,
                "JsCallerComplex::%{public}s is called, but input parameters %{public}s is nullptr", __func__,
                ((env == nullptr) ? "env" : "info"));
            return nullptr;
        }
        GET_NAPI_INFO_AND_CALL(env, info, JsCallerComplex, SetOnRemoteStateChangedInner);
    }

    static bool AddJsCallerComplex(JsCallerComplex* ptr)
    {
        if (ptr == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsAbilityContext::%{public}s, input parameters is nullptr", __func__);
            return false;
        }

        std::lock_guard<std::mutex> lck (jsCallerComplexMutex);
        auto iter = jsCallerComplexManagerList.find(ptr);
        if (iter != jsCallerComplexManagerList.end()) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsAbilityContext::%{public}s, address exists", __func__);
            return false;
        }

        auto iterRet = jsCallerComplexManagerList.emplace(ptr);
        TAG_LOGD(AAFwkTag::DEFAULT, "JsAbilityContext::%{public}s, execution ends and retval is %{public}s", __func__,
            iterRet.second ? "true" : "false");
        return iterRet.second;
    }

    static bool RemoveJsCallerComplex(JsCallerComplex* ptr)
    {
        if (ptr == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsAbilityContext::%{public}s, input parameters is nullptr", __func__);
            return false;
        }

        std::lock_guard<std::mutex> lck (jsCallerComplexMutex);
        auto iter = jsCallerComplexManagerList.find(ptr);
        if (iter == jsCallerComplexManagerList.end()) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsAbilityContext::%{public}s, input parameters not found", __func__);
            return false;
        }

        jsCallerComplexManagerList.erase(ptr);
        TAG_LOGD(AAFwkTag::DEFAULT, "JsAbilityContext::%{public}s, called", __func__);
        return true;
    }

    static bool FindJsCallerComplex(JsCallerComplex* ptr)
    {
        if (ptr == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsAbilityContext::%{public}s, input parameters is nullptr", __func__);
            return false;
        }
        auto ret = true;
        std::lock_guard<std::mutex> lck (jsCallerComplexMutex);
        auto iter = jsCallerComplexManagerList.find(ptr);
        if (iter == jsCallerComplexManagerList.end()) {
            ret = false;
        }
        TAG_LOGD(AAFwkTag::DEFAULT, "JsAbilityContext::%{public}s, execution ends and retval is %{public}s", __func__,
            ret ? "true" : "false");
        return ret;
    }

    static bool FindJsCallerComplexAndChangeState(JsCallerComplex* ptr, OBJSTATE state)
    {
        if (ptr == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsAbilityContext::%{public}s, input parameters is nullptr", __func__);
            return false;
        }

        std::lock_guard<std::mutex> lck (jsCallerComplexMutex);
        auto iter = jsCallerComplexManagerList.find(ptr);
        if (iter == jsCallerComplexManagerList.end()) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsAbilityContext::%{public}s, execution end, but not found", __func__);
            return false;
        }

        auto ret = ptr->ChangeCurrentState(state);
        TAG_LOGD(AAFwkTag::DEFAULT,
            "JsAbilityContext::%{public}s, execution ends and ChangeCurrentState retval is %{public}s", __func__,
            ret ? "true" : "false");

        return ret;
    }

    sptr<IRemoteObject> GetRemoteObject()
    {
        return callee_;
    }

    std::shared_ptr<AppExecFwk::EventHandler> GetEventHandler()
    {
        return handler_;
    }

    bool ChangeCurrentState(OBJSTATE state)
    {
        auto ret = false;
        if (stateMechanismMutex_.try_lock() == false) {
            TAG_LOGE(AAFwkTag::DEFAULT, "mutex try_lock false");
            return ret;
        }

        if (currentState_ == OBJSTATE::OBJ_NORMAL) {
            currentState_ = state;
            ret = true;
            TAG_LOGD(AAFwkTag::DEFAULT, "currentState_ == OBJSTATE::OBJ_NORMAL");
        } else if (currentState_ == state) {
            ret = true;
            TAG_LOGD(AAFwkTag::DEFAULT, "currentState_ == state");
        } else {
            ret = false;
            TAG_LOGD(AAFwkTag::DEFAULT, "ret = false");
        }

        stateMechanismMutex_.unlock();
        return ret;
    }

    OBJSTATE GetCurrentState()
    {
        return currentState_;
    }

    void StateReset()
    {
        currentState_ = OBJSTATE::OBJ_NORMAL;
    }

private:

    void OnReleaseNotify(const std::string &str)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "OnReleaseNotify begin");
        if (handler_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "handler parameters error");
            return;
        }

        auto task = [notify = this, &str] () {
            if (!FindJsCallerComplex(notify)) {
                TAG_LOGE(AAFwkTag::DEFAULT, "ptr not found, address error");
                return;
            }
            notify->OnReleaseNotifyTask(str);
        };
        handler_->PostSyncTask(task, "OnReleaseNotify");
        TAG_LOGD(AAFwkTag::DEFAULT, "OnReleaseNotify end");
    }

    void OnReleaseNotifyTask(const std::string &str)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "OnReleaseNotifyTask begin");
        if (jsReleaseCallBackObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, jsreleaseObj is nullptr", __func__);
            return;
        }

        napi_value value = jsReleaseCallBackObj_->GetNapiValue();
        napi_value callback = jsReleaseCallBackObj_->GetNapiValue();
        napi_value args[] = { CreateJsValue(releaseCallBackEngine_, str) };
        napi_call_function(releaseCallBackEngine_, value, callback, 1, args, nullptr);
        TAG_LOGD(AAFwkTag::DEFAULT, "OnReleaseNotifyTask CallFunction call done");
        callee_ = nullptr;
        StateReset();
        TAG_LOGD(AAFwkTag::DEFAULT, "OnReleaseNotifyTask end");
    }

    void OnRemoteStateChangedNotify(const std::string &str)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "OnRemoteStateChangedNotify begin");
        if (handler_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "handler parameters error");
            return;
        }

        auto task = [notify = this, &str] () {
            if (!FindJsCallerComplex(notify)) {
                TAG_LOGE(AAFwkTag::DEFAULT, "ptr not found, address error");
                return;
            }
            notify->OnRemoteStateChangedNotifyTask(str);
        };
        handler_->PostSyncTask(task, "OnRemoteStateChangedNotify");
        TAG_LOGD(AAFwkTag::DEFAULT, "OnRemoteStateChangedNotify end");
    }

    void OnRemoteStateChangedNotifyTask(const std::string &str)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "OnRemoteStateChangedNotifyTask begin");
        if (jsRemoteStateChangedObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, jsRemoteStateChangedObj is nullptr", __func__);
            return;
        }

        napi_value value = jsRemoteStateChangedObj_->GetNapiValue();
        napi_value callback = jsRemoteStateChangedObj_->GetNapiValue();
        napi_value args[] = { CreateJsValue(remoteStateChanegdEngine_, str) };
        napi_call_function(remoteStateChanegdEngine_, value, callback, 1, args, nullptr);
        TAG_LOGD(AAFwkTag::DEFAULT, "OnRemoteStateChangedNotifyTask CallFunction call done");
        StateReset();
        TAG_LOGD(AAFwkTag::DEFAULT, "OnRemoteStateChangedNotifyTask end");
    }

    napi_value ReleaseCallInner(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, called", __func__);
        if (callerCallBackObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, CallBacker is nullptr", __func__);
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        }

        if (!releaseCallFunc_) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, releaseFunc is nullptr", __func__);
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        }
        int32_t innerErrorCode = releaseCallFunc_(callerCallBackObj_);
        if (innerErrorCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, ReleaseAbility failed %{public}d",
                __func__, static_cast<int>(innerErrorCode));
            ThrowError(env, innerErrorCode);
        }

        return CreateJsUndefined(env);
    }

    napi_value SetOnReleaseCallBackInner(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, start", __func__);
        constexpr size_t argcOne = 1;
        if (info.argc < argcOne) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, Invalid input parameters", __func__);
            ThrowTooFewParametersError(env);
        }
        bool isCallable = false;
        napi_is_callable(env, info.argv[0], &isCallable);
        if (!isCallable) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, IsCallable is %{public}s.",
                __func__, isCallable ? "true" : "false");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        }

        if (callerCallBackObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, CallBacker is null", __func__);
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        }

        auto param1 = info.argv[0];
        if (param1 == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, param1 is null", __func__);
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        }

        napi_ref ref = nullptr;
        napi_create_reference(releaseCallBackEngine_, param1, 1, &ref);
        jsReleaseCallBackObj_.reset(reinterpret_cast<NativeReference*>(ref));
        auto task = [notify = this] (const std::string &str) {
            if (!FindJsCallerComplexAndChangeState(notify, OBJSTATE::OBJ_EXECUTION)) {
                TAG_LOGE(AAFwkTag::DEFAULT, "ptr not found, address error");
                return;
            }
            notify->OnReleaseNotify(str);
        };
        callerCallBackObj_->SetOnRelease(task);
        TAG_LOGD(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, end", __func__);
        return CreateJsUndefined(env);
    }

    napi_value SetOnRemoteStateChangedInner(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, begin", __func__);
        constexpr size_t argcOne = 1;
        if (info.argc < argcOne) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, Invalid input params", __func__);
            ThrowTooFewParametersError(env);
        }
        bool isCallable = false;
        napi_is_callable(env, info.argv[0], &isCallable);
        if (!isCallable) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, IsCallable is %{public}s.",
                __func__, isCallable ? "true" : "false");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        }

        if (callerCallBackObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, CallBacker is nullptr", __func__);
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        }

        auto param1 = info.argv[0];
        if (param1 == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, param1 is nullptr", __func__);
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        }

        napi_ref ref = nullptr;
        napi_create_reference(remoteStateChanegdEngine_, param1, 1, &ref);
        jsRemoteStateChangedObj_.reset(reinterpret_cast<NativeReference*>(ref));
        auto task = [notify = this] (const std::string &str) {
            TAG_LOGI(AAFwkTag::DEFAULT, "state changed");
            if (!FindJsCallerComplexAndChangeState(notify, OBJSTATE::OBJ_EXECUTION)) {
                TAG_LOGE(AAFwkTag::DEFAULT, "ptr not found, address error");
                return;
            }
            notify->OnRemoteStateChangedNotify(str);
        };
        callerCallBackObj_->SetOnRemoteStateChanged(task);
        TAG_LOGD(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, end", __func__);
        return CreateJsUndefined(env);
    }

private:
    ReleaseCallFunc releaseCallFunc_;
    sptr<IRemoteObject> callee_;
    napi_env releaseCallBackEngine_;
    napi_env remoteStateChanegdEngine_;
    std::shared_ptr<CallerCallBack> callerCallBackObj_;
    std::unique_ptr<NativeReference> jsReleaseCallBackObj_;
    std::unique_ptr<NativeReference> jsRemoteStateChangedObj_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    std::mutex stateMechanismMutex_;
    OBJSTATE currentState_;

    static std::set<JsCallerComplex*> jsCallerComplexManagerList;
    static std::mutex jsCallerComplexMutex;
};

std::set<JsCallerComplex*> JsCallerComplex::jsCallerComplexManagerList;
std::mutex JsCallerComplex::jsCallerComplexMutex;
} // nameless

napi_value CreateJsCallerComplex(
    napi_env env, ReleaseCallFunc releaseCallFunc, sptr<IRemoteObject> callee,
    std::shared_ptr<CallerCallBack> callerCallBack)
{
    TAG_LOGD(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, begin", __func__);
    if (callee == nullptr || callerCallBack == nullptr || releaseCallFunc == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s is called, input params error. %{public}s is nullptr", __func__,
            (callee == nullptr) ? ("callee") :
            ((releaseCallFunc == nullptr) ? ("releaseCallFunc") : ("callerCallBack")));
        return CreateJsUndefined(env);
    }

    napi_value object = nullptr;
    napi_create_object(env, &object);
    auto jsCaller = std::make_unique<JsCallerComplex>(env, releaseCallFunc, callee, callerCallBack);
    auto remoteObj = jsCaller->GetRemoteObject();
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s is called,remoteObj is nullptr", __func__);
        return CreateJsUndefined(env);
    }

    napi_wrap(env, object, jsCaller.release(), JsCallerComplex::Finalizer, nullptr, nullptr);
    napi_set_named_property(env, object, "callee", CreateJsCalleeRemoteObject(env, remoteObj));
    const char *moduleName = "JsCallerComplex";
    BindNativeFunction(env, object, "release", moduleName, JsCallerComplex::JsReleaseCall);
    BindNativeFunction(env, object, "onRelease", moduleName, JsCallerComplex::JsSetOnReleaseCallBack);
    BindNativeFunction(env, object, "onRemoteStateChange", moduleName, JsCallerComplex::JsSetOnRemoteStateChanged);

    TAG_LOGD(AAFwkTag::DEFAULT, "JsCallerComplex::%{public}s, end", __func__);
    return object;
}

napi_value CreateJsCalleeRemoteObject(napi_env env, sptr<IRemoteObject> callee)
{
    if (callee == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s is called, input params is nullptr", __func__);
        return CreateJsUndefined(env);
    }
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(env, callee);
    if (napiRemoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s is called, but remoteObj is nullptr", __func__);
    }
    return napiRemoteObject;
}
} // AbilityRuntime
} // OHOS
