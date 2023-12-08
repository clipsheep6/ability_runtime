/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "js_vpn_extension_context.h"

#include <chrono>
#include <cstdint>

#include "ability_manager_client.h"
#include "ability_runtime/js_caller_complex.h"
#include "hilog_wrapper.h"
#include "js_extension_context.h"
#include "js_error_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_ability.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "napi_remote_object.h"
#include "napi_common_start_options.h"
#include "start_options.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr int32_t ERROR_CODE_TWO = 2;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;

class StartAbilityByCallParameters {
public:
    int err = 0;
    sptr<IRemoteObject> remoteCallee = nullptr;
    std::shared_ptr<CallerCallBack> callerCallBack = nullptr;
    std::mutex mutexlock;
    std::condition_variable condition;
};

static std::map<ConnectionKey, sptr<JSVpnExtensionConnection>, key_compare> g_connects;
static int64_t g_serialNumber = 0;

void RemoveConnection(int64_t connectId)
{
    HILOG_DEBUG("enter");
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
    [&connectId](const auto &obj) {
        return connectId == obj.first.id;
    });
    if (item != g_connects.end()) {
        HILOG_DEBUG("remove conn ability exist.");
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_connects.erase(item);
    } else {
        HILOG_DEBUG("remove conn ability not exist.");
    }
}

class JsVpnExtensionContext final {
public:
    explicit JsVpnExtensionContext(const std::shared_ptr<VpnExtensionContext>& context) : context_(context) {}
    ~JsVpnExtensionContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        HILOG_DEBUG("JsAbilityContext::Finalizer is called");
        std::unique_ptr<JsVpnExtensionContext>(static_cast<JsVpnExtensionContext*>(data));
    }

    static napi_value StartAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsVpnExtensionContext, OnStartAbility);
    }

    static napi_value StartAbilityAsCaller(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsVpnExtensionContext, OnStartAbilityAsCaller);
    }

    static napi_value StartRecentAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsVpnExtensionContext, OnStartRecentAbility);
    }

    static napi_value StartAbilityByCall(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsVpnExtensionContext, OnStartAbilityByCall);
    }

    static napi_value StartAbilityWithAccount(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsVpnExtensionContext, OnStartAbilityWithAccount);
    }

    static napi_value ConnectAbilityWithAccount(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsVpnExtensionContext, OnConnectAbilityWithAccount);
    }

    static napi_value TerminateAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsVpnExtensionContext, OnTerminateAbility);
    }

    static napi_value ConnectAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsVpnExtensionContext, OnConnectAbility);
    }

    static napi_value DisconnectAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsVpnExtensionContext, OnDisconnectAbility);
    }

    static napi_value StartVpnExtensionAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsVpnExtensionContext, OnStartExtensionAbility);
    }

    static napi_value StartVpnExtensionAbilityWithAccount(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsVpnExtensionContext, OnStartExtensionAbilityWithAccount);
    }

    static napi_value StopVpnExtensionAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsVpnExtensionContext, OnStopExtensionAbility);
    }

    static napi_value StopVpnExtensionAbilityWithAccount(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsVpnExtensionContext, OnStopExtensionAbilityWithAccount);
    }

