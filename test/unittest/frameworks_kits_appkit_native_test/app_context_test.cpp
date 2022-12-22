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

#define private public
#define protected public
#include "app_context.h"
#undef private
#undef protected

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace {
const int INT_PARAMETER = 0;
const int GET_MISSION_ID = -1;
} // namespace

class AppContextTest : public testing::Test {
public:
    AppContextTest()
    {}
    ~AppContextTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppContextTest::SetUpTestCase(void)
{}

void AppContextTest::TearDownTestCase(void)
{}

void AppContextTest::SetUp(void)
{}

void AppContextTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_AppContextTest_GetAbilityInfo_0100
 * @tc.name: AppContext GetAbilityInfo
 * @tc.desc: Test whether the function of GetAbilityInfo is normal.
 */
HWTEST_F(AppContextTest, AppExecFwk_AppContextTest_GetAbilityInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_GetAbilityInfo_0100 start";
    auto appContext = std::make_shared<AppContext>();
    std::shared_ptr<AbilityInfo> result = appContext->GetAbilityInfo();
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_GetAbilityInfo_0100 end";
}

/**
 * @tc.number: AppExecFwk_AppContextTest_StartAbility_0100
 * @tc.name: AppContext StartAbility
 * @tc.desc: Test whether the function of GetAbilityInfo is normal.
 */
HWTEST_F(AppContextTest, AppExecFwk_AppContextTest_StartAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_StartAbility_0100 start";
    auto appContext = std::make_shared<AppContext>();
    AAFwk::Want want;
    int requestCode = INT_PARAMETER;
    auto result = appContext->StartAbility(want, requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_StartAbility_0100 end";
}

/**
 * @tc.number: AppExecFwk_AppContextTest_StartAbility_0200
 * @tc.name: AppContext StartAbility
 * @tc.desc: Test whether the function of StartAbility is normal.
 */
HWTEST_F(AppContextTest, AppExecFwk_AppContextTest_StartAbility_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_StartAbility_0200 start";
    auto appContext = std::make_shared<AppContext>();
    AbilityStartSetting abilityStartSetting;
    AAFwk::Want want;
    int requestCode = INT_PARAMETER;
    auto result = appContext->StartAbility(want, requestCode, abilityStartSetting);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_StartAbility_0200 end";
}

/**
 * @tc.number: AppExecFwk_AppContextTest_TerminateAbility_0100
 * @tc.name: AppContext TerminateAbility
 * @tc.desc: Test whether the function of TerminateAbility is normal.
 */
HWTEST_F(AppContextTest, AppExecFwk_AppContextTest_TerminateAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_TerminateAbility_0100 start";
    auto appContext = std::make_shared<AppContext>();
    int requestCode = INT_PARAMETER;
    auto result = appContext->TerminateAbility(requestCode);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_TerminateAbility_0100 end";
}

/**
 * @tc.number: AppExecFwk_AppContextTest_TerminateAbility_0200
 * @tc.name: AppContext TerminateAbility
 * @tc.desc: Test whether the function of TerminateAbility is normal.
 */
HWTEST_F(AppContextTest, AppExecFwk_AppContextTest_TerminateAbility_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_TerminateAbility_0200 start";
    auto appContext = std::make_shared<AppContext>();
    auto result = appContext->TerminateAbility();
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_TerminateAbility_0200 end";
}

/**
 * @tc.number: AppExecFwk_AppContextTest_TerminateAbilityResult_0100
 * @tc.name: AppContext TerminateAbilityResult
 * @tc.desc: Test whether the function of TerminateAbilityResult is normal.
 */
HWTEST_F(AppContextTest, AppExecFwk_AppContextTest_TerminateAbilityResult_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_TerminateAbilityResult_0100 start";
    auto appContext = std::make_shared<AppContext>();
    int startId = INT_PARAMETER;
    auto result = appContext->TerminateAbilityResult(startId);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_TerminateAbilityResult_0100 end";
}

/**
 * @tc.number: AppExecFwk_AppContextTest_GetCallingBundle_0100
 * @tc.name: AppContext GetCallingBundle
 * @tc.desc: Test whether the function of GetCallingBundle is normal.
 */
HWTEST_F(AppContextTest, AppExecFwk_AppContextTest_GetCallingBundle_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_GetCallingBundle_0100 start";
    auto appContext = std::make_shared<AppContext>();
    auto result = appContext->GetCallingBundle();
    EXPECT_EQ(result, "");
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_GetCallingBundle_0100 end";
}

/**
 * @tc.number: AppExecFwk_AppContextTest_ConnectAbility_0100
 * @tc.name: AppContext ConnectAbility
 * @tc.desc: Test whether the function of ConnectAbility is normal.
 */
HWTEST_F(AppContextTest, AppExecFwk_AppContextTest_ConnectAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_ConnectAbility_0100 start";
    auto appContext = std::make_shared<AppContext>();
    sptr<IAbilityConnection> connect = nullptr;
    AAFwk::Want want;
    auto result = appContext->ConnectAbility(want, connect);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_ConnectAbility_0100 end";
}

/**
 * @tc.number: AppExecFwk_AppContextTest_DisconnectAbility_0100
 * @tc.name: AppContext DisconnectAbility
 * @tc.desc: Test whether the function of DisconnectAbility is normal.
 */
HWTEST_F(AppContextTest, AppExecFwk_AppContextTest_DisconnectAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_DisconnectAbility_0100 start";
    auto appContext = std::make_shared<AppContext>();
    sptr<IAbilityConnection> connect = nullptr;
    auto result = appContext->DisconnectAbility(connect);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_DisconnectAbility_0100 end";
}

/**
 * @tc.number: AppExecFwk_AppContextTest_StopAbility_0100
 * @tc.name: AppContext StopAbility
 * @tc.desc: Test whether the function of StopAbility is normal.
 */
HWTEST_F(AppContextTest, AppExecFwk_AppContextTest_StopAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_StopAbility_0100 start";
    auto appContext = std::make_shared<AppContext>();
    AAFwk::Want want;
    auto result = appContext->StopAbility(want);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_StopAbility_0100 end";
}

/**
 * @tc.number: AppExecFwk_AppContextTest_GetToken_0100
 * @tc.name: AppContext GetToken
 * @tc.desc: Test whether the function of GetToken is normal.
 */
HWTEST_F(AppContextTest, AppExecFwk_AppContextTest_GetToken_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_GetToken_0100 start";
    auto appContext = std::make_shared<AppContext>();
    auto result = appContext->GetToken();
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_GetToken_0100 end";
}

/**
 * @tc.number: AppExecFwk_AppContextTest_GetMissionId_0100
 * @tc.name: AppContext GetMissionId
 * @tc.desc: Test whether the function of GetMissionId is normal.
 */
HWTEST_F(AppContextTest, AppExecFwk_AppContextTest_GetMissionId_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_GetMissionId_0100 start";
    auto appContext = std::make_shared<AppContext>();
    auto result = appContext->GetMissionId();
    EXPECT_EQ(result, GET_MISSION_ID);
    GTEST_LOG_(INFO) << "AppExecFwk_AppContextTest_GetMissionId_0100 end";
}

}  // namespace AppExecFwk
}  // namespace OHOS
