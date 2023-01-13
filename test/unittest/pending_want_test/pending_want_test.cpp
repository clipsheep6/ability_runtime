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
#include "ability_manager_client.h"
#include "ability_runtime_error_util.h"
#include "cancel_listener.h"
#include "completed_callback.h"
#include "completed_dispatcher.h"
#include "context/application_context.h"
#include "context_container.h"
#include "context_impl.h"
#include "element_name.h"
#include "event_handler.h"
#include "iservice_registry.h"
#include "base_types.h"
#define private public
#define protected public
#include "pending_want.h"
#undef private
#undef protected
#include "pending_want_record.h"
#include "remote_native_token.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "want.h"
#include "wants_info.h"
#include "want_params.h"
#include "want_receiver_stub.h"
#include "want_agent_helper.h"
#include "want_sender_info.h"
#include "want_sender_stub.h"
#include "bool_wrapper.h"
#include "zchar_wrapper.h"
#include "byte_wrapper.h"
#include "double_wrapper.h"
#include "float_wrapper.h"
#include "int_wrapper.h"
#include "long_wrapper.h"
#include "short_wrapper.h"
#include "string_wrapper.h"
#include "array_wrapper.h"
#include "hilog_wrapper.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS;
using OHOS::AppExecFwk::ElementName;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;
using vector_str = std::vector<std::string>;

namespace OHOS::AbilityRuntime::WantAgent {
namespace {
    constexpr int32_t NOTEQ = -1;
}
class PendingWantTest : public testing::Test {
public:
    PendingWantTest()
    {}
    ~PendingWantTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static Want MakeWant(std::string deviceId, std::string abilityName, std::string bundleName);

    static int callBackCancelListenerConnt;

