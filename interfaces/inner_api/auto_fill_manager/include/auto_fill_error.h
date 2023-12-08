/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_AUTO_FILL_ERROR_H
#define OHOS_ABILITY_RUNTIME_AUTO_FILL_ERROR_H

namespace OHOS {
namespace AbilityRuntime {
namespace AutoFill {
enum {
    AUTO_FILL_SUCCESS = 0,
    AUTO_FILL_FAILED,
    AUTO_FILL_CANCEL,
    AUTO_FILL_CANCEL_TIME_OUT,
    AUTO_FILL_RELEASE_FAILED,
    AUTO_FILL_ON_ERROR,
    AUTO_FILL_OBJECT_IS_NULL,
    AUTO_FILL_CREATE_MODULE_UI_EXTENSION_FAILED,
    AUTO_FILL_REQUEST_TIME_OUT,
};
} // namespace AutoFill
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTO_FILL_ERROR_H