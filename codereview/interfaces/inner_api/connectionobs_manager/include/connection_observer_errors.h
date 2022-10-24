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

#ifndef OHOS_ABILITYRUNTIME_CONNECTION_OBSERVER_ERRORS_H
#define OHOS_ABILITYRUNTIME_CONNECTION_OBSERVER_ERRORS_H

#include "errors.h"

namespace OHOS {
namespace AbilityRuntime {
enum {
    /**
     *  Module type: Ability  connection kit side
     */
    ABILITY_MODULE_TYPE_CONNECTION_KIT = 2,
};

// offset of aafwk error, only be used in this file.
constexpr ErrCode KIT_OFFSET = ErrCodeOffset(SUBSYS_AAFWK, ABILITY_MODULE_TYPE_CONNECTION_KIT);

enum {
    /**
     * Result(2228224) for no connection client implement.
     */
    ERR_NO_CLIENT_IMPL = KIT_OFFSET,

    /**
     * Result(2228225) for invalid observer.
     */
    ERR_INVALID_OBSERVER,

    /**
     * Result(2228226) for observer was already registered.
     */
    ERR_OBSERVER_ALREADY_REGISTERED,

    /**
     * Result(2228227) for observer that not registered.
     */
    ERR_OBSERVER_NOT_REGISTERED,

    /**
     * Result(2228228) for no proxy.
     */
    ERR_NO_PROXY,

    /**
     * Result(2228229) for no proxy.
     */
    ERR_REGISTER_FAILED,

    /**
     * Result(2228230) for service not init.
     */
    ERR_SERVICE_NOT_INIT,

    /**
     * Result(2228231) for service invalid info.
     */
    ERR_READ_INFO_FAILED,
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITYRUNTIME_CONNECTION_OBSERVER_ERRORS_H