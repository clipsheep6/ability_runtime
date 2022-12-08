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
#include "form_extension_context.h"
#undef private
#include "ability_manager_client.h"
#include "appexecfwk_errors.h"
#include "form_mgr.h"
#include "form_mgr_errors.h"
#include "hilog_wrapper.h"
#include "ability_connection.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AbilityRuntime {

class FormExtensionContextTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FormExtensionContextTest::SetUpTestCase(void)
{}
void FormExtensionContextTest::TearDownTestCase(void)
{}
void FormExtensionContextTest::SetUp(void)
{}
void FormExtensionContextTest::TearDown(void)
{}

/*
 * Feature: FormExtensionContext
 * Function: startAbility
 * SubFunction: NA
 * FunctionPoints: ServiceExtensionContextTest StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify startAbility
 */
HWTEST_F(FormExtensionContextTest, form_extension_context_startAbility_001, TestSize.Level1)
{   
    FormExtensionContext formExtensionContext;
    Want want;
    int result = formExtensionContext.StartAbility(want);
    EXPECT_NE(ERR_IMPLICIT_START_ABILITY_FAIL, result);
}

/*
 * Feature: FormExtensionContext
 * Function: startAbility
 * SubFunction: NA
 * FunctionPoints: ServiceExtensionContextTest GetAbilityInfoType
 * EnvConditions: NA
 * CaseDescription: Verify startAbility
 */
HWTEST_F(FormExtensionContextTest, form_extension_context_GetAbilityInfoType_001, TestSize.Level1)
{   
    FormExtensionContext formExtensionContext;
    EXPECT_EQ(AppExecFwk::AbilityType::UNKNOWN, formExtensionContext.GetAbilityInfoType());
}
}
}