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

#include "authorization_result.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
class AuthorizationResultTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AuthorizationResultTest::SetUpTestCase(void)
{}

void AuthorizationResultTest::TearDownTestCase(void)
{}

void AuthorizationResultTest::SetUp(void)
{}

void AuthorizationResultTest::TearDown(void)
{}

/**
 * @tc.number: GrantResultsCallback_0100
 * @tc.name: GrantResultsCallback
 * @tc.desc: Grant Results Callback sucess
 */
HWTEST_F(AuthorizationResultTest, GrantResultsCallback_0100, Function | MediumTest | Level1)
{
    PermissionRequestTask task = [](const std::vector<std::string>& vecString, const std::vector<int>& vecInt)
    { GTEST_LOG_(INFO) << "GrantResultsCallback_0100 task called"; };
    auto authorizationResult = new AbilityRuntime::AuthorizationResult(std::move(task));
    EXPECT_NE(authorizationResult, nullptr);
    std::vector<std::string> permissions;
    std::vector<int> grantResults;
    authorizationResult->GrantResultsCallback(permissions, grantResults);
}

/**
 * @tc.number: GrantResultsCallback_0200
 * @tc.name: GrantResultsCallback
 * @tc.desc: Grant Results Callback failed
 */
HWTEST_F(AuthorizationResultTest, GrantResultsCallback_0200, Function | MediumTest | Level1)
{
    PermissionRequestTask task;
    auto authorizationResult = new AbilityRuntime::AuthorizationResult(std::move(task));
    EXPECT_NE(authorizationResult, nullptr);
    std::vector<std::string> permissions;
    std::vector<int> grantResults;
    authorizationResult->GrantResultsCallback(permissions, grantResults);
}
} // namespace AppExecFwk
} // namespace OHOS