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

#include "application_state_observer_stub.h"
#include "native_engine/native_engine.h"
#include "running_process_info.h"

#ifndef OHOS_ABILITY_RUNTIME_JS_APP_MANAGER_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_APP_MANAGER_UTILS_H

namespace OHOS {
namespace AbilityRuntime {
using OHOS::AppExecFwk::AppStateData;
using OHOS::AppExecFwk::AbilityStateData;
using OHOS::AppExecFwk::ProcessData;
using OHOS::AppExecFwk::RunningProcessInfo;
napi_value CreateJsAppStateData(napi_env env, const AppStateData &appStateData);
napi_value CreateJsAbilityStateData(napi_env env, const AbilityStateData &abilityStateData);
napi_value CreateJsProcessData(napi_env env, const ProcessData &processData);
napi_value CreateJsAppStateDataArray(napi_env env, const std::vector<AppStateData> &appStateDatas);
napi_value CreateJsProcessRunningInfoArray(napi_env env, const std::vector<RunningProcessInfo> &infos);
napi_value CreateJsProcessRunningInfo(napi_env env, const RunningProcessInfo &info);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_APP_MANAGER_UTILS_H
