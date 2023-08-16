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

#include "application_anr_listener.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class ApplicationAnrListenerTest : public testing::Test {
public:
    ApplicationAnrListenerTest()
    {}
    ~ApplicationAnrListenerTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ApplicationAnrListenerTest::SetUpTestCase(void)
{}

void ApplicationAnrListenerTest::TearDownTestCase(void)
{}

void ApplicationAnrListenerTest::SetUp(void)
{}

void ApplicationAnrListenerTest::TearDown(void)
{}

/**
 * @tc.number: ApplicationAnrListenerTest_OnAnr_001
 * @tc.name: OnAnr
 * @tc.desc: Verify that function OnAnr.
 */
HWTEST_F(ApplicationAnrListenerTest, ApplicationAnrListenerTest_OnAnr_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplicationAnrListenerTest_OnAnr_001 start";
    std::shared_ptr<ApplicationAnrListener> listener = std::make_shared<ApplicationAnrListener>();
    listener->OnAnr(10078);
    GTEST_LOG_(INFO) << "ApplicationAnrListenerTest_OnAnr_001 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
