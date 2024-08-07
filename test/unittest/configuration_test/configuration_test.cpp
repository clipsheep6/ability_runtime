/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "ability_manager_service.h"
#include "configuration.h"
#include "configuration_convertor.h"
#include "configuration_utils.h"
#include "hilog_tag_wrapper.h"
#ifdef SUPPORT_GRAPHICS
#include "locale_config.h"
#include "window_scene.h"
#endif
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {
class ConfigurationTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    void InitResourceManager(std::shared_ptr<Global::Resource::ResourceManager> resourceManager);
};

void ConfigurationTest::SetUpTestCase(void)
{}

void ConfigurationTest::TearDownTestCase(void)
{}

void ConfigurationTest::SetUp(void)
{}

void ConfigurationTest::TearDown(void)
{}

void ConfigurationTest::InitResourceManager(std::shared_ptr<Global::Resource::ResourceManager> resourceManager)
{
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "res config is invalid.");
        return;
    }

#ifdef SUPPORT_GRAPHICS
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag("zh", status);
    TAG_LOGI(AAFwkTag::TEST, "language: %{public}s, script: %{public}s, region: %{public}s",
             locale.getLanguage(), locale.getScript(), locale.getCountry());
    resConfig->SetLocaleInfo(locale);
#endif
    Global::Resource::RState updateRet = resourceManager->UpdateResConfig(*resConfig);
    if (updateRet != Global::Resource::RState::SUCCESS) {
        TAG_LOGE(AAFwkTag::TEST, "Init locale failed.");
    }
}

/*
 * Feature: Configuration
 * Function: AddItem
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Update Configuration
 */
HWTEST_F(ConfigurationTest, AddItem_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    config.RemoveItem(AAFwk::GlobalConfigurationKey::APP_FONT_SIZE_SCALE);
    int displayId = 1001;
    std::string val{ "中文" };
    EXPECT_EQ(0, config.GetItemSize());
    config.AddItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);
    EXPECT_EQ(1, config.GetItemSize());

    // replace
    config.AddItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);
    EXPECT_EQ(1, config.GetItemSize());
}

/*
 * Feature: Configuration
 * Function: AddItem
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Update Configuration
 */
HWTEST_F(ConfigurationTest, AddItem_002, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    config.RemoveItem(AAFwk::GlobalConfigurationKey::APP_FONT_SIZE_SCALE);
    std::string val{ "中文" };
    EXPECT_EQ(0, config.GetItemSize());
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);
    EXPECT_EQ(1, config.GetItemSize());

    // replace
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);
    EXPECT_EQ(1, config.GetItemSize());
}

/*
 * Feature: Configuration
 * Function: AddItem
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Update Configuration
 */
HWTEST_F(ConfigurationTest, AddItem_003, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    config.RemoveItem(AAFwk::GlobalConfigurationKey::APP_FONT_SIZE_SCALE);
    std::string val{ "中文" };
    std::string key{ "test_key" };
    EXPECT_EQ(0, config.GetItemSize());
    auto ref = config.AddItem(key, val);
    EXPECT_FALSE(ref);
}

/*
 * Feature: Configuration
 * Function: GetItem
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Process Configuration Change
 */
HWTEST_F(ConfigurationTest, GetItem_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    config.RemoveItem(AAFwk::GlobalConfigurationKey::APP_FONT_SIZE_SCALE);
    int displayId = 1001;
    std::string val{ "中文" };
    config.AddItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);

    auto item = config.GetItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, val);

    // replace
    std::string english{ "英文" };
    config.AddItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, english);
    item = config.GetItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, english);

    int displayId2 = 1002;
    std::string non{ "" };
    item = config.GetItem(displayId2, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, non);
}

/*
 * Feature: Configuration
 * Function: GetItem
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Process Configuration Change
 */
HWTEST_F(ConfigurationTest, GetItem_002, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    config.RemoveItem(AAFwk::GlobalConfigurationKey::APP_FONT_SIZE_SCALE);
    std::string val{ "中文" };
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);

    auto item = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, val);

    // replace
    std::string english{ "英文" };
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, english);
    item = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, english);

    int displayId2 = 1002;
    std::string non{ "" };
    item = config.GetItem(displayId2, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, non);
}

/*
 * Feature: Configuration
 * Function: GetItem
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Process Configuration Change
 */
HWTEST_F(ConfigurationTest, GetItem_003, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    config.RemoveItem(AAFwk::GlobalConfigurationKey::APP_FONT_SIZE_SCALE);
    std::string non{ "" };
    auto item = config.GetItem("test_kay");
    EXPECT_EQ(item, non);
}
/*
 * Feature: Configuration
 * Function: RemoveItem
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Process Configuration Change
 */
