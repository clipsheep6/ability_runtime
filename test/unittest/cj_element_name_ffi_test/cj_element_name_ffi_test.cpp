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

#include "cj_element_name_ffi.h"
#include "cj_utils_ffi.h"
#include "element_name.h"
#include "securec.h"

using namespace testing;
using namespace testing::ext;
using OHOS::AppExecFwk::ElementName;

class CjElementNameFfiTest : public testing::Test {
public:
    CjElementNameFfiTest()
    {}
    ~CjElementNameFfiTest()
    {}
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void CjElementNameFfiTest::SetUpTestCase()
{}

void CjElementNameFfiTest::TearDownTestCase()
{}

void CjElementNameFfiTest::SetUp()
{}

void CjElementNameFfiTest::TearDown()
{}

/**
 * @tc.name: CJElementNameFFITestFFICJElementNameCreateWithContent_0100
 * @tc.desc: CjElementNameFfiTest test for FFICJElementNameCreateWithContent.
 * @tc.type: FUNC
 */
HWTEST_F(CjElementNameFfiTest, CJElementNameFFITestFFICJElementNameCreateWithContent_0100, TestSize.Level1)
{
    char* deviceId = new char[9];
    strcpy_s(deviceId, 9, "deviceId");

    char* bundleName = new char[11];
    strcpy_s(bundleName, 11, "bundleName");

    char* abilityName = new char[12];
    strcpy_s(abilityName, 12, "abilityName");

    char* moduleName = new char[11];
    strcpy_s(moduleName, 11, "moduleName");

    ElementNameHandle elementNameHandle =
        FFICJElementNameCreateWithContent(deviceId, bundleName, abilityName, moduleName);
    auto actualElementName = reinterpret_cast<ElementName*>(elementNameHandle);

    EXPECT_EQ(actualElementName->GetDeviceID(), deviceId);
    EXPECT_EQ(actualElementName->GetBundleName(), bundleName);
    EXPECT_EQ(actualElementName->GetAbilityName(), abilityName);
    EXPECT_EQ(actualElementName->GetModuleName(), moduleName);

    FFICJElementNameDelete(elementNameHandle);
}

/**
 * @tc.name: CJElementNameFFITestFFICJElementNameDelete_0100
 * @tc.desc: CjElementNameFfiTest test for FFICJElementNameDelete.
 * @tc.type: FUNC
 */
HWTEST_F(CjElementNameFfiTest, CJElementNameFFITestFFICJElementNameDelete_0100, TestSize.Level1)
{
    char* deviceId = new char[9];
    strcpy_s(deviceId, 9, "deviceId");

    char* bundleName = new char[11];
    strcpy_s(bundleName, 11, "bundleName");

    char* abilityName = new char[12];
    strcpy_s(abilityName, 12, "abilityName");

    char* moduleName = new char[11];
    strcpy_s(moduleName, 11, "moduleName");

    ElementNameHandle elementNameHandle =
        FFICJElementNameCreateWithContent(deviceId, bundleName, abilityName, moduleName);
    FFICJElementNameDelete(elementNameHandle);
}

/**
 * @tc.name: CjElementNameFfiTestContext_0100
 * @tc.desc: CjElementNameFfiTest test for FFICJElementNameGetElementNameInfo.
 * @tc.type: FUNC
 */
HWTEST_F(CjElementNameFfiTest, CJElementNameFFITestFFICJElementNameGetElementNameInfo_0100, TestSize.Level1)
{
    char* deviceId = new char[9];
    strcpy_s(deviceId, 9, "deviceId");

    char* bundleName = new char[11];
    strcpy_s(bundleName, 11, "bundleName");

    char* abilityName = new char[12];
    strcpy_s(abilityName, 12, "abilityName");

    char* moduleName = new char[11];
    strcpy_s(moduleName, 11, "moduleName");

    ElementNameHandle elementNameHandle =
        FFICJElementNameCreateWithContent(deviceId, bundleName, abilityName, moduleName);
    ElementNameParams* elementNameParams = FFICJElementNameGetElementNameInfo(elementNameHandle);

    auto actualElementName = reinterpret_cast<ElementName*>(elementNameHandle);
    EXPECT_STREQ(elementNameParams->deviceId, CreateCStringFromString(actualElementName->GetDeviceID()));
    EXPECT_STREQ(elementNameParams->bundleName, CreateCStringFromString(actualElementName->GetBundleName()));
    EXPECT_STREQ(elementNameParams->abilityName, CreateCStringFromString(actualElementName->GetAbilityName()));
    EXPECT_STREQ(elementNameParams->moduleName, CreateCStringFromString(actualElementName->GetModuleName()));

    FFICJElementNameParamsDelete(elementNameParams);
    FFICJElementNameDelete(elementNameHandle);
}

/**
 * @tc.name: CjElementNameFfiTestContext_0100
 * @tc.desc: CjElementNameFfiTest test for FFICJElementNameParamsDelete.
 * @tc.type: FUNC
 */
HWTEST_F(CjElementNameFfiTest, CJElementNameFFITestFFICJElementNameParamsDelete_0100, TestSize.Level1)
{
    char* deviceId = new char[9];
    strcpy_s(deviceId, 9, "deviceId");

    char* bundleName = new char[11];
    strcpy_s(bundleName, 11, "bundleName");

    char* abilityName = new char[12];
    strcpy_s(abilityName, 12, "abilityName");

    char* moduleName = new char[11];
    strcpy_s(moduleName, 11, "moduleName");

    ElementNameParams* elementNameParams = static_cast<ElementNameParams*>(malloc(sizeof(ElementNameParams)));
    elementNameParams->deviceId = CreateCStringFromString(deviceId);
    elementNameParams->bundleName = CreateCStringFromString(bundleName);
    elementNameParams->abilityName = CreateCStringFromString(abilityName);
    elementNameParams->moduleName = CreateCStringFromString(moduleName);

    FFICJElementNameParamsDelete(elementNameParams);
}