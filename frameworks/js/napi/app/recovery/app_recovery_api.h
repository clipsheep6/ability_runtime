/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_RECOVERY_API_H
#define OHOS_ABILITY_RUNTIME_APP_RECOVERY_API_H

#include <map>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "native_engine/native_engine.h"


namespace OHOS {
namespace AbilityRuntime {
NativeValue* InitAppRecoveryApiModule(NativeEngine *engine, NativeValue *exportObj);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APP_RECOVERY_API_H