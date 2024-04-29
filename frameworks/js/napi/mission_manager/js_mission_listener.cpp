/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "js_mission_listener.h"


#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

#ifdef SUPPORT_GRAPHICS
#include "pixel_map_napi.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
void JsMissionListener::OnMissionCreated(int32_t missionId)
{
    CallJsMethod("onMissionCreated", missionId);
}

void JsMissionListener::OnMissionDestroyed(int32_t missionId)
{
    CallJsMethod("onMissionDestroyed", missionId);
}

void JsMissionListener::OnMissionSnapshotChanged(int32_t missionId)
{
    CallJsMethod("onMissionSnapshotChanged", missionId);
}

void JsMissionListener::OnMissionMovedToFront(int32_t missionId)
{
    CallJsMethod("onMissionMovedToFront", missionId);
}

void JsMissionListener::OnMissionClosed(int32_t missionId)
{
    CallJsMethod("onMissionClosed", missionId);
}

void JsMissionListener::OnMissionLabelUpdated(int32_t missionId)
{
    CallJsMethod("onMissionLabelUpdated", missionId);
}

void JsMissionListener::AddJsListenerObject(int32_t listenerId, napi_value jsListenerObject)
{
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsListenerObject, 1, &ref);
    jsListenerObjectMap_.emplace(
        listenerId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
}

bool JsMissionListener::RemoveJsListenerObject(int32_t listenerId)
{
    if (jsListenerObjectMap_.find(listenerId) != jsListenerObjectMap_.end()) {
        jsListenerObjectMap_.erase(listenerId);
        return true;
    }
    return false;
}

bool JsMissionListener::IsEmpty()
{
    return jsListenerObjectMap_.empty();
}

void JsMissionListener::CallJsMethod(const std::string &methodName, int32_t missionId)
{
    TAG_LOGI(AAFwkTag::MISSION, "methodName = %{public}s", methodName.c_str());
    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "env_ null");
        return;
    }

    // js callback should run in js thread
    wptr<JsMissionListener> jsMissionListener = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsMissionListener, methodName, missionId](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JsMissionListener> jsMissionListenerSptr = jsMissionListener.promote();
            if (jsMissionListener != nullptr) {
                jsMissionListener->CallJsMethodInner(methodName, missionId);
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsMissionListener::CallJsMethod:" + methodName,
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsMissionListener::CallJsMethodInner(const std::string &methodName, int32_t missionId)
{
    auto tmpMap = jsListenerObjectMap_;
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        if (obj == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "Failed to get object");
            continue;
        }
        napi_value method = nullptr;
        napi_get_named_property(env_, obj, methodName.c_str(), &method);
        if (method == nullptr || AppExecFwk::IsTypeForNapiValue(env_, method, napi_undefined)
            || AppExecFwk::IsTypeForNapiValue(env_, method, napi_null)) {
            TAG_LOGE(AAFwkTag::MISSION, "Failed to get %{public}s from object", methodName.c_str());
            continue;
        }
        napi_value argv[] = { CreateJsValue(env_, missionId) };
        napi_value callResult = nullptr;
        napi_call_function(env_, obj, method, ArraySize(argv), argv, &callResult);
    }
}

#ifdef SUPPORT_GRAPHICS
void JsMissionListener::OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<Media::PixelMap> &icon)
{
    TAG_LOGI(AAFwkTag::MISSION, "OnMissionIconUpdated, missionId = %{public}d.", missionId);
    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "env_ is null");
        return;
    }

    if (missionId <= 0 || !icon) {
        TAG_LOGE(AAFwkTag::MISSION, "missionId or icon is invalid, missionId:%{public}d.", missionId);
        return;
    }

    // js callback must run in js thread
    wptr<JsMissionListener> jsMissionListener = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsMissionListener, missionId, icon](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JsMissionListener> jsMissionListenerSptr = jsMissionListener.promote();
            if (jsMissionListener != nullptr) {
                jsMissionListener->CallJsMissionIconUpdated(missionId, icon);
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsMissionListener::OnMissionIconUpdated", env_,
        std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsMissionListener::CallJsMissionIconUpdated(int32_t missionId, const std::shared_ptr<Media::PixelMap> &icon)
{
    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "env_ is null, not call js mission updated.");
        return;
    }

    napi_value nativeMissionId = CreateJsValue(env_, missionId);
    auto nativeIcon = Media::PixelMapNapi::CreatePixelMap(env_, icon);

    auto tmpMap = jsListenerObjectMap_;
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        if (obj == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "Failed to get js object");
            continue;
        }
        napi_value method = nullptr;
        napi_get_named_property(env_, obj, "onMissionIconUpdated", &method);
        if (method == nullptr || AppExecFwk::IsTypeForNapiValue(env_, method, napi_undefined)
            || AppExecFwk::IsTypeForNapiValue(env_, method, napi_null)) {
            TAG_LOGE(AAFwkTag::MISSION, "Failed to get onMissionIconUpdated method from object");
            continue;
        }

        napi_value argv[] = { nativeMissionId, nativeIcon };
        napi_value callResult = nullptr;
        napi_call_function(env_, obj, method, ArraySize(argv), argv, &callResult);
    }
}
#endif
}  // namespace AbilityRuntime
}  // namespace OHOS
