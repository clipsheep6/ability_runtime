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

#include "cj_ability_delegator.h"
#include "cj_application_context.h"
#include "ability_delegator_registry.h"
#include "application_context.h"
#include "cj_utils_ffi.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::FFI;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace ApplicationContextCJ {

class CjApplicationContextTest : public testing::Test {
public:
    CjApplicationContextTest()
    {}
    ~CjApplicationContextTest()
    {}
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
protected:
    std::shared_ptr<AbilityRuntime::ApplicationContext> appContext_;
    std::shared_ptr<CJApplicationContext> cjAppContext_;
};

void CjApplicationContextTest::SetUpTestCase()
{}

void CjApplicationContextTest::TearDownTestCase()
{}

void CjApplicationContextTest::SetUp()
{
}

void CjApplicationContextTest::TearDown()
{}

/**
 * @tc.name: CJApplicationContextTestGetArea_001
 * @tc.desc: CjApplicationContextTest test for GetArea.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestGetArea_001, TestSize.Level1)
{
    // 创建一个 ApplicationContext 对象
    auto appInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    appInfo->name = "TestApp";
    appInfo->bundleName = "com.example.testapp";
    appContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    appContext_->SetApplicationInfo(appInfo);

    // 创建一个 CJApplicationContext 对象
    cjAppContext_ = std::make_shared<CJApplicationContext>(appContext_);
    // 测试 GetArea 函数
    int area = cjAppContext_->GetArea();
    EXPECT_EQ(area, 1);
}

/**
 * @tc.name: CJApplicationContextTestGetApplicationInfo_001
 * @tc.desc: CjApplicationContextTest test for GetApplicationInfo.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestGetApplicationInfo_001, TestSize.Level1)
{
    // 创建一个 ApplicationContext 对象
    auto appInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    appInfo->name = "TestApp";
    appInfo->bundleName = "com.example.testapp";
    appContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    appContext_->SetApplicationInfo(appInfo);

    // 创建一个 CJApplicationContext 对象
    cjAppContext_ = std::make_shared<CJApplicationContext>(appContext_);

    // 测试 GetApplicationInfo 函数
    auto appInfoResult = cjAppContext_->GetApplicationInfo();
}

}  // namespace AbilityRuntime
}  // namespace OHOS