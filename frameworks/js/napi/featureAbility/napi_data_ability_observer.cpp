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
        TAG_LOGE(AAFwkTag::FA, "NAPIDataAbilityObserver::ReleaseJSCallback, ref_ is null.");
        return;
    }

    if (isCallingback_) {
        needRelease_ = true;
        TAG_LOGW(AAFwkTag::FA, "%{public}s, ref_ is calling back.", __func__);
        return;
    }

    SafeReleaseJSCallback();
    TAG_LOGI(AAFwkTag::FA, "NAPIDataAbilityObserver::%{public}s, called. end", __func__);
}

void NAPIDataAbilityObserver::SafeReleaseJSCallback()
{
    uv_loop_s* loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, loop == nullptr.", __func__);
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
                TAG_LOGE(AAFwkTag::FA, "uv_queue_work input work is nullptr");
                return;
            }
            auto delRefCallbackInfo =  reinterpret_cast<DelRefCallbackInfo*>(work->data);
            if (delRefCallbackInfo == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "uv_queue_work delRefCallbackInfo is nullptr");
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
    TAG_LOGI(AAFwkTag::FA, "NAPIDataAbilityObserver::%{public}s, called. end", __func__);
}

void NAPIDataAbilityObserver::SetCallbackRef(const napi_ref &ref)
{
    ref_ = ref;
    TAG_LOGI(AAFwkTag::FA, "NAPIDataAbilityObserver::%{public}s, called. end", __func__);
}
}
}