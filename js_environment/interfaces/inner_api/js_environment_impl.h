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

#ifndef OHOS_ABILITY_JS_ENVIRONMENT_JS_ENVIRONMENT_IMPL_H
#define OHOS_ABILITY_JS_ENVIRONMENT_JS_ENVIRONMENT_IMPL_H

#include <string>
#include "native_engine/impl/ark/ark_native_engine.h"

#include "native_engine/native_engine.h"

namespace OHOS {
namespace JsEnv {
class JsEnvironmentImpl {
public:
    JsEnvironmentImpl() {}
    virtual ~JsEnvironmentImpl() {}

    virtual void PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime) = 0;

    virtual void RemoveTask(const std::string& name) = 0;

    virtual void InitTimerModule(NativeEngine* engine) = 0;

    virtual void InitConsoleModule(NativeEngine *engine) = 0;

    virtual void InitWorkerModule() = 0;

    virtual void InitSyscapModule() = 0;
};
} // namespace JsEnv
} // namespace OHOS
#endif // OHOS_ABILITY_JS_ENVIRONMENT_JS_ENVIRONMENT_IMPL_H