HWTEST_F(ConfigurationTest, RemoveItem_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    config.RemoveItem(AAFwk::GlobalConfigurationKey::APP_FONT_SIZE_SCALE);
    int displayId = 1001;
    std::string val{ "中文" };
    config.AddItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);

    auto item = config.GetItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, val);

    // remove it
    bool canRemove = config.RemoveItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_TRUE(canRemove);

    std::string non{ "" };
    item = config.GetItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, non);

    canRemove = config.RemoveItem(displayId, non);
    EXPECT_FALSE(canRemove);

    int displayId2 = 1002;
    canRemove = config.RemoveItem(displayId2, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_FALSE(canRemove);
}

/*
 * Feature: Configuration
 * Function: RemoveItem
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Process Configuration Change
 */
HWTEST_F(ConfigurationTest, RemoveItem_002, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    config.RemoveItem(AAFwk::GlobalConfigurationKey::APP_FONT_SIZE_SCALE);
    std::string val{ "中文" };
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);

    auto item = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, val);

    // remove it
    bool canRemove = config.RemoveItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_TRUE(canRemove);

    std::string non{ "" };
    item = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, non);

    canRemove = config.RemoveItem(non);
    EXPECT_FALSE(canRemove);

    canRemove = config.RemoveItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_FALSE(canRemove);
}

/*
 * Feature: Configuration
 * Function: GetItemSize
 * SubFunction: Process Configuration Change Inner
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Process Configuration Change Inner
 */
HWTEST_F(ConfigurationTest, GetItemSize_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    config.RemoveItem(AAFwk::GlobalConfigurationKey::APP_FONT_SIZE_SCALE);
    int displayId = 1001;
    std::string val{ "中文" };
    EXPECT_EQ(0, config.GetItemSize());
    config.AddItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);
    EXPECT_EQ(1, config.GetItemSize());

    int displayId2 = 1002;
    config.AddItem(displayId2, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);
    EXPECT_EQ(2, config.GetItemSize());
}

/*
 * Feature: Configuration
 * Function: CompareDifferent
 * SubFunction: Process Configuration Change Inner
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Check according to key
 */
HWTEST_F(ConfigurationTest, CompareDifferent_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string val{ "中文" };
    config.AddItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);


    AppExecFwk::Configuration config2;
    int displayId2 = 1001;
    std::string English{ "英文" };
    config2.AddItem(displayId2, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, English);

    std::vector<std::string> changeKeyV;

    config.CompareDifferent(changeKeyV, config2);
    int size = changeKeyV.size();
    EXPECT_EQ(size, 1);
}

/*
 * Feature: Configuration
 * Function: CompareDifferent
 * SubFunction: Process Configuration Change Inner
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Check according to key
 */
HWTEST_F(ConfigurationTest, CompareDifferent_002, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string val{ "中文" };
    config.AddItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);


    AppExecFwk::Configuration config2;
    int displayId2 = 1002;
    std::string English{ "英文" };
    config2.AddItem(displayId2, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, English);

    std::vector<std::string> changeKeyV;

    config.CompareDifferent(changeKeyV, config2);
    int size = changeKeyV.size();
    EXPECT_EQ(size, 1);
}

/*
 * Feature: Configuration
 * Function: Merge
 * SubFunction: Process Configuration Change Inner
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Check according to key
 */
HWTEST_F(ConfigurationTest, Merge_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string val{ "中文" };
    config.AddItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, val);


    AppExecFwk::Configuration config2;
    int displayId2 = 1002;
    std::string English{ "英文" };
    config2.AddItem(displayId2, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, English);

    std::vector<std::string> changeKeyV;

    config.CompareDifferent(changeKeyV, config2);
    int size = changeKeyV.size();
    EXPECT_EQ(size, 1);

    config.Merge(changeKeyV, config2);

    auto item = config.GetItem(displayId2, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, English);
}

/*
 * Feature: Configuration
 * Function: Merge
 * SubFunction: Process Configuration Change Inner
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Check according to key
 */
HWTEST_F(ConfigurationTest, Merge_002, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string chinese{ "Chinese " };
    config.AddItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, chinese);


    AppExecFwk::Configuration config2;
    std::string english{ "English" };
    config2.AddItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, english);

    std::vector<std::string> changeKeyV;

    config.CompareDifferent(changeKeyV, config2);
    int size = changeKeyV.size();
    EXPECT_EQ(size, 1);

    config.Merge(changeKeyV, config2);

    auto item = config.GetItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_EQ(item, english);
}
/*
 * Feature: Configuration
 * Function: GetName
 * SubFunction: Process Configuration Change Inner
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Check according to key
 */
