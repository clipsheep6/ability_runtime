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

constexpr int32_t PARAM_ONE = 1;
constexpr int32_t PARAM_TWO = 2;
constexpr int32_t PARAM_THREE = 3;
constexpr int32_t PARAM_FOUR = 4;
constexpr int32_t PARAM_FIVE = 5;
constexpr int32_t PARAM_SIX = 6;
constexpr int32_t PARAM_SEVEN = 7;
constexpr int32_t PARAM_EIGHT = 8;
constexpr int32_t PARAM_NINE = 9;
constexpr int32_t PARAM_TEN = 10;
constexpr int32_t PARAM_ELEVEN = 11;
constexpr int32_t PARAM_TWELVE = 12;
constexpr int32_t PARAM_THIRTEEN = 13;
constexpr int32_t PARAM_FOURTEEN = 14;
constexpr int32_t PARAM_FIFTEEN = 15;
constexpr int32_t PARAM_SIXTEEN = 16;
constexpr int32_t PARAM_SEVENTEEN = 17;
constexpr int32_t PARAM_EIGHTEEN = 18;
constexpr int32_t PARAM_NINETEEN = 19;
constexpr int32_t PARAM_TWENTY = 20;
constexpr int32_t PARAM_TWENTYONE = 21;
constexpr int32_t PARAM_TWENTYTWO = 22;
constexpr int32_t PARAM_TWENTYTHREE = 23;
constexpr int32_t PARAM_TWENTYFOUR = 24;
constexpr int32_t PARAM_TWENTYFIVE = 25;
constexpr int32_t PARAM_TWENTYSIX = 26;
constexpr int32_t PARAM_TWENTYSEVEN = 27;
constexpr int32_t PARAM_TWENTYEIGHT = 28;

int32_t main(int32_t argc, const char *argv[])
{
    if (argc < PARAM_TWENTYEIGHT) {
        std::cout << "Insufficient parameters." << std::endl;
        return 1;
    }

    OHOS::AbilityRuntime::Options options;
    options.bundleName = argv[PARAM_ONE];
    options.moduleName = argv[PARAM_TWO];
    options.modulePath = argv[PARAM_THREE];
    options.resourcePath = argv[PARAM_FOUR];
    options.debugPort = atoi(argv[PARAM_FIVE]);
    options.assetPath = argv[PARAM_SIX];
    options.systemResourcePath = argv[PARAM_SEVEN];
    options.appResourcePath = argv[PARAM_EIGHT];
    options.containerSdkPath = argv[PARAM_NINE];
    options.url = argv[PARAM_TEN];
    options.language = argv[PARAM_ELEVEN];
    options.region = argv[PARAM_TWELVE];
    options.script = argv[PARAM_THIRTEEN];
    options.themeId = atoi(argv[PARAM_FOURTEEN]);
    options.deviceWidth = atoi(argv[PARAM_FIFTEEN]);
    options.deviceHeight = atoi(argv[PARAM_SIXTEEN]);
    options.isRound = atoi(argv[PARAM_SEVENTEEN]);
    options.compatibleVersion = atoi(argv[PARAM_EIGHTEEN]);
    options.installationFree = atoi(argv[PARAM_NINETEEN]);
    options.labelId = atoi(argv[PARAM_TWENTY]);
    options.compileMode = argv[PARAM_TWENTYONE];
    options.pageProfile = argv[PARAM_TWENTYTWO];
    options.targetVersion = atoi(argv[PARAM_TWENTYTHREE]);
    options.releaseType = argv[PARAM_TWENTYFOUR];
    options.enablePartialUpdate = atoi(argv[PARAM_TWENTYFIVE]);
    options.previewPath = argv[PARAM_TWENTYEIGHT];

    OHOS::AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.name = "entry";
    hapModuleInfo.srcEntrance = argv[PARAM_TWENTYSEVEN];
    options.hapModuleInfo = hapModuleInfo;

    OHOS::AppExecFwk::ApplicationInfo appInfo;
    appInfo.name = "com.test.simulator";
    options.applicationInfo = appInfo;

    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.name = "EntryAbility";
    options.abilityInfo = abilityInfo;

    OHOS::AppExecFwk::Configuration config;
    config.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "testlanguage");
    config.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "light");
    config.AddItem(OHOS::AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION, "vertical");
    auto configuration = std::make_shared<OHOS::AppExecFwk::Configuration>(config);
    options.configuration = configuration;

    auto simulator = OHOS::AbilityRuntime::Simulator::Create(options);
    if (!simulator) {
        std::cout << "Create Simulator failed." << std::endl;
        return 1;
    }

    std::string abilitySrcPath {argv[PARAM_TWENTYSIX]};
    int64_t id = simulator->StartAbility(abilitySrcPath, [](int64_t abilityId) {});
    if (id < 0) {
        std::cout << "Start Ability failed." << std::endl;
        return 1;
    }

    config.AddItem(OHOS::AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION, "horizontal");
    simulator->UpdateConfiguration(config);

    simulator->TerminateAbility(id);
    return 0;
}
