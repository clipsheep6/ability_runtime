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
#include "app_loader.h"
#undef private
#undef protected
#include "ohos_application.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class AppLoaderTest : public testing::Test {
public:
    AppLoaderTest()
    {}
    ~AppLoaderTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppLoaderTest::SetUpTestCase(void)
{}

void AppLoaderTest::TearDownTestCase(void)
{}

void AppLoaderTest::SetUp(void)
{}

void AppLoaderTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_AppLoaderTest_GetApplicationByName_0100
 * @tc.name: RegisterApplication
 * @tc.desc: Test AppLoade GetApplicationByName When Application is not nullptr.
 */
HWTEST_F(AppLoaderTest, AppExecFwk_AppLoaderTest_GetApplicationByName_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppLoaderTest_0100 start";
    std::string bundleName = "OHOSApplication";
    auto createApplication = []() -> OHOSApplication *{
        OHOSApplication *callBack = new OHOSApplication;
        return callBack;
    };
    ApplicationLoader::GetInstance().applications_.clear();
    ApplicationLoader::GetInstance().RegisterApplication(bundleName, createApplication);
    EXPECT_NE(ApplicationLoader::GetInstance().GetApplicationByName(), nullptr);
    GTEST_LOG_(INFO) << "AppLoaderTest_0100 end";
}

/**
 * @tc.number: AppExecFwk_AppLoaderTest_GetApplicationByName_0200
 * @tc.name: GetApplicationByName
 * @tc.desc: Test AppLoade GetApplicationByName When Application is nullptr.
 */
HWTEST_F(AppLoaderTest, AppExecFwk_AppLoaderTest_GetApplicationByName_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_AppLoaderTest_GetApplicationByName_0200 start";
    ApplicationLoader::GetInstance().applications_.clear();
    EXPECT_EQ(ApplicationLoader::GetInstance().GetApplicationByName(), nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_AppLoaderTest_GetApplicationByName_0200 end";
}


}  // namespace AppExecFwk
}  // namespace OHOS
