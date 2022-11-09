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

#include <cstdlib>
#include <gtest/gtest.h>

#define private public
#include "main_thread.h"
#undef private

#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "mock_bundle_manager.h"
#include "process_info.h"
#include "quick_fix_callback_stub.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class QuickFixCallbackImpl : public AppExecFwk::QuickFixCallbackStub {
public:
    QuickFixCallbackImpl() = default;
    virtual ~QuickFixCallbackImpl() = default;

    void OnLoadPatchDone(int32_t resultCode) override
    {
        HILOG_DEBUG("function called.");
    }

    void OnUnloadPatchDone(int32_t resultCode) override
    {
        HILOG_DEBUG("function called.");
    }

    void OnReloadPageDone(int32_t resultCode) override
    {
        HILOG_DEBUG("function called.");
    }
};

class MainThreadTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MainThread> mainThread_ = nullptr;
};

void MainThreadTest::SetUpTestCase()
{
    sptr<IRemoteObject> bundleObject = new (std::nothrow) BundleMgrService();
    auto sysMgr = DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == nullptr) {
        GTEST_LOG_(ERROR) << "Failed to get ISystemAbilityManager.";
        return;
    }

    sysMgr->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);
}

void MainThreadTest::TearDownTestCase()
{}

void MainThreadTest::SetUp()
{
    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    ASSERT_NE(runner, nullptr);

    mainThread_ = sptr<MainThread>(new (std::nothrow) MainThread());
    ASSERT_NE(mainThread_, nullptr);

    mainThread_->Init(runner);
}

void MainThreadTest::TearDown()
{}


/**
 * @tc.name: ScheduleNotifyLoadRepairPatch_0100
 * @tc.desc: schedule notify load repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, ScheduleNotifyLoadRepairPatch_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::string bundleName;
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    auto ret = mainThread_->ScheduleNotifyLoadRepairPatch(bundleName, callback);
    EXPECT_EQ(ret, NO_ERROR);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleNotifyHotReloadPage_0100
 * @tc.desc: schedule notify ace hot reload page.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, ScheduleNotifyHotReloadPage_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    auto ret = mainThread_->ScheduleNotifyHotReloadPage(callback);
    EXPECT_EQ(ret, NO_ERROR);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetHqfFileAndHapPath_0100
 * @tc.desc: get patch file and hap path.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, GetHqfFileAndHapPath_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::string bundleName = "com.ohos.quickfix";
    std::vector<std::pair<std::string, std::string>> fileMap;
    auto ret = mainThread_->GetHqfFileAndHapPath(bundleName, fileMap);
    EXPECT_TRUE(ret);
    ASSERT_EQ(fileMap.size(), 2);
    EXPECT_EQ(fileMap[0].first, "/data/storage/el1/bundle/patch_1000/entry1.hqf");
    EXPECT_EQ(fileMap[0].second, "/data/storage/el1/bundle/entry1");
    EXPECT_EQ(fileMap[1].first, "/data/storage/el1/bundle/patch_1000/entry2.hqf");
    EXPECT_EQ(fileMap[1].second, "/data/storage/el1/bundle/entry2");
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleNotifyUnLoadRepairPatch_0100
 * @tc.desc: schedule notify unload repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, ScheduleNotifyUnLoadRepairPatch_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::string bundleName;
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    auto ret = mainThread_->ScheduleNotifyUnLoadRepairPatch(bundleName, callback);
    EXPECT_EQ(ret, NO_ERROR);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: InitResourceManager_0100
 * @tc.desc: init resourceManager.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, InitResourceManager_0100, TestSize.Level1)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    AppExecFwk::BundleInfo bundleInfo;
    Configuration config;
    bundleInfo.applicationInfo.multiProjects = true;
    mainThread_->InitResourceManager(resourceManager, bundleInfo, config);
    EXPECT_TRUE(resourceManager != nullptr);
    bundleInfo.applicationInfo.multiProjects = false;
    mainThread_->InitResourceManager(resourceManager, bundleInfo, config);
    EXPECT_TRUE(resourceManager != nullptr);

    HapModuleInfo info;
    info.name = "com.ohos.contactsdataability";
    info.moduleName = "entry";
    info.description = "dataability_description";
    info.iconPath = "$media:icon";
    info.deviceTypes = {"smartVision"};
    info.bundleName = "com.ohos.contactsdataability";
    bundleInfo.hapModuleInfos.push_back(info);
    bundleInfo.applicationInfo.multiProjects = true;
    mainThread_->InitResourceManager(resourceManager, bundleInfo, config);
    EXPECT_TRUE(resourceManager != nullptr);

    bundleInfo.applicationInfo.multiProjects = false;
    mainThread_->InitResourceManager(resourceManager, bundleInfo, config);
    EXPECT_TRUE(resourceManager != nullptr);

    info.resourcePath = "/data/app/el1/budle/public/com.ohos.contactsdataability"\
        "/com.ohos.contactsdataability/assets/entry/resources.index";
    bundleInfo.hapModuleInfos.clear();
    bundleInfo.hapModuleInfos.push_back(info);
    mainThread_->InitResourceManager(resourceManager, bundleInfo, config);
    EXPECT_TRUE(resourceManager != nullptr);

    info.hapPath = "/system/app/com.ohos.contactsdataability/Contacts_DataAbility.hap";
    bundleInfo.hapModuleInfos.clear();
    bundleInfo.hapModuleInfos.push_back(info);
    mainThread_->InitResourceManager(resourceManager, bundleInfo, config);
    EXPECT_TRUE(resourceManager != nullptr);

    info.resourcePath = "";
    bundleInfo.hapModuleInfos.clear();
    bundleInfo.hapModuleInfos.push_back(info);
    mainThread_->InitResourceManager(resourceManager, bundleInfo, config);
    EXPECT_TRUE(resourceManager != nullptr);
}

/**
 * @tc.name: HandleLaunchApplication_0100
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, HandleLaunchApplication_0100, TestSize.Level1)
{
    Configuration config;
    AppLaunchData lanchdate;
    ProcessInfo processing("TestProcess", 9999);
    ApplicationInfo appinf;
    appinf.name = "MockTestApplication";
    appinf.moduleSourceDirs.push_back("/hos/lib/libabilitydemo_native.z.so");
    lanchdate.SetApplicationInfo(appinf);
    lanchdate.SetProcessInfo(processing);
    mainThread_->HandleLaunchApplication(lanchdate, config);
    EXPECT_TRUE(mainThread_->application_ != nullptr);
}
} // namespace AppExecFwk
} // namespace OHOS