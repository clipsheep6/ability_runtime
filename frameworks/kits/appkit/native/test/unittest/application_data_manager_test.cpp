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

#include "application_data_manager.h"
#include "ierror_observer.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
class ApplicationDataManagerTest : public testing::Test {
public:
    ApplicationDataManagerTest()
    {}
    ~ApplicationDataManagerTest()
    {}
    std::shared_ptr<ApplicationDataManager> appDataManagerTest_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    static bool Flag;
    void SetUp();
    void TearDown();
};

class MyObserver : public IErrorObserver {
    void OnUnhandledException(std::string errMsg) override;
};

bool ApplicationDataManagerTest::Flag = false;

void ApplicationDataManagerTest::SetUpTestCase(void)
{}

void ApplicationDataManagerTest::TearDownTestCase(void)
{}

void ApplicationDataManagerTest::SetUp(void)
{
    appDataManagerTest_ = DelayedSingleton<AppExecFwk::ApplicationDataManager>::GetInstance();
}

void ApplicationDataManagerTest::TearDown(void)
{}

void MyObserver::OnUnhandledException(std::string errMsg)
{
    GTEST_LOG_(INFO) << "OnUnhandledException come, errMsg is" << errMsg;
    ApplicationDataManagerTest::Flag = true;
}

/**
 * @tc.number: ApplicationDataManager_AddErrorObservers_001
 * @tc.name: RegisterAbilityLifecycleCallbacks
 * @tc.desc: Test whether registerabilitylifecyclecallbacks and are called normally.
 */
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_AddErrorObservers_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_AddErrorObservers_001 start";

    EXPECT_NE(appDataManagerTest_, nullptr);
    std::shared_ptr<MyObserver> observer = std::make_shared<MyObserver>();
    if (appDataManagerTest_ != nullptr) {
        appDataManagerTest_->AddErrorObserver(observer);
        appDataManagerTest_->NotifyUnhandledException("test");
        EXPECT_EQ(true, ApplicationDataManagerTest::Flag);
    }
    GTEST_LOG_(INFO) << "ApplicationDataManager_AddErrorObservers_001 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
