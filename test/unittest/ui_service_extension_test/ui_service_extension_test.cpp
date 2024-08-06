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
#define protected public
#include "ui_service_extension.h"
#include "ui_service_extension_module_loader.h"
#include "ui_service_extension_context.h"
#undef private
#undef protected

#include "mock_ability_token.h"
#include "ability_handler.h"
#include "ohos_application.h"
#include "runtime.h"

using namespace testing::ext;
using namespace OHOS::Rosen;

namespace OHOS {
namespace AbilityRuntime {
class UIServiceExtensionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UIServiceExtensionTest::SetUpTestCase(void)
{}

void UIServiceExtensionTest::TearDownTestCase(void)
{}

void UIServiceExtensionTest::SetUp()
{}

void UIServiceExtensionTest::TearDown()
{}

/**
 * @tc.number: Create_0100
 * @tc.name: UIServiceExtension Create
 * @tc.desc: UIServiceExtension Create.
 */
HWTEST_F(UIServiceExtensionTest, Create_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Create_0100 start");

    std::unique_ptr<Runtime> runtime{nullptr};
    UIServiceExtension::Create(runtime);

    TAG_LOGI(AAFwkTag::TEST, "Create_0100 end");
}

/**
 * @tc.number: CreateAndInitContext_0100
 * @tc.name: UIServiceExtension CreateAndInitContext
 * @tc.desc: UIServiceExtension CreateAndInitContext.
 */
HWTEST_F(UIServiceExtensionTest, CreateAndInitContext_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateAndInitContext_0100 start");

    Runtime::Options options;
    auto runtime = Runtime::Create(options);
    auto uIServiceExtensionPtr = UIServiceExtension::Create(runtime);

    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    info->name = "UIServiceExtensionTest";
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();

    uIServiceExtensionPtr->CreateAndInitContext(record, application, handler, token);


    TAG_LOGI(AAFwkTag::TEST, "CreateAndInitContext_0100 end");
}

/**
 * @tc.number: Init_0100
 * @tc.name: UIServiceExtension Init
 * @tc.desc: UIServiceExtension Init.
 */
HWTEST_F(UIServiceExtensionTest, Init_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Init_0100 start");

    Runtime::Options options;
    auto runtime = Runtime::Create(options);
    auto uIServiceExtensionPtr = UIServiceExtension::Create(runtime);

    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    info->name = "UIServiceExtensionTest";
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();

    uIServiceExtensionPtr->Init(record, application, handler, token);

    TAG_LOGI(AAFwkTag::TEST, "Init_0100 end");
}

/**
 * @tc.number: StartAbility_0100
 * @tc.name: UIServiceExtension StartAbility
 * @tc.desc: UIServiceExtension StartAbility.
 */
HWTEST_F(UIServiceExtensionTest, StartAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0100 start");

    AAFwk::Want want;
    AAFwk::StartOptions startOptions;

    UIServiceExtensionContext uiServiceExtensionContext;
    uiServiceExtensionContext.StartAbility(want, startOptions);

    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0100 end");
}

/**
 * @tc.number: TerminateSelf_0100
 * @tc.name: UIServiceExtension TerminateSelf
 * @tc.desc: UIServiceExtension TerminateSelf.
 */
HWTEST_F(UIServiceExtensionTest, TerminateSelf_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_0100 start");

    UIServiceExtensionContext uiServiceExtensionContext;
    uiServiceExtensionContext.TerminateSelf();

    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_0100 end");
}

/**
 * @tc.number: GetWindow_0100
 * @tc.name: UIServiceExtension GetWindow
 * @tc.desc: UIServiceExtension GetWindow.
 */
HWTEST_F(UIServiceExtensionTest, GetWindow_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetWindow_0100 start");

    UIServiceExtensionContext uiServiceExtensionContext;
    uiServiceExtensionContext.GetWindow();

    TAG_LOGI(AAFwkTag::TEST, "GetWindow_0100 end");
}

/**
 * @tc.number: GetUIContent_0100
 * @tc.name: UIServiceExtension GetUIContent
 * @tc.desc: UIServiceExtension GetUIContent.
 */
HWTEST_F(UIServiceExtensionTest, GetUIContent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIContent_0100 start");

    UIServiceExtensionContext uiServiceExtensionContext;
    uiServiceExtensionContext.GetUIContent();

    TAG_LOGI(AAFwkTag::TEST, "GetUIContent_0100 end");
}

/**
 * @tc.number: StartAbilityByType_0100
 * @tc.name: UIServiceExtension StartAbilityByType
 * @tc.desc: UIServiceExtension StartAbilityByType.
 */
HWTEST_F(UIServiceExtensionTest, StartAbilityByType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByType_0100 start");

    std::string type;
    AAFwk::WantParams wantParam;
    std::shared_ptr<JsUIExtensionCallback> uiExtensionCallbacks;

    UIServiceExtensionContext uiServiceExtensionContext;
    uiServiceExtensionContext.StartAbilityByType(type, wantParam, uiExtensionCallbacks);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByType_0100 end");
}

/**
 * @tc.number: GetWindowOption_0100
 * @tc.name: UIServiceExtension GetWindowOption
 * @tc.desc: UIServiceExtension GetWindowOption.
 */
HWTEST_F(UIServiceExtensionTest, GetWindowOption_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetWindowOption_0100 start");

    AAFwk::Want want;
    std::shared_ptr<Rosen::ExtensionWindowConfig> extensionWindowConfig
        = std::make_shared<Rosen::ExtensionWindowConfig>();
    int32_t hostWindowId{0};

    Runtime::Options options;
    auto runtime = Runtime::Create(options);
    auto uIServiceExtensionPtr = UIServiceExtension::Create(runtime);
    uIServiceExtensionPtr->GetWindowOption(want, extensionWindowConfig, hostWindowId);

    TAG_LOGI(AAFwkTag::TEST, "GetWindowOption_0100 end");
}

/**
 * @tc.number: SetWindow_0100
 * @tc.name: UIServiceExtension SetWindow
 * @tc.desc: UIServiceExtension SetWindow.
 */
HWTEST_F(UIServiceExtensionTest, SetWindow_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetWindow_0100 start");

    sptr<Rosen::Window> window = new Rosen::Window();

    UIServiceExtensionContext uiServiceExtensionContext;
    uiServiceExtensionContext.SetWindow(window);

    TAG_LOGI(AAFwkTag::TEST, "SetWindow_0100 end");
}


} // namespace AbilityRuntime
} // namespace OHOS
