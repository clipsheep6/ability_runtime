/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#define private public
#include "advanced_security_mode_manager.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "parameters.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AdvancedSecurityModeManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();

protected:
    bool deviceAdvSecModeEnabled_ = false;
};

void AdvancedSecurityModeManagerTest::SetUp()
{
    int32_t state = OHOS::system::GetIntParameter<int32_t>("ohos.boot.advsecmode.state", 0);
    deviceAdvSecModeEnabled_ = state > 0;
}

void AdvancedSecurityModeManagerTest::TearDown()
{}

/**
 * @tc.number: AdvancedSecurityModeManager_Init_0100
 * @tc.desc: Test Init works
 * @tc.type: FUNC
 */
HWTEST_F(AdvancedSecurityModeManagerTest, AdvancedSecurityModeManager_Init_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "AdvancedSecurityModeManager_Init_0100 start.");
    auto manager = std::make_shared<AdvancedSecurityModeManager>();
    manager->Init();
    EXPECT_EQ(manager->isAdvSecModeEnabled_, deviceAdvSecModeEnabled_);
}

/**
 * @tc.number: AdvancedSecurityModeManager_IsJITEnabled_0100
 * @tc.desc: Test IsJITEnabled works
 * @tc.type: FUNC
 */
HWTEST_F(AdvancedSecurityModeManagerTest, AdvancedSecurityModeManager_IsJITEnabled_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "AdvancedSecurityModeManager_IsJITEnabled_0100 start.");
    auto manager = std::make_shared<AdvancedSecurityModeManager>();
    manager->isAdvSecModeEnabled_ = true;
    EXPECT_EQ(manager->IsJITEnabled(), false);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
