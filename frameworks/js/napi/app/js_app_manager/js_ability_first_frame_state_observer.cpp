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

#ifdef SUPPORT_GRAPHICS
#include "js_ability_first_frame_state_observer.h"

#include "hilog_tag_wrapper.h"
#include "js_app_manager_utils.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
}
JSAbilityFirstFrameStateObserver::JSAbilityFirstFrameStateObserver(napi_env env) : env_(env) {}

void JSAbilityFirstFrameStateObserver::OnAbilityFirstFrameState(
    const AbilityFirstFrameStateData &abilityFirstFrameStateData)
{
    wptr<JSAbilityFirstFrameStateObserver> self = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [self, abilityFirstFrameStateData](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSAbilityFirstFrameStateObserver> jsObserver = self.promote();
            if (jsObserver == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Js Observer Sptr is nullptr.");
                return;
            }
            jsObserver->HandleOnAbilityFirstFrameState(abilityFirstFrameStateData);
        });
    napi_ref callback = nullptr;
    NapiAsyncTask::Schedule("JSAbilityFirstFrameStateObserver::OnAbilityFirstFrameState", env_,
        std::make_unique<NapiAsyncTask>(callback, nullptr, std::move(complete)));
}

void JSAbilityFirstFrameStateObserver::HandleOnAbilityFirstFrameState(
    const AbilityFirstFrameStateData &AbilityFirstFrameStateData)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    napi_value obj = jsObserverObject_->GetNapiValue();
    napi_value argv[] = { CreateJsAbilityFirstFrameStateData(env_, AbilityFirstFrameStateData) };
    CallJsFunction(obj, "onAbilityFirstFrameDrawn", argv, ARGC_ONE);
}

void JSAbilityFirstFrameStateObserver::CallJsFunction(
    const napi_value value, const char *methodName, const napi_value *argv, const size_t argc)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "value is nullptr.");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, value, methodName, &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get name from object Failed.");
        return;
    }
    napi_value callResult = nullptr;
    napi_status status = napi_call_function(env_, value, method, argc, argv, &callResult);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Call Js Function failed %{public}d.", status);
    }
}

void JSAbilityFirstFrameStateObserver::SetJsObserverObject(const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Observer is null.");
        return;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsObserverObject, 1, &ref);
    jsObserverObject_ = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref));
}

void JSAbilityFirstFrameStateObserver::ResetJsObserverObject()
{
    if (jsObserverObject_) {
        jsObserverObject_ = nullptr;
    }
}

std::shared_ptr<NativeReference> JSAbilityFirstFrameStateObserver::GetNativeReference()
{
    return jsObserverObject_;
}

void JSAbilityFirstFrameStateObserverManager::Init(napi_env env)
{
    env_ = env;
}

void JSAbilityFirstFrameStateObserverManager::AddJSAbilityFirstFrameStateObserver(
    const sptr<JSAbilityFirstFrameStateObserver> observer)
{
    if (observer == nullptr || observer->GetNativeReference() == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Observer is null.");
        return;
    }
    std::lock_guard<std::mutex> lock(observerListLock_);
    jsAbilityFirstFrameStateObserverList_.emplace_back(observer);
}

bool JSAbilityFirstFrameStateObserverManager::IsObserverObjectExist(const napi_value &jsObserverObject)
{
    if (GetObserverObject(jsObserverObject) == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Observer is null.");
        return false;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Observer is exists.");
    return true;
}

void JSAbilityFirstFrameStateObserverManager::RemoveAllJsObserverObjects(
    sptr<OHOS::AAFwk::IAbilityManager> &abilityManager)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityManager is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> lock(observerListLock_);
    for (auto observer : jsAbilityFirstFrameStateObserverList_) {
        abilityManager->UnregisterAbilityFirstFrameStateObserver(observer);
        observer->ResetJsObserverObject();
    }
    jsAbilityFirstFrameStateObserverList_.clear();
}

void JSAbilityFirstFrameStateObserverManager::RemoveJsObserverObject(
    sptr<OHOS::AAFwk::IAbilityManager> &abilityManager, const napi_value &jsObserverObject)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityManager is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> lock(observerListLock_);
    auto it = jsAbilityFirstFrameStateObserverList_.begin();
    for (; it != jsAbilityFirstFrameStateObserverList_.end(); ++it) {
        std::shared_ptr<NativeReference> tmpObject = (*it)->GetNativeReference();
        napi_value value = tmpObject->GetNapiValue();
        if (value == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get object.");
            continue;
        }
        bool isEqual = false;
        napi_strict_equals(env_, value, jsObserverObject, &isEqual);
        if (isEqual) {
            abilityManager->UnregisterAbilityFirstFrameStateObserver(*it);
            (*it)->ResetJsObserverObject();
            jsAbilityFirstFrameStateObserverList_.erase(it);
            return;
        }
    }
}

std::shared_ptr<NativeReference> JSAbilityFirstFrameStateObserverManager::GetObserverObject(
    const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Observer is null.");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(observerListLock_);
    for (auto &observer : jsAbilityFirstFrameStateObserverList_) {
        if (observer == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid observer.");
            continue;
        }
        std::shared_ptr<NativeReference> tmpObject = observer->GetNativeReference();
        napi_value value = tmpObject->GetNapiValue();
        if (value == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get object.");
            continue;
        }

        bool isEqual = false;
        napi_strict_equals(env_, value, jsObserverObject, &isEqual);
        if (isEqual) {
            return observer->GetNativeReference();
        }
    }
    return nullptr;
}
} // namespace AbilityRuntime
} // namespace OHOS
#endif // SUPPORT_GRAPHICS