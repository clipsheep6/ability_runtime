/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "ability_context.h"
#include "ability_loader.h"
#include "ability_manager_client.h"
#include "context_deal.h"
#include "fa_ability_thread.h"
#include "mock_serviceability_manager_service.h"
#include "ohos_application.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#undef protected
#undef private

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS;
using namespace AAFwk;

namespace {
const std::string ACE_SERVICE_ABILITY_NAME = "AceServiceAbility";
}
class AbilityContextTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static constexpr int TEST_WAIT_TIME = 500 * 1000;  // 500 ms
public:
    std::unique_ptr<AbilityContext> context_ = nullptr;
};

void AbilityContextTest::SetUpTestCase(void)
{
    OHOS::sptr<OHOS::IRemoteObject> abilityObject = new (std::nothrow) MockServiceAbilityManagerService();

    auto sysMgr = OHOS::DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == nullptr) {
        GTEST_LOG_(ERROR) << "fail to get ISystemAbilityManager";
        return;
    }

    sysMgr->RegisterSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID, abilityObject);

    AbilityLoader::GetInstance().RegisterAbility(
        ACE_SERVICE_ABILITY_NAME, []()->Ability* { return new (std::nothrow) Ability; });
}

void AbilityContextTest::TearDownTestCase(void)
{}

void AbilityContextTest::SetUp(void)
{
    context_ = std::make_unique<AbilityContext>();
}

void AbilityContextTest::TearDown(void)
{}

