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

#include "extension_running_info.h"
#include "hilog_wrapper.h"
#include "iremote_object.h"
#include "want.h"

using namespace testing::ext;
using OHOS::AppExecFwk::ElementName;

namespace OHOS {
namespace AAFwk {
class AbilityExtensionRunningInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityExtensionRunningInfoTest::SetUpTestCase(void)
{}

void AbilityExtensionRunningInfoTest::TearDownTestCase(void)
{}

void AbilityExtensionRunningInfoTest::SetUp()
{}

void AbilityExtensionRunningInfoTest::TearDown()
{}

/**
 * @tc.name: ReadFromParcel_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionRunningInfoTest, ReadFromParcel_0100, TestSize.Level1)
{
    HILOG_INFO("ReadFromParcel start");

    Parcel parcel;
    ExtensionRunningInfo extensionRunningInfo;
    bool ret = extensionRunningInfo.ReadFromParcel(parcel);
    EXPECT_FALSE(ret);

    HILOG_INFO("ReadFromParcel end");
}

/**
 * @tc.name: ReadFromParcel_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionRunningInfoTest, ReadFromParcel_0200, TestSize.Level1)
{
    HILOG_INFO("ReadFromParcel start");

    std::string deviceId;
    std::string bundleName = "ohos.test.bundle";
    std::string abilityName = "TestAbility";
    std::string moduleName = "entry";
    ElementName elementName;
    elementName.SetDeviceID(deviceId);
    elementName.SetBundleName(bundleName);
    elementName.SetAbilityName(abilityName);
    elementName.SetModuleName(moduleName);
    Parcel parcel;
    parcel.WriteParcelable(&elementName);

    ExtensionRunningInfo extensionRunningInfo;
    bool ret = extensionRunningInfo.ReadFromParcel(parcel);
    EXPECT_TRUE(ret);

    HILOG_INFO("ReadFromParcel end");
}

/**
 * @tc.name: Unmarshalling_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionRunningInfoTest, Unmarshalling_0100, TestSize.Level1)
{
    HILOG_INFO("Unmarshalling start");

    std::string deviceId;
    std::string bundleName = "ohos.test.bundle";
    std::string abilityName = "TestAbility";
    std::string moduleName = "entry";
    ElementName elementName;
    elementName.SetDeviceID(deviceId);
    elementName.SetBundleName(bundleName);
    elementName.SetAbilityName(abilityName);
    elementName.SetModuleName(moduleName);
    Parcel parcel;
    parcel.WriteParcelable(&elementName);

    ExtensionRunningInfo *extensionRunningInfo = ExtensionRunningInfo::Unmarshalling(parcel);
    EXPECT_NE(extensionRunningInfo, nullptr);

    HILOG_INFO("Unmarshalling end");
}

/**
 * @tc.name: Unmarshalling_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionRunningInfoTest, Unmarshalling_0200, TestSize.Level1)
{
    HILOG_INFO("Unmarshalling start");

    Parcel parcel;
    ExtensionRunningInfo *extensionRunningInfo = ExtensionRunningInfo::Unmarshalling(parcel);
    EXPECT_EQ(extensionRunningInfo, nullptr);

    HILOG_INFO("Unmarshalling end");
}

/**
 * @tc.name: Marshalling_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionRunningInfoTest, Marshalling_0100, TestSize.Level1)
{
    HILOG_INFO("Marshalling start");

    Parcel parcel;
    ExtensionRunningInfo extensionRunningInfo;
    bool ret = extensionRunningInfo.Marshalling(parcel);
    EXPECT_TRUE(ret);

    HILOG_INFO("Marshalling end");
}

/**
 * @tc.name: Marshalling_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(AbilityExtensionRunningInfoTest, Marshalling_0200, TestSize.Level1)
{
    HILOG_INFO("Marshalling start");

    std::string deviceId;
    std::string bundleName = "ohos.test.bundle";
    std::string abilityName = "TestAbility";
    std::string moduleName = "entry";
    ElementName elementName;
    elementName.SetDeviceID(deviceId);
    elementName.SetBundleName(bundleName);
    elementName.SetAbilityName(abilityName);
    elementName.SetModuleName(moduleName);
    Parcel parcel;

    ExtensionRunningInfo extensionRunningInfo;
    extensionRunningInfo.ReadFromParcel(parcel);
    extensionRunningInfo.clientPackage.push_back("client1");
    extensionRunningInfo.clientPackage.push_back("client2");
    extensionRunningInfo.clientPackage.push_back("client3");

    bool ret = extensionRunningInfo.Marshalling(parcel);
    EXPECT_TRUE(ret);

    HILOG_INFO("Marshalling end");
}
} // namespace AAFwk
} // namespace OHOS