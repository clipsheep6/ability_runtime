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
#include "hitrace_meter.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr size_t ARGC_ONE = 1;

JsFreeInstallObserver::JsFreeInstallObserver(napi_env env) : env_(env) {}

JsFreeInstallObserver::~JsFreeInstallObserver() = default;

void JsFreeInstallObserver::OnInstallFinished(const std::string &bundleName, const std::string &abilityName,
    const std::string &startTime, const int &resultCode)
{
    HILOG_DEBUG("OnInstallFinished come.");
    wptr<JsFreeInstallObserver> jsObserver = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsObserver, bundleName, abilityName, startTime, resultCode](napi_env env, NapiAsyncTask &task,
            int32_t status) {
            sptr<JsFreeInstallObserver> jsObserverSptr = jsObserver.promote();
            if (jsObserverSptr) {
                jsObserverSptr->HandleOnInstallFinished(bundleName, abilityName, startTime, resultCode);
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsFreeInstallObserver::OnInstallFinished", env_, std::make_unique<NapiAsyncTask>(callback,
        std::move(execute), std::move(complete)));
}

void JsFreeInstallObserver::HandleOnInstallFinished(const std::string &bundleName, const std::string &abilityName,
    const std::string &startTime, const int &resultCode)
{
    HILOG_DEBUG("HandleOnInstallFinished begin.");
    for (auto it = jsObserverObjectList_.begin(); it != jsObserverObjectList_.end();) {
        if ((it->bundleName == bundleName) && (it->abilityName == abilityName) && (it->startTime == startTime)) {
            if (it->callback == nullptr) {
                continue;
            }
            if (it->isAbilityResult && resultCode == ERR_OK) {
                it = jsObserverObjectList_.erase(it);
                continue;
            }
            FinishAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartFreeInstall", atoi(startTime.c_str()));
            napi_value value = (it->callback)->GetNapiValue();
            napi_value argv[] = { CreateJsErrorByNativeErr(env_, resultCode) };
            CallJsFunction(value, argv, ARGC_ONE);
            it = jsObserverObjectList_.erase(it);
            HILOG_DEBUG("the size of jsObserverObjectList_:%{public}zu", jsObserverObjectList_.size());
        } else {
            it++;
        }
    }
}

void JsFreeInstallObserver::CallJsFunction(napi_value value, napi_value const* argv, size_t argc)
{
    HILOG_INFO("CallJsFunction begin");
    if (value == nullptr) {
        HILOG_ERROR("value is nullptr.");
        return;
    }
    napi_call_function(env_, value, value, argc, argv, nullptr);
}

void JsFreeInstallObserver::AddJsObserverObject(const std::string &bundleName, const std::string &abilityName,
    const std::string &startTime, napi_value jsObserverObject, bool isAbilityResult)
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

    StartAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartFreeInstall", atoi(startTime.c_str()));
    JsFreeInstallObserverObject object;
    object.bundleName = bundleName;
    object.abilityName = abilityName;
    object.startTime = startTime;
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsObserverObject, 1, &ref);
    object.callback = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
    object.isAbilityResult = isAbilityResult;
    jsObserverObjectList_.emplace_back(object);
}
} // namespace AbilityRuntime
} // namespace OHOS