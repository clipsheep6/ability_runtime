/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "ability_running_record.h"
#include "app_state_callback_host.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "mock_ability_token.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AbilityRunningRecordTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
public:
protected:
    static const std::string GetTestBundleName()
    {
        return "test_bundle_name";
    }
    static const std::string GetTestAbilityInfoName()
    {
        return "test_ability_info_name";
    }
    static const std::string GetTestModuleName()
    {
        return "test_module_name";
    }
};

void AbilityRunningRecordTest::SetUpTestCase()
{}

void AbilityRunningRecordTest::TearDownTestCase()
{}

void AbilityRunningRecordTest::SetUp()
{}

void AbilityRunningRecordTest::TearDown()
{}


/*
 * Feature: AbilityRunningRecord
 * Function: GetName
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetName
 * EnvConditions: NA
 * CaseDescription: GetName
 */
HWTEST_F(AbilityRunningRecordTest, GetName_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetName_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityInfoName();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto name = record->GetName();
    ASSERT_EQ(name, "test_ability_info_name");
    TAG_LOGD(AAFwkTag::TEST, "GetName_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetBundleName
 * EnvConditions: NA
 * CaseDescription: GetBundleName
 */
HWTEST_F(AbilityRunningRecordTest, GetBundleName_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetBundleName_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = GetTestBundleName();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto name = record->GetBundleName();
    ASSERT_EQ(name, "test_bundle_name");
    TAG_LOGD(AAFwkTag::TEST, "GetBundleName_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetModuleName
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetModuleName
 * EnvConditions: NA
 * CaseDescription: GetModuleName
 */
HWTEST_F(AbilityRunningRecordTest, GetModuleName_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetModuleName_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->moduleName = GetTestModuleName();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto name = record->GetModuleName();
    ASSERT_EQ(name, "test_module_name");
    TAG_LOGD(AAFwkTag::TEST, "GetModuleName_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetAbilityInfo
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetAbilityInfo
 * EnvConditions: NA
 * CaseDescription: GetAbilityInfo
 */
HWTEST_F(AbilityRunningRecordTest, GetAbilityInfo_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetAbilityInfo_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->GetAbilityInfo();
    ASSERT_NE(iret, nullptr);
    TAG_LOGD(AAFwkTag::TEST, "GetAbilityInfo_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetWant
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetWant
 * EnvConditions: NA
 * CaseDescription: GetWant
 */
HWTEST_F(AbilityRunningRecordTest, GetWant_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetWant_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->GetWant();
    ASSERT_EQ(iret, nullptr);
    TAG_LOGD(AAFwkTag::TEST, "GetWant_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetWant
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetWant
 * EnvConditions: NA
 * CaseDescription: SetWant
 */
HWTEST_F(AbilityRunningRecordTest, SetWant_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetWant_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    const std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    record->SetWant(want);
    TAG_LOGD(AAFwkTag::TEST, "SetWant_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetToken
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetToken
 * EnvConditions: NA
 * CaseDescription: GetToken
 */
HWTEST_F(AbilityRunningRecordTest, GetToken_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetToken_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->GetToken();
    ASSERT_NE(iret, nullptr);
    TAG_LOGD(AAFwkTag::TEST, "GetToken_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetState
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetState
 * EnvConditions: NA
 * CaseDescription: SetState
 */
HWTEST_F(AbilityRunningRecordTest, SetState_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetState_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    record->SetState(AbilityState::ABILITY_STATE_CREATE);
    TAG_LOGD(AAFwkTag::TEST, "SetState_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetState
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetState
 * EnvConditions: NA
 * CaseDescription: GetState
 */
HWTEST_F(AbilityRunningRecordTest, GetState_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetState_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->GetState();
    ASSERT_EQ(iret, AbilityState::ABILITY_STATE_CREATE);
    TAG_LOGD(AAFwkTag::TEST, "GetState_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: IsSameState
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord IsSameState
 * EnvConditions: NA
 * CaseDescription: IsSameState
 */
HWTEST_F(AbilityRunningRecordTest, IsSameState_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "IsSameState_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->IsSameState(AbilityState::ABILITY_STATE_CREATE);
    ASSERT_EQ(iret, true);
    TAG_LOGD(AAFwkTag::TEST, "IsSameState_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetLastLaunchTime
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetLastLaunchTime
 * EnvConditions: NA
 * CaseDescription: GetLastLaunchTime
 */
HWTEST_F(AbilityRunningRecordTest, GetLastLaunchTime_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetLastLaunchTime_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->GetLastLaunchTime();
    ASSERT_EQ(iret, 0);
    TAG_LOGD(AAFwkTag::TEST, "GetLastLaunchTime_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetPreToken
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetPreToken
 * EnvConditions: NA
 * CaseDescription: GetPreToken
 */
HWTEST_F(AbilityRunningRecordTest, GetPreToken_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetPreToken_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->GetPreToken();
    ASSERT_EQ(iret, nullptr);
    TAG_LOGD(AAFwkTag::TEST, "GetPreToken_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetPreToken
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetPreToken
 * EnvConditions: NA
 * CaseDescription: SetPreToken
 */
HWTEST_F(AbilityRunningRecordTest, SetPreToken_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetPreToken_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    sptr<IRemoteObject> pretoken = new MockAbilityToken();
    record->SetPreToken(pretoken);
    TAG_LOGD(AAFwkTag::TEST, "SetPreToken_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetVisibility
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetVisibility
 * EnvConditions: NA
 * CaseDescription: SetVisibility
 */
HWTEST_F(AbilityRunningRecordTest, SetVisibility_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetVisibility_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    const int32_t visibility = 1;
    record->SetVisibility(visibility);
    TAG_LOGD(AAFwkTag::TEST, "SetVisibility_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetVisibility
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetVisibility
 * EnvConditions: NA
 * CaseDescription: GetVisibility
 */
HWTEST_F(AbilityRunningRecordTest, GetVisibility_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetVisibility_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->GetVisibility();
    ASSERT_EQ(iret, 0);
    TAG_LOGD(AAFwkTag::TEST, "GetVisibility_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetPerceptibility
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetPerceptibility
 * EnvConditions: NA
 * CaseDescription: SetPerceptibility
 */
HWTEST_F(AbilityRunningRecordTest, SetPerceptibility_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetPerceptibility_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    const int32_t perceptibility = 1;
    record->SetPerceptibility(perceptibility);
    TAG_LOGD(AAFwkTag::TEST, "SetPerceptibility_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetPerceptibility
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetPerceptibility
 * EnvConditions: NA
 * CaseDescription: GetPerceptibility
 */
HWTEST_F(AbilityRunningRecordTest, GetPerceptibility_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetPerceptibility_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->GetPerceptibility();
    ASSERT_EQ(iret, 0);
    TAG_LOGD(AAFwkTag::TEST, "GetPerceptibility_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetConnectionState
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetConnectionState
 * EnvConditions: NA
 * CaseDescription: SetConnectionState
 */
HWTEST_F(AbilityRunningRecordTest, SetConnectionState_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetConnectionState_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    const int32_t connectionState = 1;
    record->SetConnectionState(connectionState);
    TAG_LOGD(AAFwkTag::TEST, "SetConnectionState_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetConnectionState
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetConnectionState
 * EnvConditions: NA
 * CaseDescription: GetConnectionState
 */
HWTEST_F(AbilityRunningRecordTest, GetConnectionState_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetConnectionState_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->GetConnectionState();
    ASSERT_EQ(iret, 0);
    TAG_LOGD(AAFwkTag::TEST, "GetConnectionState_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetEventId
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetEventId
 * EnvConditions: NA
 * CaseDescription: SetEventId
 */
HWTEST_F(AbilityRunningRecordTest, SetEventId_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetEventId_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    const int64_t eventId = 1;
    record->SetEventId(eventId);
    TAG_LOGD(AAFwkTag::TEST, "SetEventId_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetEventId
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetEventId
 * EnvConditions: NA
 * CaseDescription: GetEventId
 */
HWTEST_F(AbilityRunningRecordTest, GetEventId_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetEventId_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->GetEventId();
    ASSERT_EQ(iret, 0);
    TAG_LOGD(AAFwkTag::TEST, "GetEventId_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetTerminating
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetTerminating
 * EnvConditions: NA
 * CaseDescription: SetTerminating
 */
HWTEST_F(AbilityRunningRecordTest, SetTerminating_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetTerminating_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    record->SetTerminating();
    TAG_LOGD(AAFwkTag::TEST, "SetTerminating_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: IsTerminating
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord IsTerminating
 * EnvConditions: NA
 * CaseDescription: IsTerminating
 */
HWTEST_F(AbilityRunningRecordTest, IsTerminating_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "IsTerminating_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->IsTerminating();
    ASSERT_EQ(iret, false);
    TAG_LOGD(AAFwkTag::TEST, "IsTerminating_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetOwnerUserId
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetOwnerUserId
 * EnvConditions: NA
 * CaseDescription: SetOwnerUserId
 */
HWTEST_F(AbilityRunningRecordTest, SetOwnerUserId_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetOwnerUserId_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    int32_t ownerUserId = 1;
    record->SetOwnerUserId(ownerUserId);
    TAG_LOGD(AAFwkTag::TEST, "SetOwnerUserId_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetOwnerUserId
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetOwnerUserId
 * EnvConditions: NA
 * CaseDescription: GetOwnerUserId
 */
HWTEST_F(AbilityRunningRecordTest, GetOwnerUserId_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetOwnerUserId_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->GetOwnerUserId();
    ASSERT_EQ(iret, -1);
    TAG_LOGD(AAFwkTag::TEST, "GetOwnerUserId_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetIsSingleUser
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetIsSingleUser
 * EnvConditions: NA
 * CaseDescription: SetIsSingleUser
 */
HWTEST_F(AbilityRunningRecordTest, SetIsSingleUser_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetIsSingleUser_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    bool flag = true;
    record->SetIsSingleUser(flag);
    TAG_LOGD(AAFwkTag::TEST, "SetIsSingleUser_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: IsSingleUser
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord IsSingleUser
 * EnvConditions: NA
 * CaseDescription: IsSingleUser
 */
HWTEST_F(AbilityRunningRecordTest, IsSingleUser_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "IsSingleUser_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->IsSingleUser();
    ASSERT_EQ(iret, false);
    TAG_LOGD(AAFwkTag::TEST, "IsSingleUser_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: UpdateFocusState
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord UpdateFocusState
 * EnvConditions: NA
 * CaseDescription: UpdateFocusState
 */
HWTEST_F(AbilityRunningRecordTest, UpdateFocusState_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "UpdateFocusState_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    bool isFocus = true;
    record->UpdateFocusState(isFocus);
    TAG_LOGD(AAFwkTag::TEST, "UpdateFocusState_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetFocusFlag
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetFocusFlag
 * EnvConditions: NA
 * CaseDescription: GetFocusFlag
 */
HWTEST_F(AbilityRunningRecordTest, GetFocusFlag_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetFocusFlag_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->GetFocusFlag();
    ASSERT_EQ(iret, false);
    TAG_LOGD(AAFwkTag::TEST, "GetFocusFlag_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: SetUIExtensionAbilityId
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord SetUIExtensionAbilityId
 * EnvConditions: NA
 * CaseDescription: SetUIExtensionAbilityId
 */
HWTEST_F(AbilityRunningRecordTest, SetUIExtensionAbilityId_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetUIExtensionAbilityId_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    const int32_t uiExtensionAbilityId = 1;
    record->SetUIExtensionAbilityId(uiExtensionAbilityId);
    TAG_LOGD(AAFwkTag::TEST, "SetUIExtensionAbilityId_001 end.");
}

/*
 * Feature: AbilityRunningRecord
 * Function: GetUIExtensionAbilityId
 * SubFunction: NA
 * FunctionPoints: AbilityRunningRecord GetUIExtensionAbilityId
 * EnvConditions: NA
 * CaseDescription: GetUIExtensionAbilityId
 */
HWTEST_F(AbilityRunningRecordTest, GetUIExtensionAbilityId_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetUIExtensionAbilityId_001 start.");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto record = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    ASSERT_NE(record, nullptr);
    auto iret = record->GetUIExtensionAbilityId();
    ASSERT_EQ(iret, 0);
    TAG_LOGD(AAFwkTag::TEST, "GetUIExtensionAbilityId_001 end.");
}

}  // namespace AppExecFwk
}  // namespace OHOS
