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
#include "free_install_observer_manager.h"
#include "mock_native_engine.h"
#include "mock_native_value.h"
#include "js_free_install_observer.h"
#undef private

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AAFwk {
class FreeInstallObserverManagerTest : public testing::Test {
public:
    FreeInstallObserverManagerTest()
    {}
    ~FreeInstallObserverManagerTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FreeInstallObserverManagerTest::SetUpTestCase(void)
{
    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->observerList_.clear();
}

void FreeInstallObserverManagerTest::TearDownTestCase(void)
{}

void FreeInstallObserverManagerTest::SetUp()
{}

void FreeInstallObserverManagerTest::TearDown()
{}

HWTEST_F(FreeInstallObserverManagerTest, FreeInstallObserverManager_AddObserver_001, TestSize.Level1)
{
    MockNativeEngine engine;
    sptr<AbilityRuntime::IFreeInstallObserver> observer = new JsFreeInstallObserver(engine);
    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->AddObserver(observer);
    int size = DelayedSingleton<FreeInstallObserverManager>::GetInstance()->observerList_.size();
    EXPECT_EQ(size, 1);
}

HWTEST_F(FreeInstallObserverManagerTest, FreeInstallObserverManager_AddObserver_002, TestSize.Level1)
{
    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->AddObserver(nullptr);
    int size = DelayedSingleton<FreeInstallObserverManager>::GetInstance()->observerList_.size();
    EXPECT_EQ(size, 0);
}

HWTEST_F(FreeInstallObserverManagerTest, FreeInstallObserverManager_AddObserver_003, TestSize.Level1)
{
    MockNativeEngine engine;
    sptr<AbilityRuntime::IFreeInstallObserver> observer = new JsFreeInstallObserver(engine);
    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->AddObserver(observer);
    int size = DelayedSingleton<FreeInstallObserverManager>::GetInstance()->observerList_.size();
    EXPECT_EQ(size, 1);

    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->AddObserver(observer);
    size = DelayedSingleton<FreeInstallObserverManager>::GetInstance()->observerList_.size();
    EXPECT_EQ(size, 1);
}

HWTEST_F(FreeInstallObserverManagerTest, FreeInstallObserverManager_RemoveObserver_001, TestSize.Level1)
{
    MockNativeEngine engine;
    sptr<AbilityRuntime::IFreeInstallObserver> observer = new JsFreeInstallObserver(engine);
    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->AddObserver(observer);
    int size = DelayedSingleton<FreeInstallObserverManager>::GetInstance()->observerList_.size();
    EXPECT_EQ(size, 1);
    
    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->RemoveObserver(observer);
    size = DelayedSingleton<FreeInstallObserverManager>::GetInstance()->observerList_.size();
    EXPECT_EQ(size, 0);
}

HWTEST_F(FreeInstallObserverManagerTest, FreeInstallObserverManager_RemoveObserver_002, TestSize.Level1)
{
    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->observerList_.clear();

    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->RemoveObserver(nullptr);
    int size = DelayedSingleton<FreeInstallObserverManager>::GetInstance()->observerList_.size();
    EXPECT_EQ(size, 0);
}

HWTEST_F(FreeInstallObserverManagerTest, FreeInstallObserverManager_RemoveObserver_003, TestSize.Level1)
{
    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->observerList_.clear();

    MockNativeEngine engine;
    sptr<AbilityRuntime::IFreeInstallObserver> observer = new JsFreeInstallObserver(engine);
    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->AddObserver(observer);

    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->RemoveObserver(observer);
    int size = DelayedSingleton<FreeInstallObserverManager>::GetInstance()->observerList_.size();
    EXPECT_EQ(size, 0);
}

HWTEST_F(FreeInstallObserverManagerTest, FreeInstallObserverManager_RemoveObserver_004, TestSize.Level1)
{
    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->observerList_.clear();

    MockNativeEngine engine;
    sptr<AbilityRuntime::IFreeInstallObserver> observer = new JsFreeInstallObserver(engine);

    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->RemoveObserver(observer);
    int size = DelayedSingleton<FreeInstallObserverManager>::GetInstance()->observerList_.size();
    EXPECT_EQ(size, 0);
}

HWTEST_F(FreeInstallObserverManagerTest, FreeInstallObserverManager_OnInstallFinished_001, TestSize.Level1)
{
    MockNativeEngine engine;
    sptr<AbilityRuntime::JsFreeInstallObserver> observer = new JsFreeInstallObserver(engine);
    
    std::string bundleName = "com.test.demo";
    std::string abilityName = "ability";
    std::string startTime = "0";
    MockNativeValue* callback = new MockNativeValue;
    observer->AddJsObserverObject(bundleName, abilityName, startTime, callback);

    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->AddObserver(observer);
    int size = DelayedSingleton<FreeInstallObserverManager>::GetInstance()->observerList_.size();
    EXPECT_EQ(size, 1);

    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->OnInstallFinished(bundleName, abilityName,
        startTime, 0);
}

HWTEST_F(FreeInstallObserverManagerTest, FreeInstallObserverManager_ObserverExist_001, TestSize.Level1)
{
    MockNativeEngine engine;
    sptr<AbilityRuntime::IFreeInstallObserver> observer = new JsFreeInstallObserver(engine);
    bool res = DelayedSingleton<FreeInstallObserverManager>::GetInstance()->ObserverExistLocked(observer);
    EXPECT_FALSE(res);
}

HWTEST_F(FreeInstallObserverManagerTest, FreeInstallObserverManager_ObserverExist_002, TestSize.Level1)
{
    MockNativeEngine engine;
    sptr<AbilityRuntime::IFreeInstallObserver> observer = new JsFreeInstallObserver(engine);
    DelayedSingleton<FreeInstallObserverManager>::GetInstance()->AddObserver(observer);

    bool res = DelayedSingleton<FreeInstallObserverManager>::GetInstance()->ObserverExistLocked(observer);
    EXPECT_TRUE(res);
}
} // AAFwk
} // OHOS