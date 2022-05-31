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

#include "ierror_observer.h"
#include "native_engine/native_engine.h"

#ifndef OHOS_APPEXECFWK_RUNTIME_JS_ERROR_OBSERVER_H
#define OHOS_APPEXECFWK_RUNTIME_JS_ERROR_OBSERVER_H

namespace OHOS {
namespace AbilityRuntime {
class JsErrorObserver : public AppExecFwk::IErrorObserver {
public:
    explicit JsErrorObserver(NativeEngine& engine);
    ~JsErrorObserver();
    void OnUnhandledException(std::string errMsg) override;
    void HandleOnUnhandledException(const std::string &errMsg);
    void SetJsObserverObject(NativeValue* jsObserverObject);
    void CallJsFunction(const char* methodName, NativeValue* const* argv, size_t argc);

private:
    NativeEngine& engine_;
    std::unique_ptr<NativeReference> jsObserverObject_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_APPEXECFWK_RUNTIME_JS_ERROR_OBSERVER_H