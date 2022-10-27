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

#include "quick_fix_error_utils.h"
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class QuickFixErrorUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void QuickFixErrorUtilsTest::SetUpTestCase(void)
{}

void QuickFixErrorUtilsTest::TearDownTestCase(void)
{}

void QuickFixErrorUtilsTest::SetUp()
{}

void QuickFixErrorUtilsTest::TearDown()
{}

/**
 * @tc.name: GetErrorCode_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    int32_t errCode;

    // external error code
    errCode = QuickFixErrorUtil::GetErrorCode(ERR_QUICKFIX_PERMISSION_DENIED);
    EXPECT_EQ(errCode, ERR_QUICKFIX_PERMISSION_DENIED);

    // internal error code
    errCode = QuickFixErrorUtil::GetErrorCode(QUICK_FIX_COPY_FILES_FAILED);
    EXPECT_EQ(errCode, ERR_QUICKFIX_HQF_INVALID);

    // unknown error code
    errCode = QuickFixErrorUtil::GetErrorCode(1000); // 1000 is not a defined error code in quick fix
    EXPECT_EQ(errCode, ERR_QUICKFIX_INTERNAL_ERROR);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorMessage_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorMessage_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::string errMsg;

    // external error code
    errMsg = QuickFixErrorUtil::GetErrorMessage(ERR_QUICKFIX_PARAM_INVALID);
    EXPECT_EQ(errMsg, "Invalid input parameter.");

    // internal error code
    errMsg = QuickFixErrorUtil::GetErrorMessage(QUICK_FIX_WRITE_PARCEL_FAILED);
    EXPECT_EQ(errMsg, "Internal error. Write parcel failed.");

    // unknown error code
    errMsg = QuickFixErrorUtil::GetErrorMessage(1000); // 1000 is not a defined error code in quick fix
    EXPECT_EQ(errMsg, "Internal error.");

    HILOG_INFO("%{public}s end.", __func__);
}
} // namespace AAFwk
} // namespace OHOS