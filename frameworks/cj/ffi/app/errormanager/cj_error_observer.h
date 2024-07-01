/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_ABILITY_RUNTIME_CJ_FFI_APP_ERROR_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_CJ_FFI_APP_ERROR_OBSERVER_H

#include <map>
#include "ierror_observer.h"
#include "cj_common.h"
#include "cj_lambda.h"

namespace OHOS {
namespace AbilityRuntime {
class ErrorObserver : public AppExecFwk::IErrorObserver,
                      public std::enable_shared_from_this<ErrorObserver> {
public:
    explicit ErrorObserver();
    void OnExceptionObject(const AppExecFwk::ErrorObject &errorObj) override;
    void OnUnhandledException(const std::string errMsg) override;
    void AddObserverObject(const int32_t observerId, CErrorObserver observer);
    bool RemoveObserverObject(const int32_t observerId);
    bool IsEmpty();

private:
    void HandleOnUnhandledException(const std::string &errMsg);
    void HandleException(const AppExecFwk::ErrorObject &errorObj);
    std::map<int32_t, MapErrorObserver> observerObjectMap_;
};
}
}

#endif
