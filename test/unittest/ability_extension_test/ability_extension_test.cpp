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
#include "extension.h"
#undef private
#undef protected

#include "ability_handler.h"
#include "ability_transaction_callback_info.h"
#include "configuration.h"
#include "hilog_wrapper.h"
#include "iremote_object.h"
#include "mock_ability_token.h"
#include "ohos_application.h"
#include "want.h"

using namespace testing::ext;
using OHOS::AppExecFwk::ElementName;

namespace OHOS {
namespace AbilityRuntime {
class AbilityExtensionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityExtensionTest::SetUpTestCase(void)
{}

void AbilityExtensionTest::TearDownTestCase(void)
{}

void AbilityExtensionTest::SetUp()
{}

void AbilityExtensionTest::TearDown()
{}

/**
 * @tc.name: SetCallingInfo_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, SetCallingInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetCallingInfo start");

    Extension extension;
    CallingInfo callingInfo;
    extension.SetCallingInfo(callingInfo);
    EXPECT_NE(extension.callingInfo_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "SetCallingInfo end");
}

/**
 * @tc.name: GetCallingInfo_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, GetCallingInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetCallingInfo start");

    Extension extension;
    CallingInfo callingInfo;
    extension.SetCallingInfo(callingInfo);
    auto result = extension.GetCallingInfo();
    EXPECT_NE(result, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "GetCallingInfo end");
}

/**
 * @tc.name: SetLastRequestWant_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, SetLastRequestWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetLastRequestWant start");

    Extension extension;
    std::string deviceId;
    std::string bundleName = "ohos.test.bundle";
    std::string abilityName = "TestAbility";
    std::string moduleName = "entry";
    ElementName elementName;
    elementName.SetDeviceID(deviceId);
    elementName.SetBundleName(bundleName);
    elementName.SetAbilityName(abilityName);
    elementName.SetModuleName(moduleName);
    Want want;
    want.SetElement(elementName);
    extension.SetLastRequestWant(want);
    EXPECT_TRUE(true);

    TAG_LOGI(AAFwkTag::TEST, "SetLastRequestWant end");
}

/**
 * @tc.name: SetLaunchWant_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, SetLaunchWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetLaunchWant start");

    std::string deviceId;
    std::string bundleName = "ohos.test.bundle";
    std::string abilityName = "TestAbility";
    std::string moduleName = "entry";
    ElementName elementName;
    elementName.SetDeviceID(deviceId);
    elementName.SetBundleName(bundleName);
    elementName.SetAbilityName(abilityName);
    elementName.SetModuleName(moduleName);
    Want want;
    want.SetElement(elementName);

    Extension extension;
    extension.SetLaunchWant(want);
    EXPECT_TRUE(true);

    TAG_LOGI(AAFwkTag::TEST, "SetLaunchWant end");
}

/**
 * @tc.name: Init_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, Init_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Init start");

    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record = nullptr;
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = nullptr;

    Extension extension;
    extension.Init(record, application, handler, token);
    EXPECT_TRUE(true);

    TAG_LOGI(AAFwkTag::TEST, "Init end");
}

/**
 * @tc.name: Init_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, Init_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Init start");

    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = nullptr;
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = nullptr;

    Extension extension;
    extension.Init(record, application, handler, token);
    EXPECT_TRUE(true);

    TAG_LOGI(AAFwkTag::TEST, "Init end");
}

/**
 * @tc.name: Init_0300
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, Init_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Init start");

    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;

    Extension extension;
    extension.Init(record, application, handler, token);
    EXPECT_TRUE(true);

    TAG_LOGI(AAFwkTag::TEST, "Init end");
}

/**
 * @tc.name: Init_0400
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, Init_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Init start");

    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = nullptr;

    Extension extension;
    extension.Init(record, application, handler, token);
    EXPECT_TRUE(true);

    TAG_LOGI(AAFwkTag::TEST, "Init end");
}

/**
 * @tc.name: Init_0500
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, Init_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Init start");

    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();

    Extension extension;
    extension.Init(record, application, handler, token);
    EXPECT_TRUE(true);

    TAG_LOGI(AAFwkTag::TEST, "Init end");
}

/**
 * @tc.name: OnStart_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, OnStart_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStart start");

    Extension extension;
    // Init extension
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    extension.Init(record, application, handler, token);

    std::string deviceId = "0";
    std::string bundleName = "ohos.test.bundle";
    std::string abilityName = "TestAbilityOnConnect_0100";
    std::string moduleName = "OnStart_0100";
    ElementName elementName;
    elementName.SetDeviceID(deviceId);
    elementName.SetBundleName(bundleName);
    elementName.SetAbilityName(abilityName);
    elementName.SetModuleName(moduleName);
    Want want;
    want.SetElement(elementName);

    // Test
    extension.OnStart(want);
    EXPECT_TRUE(true);

    TAG_LOGI(AAFwkTag::TEST, "OnStart end");
}

/**
 * @tc.name: OnStop_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, OnStop_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStop start");

    Extension extension;
    // Init extension
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    extension.Init(record, application, handler, token);

    // Test
    extension.OnStop();
    EXPECT_TRUE(true);

    TAG_LOGI(AAFwkTag::TEST, "OnStop end");
}

/**
 * @tc.name: OnConnect_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, OnConnect_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnConnect start");

    Extension extension;
    // Init extension
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    extension.Init(record, application, handler, token);

    std::string deviceId;
    std::string bundleName = "ohos.test.bundle";
    std::string abilityName = "TestAbilityOnConnect_0100";
    std::string moduleName = "OnConnect_0100";
    ElementName elementName;
    elementName.SetDeviceID(deviceId);
    elementName.SetBundleName(bundleName);
    elementName.SetAbilityName(abilityName);
    elementName.SetModuleName(moduleName);
    Want want;
    want.SetElement(elementName);

    // Test
    extension.OnConnect(want);
    EXPECT_TRUE(true);

    TAG_LOGI(AAFwkTag::TEST, "OnConnect end");
}

/**
 * @tc.name: OnDisconnect_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, OnDisconnect_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnDisconnect start");

    Extension extension;
    // Init extension
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    extension.Init(record, application, handler, token);

    std::string deviceId;
    std::string bundleName = "ohos.test.bundle";
    std::string abilityName = "TestAbility";
    std::string moduleName = "entry";
    ElementName elementName;
    elementName.SetDeviceID(deviceId);
    elementName.SetBundleName(bundleName);
    elementName.SetAbilityName(abilityName);
    elementName.SetModuleName(moduleName);
    Want want;
    want.SetElement(elementName);

    // Test
    extension.OnDisconnect(want);
    EXPECT_TRUE(true);

    TAG_LOGI(AAFwkTag::TEST, "OnDisconnect end");
}

/**
 * @tc.name: OnDisconnect_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, OnDisconnect_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnDisconnect start");

    Extension extension;
    // Init extension
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    extension.Init(record, application, handler, token);

    std::string deviceId;
    std::string bundleName = "ohos.test.bundle";
    std::string abilityName = "TestAbility";
    std::string moduleName = "entry";
    ElementName elementName;
    elementName.SetDeviceID(deviceId);
    elementName.SetBundleName(bundleName);
    elementName.SetAbilityName(abilityName);
    elementName.SetModuleName(moduleName);
    Want want;
    want.SetElement(elementName);
    bool isAsyncCallback = true;
    AppExecFwk::AbilityTransactionCallbackInfo callbackInfo;

    // Test
    extension.OnDisconnect(want, &callbackInfo, isAsyncCallback);
    EXPECT_EQ(isAsyncCallback, false);

    TAG_LOGI(AAFwkTag::TEST, "OnDisconnect end");
}

/**
 * @tc.name: OnCommand_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, OnCommand_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCommand start");

    std::string deviceId;
    std::string bundleName = "ohos.test.bundle";
    std::string abilityName = "TestAbility";
    std::string moduleName = "entry";
    ElementName elementName;
    elementName.SetDeviceID(deviceId);
    elementName.SetBundleName(bundleName);
    elementName.SetAbilityName(abilityName);
    elementName.SetModuleName(moduleName);
    Want want;
    want.SetElement(elementName);
    bool restart = true;
    int startId = 0;

    Extension extension;
    extension.OnCommand(want, restart, startId);
    EXPECT_TRUE(true);

    TAG_LOGI(AAFwkTag::TEST, "OnCommand end");
}

/**
 * @tc.name: OnCommand_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, OnCommand_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCommand start");

    std::string deviceId;
    std::string bundleName = "ohos.test.bundle";
    std::string abilityName = "TestAbility";
    std::string moduleName = "entry";
    ElementName elementName;
    elementName.SetDeviceID(deviceId);
    elementName.SetBundleName(bundleName);
    elementName.SetAbilityName(abilityName);
    elementName.SetModuleName(moduleName);
    Want want;
    want.SetElement(elementName);
    bool restart = false;
    int startId = 0;

    Extension extension;
    extension.OnCommand(want, restart, startId);
    EXPECT_TRUE(true);

    TAG_LOGI(AAFwkTag::TEST, "OnCommand end");
}

/**
 * @tc.name: OnConfigurationUpdated_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, OnConfigurationUpdated_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnConfigurationUpdated start");

    AppExecFwk::Configuration configuration;

    Extension extension;
    extension.OnConfigurationUpdated(configuration);
    EXPECT_TRUE(true);

    TAG_LOGI(AAFwkTag::TEST, "OnConfigurationUpdated end");
}

/**
 * @tc.name: OnMemoryLevel_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, OnMemoryLevel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnMemoryLevel start");

    Extension extension;
    int level = 0;
    extension.OnMemoryLevel(level);
    EXPECT_TRUE(true);

    TAG_LOGI(AAFwkTag::TEST, "OnMemoryLevel end");
}

/**
 * @tc.name: Dump_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionTest, Dump_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Dump start");

    std::vector<std::string> params;
    params.push_back("params1");
    params.push_back("params2");
    params.push_back("params3");
    std::vector<std::string> info;
    info.push_back("info1");
    info.push_back("info2");
    info.push_back("info3");

    Extension extension;
    extension.Dump(params, info);
    EXPECT_TRUE(true);

    TAG_LOGI(AAFwkTag::TEST, "Dump end");
}

} // namespace AbilityRuntime
} // namespace OHOS