    class CancelListenerSon : public CancelListener {
    public:
        void OnCancelled(int resultCode) override;
    };
};

int PendingWantTest::callBackCancelListenerConnt = 0;

void PendingWantTest::CancelListenerSon::OnCancelled(int resultCode)
{
    callBackCancelListenerConnt++;
}

Want PendingWantTest::MakeWant(std::string deviceId, std::string abilityName, std::string bundleName)
{
    ElementName element(deviceId, bundleName, abilityName);
    Want want;
    want.SetElement(element);
    return want;
}

std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> GetAppContext()
{
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    return context;
}

void PendingWantTest::SetUpTestCase(void)
{
    RemoteNativeToken::SetNativeToken();
}

void PendingWantTest::TearDownTestCase(void)
{}

void PendingWantTest::SetUp(void)
{}

void PendingWantTest::TearDown(void)
{}

/*
 * @tc.number    : PendingWant_0100
 * @tc.name      : PendingWant Constructors
 * @tc.desc      : 1.The parameter is nullptr
 */
HWTEST_F(PendingWantTest, PendingWant_0100, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    EXPECT_EQ(pendingWant.target_, nullptr);
    EXPECT_EQ(pendingWant.cancelReceiver_, nullptr);
}

/*
 * @tc.number    : PendingWant_0200
 * @tc.name      : PendingWant Constructors
 * @tc.desc      : 1.The parameter target is not nullptr
 */
HWTEST_F(PendingWantTest, PendingWant_0200, Function | MediumTest | Level1)
{
    sptr<AAFwk::IWantSender> target(new (std::nothrow) PendingWantRecord());
    PendingWant pendingWant(target);
    EXPECT_EQ(pendingWant.target_, target);
    EXPECT_EQ(pendingWant.cancelReceiver_, nullptr);
}

/*
 * @tc.number    : PendingWant_0300
 * @tc.name      : PendingWant GetType
 * @tc.desc      : 1.Get PendingWant Type (UNKNOWN_TYPE)
 */
HWTEST_F(PendingWantTest, PendingWant_0300, Function | MediumTest | Level1)
{
    sptr<AAFwk::IWantSender> target = nullptr;
    PendingWant pendingWant(target);
    EXPECT_EQ(pendingWant.target_, target);
    EXPECT_EQ(pendingWant.GetType(target), WantAgentConstant::OperationType::UNKNOWN_TYPE);
}

/*
 * @tc.number    : PendingWant_0400
 * @tc.name      : PendingWant GetAbility
 * @tc.desc      : 1.Get pendingWant (context is nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_0400, Function | MediumTest | Level1)
{
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_NO_CREATE;
    std::shared_ptr<PendingWant> pendingWant = PendingWant::GetAbility(nullptr, requestCode, want, flags);
    EXPECT_EQ(pendingWant, nullptr);
}


/*
 * @tc.number    : PendingWant_0600
 * @tc.name      : PendingWant GetAbility
 * @tc.desc      : 1.Get pendingWant (options is not nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_0600, Function | MediumTest | Level1)
{
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::GetAbility(GetAppContext(), requestCode, want, flags, wParams, pendingWant);
    EXPECT_NE(pendingWant, nullptr);
}

/*
 * @tc.number    : PendingWant_0700
 * @tc.name      : PendingWant GetAbilities
 * @tc.desc      : 1.Get pendingWant (context is nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_0700, Function | MediumTest | Level1)
{
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    std::shared_ptr<Want> want2 = std::make_shared<Want>();
    ElementName element2("device", "bundleName", "abilityName");
    want2->SetElement(element2);
    std::vector<std::shared_ptr<Want>> wants;
    wants.emplace_back(want);
    wants.emplace_back(want2);
    unsigned int flags = 1;
    flags |= FLAG_NO_CREATE;
    std::shared_ptr<PendingWant> pendingWant = PendingWant::GetAbilities(
        nullptr, requestCode, wants, flags);
    EXPECT_EQ(pendingWant, nullptr);
}

/*
 * @tc.number    : PendingWant_0800
 * @tc.name      : PendingWant GetAbilities
 * @tc.desc      : 1.Get pendingWant (context is not nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_0800, Function | MediumTest | Level1)
{
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    std::shared_ptr<Want> want2 = std::make_shared<Want>();
    ElementName element2("device", "bundleName", "abilityName");
    want2->SetElement(element2);
    std::vector<std::shared_ptr<Want>> wants;
    wants.emplace_back(want);
    wants.emplace_back(want2);
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    std::shared_ptr<PendingWant> pendingWant = PendingWant::GetAbilities(
        GetAppContext(), requestCode, wants, flags);
    EXPECT_NE(pendingWant, nullptr);
}

/*
 * @tc.number    : PendingWant_0900
 * @tc.name      : PendingWant GetAbilities
 * @tc.desc      : 1.Get pendingWant (options is not nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_0900, Function | MediumTest | Level1)
{
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    std::shared_ptr<Want> want2 = std::make_shared<Want>();
    ElementName element2("device", "bundleName", "abilityName");
    want2->SetElement(element2);
    std::vector<std::shared_ptr<Want>> wants;
    wants.emplace_back(want);
    wants.emplace_back(want2);
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::GetAbilities(GetAppContext(), requestCode, wants, flags, wParams, pendingWant);
    EXPECT_NE(pendingWant, nullptr);
}

/*
 * @tc.number    : PendingWant_1000
 * @tc.name      : PendingWant GetCommonEventAsUser
 * @tc.desc      : 1.Get pendingWant (context is nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_1000, Function | MediumTest | Level1)
{
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_NO_CREATE;
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::GetCommonEventAsUser(nullptr, requestCode, want, flags, pendingWant);
    EXPECT_EQ(pendingWant, nullptr);
}

/*
 * @tc.number    : PendingWant_1100
 * @tc.name      : PendingWant GetCommonEventAsUser
 * @tc.desc      : 1.Get pendingWant (context is not nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_1100, Function | MediumTest | Level1)
{
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::GetCommonEventAsUser(GetAppContext(), requestCode, want, flags, pendingWant);
    EXPECT_NE(pendingWant, nullptr);
}

/*
 * @tc.number    : PendingWant_1200
 * @tc.name      : PendingWant GetCommonEventAsUser
 * @tc.desc      : 1.Get pendingWant (want is nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_1200, Function | MediumTest | Level1)
{
    int requestCode = 10;
    std::shared_ptr<Want> want;
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::GetCommonEventAsUser(GetAppContext(), requestCode, want, flags, pendingWant);
    EXPECT_NE(pendingWant, nullptr);
}

/*
 * @tc.number    : PendingWant_1300
 * @tc.name      : PendingWant GetService
 * @tc.desc      : 1.Get pendingWant (want is nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_1300, Function | MediumTest | Level1)
{
    int requestCode = 10;
    std::shared_ptr<Want> want;
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::GetService(GetAppContext(), requestCode, want, flags, pendingWant);
    EXPECT_NE(pendingWant, nullptr);
}

/*
 * @tc.number    : PendingWant_1400
 * @tc.name      : PendingWant GetService
 * @tc.desc      : 1.Get pendingWant (context is nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_1400, Function | MediumTest | Level1)
{
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::GetService(nullptr, requestCode, want, flags, pendingWant);
    EXPECT_EQ(pendingWant, nullptr);
}

/*
 * @tc.number    : PendingWant_1500
 * @tc.name      : PendingWant GetForegroundService
 * @tc.desc      : 1.Get pendingWant (want is nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_1500, Function | MediumTest | Level1)
{
    int requestCode = 10;
    std::shared_ptr<Want> want;
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::GetForegroundService(GetAppContext(), requestCode, want, flags, pendingWant);
    EXPECT_NE(pendingWant, nullptr);
}

/*
 * @tc.number    : PendingWant_1600
 * @tc.name      : PendingWant GetForegroundService
 * @tc.desc      : 1.Get pendingWant (context is nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_1600, Function | MediumTest | Level1)
{
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_NO_CREATE;
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::GetForegroundService(nullptr, requestCode, want, flags, pendingWant);
    EXPECT_EQ(pendingWant, nullptr);
}

/*
 * @tc.number    : PendingWant_1700
 * @tc.name      : PendingWant GetForegroundService
 * @tc.desc      : 1.Get pendingWant (want is nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_1700, Function | MediumTest | Level1)
{
    int requestCode = 10;
    std::shared_ptr<Want> want;
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::START_FOREGROUND_SERVICE;
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::BuildServicePendingWant(GetAppContext(), requestCode, want, flags, type, pendingWant);
    EXPECT_NE(pendingWant, nullptr);
}

/*
 * @tc.number    : PendingWant_1800
 * @tc.name      : PendingWant GetForegroundService
 * @tc.desc      : 1.Get pendingWant (context is nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_1800, Function | MediumTest | Level1)
{
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_NO_CREATE;
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::START_FOREGROUND_SERVICE;
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::BuildServicePendingWant(nullptr, requestCode, want, flags, type, pendingWant);
    EXPECT_EQ(pendingWant, nullptr);
}

/*
 * @tc.number    : PendingWant_1900
 * @tc.name      : PendingWant Equals
 * @tc.desc      : 1.Equals
 */
HWTEST_F(PendingWantTest, PendingWant_1900, Function | MediumTest | Level1)
{
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::START_FOREGROUND_SERVICE;
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::BuildServicePendingWant(context, requestCode, want, flags, type, pendingWant);

    sptr<AAFwk::IWantSender> target2(nullptr);
    std::shared_ptr<PendingWant> pendingWant2 = std::make_shared<PendingWant>(target2);
    EXPECT_NE(pendingWant->IsEquals(pendingWant, pendingWant2), ERR_OK);
}

/*
 * @tc.number    : PendingWant_2000
 * @tc.name      : PendingWant Equals
 * @tc.desc      : 1.Equals
 */
HWTEST_F(PendingWantTest, PendingWant_2000, Function | MediumTest | Level1)
{
    sptr<AAFwk::IWantSender> target(new (std::nothrow) PendingWantRecord());
    std::shared_ptr<PendingWant> pendingWant = std::make_shared<PendingWant>(target);
    std::shared_ptr<PendingWant> pendingWant2(nullptr);
    EXPECT_NE(pendingWant->IsEquals(pendingWant, pendingWant2), ERR_OK);
}

/*
 * @tc.number    : PendingWant_2100
 * @tc.name      : PendingWant SendAndReturnResult
 * @tc.desc      : SendAndReturnResult
 */
HWTEST_F(PendingWantTest, PendingWant_2100, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    bool value = true;
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    std::string key = "key";
    wParams->SetParam(key, Boolean::Box(value));
    unsigned int flags = 1;
    flags |= FLAG_NO_CREATE;
    AbilityManagerClient::GetInstance()->Connect();
    EXPECT_EQ(ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT,
        pendingWant.SendAndReturnResult(requestCode, want, nullptr, "Permission", nullptr, nullptr));
}

/*
 * @tc.number    : PendingWant_2400
 * @tc.name      : PendingWant GetTarget
 * @tc.desc      : 1.GetTarget
 */
HWTEST_F(PendingWantTest, PendingWant_2400, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    auto target = pendingWant.GetTarget();
    EXPECT_EQ(target, nullptr);
}

/*
 * @tc.number    : PendingWant_2500
 * @tc.name      : PendingWant GetTarget
 * @tc.desc      : 1.GetTarget
 */
HWTEST_F(PendingWantTest, PendingWant_2500, Function | MediumTest | Level1)
{
    sptr<AAFwk::IWantSender> target(new (std::nothrow) PendingWantRecord());
    PendingWant pendingWant(target);
    auto target1 = pendingWant.GetTarget();
    EXPECT_EQ(target1, target);
}

/*
 * @tc.number    : PendingWant_2600
 * @tc.name      : PendingWant RegisterCancelListener
 * @tc.desc      : 1.RegisterCancelListener
 */
HWTEST_F(PendingWantTest, PendingWant_2600, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    std::shared_ptr<CancelListener> cancelListener1 = std::make_shared<CancelListenerSon>();
    std::shared_ptr<CancelListener> cancelListener2 = std::make_shared<CancelListenerSon>();
    pendingWant.RegisterCancelListener(cancelListener1, nullptr);
    pendingWant.RegisterCancelListener(cancelListener2, nullptr);
    EXPECT_EQ(static_cast<int>(pendingWant.cancelListeners_.size()), 2);
}

/*
 * @tc.number    : PendingWant_2700
 * @tc.name      : PendingWant RegisterCancelListener
 * @tc.desc      : 1.RegisterCancelListener
 */
HWTEST_F(PendingWantTest, PendingWant_2700, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    std::shared_ptr<CancelListener> cancelListener2 = std::make_shared<CancelListenerSon>();
    pendingWant.RegisterCancelListener(nullptr, nullptr);
    pendingWant.RegisterCancelListener(cancelListener2, nullptr);
    EXPECT_EQ(static_cast<int>(pendingWant.cancelListeners_.size()), 1);
}

/*
 * @tc.number    : PendingWant_2800
 * @tc.name      : PendingWant RegisterCancelListener
 * @tc.desc      : 1.RegisterCancelListener
 */
HWTEST_F(PendingWantTest, PendingWant_2800, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    pendingWant.RegisterCancelListener(nullptr, nullptr);
    pendingWant.RegisterCancelListener(nullptr, nullptr);
    EXPECT_EQ(static_cast<int>(pendingWant.cancelListeners_.size()), 0);
}

/*
 * @tc.number    : PendingWant_2900
 * @tc.name      : PendingWant NotifyCancelListeners
 * @tc.desc      : 1.NotifyCancelListeners
 */
HWTEST_F(PendingWantTest, PendingWant_2900, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    std::shared_ptr<CancelListener> cancelListener1 = std::make_shared<CancelListenerSon>();
    std::shared_ptr<CancelListener> cancelListener2 = std::make_shared<CancelListenerSon>();
    pendingWant.RegisterCancelListener(cancelListener1, nullptr);
    pendingWant.RegisterCancelListener(cancelListener2, nullptr);
    pendingWant.NotifyCancelListeners(0);
    EXPECT_EQ(callBackCancelListenerConnt, 2);
    callBackCancelListenerConnt = 0;
}

/*
 * @tc.number    : PendingWant_3000
 * @tc.name      : PendingWant NotifyCancelListeners
 * @tc.desc      : 1.NotifyCancelListeners
 */
HWTEST_F(PendingWantTest, PendingWant_3000, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    std::shared_ptr<CancelListener> cancelListener2 = std::make_shared<CancelListenerSon>();
    pendingWant.RegisterCancelListener(nullptr, nullptr);
    pendingWant.RegisterCancelListener(cancelListener2, nullptr);
    pendingWant.NotifyCancelListeners(0);
    EXPECT_EQ(callBackCancelListenerConnt, 1);
    callBackCancelListenerConnt = 0;
}

/*
 * @tc.number    : PendingWant_3100
 * @tc.name      : PendingWant NotifyCancelListeners
 * @tc.desc      : 1.NotifyCancelListeners
 */
HWTEST_F(PendingWantTest, PendingWant_3100, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    pendingWant.RegisterCancelListener(nullptr, nullptr);
    pendingWant.RegisterCancelListener(nullptr, nullptr);
    pendingWant.NotifyCancelListeners(0);
    EXPECT_EQ(callBackCancelListenerConnt, 0);
    callBackCancelListenerConnt = 0;
}

/*
 * @tc.number    : PendingWant_3200
 * @tc.name      : PendingWant UnregisterCancelListener
 * @tc.desc      : 1.UnregisterCancelListener
 */
HWTEST_F(PendingWantTest, PendingWant_3200, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    std::shared_ptr<CancelListener> cancelListener1 = std::make_shared<CancelListenerSon>();
    std::shared_ptr<CancelListener> cancelListener2 = std::make_shared<CancelListenerSon>();
    pendingWant.RegisterCancelListener(cancelListener1, nullptr);
    pendingWant.RegisterCancelListener(cancelListener2, nullptr);
    EXPECT_EQ(static_cast<int>(pendingWant.cancelListeners_.size()), 2);
    pendingWant.UnregisterCancelListener(cancelListener1, nullptr);
    EXPECT_EQ(static_cast<int>(pendingWant.cancelListeners_.size()), 1);
}

/*
 * @tc.number    : PendingWant_3300
 * @tc.name      : PendingWant UnregisterCancelListener
 * @tc.desc      : 1.UnregisterCancelListener
 */
HWTEST_F(PendingWantTest, PendingWant_3300, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    std::shared_ptr<CancelListener> cancelListener2 = std::make_shared<CancelListenerSon>();
    pendingWant.RegisterCancelListener(nullptr, nullptr);
    pendingWant.RegisterCancelListener(cancelListener2, nullptr);
    EXPECT_EQ(static_cast<int>(pendingWant.cancelListeners_.size()), 1);
    pendingWant.UnregisterCancelListener(cancelListener2, nullptr);
    EXPECT_EQ(static_cast<int>(pendingWant.cancelListeners_.size()), 0);
}

/*
 * @tc.number    : PendingWant_3400
 * @tc.name      : PendingWant UnregisterCancelListener
 * @tc.desc      : 1.UnregisterCancelListener
 */
HWTEST_F(PendingWantTest, PendingWant_3400, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    pendingWant.UnregisterCancelListener(nullptr, nullptr);
    EXPECT_EQ(static_cast<int>(pendingWant.cancelListeners_.size()), 0);
}

/*
 * @tc.number    : PendingWant_3500
 * @tc.name      : PendingWant GetWant
 * @tc.desc      : 1.GetWant
 */
HWTEST_F(PendingWantTest, PendingWant_3500, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    auto want = pendingWant.GetWant(nullptr);
    EXPECT_EQ(want, nullptr);
}

/*
 * @tc.number    : PendingWant_3600
 * @tc.name      : PendingWant Equals
 * @tc.desc      : Equals
 */
HWTEST_F(PendingWantTest, PendingWant_3700, Function | MediumTest | Level1)
{
    std::shared_ptr<PendingWant> pendingWant(nullptr);
    std::shared_ptr<PendingWant> pendingWant2(nullptr);
    EXPECT_EQ(pendingWant->IsEquals(pendingWant, pendingWant2), ERR_OK);
}

/*
 * @tc.number    : PendingWant_3800
 * @tc.name      : Marshalling_01
 * @tc.desc      : test Marshalling function when target is null
 * tc.issue      : I5TGRZ
 */
HWTEST_F(PendingWantTest, PendingWant_3800, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    Parcel parcel;
    bool ret = pendingWant.Marshalling(parcel);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.number    : PendingWant_3900
 * @tc.name      : Marshalling_02
 * @tc.desc      : test Marshalling function when target is target
 * tc.issue      : I5TGRZ
 */
HWTEST_F(PendingWantTest, PendingWant_3900, Function | MediumTest | Level1)
{
    sptr<AAFwk::IWantSender> target(new (std::nothrow) PendingWantRecord());
    PendingWant pendingWant(target);
    MessageParcel parcel;
    bool ret = pendingWant.Marshalling(parcel);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.number    : PendingWant_4000
 * @tc.name      : PendingWant GetType
 * @tc.desc      : 1.Get PendingWant Type (UNKNOWN_TYPE)
 */
HWTEST_F(PendingWantTest, PendingWant_4000, Function | MediumTest | Level1)
{
    sptr<AAFwk::IWantSender> target = nullptr;
    PendingWant pendingWant(target);
    EXPECT_EQ(pendingWant.target_, target);
    int32_t type = static_cast<int32_t>(WantAgentConstant::OperationType::UNKNOWN_TYPE);
    EXPECT_EQ(pendingWant.GetType(target, type), ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    EXPECT_EQ(type, static_cast<int32_t>(WantAgentConstant::OperationType::UNKNOWN_TYPE));
}

/*
 * @tc.number    : PendingWant_4100
 * @tc.name      : PendingWant GetWant
 * @tc.desc      : 1.GetWant
 */
HWTEST_F(PendingWantTest, PendingWant_4100, Function | MediumTest | Level1)
{
    sptr<AAFwk::IWantSender> target = nullptr;
    PendingWant pendingWant(target);
    EXPECT_EQ(pendingWant.target_, target);
    std::shared_ptr<AAFwk::Want> want(nullptr);
    EXPECT_EQ(pendingWant.GetWant(target, want), ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    EXPECT_EQ(want, nullptr);
}

/*
 * @tc.number    : PendingWant_4200
 * @tc.name      : PendingWant GetBundleName
 * @tc.desc      : 1.GetBundleName
 */
HWTEST_F(PendingWantTest, PendingWant_4200, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    std::string bundleName = "";
    pendingWant.GetBundleName(nullptr, bundleName);
    EXPECT_EQ(bundleName, "");
}

/*
 * @tc.number    : PendingWant_4300
 * @tc.name      : PendingWant GetUid
 * @tc.desc      : 1.GetUid
 */
HWTEST_F(PendingWantTest, PendingWant_4300, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    int32_t uid = -1;
    pendingWant.GetUid(nullptr, uid);
    EXPECT_EQ(uid, -1);
}

/*
 * @tc.number    : PendingWant_4400
 * @tc.name      : PendingWant Send
 * @tc.desc      : Send
 */
HWTEST_F(PendingWantTest, PendingWant_4400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "PendingWant_4400 start";
    PendingWant pendingWant(nullptr);
    sptr<AAFwk::IWantSender> target;
    pendingWant.Send(target);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "PendingWant_4400 end";
}

/*
 * @tc.number    : PendingWant_4500
 * @tc.name      : PendingWant Send
 * @tc.desc      : Send
 */
HWTEST_F(PendingWantTest, PendingWant_4500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "PendingWant_4500 start";
    PendingWant pendingWant(nullptr);
    int requestCode = 10;
    sptr<AAFwk::IWantSender> target;
    pendingWant.Send(requestCode, target);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "PendingWant_4500 end";
}

/*
 * @tc.number    : PendingWant_4600
 * @tc.name      : PendingWant Send
 * @tc.desc      : Send
 */
HWTEST_F(PendingWantTest, PendingWant_4600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "PendingWant_4600 start";
    PendingWant pendingWant(nullptr);
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    sptr<AAFwk::IWantSender> target;
    pendingWant.Send(requestCode, want, target);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "PendingWant_4600 end";
}

/*
 * @tc.number    : PendingWant_4700
 * @tc.name      : PendingWant Send
 * @tc.desc      : Send
 */
HWTEST_F(PendingWantTest, PendingWant_4700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "PendingWant_4700 start";
    PendingWant pendingWant(nullptr);
    int requestCode = 10;
    sptr<CompletedDispatcher> onCompleted;
    sptr<AAFwk::IWantSender> target;
    pendingWant.Send(requestCode, onCompleted, target);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "PendingWant_4700 end";
}

/*
 * @tc.number    : PendingWant_4800
 * @tc.name      : PendingWant Send
 * @tc.desc      : Send
 */
HWTEST_F(PendingWantTest, PendingWant_4800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "PendingWant_4800 start";
    PendingWant pendingWant(nullptr);
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    sptr<CompletedDispatcher> onCompleted;
    sptr<AAFwk::IWantSender> target;
    pendingWant.Send(requestCode, want, onCompleted, target);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "PendingWant_4800 end";
}

/*
 * @tc.number    : PendingWant_4900
 * @tc.name      : PendingWant Send
 * @tc.desc      : Send
 */
HWTEST_F(PendingWantTest, PendingWant_4900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "PendingWant_4900 start";
    PendingWant pendingWant(nullptr);
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    sptr<CompletedDispatcher> onCompleted = nullptr;
    std::string requiredPermission = "Permission";
    sptr<AAFwk::IWantSender> target;
    pendingWant.Send(requestCode, want, onCompleted, requiredPermission, target);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "PendingWant_4900 end";
}

/*
 * @tc.number    : PendingWant_5000
 * @tc.name      : PendingWant Send
 * @tc.desc      : Send
 */
HWTEST_F(PendingWantTest, PendingWant_5000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "PendingWant_5000 start";
    PendingWant pendingWant(nullptr);
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    sptr<CompletedDispatcher> onCompleted;
    std::string requiredPermission = "Permission";
    std::shared_ptr<WantParams> options;
    sptr<AAFwk::IWantSender> target;
    pendingWant.Send(requestCode, want, onCompleted, requiredPermission, options, target);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "PendingWant_5000 end";
}

/*
 * @tc.number    : PendingWant_5100
 * @tc.name      : PendingWant SetTarget
 * @tc.desc      : SetTarget
 */
HWTEST_F(PendingWantTest, PendingWant_5100, Function | MediumTest | Level1)
{
    sptr<AAFwk::IWantSender> target(new (std::nothrow) PendingWantRecord());
    PendingWant pendingWant(nullptr);
    auto target1 = pendingWant.GetTarget();
    pendingWant.SetTarget(target);
    auto target2 = pendingWant.GetTarget();
    EXPECT_EQ(target1, nullptr);
    EXPECT_EQ(target2, target);
}

/*
 * @tc.number    : PendingWant_5200
 * @tc.name      : Unmarshalling_01
 * @tc.desc      : Test Unmarshalling function when target is null
 */
HWTEST_F(PendingWantTest, PendingWant_5200, Function | MediumTest | Level1)
{
    sptr<AAFwk::IWantSender> target(new (std::nothrow) PendingWantRecord());
    PendingWant pendingWant(target);
    MessageParcel parcel;
    pendingWant.Unmarshalling(parcel);
    auto target1 = pendingWant.GetTarget();
    EXPECT_EQ(target1, target);
}

/*
 * @tc.number    : PendingWant_5300
 * @tc.name      : PendingWant GetWantSenderInfo
 * @tc.desc      : GetWantSenderInfo
 */
HWTEST_F(PendingWantTest, PendingWant_5300, Function | MediumTest | Level1)
{
    PendingWant pendingWant(nullptr);
    auto wantsenderinfo = pendingWant.GetWantSenderInfo(nullptr);
    EXPECT_EQ(wantsenderinfo, nullptr);
}

/*
 * @tc.number    : PendingWant_5400
 * @tc.name      : PendingWant NotifyCancelListeners
 * @tc.desc      : NotifyCancelListeners
 */
HWTEST_F(PendingWantTest, PendingWant_5400, Function | MediumTest | Level1)
{
    std::shared_ptr<PendingWant> pendingWant = std::make_shared<PendingWant>(nullptr);
    std::shared_ptr<CancelListener> cancelListener1 = std::make_shared<CancelListenerSon>();
    std::shared_ptr<CancelListener> cancelListener2 = std::make_shared<CancelListenerSon>();
    pendingWant->RegisterCancelListener(cancelListener1, nullptr);
    pendingWant->RegisterCancelListener(cancelListener2, nullptr);
    std::weak_ptr<PendingWant> outerInstance(pendingWant);
    PendingWant::CancelReceiver cancelreceiver(outerInstance);
    cancelreceiver.Send(0);

    EXPECT_EQ(callBackCancelListenerConnt, 2);
    callBackCancelListenerConnt = 0;
}

/*
 * @tc.number    : PendingWant_5500
 * @tc.name      : PendingWant GetAbilitys
 * @tc.desc      : 1.Get pendingWant (context is nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_5500, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_5500 start.");
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    std::shared_ptr<Want> want2 = std::make_shared<Want>();
    ElementName element2("device", "bundleName", "abilityName");
    want2->SetElement(element2);
    std::vector<std::shared_ptr<Want>> wants;
    wants.emplace_back(want);
    wants.emplace_back(want2);
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    unsigned int flags = 1;
    flags |= FLAG_NO_CREATE;
    std::shared_ptr<PendingWant> pendingWant;
    ErrCode err = PendingWant::GetAbilities(nullptr, requestCode, wants, flags, wParams, pendingWant);
    EXPECT_EQ(err, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    HILOG_INFO("PendingWant_5500 end.");
}

/*
 * @tc.number    : PendingWant_5600
 * @tc.name      : PendingWant GetAbilities
 * @tc.desc      : 1.Get pendingWant (context is not nullptr FLAG_NO_CREATE)
 */
HWTEST_F(PendingWantTest, PendingWant_5600, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_5600 start.");
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    std::shared_ptr<Want> want2 = std::make_shared<Want>();
    ElementName element2("device", "bundleName", "abilityName");
    want2->SetElement(element2);
    std::vector<std::shared_ptr<Want>> wants;
    wants.emplace_back(want);
    wants.emplace_back(want2);
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    unsigned int flags = 1;
    flags |= FLAG_NO_CREATE;
    std::shared_ptr<PendingWant> pendingWant;
    ErrCode err = PendingWant::GetAbilities(GetAppContext(), requestCode, wants, flags, wParams, pendingWant);
    EXPECT_EQ(err, ERR_OK);
    HILOG_INFO("PendingWant_5600 end.");
}

/*
 * @tc.number    : PendingWant_5700
 * @tc.name      : PendingWant GetAbilities
 * @tc.desc      : 1.Get pendingWant (context is not nullptr FLAG_ONE_SHOT)
 */
HWTEST_F(PendingWantTest, PendingWant_5700, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_5700 start.");
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    std::shared_ptr<Want> want2 = std::make_shared<Want>();
    ElementName element2("device", "bundleName", "abilityName");
    want2->SetElement(element2);
    std::vector<std::shared_ptr<Want>> wants;
    wants.emplace_back(want);
    wants.emplace_back(want2);
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    std::shared_ptr<PendingWant> pendingWant;
    ErrCode err = PendingWant::GetAbilities(GetAppContext(), requestCode, wants, flags, wParams, pendingWant);
    EXPECT_EQ(err, ERR_OK);
    HILOG_INFO("PendingWant_5700 end.");
}

/*
 * @tc.number    : PendingWant_5800
 * @tc.name      : PendingWant GetAbilitie
 * @tc.desc      : 1.Get pendingWant (context is nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_5800, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_5800 start.");
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    std::shared_ptr<PendingWant> pendingWant;
    ErrCode err = PendingWant::GetAbility(nullptr, requestCode, want, flags, wParams, pendingWant);
    EXPECT_EQ(err, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    HILOG_INFO("PendingWant_5800 end.");
}

/*
 * @tc.number    : PendingWant_5800
 * @tc.name      : PendingWant GetAbilitie
 * @tc.desc      : 1.Get pendingWant (context not nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_5900, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_5900 start.");
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    std::shared_ptr<PendingWant> pendingWant;
    ErrCode err = PendingWant::GetAbility(GetAppContext(), requestCode, want, flags, wParams, pendingWant);
    EXPECT_EQ(err, ERR_OK);
    HILOG_INFO("PendingWant_5900 end.");
}

/*
 * @tc.number    : PendingWant_6000
 * @tc.name      : PendingWant GetCommonEvent
 * @tc.desc      : 1.Get pendingWant (context is nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_6000, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_6000 start.");
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_NO_CREATE;
    std::shared_ptr<PendingWant> pendingWant;
    ErrCode err = PendingWant::GetCommonEvent(nullptr, requestCode, want, flags, pendingWant);
    EXPECT_EQ(err, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    HILOG_INFO("PendingWant_6000 end.");
}

/*
 * @tc.number    : PendingWant_6100
 * @tc.name      : PendingWant GetCommonEvent
 * @tc.desc      : 1.Get pendingWant (context is not nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_6100, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_6100 start.");
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    std::shared_ptr<PendingWant> pendingWant;
    ErrCode err = PendingWant::GetCommonEvent(GetAppContext(), requestCode, want, flags, pendingWant);
    EXPECT_EQ(err, ERR_OK);
    HILOG_INFO("PendingWant_6100 end.");
}

/*
 * @tc.number    : PendingWant_6200
 * @tc.name      : PendingWant GetService
 * @tc.desc      : 1.Get pendingWant (context is nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_6200, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_6200 start.");
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_NO_CREATE;
    std::shared_ptr<PendingWant> pendingWant;
    ErrCode err = PendingWant::GetService(nullptr, requestCode, want, flags, pendingWant);
    EXPECT_EQ(err, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    HILOG_INFO("PendingWant_6200 end.");
}

/*
 * @tc.number    : PendingWant_6300
 * @tc.name      : PendingWant GetService
 * @tc.desc      : 1.Get pendingWant (context is not nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_6300, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_6300 start.");
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_NO_CREATE;
    std::shared_ptr<PendingWant> pendingWant;
    ErrCode err = PendingWant::GetService(nullptr, requestCode, want, flags, pendingWant);
    EXPECT_EQ(err, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    HILOG_INFO("PendingWant_6300 end.");
}

/*
 * @tc.number    : PendingWant_6400
 * @tc.name      : PendingWant GetService
 * @tc.desc      : 1.Get pendingWant (context is not nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_6400, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_6400 start.");
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    std::shared_ptr<PendingWant> pendingWant;
    ErrCode err = PendingWant::GetService(GetAppContext(), requestCode, want, flags, pendingWant);
    EXPECT_EQ(err, ERR_OK);
    HILOG_INFO("PendingWant_6400 end.");
}

/*
 * @tc.number    : PendingWant_6500
 * @tc.name      : PendingWant GetService
 * @tc.desc      : 1.Get pendingWant (context is not nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_6500, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_6500 start.");
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_NO_CREATE;
    std::shared_ptr<PendingWant> pendingWant;
    ErrCode err = PendingWant::GetForegroundService(GetAppContext(), requestCode, want, flags, pendingWant);
    EXPECT_EQ(err, ERR_OK);
    HILOG_INFO("PendingWant_6500 start.");
}

/*
 * @tc.number    : PendingWant_6600
 * @tc.name      : PendingWant GetService
 * @tc.desc      : 1.Get pendingWant (context is nullptr)
 */
HWTEST_F(PendingWantTest, PendingWant_6600, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_6600 start.");
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_NO_CREATE;
    std::shared_ptr<PendingWant> pendingWant;
    ErrCode err = PendingWant::GetForegroundService(nullptr, requestCode, want, flags, pendingWant);
    EXPECT_EQ(err, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    HILOG_INFO("PendingWant_6500 end.");
}

/*
 * @tc.number    : PendingWant_6700
 * @tc.name      : PendingWant Cancel
 * @tc.desc      : Cancel a WantAgent
 */
HWTEST_F(PendingWantTest, PendingWant_6700, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_6700 start.");
    sptr<AAFwk::IWantSender> target = nullptr;
    std::shared_ptr<PendingWant> pendingWant = std::make_shared<PendingWant>();
    ErrCode err = pendingWant->Cancel(target);
    EXPECT_EQ(err, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    HILOG_INFO("PendingWant_6700 end.");
}

/*
 * @tc.number    : PendingWant_6800
 * @tc.name      : PendingWant IsEquals
 * @tc.desc      : 1.IsEquals
 */
HWTEST_F(PendingWantTest, PendingWant_6800, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_6800 start.");
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::START_FOREGROUND_SERVICE;
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::BuildServicePendingWant(context, requestCode, want, flags, type, pendingWant);

    sptr<AAFwk::IWantSender> target(nullptr);
    std::shared_ptr<PendingWant> pendingWant2 = std::make_shared<PendingWant>(target);
    EXPECT_EQ(pendingWant->IsEquals(pendingWant, pendingWant2), ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    HILOG_INFO("PendingWant_6800 end.");
}

/*
 * @tc.number    : PendingWant_6900
 * @tc.name      : PendingWant IsEquals
 * @tc.desc      : 1.IsEquals
 */
HWTEST_F(PendingWantTest, PendingWant_6900, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_6900 start.");
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::START_FOREGROUND_SERVICE;
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::BuildServicePendingWant(context, requestCode, want, flags, type, pendingWant);

    std::shared_ptr<PendingWant> pendingWant2 = nullptr;
    PendingWant::BuildServicePendingWant(context, requestCode, want, flags, type, pendingWant2);
    EXPECT_EQ(pendingWant->IsEquals(pendingWant, pendingWant2), ERR_OK);
    HILOG_INFO("PendingWant_6900 end.");
}

/*
 * @tc.number    : PendingWant_7000
 * @tc.name      : PendingWant IsEquals
 * @tc.desc      : 1.IsEquals
 */
HWTEST_F(PendingWantTest, PendingWant_7000, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_7000 start.");
    int requestCode = 10;
    std::shared_ptr<Want> want1 = std::make_shared<Want>();
    ElementName element1("ohos_device", "bundleName", "abilityName");
    want1->SetElement(element1);
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::START_FOREGROUND_SERVICE;
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::BuildServicePendingWant(context, requestCode, want1, flags, type, pendingWant);

    std::shared_ptr<Want> want2 = std::make_shared<Want>();
    ElementName element2("ohos_device", "ohos_bundleName", "ohos_abilityName");
    want2->SetElement(element2);
    std::shared_ptr<PendingWant> pendingWant2 = nullptr;
    PendingWant::BuildServicePendingWant(context, requestCode, want2, flags, type, pendingWant2);
    EXPECT_EQ(pendingWant->IsEquals(pendingWant, pendingWant2), NOTEQ);
    HILOG_INFO("PendingWant_7000 end.");
}

/*
 * @tc.number    : PendingWant_7100
 * @tc.name      : PendingWant GetBundleName
 * @tc.desc      : 1.GetBundleName
 */
HWTEST_F(PendingWantTest, PendingWant_7100, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_7100 start.");
    PendingWant pendingWant(nullptr);
    std::string bundleName;
    ErrCode err = pendingWant.GetBundleName(nullptr, bundleName);
    EXPECT_EQ(err, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    HILOG_INFO("PendingWant_7100 end.");
}

/*
 * @tc.number    : PendingWant_7200
 * @tc.name      : PendingWant GetBundleName
 * @tc.desc      : 1.GetBundleName
 */
HWTEST_F(PendingWantTest, PendingWant_7200, Function | MediumTest | Level1)
{
    HILOG_INFO("PendingWant_7200 start.");
    int requestCode = 10;
    std::shared_ptr<Want> want1 = std::make_shared<Want>();
    ElementName element1("ohos_device", "bundleName", "abilityName");
    want1->SetElement(element1);
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::START_FOREGROUND_SERVICE;
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::BuildServicePendingWant(context, requestCode, want1, flags, type, pendingWant);

    std::string bundleName;
    ErrCode err = pendingWant->GetBundleName(pendingWant->GetTarget(), bundleName);
    EXPECT_EQ(err, ERR_OK);
    HILOG_INFO("PendingWant_7200 start.");
}
}  // namespace OHOS::AbilityRuntime::WantAgent
