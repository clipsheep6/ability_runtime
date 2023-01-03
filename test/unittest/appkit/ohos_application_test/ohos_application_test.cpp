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
#include "ability.h"
#include "ability_local_record.h"
#include "ability_record_mgr.h"
#include "ability_thread.h"
#include "application_context.h"
#include "application_impl.h"
#include "application_info.h"
#include "context_deal.h"
#include "context_impl.h"
#include "mock_ability_lifecycle_callbacks.h"
#include "mock_element_callback.h"
#include "mock_i_remote_object.h"
#include "mock_runtime.h"
#include "ohos_application.h"
#include "pac_map.h"
#include "runtime.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class OHOSApplicationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<OHOSApplication> ohosApplication_;
};

void OHOSApplicationTest::SetUpTestCase()
{}

void OHOSApplicationTest::TearDownTestCase()
{}

void OHOSApplicationTest::SetUp()
{
    ohosApplication_ = std::make_shared<OHOSApplication>();
}

void OHOSApplicationTest::TearDown()
{
    ohosApplication_ = nullptr;
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_DispatchAbilitySavedState_0100
* @tc.name: DispatchAbilitySavedState
* @tc.desc: Verify function DispatchAbilitySavedState list abilityLifecycleCallbacks_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_DispatchAbilitySavedState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_DispatchAbilitySavedState_0100 start.";
    PacMap outState;
    ohosApplication_->DispatchAbilitySavedState(outState);
    EXPECT_TRUE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_DispatchAbilitySavedState_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_DispatchAbilitySavedState_0200
* @tc.name: DispatchAbilitySavedState
* @tc.desc: Verify function DispatchAbilitySavedState list abilityLifecycleCallbacks_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_DispatchAbilitySavedState_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_DispatchAbilitySavedState_0200 start.";
    PacMap outState;
    std::shared_ptr<MockAbilityLifecycleCallbacks> abilityLifecycleCallbacks =
        std::make_shared<MockAbilityLifecycleCallbacks>();
    EXPECT_TRUE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    ohosApplication_->abilityLifecycleCallbacks_.emplace_back(abilityLifecycleCallbacks);
    ohosApplication_->DispatchAbilitySavedState(outState);
    EXPECT_TRUE(!ohosApplication_->abilityLifecycleCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_DispatchAbilitySavedState_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnForeground_0100
* @tc.name: OnForeground
* @tc.desc: Verify function OnForeground pointer runtime_  empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnForeground_0100 start.";
    ohosApplication_->OnForeground();
    EXPECT_TRUE(ohosApplication_->runtime_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnForeground_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnForeground_0200
* @tc.name: OnForeground
* @tc.desc: Verify function OnForeground pointer runtime_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnForeground_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnForeground_0200 start.";
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    ohosApplication_->OnForeground();
    EXPECT_TRUE(ohosApplication_->runtime_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnForeground_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnBackground_0100
* @tc.name: OnBackground
* @tc.desc: Verify function OnBackground pointer runtime_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnBackground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnBackground_0100 start.";
    ohosApplication_->OnBackground();
    EXPECT_TRUE(ohosApplication_->runtime_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnBackground_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnBackground_0200
* @tc.name: OnBackground
* @tc.desc: Verify function OnBackground pointer runtime_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnBackground_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnBackground_0200 start.";
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    ohosApplication_->OnBackground();
    EXPECT_TRUE(ohosApplication_->runtime_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnBackground_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_DumpApplication_0100
* @tc.name: DumpApplication
* @tc.desc: Verify function DumpApplication pointer record not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_DumpApplication_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_DumpApplication_0100 start.";
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = nullptr;
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(info, token);
    ohosApplication_->abilityRecordMgr_->abilityRecords_.emplace(token, record);
    ohosApplication_->DumpApplication();
    EXPECT_TRUE(record != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_DumpApplication_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_DumpApplication_0200
* @tc.name: DumpApplication
* @tc.desc: Verify function DumpApplication pointer abilityInfo not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_DumpApplication_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_DumpApplication_0200 start.";
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    std::shared_ptr<AbilityLocalRecord> record =  std::make_shared<AbilityLocalRecord>(info, token);
    info->permissions.push_back(std::string("abc"));
    ohosApplication_->abilityRecordMgr_->abilityRecords_.emplace(token, record);
    ohosApplication_->DumpApplication();
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_DumpApplication_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_DumpApplication_0300
* @tc.name: DumpApplication
* @tc.desc: Verify function DumpApplication pointer applicationInfoPtr not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_DumpApplication_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_DumpApplication_0300 start.";
    ohosApplication_->DumpApplication();
    auto contextDeal = std::make_shared<ContextDeal>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    contextDeal->SetApplicationInfo(appInfo);
    ohosApplication_->AttachBaseContext(contextDeal);
    ohosApplication_->DumpApplication();
    EXPECT_TRUE(ohosApplication_->GetApplicationInfo() != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_DumpApplication_0300 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetRuntime_0100
* @tc.name: SetRuntime
* @tc.desc: Verify function SetRuntime pointer runtime empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetRuntime_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetRuntime_0100 start.";
    std::unique_ptr<AbilityRuntime::Runtime> runtime = nullptr;
    ohosApplication_->SetRuntime(std::move(runtime));
    EXPECT_TRUE(runtime == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetRuntime_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetRuntime_0200
* @tc.name: SetRuntime
* @tc.desc: Verify function SetRuntime pointer runtime_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetRuntime_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetRuntime_0200 start.";
    std::unique_ptr<AbilityRuntime::Runtime> runtime = std::make_unique<AbilityRuntime::MockRuntime>();
    EXPECT_TRUE(runtime != nullptr);
    ohosApplication_->SetRuntime(std::move(runtime));
    EXPECT_TRUE(ohosApplication_->runtime_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetRuntime_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetApplicationContext_0100
* @tc.name: SetApplicationContext
* @tc.desc: Verify function SetApplicationContext pointer abilityRuntimeContext_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetApplicationContext_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetApplicationContext_0100 start.";
    std::shared_ptr<AbilityRuntime::ApplicationContext> abilityRuntimeContext = nullptr;
    ohosApplication_->SetApplicationContext(abilityRuntimeContext);
    EXPECT_TRUE(ohosApplication_->abilityRuntimeContext_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetApplicationContext_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetApplicationContext_0200
* @tc.name: SetApplicationContext
* @tc.desc: Verify function SetApplicationContext pointer abilityRuntimeContext_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetApplicationContext_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetApplicationContext_0200 start.";
    std::shared_ptr<AbilityRuntime::ApplicationContext> abilityRuntimeContext =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    EXPECT_TRUE(abilityRuntimeContext != nullptr);
    ohosApplication_->SetApplicationContext(abilityRuntimeContext);
    EXPECT_TRUE(ohosApplication_->abilityRuntimeContext_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetApplicationContext_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetAbilityRecordMgr_0100
* @tc.name: SetAbilityRecordMgr
* @tc.desc: Verify function SetAbilityRecordMgr pointer abilityRecordMgr_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetAbilityRecordMgr_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetAbilityRecordMgr_0100 start.";
    std::shared_ptr<AbilityRecordMgr> abilityRecordMgr = nullptr;
    ohosApplication_->SetAbilityRecordMgr(abilityRecordMgr);
    EXPECT_TRUE(ohosApplication_->abilityRecordMgr_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetAbilityRecordMgr_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetAbilityRecordMgr_0200
* @tc.name: SetAbilityRecordMgr
* @tc.desc: Verify function SetAbilityRecordMgr pointer abilityRecordMgr_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetAbilityRecordMgr_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetAbilityRecordMgr_0200 start.";
    std::shared_ptr<AbilityRecordMgr> abilityRecordMgr = std::make_shared<AbilityRecordMgr>();
    EXPECT_TRUE(abilityRecordMgr != nullptr);
    ohosApplication_->SetAbilityRecordMgr(abilityRecordMgr);
    EXPECT_TRUE(ohosApplication_->abilityRecordMgr_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetAbilityRecordMgr_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_RegisterAbilityLifecycleCallbacks_0100
* @tc.name: RegisterAbilityLifecycleCallbacks
* @tc.desc: Verify function RegisterAbilityLifecycleCallbacks list abilityLifecycleCallbacks_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_RegisterAbilityLifecycleCallbacks_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_RegisterAbilityLifecycleCallbacks_0100 start.";
    std::shared_ptr<MockAbilityLifecycleCallbacks> callBack = nullptr;
    EXPECT_TRUE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    ohosApplication_->RegisterAbilityLifecycleCallbacks(callBack);
    EXPECT_TRUE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_RegisterAbilityLifecycleCallbacks_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_RegisterAbilityLifecycleCallbacks_0200
* @tc.name: RegisterAbilityLifecycleCallbacks
* @tc.desc: Verify function RegisterAbilityLifecycleCallbacks list abilityLifecycleCallbacks_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_RegisterAbilityLifecycleCallbacks_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_RegisterAbilityLifecycleCallbacks_0200 start.";
    EXPECT_TRUE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    std::shared_ptr<MockAbilityLifecycleCallbacks> callBack = std::make_shared<MockAbilityLifecycleCallbacks>();
    ohosApplication_->RegisterAbilityLifecycleCallbacks(callBack);
    EXPECT_FALSE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_RegisterAbilityLifecycleCallbacks_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_UnregisterAbilityLifecycleCallbacks_0100
* @tc.name: UnregisterAbilityLifecycleCallbacks
* @tc.desc: Verify function UnregisterAbilityLifecycleCallbacks list abilityLifecycleCallbacks_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_UnregisterAbilityLifecycleCallbacks_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_UnregisterAbilityLifecycleCallbacks_0100 start.";
    EXPECT_TRUE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    std::shared_ptr<MockAbilityLifecycleCallbacks> callBack = nullptr;
    ohosApplication_->UnregisterAbilityLifecycleCallbacks(callBack);
    EXPECT_TRUE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_UnregisterAbilityLifecycleCallbacks_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_UnregisterAbilityLifecycleCallbacks_0200
* @tc.name: UnregisterAbilityLifecycleCallbacks
* @tc.desc: Verify function UnregisterAbilityLifecycleCallbacks list abilityLifecycleCallbacks_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_UnregisterAbilityLifecycleCallbacks_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_UnregisterAbilityLifecycleCallbacks_0200 start.";
    EXPECT_TRUE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    std::shared_ptr<MockAbilityLifecycleCallbacks> callBack = std::make_shared<MockAbilityLifecycleCallbacks>();
    ohosApplication_->abilityLifecycleCallbacks_.emplace_back(callBack);
    EXPECT_FALSE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    ohosApplication_->UnregisterAbilityLifecycleCallbacks(callBack);
    EXPECT_TRUE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_UnregisterAbilityLifecycleCallbacks_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnAbilityStart_0100
* @tc.name: OnAbilityStart
* @tc.desc: Verify function OnAbilityStart pointer ability empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnAbilityStart_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityStart_0100 start.";
    std::shared_ptr<Ability> ability = nullptr;
    EXPECT_TRUE(ability == nullptr);
    ohosApplication_->OnAbilityStart(ability);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityStart_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnAbilityStart_0200
* @tc.name: OnAbilityStart
* @tc.desc: Verify function OnAbilityStart pointer abilityLifecycleCallbacks_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnAbilityStart_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityStart_0200 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    ohosApplication_->OnAbilityStart(ability);
    EXPECT_TRUE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    std::shared_ptr<MockAbilityLifecycleCallbacks> callback1 = std::make_shared<MockAbilityLifecycleCallbacks>();
    std::shared_ptr<MockAbilityLifecycleCallbacks> callback2 = nullptr;
    ohosApplication_->abilityLifecycleCallbacks_.emplace_back(callback1);
    ohosApplication_->abilityLifecycleCallbacks_.emplace_back(callback2);
    ohosApplication_->OnAbilityStart(ability);
    EXPECT_FALSE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityStart_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnAbilityInactive_0100
* @tc.name: OnAbilityInactive
* @tc.desc: Verify function OnAbilityInactive pointer ability empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnAbilityInactive_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityInactive_0100 start.";
    std::shared_ptr<Ability> ability = nullptr;
    ohosApplication_->OnAbilityInactive(ability);
    EXPECT_TRUE(ability == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityInactive_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnAbilityInactive_0200
* @tc.name: OnAbilityInactive
* @tc.desc: Verify function OnAbilityInactive pointer abilityLifecycleCallbacks_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnAbilityInactive_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityInactive_0200 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    ohosApplication_->OnAbilityInactive(ability);
    EXPECT_TRUE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    std::shared_ptr<MockAbilityLifecycleCallbacks> callback1 = std::make_shared<MockAbilityLifecycleCallbacks>();
    std::shared_ptr<MockAbilityLifecycleCallbacks> callback2 = nullptr;
    ohosApplication_->abilityLifecycleCallbacks_.emplace_back(callback1);
    ohosApplication_->abilityLifecycleCallbacks_.emplace_back(callback2);
    ohosApplication_->OnAbilityInactive(ability);
    EXPECT_FALSE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityInactive_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnAbilityBackground_0100
* @tc.name: OnAbilityBackground
* @tc.desc: Verify function OnAbilityBackground pointer ability empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnAbilityBackground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityBackground_0100 start.";
    std::shared_ptr<Ability> ability = nullptr;
    ohosApplication_->OnAbilityBackground(ability);
    EXPECT_TRUE(ability == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityBackground_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnAbilityBackground_0200
* @tc.name: OnAbilityBackground
* @tc.desc: Verify function OnAbilityBackground pointer abilityLifecycleCallbacks_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnAbilityBackground_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityBackground_0200 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    ohosApplication_->OnAbilityBackground(ability);
    EXPECT_TRUE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    std::shared_ptr<MockAbilityLifecycleCallbacks> callback = std::make_shared<MockAbilityLifecycleCallbacks>();
    ohosApplication_->abilityLifecycleCallbacks_.emplace_back(callback);
    ohosApplication_->OnAbilityBackground(ability);
    EXPECT_TRUE(!ohosApplication_->abilityLifecycleCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityBackground_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnAbilityForeground_0100
* @tc.name: OnAbilityForeground
* @tc.desc: Verify function OnAbilityForeground pointer ability empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnAbilityForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityForeground_0100 start.";
    std::shared_ptr<Ability> ability = nullptr;
    ohosApplication_->OnAbilityForeground(ability);
    EXPECT_TRUE(ability == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityForeground_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnAbilityForeground_0200
* @tc.name: OnAbilityForeground
* @tc.desc: Verify function OnAbilityForeground pointer abilityLifecycleCallbacks_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnAbilityForeground_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityForeground_0200 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    ohosApplication_->OnAbilityForeground(ability);
    EXPECT_TRUE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    std::shared_ptr<MockAbilityLifecycleCallbacks> callback = std::make_shared<MockAbilityLifecycleCallbacks>();
    ohosApplication_->abilityLifecycleCallbacks_.emplace_back(callback);
    ohosApplication_->OnAbilityForeground(ability);
    EXPECT_TRUE(!ohosApplication_->abilityLifecycleCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityForeground_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnAbilityActive_0100
* @tc.name: OnAbilityActive
* @tc.desc: Verify function OnAbilityActive pointer ability empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnAbilityActive_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityActive_0100 start.";
    std::shared_ptr<Ability> ability = nullptr;
    ohosApplication_->OnAbilityActive(ability);
    EXPECT_TRUE(ability == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityActive_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnAbilityActive_0200
* @tc.name: OnAbilityActive
* @tc.desc: Verify function OnAbilityActive pointer abilityLifecycleCallbacks_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnAbilityActive_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityActive_0200 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    ohosApplication_->OnAbilityActive(ability);
    EXPECT_TRUE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    std::shared_ptr<MockAbilityLifecycleCallbacks> callback = std::make_shared<MockAbilityLifecycleCallbacks>();
    ohosApplication_->abilityLifecycleCallbacks_.emplace_back(callback);
    ohosApplication_->OnAbilityActive(ability);
    EXPECT_TRUE(!ohosApplication_->abilityLifecycleCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityActive_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnAbilityStop_0100
* @tc.name: OnAbilityStop
* @tc.desc: Verify function OnAbilityStop pointer ability empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnAbilityStop_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityStop_0100 start.";
    std::shared_ptr<Ability> ability = nullptr;
    ohosApplication_->OnAbilityStop(ability);
    EXPECT_TRUE(ability == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityStop_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnAbilityStop_0200
* @tc.name: OnAbilityStop
* @tc.desc: Verify function OnAbilityStop pointer abilityLifecycleCallbacks_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnAbilityStop_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityStop_0200 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    ohosApplication_->OnAbilityStop(ability);
    EXPECT_TRUE(ohosApplication_->abilityLifecycleCallbacks_.empty());
    std::shared_ptr<MockAbilityLifecycleCallbacks> callback = std::make_shared<MockAbilityLifecycleCallbacks>();
    ohosApplication_->abilityLifecycleCallbacks_.emplace_back(callback);
    ohosApplication_->OnAbilityStop(ability);
    EXPECT_TRUE(!ohosApplication_->abilityLifecycleCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilityStop_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_RegisterElementsCallbacks_0100
* @tc.name: RegisterElementsCallbacks
* @tc.desc: Verify function RegisterElementsCallbacks list elementsCallbacks_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_RegisterElementsCallbacks_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_RegisterElementsCallbacks_0100 start.";
    std::shared_ptr<MockElementsCallback> callback = nullptr;
    EXPECT_TRUE(ohosApplication_->elementsCallbacks_.empty());
    ohosApplication_->RegisterElementsCallbacks(callback);
    EXPECT_TRUE(ohosApplication_->elementsCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_RegisterElementsCallbacks_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_RegisterElementsCallbacks_0200
* @tc.name: RegisterElementsCallbacks
* @tc.desc: Verify function RegisterElementsCallbacks list elementsCallbacks_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_RegisterElementsCallbacks_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_RegisterElementsCallbacks_0200 start.";
    std::shared_ptr<MockElementsCallback> callback = std::make_shared<MockElementsCallback>();
    EXPECT_TRUE(ohosApplication_->elementsCallbacks_.empty());
    ohosApplication_->RegisterElementsCallbacks(callback);
    EXPECT_TRUE(!ohosApplication_->elementsCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_RegisterElementsCallbacks_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_UnregisterElementsCallbacks_0100
* @tc.name: UnregisterElementsCallbacks
* @tc.desc: Verify function UnregisterElementsCallbacks list elementsCallbacks_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_UnregisterElementsCallbacks_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_UnregisterElementsCallbacks_0100 start.";
    std::shared_ptr<MockElementsCallback> callback = nullptr;
    EXPECT_TRUE(ohosApplication_->elementsCallbacks_.empty());
    ohosApplication_->UnregisterElementsCallbacks(callback);
    EXPECT_TRUE(ohosApplication_->elementsCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_UnregisterElementsCallbacks_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_UnregisterElementsCallbacks_0200
* @tc.name: UnregisterElementsCallbacks
* @tc.desc: Verify function UnregisterElementsCallbacks list elementsCallbacks_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_UnregisterElementsCallbacks_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_UnregisterElementsCallbacks_0200 start.";
    std::shared_ptr<MockElementsCallback> callback = std::make_shared<MockElementsCallback>();
    EXPECT_TRUE(ohosApplication_->elementsCallbacks_.empty());
    ohosApplication_->elementsCallbacks_.emplace_back(callback);
    EXPECT_FALSE(ohosApplication_->elementsCallbacks_.empty());
    ohosApplication_->UnregisterElementsCallbacks(callback);
    EXPECT_TRUE(ohosApplication_->elementsCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_UnregisterElementsCallbacks_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0100
* @tc.name: OnConfigurationUpdated
* @tc.desc: Verify function OnConfigurationUpdated pointer abilityRecordMgr_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0100 start.";
    Configuration config;
    ohosApplication_->OnConfigurationUpdated(config);
    EXPECT_TRUE(ohosApplication_->abilityRecordMgr_ == nullptr);
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ohosApplication_->OnConfigurationUpdated(config);
    EXPECT_TRUE(ohosApplication_->configuration_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0200
* @tc.name: OnConfigurationUpdated
* @tc.desc: Verify function OnConfigurationUpdated pointer abilityRecord not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0200 start.";
    Configuration config;
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info =  nullptr;
    std::shared_ptr<AbilityLocalRecord> abilityRecord =  std::make_shared<AbilityLocalRecord>(info, token);
    ohosApplication_->abilityRecordMgr_->abilityRecords_.emplace(token, abilityRecord);
    sptr<AbilityThread> abilityThread = new (std::nothrow) AbilityThread();
    abilityRecord->SetAbilityThread(abilityThread);
    ohosApplication_->OnConfigurationUpdated(config);
    EXPECT_TRUE(!ohosApplication_->abilityRecordMgr_->abilityRecords_.empty());
    EXPECT_TRUE(abilityRecord != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0300
* @tc.name: OnConfigurationUpdated
* @tc.desc: Verify function OnConfigurationUpdated map abilityStages_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0300 start.";
    Configuration config;
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    std::string moduleName = "entry";
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStages = std::make_shared<AbilityRuntime::AbilityStage>();
    ohosApplication_->abilityStages_.emplace(moduleName, abilityStages);
    ohosApplication_->OnConfigurationUpdated(config);
    EXPECT_TRUE(!ohosApplication_->abilityStages_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0300 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0400
* @tc.name: OnConfigurationUpdated
* @tc.desc: Verify function OnConfigurationUpdated variable configurationUpdated_ true
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0400 start.";
    Configuration config;
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    EXPECT_TRUE(ohosApplication_->elementsCallbacks_.empty());
    std::shared_ptr<MockElementsCallback> callback = std::make_shared<MockElementsCallback>();
    ohosApplication_->elementsCallbacks_.emplace_back(callback);
    EXPECT_FALSE(callback->configurationUpdated_);
    ohosApplication_->OnConfigurationUpdated(config);
    EXPECT_TRUE(callback != nullptr);
    EXPECT_FALSE(ohosApplication_->elementsCallbacks_.empty());
    EXPECT_TRUE(callback->configurationUpdated_);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0400 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0500
* @tc.name: OnConfigurationUpdated
* @tc.desc: Verify function OnConfigurationUpdated list elementsCallbacks_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0500 start.";
    Configuration config;
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    EXPECT_TRUE(ohosApplication_->elementsCallbacks_.empty());
    ohosApplication_->OnConfigurationUpdated(config);
    EXPECT_TRUE(ohosApplication_->elementsCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0500 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0100
* @tc.name: OnMemoryLevel
* @tc.desc: Verify function OnMemoryLevel pointer abilityRecord not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0100 start.";
    constexpr int32_t level = 1;
    ohosApplication_->OnMemoryLevel(level);
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    std::shared_ptr<AbilityInfo> info = nullptr;
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token);
    EXPECT_TRUE(abilityRecord != nullptr);
    ohosApplication_->abilityRecordMgr_->abilityRecords_.emplace(token, abilityRecord);
    sptr<AbilityThread> abilityThread = new (std::nothrow) AbilityThread();
    abilityRecord->SetAbilityThread(abilityThread);
    ohosApplication_->OnMemoryLevel(level);
    EXPECT_FALSE(ohosApplication_->abilityRecordMgr_->abilityRecords_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0200
* @tc.name: OnMemoryLevel
* @tc.desc: Verify function OnMemoryLevel map abilityStages_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0200 start.";
    constexpr int32_t level = 1;
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    std::string moduleName1 = "entry1";
    std::string moduleName2 = "entry2";
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStages1 = std::make_shared<AbilityRuntime::AbilityStage>();
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStages2 = nullptr;
    ohosApplication_->abilityStages_.emplace(moduleName1, abilityStages1);
    ohosApplication_->abilityStages_.emplace(moduleName2, abilityStages2);
    ohosApplication_->OnMemoryLevel(level);
    EXPECT_TRUE(!ohosApplication_->abilityStages_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0300
* @tc.name: OnMemoryLevel
* @tc.desc: Verify function OnMemoryLevel variable onMemoryLevel_ true
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0300 start.";
    constexpr int32_t level = 1;
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    EXPECT_TRUE(ohosApplication_->elementsCallbacks_.empty());
    std::shared_ptr<MockElementsCallback> callback1 = std::make_shared<MockElementsCallback>();
    std::shared_ptr<MockElementsCallback> callback2 = nullptr;
    ohosApplication_->elementsCallbacks_.emplace_back(callback1);
    ohosApplication_->elementsCallbacks_.emplace_back(callback2);
    EXPECT_FALSE(callback1->onMemoryLevel_);
    ohosApplication_->OnMemoryLevel(level);
    EXPECT_FALSE(ohosApplication_->elementsCallbacks_.empty());
    EXPECT_TRUE(callback1->onMemoryLevel_);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0300 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0400
* @tc.name: OnMemoryLevel
* @tc.desc: Verify function OnMemoryLevel variable onMemoryLevel_ true
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0400 start.";
    constexpr int32_t level = 1;
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    EXPECT_TRUE(ohosApplication_->elementsCallbacks_.empty());
    ohosApplication_->OnMemoryLevel(level);
    EXPECT_TRUE(ohosApplication_->elementsCallbacks_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0400 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnStart_0100
* @tc.name: OnStart
* @tc.desc: Verify function OnStart called
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnStart_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnStart_0100 start.";
    ohosApplication_->OnStart();
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnStart_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnTerminate_0100
* @tc.name: OnTerminate
* @tc.desc: Verify function OnTerminate called
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnTerminate_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnTerminate_0100 start.";
    ohosApplication_->OnTerminate();
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnTerminate_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnAbilitySaveState_0100
* @tc.name: OnAbilitySaveState
* @tc.desc: Verify function OnAbilitySaveState called
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnAbilitySaveState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilitySaveState_0100 start.";
    const PacMap outState;
    ohosApplication_->OnAbilitySaveState(outState);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnAbilitySaveState_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0100
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage pointer abilityRecord empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0100 start.";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = nullptr;
    ohosApplication_->AddAbilityStage(abilityRecord);
    EXPECT_TRUE(abilityRecord == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0200
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage pointer abilityInfo empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0200 start.";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    ohosApplication_->AddAbilityStage(abilityRecord);
    EXPECT_TRUE(abilityInfo == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0300
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage pointer hapModuleInfo empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0300 start.";
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = nullptr;
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token);
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    ohosApplication_->AddAbilityStage(abilityRecord);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0300 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0400
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage abilityRecord->GetWant() not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0400 start.";
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->applicationInfo.multiProjects = true;
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token);
    auto want = std::make_shared<AAFwk::Want>();
    abilityRecord->SetWant(want);
    ohosApplication_->AddAbilityStage(abilityRecord);
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0400 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0500
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage pointer abilityStages not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0500 start.";
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = nullptr;
    std::string moduleName = "entry";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token);
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStages = std::make_shared<AbilityRuntime::AbilityStage>();
    ohosApplication_->abilityStages_.emplace(moduleName, abilityStages);
    ohosApplication_->AddAbilityStage(abilityRecord);
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    EXPECT_TRUE(abilityStages != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0500 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0600
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage pointer token empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0600 start.";
    sptr<Notification::MockIRemoteObject> token;
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->moduleName = "entry";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token);
    ohosApplication_->AddAbilityStage(abilityRecord);
    ohosApplication_->AddAbilityStage(abilityRecord);
    EXPECT_TRUE(token == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0600 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0700
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage pointer token not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0700 start.";
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token);
    abilityRecord->token_ = new (std::nothrow) Notification::MockIRemoteObject();
    ohosApplication_->AddAbilityStage(abilityRecord);
    EXPECT_TRUE(token != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0700 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0800
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage pointer abilityRuntimeContext_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0800 start.";
    HapModuleInfo hapModuleInfo;
    ohosApplication_->AddAbilityStage(hapModuleInfo);
    EXPECT_TRUE(ohosApplication_->abilityRuntimeContext_ == nullptr);
    EXPECT_FALSE(ohosApplication_->AddAbilityStage(hapModuleInfo));
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0800 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0900
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage pointer runtime_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0900 start.";
    HapModuleInfo hapModuleInfo;
    ohosApplication_->abilityRuntimeContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    ohosApplication_->AddAbilityStage(hapModuleInfo);
    EXPECT_TRUE(ohosApplication_->runtime_ == nullptr);
    EXPECT_FALSE(ohosApplication_->AddAbilityStage(hapModuleInfo));
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0900 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_01000
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage map abilityStages_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_01000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_01000 start.";
    HapModuleInfo hapModuleInfo;
    std::string moduleName = "entry";
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStages = std::make_shared<AbilityRuntime::AbilityStage>();
    ohosApplication_->abilityStages_.emplace(moduleName, abilityStages);
    ohosApplication_->abilityRuntimeContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    ohosApplication_->AddAbilityStage(hapModuleInfo);
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    EXPECT_FALSE(ohosApplication_->AddAbilityStage(hapModuleInfo));
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_01000 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_01100
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage variable moduleInfo empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_01100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_01100 start.";
    HapModuleInfo hapModuleInfo;
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    ohosApplication_->abilityRuntimeContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    ohosApplication_->abilityRuntimeContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    ohosApplication_->AddAbilityStage(hapModuleInfo);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->multiProjects = true;
    contextImpl->SetApplicationInfo(appInfo);
    ohosApplication_->abilityRuntimeContext_->AttachContextImpl(contextImpl);
    ohosApplication_->AddAbilityStage(hapModuleInfo);
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_01100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_01200
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage abilityRuntimeContext_->GetApplicationInfo() true
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_01200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_01200 start.";
    HapModuleInfo hapModuleInfo;
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    ohosApplication_->abilityRuntimeContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    ohosApplication_->abilityRuntimeContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    ohosApplication_->AddAbilityStage(hapModuleInfo);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->multiProjects = false;
    contextImpl->SetApplicationInfo(appInfo);
    ohosApplication_->abilityRuntimeContext_->AttachContextImpl(contextImpl);
    ohosApplication_->AddAbilityStage(hapModuleInfo);
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_01200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0100
* @tc.name: CleanAbilityStage
* @tc.desc: Verify function CleanAbilityStage pointer abilityInfo empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0100 start.";
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    ohosApplication_->CleanAbilityStage(token, abilityInfo);
    EXPECT_TRUE(abilityInfo == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0200
* @tc.name: CleanAbilityStage
* @tc.desc: Verify function CleanAbilityStage pointer token empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0200 start.";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<Notification::MockIRemoteObject> token = nullptr;
    ohosApplication_->CleanAbilityStage(token, abilityInfo);
    EXPECT_TRUE(token == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0300
* @tc.name: CleanAbilityStage
* @tc.desc: Verify function CleanAbilityStage map abilityRecords_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0300 start.";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    abilityInfo->moduleName = "entry";
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage = std::make_shared<AbilityRuntime::AbilityStage>();
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    ohosApplication_->abilityStages_.emplace(abilityInfo->moduleName, abilityStage);
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    ohosApplication_->CleanAbilityStage(token, abilityInfo);
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0300 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_GetAppContext_0100
* @tc.name: GetAppContext
* @tc.desc: Verify function GetAppContext pointer token empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_GetAppContext_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetAppContext_0100 start.";
    auto context = ohosApplication_->GetAppContext();
    EXPECT_TRUE(context == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetAppContext_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_GetRuntime_0100
* @tc.name: GetRuntime
* @tc.desc: Verify function GetRuntime pointer token empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_GetRuntime_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetRuntime_0100 start.";
    auto &runtime = ohosApplication_->GetRuntime();
    EXPECT_TRUE(runtime == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetRuntime_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetConfiguration_0100
* @tc.name: SetConfiguration
* @tc.desc: Verify function SetConfiguration pointer token empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetConfiguration_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetConfiguration_0100 start.";
    Configuration config;
    ohosApplication_->configuration_ = nullptr;
    ohosApplication_->SetConfiguration(config);
    ohosApplication_->SetConfiguration(config);
    EXPECT_TRUE(ohosApplication_->configuration_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetConfiguration_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_ScheduleAcceptWant_0100
* @tc.name: ScheduleAcceptWant
* @tc.desc: Verify function ScheduleAcceptWant pointer abilityStage not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_ScheduleAcceptWant_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_ScheduleAcceptWant_0100 start.";
    Want want;
    std::string flag = "";
    std::string moduleName = "entry";
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage = std::make_shared<AbilityRuntime::AbilityStage>();
    ohosApplication_->abilityStages_.emplace(moduleName, abilityStage);
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    ohosApplication_->ScheduleAcceptWant(want, moduleName, flag);
    EXPECT_TRUE(abilityStage != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_ScheduleAcceptWant_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_GetConfiguration_0100
* @tc.name: GetConfiguration
* @tc.desc: Verify function GetConfiguration pointer configuration_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_GetConfiguration_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetConfiguration_0100 start.";
    Configuration config;
    ohosApplication_->configuration_ = nullptr;
    ohosApplication_->GetConfiguration();
    EXPECT_TRUE(ohosApplication_->configuration_ == nullptr);
    ohosApplication_->SetConfiguration(config);
    ohosApplication_->GetConfiguration();
    EXPECT_TRUE(ohosApplication_->configuration_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetConfiguration_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetExtensionTypeMap_0100
* @tc.name: SetExtensionTypeMap
* @tc.desc: Verify function SetExtensionTypeMap map extensionTypeMap_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetExtensionTypeMap_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetExtensionTypeMap_0100 start.";
    const std::string name = "entry";
    constexpr int32_t id = 1;
    std::map<int32_t, std::string> map;
    map.emplace(id, name);
    ohosApplication_->SetExtensionTypeMap(map);
    EXPECT_FALSE(ohosApplication_->extensionTypeMap_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetExtensionTypeMap_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_NotifyLoadRepairPatch_0100
* @tc.name: NotifyLoadRepairPatch
* @tc.desc: Verify function NotifyLoadRepairPatch pointer runtime_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_NotifyLoadRepairPatch_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyLoadRepairPatch_0100 start.";
    const std::string hqfFile = "hqfFile";
    const std::string hapPat = "hapPat";
    EXPECT_TRUE(ohosApplication_->NotifyLoadRepairPatch(hqfFile, hapPat));
    EXPECT_TRUE(ohosApplication_->runtime_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyLoadRepairPatch_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_NotifyLoadRepairPatch_0200
* @tc.name: NotifyLoadRepairPatch
* @tc.desc: Verify function NotifyLoadRepairPatch function LoadRepairPatch called
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_NotifyLoadRepairPatch_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyLoadRepairPatch_0200 start.";
    const std::string hqfFile = "hqfFile";
    const std::string hapPath = "hapPath";
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    ohosApplication_->NotifyLoadRepairPatch(hqfFile, hapPath);
    EXPECT_TRUE(ohosApplication_->runtime_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyLoadRepairPatch_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_NotifyHotReloadPage_0100
* @tc.name: NotifyHotReloadPage
* @tc.desc: Verify function NotifyHotReloadPage pointer runtime_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_NotifyHotReloadPage_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyHotReloadPage_0100 start.";
    ohosApplication_->NotifyHotReloadPage();
    EXPECT_TRUE(ohosApplication_->runtime_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyHotReloadPage_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_NotifyHotReloadPage_0200
* @tc.name: NotifyHotReloadPage
* @tc.desc: Verify function NotifyHotReloadPage pointer runtime_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_NotifyHotReloadPage_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyHotReloadPage_0200 start.";
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    ohosApplication_->NotifyHotReloadPage();
    EXPECT_TRUE(ohosApplication_->runtime_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyHotReloadPage_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_NotifyUnLoadRepairPatch_0100
* @tc.name: NotifyUnLoadRepairPatch
* @tc.desc: Verify function NotifyHotReloadPage pointer runtime_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_NotifyUnLoadRepairPatch_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyUnLoadRepairPatch_0100 start.";
    std::string hqfFile = "hqfFile";
    ohosApplication_->NotifyUnLoadRepairPatch(hqfFile);
    EXPECT_TRUE(ohosApplication_->runtime_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyUnLoadRepairPatch_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_NotifyUnLoadRepairPatch_0200
* @tc.name: NotifyUnLoadRepairPatch
* @tc.desc: Verify function NotifyHotReloadPage pointer runtime_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_NotifyUnLoadRepairPatch_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyUnLoadRepairPatch_0200 start.";
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    std::string hqfFile = "entry";
    ohosApplication_->NotifyUnLoadRepairPatch(hqfFile);
    EXPECT_TRUE(ohosApplication_->runtime_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyUnLoadRepairPatch_0200 end.";
}
}  // namespace AppExecFwk
}  // namespace OHOS