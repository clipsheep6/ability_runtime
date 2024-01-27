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

#ifndef ABILITY_ABILITY_RUNTIME_NATIVE_ERR_CODE_H
#define ABILITY_ABILITY_RUNTIME_NATIVE_ERR_CODE_H

namespace OHOS {
namespace AbilityRuntime {
enum {
    NATIVE_RUNTIME_ERR_OK,
    NATIVE_RUNTIME_THREAD_COUNT_OVERLOAD,
    NATIVE_RUNTIME_THREAD_ONLY_ONE_RUNENV,
    NATIVE_RUNTIME_INNER_ERROR,
    NATIVE_RUNTIME_DESTORY_FAILED
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // ABILITY_ABILITY_RUNTIME_NATIVE_ERR_CODE_H

