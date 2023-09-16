/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>
#include "ability_record.h"
#include "call_record.h"
#include "mock_serviceability_manager_service.h"

namespace OHOS {
namespace AAFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace {
class MyAbilityConnection : public IAbilityConnection {
public:
    MyAbilityConnection() = default;
    virtual ~MyAbilityConnection() = default;
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override
    {}
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override
    {}
    sptr<IRemoteObject> AsObject() override
    {
        return {};
    }
};
}

class CallRecordTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CallRecordTest::SetUpTestCase(void)
{}

void CallRecordTest::TearDownTestCase(void)
{}

void CallRecordTest::SetUp(void)
{}

void CallRecordTest::TearDown(void)
{}

/**
 * @tc.number: CallRecord_SchedulerConnectDone_001
 * @tc.name: SchedulerConnectDone
 * @tc.desc: CallRecord to process SchedulerConnectDone success.
 */
HWTEST_F(CallRecordTest, CallRecord_SchedulerConnectDone_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CallRecord_SchedulerConnectDone_001 begin";
    std::shared_ptr<AbilityRecord> abilityRecord;
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new MyAbilityConnection();
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    Want want;
    abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord->shared_from_this(),
        abilityRequest.connect, abilityRequest.callerToken);
    OHOS::sptr<OHOS::IRemoteObject> call = new (std::nothrow) MockServiceAbilityManagerService();
    callRecord->SetCallStub(call);
    callRecord->SetConCallBack(abilityRequest.connect);
    EXPECT_FALSE(callRecord->SchedulerConnectDone());

    GTEST_LOG_(INFO) << "CallRecord_SchedulerConnectDone_001 end";
}
}  // namespace AAFwk
}  // namespace OHOS
