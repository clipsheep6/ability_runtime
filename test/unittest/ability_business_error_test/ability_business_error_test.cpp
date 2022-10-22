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

#include <gtest/gtest.h>

#include "ability_business_error.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class AbilityBusinessErrorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityBusinessErrorTest::SetUpTestCase()
{}

void AbilityBusinessErrorTest::TearDownTestCase()
{}

void AbilityBusinessErrorTest::SetUp()
{}

void AbilityBusinessErrorTest::TearDown()
{}

/**
 * @tc.name: GetErrorMsg_0100
 * @tc.desc: GetErrorMsg_0100 Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(AbilityBusinessErrorTest, GetErrorMsg_0100, TestSize.Level0)
{
    std::string result = GetErrorMsg(AbilityErrorCode::ERROR_OK);
    EXPECT_TRUE(result == "OK.");

    result = GetErrorMsg(static_cast<AbilityErrorCode>(-1000));
    EXPECT_TRUE(result == "");
}

/**
 * @tc.name: GetJsErrorCodeByNativeError_0100
 * @tc.desc: GetJsErrorCodeByNativeError_0100 Test
 * @tc.type: FUNC
 * @tc.require: issueI581RO
 */
HWTEST_F(AbilityBusinessErrorTest, GetJsErrorCodeByNativeError_0100, TestSize.Level0)
{
    AbilityErrorCode result = GetJsErrorCodeByNativeError(0);
    EXPECT_TRUE(result == AbilityErrorCode::ERROR_OK);

    result = GetJsErrorCodeByNativeError(-1000);
    EXPECT_TRUE(result == AbilityErrorCode::ERROR_CODE_INNER);
}
}  // namespace AAFwk
}  // namespace OHOS
