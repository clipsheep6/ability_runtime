/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#define private public
#define protected public
#include <gtest/gtest.h>

#include "accesstoken_kit.h"
#include "appexecfwk_errors.h"
#include "form_constants.h"
#include "form_mgr_errors.h"
#include "form_provider_client.h"
#include "mock_form_supply_callback.h"
#include "want_params.h"
#undef private
#undef protected
namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Security;

const std::string PERMISSION_NAME_REQUIRE_FORM = "ohos.permission.REQUIRE_FORM";
const std::string FORM_MANAGER_SERVICE_BUNDLE_NAME = "com.form.fms.app";
const std::string DEF_LABEL1 = "PermissionFormRequireGrant";
const std::string FORM_SUPPLY_INFO = "com.form.supply.info.test";

class FormProviderClientTest : public testing::Test {
public:
    FormProviderClientTest()
    {}
    ~FormProviderClientTest()
    {}
    sptr<FormProviderClient> instance_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FormProviderClientTest::SetUpTestCase(void)
{}

void FormProviderClientTest::TearDownTestCase(void)
{}

void FormProviderClientTest::SetUp(void)
{
    instance_ = new FormProviderClient();

    // Permission install
    int userId = 0;
    auto tokenId = AccessToken::AccessTokenKit::GetHapTokenID(userId, FORM_MANAGER_SERVICE_BUNDLE_NAME, 0);
    auto flag = OHOS::Security::AccessToken::PERMISSION_USER_FIXED;
    AccessToken::AccessTokenKit::GrantPermission(tokenId, PERMISSION_NAME_REQUIRE_FORM, flag);
}

void FormProviderClientTest::TearDown(void)
{
    instance_ = nullptr;
}

/**
 * @tc.number: AaFwk_FormProviderClient_AcquireProviderFormInfo_0100
 * @tc.name: AcquireProviderFormInfo
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireProviderFormInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0100 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);
    instance_->SetOwner(ability);

    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();
    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 100)
        .SetParam(Constants::FORM_CONNECT_ID, 100L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 100L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);
    FormJsInfo formJsInfo;
    formJsInfo.formId = 1001L;
    instance_->AcquireProviderFormInfo(formJsInfo, want, callerToken);

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_AcquireProviderFormInfo_0200
 * @tc.name: AcquireProviderFormInfo
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireProviderFormInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0200 start";

    instance_->SetOwner(nullptr);

    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();
    ASSERT_NE(callerToken, nullptr);
    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 200)
        .SetParam(Constants::FORM_CONNECT_ID, 200L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 200L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    FormJsInfo formJsInfo;
    formJsInfo.formId = 1002L;
    instance_->AcquireProviderFormInfo(formJsInfo, want, callerToken);

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0200 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_AcquireProviderFormInfo_0300
 * @tc.name: AcquireProviderFormInfo
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireProviderFormInfo_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0300 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);
    instance_->SetOwner(ability);

    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();
    Want want;
    want.SetParam(Constants::ACQUIRE_TYPE, 300)
        .SetParam(Constants::FORM_CONNECT_ID, 300L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 300L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    FormJsInfo formJsInfo;
    formJsInfo.formId = 1003L;
    instance_->AcquireProviderFormInfo(formJsInfo, want, callerToken);

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0300 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_AcquireProviderFormInfo_0400
 * @tc.name: AcquireProviderFormInfo
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireProviderFormInfo_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0400 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ASSERT_NE(ability, nullptr);
    instance_->SetOwner(ability);
    instance_->ClearOwner(ability);

    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();
    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 400)
        .SetParam(Constants::FORM_CONNECT_ID, 400L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 400L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    FormJsInfo formJsInfo;
    formJsInfo.formId = 1004L;
    instance_->AcquireProviderFormInfo(formJsInfo, want, callerToken);

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0400 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_AcquireProviderFormInfo_0500
 * @tc.name: AcquireProviderFormInfo
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireProviderFormInfo_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0500 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 500)
        .SetParam(Constants::FORM_CONNECT_ID, 500L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 500L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    const sptr<IRemoteObject> callerToken = nullptr;
    FormJsInfo formJsInfo;
    formJsInfo.formId = 1005L;
    EXPECT_EQ(ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED,
        instance_->AcquireProviderFormInfo(formJsInfo, want, callerToken));
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0500 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_EventNotify_0100
 * @tc.name: EventNotify
 * @tc.desc: Verify that the return value of EventNotify is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_EventNotify_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_EventNotify_0100 start";

    // Ability
    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    // callerToken
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    // want
    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 100)
        .SetParam(Constants::FORM_CONNECT_ID, 100L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 100L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    // formEvents
    int64_t formId = 100;
    std::vector<int64_t> formEvents;
    formEvents.push_back(formId);

    int32_t formVisibleType = Constants::FORM_INVISIBLE;

    EXPECT_EQ(ERR_OK, instance_->EventNotify(formEvents, formVisibleType, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_EventNotify_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_EventNotify_0200
 * @tc.name: EventNotify
 * @tc.desc: Verify that the return value of EventNotify is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_EventNotify_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_EventNotify_0200 start";

    // Ability is nullptr
    instance_->SetOwner(nullptr);

    // callerToken
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    // want
    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 100)
        .SetParam(Constants::FORM_CONNECT_ID, 100L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 100L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    // formEvents
    int64_t formId = 100;
    std::vector<int64_t> formEvents;
    formEvents.push_back(formId);

    int32_t formVisibleType = Constants::FORM_INVISIBLE;

    EXPECT_EQ(ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY,
        instance_->EventNotify(formEvents, formVisibleType, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_EventNotify_0200 end";
}


/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormCastTempForm_0100
 * @tc.name: NotifyFormCastTempForm
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormCastTempForm_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormCastTempForm_0100 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 101)
        .SetParam(Constants::FORM_CONNECT_ID, 101L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 101L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    int64_t formId = 720L;
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    EXPECT_EQ(ERR_OK, instance_->NotifyFormCastTempForm(formId, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormCastTempForm_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormCastTempForm_0100
 * @tc.name: NotifyFormCastTempForm
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormCastTempForm_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormCastTempForm_0200 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(nullptr);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 102)
        .SetParam(Constants::FORM_CONNECT_ID, 102L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 102L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    int64_t formId = 722L;
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    EXPECT_EQ(ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY, instance_->NotifyFormCastTempForm(formId, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormCastTempForm_0200 end";
}


/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormCastTempForm_0100
 * @tc.name: NotifyFormCastTempForm
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormCastTempForm_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormCastTempForm_0300 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 103)
        .SetParam(Constants::FORM_CONNECT_ID, 103L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 103L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    int64_t formId = 723L;
    const sptr<IRemoteObject> callerToken = nullptr;

    EXPECT_EQ(ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED, instance_->NotifyFormCastTempForm(formId, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormCastTempForm_0300 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_FireFormEvent_0100
 * @tc.name: FireFormEvent
 * @tc.desc: Verify that the return value of FireFormEvent is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_FireFormEvent_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_FireFormEvent_0100 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::FORM_CONNECT_ID, 104L);

    int64_t formId = 724L;
    std::string message = "event message";
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    EXPECT_EQ(ERR_OK, instance_->FireFormEvent(formId, message, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_FireFormEvent_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_FireFormEvent_0200
 * @tc.name: FireFormEvent
 * @tc.desc: Verify that the return value of FireFormEvent is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_FireFormEvent_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_FireFormEvent_0200 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(nullptr);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::FORM_CONNECT_ID, 105L);
    int64_t formId = 725L;
    std::string message = "event message";
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    EXPECT_EQ(ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY, instance_->FireFormEvent(formId, message, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_FireFormEvent_0200 end";
}
/**
 * @tc.number: AaFwk_FormProviderClient_FireFormEvent_0300
 * @tc.name: NotifyFormCastTempForm
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_FireFormEvent_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_FireFormEvent_0300 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::FORM_CONNECT_ID, 106L);

    int64_t formId = 726L;
    std::string message = "event message";
    const sptr<IRemoteObject> callerToken = nullptr;

    EXPECT_EQ(ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED, instance_->FireFormEvent(formId, message, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_FireFormEvent_0300 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormsDelete_0100
 * @tc.name: FireFormEvent
 * @tc.desc: Verify that the return value of FireFormEvent is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormsDelete_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormsDelete_0100 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::FORM_CONNECT_ID, 107L);

    std::vector<int64_t> formIds = { 727L, 728L, 729L };
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    EXPECT_EQ(ERR_OK, instance_->NotifyFormsDelete(formIds, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormsDelete_0100 end";
}
/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormsDelete_0200
 * @tc.name: FireFormEvent
 * @tc.desc: Verify that the return value of FireFormEvent is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormsDelete_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormsDelete_0200 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(nullptr);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::FORM_CONNECT_ID, 108L);
    std::vector<int64_t> formIds = { 730L, 731L, 732L };
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    EXPECT_EQ(ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY, instance_->NotifyFormsDelete(formIds, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormsDelete_0200 end";
}
/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormsDelete_0300
 * @tc.name: NotifyFormCastTempForm
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormsDelete_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormsDelete_0300 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::FORM_CONNECT_ID, 109L);

    std::vector<int64_t> formIds = { 730L, 731L, 732L };
    const sptr<IRemoteObject> callerToken = nullptr;

    EXPECT_EQ(ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED, instance_->NotifyFormsDelete(formIds, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormsDelete_0300 end";
}
/**
 * @tc.name: AaFwk_FormProviderClient_AcquireShareFormData_0100
 * @tc.desc: Verify that the return value of AcquireShareFormData is correct.
 * @tc.type: AcquireShareFormData
 * @tc.require: issueI581YL
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireShareFormData_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireShareFormData_0100 start";

    const int64_t formId = 10;
    const int64_t requestCode = 1;
    const std::string remoteDeviceId = "15010038475446345206751dbcda572b";
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    EXPECT_TRUE((ERR_OK == instance_->AcquireShareFormData(formId, remoteDeviceId, callerToken, requestCode)));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireShareFormData_0100 end";
}
/**
 * @tc.name: AaFwk_FormProviderClient_AcquireShareFormData_0200
 * @tc.desc: Verify that the return value of AcquireShareFormData is correct.
 * @tc.type: AcquireShareFormData
 * @tc.require: issueI581YL
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireShareFormData_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireShareFormData_0200 start";

    const int64_t formId = 0;
    const int64_t requestCode = 1;
    const std::string remoteDeviceId = "15010038475446345206751dbcda572b";
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    EXPECT_TRUE((ERR_OK != instance_->AcquireShareFormData(formId, remoteDeviceId, callerToken, requestCode)));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireShareFormData_0200 end";
}
/**
 * @tc.name: AaFwk_FormProviderClient_AcquireShareFormData_0300
 * @tc.desc: Verify that the return value of AcquireShareFormData is correct.
 * @tc.type: AcquireShareFormData
 * @tc.require: issueI581YL
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireShareFormData_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireShareFormData_0300 start";

    const int64_t formId = 10;
    const int64_t requestCode = 1;
    const std::string remoteDeviceId = "";
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    EXPECT_TRUE((ERR_OK != instance_->AcquireShareFormData(formId, remoteDeviceId, callerToken, requestCode)));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireShareFormData_0300 end";
}
/**
 * @tc.name: AaFwk_FormProviderClient_AcquireShareFormData_0400
 * @tc.desc: Verify that the return value of AcquireShareFormData is correct.
 * @tc.type: AcquireShareFormData
 * @tc.require: issueI581YL
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireShareFormData_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireShareFormData_0400 start";

    const int64_t formId = 10;
    const int64_t requestCode = 1;
    const std::string remoteDeviceId = "15010038475446345206751dbcda572b";
    const sptr<IRemoteObject> callerToken = nullptr;

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    EXPECT_TRUE((ERR_OK != instance_->AcquireShareFormData(formId, remoteDeviceId, callerToken, requestCode)));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireShareFormData_0400 end";
}
/**
 * @tc.name: AaFwk_FormProviderClient_AcquireShareFormData_0500
 * @tc.desc: Verify that the return value of AcquireShareFormData is correct.
 * @tc.type: AcquireShareFormData
 * @tc.require: issueI581YL
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireShareFormData_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireShareFormData_0500 start";

    const int64_t formId = 10;
    const int64_t requestCode = 0;
    const std::string remoteDeviceId = "15010038475446345206751dbcda572b";
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();;

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    EXPECT_TRUE((ERR_OK != instance_->AcquireShareFormData(formId, remoteDeviceId, callerToken, requestCode)));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireShareFormData_0500 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormDelete_0100
 * @tc.name: NotifyFormDelete
 * @tc.desc: Verify that function NotifyFormDelete.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormDelete_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormDelete_0100 start";
    int64_t formId = 10;
    Want want;
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);
    want.SetParam(Constants::PARAM_FORM_HOST_TOKEN, FORM_MANAGER_SERVICE_BUNDLE_NAME);
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = instance_->NotifyFormDelete(formId, want, callerToken);
    EXPECT_EQ(result, ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormDelete_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormDelete_0200
 * @tc.name: NotifyFormDelete
 * @tc.desc: Verify that function NotifyFormDelete.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormDelete_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormDelete_0200 start";
    int64_t formId = 10;
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = instance_->NotifyFormDelete(formId, want, callerToken);
    EXPECT_EQ(result, ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormDelete_0200 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormDelete_0300
 * @tc.name: NotifyFormDelete
 * @tc.desc: Verify that function NotifyFormDelete.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormDelete_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormDelete_0300 start";
    int64_t formId = 10;
    Want want;
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);
    sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();
    auto result = instance_->NotifyFormDelete(formId, want, callerToken);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormDelete_0300 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormUpdate_0100
 * @tc.name: NotifyFormUpdate
 * @tc.desc: Verify that function NotifyFormUpdate.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormUpdate_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormUpdate_0100 start";
    int64_t formId = 10;
    Want want;
    WantParams wantParams;
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);
    sptr<IRemoteObject> callerToken = nullptr;
    wantParams.params_.emplace(Constants::FORM_CONNECT_ID, nullptr);
    auto result = instance_->NotifyFormUpdate(formId, want, callerToken);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormUpdate_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormUpdate_0200
 * @tc.name: NotifyFormUpdate
 * @tc.desc: Verify that function NotifyFormUpdate.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormUpdate_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormUpdate_0200 start";
    int64_t formId = 10;
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = instance_->NotifyFormUpdate(formId, want, callerToken);
    EXPECT_EQ(result, ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormUpdate_0200 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormUpdate_0300
 * @tc.name: NotifyFormUpdate
 * @tc.desc: Verify that function NotifyFormUpdate.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormUpdate_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormUpdate_0300 start";
    int64_t formId = 10;
    Want want;
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);
    sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();
    auto result = instance_->NotifyFormUpdate(formId, want, callerToken);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormUpdate_0300 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_CheckIsSystemApp_0100
 * @tc.name: CheckIsSystemApp
 * @tc.desc: Verify that function CheckIsSystemApp.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_CheckIsSystemApp_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_CheckIsSystemApp_0100 start";
    auto result = instance_->CheckIsSystemApp();
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_CheckIsSystemApp_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_AcquireState_0100
 * @tc.name: AcquireState
 * @tc.desc: Verify that function AcquireState.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireState_0100 start";
    Want wantArg;
    std::string provider;
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = instance_->AcquireState(wantArg, provider, want, callerToken);
    EXPECT_EQ(result, ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireState_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_AcquireState_0200
 * @tc.name: AcquireState
 * @tc.desc: Verify that function AcquireState.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireState_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireState_0200 start";
    Want wantArg;
    std::string provider;
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);
    auto result = instance_->AcquireState(wantArg, provider, want, callerToken);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireState_0200 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_ClearOwner_0200
 * @tc.name: ClearOwner
 * @tc.desc: Verify that function ClearOwner.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_ClearOwner_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_ClearOwner_0200 start";
    instance_->ClearOwner(nullptr);
    EXPECT_TRUE(instance_->GetOwner() == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_ClearOwner_0200 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_GetOwner_0100
 * @tc.name: GetOwner
 * @tc.desc: Verify that function GetOwner.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_GetOwner_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_GetOwner_0100 start";
    auto result = instance_->GetOwner();
    EXPECT_TRUE(result == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_GetOwner_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_GetOwner_0200
 * @tc.name: GetOwner
 * @tc.desc: Verify that function GetOwner.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_GetOwner_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_GetOwner_0200 start";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);
    auto result = instance_->GetOwner();
    EXPECT_TRUE(result != nullptr);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_GetOwner_0200 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_HandleAcquire_0100
 * @tc.name: HandleAcquire
 * @tc.desc: Verify that function HandleAcquire.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_HandleAcquire_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_HandleAcquire_0100 start";
    FormProviderInfo formProviderInfo;
    Want newWant;
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = instance_->HandleAcquire(formProviderInfo, newWant, callerToken);
    EXPECT_EQ(result, ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_HandleAcquire_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_HandleAcquire_0200
 * @tc.name: HandleAcquire
 * @tc.desc: Verify that function HandleAcquire.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_HandleAcquire_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_HandleAcquire_0200 start";
    FormProviderInfo formProviderInfo;
    Want newWant;
    sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();
    auto result = instance_->HandleAcquire(formProviderInfo, newWant, callerToken);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_HandleAcquire_0200 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_DCRtnHelper_0100
 * @tc.name: DCRtnHelper
 * @tc.desc: Verify that function DCRtnHelper.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_DCRtnHelper_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_DCRtnHelper_0100 start";
    int32_t errCode = ERR_NO_MEMORY;
    Want Want;
    sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();
    auto result = instance_->DCRtnHelper(errCode, Want, callerToken);
    EXPECT_EQ(result, ERR_NO_MEMORY);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_DCRtnHelper_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_DCRtnHelper_0200
 * @tc.name: DCRtnHelper
 * @tc.desc: Verify that function DCRtnHelper.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_DCRtnHelper_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_DCRtnHelper_0200 start";
    int32_t errCode = ERR_OK;
    Want Want;
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = instance_->DCRtnHelper(errCode, Want, callerToken);
    EXPECT_EQ(result, ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_DCRtnHelper_0200 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_HandleDisconnect_0100
 * @tc.name: HandleDisconnect
 * @tc.desc: Verify that function HandleDisconnect.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_HandleDisconnect_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_HandleDisconnect_0100 start";
    Want Want;
    sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();
    auto result = instance_->HandleDisconnect(Want, callerToken);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_HandleDisconnect_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_HandleAcquireStateResult_0100
 * @tc.name: HandleAcquireStateResult
 * @tc.desc: Verify that function HandleAcquireStateResult.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_HandleAcquireStateResult_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_HandleAcquireStateResult_0100 start";
    FormState state = FormState::UNKNOWN;
    Want wantArg;
    Want Want;
    std::string str;
    sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();
    auto result = instance_->HandleAcquireStateResult(state, str, wantArg, Want, callerToken);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_HandleAcquireStateResult_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_HandleAcquireStateResult_0200
 * @tc.name: HandleAcquireStateResult
 * @tc.desc: Verify that function HandleAcquireStateResult.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_HandleAcquireStateResult_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_HandleAcquireStateResult_0200 start";
    FormState state = FormState::UNKNOWN;
    Want wantArg;
    Want Want;
    std::string str;
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = instance_->HandleAcquireStateResult(state, str, wantArg, Want, callerToken);
    EXPECT_EQ(result, ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_HandleAcquireStateResult_0200 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_HandleRemoteAcquire_0100
 * @tc.name: HandleRemoteAcquire
 * @tc.desc: Verify that function HandleRemoteAcquire.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_HandleRemoteAcquire_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_HandleRemoteAcquire_0100 start";
    FormJsInfo formJsInfo;
    FormProviderInfo formProviderInfo;
    Want Want;
    sptr<IRemoteObject> Token = nullptr;
    auto result = Want.GetRemoteObject(Constants::PARAM_FORM_HOST_TOKEN);
    instance_->HandleRemoteAcquire(formJsInfo, formProviderInfo, Want, Token);
    EXPECT_TRUE(result == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_HandleRemoteAcquire_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_HandleRemoteAcquire_0200
 * @tc.name: HandleRemoteAcquire
 * @tc.desc: Verify that function HandleRemoteAcquire.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_HandleRemoteAcquire_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_HandleRemoteAcquire_0200 start";
    FormJsInfo formJsInfo;
    FormProviderInfo formProviderInfo;
    Want Want;
    WantParams wantParams;
    sptr<IInterface> iInterface = nullptr;
    wantParams.params_.emplace(TYPE_PROPERTY, iInterface);
    wantParams.params_.emplace(VALUE_PROPERTY, iInterface);
    wantParams.SetParam(TYPE_PROPERTY, iInterface);
    sptr<IRemoteObject> Token = MockFormSupplyCallback::GetInstance();
    Want.SetParam(Constants::PARAM_FORM_HOST_TOKEN, FORM_MANAGER_SERVICE_BUNDLE_NAME);
    auto result = Want.GetRemoteObject(Constants::PARAM_FORM_HOST_TOKEN);
    instance_->HandleRemoteAcquire(formJsInfo, formProviderInfo, Want, Token);
    EXPECT_TRUE(result == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_HandleRemoteAcquire_0200 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_IsCallBySelfBundle_0100
 * @tc.name: IsCallBySelfBundle
 * @tc.desc: Verify that function IsCallBySelfBundle.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_IsCallBySelfBundle_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_IsCallBySelfBundle_0100 start";
    auto result = instance_->IsCallBySelfBundle();
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_IsCallBySelfBundle_0100 end";
}
} // namespace AppExecFwk
} // namespace OHOS
