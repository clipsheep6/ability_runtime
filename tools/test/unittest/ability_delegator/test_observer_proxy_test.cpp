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
#include "hilog_wrapper.h"
#include "iremote_proxy.h"
#include "test_observer_proxy.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
const std::string CMD = "ls -l";
}  // namespace

class TestObserverProxyTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void TestObserverProxyTest::SetUpTestCase()
{}

void TestObserverProxyTest::TearDownTestCase()
{}

void TestObserverProxyTest::SetUp()
{}

void TestObserverProxyTest::TearDown()
{}

namespace OHOS {
class MockIRemoteObject : public IRemoteObject {
public:
    MockIRemoteObject() : IRemoteObject(u"mock_i_remote_object") {}

    ~MockIRemoteObject() {}

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool CheckObjectLegality() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }

    std::u16string GetObjectDescriptor() const
    {
        std::u16string descriptor = std::u16string();
        return descriptor;
    }
};
}

/**
 * @tc.number: Test_Observer_Proxy_Test_0100
 * @tc.name: Test ExecuteShellCommand
 * @tc.desc: Verify the ExecuteShellCommand fail because failed to read result.
 */
HWTEST_F(TestObserverProxyTest, Test_Observer_Proxy_Test_0100, TestSize.Level1)
{
    HILOG_INFO("Test_Observer_Proxy_Test_0100 start");
    OHOS::sptr<OHOS::IRemoteObject> object = new OHOS::MockIRemoteObject();
    TestObserverProxy testObserverProxy(object);
    EXPECT_EQ(testObserverProxy.ExecuteShellCommand(CMD.c_str(), 0).stdResult.size(), 0);
    HILOG_INFO("Test_Observer_Proxy_Test_0100 end");
}

/**
 * @tc.number: Test_Observer_Proxy_Test_0200
 * @tc.name: Test TestStatus
 * @tc.desc: Verify the TestStatus process not crush.
 */
HWTEST_F(TestObserverProxyTest, Test_Observer_Proxy_Test_0200, TestSize.Level1)
{
    HILOG_INFO("Test_Observer_Proxy_Test_0200 start");
    OHOS::sptr<OHOS::IRemoteObject> object = new OHOS::MockIRemoteObject();
    TestObserverProxy testObserverProxy(object);
    testObserverProxy.TestStatus(CMD.c_str(), 0);
    HILOG_INFO("Test_Observer_Proxy_Test_0200 end");
}

/**
 * @tc.number: Test_Observer_Proxy_Test_0300
 * @tc.name: Test TestFinished
 * @tc.desc: Verify the TestFinished process not crush.
 */
HWTEST_F(TestObserverProxyTest, Test_Observer_Proxy_Test_0300, TestSize.Level1)
{
    HILOG_INFO("Test_Observer_Proxy_Test_0300 start");
    OHOS::sptr<OHOS::IRemoteObject> object = new OHOS::MockIRemoteObject();
    TestObserverProxy testObserverProxy(object);
    testObserverProxy.TestFinished(CMD.c_str(), 0);
    HILOG_INFO("Test_Observer_Proxy_Test_0300 end");
}