/**
 * @tc.number: AaFwk_Ability_Context_ConnectAbility_0100
 * @tc.name: AbilityFwk
 * @tc.desc: When connecting ability, AMS will inform ability to process OnStart in the life cycle, and then inform
 * ability to process onconnect, and the connection is successful
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_ConnectAbility_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    abilityInfo->name = "DemoAbility";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

    AbilityThread::AbilityThreadMain(application, abilityRecord, nullptr);

    std::shared_ptr<ContextDeal> deal = std::make_shared<ContextDeal>();
    deal->SetAbilityInfo(abilityInfo);
    context_->AttachBaseContext(deal);

    Want want;
    bool ret = context_->ConnectAbility(want, nullptr);
    EXPECT_TRUE(ret);
    usleep(AbilityContextTest::TEST_WAIT_TIME);
}

/**
 * @tc.number: AaFwk_Ability_Context_DisconnectAbility_0100
 * @tc.name: AbilityFwk
 * @tc.desc: AMS notifies the abilityondisconnect event when disconnectservice.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_DisconnectAbility_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    abilityInfo->name = "DemoAbility";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

    AbilityThread::AbilityThreadMain(application, abilityRecord, nullptr);

    std::shared_ptr<ContextDeal> deal = std::make_shared<ContextDeal>();
    deal->SetAbilityInfo(abilityInfo);
    context_->AttachBaseContext(deal);

    Want want;
    context_->ConnectAbility(want, nullptr);
    context_->DisconnectAbility(nullptr);
    usleep(AbilityContextTest::TEST_WAIT_TIME);
}

/**
 * @tc.number: AaFwk_Ability_Context_StartAbility_0100
 * @tc.name: AbilityFwk
 * @tc.desc: Starting ability service, AMS will inform ability to perform OnStart lifecycle conversion, and then inform
 * oncommand event.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_StartAbility_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    abilityInfo->name = "DemoAbility";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

    AbilityThread::AbilityThreadMain(application, abilityRecord, nullptr);
    Want want;
    context_->StartAbility(want, -1);
    usleep(AbilityContextTest::TEST_WAIT_TIME);
}

/**
 * @tc.number: AaFwk_Ability_Context_TerminateAbility_0100
 * @tc.name: AbilityFwk
 * @tc.desc: To terminate ability service, AMS will notify ability to perform onbackground lifecycle conversion, and
 * then notify onstop event.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_TerminateAbility_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    abilityInfo->name = "DemoAbility";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

    AbilityThread::AbilityThreadMain(application, abilityRecord, nullptr);
    Want want;
    context_->StartAbility(want, -1);
    usleep(AbilityContextTest::TEST_WAIT_TIME);

    std::shared_ptr<ContextDeal> deal = std::make_shared<ContextDeal>();
    deal->SetAbilityInfo(abilityInfo);
    context_->AttachBaseContext(deal);
    context_->TerminateAbility();
    usleep(AbilityContextTest::TEST_WAIT_TIME);
}

/**
 * @tc.number: AaFwk_Ability_Context_TerminateAbility_0200
 * @tc.name: AbilityFwk
 * @tc.desc: When there is no startability, calling terminateability directly will not respond to onbackground and
 * onstop events.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_TerminateAbility_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    abilityInfo->name = "DemoAbility";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

    AbilityThread::AbilityThreadMain(application, abilityRecord, nullptr);

    std::shared_ptr<ContextDeal> deal = std::make_shared<ContextDeal>();
    deal->SetAbilityInfo(abilityInfo);
    context_->AttachBaseContext(deal);
    context_->TerminateAbility();
    usleep(AbilityContextTest::TEST_WAIT_TIME);
}

/**
 * @tc.number: AaFwk_Ability_Context_StopService_0100
 * @tc.name: AbilityFwk
 * @tc.desc: To stop ability service, AMS will notify ability to perform onbackground lifecycle conversion, and then
 * notify onstop event.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_StopService_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    abilityInfo->name = "DemoAbility";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

    AbilityThread::AbilityThreadMain(application, abilityRecord, nullptr);

    std::shared_ptr<ContextDeal> deal = std::make_shared<ContextDeal>();
    deal->SetAbilityInfo(abilityInfo);
    context_->AttachBaseContext(deal);

    Want want;
    context_->StartAbility(want, -1);
    usleep(AbilityContextTest::TEST_WAIT_TIME);
    bool ret = context_->StopAbility(want);
    EXPECT_TRUE(ret);
    usleep(AbilityContextTest::TEST_WAIT_TIME);
}

/**
 * @tc.number: AaFwk_Ability_Context_StopService_0200
 * @tc.name: AbilityFwk
 * @tc.desc: When there is no startability, calling stop ability directly will not respond to onbackground and onstop
 * events.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_StopService_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    abilityInfo->name = "DemoAbility";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

    AbilityThread::AbilityThreadMain(application, abilityRecord, nullptr);
    std::shared_ptr<ContextDeal> deal = std::make_shared<ContextDeal>();
    deal->SetAbilityInfo(abilityInfo);
    context_->AttachBaseContext(deal);

    Want want;
    bool ret = context_->StopAbility(want);
    EXPECT_TRUE(ret);
    usleep(AbilityContextTest::TEST_WAIT_TIME);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetCallingBundle_0100
 * @tc.name: SetCallingContext and GetCallingBundle
 * @tc.desc: Verify that function SetCallingContext and GetCallingBundle.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetCallingBundle_0100, Function | MediumTest | Level1)
{
    std::string deviceId = "deviceId";
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    std::string moduleName = "moduleName";
    context_->SetCallingContext(deviceId, bundleName, abilityName, moduleName);
    EXPECT_EQ(context_->GetCallingBundle(), "bundleName");
}

/**
 * @tc.number: AaFwk_Ability_Context_GetElementName_0100
 * @tc.name: GetElementName
 * @tc.desc: Verify that function GetElementName.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetElementName_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ContextDeal> deal = std::make_shared<ContextDeal>();
    deal->SetAbilityInfo(abilityInfo);
    auto result = context_->GetElementName();
    EXPECT_TRUE(result == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetElementName_0200
 * @tc.name: GetElementName
 * @tc.desc: Verify that function GetElementName.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetElementName_0200, Function | MediumTest | Level1)
{
    auto result = context_->GetElementName();
    EXPECT_TRUE(result == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetCallingAbility_0100
 * @tc.name: GetCallingAbility
 * @tc.desc: Verify that function GetCallingAbility.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetCallingAbility_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetCallingAbility();
    EXPECT_TRUE(result != nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_ConnectAbility_0200
 * @tc.name: ConnectAbility
 * @tc.desc: Verify that function ConnectAbility.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_ConnectAbility_0200, Function | MediumTest | Level1)
{
    Want want;
    bool ret = context_->ConnectAbility(want, nullptr);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: AaFwk_Ability_Context_ConnectAbility_0300
 * @tc.name: ConnectAbility
 * @tc.desc: Verify that function ConnectAbility.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_ConnectAbility_0300, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::DATA;
    abilityInfo->name = "DemoAbility";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    AbilityThread::AbilityThreadMain(application, abilityRecord, nullptr);
    std::shared_ptr<ContextDeal> deal = std::make_shared<ContextDeal>();
    deal->SetAbilityInfo(abilityInfo);
    context_->AttachBaseContext(deal);
    Want want;
    bool ret = context_->ConnectAbility(want, nullptr);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: AaFwk_Ability_Context_StopService_0300
 * @tc.name: StopAbility
 * @tc.desc: Verify that function StopAbility.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_StopService_0300, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::DATA;
    abilityInfo->name = "DemoAbility";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

    AbilityThread::AbilityThreadMain(application, abilityRecord, nullptr);
    std::shared_ptr<ContextDeal> deal = std::make_shared<ContextDeal>();
    deal->SetAbilityInfo(abilityInfo);
    context_->AttachBaseContext(deal);

    Want want;
    bool ret = context_->StopAbility(want);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetApplicationInfo_0100
 * @tc.name: GetApplicationInfo
 * @tc.desc: Verify that function GetApplicationInfo.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetApplicationInfo_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetApplicationInfo();
    EXPECT_TRUE(result == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetDatabaseDir_0100
 * @tc.name: GetDatabaseDir
 * @tc.desc: Verify that function GetDatabaseDir.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetDatabaseDir_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetDatabaseDir();
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.number: AaFwk_Ability_Context_GetDataDir_0100
 * @tc.name: GetDataDir
 * @tc.desc: Verify that function GetDataDir.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetDataDir_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetDataDir();
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.number: AaFwk_Ability_Context_GetBundleManager_0100
 * @tc.name: GetBundleManager
 * @tc.desc: Verify that function GetBundleManager.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetBundleManager_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetBundleManager();
    EXPECT_TRUE(result == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetDir_0100
 * @tc.name: GetDir
 * @tc.desc: Verify that function GetDir.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetDir_0100, Function | MediumTest | Level1)
{
    std::string name = "name";
    int32_t mode = 1;
    auto result = context_->GetDir(name, mode);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.number: AaFwk_Ability_Context_GetBundleCodePath_0100
 * @tc.name: GetBundleCodePath
 * @tc.desc: Verify that function GetBundleCodePath.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetBundleCodePath_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetBundleCodePath();
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.number: AaFwk_Ability_Context_GetBundleName_0100
 * @tc.name: GetBundleName
 * @tc.desc: Verify that function GetBundleName.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetBundleName_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetBundleName();
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.number: AaFwk_Ability_Context_GetBundleResourcePath_0100
 * @tc.name: GetBundleResourcePath
 * @tc.desc: Verify that function GetBundleResourcePath.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetBundleResourcePath_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetBundleResourcePath();
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.number: AaFwk_Ability_Context_GetApplicationContext_0100
 * @tc.name: GetApplicationContext
 * @tc.desc: Verify that function GetApplicationContext.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetApplicationContext_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetApplicationContext();
    EXPECT_TRUE(result == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetContext_0100
 * @tc.name: GetContext
 * @tc.desc: Verify that function GetContext.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetContext_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetContext();
    EXPECT_TRUE(result == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetAbilityManager_0100
 * @tc.name: GetAbilityManager
 * @tc.desc: Verify that function GetAbilityManager.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetAbilityManager_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetAbilityManager();
    EXPECT_TRUE(result == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetProcessInfo_0100
 * @tc.name: GetProcessInfo
 * @tc.desc: Verify that function GetProcessInfo.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetProcessInfo_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetProcessInfo();
    EXPECT_TRUE(result == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetAppType_0100
 * @tc.name: GetAppType
 * @tc.desc: Verify that function GetAppType.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetAppType_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetAppType();
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.number: AaFwk_Ability_Context_GetAbilityInfo_0100
 * @tc.name: GetAbilityInfo
 * @tc.desc: Verify that function GetAbilityInfo.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetAbilityInfo_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetAbilityInfo();
    EXPECT_TRUE(result == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetHapModuleInfo_0100
 * @tc.name: GetHapModuleInfo
 * @tc.desc: Verify that function GetHapModuleInfo.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetHapModuleInfo_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetHapModuleInfo();
    EXPECT_TRUE(result == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetHapModuleInfo_0100
 * @tc.name: GetAbilityInfoType
 * @tc.desc: Verify that function GetAbilityInfoType.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetAbilityInfoType_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetAbilityInfoType();
    EXPECT_EQ(result, AppExecFwk::AbilityType::UNKNOWN);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetAbilityInfoType_0200
 * @tc.name: GetAbilityInfoType
 * @tc.desc: Verify that function GetAbilityInfoType.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetAbilityInfoType_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    abilityInfo->name = "DemoAbility";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

    AbilityThread::AbilityThreadMain(application, abilityRecord, nullptr);
    std::shared_ptr<ContextDeal> deal = std::make_shared<ContextDeal>();
    deal->SetAbilityInfo(abilityInfo);
    context_->AttachBaseContext(deal);
    auto result = context_->GetAbilityInfoType();
    EXPECT_EQ(result, AppExecFwk::AbilityType::SERVICE);
}

/**
 * @tc.number: AaFwk_Ability_Context_CreateBundleContext_0100
 * @tc.name: CreateBundleContext
 * @tc.desc: Verify that function CreateBundleContext.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_CreateBundleContext_0100, Function | MediumTest | Level1)
{
    std::string bundleName;
    int32_t flag = 1;
    int32_t accountId = 1;
    auto result = context_->CreateBundleContext(bundleName, flag, accountId);
    EXPECT_TRUE(result == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetResourceManager_0100
 * @tc.name: GetResourceManager
 * @tc.desc: Verify that function GetResourceManager.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetResourceManager_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetResourceManager();
    EXPECT_TRUE(result == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetResourceManager_0200
 * @tc.name: GetResourceManager
 * @tc.desc: Verify that function GetResourceManager.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetResourceManager_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    abilityInfo->name = "DemoAbility";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    AbilityThread::AbilityThreadMain(application, abilityRecord, nullptr);
    std::shared_ptr<ContextDeal> deal = std::make_shared<ContextDeal>();
    deal->SetAbilityInfo(abilityInfo);
    context_->AttachBaseContext(deal);
    auto result = context_->GetResourceManager();
    EXPECT_TRUE(result == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_VerifyPermission_0100
 * @tc.name: VerifyPermission
 * @tc.desc: Verify that function VerifyPermission.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_VerifyPermission_0100, Function | MediumTest | Level1)
{
    std::string permission;
    int32_t pid = 1;
    int32_t uid = 1;
    auto result = context_->VerifyPermission(permission, pid, uid);
    EXPECT_EQ(result, AppExecFwk::Constants::PERMISSION_NOT_GRANTED);
}

/**
 * @tc.number: AaFwk_Ability_Context_VerifyPermission_0200
 * @tc.name: VerifyPermission
 * @tc.desc: Verify that function VerifyPermission.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_VerifyPermission_0200, Function | MediumTest | Level1)
{
    std::string permission = "permission";
    int32_t pid = 1;
    int32_t uid = 1;
    auto result = context_->VerifyPermission(permission, pid, uid);
    EXPECT_EQ(result, AppExecFwk::Constants::PERMISSION_NOT_GRANTED);
}

/**
 * @tc.number: AaFwk_Ability_Context_VerifyPermission_0300
 * @tc.name: VerifyPermission
 * @tc.desc: Verify that function VerifyPermission.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_VerifyPermission_0300, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    abilityInfo->name = "DemoAbility";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    AbilityThread::AbilityThreadMain(application, abilityRecord, nullptr);
    std::shared_ptr<ContextDeal> deal = std::make_shared<ContextDeal>();
    deal->SetAbilityInfo(abilityInfo);
    context_->AttachBaseContext(deal);
    std::string permission = "permission";
    int32_t pid = 1;
    int32_t uid = 1;
    auto result = context_->VerifyPermission(permission, pid, uid);
    EXPECT_EQ(result, AppExecFwk::Constants::PERMISSION_NOT_GRANTED);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetPermissionDes_0100
 * @tc.name: GetPermissionDes
 * @tc.desc: Verify that function GetPermissionDes.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetPermissionDes_0100, Function | MediumTest | Level1)
{
    std::string permissionName = "permissionName";
    std::string des = "des";
    context_->GetPermissionDes(permissionName, des);
    EXPECT_TRUE(permissionName.length() != des.length());
}

/**
 * @tc.number: AaFwk_Ability_Context_RequestPermissionsFromUser_0100
 * @tc.name: RequestPermissionsFromUser
 * @tc.desc: Verify that function RequestPermissionsFromUser.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_RequestPermissionsFromUser_0100, Function | MediumTest | Level1)
{
    std::vector<std::string> permissions;
    std::vector<int> permissionsState;
    context_->RequestPermissionsFromUser(permissions, permissionsState, nullptr);
    EXPECT_TRUE(permissions.empty());
}

/**
 * @tc.number: AaFwk_Ability_Context_RequestPermissionsFromUser_0200
 * @tc.name: RequestPermissionsFromUser
 * @tc.desc: Verify that function RequestPermissionsFromUser.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_RequestPermissionsFromUser_0200, Function | MediumTest | Level1)
{
    std::vector<std::string> permissions;
    permissions.emplace_back("a");
    std::vector<int> permissionsState;
    context_->RequestPermissionsFromUser(permissions, permissionsState, nullptr);
    EXPECT_TRUE(!permissions.empty());
}

/**
 * @tc.number: AaFwk_Ability_Context_GetCaller_0100
 * @tc.name: GetCaller
 * @tc.desc: Verify that function GetCaller.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetCaller_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetCaller();
    EXPECT_EQ(result.GetScheme(), "");
}

/**
 * @tc.number: AaFwk_Ability_Context_GetExternalCacheDir_0100
 * @tc.name: GetExternalCacheDir
 * @tc.desc: Verify that function GetExternalCacheDir.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetExternalCacheDir_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetExternalCacheDir();
    EXPECT_EQ(result, "");
}

/**
 * @tc.number: AaFwk_Ability_Context_GetExternalFilesDir_0100
 * @tc.name: GetExternalFilesDir
 * @tc.desc: Verify that function GetExternalFilesDir.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetExternalFilesDir_0100, Function | MediumTest | Level1)
{
    std::string type;
    auto result = context_->GetExternalFilesDir(type);
    EXPECT_EQ(result, "");
}

/**
 * @tc.number: AaFwk_Ability_Context_GetFilesDir_0100
 * @tc.name: GetFilesDir
 * @tc.desc: Verify that function GetFilesDir.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetFilesDir_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetFilesDir();
    EXPECT_EQ(result, "");
}

/**
 * @tc.number: AaFwk_Ability_Context_GetAbilityPackageContext_0100
 * @tc.name: GetAbilityPackageContext
 * @tc.desc: Verify that function GetAbilityPackageContext.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetAbilityPackageContext_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetAbilityPackageContext();
    EXPECT_TRUE(result == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetProcessName_0100
 * @tc.name: GetProcessName
 * @tc.desc: Verify that function GetProcessName.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetProcessName_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetProcessName();
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.number: AaFwk_Ability_Context_InitResourceManager_0100
 * @tc.name: InitResourceManager
 * @tc.desc: Verify that function InitResourceManager.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_InitResourceManager_0100, Function | MediumTest | Level1)
{
    BundleInfo bundleInfo;
    std::shared_ptr<ContextDeal> deal = nullptr;
    context_->InitResourceManager(bundleInfo, deal);
    EXPECT_TRUE(deal == nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetString_0100
 * @tc.name: GetString
 * @tc.desc: Verify that function GetString.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetString_0100, Function | MediumTest | Level1)
{
    int32_t resId = 1;
    auto result = context_->GetString(resId);
    EXPECT_EQ(result, "");
}

/**
 * @tc.number: AaFwk_Ability_Context_GetStringArray_0100
 * @tc.name: GetStringArray
 * @tc.desc: Verify that function GetStringArray.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetStringArray_0100, Function | MediumTest | Level1)
{
    int32_t resId = 1;
    auto result = context_->GetStringArray(resId);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.number: AaFwk_Ability_Context_GetIntArray_0100
 * @tc.name: GetIntArray
 * @tc.desc: Verify that function GetIntArray.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetIntArray_0100, Function | MediumTest | Level1)
{
    int32_t resId = 1;
    auto result = context_->GetIntArray(resId);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.number: AaFwk_Ability_Context_GetTheme_0100
 * @tc.name: GetTheme
 * @tc.desc: Verify that function GetTheme.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetTheme_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetTheme();
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.number: AaFwk_Ability_Context_GetPattern_0100
 * @tc.name: GetPattern
 * @tc.desc: Verify that function GetPattern.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetPattern_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetPattern();
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.number: AaFwk_Ability_Context_GetColor_0100
 * @tc.name: GetColor
 * @tc.desc: Verify that function GetColor.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetColor_0100, Function | MediumTest | Level1)
{
    int32_t resId = 1;
    auto result = context_->GetColor(resId);
    EXPECT_EQ(result, INVALID_RESOURCE_VALUE);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetThemeId_0100
 * @tc.name: GetThemeId
 * @tc.desc: Verify that function GetThemeId.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetThemeId_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetThemeId();
    EXPECT_EQ(result, -1);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetDisplayOrientation_0100
 * @tc.name: GetDisplayOrientation
 * @tc.desc: Verify that function GetDisplayOrientation.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetDisplayOrientation_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetDisplayOrientation();
    EXPECT_EQ(result, 0);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetPreferencesDir_0100
 * @tc.name: GetPreferencesDir
 * @tc.desc: Verify that function GetPreferencesDir.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetPreferencesDir_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetPreferencesDir();
    EXPECT_EQ(result, "");
}

/**
 * @tc.number: AaFwk_Ability_Context_SetColorMode_0100
 * @tc.name: SetColorMode and GetColorMode
 * @tc.desc: Verify that function SetColorMode and GetColorMode.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_SetColorMode_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    AbilityThread::AbilityThreadMain(application, abilityRecord, nullptr);
    std::shared_ptr<ContextDeal> deal = std::make_shared<ContextDeal>();
    deal->SetAbilityInfo(abilityInfo);
    context_->AttachBaseContext(deal);
    int32_t mode = 1;
    context_->SetColorMode(mode);
    auto result = context_->GetColorMode();
    EXPECT_EQ(result, 1);
    EXPECT_TRUE(context_->baseContext_ != nullptr);
}

/**
 * @tc.number: AaFwk_Ability_Context_GetMissionId_0100
 * @tc.name: GetMissionId
 * @tc.desc: Verify that function GetMissionId.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_GetMissionId_0100, Function | MediumTest | Level1)
{
    auto result = context_->GetMissionId();
    EXPECT_EQ(result, -1);
}

/**
 * @tc.number: AaFwk_Ability_Context_IsUpdatingConfigurations_0100
 * @tc.name: IsUpdatingConfigurations
 * @tc.desc: Verify that function IsUpdatingConfigurations.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_IsUpdatingConfigurations_0100, Function | MediumTest | Level1)
{
    auto result = context_->IsUpdatingConfigurations();
    EXPECT_FALSE(result);
}

/**
 * @tc.number: AaFwk_Ability_Context_PrintDrawnCompleted_0100
 * @tc.name: PrintDrawnCompleted
 * @tc.desc: Verify that function PrintDrawnCompleted.
 */
HWTEST_F(AbilityContextTest, AaFwk_Ability_Context_PrintDrawnCompleted_0100, Function | MediumTest | Level1)
{
    auto result = context_->PrintDrawnCompleted();
    EXPECT_FALSE(result);
}
}  // namespace AppExecFwk
}  // namespace OHOS
