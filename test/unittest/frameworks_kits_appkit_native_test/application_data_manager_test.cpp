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

#include <gtest/gtest.h>

#define private public
#define protected public
#include "application_data_manager.h"
#undef private
#undef protected
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
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    static bool Flag;
    void SetUp();
    void TearDown();
};

class MyObserver : public IErrorObserver {
    void OnUnhandledException(std::string errMsg) override;
    void OnExceptionObject(const AppExecFwk::ErrorObject &errorObj) override;
};

bool ApplicationDataManagerTest::Flag = false;

void ApplicationDataManagerTest::SetUpTestCase(void)
{}

void ApplicationDataManagerTest::TearDownTestCase(void)
{}

void ApplicationDataManagerTest::SetUp(void)
{
}

void ApplicationDataManagerTest::TearDown(void)
{}

void MyObserver::OnUnhandledException(std::string errMsg)
{
    GTEST_LOG_(INFO) << "OnUnhandledException come, errMsg is" << errMsg;
    ApplicationDataManagerTest::Flag = true;
}

void MyObserver::OnExceptionObject(const AppExecFwk::ErrorObject &errorObj)
{
    GTEST_LOG_(INFO) << "OnExceptionObject come, errorObj.name is " << errorObj.name <<
                        " errorObj.message is " << errorObj.message << " errorObj.stack is " << errorObj.stack;
    EXPECT_STREQ("errorName", errorObj.name.c_str());
    EXPECT_STREQ("errorMessage", errorObj.message.c_str());
    EXPECT_STREQ("errorStack", errorObj.stack.c_str());
}

/**
 * @tc.number: ApplicationDataManager_AddErrorObservers_001
 * @tc.name: RegisterAbilityLifecycleCallbacks
 * @tc.desc: Test whether registerabilitylifecyclecallbacks and are called normally.
 */
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_AddErrorObservers_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_AddErrorObservers_001 start";

    std::shared_ptr<MyObserver> observer = std::make_shared<MyObserver>();
    ApplicationDataManager::GetInstance().AddErrorObserver(observer);
    ApplicationDataManager::GetInstance().NotifyUnhandledException("test");
    EXPECT_EQ(true, ApplicationDataManagerTest::Flag);
    GTEST_LOG_(INFO) << "ApplicationDataManager_AddErrorObservers_001 end";
}

/**
 * @tc.number: ApplicationDataManager_RemoveErrorObserver_001
 * @tc.name: ApplicationDataManager RemoveErrorObserver
 * @tc.desc: Test whether remove Registerabilitylifecyclecallbacks and are called normally.
 */
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_RemoveErrorObserver_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_RemoveErrorObserver_001 start";
    std::shared_ptr<MyObserver> observer = std::make_shared<MyObserver>();
    ApplicationDataManager::GetInstance().AddErrorObserver(observer);
    ApplicationDataManager::GetInstance().NotifyUnhandledException("test");
    EXPECT_EQ(true, ApplicationDataManagerTest::Flag);
    ApplicationDataManager::GetInstance().RemoveErrorObserver();
    EXPECT_EQ(nullptr, ApplicationDataManager::GetInstance().errorObserver_);
    GTEST_LOG_(INFO) << "ApplicationDataManager_RemoveErrorObserver_001 end";
}

/**
 * @tc.number: ApplicationDataManager_NotifyExceptionObject_001
 * @tc.name: ApplicationDataManager NotifyExceptionObject
 * @tc.desc: Test whether NotifyExceptionObject are called normally.
 */
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_NotifyExceptionObject_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyExceptionObject_001 start";
    AppExecFwk::ErrorObject errorObj;
    errorObj.name = "errorName";
    errorObj.message = "errorMessage";
    errorObj.stack = "errorStack";
    std::shared_ptr<MyObserver> observer = std::make_shared<MyObserver>();
    ApplicationDataManager::GetInstance().AddErrorObserver(observer);
    EXPECT_NE(nullptr, ApplicationDataManager::GetInstance().errorObserver_);
    EXPECT_TRUE(ApplicationDataManager::GetInstance().NotifyExceptionObject(errorObj));
    ApplicationDataManager::GetInstance().RemoveErrorObserver();
    EXPECT_EQ(nullptr, ApplicationDataManager::GetInstance().errorObserver_);
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyExceptionObject_001 end";
}

/**
 * @tc.number: ApplicationDataManager_NotifyExceptionObject_002
 * @tc.name: ApplicationDataManager NotifyExceptionObject
 * @tc.desc: Test whether NotifyExceptionObject are called normally.
 */
HWTEST_F(ApplicationDataManagerTest, ApplicationDataManager_NotifyExceptionObject_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyExceptionObject_002 start";
    AppExecFwk::ErrorObject errorObj;
    errorObj.name = "errorName";
    errorObj.message = "errorMessage";
    errorObj.stack = "errorStack";
    ApplicationDataManager::GetInstance().RemoveErrorObserver();
    EXPECT_EQ(nullptr, ApplicationDataManager::GetInstance().errorObserver_);
    EXPECT_FALSE(ApplicationDataManager::GetInstance().NotifyExceptionObject(errorObj));
    GTEST_LOG_(INFO) << "ApplicationDataManager_NotifyExceptionObject_002 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
