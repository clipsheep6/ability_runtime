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
#define private public
#define protected public
#include "deeplink_reserve_config.h"
#undef private
#undef protected


namespace OHOS {
namespace AAFwk {
using namespace testing::ext;
class DeepLinkReserveConfigTest : public testing::Test {
public:
    DeepLinkReserveConfigTest() = default;
    virtual ~DeepLinkReserveConfigTest() = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void DeepLinkReserveConfigTest::SetUpTestCase(void)
{}
void DeepLinkReserveConfigTest::TearDownTestCase(void)
{}
void DeepLinkReserveConfigTest::SetUp()
{}
void DeepLinkReserveConfigTest::TearDown()
{}

/*
 * Feature: deepLinkReserveConfig
 * Function: LoadConfiguration
 * SubFunction: NA
 * FunctionPoints: deepLinkReserveConfig LoadConfiguration
 * EnvConditions: NA
 * CaseDescription: Verify that the deepLinkReserveConfig LoadConfiguration is normal.
 */
HWTEST_F(DeepLinkReserveConfigTest, AaFwk_DeepLinkReserveConfigTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DeepLinkReserveConfigTest_0100 start";
    DeepLinkReserveConfig deepLinkReserveConfig;
    EXPECT_EQ(deepLinkReserveConfig.LoadConfiguration(), false);
    GTEST_LOG_(INFO) << "AaFwk_DeepLinkReserveConfigTest_0100 end";
}

/*
 * Feature: deepLinkReserveConfig
 * Function: isLinkReserved
 * SubFunction: NA
 * FunctionPoints: deepLinkReserveConfig isLinkReserved
 * EnvConditions: NA
 * CaseDescription: Verify that the deepLinkReserveConfig isLinkReserved is normal.
 */
HWTEST_F(DeepLinkReserveConfigTest, AaFwk_DeepLinkReserveConfigTest_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DeepLinkReserveConfigTest_0200 start";
    DeepLinkReserveConfig deepLinkReserveConfig;
    const nlohmann::json DEFAULT_CONFIG = R"(
        {
            "deepLinkReservedUri": [
                {
                    "bundleName": "bundleName",
                    "uris": [
                        {
                            "scheme": "http",
                            "host": "www.xxx.com",
                            "port": "80",
                            "path": "path",
                            "pathStartWith": "pathStartWith",
                            "pathRegex": "pathRegex",
                            "type": "type",
                            "utd": "utd"
                        }
                    ]
                }
            ]
        }
    )"_json;
    deepLinkReserveConfig.LoadReservedUriList(DEFAULT_CONFIG);
    std::string linkString = "http://www.xxx.com:80/pathRegex";
    std::string bundleName = "just a test";
    auto ans = deepLinkReserveConfig.isLinkReserved(linkString, bundleName);
    EXPECT_EQ(ans, true);
    GTEST_LOG_(INFO) << "AaFwk_DeepLinkReserveConfigTest_0200 end";
}

}  // namespace AAFwk
}  // namespace OHOS
