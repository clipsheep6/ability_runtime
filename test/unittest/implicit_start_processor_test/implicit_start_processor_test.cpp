/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "implicit_start_processor.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {

class ImplicitStartProcessorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ImplicitStartProcessorTest::SetUpTestCase(void)
{}
void ImplicitStartProcessorTest::TearDownTestCase(void)
{}
void ImplicitStartProcessorTest::SetUp()
{}
void ImplicitStartProcessorTest::TearDown()
{}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbility
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbility
 * EnvConditions: NA
 * CaseDescription: Verify ImplicitStartAbility
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbility_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    int32_t userId = 0;
    bool res = processor->ImplicitStartAbility(request, userId);
    EXPECT_TRUE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CheckImplicitStartExtensionIsValid
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CheckImplicitStartExtensionIsValid
 * EnvConditions: NA
 * CaseDescription: Verify CheckImplicitStartExtensionIsValid
 */
HWTEST_F(ImplicitStartProcessorTest, CheckImplicitStartExtensionIsValid_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    ExtensionAbilityInfo extensionInfo;
    Want want;
    want.SetElementName("bundle", "");
    request.want = want;
    bool res = processor->CheckImplicitStartExtensionIsValid(request, extensionInfo);
    EXPECT_TRUE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CheckImplicitStartExtensionIsValid
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CheckImplicitStartExtensionIsValid
 * EnvConditions: NA
 * CaseDescription: Verify CheckImplicitStartExtensionIsValid
 */
HWTEST_F(ImplicitStartProcessorTest, CheckImplicitStartExtensionIsValid_002, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    ExtensionAbilityInfo extensionInfo;
    Want want;
    want.SetElementName("", "");
    request.want = want;
    extensionInfo.type = ExtensionAbilityType::WORK_SCHEDULER;
    bool res = processor->CheckImplicitStartExtensionIsValid(request, extensionInfo);
    EXPECT_FALSE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CheckImplicitStartExtensionIsValid
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CheckImplicitStartExtensionIsValid
 * EnvConditions: NA
 * CaseDescription: Verify CheckImplicitStartExtensionIsValid
 */
HWTEST_F(ImplicitStartProcessorTest, CheckImplicitStartExtensionIsValid_003, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    ExtensionAbilityInfo extensionInfo;
    Want want;
    want.SetElementName("", "");
    request.want = want;
    extensionInfo.type = ExtensionAbilityType::FORM;
    bool res = processor->CheckImplicitStartExtensionIsValid(request, extensionInfo);
    EXPECT_TRUE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify ImplicitStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbilityInner_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    AbilityRequest request;
    int32_t userId = 0;
    request.callType = AbilityCallType::START_OPTIONS_TYPE;
    bool res = processor->ImplicitStartAbilityInner(want, request, userId);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify ImplicitStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbilityInner_002, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    AbilityRequest request;
    int32_t userId = 0;
    request.callType = AbilityCallType::START_SETTINGS_TYPE;
    bool res = processor->ImplicitStartAbilityInner(want, request, userId);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify ImplicitStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbilityInner_003, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    AbilityRequest request;
    int32_t userId = 0;
    request.callType = AbilityCallType::START_EXTENSION_TYPE;
    bool res = processor->ImplicitStartAbilityInner(want, request, userId);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify ImplicitStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbilityInner_004, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    AbilityRequest request;
    int32_t userId = 0;
    request.callType = AbilityCallType::CALL_REQUEST_TYPE;
    bool res = processor->ImplicitStartAbilityInner(want, request, userId);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CallStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CallStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify CallStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, CallStartAbilityInner_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    int32_t userId = 0;
    Want want;
    ImplicitStartProcessor::StartAbilityClosure callBack = []() -> int32_t {
        return 1;
    };
    AbilityCallType callType = AbilityCallType::INVALID_TYPE;
    bool res = processor->CallStartAbilityInner(userId, want, callBack, callType);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CallStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CallStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify CallStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, CallStartAbilityInner_002, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    int32_t userId = 0;
    Want want;
    ImplicitStartProcessor::StartAbilityClosure callBack = []() -> int32_t {
        return 1;
    };
    AbilityCallType callType = AbilityCallType::CALL_REQUEST_TYPE;
    bool res = processor->CallStartAbilityInner(userId, want, callBack, callType);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CallStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CallStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify CallStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, CallStartAbilityInner_003, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    int32_t userId = 0;
    Want want;
    ImplicitStartProcessor::StartAbilityClosure callBack = []() -> int32_t {
        return ERR_OK;
    };
    AbilityCallType callType = AbilityCallType::CALL_REQUEST_TYPE;
    bool res = processor->CallStartAbilityInner(userId, want, callBack, callType);
    EXPECT_EQ(res, ERR_OK);
}
}  // namespace AAFwk
}  // namespace OHOS
