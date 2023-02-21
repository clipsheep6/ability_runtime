/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "js_free_install_observer.h"

#include "hilog_wrapper.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr size_t ARGC_ONE = 1;

JsFreeInstallObserver::JsFreeInstallObserver(NativeEngine& engine) : engine_(engine) {}

JsFreeInstallObserver::~JsFreeInstallObserver() = default;

void JsFreeInstallObserver::OnInstallFinished(const std::string bundleName, const std::string abilityName,
    const std::string startTime, int resultCode)
{
    HILOG_DEBUG("OnInstallFinished come.");
    wptr<JsFreeInstallObserver> jsObserver = this;
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([jsObserver, bundleName, abilityName, startTime, resultCode](NativeEngine &engine, AsyncTask &task,
            int32_t status) {
            sptr<JsFreeInstallObserver> jsObserverSptr = jsObserver.promote();
            if (jsObserverSptr) {
                jsObserverSptr->HandleOnInstallFinished(bundleName, abilityName, startTime, resultCode);
            }
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JsFreeInstallObserver::OnInstallFinished", engine_, std::make_unique<AsyncTask>(callback,
        std::move(execute), std::move(complete)));
}

void JsFreeInstallObserver::HandleOnInstallFinished(const std::string bundleName, const std::string abilityName,
    const std::string startTime, int resultCode)
{
    HILOG_DEBUG("HandleOnInstallFinished begin.");
    for (auto it = jsObserverObjectList_.begin(); it != jsObserverObjectList_.end();) {
        if ((it->bundleName == bundleName) && (it->abilityName == abilityName) && (it->startTime == startTime)) {
            NativeValue* value = (it->callback)->Get();
            NativeValue* argv[] = { CreateJsErrorByNativeErr(engine_, resultCode) };
            CallJsFunction(value, argv, ARGC_ONE);
            jsObserverObjectList_.erase(it);
            HILOG_DEBUG("the size of jsObserverObjectList_:%{public}d", jsObserverObjectList_.size());
        } else {
            it++;
        }
    }
}

void JsFreeInstallObserver::CallJsFunction(NativeValue* value, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("CallJsFunction begin");
    if (value == nullptr) {
        HILOG_ERROR("value is nullptr.");
        return;
    }
    engine_.CallFunction(value, value, argv, argc);
}

void JsFreeInstallObserver::AddJsObserverObject(const std::string bundleName, const std::string abilityName, const std::string startTime, NativeValue* jsObserverObject)
{
    HILOG_INFO("AddJsObserverObject begin.");
    if (jsObserverObject == nullptr) {
        HILOG_ERROR("jsObserverObject is nullptr.");
        return;
    }
    
    for (auto it = jsObserverObjectList_.begin(); it != jsObserverObjectList_.end(); ++it) {
        if (it->bundleName == bundleName && it->abilityName == abilityName && it->startTime == startTime) {
            HILOG_WARN("The jsObject has been added.");
            return;
        }
    }

    JsFreeInstallObserverObject object;
    object.bundleName = bundleName;
    object.abilityName = abilityName;
    object.startTime = startTime;
    object.callback = std::shared_ptr<NativeReference>(engine_.CreateReference(jsObserverObject, 1));
    jsObserverObjectList_.emplace_back(object);
}
} // namespace AbilityRuntime
} // namespace OHOS