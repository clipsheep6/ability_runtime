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

#ifndef OHOS_ABILITY_RUNTIME_DISTRIBUTED_MISSION_MANAGER_HELPER_H
#define OHOS_ABILITY_RUNTIME_DISTRIBUTED_MISSION_MANAGER_HELPER_H

#include <uv.h>

#include "mission_continue_interface.h"
#include "mission_continue_stub.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "js_runtime_utils.h"
#include "securec.h"
#include "want.h"
#include "remote_mission_listener_stub.h"
#include "remote_on_listener_stub.h"

namespace OHOS {
namespace AAFwk {
using namespace std;

bool CheckContinueKeyExist(napi_env &env, const napi_value &value);
bool CheckBundleNameExist(napi_env &env, const napi_value &value);
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DISTRIBUTED_MISSION_MANAGER_HELPER_H