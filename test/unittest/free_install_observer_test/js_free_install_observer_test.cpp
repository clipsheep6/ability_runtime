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
#include "js_free_install_observer.h"
#include "mock_native_engine.h"
#include "mock_native_value.h"
#undef private

using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace AbilityRuntime {
class JsFreeInstallObserverTest : public testing::Test {
public:
    JsFreeInstallObserverTest()
    {}
    ~JsFreeInstallObserverTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void JsFreeInstallObserverTest::SetUpTestCase(void)
{
}

void JsFreeInstallObserverTest::TearDownTestCase(void)
{}

void JsFreeInstallObserverTest::SetUp()
{}

void JsFreeInstallObserverTest::TearDown()
{}

/**
 * @tc.name: JsFreeInstallObserver_AddJsObserverObject_001
 * @tc.desc: AddJsObserverObject
 * @tc.type: FUNC
 * @tc.require: issueI6F3F6
 */
HWTEST_F(JsFreeInstallObserverTest, JsFreeInstallObserver_AddJsObserverObject_001, TestSize.Level1)
{
    MockNativeEngine engine;
    MockNativeValue callback;
    sptr<JsFreeInstallObserver> observer = new JsFreeInstallObserver(engine);
    std::string bundleName = "";
    std::string abilityName = "";
    std::string startTime = "";
    observer->AddJsObserverObject(bundleName, abilityName, startTime, callback);
    int size = observer->jsObserverObjectList_.size();
    EXPECT_EQ(size, 1);
}

/**
 * @tc.name: JsFreeInstallObserver_AddJsObserverObject_002
 * @tc.desc: AddJsObserverObject
 * @tc.type: FUNC
 * @tc.require: issueI6F3F6
 */
HWTEST_F(JsFreeInstallObserverTest, JsFreeInstallObserver_AddJsObserverObject_002, TestSize.Level1)
{
    MockNativeEngine engine;
    sptr<JsFreeInstallObserver> observer = new JsFreeInstallObserver(engine);
    std::string bundleName = "";
    std::string abilityName = "";
    std::string startTime = "";
    observer->AddJsObserverObject(bundleName, abilityName, startTime, nullptr);
    int size = observer->jsObserverObjectList_.size();
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: JsFreeInstallObserver_AddJsObserverObject_003
 * @tc.desc: AddJsObserverObject
 * @tc.type: FUNC
 * @tc.require: issueI6F3F6
 */
HWTEST_F(JsFreeInstallObserverTest, JsFreeInstallObserver_AddJsObserverObject_003, TestSize.Level1)
{
    MockNativeEngine engine;
    MockNativeValue callback;
    sptr<JsFreeInstallObserver> observer = new JsFreeInstallObserver(engine);
    std::string bundleName = "com.test.demo";
    std::string abilityName = "abilityName";
    std::string startTime = "0";
    observer->AddJsObserverObject(bundleName, abilityName, startTime, callback);
    observer->AddJsObserverObject(bundleName, abilityName, startTime, callback);
    int size = observer->jsObserverObjectList_.size();
    EXPECT_EQ(size, 1);
}

/**
 * @tc.name: JsFreeInstallObserver_OnInstallFinished_001
 * @tc.desc: OnInstallFinished
 * @tc.type: FUNC
 * @tc.require: issueI6F3F6
 */
HWTEST_F(JsFreeInstallObserverTest, JsFreeInstallObserver_OnInstallFinished_001, TestSize.Level1)
{
    MockNativeEngine engine;
    MockNativeValue callback;
    sptr<JsFreeInstallObserver> observer = new JsFreeInstallObserver(engine);
    std::string bundleName = "com.test.demo";
    std::string abilityName = "abilityName";
    std::string startTime = "0";
    observer->AddJsObserverObject(bundleName, abilityName, startTime, callback);
    observer->OnInstallFinished(bundleName, abilityName, startTime, 0);
    int size = observer->jsObserverObjectList_.size();
    EXPECT_EQ(size, 0);
}
} // AbilityRuntime
} // OHOS