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
#include "ability_impl_factory.h"
#include "ohos_application.h"
#include "page_ability_impl.h"
#undef private

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;

class AbilityImplFactoryTest : public testing::Test {
public:
    AbilityImplFactoryTest() : abilityImplFactory_(nullptr)
    {}
    ~AbilityImplFactoryTest()
    {}
    std::shared_ptr<AbilityImplFactory> abilityImplFactory_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AbilityImplFactoryTest::SetUpTestCase(void)
{}

void AbilityImplFactoryTest::TearDownTestCase(void)
{}

void AbilityImplFactoryTest::SetUp(void)
{
    abilityImplFactory_ = std::make_shared<AbilityImplFactory>();
}

void AbilityImplFactoryTest::TearDown(void)
{}

/**
 * @tc.number: Ability_Impl_Factory_MakeAbilityImplObject_0100
 * @tc.name: MakeAbilityImplObject
 * @tc.desc: call MakeAbilityImplObject with info is null
 */
HWTEST_F(AbilityImplFactoryTest, Ability_Impl_Factory_MakeAbilityImplObject_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Impl_Factory_MakeAbilityImplObject_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    auto result = abilityImplFactory_->MakeAbilityImplObject(abilityInfo);
    EXPECT_TRUE(result == nullptr);
    GTEST_LOG_(INFO) << "Ability_Impl_Factory_MakeAbilityImplObject_0100 end";
}
#ifdef SUPPORT_GRAPHICS
/**
 * @tc.number: Ability_Impl_Factory_MakeAbilityImplObject_0200
 * @tc.name: MakeAbilityImplObject
 * @tc.desc: call MakeAbilityImplObject with type is PAGE and isStageBasedModel is true
 */
HWTEST_F(AbilityImplFactoryTest, Ability_Impl_Factory_MakeAbilityImplObject_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Impl_Factory_MakeAbilityImplObject_0200 start";
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    abilityInfo->isStageBasedModel = true;
    auto result = abilityImplFactory_->MakeAbilityImplObject(abilityInfo);
    EXPECT_TRUE(result != nullptr);
    GTEST_LOG_(INFO) << "Ability_Impl_Factory_MakeAbilityImplObject_0200 end";
}

/**
 * @tc.number: Ability_Impl_Factory_MakeAbilityImplObject_0300
 * @tc.name: MakeAbilityImplObject
 * @tc.desc: call MakeAbilityImplObject with type is PAGE and isStageBasedModel is false
 */
HWTEST_F(AbilityImplFactoryTest, Ability_Impl_Factory_MakeAbilityImplObject_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Impl_Factory_MakeAbilityImplObject_0300 start";
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    abilityInfo->isStageBasedModel = false;
    auto result = abilityImplFactory_->MakeAbilityImplObject(abilityInfo);
    EXPECT_TRUE(result != nullptr);
    GTEST_LOG_(INFO) << "Ability_Impl_Factory_MakeAbilityImplObject_0300 end";
}
#endif
/**
 * @tc.number: Ability_Impl_Factory_MakeAbilityImplObject_0400
 * @tc.name: MakeAbilityImplObject
 * @tc.desc: call MakeAbilityImplObject with type is SERVICE
 */
HWTEST_F(AbilityImplFactoryTest, Ability_Impl_Factory_MakeAbilityImplObject_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Impl_Factory_MakeAbilityImplObject_0400 start";
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    auto result = abilityImplFactory_->MakeAbilityImplObject(abilityInfo);
    EXPECT_TRUE(result != nullptr);
    GTEST_LOG_(INFO) << "Ability_Impl_Factory_MakeAbilityImplObject_0400 end";
}

/**
 * @tc.number: Ability_Impl_Factory_MakeAbilityImplObject_0500
 * @tc.name: MakeAbilityImplObject
 * @tc.desc: call MakeAbilityImplObject with type is DATA
 */
HWTEST_F(AbilityImplFactoryTest, Ability_Impl_Factory_MakeAbilityImplObject_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Impl_Factory_MakeAbilityImplObject_0500 start";
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::DATA;
    auto result = abilityImplFactory_->MakeAbilityImplObject(abilityInfo);
    EXPECT_TRUE(result != nullptr);
    GTEST_LOG_(INFO) << "Ability_Impl_Factory_MakeAbilityImplObject_0500 end";
}

/**
 * @tc.number: Ability_Impl_Factory_MakeAbilityImplObject_0600
 * @tc.name: MakeAbilityImplObject
 * @tc.desc: call MakeAbilityImplObject with type is illeagal
 */
HWTEST_F(AbilityImplFactoryTest, Ability_Impl_Factory_MakeAbilityImplObject_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Ability_Impl_Factory_MakeAbilityImplObject_0600 start";
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::FORM;
    auto result = abilityImplFactory_->MakeAbilityImplObject(abilityInfo);
    EXPECT_TRUE(result == nullptr);
    GTEST_LOG_(INFO) << "Ability_Impl_Factory_MakeAbilityImplObject_0600 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
