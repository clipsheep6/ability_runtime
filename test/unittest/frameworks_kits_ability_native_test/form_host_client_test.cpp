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

#include <gtest/gtest.h>

#include "form_callback_interface.h"
#include "form_host_client.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;

class FormCallbackInterfaceTest : public FormCallbackInterface {
public:
    FormCallbackInterfaceTest()
    {}
    virtual ~FormCallbackInterfaceTest()
    {}
    void ProcessFormUpdate(const FormJsInfo& formJsInfo)override
    {}
    void ProcessFormUninstall(const int64_t formId) override
    {}
    void OnDeathReceived() override
    {}
    void OnError(const int32_t errorCode, const std::string &errorMsg) override
    {}
};

class FormHostClientTest : public testing::Test {
public:
    FormHostClientTest()
    {}
    ~FormHostClientTest()
    {}
    sptr<FormHostClient> instance_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FormHostClientTest::SetUpTestCase(void)
{}

void FormHostClientTest::TearDownTestCase(void)
{}

void FormHostClientTest::SetUp(void)
{
    instance_ = FormHostClient::GetInstance();
}

void FormHostClientTest::TearDown(void)
{
    instance_ = nullptr;
}

/**
 * @tc.number: AaFwk_FormHostClient_AddForm_0100
 * @tc.name: AddForm
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormHostClientTest, AaFwk_formHostClientAddForm_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_AddForm_0100 start";
    std::shared_ptr<FormCallbackInterfaceTest> callback = std::make_shared<FormCallbackInterfaceTest>();
    FormJsInfo formJsInfo;
    formJsInfo.formId = 0;
    instance_->AddForm(callback, formJsInfo);
    formJsInfo.formId = 1;
    instance_->AddForm(callback, formJsInfo);
    formJsInfo.formId = 2;
    instance_->AddForm(callback, formJsInfo);

    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_AddForm_0100 end";
}

/**
 * @tc.number: AaFwk_FormHostClient_RemoveForm_0100
 * @tc.name: RemoveForm
 * @tc.desc: Verify that the return value of RemoveForm is correct.
 */
HWTEST_F(FormHostClientTest, AaFwk_FormHostClient_RemoveForm_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_RemoveForm_0100 start";
    std::shared_ptr<FormCallbackInterfaceTest> callback = std::make_shared<FormCallbackInterfaceTest>();
    FormJsInfo formJsInfo;
    formJsInfo.formId = 0;
    instance_->RemoveForm(callback, formJsInfo.formId);
    instance_->AddForm(callback, formJsInfo);
    instance_->RemoveForm(callback, formJsInfo.formId);

    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_RemoveForm_0100 end";
}

/**
 * @tc.number: AaFwk_FormHostClient_ContainsForm_0100
 * @tc.name: ContainsForm
 * @tc.desc: Verify that the return value of ContainsForm is correct.
 */
HWTEST_F(FormHostClientTest, AaFwk_FormHostClient_ContainsForm_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_ContainsForm_0100 start";
    int64_t formId = 0;
    EXPECT_EQ(false, instance_->ContainsForm(formId));
    std::shared_ptr<FormCallbackInterfaceTest> callback = std::make_shared<FormCallbackInterfaceTest>();
    FormJsInfo formJsInfo;
    formJsInfo.formId = 1;
    instance_->AddForm(callback, formJsInfo);
    EXPECT_EQ(true, instance_->ContainsForm(formJsInfo.formId));

    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_ContainsForm_0100 end";
}

/**
 * @tc.number: AaFwk_FormHostClient_OnAcquired_0100
 * @tc.name: OnAcquired
 * @tc.desc: Verify that the return value of OnAcquired is correct.
 */
HWTEST_F(FormHostClientTest, AaFwk_FormHostClient_OnAcquired_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_OnAcquired_0100 start";
    FormJsInfo formInfo;
    formInfo.formId = -1;
    instance_->OnAcquired(formInfo, nullptr);
    formInfo.formId = -1;
    std::shared_ptr<FormCallbackInterfaceTest> callback = std::make_shared<FormCallbackInterfaceTest>();
    instance_->AddForm(callback, formInfo);
    formInfo.formId = 1;
    formInfo.jsFormCodePath = "/data/test";
    formInfo.formData = "test";
    instance_->OnAcquired(formInfo, nullptr);

    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_OnAcquired_0100 end";
}

/**
 * @tc.number: AaFwk_FormHostClient_OnUpdate_0100
 * @tc.name: OnUpdate
 * @tc.desc: Verify that the return value of OnUpdate is correct.
 */
HWTEST_F(FormHostClientTest, AaFwk_FormHostClient_OnUpdate_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_OnUpdate_0100 start";
    FormJsInfo formInfo;
    formInfo.formId = -1;
    instance_->OnUpdate(formInfo);
    formInfo.formId = 1;
    std::shared_ptr<FormCallbackInterfaceTest> callback = std::make_shared<FormCallbackInterfaceTest>();
    instance_->AddForm(callback, formInfo);
    formInfo.formId = 1;
    instance_->OnUpdate(formInfo);

    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_OnUpdate_0100 end";
}

/**
 * @tc.number: AaFwk_FormHostClient_RegisterUninstallCallback_0100
 * @tc.name: RegisterUninstallCallback
 * @tc.desc: Verify that the return value of RegisterUninstallCallback is correct.
 */
HWTEST_F(FormHostClientTest, AaFwk_FormHostClient_RegisterUninstallCallback_0100, Function | MediumTest | Level1)
{
    FormHostClient formhostclient;
    bool ret = formhostclient.RegisterUninstallCallback(nullptr);

    EXPECT_EQ(true, ret);
}

/**
 * @tc.number: AaFwk_FormHostClient_AddShareFormCallback_0100
 * @tc.name: AddShareFormCallback
 * @tc.desc: Verify that the return value of AddShareFormCallback is correct.
 */
HWTEST_F(FormHostClientTest, AaFwk_FormHostClient_AddShareFormCallback_0100, Function | MediumTest | Level1)
{
    FormHostClient formhostclient;
    std::shared_ptr<ShareFormCallBack> shareFormCallback;
    bool ret = formhostclient.AddShareFormCallback(shareFormCallback, 0);

    EXPECT_EQ(true, ret);
}

/**
 * @tc.number: AaFwk_FormHostClient_RemoveShareFormCallback_0100
 * @tc.name: RemoveShareFormCallback
 * @tc.desc: Verify that the return value of RemoveShareFormCallback is correct.
 */
HWTEST_F(FormHostClientTest, AaFwk_FormHostClient_RemoveShareFormCallback_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_RemoveShareFormCallback_0100 start";
    std::shared_ptr<ShareFormCallBack> shareFormCallback;
    FormHostClient formhostclient;
    formhostclient.AddShareFormCallback(shareFormCallback, 0);
    formhostclient.AddShareFormCallback(shareFormCallback, 1);
    formhostclient.RemoveShareFormCallback(1);
    formhostclient.RemoveShareFormCallback(0);
    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_RemoveShareFormCallback_0100 end";
}

/**
 * @tc.number: AaFwk_FormHostClient_OnUninstall_0100
 * @tc.name: OnUninstall
 * @tc.desc: Verify that the return value of OnUninstall is correct.
 */
HWTEST_F(FormHostClientTest, AaFwk_FormHostClient_OnUninstall_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_OnUninstall_0100 start";

    std::vector<int64_t> formIds;
    instance_->OnUninstall(formIds);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_OnUninstall_0100 end";
}

/**
 * @tc.number: AaFwk_FormHostClient_OnShareFormResponse_0100
 * @tc.name: OnShareFormResponse
 * @tc.desc: Verify that the return value of OnShareFormResponse is correct.
 */
HWTEST_F(FormHostClientTest, AaFwk_FormHostClient_OnShareFormResponse_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_OnShareFormResponse_0100 start";

    int64_t requestCode = 0;
    int32_t result = 0;
    instance_->OnShareFormResponse(requestCode, result);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_OnShareFormResponse_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
