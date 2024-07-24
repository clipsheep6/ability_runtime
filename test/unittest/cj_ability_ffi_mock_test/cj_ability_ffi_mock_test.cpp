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

#include "foundation/ability/ability_runtime/frameworks/cj/mock/cj_ability_ffi.cpp"
#include <gtest/gtest.h>

using namespace testing;
using namespace testing::ext;

class CjAbilityFfiMockTest : public testing::Test {
public:
    CjAbilityFfiMockTest()
    {}
    ~CjAbilityFfiMockTest()
    {}
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void CjAbilityFfiMockTest::SetUpTestCase()
{}

void CjAbilityFfiMockTest::TearDownTestCase()
{}

void CjAbilityFfiMockTest::SetUp()
{}

void CjAbilityFfiMockTest::TearDown()
{}

/**
 * @tc.name: CjAbilityFfiMockTestGlobalVariables_0100
 * @tc.desc: CjAbilityFfiMockTest test for GlobalVariables.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityFfiMockTest, CjAbilityFfiMockTestGlobalVariables_0100, TestSize.Level1)
{
    EXPECT_NE(nullptr, FFICJWantDelete);
    EXPECT_NE(nullptr, FFICJWantGetWantInfo);
    EXPECT_NE(nullptr, FFICJWantParamsDelete);
    EXPECT_NE(nullptr, FFICJWantCreateWithWantInfo);
    EXPECT_NE(nullptr, FFICJWantParseUri);
    EXPECT_NE(nullptr, FFICJWantAddEntity);
    EXPECT_NE(nullptr, FFICJElementNameCreateWithContent);
    EXPECT_NE(nullptr, FFICJElementNameDelete);
    EXPECT_NE(nullptr, FFICJElementNameGetElementNameInfo);
    EXPECT_NE(nullptr, FFICJElementNameParamsDelete);
    EXPECT_NE(nullptr, FFIAbilityGetAbilityContext);
    EXPECT_NE(nullptr, FFIAbilityContextGetFilesDir);
    EXPECT_NE(nullptr, FFIGetContext);
    EXPECT_NE(nullptr, FFICreateNapiValue);
    EXPECT_NE(nullptr, FFIGetArea);
    EXPECT_NE(nullptr, FFICJApplicationInfo);
    EXPECT_NE(nullptr, FFIAbilityDelegatorRegistryGetAbilityDelegator);
    EXPECT_NE(nullptr, FFIAbilityDelegatorStartAbility);
    EXPECT_NE(nullptr, FFIAbilityDelegatorExecuteShellCommand);
    EXPECT_NE(nullptr, FFIGetExitCode);
    EXPECT_NE(nullptr, FFIGetStdResult);
    EXPECT_NE(nullptr, FFIDump);
    EXPECT_NE(nullptr, FFIAbilityDelegatorApplicationContext);
}


/**
 * @tc.name: CjAbilityFfiMockTestGetBroker_0100
 * @tc.desc: CjAbilityFfiMockTest test for GetBroker.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityFfiMockTest, CjAbilityFfiMockTestGetBroker_0100, TestSize.Level1)
{
    AbilityContextBroker* broker = FFIAbilityContextGetBroker();
    EXPECT_NE(nullptr, broker);
    EXPECT_EQ(1, broker->isAbilityContextExisted);
    EXPECT_EQ(1, broker->getSizeOfStartOptions);
    EXPECT_EQ(1, broker->getAbilityInfo);
    EXPECT_EQ(1, broker->getHapModuleInfo);
    EXPECT_EQ(1, broker->getConfiguration);
    EXPECT_EQ(1, broker->startAbility);
    EXPECT_EQ(1, broker->startAbilityWithOption);
    EXPECT_EQ(1, broker->startAbilityWithAccount);
    EXPECT_EQ(1, broker->startAbilityWithAccountAndOption);
    EXPECT_EQ(1, broker->startServiceExtensionAbility);
    EXPECT_EQ(1, broker->startServiceExtensionAbilityWithAccount);
    EXPECT_EQ(1, broker->stopServiceExtensionAbility);
    EXPECT_EQ(1, broker->stopServiceExtensionAbilityWithAccount);
    EXPECT_EQ(1, broker->terminateSelf);
    EXPECT_EQ(1, broker->terminateSelfWithResult);
    EXPECT_EQ(1, broker->isTerminating);
    EXPECT_EQ(1, broker->connectAbility);
    EXPECT_EQ(1, broker->connectAbilityWithAccount);
    EXPECT_EQ(1, broker->disconnectAbility);
    EXPECT_EQ(1, broker->startAbilityForResult);
    EXPECT_EQ(1, broker->startAbilityForResultWithOption);
    EXPECT_EQ(1, broker->startAbilityForResultWithAccount);
    EXPECT_EQ(1, broker->startAbilityForResultWithAccountAndOption);
    EXPECT_EQ(1, broker->requestPermissionsFromUser);
    EXPECT_EQ(1, broker->setMissionLabel);
    EXPECT_EQ(1, broker->setMissionIcon);
}

/**
 * @tc.name: CjAbilityFfiMockTestRegisterCJAbilityStageFuncs_0100
 * @tc.desc: CjAbilityFfiMockTest test for RegisterCJAbilityStageFuncs.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityFfiMockTest, CjAbilityFfiMockTestRegisterCJAbilityStageFuncs_0100, TestSize.Level1)
{
    RegisterCJAbilityStageFuncs();  // 实际执行函数
}

/**
 * @tc.name: CjAbilityFfiMockTestRegisterCJAbilityConnectCallbackFuncs_0100
 * @tc.desc: CjAbilityFfiMockTest test for RegisterCJAbilityConnectCallbackFuncs.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityFfiMockTest, CjAbilityFfiMockTestRegisterCJAbilityConnectCallbackFuncs_0100, TestSize.Level1)
{
    RegisterCJAbilityConnectCallbackFuncs();
}

/**
 * @tc.name: CjAbilityFfiMockTestRegisterCJAbilityCallbacks_0100
 * @tc.desc: CjAbilityFfiMockTest test for RegisterCJAbilityCallbacks.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityFfiMockTest, CjAbilityFfiMockTestRegisterCJAbilityCallbacks_0100, TestSize.Level1)
{
    RegisterCJAbilityCallbacks();
}

/**
 * @tc.name: CjAbilityFfiMockTestRegisterCJAbilityFuncs_0100
 * @tc.desc: CjAbilityFfiMockTest test for RegisterCJAbilityFuncs.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityFfiMockTest, CjAbilityFfiMockTestRegisterCJAbilityFuncs_0100, TestSize.Level1)
{
    RegisterCJAbilityFuncs();
}

/**
 * @tc.name: CjAbilityFfiMockTestRequestDialogService_0100
 * @tc.desc: CjAbilityFfiMockTest test for FFIAbilityContextRequestDialogService.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityFfiMockTest, CjAbilityFfiMockTestRequestDialogService_0100, TestSize.Level1)
{
    FFIAbilityContextRequestDialogService();
}