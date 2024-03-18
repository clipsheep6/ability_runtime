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

#include "child_process.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "runtime.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class ChildProcessTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ChildProcessTest::SetUpTestCase()
{}

void ChildProcessTest::TearDownTestCase()
{}

void ChildProcessTest::SetUp()
{}

void ChildProcessTest::TearDown()
{}

/**
 * @tc.number: ChildProcessCreate_0100
 * @tc.desc: Test ChildProcess Create works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessTest, ChildProcessCreate_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessCreate_0100 called.");
    std::unique_ptr<Runtime> runtime;
    auto process = ChildProcess::Create(runtime);
    EXPECT_TRUE(process != nullptr);
}

/**
 * @tc.number: ChildProcessInit_0100
 * @tc.desc: Test ChildProcess Init works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessTest, ChildProcessInit_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessInit_0100 called.");
    std::unique_ptr<Runtime> runtime;
    auto process = ChildProcess::Create(runtime);
    EXPECT_TRUE(process != nullptr);

    std::shared_ptr<ChildProcessStartInfo> info = std::make_shared<ChildProcessStartInfo>();
    auto ret = process->Init(info);
    EXPECT_TRUE(ret);
}
}  // namespace AbilityRuntime
}  // namespace OHOS