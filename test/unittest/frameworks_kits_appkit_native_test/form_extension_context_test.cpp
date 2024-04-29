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
#include "form_extension_context.h"
#undef private

#include "form_extension_context_mock_test.h"
#include "form_mgr_errors.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class FormExtensionContextTest : public testing::Test {
public:
    std::shared_ptr<AbilityRuntime::FormExtensionContext> formextension_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FormExtensionContextTest::SetUpTestCase(void)
{}

void FormExtensionContextTest::TearDownTestCase(void)
{}

void FormExtensionContextTest::SetUp(void)
{
    formextension_ = std::make_shared<AbilityRuntime::FormExtensionContext>();
}

void FormExtensionContextTest::TearDown(void)
{}

/**
 * @tc.number: formExtensionContext_UpdateForm_0100
 * @tc.name: UpdateForm
 * @tc.desc: Verify whether UpdateForm is called normally (formID is less than 0).
 */
HWTEST_F(FormExtensionContextTest, formExtensionContext_UpdateForm_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "formExtensionContext_UpdateForm_0100 start";
    FormProviderData formProviderData;
    std::string inputTestJsonStr = "{"
                                   "  \"deviceType\": \"PHONE\",          "
                                   "  \"fontScale\": 1.0 ,                 "
                                   "  \"fontScalse\": 1.0                  "
                                   "}";
    formProviderData.SetDataString(inputTestJsonStr);
    int32_t formId = -1;
    auto result = formextension_->UpdateForm(formId, formProviderData);
    EXPECT_EQ(ERR_APPEXECFWK_FORM_INVALID_PARAM, result);
    GTEST_LOG_(INFO) << "formExtensionContext_UpdateForm_0100 end";
}

/**
 * @tc.number: formExtensionContext_UpdateForm_0200
 * @tc.name: UpdateForm
 * @tc.desc: Verify whether UpdateForm is called normally (the formProviderData parameter is not set).
 */
HWTEST_F(FormExtensionContextTest, formExtensionContext_UpdateForm_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "formExtensionContext_UpdateForm_0200 start";
    FormProviderData formProviderData;
    int32_t formId = 200;
    auto result = formextension_->UpdateForm(formId, formProviderData);
    EXPECT_EQ(ERR_APPEXECFWK_FORM_INVALID_PARAM, result);
    GTEST_LOG_(INFO) << "formExtensionContext_UpdateForm_0200 end";
}

/**
 * @tc.number: formExtensionContext_UpdateForm_0300
 * @tc.name: UpdateForm
 * @tc.desc: Verify that the setting status is IN_ RECOVERING returned result execution exception.
 */
HWTEST_F(FormExtensionContextTest, formExtensionContext_UpdateForm_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "formExtensionContext_UpdateForm_0300 start";
    FormMgr::SetRecoverStatus(AppExecFwk::Constants::IN_RECOVERING);
    FormProviderData formProviderData;
    std::string inputTestJsonStr = "{"
                                   "  \"deviceType\": \"PHONE\",          "
                                   "  \"fontScale\": 1.0 ,                 "
                                   "  \"fontScalse\": 1.0                  "
                                   "}";
    formProviderData.SetDataString(inputTestJsonStr);
    int32_t formId = 200;
    auto result = formextension_->UpdateForm(formId, formProviderData);
    EXPECT_EQ(ERR_APPEXECFWK_FORM_IN_RECOVER, result);
    FormMgr::SetRecoverStatus(Constants::NOT_IN_RECOVERY);
    GTEST_LOG_(INFO) << "formExtensionContext_UpdateForm_0300 end";
}

/**
 * @tc.number: formExtensionContext_UpdateForm_0400
 * @tc.name: UpdateForm
 * @tc.desc: Verify whether calling UpdateForm normally will trigger a mock (parameters are normal).
 */
HWTEST_F(FormExtensionContextTest, formExtensionContext_UpdateForm_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "formExtensionContext_UpdateForm_0400 start";
    FormProviderData formProviderData;
    std::string inputTestJsonStr = "{"
                                   "  \"deviceType\": \"PHONE\",          "
                                   "  \"fontScale\": 1.0 ,                 "
                                   "  \"fontScalse\": 1.0                  "
                                   "}";
    formProviderData.SetDataString(inputTestJsonStr);
    int32_t formId = 200;
    sptr<MockIFormMgr> mockIFormMgr = new (std::nothrow) MockIFormMgr();
    OHOS::AppExecFwk::FormMgr::GetInstance().SetFormMgrService(mockIFormMgr);
    auto result = formextension_->UpdateForm(formId, formProviderData);
    EXPECT_EQ(ERR_OK, result);
    OHOS::AppExecFwk::FormMgr::GetInstance().SetFormMgrService(nullptr);
    GTEST_LOG_(INFO) << "formExtensionContext_UpdateForm_0400 end";
}

/**
 * @tc.number: formExtensionContext_GetAbilityInfo_0100
 * @tc.name: SetAbilityInfo/GetAbilityInfo
 * @tc.desc: Verify whether the SetAbilityInfo setting content and GetAbilityInformation get.
 */