HWTEST_F(ConfigurationTest, GetName_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string chinese{ "Chinese " };
    config.AddItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, chinese);

    int displayId2 = 1002;
    std::string english{ "English" };
    config.AddItem(displayId2, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, english);

    auto name = config.GetName();
    EXPECT_TRUE(name != "");
    GTEST_LOG_(INFO) << "get name pair [" << name << "]";
}

/*
 * Feature: Configuration
 * Function: Assignment structure
 * SubFunction: Process Configuration Change Inner
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Check according to key
 */
HWTEST_F(ConfigurationTest, structure_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    int displayId = 1001;
    std::string chinese{ "Chinese " };
    config.AddItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, chinese);

    AppExecFwk::Configuration configOther(config);
    auto item = configOther.GetItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_TRUE(item == chinese);

    AppExecFwk::Configuration configThird;
    configThird = configOther;

    auto item2 = configThird.GetItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_TRUE(item2 == chinese);

    AppExecFwk::Configuration configFourth = configThird;
    auto item3 = configFourth.GetItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_TRUE(item3 == chinese);
}

/**
 * @tc.name: UpdateConfigToResMgr_001
 * @tc.desc: Update configuration to resource manager.
 * @tc.type: FUNC
 * @tc.require: issueI6CWJD
 */
HWTEST_F(ConfigurationTest, UpdateConfigToResMgr_001, TestSize.Level1)
{
    // init configuration, "en", "dark", "false"
    AppExecFwk::Configuration configuration;
    bool addItemRet;
    addItemRet = configuration.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en");
    EXPECT_EQ(addItemRet, true);
    addItemRet = configuration.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "dark");
    EXPECT_EQ(addItemRet, true);
    addItemRet = configuration.AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, "false");
    EXPECT_EQ(addItemRet, true);

    // init resource manager, "zh", "light", "true"
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    ASSERT_NE(resourceManager, nullptr);
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    ASSERT_NE(resConfig, nullptr);

#ifdef SUPPORT_GRAPHICS
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag("zh", status);
    EXPECT_EQ(status, U_ZERO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "language: %{public}s, script: %{public}s, region: %{public}s",
             locale.getLanguage(), locale.getScript(), locale.getCountry());
    resConfig->SetLocaleInfo(locale);
#endif
    resConfig->SetColorMode(ConvertColorMode("light"));
    resConfig->SetInputDevice(ConvertHasPointerDevice("true"));
    Global::Resource::RState updateRet = resourceManager->UpdateResConfig(*resConfig);
    EXPECT_EQ(updateRet, Global::Resource::RState::SUCCESS);

    // update configuration to resource manager
    auto configUtils = std::make_shared<AbilityRuntime::ConfigurationUtils>();
    ASSERT_NE(configUtils, nullptr);
    configUtils->UpdateGlobalConfig(configuration, nullptr);
    configUtils->UpdateGlobalConfig(configuration, resourceManager);

    // check resource manager has updated to "en", "dark", "false"
    std::unique_ptr<Global::Resource::ResConfig> updatedResConfig(Global::Resource::CreateResConfig());
    ASSERT_NE(updatedResConfig, nullptr);
    resourceManager->GetResConfig(*updatedResConfig);
#ifdef SUPPORT_GRAPHICS
    const icu::Locale *localeInfo = updatedResConfig->GetLocaleInfo();
    ASSERT_NE(localeInfo, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "language: %{public}s, script: %{public}s, region: %{public}s",
             localeInfo->getLanguage(), localeInfo->getScript(), localeInfo->getCountry());
    EXPECT_EQ(strcmp(localeInfo->getLanguage(), "en"), 0);
#endif
    EXPECT_EQ(updatedResConfig->GetColorMode(), ConvertColorMode("dark"));
    EXPECT_EQ(updatedResConfig->GetInputDevice(), ConvertHasPointerDevice("false"));
}

/**
 * @tc.name: InitDisplayConfig_0100
 * @tc.desc: Init display config.
 * @tc.type: FUNC
 * @tc.require: issueI7HPHB
 */
