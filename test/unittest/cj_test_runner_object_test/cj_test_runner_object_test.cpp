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

#include "runner_runtime/cj_test_runner_object.h"

#include "gtest/gtest.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace RunnerRuntime {
class CjTestRunnerObjectTest : public ::testing::Test {
public:
    CjTestRunnerObjectTest()
    {}
    ~CjTestRunnerObjectTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void CjTestRunnerObjectTest::SetUpTestCase()
{}

void CjTestRunnerObjectTest::TearDownTestCase()
{}

int64_t create(const char* name)
{
    return 1;
}

void release(int64_t id) {}

void onRun(int64_t id) {}

void onPrepare(int64_t id) {}

// 模拟 Cangjie 侧的函数注册函数
void RegisterCangjieFuncs(CJTestRunnerFuncs* funcs)
{
    funcs->cjTestRunnerCreate = create;
    funcs->cjTestRunnerRelease = release;
    funcs->cjTestRunnerOnRun = onRun;
    funcs->cjTestRunnerOnPrepare = onPrepare;
}

void CjTestRunnerObjectTest::SetUp()
{
}

void CjTestRunnerObjectTest::TearDown()
{
}

/**
 * @tc.name: CjTestRunnerObjectTestLoadModule_Success_001
 * @tc.desc: CjTestRunnerObjectTest test for LoadModule.
 * @tc.type: FUNC
 */
HWTEST_F(CjTestRunnerObjectTest, CjTestRunnerObjectTestLoadModule_Success_001, TestSize.Level0)
{
    RegisterCJTestRunnerFuncs(nullptr);
    RegisterCJTestRunnerFuncs(RegisterCangjieFuncs);
    std::shared_ptr<CJTestRunnerObject> proxy = CJTestRunnerObject::LoadModule("test_ability");
    EXPECT_NE(nullptr, proxy);
    proxy->OnPrepare();
    proxy->OnRun();

    proxy.reset();
}

/**
 * @tc.name: CjTestRunnerObjectTestLoadModule_Failed_NoRegistration_001
 * @tc.desc: CjTestRunnerObjectTest test for LoadModule.
 * @tc.type: FUNC
 */
HWTEST_F(CjTestRunnerObjectTest, CjTestRunnerObjectTestLoadModule_Failed_NoRegistration_001, TestSize.Level0)
{
    auto proxy = CJTestRunnerObject::LoadModule("test_ability");
    EXPECT_NE(nullptr, proxy);
}

/**
 * @tc.name: CjAbilityStageTestOnMemoryLevel_001
 * @tc.desc: CjTestRunnerObjectTest test for OnMemoryLevel.
 * @tc.type: FUNC
 */
HWTEST_F(CjTestRunnerObjectTest, CjTestRunnerObjectTestLoadModule_Failed_CangjieCreateFailed_001, TestSize.Level0)
{
    RegisterCJTestRunnerFuncs(RegisterCangjieFuncs);
    auto proxy = CJTestRunnerObject::LoadModule("test_ability");
    EXPECT_NE(nullptr, proxy);
}

/**
 * @tc.name: CjTestRunnerObjectTestOnRun_Success_001
 * @tc.desc: CjTestRunnerObjectTest test for OnRun.
 * @tc.type: FUNC
 */
HWTEST_F(CjTestRunnerObjectTest, CjTestRunnerObjectTestOnRun_Success_001, TestSize.Level0)
{
    RegisterCJTestRunnerFuncs(RegisterCangjieFuncs);
    auto proxy = CJTestRunnerObject::LoadModule("test_ability");
    proxy->OnRun();
    proxy.reset();
}

/**
 * @tc.name: CjTestRunnerObjectTestOnRun_Failed_NoRegistration_001
 * @tc.desc: CjTestRunnerObjectTest test for OnRun.
 * @tc.type: FUNC
 */
HWTEST_F(CjTestRunnerObjectTest, CjTestRunnerObjectTestOnRun_Failed_NoRegistration_001, TestSize.Level0)
{
    auto proxy = CJTestRunnerObject::LoadModule("test_ability");
    proxy->OnRun();
}

/**
 * @tc.name: CjTestRunnerObjectTestOnPrepare_Success_001
 * @tc.desc: CjTestRunnerObjectTest test for OnPrepare.
 * @tc.type: FUNC
 */
HWTEST_F(CjTestRunnerObjectTest, CjTestRunnerObjectTestOnPrepare_Success_001, TestSize.Level0)
{
    RegisterCJTestRunnerFuncs(RegisterCangjieFuncs);
    auto proxy = CJTestRunnerObject::LoadModule("test_ability");
    proxy->OnPrepare();
}

/**
 * @tc.name: CjTestRunnerObjectTestOnPrepare_Failed_NoRegistration_001
 * @tc.desc: CjTestRunnerObjectTest test for OnPrepare.
 * @tc.type: FUNC
 */
HWTEST_F(CjTestRunnerObjectTest, CjTestRunnerObjectTestOnPrepare_Failed_NoRegistration_001, TestSize.Level0)
{
    auto proxy = CJTestRunnerObject::LoadModule("test_ability");
    proxy->OnPrepare();
}

/**
 * @tc.name: CjTestRunnerObjectTestDestructor_Success_001
 * @tc.desc: CjTestRunnerObjectTest test for reset.
 * @tc.type: FUNC
 */
HWTEST_F(CjTestRunnerObjectTest, CjTestRunnerObjectTestDestructor_Success_001, TestSize.Level0)
{
    RegisterCJTestRunnerFuncs(RegisterCangjieFuncs);
    auto proxy = CJTestRunnerObject::LoadModule("test_ability");
    proxy.reset();
}
} // namespace RunnerRuntime
} // namespace OHOS
