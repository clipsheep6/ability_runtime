/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "ability_context.h"
#include "completed_callback.h"
#include "context_container.h"
#include "element_name.h"
#include "event_handler.h"
#include "base_types.h"
#include "pending_want.h"
#include "pending_want_record.h"
#include "process_options.h"
#include "want.h"
#define private public
#define protected public
#include "want_agent.h"
#undef private
#undef protected
#include "want_agent_constant.h"
#include "want_agent_helper.h"
#include "want_agent_info.h"
#include "want_params.h"
#include "want_receiver_stub.h"
#include "want_sender_stub.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS;
using OHOS::AppExecFwk::ElementName;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime::WantAgent;
using vector_str = std::vector<std::string>;

namespace OHOS::AbilityRuntime::WantAgent {
class WantAgentTest : public testing::Test {
public:
    WantAgentTest()
    {}
    ~WantAgentTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void WantAgentTest::SetUpTestCase(void)
{}

void WantAgentTest::TearDownTestCase(void)
{}

void WantAgentTest::SetUp(void)
{}

void WantAgentTest::TearDown(void)
{}

/*
 * @tc.number    : WantAgent_0100
 * @tc.name      : WantAgentInfo Constructors
 * @tc.desc      : 1.Constructors and GetPendingWant
 */
HWTEST_F(WantAgentTest, WantAgent_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(nullptr);
    EXPECT_EQ(wantAgent->GetPendingWant(), nullptr);
}

/*
 * @tc.number    : WantAgent_0200
 * @tc.name      : WantAgentInfo Constructors
 * @tc.desc      : 1.Constructors and GetPendingWant
 */
HWTEST_F(WantAgentTest, WantAgent_0200, Function | MediumTest | Level1)
{
    sptr<IWantSender> target(new (std::nothrow) PendingWantRecord());
    std::shared_ptr<PendingWant> pendingWant = std::make_shared<PendingWant>(target);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    EXPECT_EQ(wantAgent->GetPendingWant(), pendingWant);
}

/*
 * @tc.number    : WantAgent_0300
 * @tc.name      : WantAgentInfo Constructors
 * @tc.desc      : 1.Constructors and SetPendingWant
 */
HWTEST_F(WantAgentTest, WantAgent_0300, Function | MediumTest | Level1)
{
    sptr<IWantSender> target(new (std::nothrow) PendingWantRecord());
    std::shared_ptr<PendingWant> pendingWant = std::make_shared<PendingWant>(target);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    EXPECT_NE(wantAgent, nullptr);
    wantAgent->SetPendingWant(pendingWant);
}

/*
 * @tc.number    : WantAgent_0400
 * @tc.name      : WantAgentInfo Constructors
 * @tc.desc      : 1.Constructors and Marshalling
 */
HWTEST_F(WantAgentTest, WantAgent_0400, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(nullptr);
    Parcel parcel;
    bool ret = wantAgent->Marshalling(parcel);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.number    : WantAgent_0500
 * @tc.name      : WantAgentInfo Constructors
 * @tc.desc      : 1.Constructors and Unmarshalling
 */
HWTEST_F(WantAgentTest, WantAgent_0500, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(nullptr);
    Parcel parcel;
    bool ret = wantAgent->Unmarshalling(parcel);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.number    : ProcessOptionsTest_0100
 * @tc.name      : Marshalling
 * @tc.desc      : Marshalling
 */
HWTEST_F(WantAgentTest, ProcessOptionsTest_0100, TestSize.Level1)
{
    auto option = std::make_shared<ProcessOptions>();
    Parcel parcel;
    auto result = option->Marshalling(parcel);
    EXPECT_EQ(result, true);
}

/*
 * @tc.number    : ProcessOptionsTest_0200
 * @tc.name      : Unmarshalling
 * @tc.desc      : Unmarshalling
 */
HWTEST_F(WantAgentTest, ProcessOptionsTest_0200, TestSize.Level1)
{
    auto option = std::make_shared<ProcessOptions>();
    Parcel parcel;
    option->Unmarshalling(parcel);
    EXPECT_NE(option, nullptr);
}

/*
 * @tc.number    : ProcessOptionsTest_0300
 * @tc.name      : ConvertInt32ToProcessMode
 * @tc.desc      : ConvertInt32ToProcessMode
 */
HWTEST_F(WantAgentTest, ProcessOptionsTest_0300, TestSize.Level1)
{
    auto option = std::make_shared<ProcessOptions>();
    int32_t value = 1;
    option->ConvertInt32ToProcessMode(value);
    EXPECT_NE(option, nullptr);
}

/*
 * @tc.number    : ProcessOptionsTest_0400
 * @tc.name      : ConvertInt32ToStartupVisibility
 * @tc.desc      : ConvertInt32ToStartupVisibility
 */
HWTEST_F(WantAgentTest, ProcessOptionsTest_0400, TestSize.Level1)
{
    auto option = std::make_shared<ProcessOptions>();
    int32_t value = 1;
    option->ConvertInt32ToStartupVisibility(value);
    EXPECT_NE(option, nullptr);
}

/*
 * @tc.number    : ProcessOptionsTest_0500
 * @tc.name      : IsNewProcessMode
 * @tc.desc      : IsNewProcessMode
 */
HWTEST_F(WantAgentTest, ProcessOptionsTest_0500, TestSize.Level1)
{
    auto option = std::make_shared<ProcessOptions>();
    ProcessMode value = ProcessMode::UNSPECIFIED;
    option->IsNewProcessMode(value);
    EXPECT_NE(option, nullptr);
}
}  // namespace OHOS::AbilityRuntime::WantAgent
