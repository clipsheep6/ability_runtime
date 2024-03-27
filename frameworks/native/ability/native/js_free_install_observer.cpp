/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
JsFreeInstallObserver::JsFreeInstallObserver(napi_env env) : env_(env) {}

JsFreeInstallObserver::~JsFreeInstallObserver() = default;

void JsFreeInstallObserver::OnInstallFinished(const std::string &bundleName, const std::string &abilityName,
    const std::string &startTime, const int &resultCode)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "call");
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

void JsFreeInstallObserver::OnInstallFinished(const std::string &bundleName, const std::string &abilityName,
    const std::string &startTime, napi_value abilityResult)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "call");
    for (auto it = jsObserverObjectList_.begin(); it != jsObserverObjectList_.end();) {
        if ((it->bundleName == bundleName) && (it->abilityName == abilityName) && (it->startTime == startTime)) {
            if (it->callback == nullptr && it->deferred == nullptr) {
                it++;
                continue;
            }
            if (!it->isAbilityResult) {
                it++;
                continue;
            }
            if (it->deferred != nullptr) {
                CallPromise(it->deferred, abilityResult);
            } else {
                CallCallback(it->callback, abilityResult);
            }
            FinishAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartFreeInstall", atoi(startTime.c_str()));
            it = jsObserverObjectList_.erase(it);
            TAG_LOGD(AAFwkTag::FREE_INSTALL,
                "the size of jsObserverObjectList_:%{public}zu", jsObserverObjectList_.size());
        } else {
            it++;
        }
    }
}

void JsFreeInstallObserver::HandleOnInstallFinished(const std::string &bundleName, const std::string &abilityName,
    const std::string &startTime, const int &resultCode)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "call");
    for (auto it = jsObserverObjectList_.begin(); it != jsObserverObjectList_.end();) {
        if ((it->bundleName == bundleName) && (it->abilityName == abilityName) && (it->startTime == startTime)) {
            if (it->callback == nullptr && it->deferred == nullptr) {
                it++;
                continue;
            }
            if (it->isAbilityResult && resultCode == ERR_OK) {
                it++;
                continue;
            }
            if (it->deferred != nullptr) {
                CallPromise(it->deferred, resultCode);
            } else {
                CallCallback(it->callback, resultCode);
            }
            FinishAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartFreeInstall", atoi(startTime.c_str()));
            it = jsObserverObjectList_.erase(it);
            TAG_LOGD(
                AAFwkTag::FREE_INSTALL, "the size of jsObserverObjectList_:%{public}zu", jsObserverObjectList_.size());
        } else {
            it++;
        }
    }
}

void JsFreeInstallObserver::CallCallback(napi_ref callback, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "call");
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "callback is nullptr.");
        return;
    }
    napi_value value;
    if (resultCode == ERR_OK) {
        value = CreateJsUndefined(env_);
    } else {
        value = CreateJsError(env_, GetJsErrorCodeByNativeError(resultCode));
    }
    napi_value argv[] = { value };
    napi_value func = nullptr;
    napi_get_reference_value(env_, callback, &func);
    napi_call_function(env_, CreateJsUndefined(env_), func, ArraySize(argv), argv, nullptr);
}

void JsFreeInstallObserver::CallCallback(napi_ref callback, napi_value abilityResult)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "call");
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "callback is nullptr.");
        return;
    }
    napi_value argv[] = {
        CreateJsError(env_, 0),
        abilityResult,
    };
    napi_value func = nullptr;
    napi_get_reference_value(env_, callback, &func);
    napi_call_function(env_, CreateJsUndefined(env_), func, ArraySize(argv), argv, nullptr);
}

void JsFreeInstallObserver::CallPromise(napi_deferred deferred, int32_t resultCode)
{
    if (deferred == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "deferred is nullptr.");
        return;
    }
    if (resultCode == ERR_OK) {
        napi_value value = CreateJsUndefined(env_);
        napi_resolve_deferred(env_, deferred, value);
    } else {
        napi_value error = CreateJsError(env_, GetJsErrorCodeByNativeError(resultCode));
        napi_reject_deferred(env_, deferred, error);
    }
}

void JsFreeInstallObserver::CallPromise(napi_deferred deferred, napi_value abilityResult)
{
    if (deferred == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "deferred is nullptr.");
        return;
    }
    napi_resolve_deferred(env_, deferred, abilityResult);
}

void JsFreeInstallObserver::AddJsObserverObject(const std::string &bundleName, const std::string &abilityName,
    const std::string &startTime, napi_value jsObserverObject, napi_value* result, bool isAbilityResult)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "call");
    for (auto it = jsObserverObjectList_.begin(); it != jsObserverObjectList_.end(); ++it) {
        if (it->bundleName == bundleName && it->abilityName == abilityName &&
            it->startTime == startTime) {
            TAG_LOGW(AAFwkTag::FREE_INSTALL, "The jsObject has been added.");
            return;
        }
    }

    StartAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartFreeInstall", atoi(startTime.c_str()));
    JsFreeInstallObserverObject object;
    object.bundleName = bundleName;
    object.abilityName = abilityName;
    object.startTime = startTime;
    object.isAbilityResult = isAbilityResult;
    napi_valuetype type = napi_undefined;
    napi_typeof(env_, jsObserverObject, &type);
    if (jsObserverObject == nullptr || type != napi_function) {
        napi_deferred deferred;
        napi_create_promise(env_, &deferred, result);
        object.deferred = deferred;
        object.callback = nullptr;
    } else {
        napi_ref ref = nullptr;
        napi_get_undefined(env_, result);
        napi_create_reference(env_, jsObserverObject, 1, &ref);
        object.deferred = nullptr;
        object.callback = ref;
    }
    jsObserverObjectList_.emplace_back(object);
}
} // namespace AbilityRuntime
} // namespace OHOS