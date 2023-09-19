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

#include "js_app_manager_utils.h"

#include <cstdint>

#include "hilog_wrapper.h"
#include "iapplication_state_observer.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsAppStateData(NativeEngine &engine, const AppStateData &appStateData)
{
    HILOG_INFO("%{public}s CreateJsAppStateData start.", __func__);
    NativeValue* objValue = engine.CreateObject();
    if (objValue == nullptr) {
        HILOG_ERROR("objValue nullptr");
        return nullptr;
    }
    HILOG_INFO("%{public}s ConvertNativeValueTo start.", __func__);
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("object nullptr");
        return nullptr;
    }
    HILOG_INFO("%{public}s SetProperty start.", __func__);
    object->SetProperty("bundleName", CreateJsValue(engine, appStateData.bundleName));
    object->SetProperty("uid", CreateJsValue(engine, appStateData.uid));
    object->SetProperty("state", CreateJsValue(engine, appStateData.state));
    HILOG_INFO("%{public}s CreateJsAppStateData exit.", __func__);
    return objValue;
}

NativeValue* CreateJsAbilityStateData(NativeEngine &engine, const AbilityStateData &abilityStateData)
{
    HILOG_INFO("%{public}s CreateJsAbilityStateData called.", __func__);
    NativeValue* objValue = engine.CreateObject();
    if (objValue == nullptr) {
        HILOG_ERROR("objValue null.");
        return nullptr;
    }
    HILOG_INFO("%{public}s ConvertNativeValueTo start.", __func__);
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("object null.");
        return nullptr;
    }
    HILOG_INFO("%{public}s SetProperty start.", __func__);
    object->SetProperty("bundleName", CreateJsValue(engine, abilityStateData.bundleName));
    object->SetProperty("moduleName", CreateJsValue(engine, abilityStateData.moduleName));
    object->SetProperty("abilityName", CreateJsValue(engine, abilityStateData.abilityName));
    object->SetProperty("pid", CreateJsValue(engine, abilityStateData.pid));
    object->SetProperty("uid", CreateJsValue(engine, abilityStateData.uid));
    object->SetProperty("state", CreateJsValue(engine, abilityStateData.abilityState));
    object->SetProperty("abilityType", CreateJsValue(engine, abilityStateData.abilityType));
    HILOG_INFO("%{public}s CreateJsAbilityStateData exit.", __func__);
    return objValue;
}

NativeValue* CreateJsProcessData(NativeEngine &engine, const ProcessData &processData)
{
    HILOG_INFO("%{public}s CreateJsProcessData start.", __func__);
    NativeValue* objValue = engine.CreateObject();
    if (objValue == nullptr) {
        HILOG_ERROR("objValue null.");
        return nullptr;
    }
    HILOG_INFO("%{public}s ConvertNativeValueTo enter.", __func__);
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("object null.");
        return nullptr;
    }
    HILOG_INFO("%{public}s SetProperty enter.", __func__);
    object->SetProperty("bundleName", CreateJsValue(engine, processData.bundleName));
    object->SetProperty("pid", CreateJsValue(engine, processData.pid));
    object->SetProperty("uid", CreateJsValue(engine, processData.uid));
    object->SetProperty("state", CreateJsValue(engine, processData.state));
    object->SetProperty("isContinuousTask", CreateJsValue(engine, processData.isContinuousTask));
    object->SetProperty("isKeepAlive", CreateJsValue(engine, processData.isKeepAlive));
    HILOG_INFO("%{public}s CreateJsProcessData end.", __func__);
    return objValue;
}

NativeValue* CreateJsAppStateDataArray(NativeEngine &engine, const std::vector<AppStateData> &appStateDatas)
{
    HILOG_INFO("%{public}s CreateJsAppStateDataArray start.", __func__);
    NativeValue* arrayValue = engine.CreateArray(appStateDatas.size());
    NativeArray* array = ConvertNativeValueTo<NativeArray>(arrayValue);
    uint32_t index = 0;
    for (const auto &appStateData : appStateDatas) {
        array->SetElement(index++, CreateJsAppStateData(engine, appStateData));
    }
    HILOG_INFO("%{public}s CreateJsAppStateDataArray end.", __func__);
    return arrayValue;
}

NativeValue* CreateJsProcessRunningInfoArray(NativeEngine &engine, const std::vector<RunningProcessInfo> &infos)
{
    NativeValue* arrayValue = engine.CreateArray(infos.size());
    NativeArray* array = ConvertNativeValueTo<NativeArray>(arrayValue);
    uint32_t index = 0;
    for (const auto &runningInfo : infos) {
        array->SetElement(index++, CreateJsProcessRunningInfo(engine, runningInfo));
    }
    return arrayValue;
}

NativeValue* CreateJsProcessRunningInfo(NativeEngine &engine, const RunningProcessInfo &info)
{
    HILOG_DEBUG("enter");
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    object->SetProperty("processName", CreateJsValue(engine, info.processName_));
    object->SetProperty("pid", CreateJsValue(engine, info.pid_));
    object->SetProperty("uid", CreateJsValue(engine, info.uid_));
    object->SetProperty("bundleNames", CreateNativeArray(engine, info.bundleNames));
    object->SetProperty("state", CreateJsValue(engine, info.state_));
    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
