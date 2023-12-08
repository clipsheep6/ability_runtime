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
#define protected public
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_worker.h"
#include "hdc_register.h"
#undef private
#undef protected

#include "event_runner.h"
#include "mock_js_runtime.h"
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class HdcRegisterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void HdcRegisterTest::SetUpTestCase()
{}

void HdcRegisterTest::TearDownTestCase()
{}

void HdcRegisterTest::SetUp()
{}

void HdcRegisterTest::TearDown()
{}

/**
 * @tc.name: HdcRegisterTest_0100
 * @tc.desc: HdcRegisterTest Test
 * @tc.type: FUNC
 */
HWTEST_F(HdcRegisterTest, HdcRegisterTest_0100, TestSize.Level0)
{
    const std::string processName = "";
    const std::string bundleName = "";
    bool debugApp = true;
    auto &pHdcRegister = AbilityRuntime::HdcRegister::Get();
    
    pHdcRegister.StartHdcRegister(bundleName, processName, debugApp, nullptr);

    EXPECT_TRUE(true);
}

/**
 * @tc.name: HdcRegisterTest_0200
 * @tc.desc: HdcRegisterTest Test
 * @tc.type: FUNC
 */
HWTEST_F(HdcRegisterTest, HdcRegisterTest_0200, TestSize.Level0)
{
    auto &pHdcRegister = AbilityRuntime::HdcRegister::Get();
    pHdcRegister.registerHandler_ = nullptr;
    pHdcRegister.StopHdcRegister();
    
    EXPECT_TRUE(true);
}

/**
 * @tc.name: HdcRegisterTest_0300
 * @tc.desc: HdcRegisterTest Test
 * @tc.type: FUNC
 */
HWTEST_F(HdcRegisterTest, HdcRegisterTest_0300, TestSize.Level0)
{
    const std::string processName = "123";
    const std::string bundleName = "123";
    bool debugApp = true;
    auto &pHdcRegister = AbilityRuntime::HdcRegister::Get();
    pHdcRegister.StartHdcRegister(bundleName, processName, debugApp, nullptr);
    pHdcRegister.StopHdcRegister();

    EXPECT_TRUE(true);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
