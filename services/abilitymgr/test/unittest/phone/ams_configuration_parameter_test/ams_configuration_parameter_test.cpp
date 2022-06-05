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

#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#define private public
#include "ams_configuration_parameter.h"
#undef private

using namespace testing::ext;
using namespace OHOS::AAFwk;

/*
 * Feature: ams_configuration_parameter
 * Function: LoadAmsConfiguration
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify parsing erroneous input file without corruption.
 */
HWTEST(ACPTest, json_test_001, TestSize.Level1) {
    AmsConfigurationParameter acp;
    std::ofstream inputFile("/tmp/test.txt");
    if (!inputFile.is_open()) {
        FAIL() << "file not open";
    }
    inputFile << "hello\n";
    EXPECT_EQ(AmsConfigurationParameter::READ_JSON_FAIL,  acp.LoadAmsConfiguration("/tmp/test.txt"));
    inputFile.close();

    if (remove("/tmp/test.txt") != 0) {
        FAIL() << "fail to delete the file";
    };
}
/*
 * Feature: ams_configuration_parameter
 * Function: LoadAppConfigurationForStartUpService
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify parsing erroneous json input without corruption.
 */
HWTEST(ACPTest, json_test_002, TestSize.Level1) {
    AmsConfigurationParameter acp;
    using json = nlohmann::json;
    json jsonObject = {{
        AmsConfig::SERVICE_ITEM_AMS, {
            {AmsConfig::STARTUP_SETTINGS_DATA, "hello"},
            {AmsConfig::MISSION_SAVE_TIME, 1},
            {AmsConfig::APP_NOT_RESPONSE_PROCESS_TIMEOUT_TIME, 1},
            {AmsConfig::AMS_TIMEOUT_TIME, 1},
            {AmsConfig::ROOT_LAUNCHER_RESTART_MAX, 1},
        }
    }};
    EXPECT_EQ(-1, acp.LoadAppConfigurationForStartUpService(jsonObject));
}
/*
 * Feature: ams_configuration_parameter
 * Function: LoadAppConfigurationForStartUpService
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify parsing erroneous json input without corruption.
 */
HWTEST(ACPTest, json_test_003, TestSize.Level1) {
    AmsConfigurationParameter acp;
    using json = nlohmann::json;
    json jsonObject = {{
        AmsConfig::SERVICE_ITEM_AMS, {
            {AmsConfig::STARTUP_SETTINGS_DATA, true},
            {AmsConfig::MISSION_SAVE_TIME, nullptr},
            {AmsConfig::APP_NOT_RESPONSE_PROCESS_TIMEOUT_TIME, 1},
            {AmsConfig::AMS_TIMEOUT_TIME, 1},
            {AmsConfig::ROOT_LAUNCHER_RESTART_MAX, 1},
        }
    }};
    EXPECT_EQ(-1, acp.LoadAppConfigurationForStartUpService(jsonObject));
}

/*
 * Feature: ams_configuration_parameter
 * Function: LoadAppConfigurationForStartUpService
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify parsing erroneous json input without corruption.
 */
HWTEST(ACPTest, json_test_004, TestSize.Level1) {
    AmsConfigurationParameter acp;
    using json = nlohmann::json;
    json jsonObject = {{
        AmsConfig::SERVICE_ITEM_AMS, {
            {AmsConfig::STARTUP_SETTINGS_DATA, true},
            {AmsConfig::MISSION_SAVE_TIME, nullptr},
            {AmsConfig::APP_NOT_RESPONSE_PROCESS_TIMEOUT_TIME, 1},
        }
    }};
    EXPECT_EQ(-1, acp.LoadAppConfigurationForStartUpService(jsonObject));
}