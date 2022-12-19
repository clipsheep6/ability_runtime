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
#include "application_env_impl.h"
#undef private
#undef protected
#include "application_env.h"
#include "application_context.h"
#include "context_impl.h"
#include "mock_application.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class ApplicationEnvImplTest : public testing::Test {
public:
    ApplicationEnvImplTest()
    {}
    ~ApplicationEnvImplTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ApplicationEnvImplTest::SetUpTestCase(void)
{}

void ApplicationEnvImplTest::TearDownTestCase(void)
{}

void ApplicationEnvImplTest::SetUp(void)
{}

void ApplicationEnvImplTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_ApplicationEnvImplTest_SetAppInfo_0100
 * @tc.name: ApplicationEnvImpl SetAppInfo
 * @tc.desc: Test ApplicationEnvImpl set and get applicationInfo.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvImplTest_SetAppInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvImplTest_SetAppInfo_0100 start";
    ApplicationInfo applicationInfo;
    applicationInfo.bundleName = "bundleName";
    applicationInfo.dataDir = "/dataDir";
    applicationInfo.codePath = "/codePath";
    ApplicationEnvImpl::GetInstance()->SetAppInfo(applicationInfo);
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetBundleName(), "bundleName");
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetSrcPath(), "/codePath");
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetDataPath(), "/dataDir");
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvImplTest_SetAppInfo_0100 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationEnvImplTest_SetAppInfo_0200
 * @tc.name: ApplicationEnvImpl SetAppInfo
 * @tc.desc: Test ApplicationEnvImpl set and get appInfo.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvImplTest_SetAppInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvImplTest_SetAppInfo_0200 start";
    AppInfo appInfo;
    appInfo.bundleName = "bundleName";
    appInfo.dataPath = "/dataDir";
    appInfo.srcPath = "/codePath";
    ApplicationEnvImpl::GetInstance()->SetAppInfo(appInfo);
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetBundleName(), "bundleName");
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetSrcPath(), "/codePath");
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetDataPath(), "/dataDir");
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvImplTest_SetAppInfo_0200 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationEnvTest_SetAppInfo_0100
 * @tc.name: ApplicationEnv SetAppInfo
 * @tc.desc: Test ApplicationEnv get appInfo.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvTest_SetAppInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0100 start";
    AppInfo appInfo;
    appInfo.bundleName = "bundleName";
    appInfo.dataPath = "/dataDir";
    appInfo.srcPath = "/codePath";
    ApplicationEnvImpl::GetInstance()->SetAppInfo(appInfo);
    std::string bundleName(GetBundleName());
    std::string codePath(GetSrcPath());
    std::string dataDir(GetDataPath());
    EXPECT_EQ(bundleName, "bundleName");
    EXPECT_EQ(codePath, "/codePath");
    EXPECT_EQ(dataDir, "/dataDir");
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0100 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationEnvTest_SetAppInfo_0200
 * @tc.name: ApplicationEnv SetAppInfo
 * @tc.desc: Test ApplicationEnv get applicationInfo.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvTest_SetAppInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0200 start";
    ApplicationInfo applicationInfo;
    applicationInfo.bundleName = "bundleName";
    applicationInfo.dataDir = "/dataDir";
    applicationInfo.codePath = "/codePath";
    ApplicationEnvImpl::GetInstance()->SetAppInfo(applicationInfo);
    std::string bundleName(GetBundleName());
    std::string codePath(GetSrcPath());
    std::string dataDir(GetDataPath());
    EXPECT_EQ(bundleName, "bundleName");
    EXPECT_EQ(codePath, "/codePath");
    EXPECT_EQ(dataDir, "/dataDir");
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0200 end";
}

}  // namespace AppExecFwk
}  // namespace OHOS
