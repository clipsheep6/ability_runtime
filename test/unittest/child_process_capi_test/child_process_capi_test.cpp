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
#include <gmock/gmock.h>
#include "native_child_process.h"
#include "app_utils.h"

namespace OHOS {
namespace AbilityRuntime {

using namespace testing;
using namespace testing::ext;

class ChildProcessCapiTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);

    static void OnNativeChildProcessStarted(int errCode, OHIPCRemoteProxy *remoteProxy);

    void SetUp();
    void TearDown();
};

void ChildProcessCapiTest::SetUpTestCase(void)
{}

void ChildProcessCapiTest::TearDownTestCase(void)
{}

void ChildProcessCapiTest::SetUp(void)
{}

void ChildProcessCapiTest::TearDown(void)
{}

void ChildProcessCapiTest::OnNativeChildProcessStarted(int errCode, OHIPCRemoteProxy *remoteProxy)
{
}

/**
 * @tc.number: OH_Ability_CreateNativeChildProcess_001
 * @tc.desc: Test API OH_Ability_CreateNativeChildProcess works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_CreateNativeChildProcess_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "OH_Ability_CreateNativeChildProcess_001 begin";
    int ret = OH_Ability_CreateNativeChildProcess(nullptr, ChildProcessCapiTest::OnNativeChildProcessStarted);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);

    ret = OH_Ability_CreateNativeChildProcess("test.so", nullptr);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);

    ret = OH_Ability_CreateNativeChildProcess("test.so", ChildProcessCapiTest::OnNativeChildProcessStarted);
    if (!AAFwk::AppUtils::GetInstance().IsMultiProcessModel()) {
        EXPECT_EQ(ret, NCP_ERR_MULTI_PROCESS_DISABLED);
        return;
    } else if (!AAFwk::AppUtils::GetInstance().IsSupportNativeChildProcess()) {
        EXPECT_EQ(ret, NCP_ERR_NOT_SUPPORTED);
        return;
    }

    GTEST_LOG_(INFO) << "OH_Ability_CreateNativeChildProcess return " << ret;
    EXPECT_NE(ret, NCP_ERR_NOT_SUPPORTED);
}

}  // namespace AbilityRuntime
}  // namespace OHOS
