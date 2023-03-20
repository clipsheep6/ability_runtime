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
#define protected public
#include "mock_free_install_observer_stub.h"
#include "free_install_observer_interface.h"
#undef protected
#include "message_parcel.h"
#include "string_ex.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.aafwk.IFreeInstallObserver";
}

class FreeInstallObserverStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FreeInstallObserverStubTest::SetUpTestCase(void)
{}
void FreeInstallObserverStubTest::TearDownTestCase(void)
{}
void FreeInstallObserverStubTest::SetUp()
{}
void FreeInstallObserverStubTest::TearDown()
{}

/**
 * @tc.name: FreeInstallObserverStub_OnInstallFinishedInner_001
 * @tc.desc: OnInstallFinishedInner
 * @tc.type: FUNC
 * @tc.require: issueI6F3F6
 */
HWTEST_F(FreeInstallObserverStubTest, FreeInstallObserverStub_OnInstallFinishedInner_001, TestSize.Level1)
{
    sptr<MockFreeInstallObserverStub> mockFreeInstallObserverStub(
        new MockFreeInstallObserverStub());
    
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::string bundleName = "";
    std::string abilityName = "";
    std::string startTime = "";
    int resultCode = 0;

    data.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    data.WriteString(bundleName);
    data.WriteString(abilityName);
    data.WriteString(startTime);
    data.WriteInt32(resultCode);
    EXPECT_CALL(*mockFreeInstallObserverStub, OnInstallFinished(_, _, _, _)).Times(1);
    int res = mockFreeInstallObserverStub->OnRemoteRequest(
        IFreeInstallObserver::ON_INSTALL_FINISHED, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}
}  // namespace AAFwk
}  // namespace OHOS