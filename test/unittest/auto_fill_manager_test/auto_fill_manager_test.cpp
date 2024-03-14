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
#include "auto_fill_manager.h"
#include "auto_fill_error.h"
#include "auto_fill_extension_callback.h"
#include "extension_ability_info.h"
#include "hilog_wrapper.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class AutoFillManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    virtual Ace::UIContent* GetUIContent()
    {
        return nullptr;
    }
};

void AutoFillManagerTest::SetUpTestCase(void)
{}

void AutoFillManagerTest::TearDownTestCase(void)
{}

void AutoFillManagerTest::SetUp()
{}

void AutoFillManagerTest::TearDown()
{}

/**
 * @tc.name: RequestAutoFill_0100
 * @tc.desc: RequestAutoFill
 */
HWTEST_F(AutoFillManagerTest, RequestAutoFill_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AutoFillManagerTest, RequestAutoFill_0100, TestSize.Level1";
    auto& manager = AbilityRuntime::AutoFillManager::GetInstance();
    const AbilityBase::AutoFillType autoFillType = AbilityBase::AutoFillType::UNSPECIFIED;
    const AbilityBase::ViewData viewdata;
    const std::shared_ptr<AbilityRuntime::IFillRequestCallback> fillCallback = nullptr;
    int32_t result = manager.RequestAutoFill(autoFillType, GetUIContent(), viewdata, fillCallback);
    EXPECT_EQ(result, AbilityRuntime::AutoFill::AUTO_FILL_OBJECT_IS_NULL);
}

/**
 * @tc.name: RequestAutoSave_0100
 * @tc.desc: RequestAutoSave
 */
HWTEST_F(AutoFillManagerTest, RequestAutoSave_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AutoFillManagerTest, RequestAutoSave_0100, TestSize.Level1";
    auto& manager = AbilityRuntime::AutoFillManager::GetInstance();
    const AbilityBase::ViewData viewdata;
    const std::shared_ptr<AbilityRuntime::ISaveRequestCallback> saveCallback = nullptr;
    int32_t result = manager.RequestAutoSave(GetUIContent(), viewdata, saveCallback);
    EXPECT_EQ(result, AbilityRuntime::AutoFill::AUTO_FILL_OBJECT_IS_NULL);
}

/**
 * @tc.name: HandleRequestExecuteInner_0100
 * @tc.desc: HandleRequestExecuteInner
 */
HWTEST_F(AutoFillManagerTest, HandleRequestExecuteInner_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AutoFillManagerTest, HandleRequestExecuteInner_0100, TestSize.Level1";
    auto& manager = AbilityRuntime::AutoFillManager::GetInstance();
    const AbilityBase::AutoFillType autoFillType = AbilityBase::AutoFillType::UNSPECIFIED;
    const AbilityBase::ViewData viewdata;
    const std::shared_ptr<AbilityRuntime::IFillRequestCallback> fillCallback = nullptr;
    const std::shared_ptr<AbilityRuntime::ISaveRequestCallback> saveCallback = nullptr;
    int32_t result =
        manager.HandleRequestExecuteInner(autoFillType, GetUIContent(), viewdata, fillCallback, saveCallback);
    EXPECT_EQ(result, AbilityRuntime::AutoFill::AUTO_FILL_OBJECT_IS_NULL);
}

/**
 * @tc.name: SetTimeOutEvent_0100
 * @tc.desc: SetTimeOutEvent
 */
HWTEST_F(AutoFillManagerTest, SetTimeOutEvent_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AutoFillManagerTest, SetTimeOutEvent_0100, TestSize.Level1";
    auto& manager = AbilityRuntime::AutoFillManager::GetInstance();
    manager.SetTimeOutEvent(1);
    EXPECT_EQ(manager.eventId_, 0);
}

/**
 * @tc.name: HandleTimeOut_0100
 * @tc.desc: HandleTimeOut
 */
HWTEST_F(AutoFillManagerTest, HandleTimeOut_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AutoFillManagerTest, HandleTimeOut_0100, TestSize.Level1";
    auto& manager = AbilityRuntime::AutoFillManager::GetInstance();
    manager.HandleTimeOut(1);
    EXPECT_EQ(manager.eventId_, 0);
}

/**
 * @tc.name: RemoveEvent_0100
 * @tc.desc: RemoveEvent
 */
HWTEST_F(AutoFillManagerTest, RemoveEvent_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AutoFillManagerTest, RemoveEvent_0100, TestSize.Level1";
    auto& manager = AbilityRuntime::AutoFillManager::GetInstance();
    manager.RemoveEvent(1);
    EXPECT_EQ(manager.eventId_, 0);
}
} // namespace AppExecFwk
} // namespace OHOS