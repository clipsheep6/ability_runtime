/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define private public
#include "child_main_thread.h"
#undef private
#include "child_process_info.h"
#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "mock_app_mgr_service.h"
#include "mock_bundle_manager.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class ChildMainThreadTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ChildMainThreadTest::SetUpTestCase()
{
    sptr<IRemoteObject> bundleMgrService = sptr<IRemoteObject>(new (std::nothrow) BundleMgrService());
    sptr<IRemoteObject> mockAppMgrService = sptr<IRemoteObject>(new (std::nothrow) MockAppMgrService());
    auto sysMgr = DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == nullptr) {
        GTEST_LOG_(ERROR) << "Failed to get ISystemAbilityManager.";
        return;
    }

    sysMgr->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleMgrService);
    sysMgr->RegisterSystemAbility(APP_MGR_SERVICE_ID, mockAppMgrService);
}

void ChildMainThreadTest::TearDownTestCase()
{}

void ChildMainThreadTest::SetUp()
{}

void ChildMainThreadTest::TearDown()
{}

/**
 * @tc.number: Init_0100
 * @tc.desc: Test Init works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, Init_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "Init_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    ChildProcessInfo info;
    auto ret = thread->Init(runner, info);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: Attach_0100
 * @tc.desc: Test Attach works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, Attach_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "Attach_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    thread->mainHandler_ = std::make_shared<EventHandler>(runner);

    auto ret = thread->Attach();
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: ScheduleLoadJs_0100
 * @tc.desc: Test ScheduleLoadJs_0100 works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, ScheduleLoadJs_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "ScheduleLoadJs_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    std::shared_ptr<EventHandler> handler = std::make_shared<EventHandler>(runner);
    thread->mainHandler_ = handler;

    auto ret = thread->ScheduleLoadJs();
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: HandleLoadJs_0100
 * @tc.desc: Test HandleLoadJs works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, HandleLoadJs_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "HandleLoadJs_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    BundleInfo bundleInfo;
    std::vector<HapModuleInfo> hapModuleInfos;
    HapModuleInfo moduleInfo;
    moduleInfo.name = "entry";
    moduleInfo.moduleName = "entry";
    moduleInfo.moduleType = ModuleType::ENTRY;
    moduleInfo.hapPath = "/data/app/el1/bundle/public/com.ohos.demoprocess/entry";
    moduleInfo.compileMode = CompileMode::ES_MODULE;
    moduleInfo.isStageBasedModel = true;
    hapModuleInfos.push_back(moduleInfo);
    bundleInfo.hapModuleInfos = hapModuleInfos;

    ApplicationInfo applicationInfo;
    applicationInfo.uid = 2001;
    bundleInfo.applicationInfo = applicationInfo;
    
    thread->bundleInfo_ = std::make_shared<BundleInfo>(bundleInfo);
    thread->processInfo_ = std::make_shared<ChildProcessInfo>();
    thread->appMgr_ = sptr<MockAppMgrService>(new (std::nothrow) MockAppMgrService());
    thread->HandleLoadJs();
    ASSERT_NE(thread->runtime_, nullptr);
}

/**
 * @tc.number: ScheduleExitProcessSafely_0100
 * @tc.desc: Test ScheduleExitProcessSafely works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, ScheduleExitProcessSafely_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "ScheduleExitProcessSafely_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    std::shared_ptr<EventHandler> handler = std::make_shared<EventHandler>(runner);
    thread->mainHandler_ = handler;

    auto ret = thread->ScheduleExitProcessSafely();
    EXPECT_TRUE(ret);
}
} // namespace AppExecFwk
} // namespace OHOS
