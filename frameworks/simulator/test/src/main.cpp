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

#include <cstdint>

#include "simulator.h"

int32_t main(int32_t argc, const char* argv[])
{
    OHOS::AbilityRuntime::Simulator::Options options;
    auto simulator = OHOS::AbilityRuntime::Simulator::Create(options);
    if (!simulator) {
        return 1;
    }

    std::string abilitySrcPath;
    if (argc > 1) {
        abilitySrcPath = argv[1];
    }

    int64_t id = simulator->StartAbility(abilitySrcPath, [](int64_t abilityId) {});
    if (id < 0) {
        return 1;
    }

    simulator->TerminateAbility(id);
    return 0;
}
