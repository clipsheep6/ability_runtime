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

#include "js_app_state_observer.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "js_app_manager_utils.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr size_t ARGC_ONE = 1;

JSAppStateObserver::JSAppStateObserver(napi_env env) : env_(env) {}

JSAppStateObserver::~JSAppStateObserver() = default;

void JSAppStateObserver::OnForegroundApplicationChanged(const AppStateData &appStateData)
{
    HILOG_DEBUG("onForegroundApplicationChanged bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    wptr<JSAppStateObserver> jsObserver = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsObserver, appStateData](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_WARN("jsObserverSptr null");
                return;
            }
            jsObserverSptr->HandleOnForegroundApplicationChanged(appStateData);
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSAppStateObserver::OnForegroundApplicationChanged",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
    HILOG_DEBUG("OnForegroundApplicationChanged end");
}

void JSAppStateObserver::HandleOnForegroundApplicationChanged(const AppStateData &appStateData)
{
    HILOG_DEBUG("HandleOnForegroundApplicationChanged bundleName:%{public}s, uid:%{public}d, state:%{public}d.",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        napi_value argv[] = {CreateJsAppStateData(env_, appStateData)};
        CallJsFunction(obj, "onForegroundApplicationChanged", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_INFO("OnAbilityStateChanged start");
    wptr<JSAppStateObserver> jsObserver = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsObserver, abilityStateData](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_WARN("jsObserverSptr null");
                return;
            }
            jsObserverSptr->HandleOnAbilityStateChanged(abilityStateData);
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSAppStateObserver::OnAbilityStateChanged",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAppStateObserver::HandleOnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_INFO("HandleOnAbilityStateChanged start");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        napi_value argv[] = {CreateJsAbilityStateData(env_, abilityStateData)};
        CallJsFunction(obj, "onAbilityStateChanged", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_INFO("OnExtensionStateChanged start");
    wptr<JSAppStateObserver> jsObserver = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsObserver, abilityStateData](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_WARN("jsObserverSptr nullptr");
                return;
            }
            jsObserverSptr->HandleOnExtensionStateChanged(abilityStateData);
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSAppStateObserver::OnExtensionStateChanged",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAppStateObserver::HandleOnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_INFO("HandleOnExtensionStateChanged start");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        napi_value argv[] = {CreateJsAbilityStateData(env_, abilityStateData)};
        CallJsFunction(obj, "onAbilityStateChanged", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnProcessCreated(const ProcessData &processData)
{
    HILOG_INFO("OnProcessCreated start");
    wptr<JSAppStateObserver> jsObserver = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsObserver, processData](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_WARN("jsObserverSptr nullptr");
                return;
            }
            jsObserverSptr->HandleOnProcessCreated(processData);
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSAppStateObserver::OnProcessCreated",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAppStateObserver::HandleOnProcessCreated(const ProcessData &processData)
{
    HILOG_INFO("HandleOnProcessCreated start");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        napi_value argv[] = {CreateJsProcessData(env_, processData)};
        CallJsFunction(obj, "onProcessCreated", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnProcessStateChanged(const ProcessData &processData)
{
    HILOG_INFO("OnProcessStateChanged start");
    wptr<JSAppStateObserver> jsObserver = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsObserver, processData](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_WARN("jsObserverSptr nullptr");
                return;
            }
            jsObserverSptr->HandleOnProcessStateChanged(processData);
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSAppStateObserver::OnProcessStateChanged",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAppStateObserver::HandleOnProcessStateChanged(const ProcessData &processData)
{
    HILOG_INFO("HandleOnProcessStateChanged begin");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        napi_value argv[] = {CreateJsProcessData(env_, processData)};
        CallJsFunction(obj, "onProcessStateChanged", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnProcessDied(const ProcessData &processData)
{
    HILOG_INFO("OnProcessDied begin");
    wptr<JSAppStateObserver> jsObserver = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsObserver, processData](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_WARN("jsObserverSptr null");
                return;
            }
            jsObserverSptr->HandleOnProcessDied(processData);
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSAppStateObserver::OnProcessCreated",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAppStateObserver::HandleOnProcessDied(const ProcessData &processData)
{
    HILOG_INFO("HandleOnProcessDied start");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        napi_value argv[] = {CreateJsProcessData(env_, processData)};
        CallJsFunction(obj, "onProcessDied", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::CallJsFunction(
    napi_value value, const char *methodName, napi_value* argv, size_t argc)
{
    HILOG_DEBUG("CallJsFunction start, method:%{public}s", methodName);
    if (value == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, value, methodName, &method);
    if (method == nullptr) {
        HILOG_ERROR("Wrong to get from object");
        return;
    }
    napi_value callResult = nullptr;
    napi_call_function(env_, value, method, argc, argv, &callResult);
    HILOG_DEBUG("CallJsFunction finish");
}

void JSAppStateObserver::AddJsObserverObject(const int32_t observerId, napi_value jsObserverObject)
{
    HILOG_DEBUG("AddJsObserverObject start.");
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsObserverObject, 1, &ref);
    jsObserverObjectMap_.emplace(observerId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
}

bool JSAppStateObserver::RemoveJsObserverObject(const int32_t observerId)
{
    HILOG_DEBUG("RemoveJsObserverObject start.");
    bool result = (jsObserverObjectMap_.erase(observerId) == 1);
    return result;
}

bool JSAppStateObserver::FindObserverByObserverId(const int32_t observerId)
{
    auto item = jsObserverObjectMap_.find(observerId);
    bool isExist = (item != jsObserverObjectMap_.end());
    return isExist;
}

size_t JSAppStateObserver::GetJsObserverMapSize()
{
    HILOG_DEBUG("GetJsObserverMapSize start");
    size_t length = jsObserverObjectMap_.size();
    return length;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
