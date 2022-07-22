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

#ifndef OHOS_ABILITY_RUNTIME_IERROR_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_IERROR_OBSERVER_H

#include <string>

namespace OHOS {
namespace AppExecFwk {
class IErrorObserver {
public:
    IErrorObserver() = default;
    virtual ~IErrorObserver() = default;
    /**
     * Will be called when the js runtime throws an exception which doesn't caught by user.
     *
     * @param errMsg the message and error stacktrace about the exception.
     */
    virtual void OnUnhandledException(std::string errMsg) = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_IERROR_OBSERVER_H