HWTEST_F(ConfigurationTest, InitDisplayConfig_0100, TestSize.Level1)
{
    auto configUtils = std::make_shared<AbilityRuntime::ConfigurationUtils>();
    ASSERT_NE(configUtils, nullptr);

    int displayId = Rosen::WindowScene::DEFAULT_DISPLAY_ID;
    float originDensity;
    std::string originDirection;
    auto ret = configUtils->GetDisplayConfig(displayId, originDensity, originDirection);
    EXPECT_EQ(ret, true);

    configUtils->InitDisplayConfig(displayId, nullptr, nullptr);

    AppExecFwk::Configuration originConfig;
    auto configuration = std::make_shared<Configuration>(originConfig);
    ASSERT_NE(configuration, nullptr);
    configUtils->InitDisplayConfig(displayId, configuration, nullptr);

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    ASSERT_NE(resourceManager, nullptr);
    InitResourceManager(resourceManager);
    configUtils->InitDisplayConfig(displayId, configuration, resourceManager);

    // check configurtion
    std::string displayIdStr = configuration->GetItem(ConfigurationInner::APPLICATION_DISPLAYID);
    EXPECT_EQ(displayIdStr, std::to_string(displayId));
    std::string densityStr = configuration->GetItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI);
    EXPECT_EQ(densityStr, GetDensityStr(originDensity));
    std::string directionStr = configuration->GetItem(displayId, ConfigurationInner::APPLICATION_DIRECTION);
    EXPECT_EQ(directionStr, originDirection);

    // check resourcemanager
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    resourceManager->GetResConfig(*resConfig);
    EXPECT_EQ(originDensity, resConfig->GetScreenDensity());
    EXPECT_EQ(ConvertDirection(originDirection), resConfig->GetDirection());
}

/**
 * @tc.name: UpdateDisplayConfig_0100
 * @tc.desc: Update display config with changed config.
 * @tc.type: FUNC
 * @tc.require: issueI7HPHB
 */
HWTEST_F(ConfigurationTest, UpdateDisplayConfig_0100, TestSize.Level1)
{
    auto configUtils = std::make_shared<AbilityRuntime::ConfigurationUtils>();
    ASSERT_NE(configUtils, nullptr);

    int displayId = Rosen::WindowScene::DEFAULT_DISPLAY_ID;
    float originDensity;
    std::string originDirection;
    auto ret = configUtils->GetDisplayConfig(displayId, originDensity, originDirection);
    EXPECT_EQ(ret, true);

    bool configChanged;
    configUtils->UpdateDisplayConfig(displayId, nullptr, nullptr, configChanged);

    AppExecFwk::Configuration originConfig;
    auto configuration = std::make_shared<Configuration>(originConfig);
    ASSERT_NE(configuration, nullptr);
    configUtils->UpdateDisplayConfig(displayId, configuration, nullptr, configChanged);

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    ASSERT_NE(resourceManager, nullptr);
    InitResourceManager(resourceManager);
    configUtils->UpdateDisplayConfig(displayId, configuration, resourceManager, configChanged);
    EXPECT_EQ(configChanged, true);

    // check configurtion
    std::string densityStr = configuration->GetItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI);
    EXPECT_EQ(densityStr, GetDensityStr(originDensity));
    std::string directionStr = configuration->GetItem(displayId, ConfigurationInner::APPLICATION_DIRECTION);
    EXPECT_EQ(directionStr, originDirection);

    // check resourcemanager
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    resourceManager->GetResConfig(*resConfig);
    EXPECT_EQ(originDensity, resConfig->GetScreenDensity());
    EXPECT_EQ(ConvertDirection(originDirection), resConfig->GetDirection());
}

/**
 * @tc.name: UpdateDisplayConfig_0200
 * @tc.desc: Update display config with unchanged config.
 * @tc.type: FUNC
 * @tc.require: issueI7HPHB
 */
HWTEST_F(ConfigurationTest, UpdateDisplayConfig_0200, TestSize.Level1)
{
    auto configUtils = std::make_shared<AbilityRuntime::ConfigurationUtils>();
    ASSERT_NE(configUtils, nullptr);

    int displayId = Rosen::WindowScene::DEFAULT_DISPLAY_ID;
    float originDensity;
    std::string originDirection;
    auto ret = configUtils->GetDisplayConfig(displayId, originDensity, originDirection);
    EXPECT_EQ(ret, true);

    bool configChanged;
    configUtils->UpdateDisplayConfig(displayId, nullptr, nullptr, configChanged);

    AppExecFwk::Configuration originConfig;
    auto configuration = std::make_shared<Configuration>(originConfig);
    ASSERT_NE(configuration, nullptr);
    configUtils->UpdateDisplayConfig(displayId, configuration, nullptr, configChanged);

    // Add configuration.
    configuration->AddItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI, GetDensityStr(originDensity));
    configuration->AddItem(displayId, ConfigurationInner::APPLICATION_DIRECTION, originDirection);
    configuration->AddItem(ConfigurationInner::APPLICATION_DISPLAYID, std::to_string(displayId));

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    ASSERT_NE(resourceManager, nullptr);
    configUtils->UpdateDisplayConfig(displayId, configuration, resourceManager, configChanged);
    EXPECT_EQ(configChanged, false);
}
} // namespace AAFwk
} // namespace OHOS
