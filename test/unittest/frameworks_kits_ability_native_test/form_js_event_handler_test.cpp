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
#include "form_js_event_handler.h"
#include "form_mgr.h"
#include "inner_event.h"
#undef private
#undef protected
#include "mock_form_mgr_proxy.h"
#include "mock_service_ability.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using testing::_;

class FormJsEventHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FormJsEventHandlerTest::SetUpTestCase(void)
{
}

void FormJsEventHandlerTest::TearDownTestCase(void)
{
}

void FormJsEventHandlerTest::SetUp(void)
{
}

void FormJsEventHandlerTest::TearDown(void)
{
}

/**
 * @tc.number: AaFwk_FormJsEventHandler_0100
 * @tc.name: Create
 * @tc.desc: Constructor method
 */
HWTEST_F(FormJsEventHandlerTest, AaFwk_FormJsEventHandler_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0100 start";
    std::shared_ptr<EventRunner> runner;
    std::shared_ptr<Ability> ability;
    FormJsInfo formJsInfo;
    auto eventHandler = std::make_shared<FormJsEventHandler>(runner, ability, formJsInfo);
    EXPECT_TRUE(eventHandler->ability_ == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0100 end";
}

/**
 * @tc.number: AaFwk_FormJsEventHandler_0200
 * @tc.name: Create
 * @tc.desc: Constructor method
 */
HWTEST_F(FormJsEventHandlerTest, AaFwk_FormJsEventHandler_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0200 start";
    std::shared_ptr<EventRunner> runner;
    std::shared_ptr<Ability> ability = std::make_shared<MockServiceAbility>();
    FormJsInfo formJsInfo;
    auto eventHandler = std::make_shared<FormJsEventHandler>(runner, ability, formJsInfo);
    EXPECT_TRUE(eventHandler->ability_ != nullptr);
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0200 end";
}

/**
 * @tc.number: AaFwk_FormJsEventHandler_0300
 * @tc.name: IsSystemApp
 * @tc.desc: Validation IsSystemApp succeeded.
 */
HWTEST_F(FormJsEventHandlerTest, AaFwk_FormJsEventHandler_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0300 start";
    std::shared_ptr<EventRunner> runner;
    std::shared_ptr<Ability> ability = std::make_shared<MockServiceAbility>();
    FormJsInfo formJsInfo;
    auto eventHandler = std::make_shared<FormJsEventHandler>(runner, ability, formJsInfo);
    auto result = eventHandler->IsSystemApp();
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0300 end";
}

/**
 * @tc.number: AaFwk_FormJsEventHandler_0400
 * @tc.name: ProcessEvent
 * @tc.desc: event is nullptr, Verify that the ProcessEvent succeeded.
 */
HWTEST_F(FormJsEventHandlerTest, AaFwk_FormJsEventHandler_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0400 start";
    std::shared_ptr<EventRunner> runner;
    std::shared_ptr<Ability> ability = std::make_shared<MockServiceAbility>();
    FormJsInfo formJsInfo;
    auto eventHandler = std::make_shared<FormJsEventHandler>(runner, ability, formJsInfo);
    InnerEvent::Pointer event = InnerEvent::Pointer(nullptr, nullptr);
    eventHandler->ProcessEvent(event);
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0400 end";
}

/**
 * @tc.number: AaFwk_FormJsEventHandler_0500
 * @tc.name: ProcessEvent
 * @tc.desc: empty event, Verify that the ProcessEvent succeeded.
 */
HWTEST_F(FormJsEventHandlerTest, AaFwk_FormJsEventHandler_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0500 start";
    std::shared_ptr<EventRunner> runner;
    std::shared_ptr<Ability> ability = std::make_shared<MockServiceAbility>();
    FormJsInfo formJsInfo;
    auto eventHandler = std::make_shared<FormJsEventHandler>(runner, ability, formJsInfo);
    InnerEvent::Pointer event = InnerEvent::Get();
    eventHandler->ProcessEvent(event);
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0500 end";
}

/**
 * @tc.number: AaFwk_FormJsEventHandler_0600
 * @tc.name: ProcessRouterEvent
 * @tc.desc: Validation ProcessRouterEvent succeeded.
 */
HWTEST_F(FormJsEventHandlerTest, AaFwk_FormJsEventHandler_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0600 start";
    std::shared_ptr<EventRunner> runner;
    std::shared_ptr<Ability> ability = std::make_shared<MockServiceAbility>();
    FormJsInfo formJsInfo;
    formJsInfo.bundleName = "bundleName";
    auto eventHandler = std::make_shared<FormJsEventHandler>(runner, ability, formJsInfo);
    Want want;
    eventHandler->ProcessRouterEvent(want);
    std::string abilityName = "formAbility";
    want.SetParam(Constants::PARAM_FORM_ABILITY_NAME_KEY, abilityName);
    eventHandler->ProcessRouterEvent(want);
    auto bundleName = want.GetElement().GetBundleName();
    EXPECT_STREQ(bundleName.c_str(), "bundleName");
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0600 end";
}

/**
 * @tc.number: AaFwk_FormJsEventHandler_0700
 * @tc.name: ProcessMessageEvent
 * @tc.desc: Validation ProcessMessageEvent succeeded.
 */
HWTEST_F(FormJsEventHandlerTest, AaFwk_FormJsEventHandler_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0700 start";
    sptr<IRemoteObject> impl = nullptr;
    sptr<MockFormMgrProxy> mockProxy = new (std::nothrow) MockFormMgrProxy(impl);
    FormMgr::GetInstance().SetFormMgrService(mockProxy);

    std::shared_ptr<EventRunner> runner;
    std::shared_ptr<Ability> ability = std::make_shared<MockServiceAbility>();
    FormJsInfo formJsInfo;
    auto eventHandler = std::make_shared<FormJsEventHandler>(runner, ability, formJsInfo);
    Want want;
    std::string formId = "1";
    want.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, formId);
    std::string message = "message";
    want.SetParam(Constants::PARAM_MESSAGE_KEY, message);

    EXPECT_CALL(*mockProxy, MessageEvent(_, _, _)).Times(1);
    eventHandler->ProcessMessageEvent(want);
    testing::Mock::AllowLeak(mockProxy);
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0700 end";
}

