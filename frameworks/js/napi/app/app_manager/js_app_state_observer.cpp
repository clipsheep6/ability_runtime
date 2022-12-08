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

JSAppStateObserver::JSAppStateObserver(NativeEngine& engine) : engine_(engine) {}

JSAppStateObserver::~JSAppStateObserver() = default;

void JSAppStateObserver::OnForegroundApplicationChanged(const AppStateData &appStateData)
{
    HILOG_DEBUG("onForegroundApplicationChanged bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    wptr<JSAppStateObserver> jsObserver = this;
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([jsObserver, appStateData](NativeEngine &engine, AsyncTask &task, int32_t status) {
            sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_WARN("jsObserverSptr nullptr");
                return;
            }
            jsObserverSptr->HandleOnForegroundApplicationChanged(appStateData);
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JSAppStateObserver::OnForegroundApplicationChanged",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAppStateObserver::HandleOnForegroundApplicationChanged(const AppStateData &appStateData)
{
    HILOG_DEBUG("HandleOnForegroundApplicationChanged bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        NativeValue* value = (item.second)->Get();
        NativeValue* argv[] = {CreateJsAppStateData(engine_, appStateData)};
        CallJsFunction(value, "onForegroundApplicationChanged", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_INFO("OnAbilityStateChanged begin");
    wptr<JSAppStateObserver> jsObserver = this;
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([jsObserver, abilityStateData](NativeEngine &engine, AsyncTask &task, int32_t status) {
            sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_WARN("jsObserverSptr nullptr");
                return;
            }
            jsObserverSptr->HandleOnAbilityStateChanged(abilityStateData);
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JSAppStateObserver::OnAbilityStateChanged",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAppStateObserver::HandleOnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_INFO("HandleOnAbilityStateChanged begin");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        NativeValue* value = (item.second)->Get();
        NativeValue* argv[] = {CreateJsAbilityStateData(engine_, abilityStateData)};
        CallJsFunction(value, "onAbilityStateChanged", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_INFO("OnExtensionStateChanged begin");
    wptr<JSAppStateObserver> jsObserver = this;
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([jsObserver, abilityStateData](NativeEngine &engine, AsyncTask &task, int32_t status) {
            sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_WARN("jsObserverSptr nullptr");
                return;
            }
            jsObserverSptr->HandleOnExtensionStateChanged(abilityStateData);
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JSAppStateObserver::OnExtensionStateChanged",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAppStateObserver::HandleOnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_INFO("HandleOnExtensionStateChanged begin");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        NativeValue* value = (item.second)->Get();
        NativeValue* argv[] = {CreateJsAbilityStateData(engine_, abilityStateData)};
        CallJsFunction(value, "onAbilityStateChanged", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnProcessCreated(const ProcessData &processData)
{
    HILOG_INFO("OnProcessCreated begin");
    wptr<JSAppStateObserver> jsObserver = this;
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([jsObserver, processData](NativeEngine &engine, AsyncTask &task, int32_t status) {
            sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_WARN("jsObserverSptr nullptr");
                return;
            }
            jsObserverSptr->HandleOnProcessCreated(processData);
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JSAppStateObserver::OnProcessCreated",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAppStateObserver::HandleOnProcessCreated(const ProcessData &processData)
{
    HILOG_INFO("HandleOnProcessCreated begin");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        NativeValue* value = (item.second)->Get();
        NativeValue* argv[] = {CreateJsProcessData(engine_, processData)};
        CallJsFunction(value, "onProcessCreated", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnProcessStateChanged(const ProcessData &processData)
{
    HILOG_INFO("OnProcessStateChanged begin");
    wptr<JSAppStateObserver> jsObserver = this;
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([jsObserver, processData](NativeEngine &engine, AsyncTask &task, int32_t status) {
            sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_WARN("jsObserverSptr nullptr");
                return;
            }
            jsObserverSptr->HandleOnProcessStateChanged(processData);
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JSAppStateObserver::OnProcessStateChanged",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAppStateObserver::HandleOnProcessStateChanged(const ProcessData &processData)
{
    HILOG_INFO("HandleOnProcessStateChanged begin");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        NativeValue* value = (item.second)->Get();
        NativeValue* argv[] = {CreateJsProcessData(engine_, processData)};
        CallJsFunction(value, "onProcessStateChanged", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnProcessDied(const ProcessData &processData)
{
    HILOG_INFO("OnProcessDied begin");
    wptr<JSAppStateObserver> jsObserver = this;
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([jsObserver, processData](NativeEngine &engine, AsyncTask &task, int32_t status) {
            sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
            if (!jsObserverSptr) {
                HILOG_WARN("jsObserverSptr nullptr");
                return;
            }
            jsObserverSptr->HandleOnProcessDied(processData);
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JSAppStateObserver::OnProcessCreated",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAppStateObserver::HandleOnProcessDied(const ProcessData &processData)
{
    HILOG_INFO("HandleOnProcessDied begin");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        NativeValue* value = (item.second)->Get();
        NativeValue* argv[] = {CreateJsProcessData(engine_, processData)};
        CallJsFunction(value, "onProcessDied", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::CallJsFunction(
    NativeValue* value, const char *methodName, NativeValue *const *argv, size_t argc)
{
    HILOG_INFO("CallJsFunction begin, method:%{public}s", methodName);
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue* method = obj->GetProperty(methodName);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get from object");
        return;
    }
    engine_.CallFunction(value, method, argv, argc);
    HILOG_INFO("CallJsFunction end");
}

void JSAppStateObserver::AddJsObserverObject(const int32_t observerId, NativeValue* jsObserverObject)
{
    jsObserverObjectMap_.emplace(
        observerId, std::shared_ptr<NativeReference>(engine_.CreateReference(jsObserverObject, 1)));
}

bool JSAppStateObserver::RemoveJsObserverObject(const int32_t observerId)
{
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
    size_t length = jsObserverObjectMap_.size();
    return length;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
