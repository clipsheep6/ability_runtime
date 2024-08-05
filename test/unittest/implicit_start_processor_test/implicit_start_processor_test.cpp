/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "implicit_start_processor.h"
#undef private
#undef protected
#include "ability_ecological_rule_mgr_service_param.h"
#include "parameters.h"

using namespace OHOS;
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace {
    constexpr const char* SUPPORT_ACTION_START_SELECTOR = "persist.sys.ability.support.action_start_selector";
    const std::string PARAM_ABILITY_APPINFOS = "ohos.ability.params.appInfos";
    const std::string OPEN_LINK_APP_LINKING_ONLY = "appLinkingOnly";
    const int NFC_CALLER_UID = 1027;
}
namespace OHOS {
namespace AAFwk {

class ImplicitStartProcessorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ImplicitStartProcessorTest::SetUpTestCase(void)
{}
void ImplicitStartProcessorTest::TearDownTestCase(void)
{}
void ImplicitStartProcessorTest::SetUp()
{}
void ImplicitStartProcessorTest::TearDown()
{}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbility
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbility
 * EnvConditions: NA
 * CaseDescription: Verify ImplicitStartAbility
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbility_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    int32_t userId = 0;
    bool res = processor->ImplicitStartAbility(request, userId,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
    EXPECT_TRUE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CheckImplicitStartExtensionIsValid
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CheckImplicitStartExtensionIsValid
 * EnvConditions: NA
 * CaseDescription: Verify CheckImplicitStartExtensionIsValid
 */
HWTEST_F(ImplicitStartProcessorTest, CheckImplicitStartExtensionIsValid_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    ExtensionAbilityInfo extensionInfo;
    Want want;
    want.SetElementName("bundle", "");
    request.want = want;
    bool res = processor->CheckImplicitStartExtensionIsValid(request, extensionInfo);
    EXPECT_TRUE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CheckImplicitStartExtensionIsValid
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CheckImplicitStartExtensionIsValid
 * EnvConditions: NA
 * CaseDescription: Verify CheckImplicitStartExtensionIsValid
 */
HWTEST_F(ImplicitStartProcessorTest, CheckImplicitStartExtensionIsValid_002, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    ExtensionAbilityInfo extensionInfo;
    Want want;
    want.SetElementName("", "");
    request.want = want;
    extensionInfo.type = ExtensionAbilityType::WORK_SCHEDULER;
    bool res = processor->CheckImplicitStartExtensionIsValid(request, extensionInfo);
    EXPECT_FALSE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CheckImplicitStartExtensionIsValid
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CheckImplicitStartExtensionIsValid
 * EnvConditions: NA
 * CaseDescription: Verify CheckImplicitStartExtensionIsValid
 */
HWTEST_F(ImplicitStartProcessorTest, CheckImplicitStartExtensionIsValid_003, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    ExtensionAbilityInfo extensionInfo;
    Want want;
    want.SetElementName("", "");
    request.want = want;
    extensionInfo.type = ExtensionAbilityType::FORM;
    bool res = processor->CheckImplicitStartExtensionIsValid(request, extensionInfo);
    EXPECT_TRUE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify ImplicitStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbilityInner_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    AbilityRequest request;
    int32_t userId = 0;
    request.callType = AbilityCallType::START_OPTIONS_TYPE;
    bool res = processor->ImplicitStartAbilityInner(want, request, userId);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify ImplicitStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbilityInner_002, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    AbilityRequest request;
    int32_t userId = 0;
    request.callType = AbilityCallType::START_SETTINGS_TYPE;
    bool res = processor->ImplicitStartAbilityInner(want, request, userId);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify ImplicitStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbilityInner_003, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    AbilityRequest request;
    int32_t userId = 0;
    request.callType = AbilityCallType::START_EXTENSION_TYPE;
    bool res = processor->ImplicitStartAbilityInner(want, request, userId);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify ImplicitStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbilityInner_004, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    AbilityRequest request;
    int32_t userId = 0;
    request.callType = AbilityCallType::CALL_REQUEST_TYPE;
    bool res = processor->ImplicitStartAbilityInner(want, request, userId);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CallStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CallStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify CallStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, CallStartAbilityInner_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    int32_t userId = 0;
    Want want;
    ImplicitStartProcessor::StartAbilityClosure callBack = []() -> int32_t {
        return 1;
    };
    AbilityCallType callType = AbilityCallType::INVALID_TYPE;
    bool res = processor->CallStartAbilityInner(userId, want, callBack, callType);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CallStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CallStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify CallStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, CallStartAbilityInner_002, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    int32_t userId = 0;
    Want want;
    ImplicitStartProcessor::StartAbilityClosure callBack = []() -> int32_t {
        return 1;
    };
    AbilityCallType callType = AbilityCallType::CALL_REQUEST_TYPE;
    bool res = processor->CallStartAbilityInner(userId, want, callBack, callType);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CallStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CallStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify CallStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, CallStartAbilityInner_003, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    int32_t userId = 0;
    Want want;
    ImplicitStartProcessor::StartAbilityClosure callBack = []() -> int32_t {
        return ERR_OK;
    };
    AbilityCallType callType = AbilityCallType::CALL_REQUEST_TYPE;
    bool res = processor->CallStartAbilityInner(userId, want, callBack, callType);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: SetTargetLinkInfo
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor SetTargetLinkInfo
 * EnvConditions: NA
 * CaseDescription: Verify SetTargetLinkInfo
 */
HWTEST_F(ImplicitStartProcessorTest, SetTargetLinkInfo_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<AppExecFwk::SkillUriForAbilityAndExtension> skillUri;
    AppExecFwk::SkillUriForAbilityAndExtension uri;
    uri.isMatch = true;
    uri.scheme = "https";
    skillUri.emplace_back(uri);
    Want want;
    want.SetParam("appLinkingOnly", true);
    processor->SetTargetLinkInfo(skillUri, want);
    int32_t targetLinkType = want.GetIntParam("send_to_erms_targetLinkType", 0);
    EXPECT_EQ(targetLinkType, 1);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: SetTargetLinkInfo
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor SetTargetLinkInfo
 * EnvConditions: NA
 * CaseDescription: Verify SetTargetLinkInfo
 */
HWTEST_F(ImplicitStartProcessorTest, SetTargetLinkInfo_002, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<AppExecFwk::SkillUriForAbilityAndExtension> skillUri;
    AppExecFwk::SkillUriForAbilityAndExtension uri;
    uri.isMatch = true;
    uri.scheme = "https";
    skillUri.emplace_back(uri);
    Want want;
    want.SetAction("ohos.want.action.viewData");
    processor->SetTargetLinkInfo(skillUri, want);
    int32_t targetLinkType = want.GetIntParam("send_to_erms_targetLinkType", 0);
    EXPECT_EQ(targetLinkType, 3);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: SetTargetLinkInfo
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor SetTargetLinkInfo
 * EnvConditions: NA
 * CaseDescription: Verify SetTargetLinkInfo
 */
HWTEST_F(ImplicitStartProcessorTest, SetTargetLinkInfo_003, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<AppExecFwk::SkillUriForAbilityAndExtension> skillUri;
    AppExecFwk::SkillUriForAbilityAndExtension uri;
    uri.isMatch = true;
    uri.scheme = "https";
    skillUri.emplace_back(uri);
    Want want;
    processor->SetTargetLinkInfo(skillUri, want);
    int32_t targetLinkType = want.GetIntParam("send_to_erms_targetLinkType", 0);
    EXPECT_EQ(targetLinkType, 2);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: MatchTypeAndUri
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor MatchTypeAndUri
 * EnvConditions: NA
 * CaseDescription: Verify MatchTypeAndUri
 */
HWTEST_F(ImplicitStartProcessorTest, MatchTypeAndUri_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    processor->MatchTypeAndUri(want);
    want.SetType("haha");
    EXPECT_EQ("haha", processor->MatchTypeAndUri(want));
    want.SetType("");
    want.SetUri("http://wwwsocom");
    EXPECT_EQ("", processor->MatchTypeAndUri(want));
    want.SetUri("http://www.so.com");
    processor->MatchTypeAndUri(want);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ProcessLinkType
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ProcessLinkType
 * EnvConditions: NA
 * CaseDescription: Verify ProcessLinkType SetUriReservedFlag SetUriReservedBundle etc.
 */
HWTEST_F(ImplicitStartProcessorTest, ProcessLinkType_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    AbilityInfo abilityInfo;
    abilityInfos.push_back(abilityInfo);
    AbilityInfo abilityInfo2;
    abilityInfo2.linkType = AppExecFwk::LinkType::APP_LINK;
    abilityInfos.push_back(abilityInfo2);

    processor->SetUriReservedFlag(true);
    processor->SetUriReservedBundle("haha");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: OnlyKeepReserveApp
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor OnlyKeepReserveApp
 * EnvConditions: NA
 * CaseDescription: Verify OnlyKeepReserveApp  etc.
 */
HWTEST_F(ImplicitStartProcessorTest, OnlyKeepReserveApp_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    processor->SetUriReservedFlag(false);
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionAbInfos;
    AppExecFwk::ExtensionAbilityInfo  extensionAbInfo;
    extensionAbInfos.push_back(extensionAbInfo);
    AppExecFwk::AbilityInfo  abilityInfo;
    abilityInfo.bundleName = "haha";
    abilityInfos.push_back(abilityInfo);
    processor->SetUriReservedBundle("haha");
    processor->OnlyKeepReserveApp(abilityInfos, extensionAbInfos);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: GetDefaultAppProxy
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor GetDefaultAppProxy
 * EnvConditions: NA
 * CaseDescription: Verify GetDefaultAppProxy  etc.
 */
HWTEST_F(ImplicitStartProcessorTest, GetDefaultAppProxy_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    processor->GetDefaultAppProxy();
}

/*
 * Feature: ImplicitStartProcessor
 * Function: FilterAbilityList
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor FilterAbilityList
 * EnvConditions: NA
 * CaseDescription: Verify FilterAbilityList  etc.
 */
HWTEST_F(ImplicitStartProcessorTest, FilterAbilityList_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionAbInfos;
    int32_t  userId = 100;
    processor->FilterAbilityList(want, abilityInfos, extensionAbInfos, userId);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: GetEcologicalCallerInfo
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor GetEcologicalCallerInfo
 * EnvConditions: NA
 * CaseDescription: Verify GetEcologicalCallerInfo  etc.
 */
HWTEST_F(ImplicitStartProcessorTest, GetEcologicalCallerInfo_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    ErmsCallerInfo callerInfo;
    int32_t  userId = 100;
    processor->GetEcologicalCallerInfo(want, callerInfo, userId);
    want.SetBundle("haha");
    processor->GetEcologicalCallerInfo(want, callerInfo, userId);
    want.SetBundle("com.ix.hiservcie");
    processor->GetEcologicalCallerInfo(want, callerInfo, userId);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: AddIdentity
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor AddIdentity
 * EnvConditions: NA
 * CaseDescription: Verify AddIdentity  ResetCallingIdentityAsCaller AddAbilityInfoToDialogInfos etc.
 */
HWTEST_F(ImplicitStartProcessorTest, AddIdentity_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    int32_t  userId = 102;
    std::string  identity;
    processor->AddIdentity(userId, identity);
    processor->ResetCallingIdentityAsCaller(userId);
    AddInfoParam param;
    std::vector<DialogAppInfo> dialogAppInfos;
    param.isExtension = true;
    param.info.type = AbilityType::FORM;
    processor->AddAbilityInfoToDialogInfos(param, dialogAppInfos);
    param.isExtension = false;
    processor->AddAbilityInfoToDialogInfos(param, dialogAppInfos);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: IsExistDefaultApp
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor IsExistDefaultApp
 * EnvConditions: NA
 * CaseDescription: Verify IsExistDefaultApp  etc.
 */
HWTEST_F(ImplicitStartProcessorTest, IsExistDefaultApp_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    int32_t  userId = 100;
}
}  // namespace AAFwk
}  // namespace OHOS
