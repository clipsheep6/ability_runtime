/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "test_runner.h"
#include "runtime.h"

namespace OHOS {
namespace AppExecFwk {
std::unique_ptr<TestRunner> TestRunner::Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime)
{
    if (!runtime) {
        return std::make_unique<TestRunner>();
    }

    switch (runtime->GetLanguage()) {
        case AbilityRuntime::Runtime::Language::JS:
            return {};
        default:
            return std::make_unique<TestRunner>();
    }
}

void TestRunner::Prepare()
{}

void TestRunner::Run()
{}
}  // namespace AppExecFwk
}  // namespace OHOS
