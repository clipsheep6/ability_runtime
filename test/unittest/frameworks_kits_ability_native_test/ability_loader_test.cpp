/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "ability_loader.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "iremote_object.h"
#undef protected
#undef private

using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace testing;
using namespace testing::ext;
using CreateAblity = std::function<Ability *(void)>;
using CreateExtension = std::function<AbilityRuntime::Extension *(void)>;
using CreateUIAblity = std::function<AbilityRuntime::UIAbility *(void)>;

class AbilityLoaderTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityLoaderTest::SetUpTestCase()
{}

void AbilityLoaderTest::TearDownTestCase()
{}

void AbilityLoaderTest::SetUp()
{}

void AbilityLoaderTest::TearDown()
{}

/**
 * @tc.number: RegisterAbility_0100
 * @tc.name: RegisterAbility
 * @tc.desc: RegisterAbility Test, return true.
 */
HWTEST_F(AbilityLoaderTest, RegisterAbility_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityLoaderTest RegisterAbility_0100 start";
    auto loader = std::make_shared<OHOS::AppExecFwk::AbilityLoader>();
    std::string abilityName;
    CreateAblity createFunc;
    loader->RegisterAbility(abilityName, createFunc);
    bool result = false;
    auto it = loader->abilities_.find(abilityName);
    if (it != loader->abilities_.end()) {
        result = true;
    }
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "AbilityLoaderTest RegisterAbility_0100 end";
}

/**
 * @tc.number: RegisterExtension_0100
 * @tc.name: RegisterExtension
 * @tc.desc: RegisterExtension Test, return true.
 */
HWTEST_F(AbilityLoaderTest, RegisterExtension_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityLoaderTest RegisterExtension_0100 start";
    auto loader = std::make_shared<OHOS::AppExecFwk::AbilityLoader>();
    std::string abilityName;
    CreateExtension createFunc;
    loader->RegisterExtension(abilityName, createFunc);
    bool result = false;
    auto it = loader->extensions_.find(abilityName);
    if (it != loader->extensions_.end()) {
        result = true;
    }
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "AbilityLoaderTest RegisterExtension_0100 end";
}

/**
 * @tc.number: GetAbilityByName_0100
 * @tc.name: GetAbilityByName
 * @tc.desc: GetAbilityByName Test When Ability is not nullptr.
 */
HWTEST_F(AbilityLoaderTest, GetAbilityByName_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityLoaderTest GetAbilityByName_0100 start";
    std::string abilityName = "Ability";
    CreateAblity createFunc;
    auto createAblity = []() -> Ability *{
        Ability *callBack = new (std::nothrow) Ability;
        return callBack;
    };
    AbilityLoader::GetInstance().abilities_.clear();
    AbilityLoader::GetInstance().RegisterAbility(abilityName, createAblity);
    EXPECT_TRUE(AbilityLoader::GetInstance().GetAbilityByName(abilityName) != nullptr);
    GTEST_LOG_(INFO) << "AbilityLoaderTest GetAbilityByName_0100 end";
}

/**
 * @tc.number: GetAbilityByName_0200
 * @tc.name: GetAbilityByName
 * @tc.desc: GetAbilityByName Test When Ability is nullptr.
 */
HWTEST_F(AbilityLoaderTest, GetAbilityByName_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityLoaderTest GetAbilityByName_0200 start";
    std::string abilityName = "abilityName";
    AbilityLoader::GetInstance().abilities_.clear();
    EXPECT_FALSE(AbilityLoader::GetInstance().GetAbilityByName(abilityName) != nullptr);
    GTEST_LOG_(INFO) << "AbilityLoaderTest GetAbilityByName_0200 start";
}

/**
 * @tc.number: GetExtensionByName_0100
 * @tc.name: GetExtensionByName
 * @tc.desc: GetExtensionByName Test When AbilityRuntime::Extension is not nullptr.
 */
