/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_ERROR_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_JS_ERROR_OBSERVER_H

#include <map>

#include "ierror_observer.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
class JsErrorObserver : public AppExecFwk::IErrorObserver,
                        public std::enable_shared_from_this<JsErrorObserver> {
public:
    explicit JsErrorObserver(NativeEngine &engine);
    virtual ~JsErrorObserver();
    void OnExceptionObject(const AppExecFwk::ErrorObject &errorObj) override;
    void OnUnhandledException(const std::string errMsg) override;
    void AddJsObserverObject(const int32_t observerId, NativeValue* jsObserverObject);
    bool RemoveJsObserverObject(const int32_t observerId, bool &isEmpty);

private:
    void CallJsFunction(NativeValue* value, const char* methodName, NativeValue* const* argv, size_t argc);
    void HandleOnUnhandledException(const std::string &errMsg);
    void HandleException(const AppExecFwk::ErrorObject &errorObj);
    NativeValue* CreateJsErrorObject(NativeEngine &engine, const AppExecFwk::ErrorObject &errorObj);

private:
    NativeEngine &engine_;
    std::map<int32_t, std::shared_ptr<NativeReference>> jsObserverObjectMap_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_ERROR_OBSERVER_H
