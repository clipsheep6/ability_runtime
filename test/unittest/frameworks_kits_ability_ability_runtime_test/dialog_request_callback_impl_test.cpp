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
#include "dialog_request_callback_impl.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;
namespace OHOS {
namespace AAFwk {
class DialogRequestCallbackImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void DialogRequestCallbackImplTest::SetUpTestCase(void) {}
void DialogRequestCallbackImplTest::TearDownTestCase(void) {}
void DialogRequestCallbackImplTest::TearDown() {}
void DialogRequestCallbackImplTest::SetUp() {}

void RequestDialogResultTaskCallBack(int32_t resultCode, const AAFwk::Want&)
{
    GTEST_LOG_(INFO) << "RequestDialogResultTask call back";
}

/**
 * @tc.name: DialogRequestCallbackImplTest_SendResult_0100
 * @tc.desc: Test the state of SendResult
 * @tc.type: FUNC
 */
HWTEST_F(DialogRequestCallbackImplTest, SendResult_0100, TestSize.Level1)
{
    auto dialogRequestCallbackImpl = std::make_shared<DialogRequestCallbackImpl>(RequestDialogResultTaskCallBack);
    Want want;
    dialogRequestCallbackImpl->SendResult(401, want);
}

} // namespace AAFwk
} // namespace OHOS
