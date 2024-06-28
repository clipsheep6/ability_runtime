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

#define protected public
#include "js_child_process.h"
#undef protected
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class JsChildProcessTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void JsChildProcessTest::SetUpTestCase()
{}

void JsChildProcessTest::TearDownTestCase()
{}

void JsChildProcessTest::SetUp()
{}

void JsChildProcessTest::TearDown()
{}

/**
 * @tc.number: JsChildProcessCreate_0100
 * @tc.desc: Test JsChildProcessTest Create works
 * @tc.type: FUNC
 */
HWTEST_F(JsChildProcessTest, JsChildProcessCreate_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "JsChildProcessCreate_0100 called.");
    std::unique_ptr<Runtime> runtime = std::make_unique<JsRuntime>();
    auto process = JsChildProcess::Create(runtime);
    EXPECT_TRUE(process != nullptr);
}

/**
 * @tc.number: JsChildProcessInit_0100
 * @tc.desc: Test JsChildProcess Init works
 * @tc.type: FUNC
 */
HWTEST_F(JsChildProcessTest, JsChildProcessInit_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "JsChildProcessInit_0100 called.");
    std::unique_ptr<Runtime> runtime = std::make_unique<JsRuntime>();
    auto process = JsChildProcess::Create(runtime);
    EXPECT_TRUE(process != nullptr);

    std::shared_ptr<ChildProcessStartInfo> info = std::make_shared<ChildProcessStartInfo>();
    info->name = "AProcess";
    info->srcEntry = "./ets/process/AProcess.ts";
    info->moduleName = "entry";

    process->Init(info);
    EXPECT_TRUE(process->processStartInfo_ != nullptr);
}

/**
 * @tc.number: JsChildProcessInit_0200
 * @tc.desc: Test JsChildProcess Init works
 * @tc.type: FUNC
 */
HWTEST_F(JsChildProcessTest, JsChildProcessInit_0200, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "JsChildProcessInit_0200 called.");
    std::unique_ptr<Runtime> runtime = std::make_unique<JsRuntime>();
    auto process = JsChildProcess::Create(runtime);
    EXPECT_TRUE(process != nullptr);

    auto ret = process->Init(nullptr);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: JsChildProcessInit_0300
 * @tc.desc: Test JsChildProcess Init works
 * @tc.type: FUNC
 */
HWTEST_F(JsChildProcessTest, JsChildProcessInit_0300, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "JsChildProcessInit_0300 called.");
    std::unique_ptr<Runtime> runtime = std::make_unique<JsRuntime>();
    auto process = JsChildProcess::Create(runtime);
    EXPECT_TRUE(process != nullptr);

    std::shared_ptr<ChildProcessStartInfo> info = std::make_shared<ChildProcessStartInfo>();
    info->name = "AProcess";
    info->srcEntry = "";
    info->moduleName = "entry";

    auto ret = process->Init(info);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: JsChildProcessOnStart_0100
 * @tc.desc: Test JsChildProcess OnStart works
 * @tc.type: FUNC
 */
HWTEST_F(JsChildProcessTest, JsChildProcessOnStart_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "JsChildProcessOnStart_0100 called.");
    std::unique_ptr<Runtime> runtime = std::make_unique<JsRuntime>();
    auto process = JsChildProcess::Create(runtime);
    EXPECT_TRUE(process != nullptr);

    std::shared_ptr<ChildProcessStartInfo> info = std::make_shared<ChildProcessStartInfo>();
    info->name = "AProcess";
    info->srcEntry = "./ets/process/AProcess.ts";
    info->moduleName = "entry";

    process->Init(info);
    process->OnStart();
    EXPECT_TRUE(process->processStartInfo_ != nullptr);
}
}  // namespace AbilityRuntime
}  // namespace OHOS