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
#include <gtest/hwext/gtest-multithread.h>

#define private public
#define protected public
#include "js_ui_service_extension.h"
#include "js_ui_service_extension_context.cpp"
#undef private
#undef protected

#include "mock_ability_token.h"
#include "ability_handler.h"
#include "ohos_application.h"
#include "runtime.h"

using namespace testing::ext;


namespace OHOS {
namespace AbilityRuntime {

class JsUIServiceExtensionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::unique_ptr<Runtime> runtime;
    JsUIServiceExtension* jsUIServiceExtension;
};

void JsUIServiceExtensionTest::SetUpTestCase()
{}

void JsUIServiceExtensionTest::TearDownTestCase()
{}

void JsUIServiceExtensionTest::SetUp()
{
    Runtime::Options options;
    runtime = Runtime::Create(options);
    jsUIServiceExtension = JsUIServiceExtension::Create(runtime);

    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    info->name = "JsUIServiceExtensionTest";
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();

    jsUIServiceExtension->Init(record, application, handler, token);
}

void JsUIServiceExtensionTest::TearDown()
{}

/**
 * @tc.number: OnAddSystemAbility_0100
 * @tc.name: OnAddSystemAbility
 * @tc.desc: SystemAbilityStatusChangeListener OnAddSystemAbility
 */
HWTEST_F(JsUIServiceExtensionTest, OnAddSystemAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAddSystemAbility_0100 start");

    constexpr int32_t WINDOW_MANAGER_SERVICE_ID = 4606;
    std::string deviceId{"deviceId"};
    JsUIServiceExtension::SystemAbilityStatusChangeListener systemAbilityStatusChangeListener{nullptr};
    systemAbilityStatusChangeListener.OnAddSystemAbility(WINDOW_MANAGER_SERVICE_ID, deviceId);

    TAG_LOGI(AAFwkTag::TEST, "OnAddSystemAbility_0100 end");
}

/**
 * @tc.number: OnAddSystemAbility_0100
 * @tc.name: BindContext
 * @tc.desc: JsUIServiceExtension BindContext
 */
HWTEST_F(JsUIServiceExtensionTest, BindContext_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BindContext_0100 start");

    napi_env env{nullptr};
    napi_value object{nullptr};

    jsUIServiceExtension->BindContext(env, object);

    TAG_LOGI(AAFwkTag::TEST, "BindContext_0100 end");
}

/**
 * @tc.number: OnStartAndStop_0100
 * @tc.name: OnStartAndStop
 * @tc.desc: JsUIServiceExtension OnStart and OnStop
 */
HWTEST_F(JsUIServiceExtensionTest, OnStartAndStop_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStartAndStop_0100 start");

    AAFwk::Want want;

    jsUIServiceExtension->OnStart(want);
    jsUIServiceExtension->OnStop();

    TAG_LOGI(AAFwkTag::TEST, "OnStartAndStop_0100 end");
}

/**
 * @tc.number: OnCommand_0100
 * @tc.name: OnCommand
 * @tc.desc: JsUIServiceExtension OnCommand
 */
HWTEST_F(JsUIServiceExtensionTest, OnCommand_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCommand_0100 start");

    AAFwk::Want want;
    bool restart{true};
    int startId{0};

    jsUIServiceExtension->OnCommand(want, restart, startId);

    TAG_LOGI(AAFwkTag::TEST, "OnCommand_0100 end");
}

/**
 * @tc.number: CallObjectMethod_0100
 * @tc.name: CallObjectMethod
 * @tc.desc: JsUIServiceExtension CallObjectMethod
 */
HWTEST_F(JsUIServiceExtensionTest, CallObjectMethod_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CallObjectMethod_0100 start");

    napi_value object{nullptr};
    size_t argc{0};

    jsUIServiceExtension->CallObjectMethod("Test", &object, argc);

    TAG_LOGI(AAFwkTag::TEST, "CallObjectMethod_0100 end");
}

/**
 * @tc.number: GetSrcPath_0100
 * @tc.name: GetSrcPath
 * @tc.desc: JsUIServiceExtension GetSrcPath
 */
HWTEST_F(JsUIServiceExtensionTest, GetSrcPath_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetSrcPath_0100 start");

    std::string srcPath{""};

    jsUIServiceExtension->GetSrcPath(srcPath);

    TAG_LOGI(AAFwkTag::TEST, "GetSrcPath_0100 end");
}

/**
 * @tc.number: OnConfigurationUpdatedAndConfigurationUpdated_0100
 * @tc.name: OnConfigurationUpdatedAndConfigurationUpdated
 * @tc.desc: JsUIServiceExtension OnConfigurationUpdated and ConfigurationUpdated
 */
HWTEST_F(JsUIServiceExtensionTest, OnConfigurationUpdatedAndConfigurationUpdated_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnConfigurationUpdatedAndConfigurationUpdated_0100 start");

    AppExecFwk::Configuration configuration;

    jsUIServiceExtension->OnConfigurationUpdated(configuration);
    jsUIServiceExtension->ConfigurationUpdated();

    TAG_LOGI(AAFwkTag::TEST, "OnConfigurationUpdatedAndConfigurationUpdated_0100 end");
}

/**
 * @tc.number: OnCreateAndOnChangeAndOnDestroy_0100
 * @tc.name: OnCreateAndOnChangeAndOnDestroy
 * @tc.desc: JsUIServiceExtension OnCreate And OnChange And OnDestroy
 */
HWTEST_F(JsUIServiceExtensionTest, OnCreateAndOnChangeAndOnDestroy_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCreate_0100 start");

    Rosen::DisplayId displayId{0};

    jsUIServiceExtension->OnCreate(displayId);
    jsUIServiceExtension->OnChange(displayId);
    jsUIServiceExtension->OnDestroy(displayId);

    TAG_LOGI(AAFwkTag::TEST, "OnCreate_0100 end");
}

/**
 * @tc.number: OnStart_0200
 * @tc.name: OnStart2
 * @tc.desc: JsUIServiceExtension OnStart
 */
HWTEST_F(JsUIServiceExtensionTest, OnStart_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStart_0200 start");

    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();

    jsUIServiceExtension->OnStart(want, sessionInfo);

    TAG_LOGI(AAFwkTag::TEST, "OnStart_0200 end");
}

/**
 * @tc.number: OnSceneWillCreated_0100
 * @tc.name: OnSceneWillCreated
 * @tc.desc: JsUIServiceExtension OnSceneWillCreated
 */
HWTEST_F(JsUIServiceExtensionTest, OnSceneWillCreated_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnSceneWillCreated_0100 start");

    std::shared_ptr<Rosen::ExtensionWindowConfig> extensionWindowConfig
        = std::make_shared<Rosen::ExtensionWindowConfig>();

    jsUIServiceExtension->OnSceneWillCreated(extensionWindowConfig);

    TAG_LOGI(AAFwkTag::TEST, "OnSceneWillCreated_0100 end");
}

/**
 * @tc.number: OnSceneDidCreated_0100
 * @tc.name: OnSceneDidCreated
 * @tc.desc: JsUIServiceExtension OnSceneDidCreated
 */
HWTEST_F(JsUIServiceExtensionTest, OnSceneDidCreated_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnSceneDidCreated_0100 start");

    sptr<Rosen::Window> window = new Rosen::Window();

    jsUIServiceExtension->OnSceneDidCreated(window);

    TAG_LOGI(AAFwkTag::TEST, "OnSceneDidCreated_0100 end");
}

/**
 * @tc.number: Finalizer_0100
 * @tc.name: Finalizer
 * @tc.desc: JSUIServiceExtensionContext Finalizer
 */
HWTEST_F(JsUIServiceExtensionTest, Finalizer_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Finalizer_0100 start");

    napi_env env{nullptr};

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    jsUIServiceExtensionContext.Finalizer(env, nullptr, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "Finalizer_0100 end");
}

/**
 * @tc.number: TerminateSelf_0100
 * @tc.name: TerminateSelf
 * @tc.desc: JSUIServiceExtensionContext TerminateSelf
 */
HWTEST_F(JsUIServiceExtensionTest, TerminateSelf_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_0100 start");

    napi_env env{nullptr};
    napi_callback_info info{nullptr};

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    jsUIServiceExtensionContext.TerminateSelf(env, info);

    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_0100 end");
}

/**
 * @tc.number: StartAbilityByType_0100
 * @tc.name: StartAbilityByType
 * @tc.desc: JSUIServiceExtensionContext StartAbilityByType
 */
HWTEST_F(JsUIServiceExtensionTest, StartAbilityByType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByType_0100 start");

    napi_env env{nullptr};
    napi_callback_info info{nullptr};

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    jsUIServiceExtensionContext.StartAbilityByType(env, info);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByType_0100 end");
}

/**
 * @tc.number: OnStartAbility_0100
 * @tc.name: OnStartAbility
 * @tc.desc: JSUIServiceExtensionContext OnStartAbility
 */
HWTEST_F(JsUIServiceExtensionTest, OnStartAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStartAbility_0100 start");

    napi_env env{nullptr};
    NapiCallbackInfo info{0};

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    jsUIServiceExtensionContext.OnStartAbility(env, info);

    TAG_LOGI(AAFwkTag::TEST, "OnStartAbility_0100 end");
}

/**
 * @tc.number: CheckStartAbilityInputParam_0100
 * @tc.name: CheckStartAbilityInputParam
 * @tc.desc: JSUIServiceExtensionContext CheckStartAbilityInputParam
 */
HWTEST_F(JsUIServiceExtensionTest, CheckStartAbilityInputParam_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckStartAbilityInputParam_0100 start");

    napi_env env{nullptr};
    NapiCallbackInfo info{0};
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    size_t unwrapArgc{0};

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    jsUIServiceExtensionContext.CheckStartAbilityInputParam(env, info, want, startOptions, unwrapArgc);

    TAG_LOGI(AAFwkTag::TEST, "CheckStartAbilityInputParam_0100 end");
}

/**
 * @tc.number: OnTerminateSelf_0100
 * @tc.name: OnTerminateSelf
 * @tc.desc: JSUIServiceExtensionContext OnTerminateSelf
 */
HWTEST_F(JsUIServiceExtensionTest, OnTerminateSelf_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnTerminateSelf_0100 start");

    napi_env env{nullptr};
    NapiCallbackInfo info{0};

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    jsUIServiceExtensionContext.OnTerminateSelf(env, info);

    TAG_LOGI(AAFwkTag::TEST, "OnTerminateSelf_0100 end");
}

/**
 * @tc.number: OnStartAbilityByType_0100
 * @tc.name: OnStartAbilityByType
 * @tc.desc: JSUIServiceExtensionContext OnStartAbilityByType
 */
HWTEST_F(JsUIServiceExtensionTest, OnStartAbilityByType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStartAbilityByType_0100 start");

    napi_env env{nullptr};
    NapiCallbackInfo info{0};

    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    JSUIServiceExtensionContext jsUIServiceExtensionContext(uiServiceExtensionContext);

    jsUIServiceExtensionContext.OnStartAbilityByType(env, info);

    TAG_LOGI(AAFwkTag::TEST, "OnStartAbilityByType_0100 end");
}

/**
 * @tc.number: CreateJsUIServiceExtensionContext_0100
 * @tc.name: CreateJsUIServiceExtensionContext
 * @tc.desc: CreateJsUIServiceExtensionContext
 */
HWTEST_F(JsUIServiceExtensionTest, CreateJsUIServiceExtensionContext_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateJsUIServiceExtensionContext_0100 start");

    napi_env env{nullptr};
    std::shared_ptr<UIServiceExtensionContext> uiServiceExtensionContext =
        std::make_shared<UIServiceExtensionContext>();
    CreateJsUIServiceExtensionContext(env, uiServiceExtensionContext);

    TAG_LOGI(AAFwkTag::TEST, "CreateJsUIServiceExtensionContext_0100 end");
}
} // namespace AbilityRuntime
} // namespace OHOS