HWTEST_F(FormExtensionContextTest, formExtensionContext_GetAbilityInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "formExtensionContext_GetAbilityInfo_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    EXPECT_EQ(formextension_->GetAbilityInfo(), nullptr);
    abilityInfo->name = "Ability";
    formextension_->SetAbilityInfo(abilityInfo);
    abilityInfo = formextension_->GetAbilityInfo();
    EXPECT_STREQ(abilityInfo->name.c_str(), "Ability");
    GTEST_LOG_(INFO) << "formExtensionContext_GetAbilityInfo_0100 end";
}

/**
 * @tc.number: formExtensionContext_GetAbilityInfo_0200
 * @tc.name: SetAbilityInfo/GetAbilityInfo
 * @tc.desc: Verify that SetAbilityInfo is set to null and GetAbilityInformation gets.
 */
HWTEST_F(FormExtensionContextTest, formExtensionContext_GetAbilityInfo_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "formExtensionContext_GetAbilityInfo_0200 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    EXPECT_EQ(formextension_->GetAbilityInfo(), nullptr);
    abilityInfo->name = "";
    formextension_->SetAbilityInfo(abilityInfo);
    abilityInfo = formextension_->GetAbilityInfo();
    EXPECT_STREQ(abilityInfo->name.c_str(), "");
    GTEST_LOG_(INFO) << "formExtensionContext_GetAbilityInfo_0200 end";
}

/**
 * @tc.number: formExtensionContext_SetAbilityInfo_0100
 * @tc.name: SetAbilityInfo
 * @tc.desc: Verify that the SetAbilityInfo parameter is null and exit without exception.
 */
HWTEST_F(FormExtensionContextTest, formExtensionContext_SetAbilityInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "formExtensionContext_SetAbilityInfo_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    formextension_->SetAbilityInfo(abilityInfo);
    EXPECT_EQ(abilityInfo, nullptr);
    GTEST_LOG_(INFO) << "formExtensionContext_SetAbilityInfo_0100 end";
}

/**
 * @tc.number: formExtensionContext_StartAbility_0100
 * @tc.name: StartAbility
 * @tc.desc: Verify that StartAbility return is not equal to ERR_OK.
 */
HWTEST_F(FormExtensionContextTest, formExtensionContext_StartAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "formExtensionContext_StartAbility_0100 start";
    AAFwk::Want want;
    ElementName element("device", "ohos.samples", "form_extension_context_test");
    want.SetElement(element);
    EXPECT_TRUE(formextension_->StartAbility(want) != ERR_OK);
    GTEST_LOG_(INFO) << "formExtensionContext_StartAbility_0100 end";
}

/**
 * @tc.number: formExtensionContext_StartAbility_0200
 * @tc.name: StartAbility
 * @tc.desc: Verify that StartAbility returns equal to ERR_OK.
 */
HWTEST_F(FormExtensionContextTest, formExtensionContext_StartAbility_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "formExtensionContext_StartAbility_0200 start";
    AAFwk::Want want;
    sptr<MockIFormMgr> mockIFormMgr = new (std::nothrow) MockIFormMgr();
    OHOS::AppExecFwk::FormMgr::GetInstance().SetFormMgrService(mockIFormMgr);
    ElementName element("device", "ohos.samples", "form_extension_context_test");
    want.SetElement(element);
    EXPECT_TRUE(formextension_->StartAbility(want) == ERR_OK);
    OHOS::AppExecFwk::FormMgr::GetInstance().SetFormMgrService(nullptr);
    GTEST_LOG_(INFO) << "formExtensionContext_StartAbility_0200 end";
}

/**
 * @tc.number: formExtensionContext_GetAbilityInfoType_0100
 * @tc.name: GetAbilityInfoType
 * @tc.desc: Verify whether GetAbilityInfoType gets the unset information.
 */
HWTEST_F(FormExtensionContextTest, formExtensionContext_GetAbilityInfoType_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "formExtensionContext_GetAbilityInfoType_0100 start";
    EXPECT_EQ(OHOS::AppExecFwk::AbilityType::UNKNOWN, formextension_->GetAbilityInfoType());
    GTEST_LOG_(INFO) << "formExtensionContext_GetAbilityInfoType_0100 end";
}

/**
 * @tc.number: formExtensionContext_GetAbilityInfoType_0200
 * @tc.name: GetAbilityInfoType
 * @tc.desc: Verify whether the setting information GetAbilityInfoType gets.
 */
HWTEST_F(FormExtensionContextTest, formExtensionContext_GetAbilityInfoType_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "formExtensionContext_GetAbilityInfoType_0200 start";
    EXPECT_EQ(OHOS::AppExecFwk::AbilityType::UNKNOWN, formextension_->GetAbilityInfoType());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = OHOS::AppExecFwk::AbilityType::SERVICE;
    formextension_->SetAbilityInfo(abilityInfo);
    EXPECT_EQ(abilityInfo->type, formextension_->GetAbilityInfoType());
    GTEST_LOG_(INFO) << "formExtensionContext_GetAbilityInfoType_0200 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS