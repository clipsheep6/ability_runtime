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
#include <securec.h>

#include "cj_ability_stage.h"
#include "cj_runtime.h"
#include "hilog_wrapper.h"
#include "runtime.h"
#include "cj_ability_stage_object.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class CjAbilityStageTest : public testing::Test {
public:
    CjAbilityStageTest()
    {}
    ~CjAbilityStageTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
    std::unique_ptr<AbilityRuntime::CJAbilityStage> cjAbilityStage_ = nullptr;
    std::unique_ptr<AbilityRuntime::CJAbilityStage> initCjAbilityStage_ = nullptr;
};

void (*g_registerFunc)(CJAbilityStageFuncs* result) = [](CJAbilityStageFuncs *result) {
        result->LoadAbilityStage = [](const char *moduleName) -> int64_t { return moduleName[0] == '0' ? 0 : 1; };
        result->ReleaseAbilityStage = [](int64_t handle) {};
        result->AbilityStageOnCreate = [](int64_t handle) {};
        result->AbilityStageOnAcceptWant = [](int64_t handle, OHOS::AAFwk::Want *want) -> char* {
            std::string str = "Hello, world!";
            char* cstr = new char[str.length() + 1];
            if (memcpy_s(cstr, str.length() + 1, str.c_str(), str.size()) != EOK) {
                delete[] cstr;
                return nullptr;
            }
            return cstr;
            };
        result->AbilityStageOnConfigurationUpdated = [](int64_t id, CJConfiguration configuration) {};
        result->AbilityStageOnMemoryLevel = [](int64_t id, int32_t level) {};
    };
void CjAbilityStageTest::SetUpTestCase(void)
{}

void CjAbilityStageTest::TearDownTestCase(void)
{}

void CjAbilityStageTest::SetUp(void)
{
    RegisterCJAbilityStageFuncs(g_registerFunc);
    std::string moduleName = "0";
    auto proxy = CJAbilityStageObject::LoadModule(moduleName);
    cjAbilityStage_ = std::make_unique<AbilityRuntime::CJAbilityStage>(proxy);
    auto cjProxy = CJAbilityStageObject::LoadModule("1");
    initCjAbilityStage_ = std::make_unique<AbilityRuntime::CJAbilityStage>(cjProxy);
}

void CjAbilityStageTest::TearDown(void)
{}

/**
 * @tc.name: CjAbilityStageTestCreate_001
 * @tc.desc: CjAbilityStageTest test for Create.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityStageTest, CjAbilityStageTestCreate_001, TestSize.Level0)
{
    AppExecFwk::HapModuleInfo hapModuleInfo;
    std::unique_ptr<Runtime> nullRuntime = nullptr;
    auto cjStage = CJAbilityStage::Create(nullRuntime, hapModuleInfo);
    Runtime::Options options;
    auto runtime = Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);
    auto cjAbilityStage = CJAbilityStage::Create(runtime, hapModuleInfo);
    EXPECT_TRUE(cjAbilityStage == nullptr);
}

/**
 * @tc.name: CjAbilityStageTestOnCreate_001
 * @tc.desc: CjAbilityStageTest test for OnCreate.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityStageTest, CjAbilityStageTestOnCreate_001, TestSize.Level0)
{
    AAFwk::Want want;
    cjAbilityStage_->OnCreate(want);
    initCjAbilityStage_->OnCreate(want);
}

/**
 * @tc.name: CjAbilityStageTestOnAcceptWant_001
 * @tc.desc: CjAbilityStageTest test for OnAcceptWant.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityStageTest, CjAbilityStageTestOnAcceptWant_001, TestSize.Level0)
{
    AAFwk::Want want;
    auto info = cjAbilityStage_->OnAcceptWant(want);
    EXPECT_TRUE(info == "");
    auto ret = initCjAbilityStage_->OnAcceptWant(want);
    EXPECT_TRUE(ret != "");
}

/**
 * @tc.name: CjAbilityStageTestOnMemoryLevel_001
 * @tc.desc: CjAbilityStageTest test for OnMemoryLevel.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityStageTest, CjAbilityStageTestOnMemoryLevel_001, TestSize.Level0)
{
    int level = 1;
    cjAbilityStage_->OnMemoryLevel(level);
    initCjAbilityStage_->OnMemoryLevel(level);
}

}  // namespace AbilityRuntime
}  // namespace OHOS
