/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "ui_extension_record.h"
#undef private
#undef protected

#include "ability_record.h"
#include "hilog_wrapper.h"
#include "mock_session_interface.h"
#include "parcel.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class UIExtensionRecordTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UIExtensionRecordTest::SetUpTestCase(void) {}

void UIExtensionRecordTest::TearDownTestCase(void) {}

void UIExtensionRecordTest::SetUp() {}

void UIExtensionRecordTest::TearDown() {}

/*
 * Feature: UIEextensionRecord
 * Function: HandleNotifyUIExtensionTimeout
 * SubFunction: NA
 * FunctionPoints: UIEextensionRecord HandleNotifyUIExtensionTimeout
 */
HWTEST_F(UIExtensionRecordTest, HandleNotifyUIExtensionTimeout_0100, TestSize.Level1)
{
    HILOG_INFO("Begin.");
    const AAFwk::Want want;
    const AppExecFwk::AbilityInfo abilityInfo;
    const AppExecFwk::ApplicationInfo applicationInfo;
    const std::shared_ptr<AAFwk::AbilityRecord> abilityRecord =
        std::make_shared<AAFwk::AbilityRecord>(want, abilityInfo, applicationInfo);
    sptr<MockISession> mockSession = new (std::nothrow) MockISession();
    sptr<AAFwk::SessionInfo> sessionData = new (std::nothrow) AAFwk::SessionInfo();
    sessionData->sessionToken = mockSession->AsObject();
    abilityRecord->SetSessionInfo(sessionData);
    auto uiExtensionRecord = std::make_shared<UIExtensionRecord>(abilityRecord);
    UIExtensionRecord::ErrorCode code = UIExtensionRecord::ErrorCode::LOAD_TIMEOUT;
    uiExtensionRecord->HandleNotifyUIExtensionTimeout(code);
    EXPECT_EQ(code, mockSession->timeoutCode_);
    HILOG_INFO("End.");
}

/*
 * Feature: UIEextensionRecord
 * Function: LoadTimeout
 * SubFunction: NA
 * FunctionPoints: UIEextensionRecord LoadTimeout
 */
HWTEST_F(UIExtensionRecordTest, LoadTimeout_0100, TestSize.Level1)
{
    HILOG_INFO("Begin.");
    const AAFwk::Want want;
    const AppExecFwk::AbilityInfo abilityInfo;
    const AppExecFwk::ApplicationInfo applicationInfo;
    const std::shared_ptr<AAFwk::AbilityRecord> abilityRecord =
        std::make_shared<AAFwk::AbilityRecord>(want, abilityInfo, applicationInfo);
    sptr<MockISession> mockSession = new (std::nothrow) MockISession();
    sptr<AAFwk::SessionInfo> sessionData = new (std::nothrow) AAFwk::SessionInfo();
    sessionData->sessionToken = mockSession->AsObject();
    abilityRecord->SetSessionInfo(sessionData);

    auto uiExtensionRecord = std::make_shared<UIExtensionRecord>(abilityRecord);
    uiExtensionRecord->LoadTimeout();
    EXPECT_EQ(UIExtensionRecord::ErrorCode::LOAD_TIMEOUT, mockSession->timeoutCode_);
    HILOG_INFO("End.");
}

/*
 * Feature: UIEextensionRecord
 * Function: ForegroundTimeout
 * SubFunction: NA
 * FunctionPoints: UIEextensionRecord ForegroundTimeout
 */
HWTEST_F(UIExtensionRecordTest, ForegroundTimeout_0100, TestSize.Level1)
{
    HILOG_INFO("Begin.");
    const AAFwk::Want want;
    const AppExecFwk::AbilityInfo abilityInfo;
    const AppExecFwk::ApplicationInfo applicationInfo;
    const std::shared_ptr<AAFwk::AbilityRecord> abilityRecord =
        std::make_shared<AAFwk::AbilityRecord>(want, abilityInfo, applicationInfo);
    sptr<MockISession> mockSession = new (std::nothrow) MockISession();
    sptr<AAFwk::SessionInfo> sessionData = new (std::nothrow) AAFwk::SessionInfo();
    sessionData->sessionToken = mockSession->AsObject();
    abilityRecord->SetSessionInfo(sessionData);

    auto uiExtensionRecord = std::make_shared<UIExtensionRecord>(abilityRecord);
    uiExtensionRecord->ForegroundTimeout();
    EXPECT_EQ(UIExtensionRecord::ErrorCode::FOREGROUND_TIMEOUT, mockSession->timeoutCode_);
    HILOG_INFO("End.");
}

/*
 * Feature: UIEextensionRecord
 * Function: BackgroundTimeout
 * SubFunction: NA
 * FunctionPoints: UIEextensionRecord BackgroundTimeout
 */
HWTEST_F(UIExtensionRecordTest, BackgroundTimeout_0100, TestSize.Level1)
{
    HILOG_INFO("Begin.");
    const AAFwk::Want want;
    const AppExecFwk::AbilityInfo abilityInfo;
    const AppExecFwk::ApplicationInfo applicationInfo;
    const std::shared_ptr<AAFwk::AbilityRecord> abilityRecord =
        std::make_shared<AAFwk::AbilityRecord>(want, abilityInfo, applicationInfo);
    sptr<MockISession> mockSession = new (std::nothrow) MockISession();
    sptr<AAFwk::SessionInfo> sessionData = new (std::nothrow) AAFwk::SessionInfo();
    sessionData->sessionToken = mockSession->AsObject();
    abilityRecord->SetSessionInfo(sessionData);

    auto uiExtensionRecord = std::make_shared<UIExtensionRecord>(abilityRecord);
    uiExtensionRecord->BackgroundTimeout();
    EXPECT_EQ(UIExtensionRecord::ErrorCode::BACKGROUND_TIMEOUT, mockSession->timeoutCode_);
    HILOG_INFO("End.");
}

/*
 * Feature: UIEextensionRecord
 * Function: TerminateTimeout
 * SubFunction: NA
 * FunctionPoints: UIEextensionRecord TerminateTimeout
 */
HWTEST_F(UIExtensionRecordTest, TerminateTimeout_0100, TestSize.Level1)
{
    HILOG_INFO("Begin.");
    const AAFwk::Want want;
    const AppExecFwk::AbilityInfo abilityInfo;
    const AppExecFwk::ApplicationInfo applicationInfo;
    const std::shared_ptr<AAFwk::AbilityRecord> abilityRecord =
        std::make_shared<AAFwk::AbilityRecord>(want, abilityInfo, applicationInfo);
    sptr<MockISession> mockSession = new (std::nothrow) MockISession();
    sptr<AAFwk::SessionInfo> sessionData = new (std::nothrow) AAFwk::SessionInfo();
    sessionData->sessionToken = mockSession->AsObject();
    abilityRecord->SetSessionInfo(sessionData);

    auto uiExtensionRecord = std::make_shared<UIExtensionRecord>(abilityRecord);
    uiExtensionRecord->TerminateTimeout();
    EXPECT_EQ(UIExtensionRecord::ErrorCode::TERMINATE_TIMEOUT, mockSession->timeoutCode_);
    HILOG_INFO("End.");
}

} // namespace AbilityRuntime
} // namespace OHOS