HWTEST_F(AbilityLoaderTest, GetExtensionByName_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityLoaderTest GetExtensionByName_0100 start";
    std::string abilityName = "AbilityRuntime::Extension";
    CreateExtension createFunc;
    auto createExtension = []() -> AbilityRuntime::Extension *{
        AbilityRuntime::Extension *callBack = new (std::nothrow) AbilityRuntime::Extension;
        return callBack;
    };
    AbilityLoader::GetInstance().extensions_.clear();
    AbilityLoader::GetInstance().RegisterExtension(abilityName, createExtension);
    EXPECT_TRUE(AbilityLoader::GetInstance().GetExtensionByName(abilityName) != nullptr);
    GTEST_LOG_(INFO) << "AbilityLoaderTest GetExtensionByName_0100 end";
}

/**
 * @tc.number: GetExtensionByName_0200
 * @tc.name: GetExtensionByName
 * @tc.desc: GetExtensionByName Test When AbilityRuntime::Extension is nullptr.
 */
HWTEST_F(AbilityLoaderTest, GetExtensionByName_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityLoaderTest GetExtensionByName_0200 start";
    std::string abilityName = "AbilityRuntime";
    AbilityLoader::GetInstance().extensions_.clear();
    EXPECT_FALSE(AbilityLoader::GetInstance().GetExtensionByName(abilityName) != nullptr);
    GTEST_LOG_(INFO) << "AbilityLoaderTest GetExtensionByName_0200 end";
}

/**
 * @tc.number: RegisterUIAbility_0100
 * @tc.name: RegisterUIAbility
 * @tc.desc: RegisterUIAbility Test, return true.
 */
HWTEST_F(AbilityLoaderTest, RegisterUIAbility_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityLoaderTest RegisterUIAbility_0100 start";
    auto loader = std::make_shared<OHOS::AppExecFwk::AbilityLoader>();
    std::string abilityName;
    CreateUIAblity createFunc;
    loader->RegisterUIAbility(abilityName, createFunc);
    bool result = false;
    auto it = loader->uiAbilities_.find(abilityName);
    if (it != loader->uiAbilities_.end()) {
        result = true;
    }
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "AbilityLoaderTest RegisterUIAbility_0100 end";
}

/**
 * @tc.number: GetUIAbilityByName_0100
 * @tc.name: GetUIAbilityByName
 * @tc.desc: GetUIAbilityByName Test When UIAbility is not nullptr.
 */
HWTEST_F(AbilityLoaderTest, GetUIAbilityByName_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityLoaderTest GetUIAbilityByName_0100 start";
    std::string abilityName = "UIAbility";
    CreateAblity createFunc;
    auto createAblity = []() -> AbilityRuntime::UIAbility *{
        AbilityRuntime::UIAbility *callBack = new (std::nothrow) AbilityRuntime::UIAbility;
        return callBack;
    };
    AbilityLoader::GetInstance().uiAbilities_.clear();
    AbilityLoader::GetInstance().RegisterUIAbility(abilityName, createAblity);
    EXPECT_TRUE(AbilityLoader::GetInstance().GetUIAbilityByName(abilityName) != nullptr);
    GTEST_LOG_(INFO) << "AbilityLoaderTest GetUIAbilityByName_0100 end";
}

/**
 * @tc.number: GetUIAbilityByName_0200
 * @tc.name: GetUIAbilityByName
 * @tc.desc: GetUIAbilityByName Test When UIAbility is nullptr.
 */
HWTEST_F(AbilityLoaderTest, GetUIAbilityByName_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityLoaderTest GetAbilityByName_0200 start";
    std::string abilityName = "UIAbilityName";
    AbilityLoader::GetInstance().abilities_.clear();
    EXPECT_FALSE(AbilityLoader::GetInstance().GetUIAbilityByName(abilityName) != nullptr);
    GTEST_LOG_(INFO) << "AbilityLoaderTest GetUIAbilityByName_0200 start";
}