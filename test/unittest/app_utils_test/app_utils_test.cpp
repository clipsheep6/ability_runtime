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

#include "app_utils.h"
#include "hilog_wrapper.h"
#include "parameters.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class AppUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppUtilsTest::SetUpTestCase(void)
{}

void AppUtilsTest::TearDownTestCase(void)
{}

void AppUtilsTest::SetUp()
{}

void AppUtilsTest::TearDown()
{}

/**
 * @tc.number: AppUtilsTest_0100
 * @tc.desc: Test IsInheritWindowSplitScreenMode works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0100, TestSize.Level0)
{
    HILOG_INFO("AppUtilsTest_0100 called.");
    bool isInheritWindowSplitScreenMode = AAFwk::AppUtils::GetInstance().IsInheritWindowSplitScreenMode();
    std::string deviceType = OHOS::system::GetDeviceType();
    HILOG_INFO("current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isInheritWindowSplitScreenMode == true);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isInheritWindowSplitScreenMode == true);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isInheritWindowSplitScreenMode == false);
    }
}

/**
 * @tc.number: AppUtilsTest_0200
 * @tc.desc: Test IsSupportAncoApp works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0200, TestSize.Level0)
{
    HILOG_INFO("AppUtilsTest_0200 called.");
    bool isSupportAncoApp = AAFwk::AppUtils::GetInstance().IsSupportAncoApp();
    std::string deviceType = OHOS::system::GetDeviceType();
    HILOG_INFO("current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isSupportAncoApp == false);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isSupportAncoApp == true);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isSupportAncoApp == false);
    }
}

/**
 * @tc.number: AppUtilsTest_0300
 * @tc.desc: Test GetTimeoutUnitTimeRatio works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0300, TestSize.Level0)
{
    HILOG_INFO("AppUtilsTest_0300 called.");
    int32_t timeoutUnitTimeRatio = AAFwk::AppUtils::GetInstance().GetTimeoutUnitTimeRatio();
    std::string deviceType = OHOS::system::GetDeviceType();
    HILOG_INFO("current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(timeoutUnitTimeRatio == 1);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(timeoutUnitTimeRatio == 1);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(timeoutUnitTimeRatio == 10);
    }
}

/**
 * @tc.number: AppUtilsTest_0400
 * @tc.desc: Test IsSelectorDialogDefaultPossion works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0400, TestSize.Level0)
{
    HILOG_INFO("AppUtilsTest_0400 called.");
    bool isSelectorDialogDefaultPossion = AAFwk::AppUtils::GetInstance().IsSelectorDialogDefaultPossion();
    std::string deviceType = OHOS::system::GetDeviceType();
    HILOG_INFO("current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isSelectorDialogDefaultPossion == true);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isSelectorDialogDefaultPossion == true);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isSelectorDialogDefaultPossion == false);
    }
}

/**
 * @tc.number: AppUtilsTest_0500
 * @tc.desc: Test IsStartSpecifiedProcess works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0500, TestSize.Level0)
{
    HILOG_INFO("AppUtilsTest_0500 called.");
    bool isStartSpecifiedProcess = AAFwk::AppUtils::GetInstance().IsStartSpecifiedProcess();
    std::string deviceType = OHOS::system::GetDeviceType();
    HILOG_INFO("current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isStartSpecifiedProcess == false);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isStartSpecifiedProcess == false);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isStartSpecifiedProcess == true);
    }
}

/**
 * @tc.number: AppUtilsTest_0600
 * @tc.desc: Test IsUseMultiRenderProcess works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0600, TestSize.Level0)
{
    HILOG_INFO("AppUtilsTest_0600 called.");
    bool isUseMultiRenderProcess = AAFwk::AppUtils::GetInstance().IsUseMultiRenderProcess();
    std::string deviceType = OHOS::system::GetDeviceType();
    HILOG_INFO("current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isUseMultiRenderProcess == true);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isUseMultiRenderProcess == true);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isUseMultiRenderProcess == true);
    }
}

/**
 * @tc.number: AppUtilsTest_0700
 * @tc.desc: Test IsLimitMaximumOfRenderProcess works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0700, TestSize.Level0)
{
    HILOG_INFO("AppUtilsTest_0700 called.");
    bool isLimitMaximumOfRenderProcess = AAFwk::AppUtils::GetInstance().IsLimitMaximumOfRenderProcess();
    std::string deviceType = OHOS::system::GetDeviceType();
    HILOG_INFO("current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isLimitMaximumOfRenderProcess == true);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isLimitMaximumOfRenderProcess == true);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isLimitMaximumOfRenderProcess == false);
    }
}

/**
 * @tc.number: AppUtilsTest_0800
 * @tc.desc: Test IsGrantPersistUriPermission works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0800, TestSize.Level0)
{
    HILOG_INFO("AppUtilsTest_0800 called.");
    bool isGrantPersistUriPermission = AAFwk::AppUtils::GetInstance().IsGrantPersistUriPermission();
    std::string deviceType = OHOS::system::GetDeviceType();
    HILOG_INFO("current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isGrantPersistUriPermission == false);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isGrantPersistUriPermission == false);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isGrantPersistUriPermission == true);
    }
}

/**
 * @tc.number: AppUtilsTest_0900
 * @tc.desc: Test IsStartOptionsWithAnimation works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0900, TestSize.Level0)
{
    HILOG_INFO("AppUtilsTest_0900 called.");
    bool isStartOptionsWithAnimation = AAFwk::AppUtils::GetInstance().IsStartOptionsWithAnimation();
    std::string deviceType = OHOS::system::GetDeviceType();
    HILOG_INFO("current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isStartOptionsWithAnimation == false);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isStartOptionsWithAnimation == false);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isStartOptionsWithAnimation == true);
    }
}

/**
 * @tc.number: AppUtilsTest_1000
 * @tc.desc: Test IsMultiProcessModel works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_1000, TestSize.Level0)
{
    HILOG_INFO("AppUtilsTest_1000 called.");
    bool isMultiProcessModel = AAFwk::AppUtils::GetInstance().IsMultiProcessModel();
    std::string deviceType = OHOS::system::GetDeviceType();
    HILOG_INFO("current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isMultiProcessModel == false);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isMultiProcessModel == false);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isMultiProcessModel == true);
    }
}

/**
 * @tc.number: AppUtilsTest_1100
 * @tc.desc: Test IsStartOptionsWithProcessOptions works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_1100, TestSize.Level0)
{
    HILOG_INFO("AppUtilsTest_1100 called.");
    bool isStartOptionsWithProcessOptions = AAFwk::AppUtils::GetInstance().IsStartOptionsWithProcessOptions();
    std::string deviceType = OHOS::system::GetDeviceType();
    HILOG_INFO("current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isStartOptionsWithProcessOptions == false);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isStartOptionsWithProcessOptions == false);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isStartOptionsWithProcessOptions == true);
    }
}

/**
 * @tc.number: AppUtilsTest_1200
 * @tc.desc: Test EnableMoveUIAbilityToBackgroundApi works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_1200, TestSize.Level0)
{
    HILOG_INFO("AppUtilsTest_1200 called.");
    bool enableMoveUIAbilityToBackgroundApi = AAFwk::AppUtils::GetInstance().EnableMoveUIAbilityToBackgroundApi();
    std::string deviceType = OHOS::system::GetDeviceType();
    HILOG_INFO("current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(enableMoveUIAbilityToBackgroundApi == true);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(enableMoveUIAbilityToBackgroundApi == true);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(enableMoveUIAbilityToBackgroundApi == false);
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS