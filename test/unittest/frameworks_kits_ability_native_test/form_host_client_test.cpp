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
    void ProcessFormUpdate(const FormJsInfo &formJsInfo)override
    {}
    void ProcessFormUninstall(const int64_t formId) override
    {}
    void OnDeathReceived() override
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
    int64_t formId = 0;
    instance_->AddForm(callback, formId);
    formId = 1;
    instance_->AddForm(callback, formId);
    formId = 2;
    instance_->AddForm(callback, formId);

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
    int64_t formId = 0;
    instance_->RemoveForm(callback, formId);
    instance_->AddForm(callback, formId);
    instance_->RemoveForm(callback, formId);

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
    formId = 1;
    instance_->AddForm(callback, formId);
    EXPECT_EQ(true, instance_->ContainsForm(formId));

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
    int64_t formId = 1;
    std::shared_ptr<FormCallbackInterfaceTest> callback = std::make_shared<FormCallbackInterfaceTest>();
    instance_->AddForm(callback, formId);
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
    int64_t formId = 1;
    std::shared_ptr<FormCallbackInterfaceTest> callback = std::make_shared<FormCallbackInterfaceTest>();
    instance_->AddForm(callback, formId);
    formInfo.formId = 1;
    instance_->OnUpdate(formInfo);

    GTEST_LOG_(INFO) << "AaFwk_FormHostClient_OnUpdate_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
