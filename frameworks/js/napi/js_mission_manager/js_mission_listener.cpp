/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

#ifdef SUPPORT_GRAPHICS
#include "pixel_map_napi.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
constexpr size_t ARGC_ONE = 1;

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

void JsMissionListener::AddJsListenerObject(int32_t listenerId, NativeValue* jsListenerObject, bool isSync)
{
    if (isSync) {
        jsListenerObjectMapSync_.emplace(
            listenerId, std::shared_ptr<NativeReference>(engine_->CreateReference(jsListenerObject, 1)));
    } else {
        jsListenerObjectMap_.emplace(
            listenerId, std::shared_ptr<NativeReference>(engine_->CreateReference(jsListenerObject, 1)));
    }
}

bool JsMissionListener::RemoveJsListenerObject(int32_t listenerId, bool isSync)
{
    bool result = false;
    if (isSync) {
        result = (jsListenerObjectMapSync_.erase(listenerId) == 1);
    } else {
        result = (jsListenerObjectMap_.erase(listenerId) == 1);
    }
    return result;
}

bool JsMissionListener::IsEmpty()
{
    return jsListenerObjectMap_.empty() && jsListenerObjectMapSync_.empty();
}

void JsMissionListener::CallJsMethod(const std::string &methodName, int32_t missionId)
{
    HILOG_DEBUG("methodName = %{public}s", methodName.c_str());
    if (engine_ == nullptr) {
        HILOG_ERROR("engine_ nullptr");
        return;
    }

    // js callback should run in js thread
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([jsMissionListener = this, methodName, missionId](NativeEngine &engine, AsyncTask &task, int32_t status) {
            if (jsMissionListener) {
                jsMissionListener->CallJsMethodInner(methodName, missionId);
            }
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JsMissionListener::CallJsMethod:" + methodName,
        *engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsMissionListener::CallJsMethodInner(const std::string &methodName, int32_t missionId)
{
    // jsListenerObjectMap_ may be changed in engine_->CallFunction()
    auto tmpMap = jsListenerObjectMap_;
    for (auto &item : tmpMap) {
        NativeValue* value = (item.second)->Get();
        NativeValue* argv[] = { CreateJsValue(*engine_, missionId) };
        CallJsFunction(value, methodName.c_str(), argv, ARGC_ONE);
    }
    tmpMap = jsListenerObjectMapSync_;
    for (auto &item : tmpMap) {
        NativeValue* value = (item.second)->Get();
        NativeValue* argv[] = { CreateJsValue(*engine_, missionId) };
        CallJsFunction(value, methodName.c_str(), argv, ARGC_ONE);
    }
}

void JsMissionListener::CallJsFunction(
    NativeValue* value, const char* methodName, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("method:%{public}s", methodName);
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue* method = obj->GetProperty(methodName);
    if (method == nullptr || method->TypeOf() == NATIVE_UNDEFINED) {
        HILOG_ERROR("Failed to get method");
        return;
    }
    engine_->CallFunction(value, method, argv, argc);
}

#ifdef SUPPORT_GRAPHICS
void JsMissionListener::OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<Media::PixelMap> &icon)
{
    HILOG_DEBUG("OnMissionIconUpdated, missionId = %{public}d", missionId);
    if (engine_ == nullptr) {
        HILOG_ERROR("engine_ is nullptr");
        return;
    }

    if (missionId <= 0 || !icon) {
        HILOG_ERROR("missionId or icon is invalid, missionId:%{public}d", missionId);
        return;
    }

    // js callback must run in js thread
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([jsMissionListener = this, missionId, icon](NativeEngine &engine, AsyncTask &task, int32_t status) {
            if (jsMissionListener) {
                jsMissionListener->CallJsMissionIconUpdated(missionId, icon);
            }
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JsMissionListener::OnMissionIconUpdated", *engine_,
        std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsMissionListener::CallJsMissionIconUpdated(int32_t missionId, const std::shared_ptr<Media::PixelMap> &icon)
{
    if (engine_ == nullptr) {
        HILOG_ERROR("engine_ is nullptr, not call js mission updated.");
        return;
    }

    NativeValue* nativeMissionId = CreateJsValue(*engine_, missionId);
    auto nativeIcon = reinterpret_cast<NativeValue*>(
        Media::PixelMapNapi::CreatePixelMap(reinterpret_cast<napi_env>(engine_), icon));

    auto tmpMap = jsListenerObjectMap_;
    for (auto &item : tmpMap) {
        NativeValue* value = (item.second)->Get();
        NativeObject* listenerObj = ConvertNativeValueTo<NativeObject>(value);
        if (listenerObj == nullptr) {
            HILOG_ERROR("Failed to get js object");
            continue;
        }
        NativeValue* method = listenerObj->GetProperty("onMissionIconUpdated");
        if (method == nullptr || method->TypeOf() == NATIVE_UNDEFINED) {
            HILOG_ERROR("Failed to get onMissionIconUpdated method from object");
            continue;
        }

        NativeValue* argv[] = { nativeMissionId, nativeIcon };
        engine_->CallFunction(value, method, argv, ArraySize(argv));
    }
}
#endif
}  // namespace AbilityRuntime
}  // namespace OHOS
