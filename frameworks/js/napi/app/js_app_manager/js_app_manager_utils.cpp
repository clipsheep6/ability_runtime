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

#include "js_app_manager_utils.h"

#include <cstdint>

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "iapplication_state_observer.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value CreateJsAppStateData(napi_env env, const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called.");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "objValue nullptr.");
        return nullptr;
    }
    napi_set_named_property(env, object, "bundleName", CreateJsValue(env, appStateData.bundleName));
    napi_set_named_property(env, object, "uid", CreateJsValue(env, appStateData.uid));
    napi_set_named_property(env, object, "state", CreateJsValue(env, appStateData.state));
    napi_set_named_property(env, object, "isSplitScreenMode", CreateJsValue(env, appStateData.isSplitScreenMode));
    napi_set_named_property(env, object, "isFloatingWindowMode", CreateJsValue(env, appStateData.isFloatingWindowMode));
    TAG_LOGD(AAFwkTag::APPMGR, "end.");
    return object;
}

napi_value CreateJsAbilityStateData(napi_env env, const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called.");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "objValue nullptr.");
        return nullptr;
    }
    napi_set_named_property(env, object, "bundleName", CreateJsValue(env, abilityStateData.bundleName));
    napi_set_named_property(env, object, "moduleName", CreateJsValue(env, abilityStateData.moduleName));
    napi_set_named_property(env, object, "abilityName", CreateJsValue(env, abilityStateData.abilityName));
    napi_set_named_property(env, object, "pid", CreateJsValue(env, abilityStateData.pid));
    napi_set_named_property(env, object, "uid", CreateJsValue(env, abilityStateData.uid));
    napi_set_named_property(env, object, "state", CreateJsValue(env, abilityStateData.abilityState));
    napi_set_named_property(env, object, "abilityType", CreateJsValue(env, abilityStateData.abilityType));
    napi_set_named_property(env, object, "isAtomicService", CreateJsValue(env, abilityStateData.isAtomicService));
    HILOG_DEBUG("end.");
    return object;
}

#ifdef SUPPORT_GRAPHICS
napi_value CreateJsAbilityFirstFrameStateData(napi_env env,
    const AbilityFirstFrameStateData &abilityFirstFrameStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "ObjValue nullptr.");
        return nullptr;
    }
    napi_set_named_property(env, object, "bundleName", CreateJsValue(env, abilityFirstFrameStateData.bundleName));
    napi_set_named_property(env, object, "moduleName", CreateJsValue(env, abilityFirstFrameStateData.moduleName));
    napi_set_named_property(env, object, "abilityName", CreateJsValue(env, abilityFirstFrameStateData.abilityName));
    napi_set_named_property(env, object, "appIndex", CreateJsValue(env, abilityFirstFrameStateData.appIndex));
    napi_set_named_property(env, object, "isColdStart", CreateJsValue(env, abilityFirstFrameStateData.coldStart));
    return object;
}
#endif

napi_value CreateJsProcessData(napi_env env, const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called.");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "objValue nullptr.");
        return nullptr;
    }
    napi_set_named_property(env, object, "bundleName", CreateJsValue(env, processData.bundleName));
    napi_set_named_property(env, object, "pid", CreateJsValue(env, processData.pid));
    napi_set_named_property(env, object, "uid", CreateJsValue(env, processData.uid));
    napi_set_named_property(env, object, "state", CreateJsValue(env, processData.state));
    napi_set_named_property(env, object, "isContinuousTask", CreateJsValue(env, processData.isContinuousTask));
    napi_set_named_property(env, object, "isKeepAlive", CreateJsValue(env, processData.isKeepAlive));
    TAG_LOGD(AAFwkTag::APPMGR, "end.");
    return object;
}

napi_value CreateJsAppStateDataArray(napi_env env, const std::vector<AppStateData> &appStateDatas)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, appStateDatas.size(), &arrayValue);
    uint32_t index = 0;
    for (const auto &appStateData : appStateDatas) {
        napi_set_element(env, arrayValue, index++, CreateJsAppStateData(env, appStateData));
    }
    return arrayValue;
}

