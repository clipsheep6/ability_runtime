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

#include "js_ui_service_extension_context.h"

#include <chrono>
#include <cstdint>

#include "ability_manager_client.h"
#include "ability_runtime/js_caller_complex.h"
#include "ui_service_extension.h"
#include "hilog_tag_wrapper.h"
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
#include "js_free_install_observer.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t INDEX_THREE = 3;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
static std::mutex g_connectsMutex;
static std::map<ConnectionKey, sptr<JSUIServiceExtensionContext>, key_compare> g_connects;

class JSUIServiceExtensionContext final {
public:
    explicit JSUIServiceExtensionContext(
        const std::shared_ptr<UIServiceExtensionContext>& context) : context_(context) {}
    ~JSUIServiceExtensionContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "JsAbilityContext::Finalizer is called");
        std::unique_ptr<JSUIServiceExtensionContext>(static_cast<JSUIServiceExtensionContext*>(data));
    }

    static napi_value StartAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JSUIServiceExtensionContext, OnStartAbility);
    }

    static napi_value TerminateSelf(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JSUIServiceExtensionContext, OnTerminateSelf);
    }

    static napi_value StartAbilityByType(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JSUIServiceExtensionContext, OnStartAbilityByType);
    }
private:
    std::weak_ptr<UIServiceExtensionContext> context_;
    sptr<JsFreeInstallObserver> freeInstallObserver_ = nullptr;
    void AddFreeInstallObserver(napi_env env, const AAFwk::Want &want, napi_value callback, napi_value* result)
    {
        // adapter free install async return install and start result
        int ret = 0;
        if (freeInstallObserver_ == nullptr) {
            freeInstallObserver_ = new JsFreeInstallObserver(env);
            ret = AAFwk::AbilityManagerClient::GetInstance()->AddFreeInstallObserver(freeInstallObserver_);
        }

        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "AddFreeInstallObserver failed.");
        } else {
            // build a callback observer with last param
            std::string bundleName = want.GetElement().GetBundleName();
            std::string abilityName = want.GetElement().GetAbilityName();
            std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
            freeInstallObserver_->AddJsObserverObject(
                bundleName, abilityName, startTime, callback, result);
        }
    }

    napi_value OnStartAbility(napi_env env, NapiCallbackInfo& info, bool isStartRecent = false)
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
        TAG_LOGI(AAFwkTag::UISERVC_EXT, "StartAbility");
        if (info.argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "Start ability failed, not enough params.");
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
            TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnStartRecentAbility is called");
            want.SetParam(Want::PARAM_RESV_START_RECENT, true);
        }

        if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
            std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
                system_clock::now().time_since_epoch()).count());
            want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
        }

        auto innerErrorCode = std::make_shared<int>(ERR_OK);
        auto execute = GetStartAbilityExecFunc(want, startOptions, DEFAULT_INVAL_VALUE, unwrapArgc != 1, innerErrorCode);
        auto complete = GetSimpleCompleteFunc(innerErrorCode);

        napi_value lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
        napi_value result = nullptr;
        if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
            AddFreeInstallObserver(env, want, lastParam, &result);
            NapiAsyncTask::ScheduleHighQos("JSServiceExtensionContext::OnStartAbility", env,
                CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), nullptr, nullptr));
        } else {
            NapiAsyncTask::ScheduleHighQos("JSServiceExtensionContext::OnStartAbility", env,
                CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        }
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
            TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnStartAbility start options is used.");
            AppExecFwk::UnwrapStartOptions(env, info.argv[1], startOptions);
            unwrapArgc++;
        }
        return true;
    }

    napi_value OnTerminateSelf(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::UISERVC_EXT, "OnTerminateSelf");

        NapiAsyncTask::CompleteCallback complete =
            [weak = context_](napi_env env, NapiAsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    TAG_LOGW(AAFwkTag::UISERVC_EXT, "context is released");
                    task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                    return;
                }

                ErrCode innerErrorCode = context->TerminateSelf();
                if (innerErrorCode == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
                }
            };

        napi_value lastParam = (info.argc == ARGC_ZERO) ? nullptr : info.argv[INDEX_ZERO];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSUIServiceExtensionContext::OnTerminateSelf",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnStartAbilityByType(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::UISERVC_EXT,"OnStartAbilityByType");
        if (info.argc < ARGC_THREE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OnStartAbilityByType, Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
        }

        std::string type;
        if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], type)) {
            TAG_LOGE(AAFwkTag::CONTEXT, "OnStartAbilityByType, parse type failed.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        AAFwk::WantParams wantParam;
        if (!AppExecFwk::UnwrapWantParams(env, info.argv[INDEX_ONE], wantParam)) {
            TAG_LOGE(AAFwkTag::CONTEXT, "OnStartAbilityByType, parse wantParam failed.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        std::shared_ptr<JsUIExtensionCallback> callback = std::make_shared<JsUIExtensionCallback>(env);
        callback->SetJsCallbackObject(info.argv[INDEX_TWO]);
        NapiAsyncTask::CompleteCallback complete =
            [weak = context_, type, wantParam, callback](napi_env env, NapiAsyncTask& task, int32_t status) mutable {
                auto context = weak.lock();
                if (!context) {
                    TAG_LOGW(AAFwkTag::CONTEXT, "OnStartAbilityByType context is released");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    return;
                }

                auto errcode = context->StartAbilityByType(type, wantParam, callback);
                if (errcode != 0) {
                    task.Reject(env, CreateJsErrorByNativeErr(env, errcode));
                } else {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                }
            };

        napi_value lastParam = (info.argc > ARGC_THREE) ? info.argv[INDEX_THREE] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsAbilityContext::OnStartAbilityByType",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
    
    NapiAsyncTask::ExecuteCallback GetStartAbilityExecFunc(const AAFwk::Want &want,
        const AAFwk::StartOptions &startOptions, int32_t userId, bool useOption, std::shared_ptr<int> retCode)
        {
            return [weak = context_, want, startOptions, useOption, userId, retCode,
                &observer = freeInstallObserver_]() {
                TAG_LOGD(AAFwkTag::UISERVC_EXT, "startAbility exec begin");
                if (!retCode) {
                    TAG_LOGE(AAFwkTag::UISERVC_EXT, "retCode null");
                    return;
                }
                auto context = weak.lock();
                if (!context) {
                    TAG_LOGW(AAFwkTag::UISERVC_EXT, "context is released");
                    *retCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
                    return;
                }

                useOption ? *retCode = context->StartAbilityWithAccount(want, userId, startOptions) :
                    *retCode = context->StartAbilityWithAccount(want, userId);
                if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND &&
                *retCode != 0 && observer != nullptr) {
                std::string bundleName = want.GetElement().GetBundleName();
                std::string abilityName = want.GetElement().GetAbilityName();
                std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
                observer->OnInstallFinished(bundleName, abilityName, startTime, *retCode);
                }
            };
        }

    NapiAsyncTask::CompleteCallback GetSimpleCompleteFunc(std::shared_ptr<int> retCode)
    {
        return [retCode](napi_env env, NapiAsyncTask& task, int32_t) {
            if (!retCode) {
                TAG_LOGE(AAFwkTag::UISERVC_EXT, "StartAbility is success");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            if (*retCode == 0) {
                TAG_LOGI(AAFwkTag::UISERVC_EXT, "StartAbility is success");
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *retCode));
            }
        };
    }
};
} // namespace

