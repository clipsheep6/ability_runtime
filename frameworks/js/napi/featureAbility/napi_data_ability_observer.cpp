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
#include "napi_data_ability_observer.h"

#include <uv.h>

#include "hilog_tag_wrapper.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
void NAPIDataAbilityObserver::ReleaseJSCallback()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (ref_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ref_");
        return;
    }

    if (isCallingback_) {
        needRelease_ = true;
        TAG_LOGW(AAFwkTag::FA, "calling back");
        return;
    }

    SafeReleaseJSCallback();
    TAG_LOGI(AAFwkTag::FA, "end");
}

void NAPIDataAbilityObserver::SafeReleaseJSCallback()
{
    uv_loop_s* loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null loop");
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
    work->data = static_cast<void*>(delRefCallbackInfo);
    int ret = uv_queue_work_with_qos(
        loop, work, [](uv_work_t* work) {},
        [](uv_work_t* work, int status) {
            // JS Thread
            if (work == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "null work");
                return;
            }
            auto delRefCallbackInfo =  reinterpret_cast<DelRefCallbackInfo*>(work->data);
            if (delRefCallbackInfo == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "null delRefCallbackInfo");
                delete work;
                work = nullptr;
                return;
            }

            napi_delete_reference(delRefCallbackInfo->env_, delRefCallbackInfo->ref_);
            delete delRefCallbackInfo;
            delRefCallbackInfo = nullptr;
            delete work;
            work = nullptr;
        }, uv_qos_user_initiated);
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
    TAG_LOGI(AAFwkTag::FA, "end");
}

void NAPIDataAbilityObserver::SetCallbackRef(const napi_ref &ref)
{
    ref_ = ref;
    TAG_LOGI(AAFwkTag::FA, "end");
}

static void OnChangeJSThreadWorker(uv_work_t *work, int status)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null work");
        return;
    }
    DAHelperOnOffCB *onCB = (DAHelperOnOffCB *)work->data;
    if (onCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null onCB");
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
    TAG_LOGI(AAFwkTag::FA, "end");
}

void NAPIDataAbilityObserver::CallJsMethod()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (ref_ == nullptr || env_ == nullptr) {
            TAG_LOGW(AAFwkTag::FA, "invalid observer");
            return;
        }
        isCallingback_ = true;
    }
    napi_value result[ARGS_TWO] = {nullptr};
    result[PARAM0] = GetCallbackErrorValue(env_, NO_ERROR);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(env_, &undefined);
    napi_value callResult = nullptr;
    napi_get_reference_value(env_, ref_, &callback);
    napi_call_function(env_, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (needRelease_ && ref_ != nullptr) {
            TAG_LOGI(AAFwkTag::FA, "delete callback");
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
        TAG_LOGE(AAFwkTag::FA, "null ret");
        return;
    }
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null loop");
        return;
    }

    uv_work_t *work = new uv_work_t;
    DAHelperOnOffCB *onCB = new DAHelperOnOffCB;
    onCB->observer = this;
    work->data = static_cast<void *>(onCB);
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
    TAG_LOGI(AAFwkTag::FA, "end");
}

}  // namespace AppExecFwk
}  // namespace OHOS