napi_value CreateJsRunningProcessInfoArray(napi_env env, const std::vector<RunningProcessInfo> &infos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, infos.size(), &arrayValue);
    uint32_t index = 0;
    for (const auto &runningInfo : infos) {
        napi_set_element(env, arrayValue, index++, CreateJsRunningProcessInfo(env, runningInfo));
    }
    return arrayValue;
}

napi_value CreateJsRunningProcessInfo(napi_env env, const RunningProcessInfo &info)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "objValue nullptr.");
        return nullptr;
    }

    napi_set_named_property(env, object, "processName", CreateJsValue(env, info.processName_));
    napi_set_named_property(env, object, "pid", CreateJsValue(env, info.pid_));
    napi_set_named_property(env, object, "uid", CreateJsValue(env, info.uid_));
    napi_set_named_property(env, object, "bundleNames", CreateNativeArray(env, info.bundleNames));
    napi_set_named_property(env, object, "state", CreateJsValue(env,
        ConvertToJsAppProcessState(info.state_, info.isFocused)));
    return object;
}

napi_value ApplicationStateInit(napi_env env)
{
    TAG_LOGD(AAFwkTag::APPMGR, "ApplicationStateInit enter");

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid input parameters");
        return nullptr;
    }

    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Wrong to get object");
        return nullptr;
    }

    napi_set_named_property(env, object, "STATE_CREATE",
        CreateJsValue(env, static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_CREATE)));
    napi_set_named_property(env, object, "STATE_FOREGROUND",
        CreateJsValue(env, static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND)));
    napi_set_named_property(env, object, "STATE_ACTIVE",
        CreateJsValue(env, static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOCUS)));
    napi_set_named_property(env, object, "STATE_BACKGROUND",
        CreateJsValue(env, static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_BACKGROUND)));
    napi_set_named_property(env, object, "STATE_DESTROY",
        CreateJsValue(env, static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_TERMINATED)));

    return object;
}

napi_value ProcessStateInit(napi_env env)
{
    TAG_LOGD(AAFwkTag::APPMGR, "ProcessStateInit enter");

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid input arguments");
        return nullptr;
    }

    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to get object");
        return nullptr;
    }
    napi_set_named_property(env, object, "STATE_CREATE",
        CreateJsValue(env, static_cast<int32_t>(AppExecFwk::AppProcessState::APP_STATE_CREATE)));
    napi_set_named_property(env, object, "STATE_FOREGROUND",
        CreateJsValue(env, static_cast<int32_t>(AppExecFwk::AppProcessState::APP_STATE_FOREGROUND)));
    napi_set_named_property(env, object, "STATE_ACTIVE",
        CreateJsValue(env, static_cast<int32_t>(AppExecFwk::AppProcessState::APP_STATE_FOCUS)));
    napi_set_named_property(env, object, "STATE_BACKGROUND",
        CreateJsValue(env, static_cast<int32_t>(AppExecFwk::AppProcessState::APP_STATE_BACKGROUND)));
    napi_set_named_property(env, object, "STATE_DESTROY",
        CreateJsValue(env, static_cast<int32_t>(AppExecFwk::AppProcessState::APP_STATE_TERMINATED)));
    return object;
}

JsAppProcessState ConvertToJsAppProcessState(
    const AppExecFwk::AppProcessState &appProcessState, const bool &isFocused)
{
    JsAppProcessState processState;
    switch (appProcessState) {
        case AppExecFwk::AppProcessState::APP_STATE_CREATE:
        case AppExecFwk::AppProcessState::APP_STATE_READY:
            processState = STATE_CREATE;
            break;
        case AppExecFwk::AppProcessState::APP_STATE_FOREGROUND:
            processState = isFocused ? STATE_ACTIVE : STATE_FOREGROUND;
            break;
        case AppExecFwk::AppProcessState::APP_STATE_BACKGROUND:
            processState = STATE_BACKGROUND;
            break;
        case AppExecFwk::AppProcessState::APP_STATE_TERMINATED:
        case AppExecFwk::AppProcessState::APP_STATE_END:
            processState = STATE_DESTROY;
            break;
        default:
            HILOG_ERROR("Process state is invalid.");
            processState = STATE_DESTROY;
            break;
    }
    return processState;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