private:
    std::weak_ptr<VpnExtensionContext> context_;
    sptr<JsFreeInstallObserver> freeInstallObserver_ = nullptr;
    static void ClearFailedCallConnection(
        const std::weak_ptr<VpnExtensionContext>& vpnContext, const std::shared_ptr<CallerCallBack> &callback)
    {
        HILOG_DEBUG("clear failed call of startup is called.");
        auto context = vpnContext.lock();
        if (context == nullptr || callback == nullptr) {
            HILOG_ERROR("clear failed call of startup input param is nullptr.");
            return;
        }

        context->ClearFailedCallConnection(callback);
    }

    void AddFreeInstallObserver(napi_env env, const AAFwk::Want &want, napi_value callback)
    {
        // adapter free install async return install and start result
        int ret = 0;
        if (freeInstallObserver_ == nullptr) {
            freeInstallObserver_ = new JsFreeInstallObserver(env);
            ret = AAFwk::AbilityManagerClient::GetInstance()->AddFreeInstallObserver(freeInstallObserver_);
        }

        if (ret != ERR_OK) {
            HILOG_ERROR("AddFreeInstallObserver failed.");
        } else {
            // build a callback observer with last param
            std::string bundleName = want.GetElement().GetBundleName();
            std::string abilityName = want.GetElement().GetAbilityName();
            std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
            freeInstallObserver_->AddJsObserverObject(
                bundleName, abilityName, startTime, callback);
        }
    }

    napi_value OnStartAbility(napi_env env, NapiCallbackInfo& info,
                bool isStartRecent = false)
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
        HILOG_INFO("StartAbility");
        if (info.argc < ARGC_ONE) {
            HILOG_ERROR("Start ability failed, not enough params.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        size_t unwrapArgc = 0;
        AAFwk::Want want;
        AAFwk::StartOptions startOptions;
        if (!CheckStartAbilityInputParam(env, info, want, startOptions, unwrapArgc)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        if (isStartRecent) {
            HILOG_DEBUG("OnStartRecentAbility is called");
            want.SetParam(Want::PARAM_RESV_START_RECENT, true);
        }

        if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
            std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
                system_clock::now().time_since_epoch()).count());
            want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
        }

        auto innerErrorCode = std::make_shared<int>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute = [weak = context_, want, startOptions, unwrapArgc, innerErrorCode,
            &observer = freeInstallObserver_]() {
            HILOG_DEBUG("startAbility begin");
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                *innerErrorCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
                return;
            }

            (unwrapArgc == 1) ? *innerErrorCode = context->StartAbility(want) :
                *innerErrorCode = context->StartAbility(want, startOptions);
            if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND &&
                *innerErrorCode != 0 && observer != nullptr) {
                std::string bundleName = want.GetElement().GetBundleName();
                std::string abilityName = want.GetElement().GetAbilityName();
                std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
                observer->OnInstallFinished(bundleName, abilityName, startTime, *innerErrorCode);
            }
        };

        NapiAsyncTask::CompleteCallback complete =
            [innerErrorCode](napi_env env, NapiAsyncTask& task, int32_t status) {
                if (*innerErrorCode == 0) {
                    HILOG_ERROR("success to StartAbility");
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
                }
            };

        napi_value lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
        napi_value result = nullptr;
        if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
            AddFreeInstallObserver(env, want, lastParam);
            NapiAsyncTask::ScheduleHighQos("JSVpnExtensionContext::OnStartAbility", env,
                CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), nullptr, &result));
        } else {
            NapiAsyncTask::ScheduleHighQos("JSVpnExtensionContext::OnStartAbility", env,
                CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        }
        return result;
    }

    napi_value OnStartRecentAbility(napi_env env, NapiCallbackInfo& info, bool isStartRecent = false)
    {
        return OnStartAbility(env, info, true);
    }

    napi_value OnStartAbilityAsCaller(napi_env env, NapiCallbackInfo& info)
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
        HILOG_INFO("StartAbilityAsCaller");
        if (info.argc < ARGC_ONE) {
            HILOG_ERROR("Start ability as caller failed, not enough params.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        size_t unwrapArgc = 0;
        AAFwk::Want want;
        AAFwk::StartOptions startOptions;
        if (!CheckStartAbilityInputParam(env, info, want, startOptions, unwrapArgc)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [weak = context_, want, startOptions, unwrapArgc](napi_env env, NapiAsyncTask& task, int32_t status) {
                HILOG_DEBUG("startAbility begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    return;
                }

                ErrCode innerErrorCode = ERR_OK;
                (unwrapArgc == 1) ? innerErrorCode = context->StartAbilityAsCaller(want) :
                    innerErrorCode = context->StartAbilityAsCaller(want, startOptions);
                if (innerErrorCode == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
                }
            };

        napi_value lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSVpnExtensionContext::OnStartAbilityAsCaller",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    bool CheckStartAbilityInputParam(napi_env env, NapiCallbackInfo& info,
        AAFwk::Want& want, AAFwk::StartOptions& startOptions, size_t& unwrapArgc) const
    {
        if (info.argc < ARGC_ONE) {
            return false;
        }
        unwrapArgc = ARGC_ZERO;
        // Check input want
        if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
            return false;
        }
        ++unwrapArgc;
        if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[1], napi_object)) {
            HILOG_DEBUG("OnStartAbility start options is used.");
            AppExecFwk::UnwrapStartOptions(env, info.argv[1], startOptions);
            unwrapArgc++;
        }
        return true;
    }

    napi_value OnStartAbilityByCall(napi_env env, NapiCallbackInfo& info)
    {
        HILOG_INFO("StartAbilityByCall");
        if (info.argc < ARGC_ONE) {
            HILOG_ERROR("Start ability by call failed, not enough params.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        AAFwk::Want want;
        int32_t accountId = DEFAULT_INVAL_VALUE;
        if (!CheckStartAbilityByCallInputParam(env, info, want, accountId)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        std::shared_ptr<StartAbilityByCallParameters> calls = std::make_shared<StartAbilityByCallParameters>();
        napi_value retsult = nullptr;
        calls->callerCallBack = std::make_shared<CallerCallBack>();
        calls->callerCallBack->SetCallBack(GetCallBackDone(calls));
        calls->callerCallBack->SetOnRelease(GetReleaseListen());

        auto context = context_.lock();
        if (context == nullptr) {
            HILOG_ERROR("OnStartAbilityByCall context is nullptr");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }

        auto ret = context->StartAbilityByCall(want, calls->callerCallBack, accountId);
        if (ret) {
            HILOG_ERROR("OnStartAbilityByCall is failed");
            ThrowErrorByNativeErr(env, ret);
            return CreateJsUndefined(env);
        }

        if (calls->remoteCallee == nullptr) {
            HILOG_DEBUG("OnStartAbilityByCall async wait execute");
            NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnStartAbilityByCall", env,
                CreateAsyncTaskWithLastParam(
                    env, nullptr, GetCallExecute(calls), GetCallComplete(calls), &retsult));
        } else {
            NapiAsyncTask::ScheduleHighQos("JSVpnExtensionContext::OnStartAbilityByCall", env,
                CreateAsyncTaskWithLastParam(env, nullptr, nullptr, GetCallComplete(calls), &retsult));
        }
        return retsult;
    }

    bool CheckStartAbilityByCallInputParam(
        napi_env env, NapiCallbackInfo& info, AAFwk::Want& want, int32_t& accountId)
    {
        if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
            return false;
        }

        if (info.argc > static_cast<size_t>(INDEX_ONE)) {
            if (CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_number)) {
                if (!ConvertFromJsValue(env, info.argv[1], accountId)) {
                    HILOG_ERROR("check input param accountId failed");
                    return false;
                }
            } else {
                HILOG_ERROR("input parameter type invalid");
                return false;
            }
        }

        HILOG_INFO("CheckStartAbilityByCallInputParam, callee:%{public}s.%{public}s.",
            want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        return true;
    }

    NapiAsyncTask::CompleteCallback GetCallComplete(std::shared_ptr<StartAbilityByCallParameters> calls)
    {
        auto callComplete = [weak = context_, calldata = calls] (
            napi_env env, NapiAsyncTask& task, int32_t) {
            if (calldata->err != 0) {
                HILOG_ERROR("OnStartAbilityByCall callComplete err is %{public}d", calldata->err);
                ClearFailedCallConnection(weak, calldata->callerCallBack);
                task.Reject(env, CreateJsError(env, calldata->err, "callComplete err."));
                return;
            }

            auto context = weak.lock();
            if (context != nullptr && calldata->callerCallBack != nullptr && calldata->remoteCallee != nullptr) {
                auto releaseCallFunc = [weak] (
                    const std::shared_ptr<CallerCallBack> &callback) -> ErrCode {
                    auto contextForRelease = weak.lock();
                    if (contextForRelease == nullptr) {
                        HILOG_ERROR("releaseCallFunction, context is nullptr");
                        return -1;
                    }
                    return contextForRelease->ReleaseCall(callback);
                };
                task.Resolve(env,
                    CreateJsCallerComplex(
                        env, releaseCallFunc, calldata->remoteCallee, calldata->callerCallBack));
            } else {
                HILOG_ERROR("OnStartAbilityByCall callComplete params error %{public}s is nullptr",
                    context == nullptr ? "context" :
                        (calldata->remoteCallee == nullptr ? "remoteCallee" : "callerCallBack"));
                task.Reject(env, CreateJsError(env, -1, "Create Call Failed."));
            }

            HILOG_DEBUG("OnStartAbilityByCall callComplete end");
        };
        return callComplete;
    }

    NapiAsyncTask::ExecuteCallback GetCallExecute(std::shared_ptr<StartAbilityByCallParameters> calls)
    {
        auto callExecute = [calldata = calls] () {
            constexpr int callerTimeOut = 10; // 10s
            std::unique_lock<std::mutex> lock(calldata->mutexlock);
            if (calldata->remoteCallee != nullptr) {
                HILOG_INFO("OnStartAbilityByCall callExecute callee isn`t null");
                return;
            }

            if (calldata->condition.wait_for(lock, std::chrono::seconds(callerTimeOut)) == std::cv_status::timeout) {
                HILOG_ERROR("OnStartAbilityByCall callExecute waiting callee timeout");
                calldata->err = -1;
            }
            HILOG_DEBUG("OnStartAbilityByCall callExecute exit");
        };
        return callExecute;
    }

    CallerCallBack::CallBackClosure GetCallBackDone(std::shared_ptr<StartAbilityByCallParameters> calls)
    {
        auto callBackDone = [calldata = calls] (const sptr<IRemoteObject> &obj) {
            HILOG_DEBUG("OnStartAbilityByCall callBackDone mutexlock");
            std::unique_lock<std::mutex> lock(calldata->mutexlock);
            HILOG_DEBUG("OnStartAbilityByCall callBackDone remoteCallee assignment");
            calldata->remoteCallee = obj;
            calldata->condition.notify_all();
            HILOG_INFO("OnStartAbilityByCall callBackDone is called end");
        };
        return callBackDone;
    }

    CallerCallBack::OnReleaseClosure GetReleaseListen()
    {
        auto releaseListen = [](const std::string &str) {
            HILOG_DEBUG("OnStartAbilityByCall releaseListen is called %{public}s", str.c_str());
        };
        return releaseListen;
    }

    napi_value OnStartAbilityWithAccount(napi_env env,
                NapiCallbackInfo& info)
    {
        HILOG_INFO("StartAbilityWithAccount");
        if (info.argc < ARGC_TWO) {
            HILOG_ERROR("Start ability with account failed, not enough params.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        size_t unwrapArgc = 0;
        AAFwk::Want want;
        int32_t accountId = 0;
        if (!CheckStartAbilityWithAccountInputParam(env, info, want, accountId, unwrapArgc)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        AAFwk::StartOptions startOptions;
        if (info.argc > ARGC_TWO && CheckTypeForNapiValue(env, info.argv[INDEX_TWO], napi_object)) {
            HILOG_DEBUG("OnStartAbilityWithAccount start options is used.");
            AppExecFwk::UnwrapStartOptions(env, info.argv[INDEX_TWO], startOptions);
            unwrapArgc++;
        }

        if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
            std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
                system_clock::now().time_since_epoch()).count());
            want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
        }
        auto innerErrorCode = std::make_shared<int>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute = [weak = context_, want, accountId, startOptions, unwrapArgc,
            innerErrorCode, &observer = freeInstallObserver_]() {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                *innerErrorCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
                return;
            }

            (unwrapArgc == ARGC_TWO) ? *innerErrorCode = context->StartAbilityWithAccount(want, accountId) :
                *innerErrorCode = context->StartAbilityWithAccount(want, accountId, startOptions);
            if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND &&
                *innerErrorCode != 0 && observer != nullptr) {
                std::string bundleName = want.GetElement().GetBundleName();
                std::string abilityName = want.GetElement().GetAbilityName();
                std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
                observer->OnInstallFinished(bundleName, abilityName, startTime, *innerErrorCode);
            }
        };

        NapiAsyncTask::CompleteCallback complete =
            [innerErrorCode](napi_env env, NapiAsyncTask& task, int32_t status) {
                if (*innerErrorCode == 0) {
                    HILOG_ERROR("StartAbility is success");
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
                }
            };

        napi_value lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
        napi_value result = nullptr;
        if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
            AddFreeInstallObserver(env, want, lastParam);
            NapiAsyncTask::ScheduleHighQos("JSVpnExtensionContext::OnStartAbilityWithAccount", env,
                CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), nullptr, &result));
        } else {
            NapiAsyncTask::ScheduleHighQos("JSVpnExtensionContext::OnStartAbilityWithAccount", env,
                CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        }
        return result;
    }

    bool CheckStartAbilityWithAccountInputParam(
        napi_env env, NapiCallbackInfo& info,
        AAFwk::Want& want, int32_t& accountId, size_t& unwrapArgc) const
    {
        if (info.argc < ARGC_TWO) {
            return false;
        }
        unwrapArgc = ARGC_ZERO;
        // Check input want
        if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
            return false;
        }
        ++unwrapArgc;
        if (!AppExecFwk::UnwrapInt32FromJS2(env, info.argv[INDEX_ONE], accountId)) {
            return false;
        }
        ++unwrapArgc;
        return true;
    }

    napi_value OnTerminateAbility(napi_env env, NapiCallbackInfo& info)
    {
        HILOG_INFO("TerminateAbility");

        NapiAsyncTask::CompleteCallback complete =
            [weak = context_](napi_env env, NapiAsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                    return;
                }

                ErrCode innerErrorCode = context->TerminateAbility();
                if (innerErrorCode == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
                }
            };

        napi_value lastParam = (info.argc == ARGC_ZERO) ? nullptr : info.argv[INDEX_ZERO];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSVpnExtensionContext::OnTerminateAbility",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnConnectAbility(napi_env env, NapiCallbackInfo& info)
    {
        HILOG_DEBUG("ConnectAbility called.");
        // Check params count
        if (info.argc < ARGC_TWO) {
            HILOG_ERROR("Connect ability error, not enough params.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        // Unwrap want and connection
        AAFwk::Want want;
        sptr<JSVpnExtensionConnection> connection = new JSVpnExtensionConnection(env);
        if (!AppExecFwk::UnwrapWant(env, info.argv[0], want) ||
            !CheckConnectionParam(env, info.argv[1], connection, want)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        int64_t connectId = connection->GetConnectionId();
        NapiAsyncTask::CompleteCallback complete =
            [weak = context_, want, connection, connectId](napi_env env, NapiAsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_ERROR("context is released");
                    task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                    RemoveConnection(connectId);
                    return;
                }
                HILOG_DEBUG("ConnectAbility connection:%{public}d", static_cast<int32_t>(connectId));
                auto innerErrorCode = context->ConnectAbility(want, connection);
                int32_t errcode = static_cast<int32_t>(AbilityRuntime::GetJsErrorCodeByNativeError(innerErrorCode));
                if (errcode) {
                    connection->CallJsFailed(errcode);
                    RemoveConnection(connectId);
                }
                task.Resolve(env, CreateJsUndefined(env));
            };
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSVpnExtensionConnection::OnConnectAbility",
            env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
        return CreateJsValue(env, connectId);
    }

    napi_value OnConnectAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
    {
        HILOG_INFO("ConnectAbilityWithAccount");
        // Check params count
        if (info.argc < ARGC_THREE) {
            HILOG_ERROR("Connect ability failed, not enough params.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        // Unwrap want, accountId and connection
        AAFwk::Want want;
        int32_t accountId = 0;
        sptr<JSVpnExtensionConnection> connection = new JSVpnExtensionConnection(env);
        if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want) ||
            !AppExecFwk::UnwrapInt32FromJS2(env, info.argv[INDEX_ONE], accountId) ||
            !CheckConnectionParam(env, info.argv[INDEX_TWO], connection, want)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        int64_t connectId = connection->GetConnectionId();
        NapiAsyncTask::CompleteCallback complete =
            [weak = context_, want, accountId, connection, connectId](
                napi_env env, NapiAsyncTask& task, int32_t status) {
                    auto context = weak.lock();
                    if (!context) {
                        HILOG_ERROR("context is released");
                        task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                        RemoveConnection(connectId);
                        return;
                    }
                    HILOG_DEBUG("ConnectAbilityWithAccount connection:%{public}d", static_cast<int32_t>(connectId));
                    auto innerErrorCode = context->ConnectAbilityWithAccount(want, accountId, connection);
                    int32_t errcode = static_cast<int32_t>(AbilityRuntime::GetJsErrorCodeByNativeError(innerErrorCode));
                    if (errcode) {
                        connection->CallJsFailed(errcode);
                        RemoveConnection(connectId);
                    }
                    task.Resolve(env, CreateJsUndefined(env));
                };
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSVpnExtensionConnection::OnConnectAbilityWithAccount",
            env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
        return CreateJsValue(env, connectId);
    }

    bool CheckConnectionParam(napi_env env, napi_value value,
        sptr<JSVpnExtensionConnection>& connection, AAFwk::Want& want) const
    {
        if (!CheckTypeForNapiValue(env, value, napi_object)) {
            HILOG_ERROR("Failed to get connection object");
            return false;
        }
        connection->SetJsConnectionObject(value);
        ConnectionKey key;
        key.id = g_serialNumber;
        key.want = want;
        connection->SetConnectionId(key.id);
        g_connects.emplace(key, connection);
        if (g_serialNumber < INT32_MAX) {
            g_serialNumber++;
        } else {
            g_serialNumber = 0;
        }
        HILOG_DEBUG("Unable to find connection, make new one");
        return true;
    }

    napi_value OnDisconnectAbility(napi_env env, NapiCallbackInfo& info)
    {
        HILOG_INFO("DisconnectAbility start");
        if (info.argc < ARGC_ONE) {
            HILOG_ERROR("Disconnect ability error, not enough params.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int64_t connectId = -1;
        if (!AppExecFwk::UnwrapInt64FromJS2(env, info.argv[INDEX_ZERO], connectId)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        AAFwk::Want want;
        sptr<JSVpnExtensionConnection> connection = nullptr;
        FindConnection(want, connection, connectId);
        // begin disconnect
        NapiAsyncTask::CompleteCallback complete =
            [weak = context_, want, connection](
                napi_env env, NapiAsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                    return;
                }
                if (connection == nullptr) {
                    HILOG_WARN("connection null");
                    task.Reject(env, CreateJsError(env, ERROR_CODE_TWO, "not found connection"));
                    return;
                }
                HILOG_DEBUG("context->DisconnectAbility");
                auto innerErrorCode = context->DisconnectAbility(want, connection);
                if (innerErrorCode == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
                }
            };

        napi_value lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSVpnExtensionConnection::OnDisconnectAbility",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    void FindConnection(AAFwk::Want& want, sptr<JSVpnExtensionConnection>& connection, int64_t& connectId) const
    {
        HILOG_INFO("Disconnect ability begin, connection:%{public}d.", static_cast<int32_t>(connectId));
        auto item = std::find_if(g_connects.begin(),
            g_connects.end(),
            [&connectId](const auto &obj) {
                return connectId == obj.first.id;
            });
        if (item != g_connects.end()) {
            // match id
            want = item->first.want;
            connection = item->second;
            HILOG_DEBUG("find conn ability exist");
        }
        return;
    }

    napi_value OnStartExtensionAbility(napi_env env, NapiCallbackInfo& info)
    {
        HILOG_INFO("StartExtensionAbility");
        if (info.argc < ARGC_ONE) {
            HILOG_ERROR("Start extension failed, not enough params.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        AAFwk::Want want;
        if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [weak = context_, want](napi_env env, NapiAsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    return;
                }
                auto innerErrorCode = context->StartVpnExtensionAbility(want);
                if (innerErrorCode == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
                }
            };

        napi_value lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[ARGC_ONE];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSVpnExtensionContext::OnStartExtensionAbility",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnStartExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
    {
        HILOG_INFO("StartExtensionAbilityWithAccount");
        if (info.argc < ARGC_TWO) {
            HILOG_ERROR("Stop extension error, not enough params.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        AAFwk::Want want;
        int32_t accountId = -1;
        if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want) ||
            !AppExecFwk::UnwrapInt32FromJS2(env, info.argv[INDEX_ONE], accountId)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [weak = context_, want, accountId](napi_env env, NapiAsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    return;
                }
                auto innerErrorCode = context->StartVpnExtensionAbility(want, accountId);
                if (innerErrorCode == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
                }
            };

        napi_value lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[ARGC_TWO];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSVpnExtensionContext::OnStartExtensionAbilityWithAccount",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnStopExtensionAbility(napi_env env, NapiCallbackInfo& info)
    {
        HILOG_INFO("StopExtensionAbility");
        if (info.argc < ARGC_ONE) {
            HILOG_ERROR("Start extension failed, not enough params.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        AAFwk::Want want;
        if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [weak = context_, want](napi_env env, NapiAsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    return;
                }
                auto innerErrorCode = context->StopVpnExtensionAbility(want);
                if (innerErrorCode == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
                }
            };

        napi_value lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[ARGC_ONE];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSVpnExtensionContext::OnStopExtensionAbility",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnStopExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
    {
        HILOG_INFO("StopExtensionAbilityWithAccount");
        if (info.argc < ARGC_TWO) {
            HILOG_ERROR("Stop extension failed, not enough params.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        AAFwk::Want want;
        int32_t accountId = -1;
        if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want) ||
            !AppExecFwk::UnwrapInt32FromJS2(env, info.argv[INDEX_ONE], accountId)) {
            HILOG_DEBUG("Failed, input parameter type invalid");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [weak = context_, want, accountId](napi_env env, NapiAsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    return;
                }
                auto innerErrorCode = context->StopVpnExtensionAbility(want, accountId);
                if (innerErrorCode == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
                }
            };

        napi_value lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[ARGC_TWO];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSVpnExtensionContext::OnStopExtensionAbilityWithAccount",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};
} // namespace

napi_value CreateJsVpnExtensionContext(napi_env env, std::shared_ptr<VpnExtensionContext> context)
{
    HILOG_DEBUG("CreateJsVpnExtensionContext");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }
    napi_value object = CreateJsExtensionContext(env, context, abilityInfo);

    std::unique_ptr<JsVpnExtensionContext> jsContext = std::make_unique<JsVpnExtensionContext>(context);
    napi_wrap(env, object, jsContext.release(), JsVpnExtensionContext::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsVpnExtensionContext";
    BindNativeFunction(env, object, "startAbility", moduleName, JsVpnExtensionContext::StartAbility);
    BindNativeFunction(env, object, "startAbilityAsCaller",
        moduleName, JsVpnExtensionContext::StartAbilityAsCaller);
    BindNativeFunction(env, object, "terminateSelf", moduleName, JsVpnExtensionContext::TerminateAbility);
    BindNativeFunction(env, object, "connectAbility", moduleName, JsVpnExtensionContext::ConnectAbility);
    BindNativeFunction(
        env, object, "connectVpnExtensionAbility", moduleName, JsVpnExtensionContext::ConnectAbility);
    BindNativeFunction(
        env, object, "connectVpnExtensionAbility", moduleName, JsVpnExtensionContext::ConnectAbility);
    BindNativeFunction(env, object, "disconnectAbility",
        moduleName, JsVpnExtensionContext::DisconnectAbility);
    BindNativeFunction(env, object, "disconnectVpnExtensionAbility",
        moduleName, JsVpnExtensionContext::DisconnectAbility);
    BindNativeFunction(env, object, "startAbilityWithAccount",
        moduleName, JsVpnExtensionContext::StartAbilityWithAccount);
    BindNativeFunction(env, object, "startAbilityByCall",
        moduleName, JsVpnExtensionContext::StartAbilityByCall);
    BindNativeFunction(
        env, object, "connectAbilityWithAccount", moduleName, JsVpnExtensionContext::ConnectAbilityWithAccount);
    BindNativeFunction(
        env, object,
        "connectVpnExtensionAbilityWithAccount", moduleName, JsVpnExtensionContext::ConnectAbilityWithAccount);
    BindNativeFunction(env, object, "startVpnExtensionAbility", moduleName,
        JsVpnExtensionContext::StartVpnExtensionAbility);
    BindNativeFunction(env, object, "startVpnExtensionAbilityWithAccount", moduleName,
        JsVpnExtensionContext::StartVpnExtensionAbilityWithAccount);
    BindNativeFunction(env, object, "stopVpnExtensionAbility", moduleName,
        JsVpnExtensionContext::StopVpnExtensionAbility);
    BindNativeFunction(env, object, "stopVpnExtensionAbilityWithAccount", moduleName,
        JsVpnExtensionContext::StopVpnExtensionAbilityWithAccount);
    BindNativeFunction(env, object, "startRecentAbility", moduleName,
        JsVpnExtensionContext::StartRecentAbility);

    return object;
}

JSVpnExtensionConnection::JSVpnExtensionConnection(napi_env env) : env_(env) {}

JSVpnExtensionConnection::~JSVpnExtensionConnection()
{
    if (jsConnectionObject_ == nullptr) {
        return;
    }

    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        return;
    }
    work->data = reinterpret_cast<void *>(jsConnectionObject_.release());
    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
    [](uv_work_t *work, int status) {
        if (work == nullptr) {
            return;
        }
        if (work->data == nullptr) {
            delete work;
            work = nullptr;
            return;
        }
        delete reinterpret_cast<NativeReference *>(work->data);
        work->data = nullptr;
        delete work;
        work = nullptr;
    });
    if (ret != 0) {
        delete reinterpret_cast<NativeReference *>(work->data);
        work->data = nullptr;
        delete work;
        work = nullptr;
    }
}

void JSVpnExtensionConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

int64_t JSVpnExtensionConnection::GetConnectionId()
{
    return connectionId_;
}

void JSVpnExtensionConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_DEBUG("OnAbilityConnectDone, resultCode:%{public}d", resultCode);
    wptr<JSVpnExtensionConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, remoteObject, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSVpnExtensionConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                HILOG_ERROR("connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
        });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSVpnExtensionConnection::OnAbilityConnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSVpnExtensionConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("HandleOnAbilityConnectDone begin, resultCode:%{public}d", resultCode);
    // wrap ElementName
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);

    // wrap RemoteObject
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(env_, remoteObject);
    napi_value argv[] = {napiElementName, napiRemoteObject};
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr.");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        HILOG_ERROR("error to get object");
        return;
    }
    napi_value methodOnConnect = nullptr;
    napi_get_named_property(env_, obj, "onConnect", &methodOnConnect);
    if (methodOnConnect == nullptr) {
        HILOG_ERROR("Failed to get onConnect from object");
        return;
    }
    napi_call_function(env_, obj, methodOnConnect, ARGC_TWO, argv, nullptr);
}

void JSVpnExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    HILOG_DEBUG("OnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    wptr<JSVpnExtensionConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSVpnExtensionConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                HILOG_INFO("connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSVpnExtensionConnection::OnAbilityDisconnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSVpnExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    HILOG_INFO("HandleOnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);
    napi_value argv[] = {napiElementName};
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        HILOG_ERROR("error to get object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onDisconnect", &method);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onDisconnect from object");
        return;
    }

    // release connect
    HILOG_DEBUG("OnAbilityDisconnectDone g_connects.size:%{public}zu", g_connects.size());
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    auto item = std::find_if(g_connects.begin(),
        g_connects.end(),
        [bundleName, abilityName, connectionId = connectionId_](
            const auto &obj) {
            return (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName()) &&
                   connectionId == obj.first.id;
        });
    if (item != g_connects.end()) {
        // match bundlename && abilityname
        g_connects.erase(item);
        HILOG_DEBUG("OnAbilityDisconnectDone erase g_connects.size:%{public}zu", g_connects.size());
    }
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
}

void JSVpnExtensionConnection::SetJsConnectionObject(napi_value jsConnectionObject)
{
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsConnectionObject, 1, &ref);
    jsConnectionObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
}

void JSVpnExtensionConnection::RemoveConnectionObject()
{
    jsConnectionObject_.reset();
}

void JSVpnExtensionConnection::CallJsFailed(int32_t errorCode)
{
    HILOG_DEBUG("CallJsFailed begin");
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        HILOG_ERROR("Wrong to get object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onFailed", &method);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onFailed from object");
        return;
    }
    napi_value argv[] = {CreateJsValue(env_, errorCode)};
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
    HILOG_DEBUG("CallJsFailed end");
}
}  // namespace AbilityRuntime
}  // namespace OHOS