napi_value CreateJsUIServiceExtensionContext(napi_env env, std::shared_ptr<UIServiceExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "CreateJSUIServiceExtensionContext");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }
    napi_value object = CreateJsExtensionContext(env, context, abilityInfo);

    std::unique_ptr<JSUIServiceExtensionContext> jsUiContext
        = std::make_unique<JSUIServiceExtensionContext>(context);
    napi_wrap(env, object, jsUiContext.release(), JSUIServiceExtensionContext::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsUIServiceExtensionContext";
    BindNativeFunction(env, object, "startAbility", moduleName, JSUIServiceExtensionContext::StartAbility);
    BindNativeFunction(env, object, "terminateSelf", moduleName, JSUIServiceExtensionContext::TerminateSelf);
    BindNativeFunction(env, object, "startAbilityByType", moduleName,
        JSUIServiceExtensionContext::StartAbilityByType);
    return object;
}

JSUIServiceExtensionContext::JSUIServiceExtensionContext(napi_env env) : env_(env) {}

JSUIServiceExtensionContext::~JSUIServiceExtensionContext()
{
    if (uiJsConnectionObject_ == nullptr) {
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
    work->data = reinterpret_cast<void *>(uiJsConnectionObject_.release());
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

void JSUIServiceExtensionContext::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnAbilityConnectDone, resultCode:%{public}d", resultCode);
    wptr<JSUIServiceExtensionContext> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, remoteObject, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSUIServiceExtensionContext> connectionSptr = connection.promote();
            if (!connectionSptr) {
                TAG_LOGE(AAFwkTag::UISERVC_EXT, "connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
        });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSUIServiceExtensionContext::OnAbilityConnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSUIServiceExtensionContext::HandleOnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "resultCode:%{public}d", resultCode);
    // wrap ElementName
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);

    // wrap RemoteObject
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(env_, remoteObject);
    napi_value argv[] = {napiElementName, napiRemoteObject};
    if (uiJsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "uiJsConnectionObject_ nullptr.");
        return;
    }
    napi_value obj = uiJsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "error to get object");
        return;
    }
    napi_value methodOnConnect = nullptr;
    napi_get_named_property(env_, obj, "onConnect", &methodOnConnect);
    if (methodOnConnect == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get onConnect from object");
        return;
    }
    napi_call_function(env_, obj, methodOnConnect, ARGC_TWO, argv, nullptr);
}

void JSUIServiceExtensionContext::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    wptr<JSUIServiceExtensionContext> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSUIServiceExtensionContext> connectionSptr = connection.promote();
            if (!connectionSptr) {
                TAG_LOGI(AAFwkTag::UISERVC_EXT, "connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSUIServiceExtensionContext::OnAbilityDisconnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSUIServiceExtensionContext::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "HandleOnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);
    napi_value argv[] = {napiElementName};
    if (uiJsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "uiJsConnectionObject_ nullptr");
        return;
    }
    napi_value obj = uiJsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "error to get object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onDisconnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get onDisconnect from object");
        return;
    }

    // release connect
    {
        std::lock_guard guard(g_connectsMutex);
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnAbilityDisconnectDone g_connects.size:%{public}zu", g_connects.size());
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
            TAG_LOGD(
                AAFwkTag::UISERVC_EXT, "OnAbilityDisconnectDone erase g_connects.size:%{public}zu", g_connects.size());
        }
    }
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
}

void JSUIServiceExtensionContext::SetJsConnectionObject(napi_value jsConnectionObject)
{
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsConnectionObject, 1, &ref);
    uiJsConnectionObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
}

void JSUIServiceExtensionContext::RemoveConnectionObject()
{
    uiJsConnectionObject_.reset();
}
} // namespace AbilityRuntime
}  // namespace OHOS