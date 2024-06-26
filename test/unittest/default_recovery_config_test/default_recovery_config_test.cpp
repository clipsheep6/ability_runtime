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
#include <memory>

#include "gtest/gtest.h"
#include "hilog_tag_wrapper.h"
#define private public
#define protected public
#include "default_recovery_config.h"
#undef private
#undef protected

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class DefaultRecoveryConfigTest : public testing::Test {
public:
    DefaultRecoveryConfigTest() = default;
    virtual ~DefaultRecoveryConfigTest() = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DefaultRecoveryConfigTest::SetUpTestCase(void)
{}

void DefaultRecoveryConfigTest::TearDownTestCase(void)
{}

void DefaultRecoveryConfigTest::SetUp()
{}

void DefaultRecoveryConfigTest::TearDown()
{}

/**
 * @tc.name: LoadConfiguration_0100
 * @tc.desc: basic function test of load configuration.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DefaultRecoveryConfigTest, LoadConfiguration_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    DefaultRecoveryConfig config;
    EXPECT_EQ(config.LoadConfiguration(), true);
    EXPECT_EQ(config.IsBundleDefaultRecoveryEnabled("com.acts.test"), false);
    EXPECT_EQ(config.GetReserveNumber(), 5);
    EXPECT_EQ(config.GetTimeoutDeleteTime(), 168);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: LoadConfiguration_0200
 * @tc.desc: basic function test of load configuration.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DefaultRecoveryConfigTest, LoadConfiguration_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    DefaultRecoveryConfig config;
    const nlohmann::json DEFAULT_CONFIG = R"(
        {
            "default_recovery": {
                "support_bundle_name_list": [
                    "com.acts.example1",
                    "com.acts.example2"
                ],
                "reserve_number_when_timeout": 10,
                "recovery_data_timeout_delete_time": 7
            }
        }
    )"_json;
    config.LoadDefaultRecovery(DEFAULT_CONFIG);
    EXPECT_EQ(config.IsBundleDefaultRecoveryEnabled("com.acts.example1"), true);
    EXPECT_EQ(config.IsBundleDefaultRecoveryEnabled("com.acts.example2"), true);
    EXPECT_EQ(config.GetReserveNumber(), 10);
    EXPECT_EQ(config.GetTimeoutDeleteTime(), 7);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}
} // namespace AbilityRuntime
} // namespace OHOS
