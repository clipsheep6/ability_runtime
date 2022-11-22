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

#include <gtest/gtest.h>

#include <thread>
#include "ability_command.h"
#include "tool_system_test.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace {
const std::string STRING_DEVICE_NAME = "device";

const std::string STRING_SERVICE_ABILITY_BUNDLE_PATH = "/data/test/resource/aa/serviceAbilityBundleForStop.hap";
const std::string STRING_SERVICE_ABILITY_BUNDLE_NAME = "com.ohos.tools.serviceAbilityBundleForStop";
const std::string STRING_SERVICE_ABILITY_BUNDLE_NAME_INVALID = STRING_SERVICE_ABILITY_BUNDLE_NAME + ".invalid";
const std::string STRING_SERVICE_ABILITY_NAME = "com.ohos.tools.serviceAbilityBundleForStop.ServiceAbility";
const std::string STRING_SERVICE_ABILITY_NAME_INVALID = STRING_SERVICE_ABILITY_NAME + ".Invalid";
}  // namespace

class AaCommandStopServiceSystemTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AaCommandStopServiceSystemTest::SetUpTestCase()
{}

void AaCommandStopServiceSystemTest::TearDownTestCase()
{}

void AaCommandStopServiceSystemTest::SetUp()
{
    // reset optind to 0
    optind = 0;
}

void AaCommandStopServiceSystemTest::TearDown()
{}

/**
 * @tc.number: Aa_Command_StopService_SystemTest_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa stop-service -d <device-id> -a <ability-name> -b <bundle-name>" command.
 */
HWTEST_F(AaCommandStopServiceSystemTest, Aa_Command_StopService_SystemTest_0200, Function | MediumTest | Level1)
{
    // stop the invalid service ability
    std::string command = "aa stop-service -d " + STRING_DEVICE_NAME + " -a " + STRING_SERVICE_ABILITY_NAME_INVALID +
                          " -b " + STRING_SERVICE_ABILITY_BUNDLE_NAME_INVALID;
    std::string commandResult = ToolSystemTest::ExecuteCommand(command);

    EXPECT_PRED2(ToolSystemTest::IsSubSequence, commandResult, STRING_STOP_SERVICE_ABILITY_NG + "\n");
}
