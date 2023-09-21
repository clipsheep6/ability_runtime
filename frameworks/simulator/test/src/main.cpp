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
#include <fstream>
#include <iostream>
#include <map>

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

int32_t main(int32_t argc, const char *argv[])
{
    if (argc < PARAM_EIGHTEEN) {
        std::cout << "Insufficient parameters." << std::endl;
        return 1;
    }

    OHOS::AbilityRuntime::Options options;
    options.modulePath = argv[PARAM_ONE];
    options.resourcePath = argv[PARAM_TWO];
    options.debugPort = atoi(argv[PARAM_THREE]);
    options.assetPath = argv[PARAM_FOUR];
    options.systemResourcePath = argv[PARAM_FIVE];
    options.appResourcePath = argv[PARAM_SIX];
    options.containerSdkPath = argv[PARAM_SEVEN];
    options.url = argv[PARAM_EIGHT];
    options.language = argv[PARAM_NINE];
    options.region = argv[PARAM_TEN];
    options.script = argv[PARAM_ELEVEN];
    options.themeId = atoi(argv[PARAM_TWELVE]);
    options.deviceWidth = atoi(argv[PARAM_THIRTEEN]);
    options.deviceHeight = atoi(argv[PARAM_FOURTEEN]);
    options.isRound = atoi(argv[PARAM_FIFTEEN]);
    options.previewPath = argv[PARAM_SIXTEEN];

    OHOS::AppExecFwk::Configuration config;
    config.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "testlanguage");
    config.AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "light");
    config.AddItem(OHOS::AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION, "vertical");
    auto configuration = std::make_shared<OHOS::AppExecFwk::Configuration>(config);
    options.configuration = configuration;

    std::string moduleJsonPath = argv[PARAM_SEVENTEEN];
    std::ifstream stream(moduleJsonPath, std::ios::ate | std::ios::binary);
    if (!stream.is_open()) {
        std::cout << "Failed to open: " << moduleJsonPath << std::endl;
        return -1;
    }

    size_t len = stream.tellg();
    std::cout << "module json len: " << len << std::endl;
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(len);
    stream.seekg(0);
    stream.read(reinterpret_cast<char*>(buffer.get()), len);
    stream.close();
    auto buf = buffer.release();
    options.moduleJsonBuffer.assign(buf, buf + len);

    auto simulator = OHOS::AbilityRuntime::Simulator::Create(options);
    if (!simulator) {
        std::cout << "Create Simulator failed." << std::endl;
        return 1;
    }
    std::map<std::string, std::string> mockList {};
    simulator->SetMockList(mockList);
    mockList.emplace("test1", "1");
    mockList.emplace("test2", "2");
    mockList.emplace("test3", "!@#$%^&*(){}[]:\";',./<>?\\|");
    mockList.emplace("test4", "中文测试");
    simulator->SetMockList(mockList);

    std::string abilitySrcPath {argv[PARAM_EIGHTEEN]};
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
