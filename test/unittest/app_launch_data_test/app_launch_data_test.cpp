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

#define private public
#define protected public
#include "app_launch_data.h"
#undef private
#undef protected
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace AppExecFwk {
namespace {
    constexpr bool isDebugApp = true;
    constexpr bool noDebugApp = false;
}
class AppLaunchDataTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<AppLaunchData> launchData_;
};

void AppLaunchDataTest::SetUpTestCase()
{}

void AppLaunchDataTest::TearDownTestCase()
{}

void AppLaunchDataTest::SetUp()
{
    launchData_ = new AppLaunchData();
}

void AppLaunchDataTest::TearDown()
{}

/**
 * @tc.name: SetDebugApp_0100
 * @tc.desc: AppLaunchData SetDebugApp, verify if AppLaunchData startup successfully.
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchDataTest, SetDebugApp_0100, TestSize.Level1)
{
    EXPECT_NE(launchData_, nullptr);
    launchData_->SetDebugApp(isDebugApp);
    EXPECT_EQ(launchData_->debugApp_, isDebugApp);
    launchData_->SetDebugApp(noDebugApp);
    EXPECT_EQ(launchData_->debugApp_, noDebugApp);
}

/**
 * @tc.name: GetDebugApp_0100
 * @tc.desc: AppLaunchData GetDebugApp, verify if AppLaunchData startup successfully.
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchDataTest, GetDebugApp_0100, TestSize.Level1)
{
    EXPECT_NE(launchData_, nullptr);
    launchData_->debugApp_ = isDebugApp;
    EXPECT_EQ(launchData_->GetDebugApp(), isDebugApp);
    launchData_->debugApp_ = noDebugApp;
    EXPECT_EQ(launchData_->GetDebugApp(), noDebugApp);
}

/**
 * @tc.name: Marshalling_0100
 * @tc.desc: Verify if launchData marshalls Sequenceable object into Parcel normally.
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchDataTest, Marshalling_0100, TestSize.Level1)
{
    EXPECT_NE(launchData_, nullptr);
    Parcel parcel;
    int32_t recordId = 0;
    int32_t uId = 0;
    int32_t appIndex = 0;
    launchData_->recordId_ = recordId;
    launchData_->uId_ = uId;
    launchData_->appIndex_ = appIndex;
    launchData_->userTestRecord_ = nullptr;
    launchData_->debugApp_ = false;
    
    EXPECT_TRUE(launchData_->Marshalling(parcel));

    parcel.ReadParcelable<ApplicationInfo>();
    parcel.ReadParcelable<Profile>();
    parcel.ReadParcelable<ProcessInfo>();

    // respectively: recordId, uId, appIndex, valid_, debugApp_
    EXPECT_EQ(parcel.ReadInt32(), recordId);
    EXPECT_EQ(parcel.ReadInt32(), uId);
    EXPECT_EQ(parcel.ReadInt32(), appIndex);
    EXPECT_FALSE(parcel.ReadBool());
    EXPECT_FALSE(parcel.ReadBool());
}

/**
 * @tc.name: ReadFromParcel_0100
 * @tc.desc: Verify if launchData reads Sequenceable object from Parcel normally.
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchDataTest, ReadFromParcel_0100, TestSize.Level1)
{
    EXPECT_NE(launchData_, nullptr);
    Parcel parcel;
    sptr<ApplicationInfo> applicationInfo = new ApplicationInfo();
    sptr<Profile> profile = new Profile();
    sptr<ProcessInfo> processInfo = new ProcessInfo();
    int32_t recordId = 0;
    int32_t uId = 0;
    int32_t appIndex = 0;
    bool valid = false;
    bool isDebug = true;

    parcel.WriteParcelable(applicationInfo);
    parcel.WriteParcelable(profile);
    parcel.WriteParcelable(processInfo);
    parcel.WriteInt32(recordId);
    parcel.WriteInt32(uId);
    parcel.WriteInt32(appIndex);
    parcel.WriteBool(valid);
    parcel.WriteBool(isDebug);

    EXPECT_TRUE(launchData_->ReadFromParcel(parcel));
    EXPECT_EQ(launchData_->recordId_, recordId);
    EXPECT_EQ(launchData_->uId_, uId);
    EXPECT_EQ(launchData_->appIndex_, appIndex);
    EXPECT_EQ(launchData_->debugApp_, isDebug);
}
} // namespace AppExecFwk
} // namespace OHOS