/**
 * @tc.number: AaFwk_FormJsEventHandler_0800
 * @tc.name: ProcessMessageEvent
 * @tc.desc: There is no formId in want, Validation ProcessMessageEvent failed.
 */
HWTEST_F(FormJsEventHandlerTest, AaFwk_FormJsEventHandler_0800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0800 start";
    std::shared_ptr<EventRunner> runner;
    std::shared_ptr<Ability> ability = std::make_shared<MockServiceAbility>();
    FormJsInfo formJsInfo;
    auto eventHandler = std::make_shared<FormJsEventHandler>(runner, ability, formJsInfo);
    Want want;
    eventHandler->ProcessMessageEvent(want);
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0800 end";
}

/**
 * @tc.number: AaFwk_FormJsEventHandler_0900
 * @tc.name: ProcessMessageEvent
 * @tc.desc: form id is equal to 0, Validation ProcessMessageEvent failed.
 */
HWTEST_F(FormJsEventHandlerTest, AaFwk_FormJsEventHandler_0900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0900 start";
    std::shared_ptr<EventRunner> runner;
    std::shared_ptr<Ability> ability = std::make_shared<MockServiceAbility>();
    FormJsInfo formJsInfo;
    auto eventHandler = std::make_shared<FormJsEventHandler>(runner, ability, formJsInfo);
    Want want;
    std::string formId = "0";
    want.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, formId);
    eventHandler->ProcessMessageEvent(want);
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_0900 end";
}

/**
 * @tc.number: AaFwk_FormJsEventHandler_1000
 * @tc.name: ProcessMessageEvent
 * @tc.desc: There is no PARAM_MESSAGE_KEY in want, Validation ProcessMessageEvent failed.
 */
HWTEST_F(FormJsEventHandlerTest, AaFwk_FormJsEventHandler_1000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_1000 start";
    std::shared_ptr<EventRunner> runner;
    std::shared_ptr<Ability> ability = std::make_shared<MockServiceAbility>();
    FormJsInfo formJsInfo;
    auto eventHandler = std::make_shared<FormJsEventHandler>(runner, ability, formJsInfo);
    Want want;
    std::string formId = "1";
    want.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, formId);
    eventHandler->ProcessMessageEvent(want);
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_1000 end";
}

/**
 * @tc.number: AaFwk_FormJsEventHandler_1100
 * @tc.name: ProcessMessageEvent
 * @tc.desc: Form mgr set IN_RECOVERING, Validation ProcessMessageEvent failed.
 */
HWTEST_F(FormJsEventHandlerTest, AaFwk_FormJsEventHandler_1100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_1100 start";
    std::shared_ptr<EventRunner> runner;
    std::shared_ptr<Ability> ability = std::make_shared<MockServiceAbility>();
    FormJsInfo formJsInfo;
    auto eventHandler = std::make_shared<FormJsEventHandler>(runner, ability, formJsInfo);
    Want want;
    std::string formId = "1";
    want.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, formId);
    std::string message = "message";
    want.SetParam(Constants::PARAM_MESSAGE_KEY, message);

    FormMgr::GetInstance().SetRecoverStatus(Constants::IN_RECOVERING);
    eventHandler->ProcessMessageEvent(want);
    GTEST_LOG_(INFO) << "AaFwk_FormJsEventHandler_1100 end";
}
} // namespace AppExecFwk
} // namespace OHOS