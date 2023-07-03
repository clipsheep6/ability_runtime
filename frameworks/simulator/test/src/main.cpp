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
#include <iostream>

#include "options.h"
#include "simulator.h"

constexpr int32_t MIN_PARAMS = 26;

int32_t main(int32_t argc, const char *argv[])
{
    if (argc < MIN_PARAMS) {
        std::cout << "Insufficient parameters." << std::endl;
        return 1;
    }

    OHOS::AbilityRuntime::Options options;
    options.bundleName = argv[1];
    options.moduleName = argv[2];
    options.modulePath = argv[3];
    options.resourcePath = argv[4];
    options.debugPort = atoi(argv[5]);
    options.assetPath = argv[6];
    options.systemResourcePath = argv[7];
    options.appResourcePath = argv[8];
    options.containerSdkPath = argv[9];
    options.url = argv[10];
    options.language = argv[11];
    options.region = argv[12];
    options.script = argv[13];
    options.themeId = atoi(argv[14]);
    options.deviceWidth = atoi(argv[15]);
    options.deviceHeight = atoi(argv[16]);
    options.isRound = atoi(argv[17]);
    options.compatibleVersion = atoi(argv[18]);
    options.installationFree = atoi(argv[19]);
    options.labelId = atoi(argv[20]);
    options.compileMode = argv[21];
    options.pageProfile = argv[22];
    options.targetVersion = atoi(argv[23]);
    options.releaseType = argv[24];
    options.enablePartialUpdate = atoi(argv[25]);
    auto simulator = OHOS::AbilityRuntime::Simulator::Create(options);
    if (!simulator) {
        std::cout << "Create Simulator failed." << std::endl;
        return 1;
    }

    std::string abilitySrcPath {argv[26]};
    int64_t id = simulator->StartAbility(abilitySrcPath, [](int64_t abilityId) {});
    if (id < 0) {
        std::cout << "Start Ability failed." << std::endl;
        return 1;
    }

    simulator->TerminateAbility(id);
    return 0;
}
