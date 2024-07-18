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

#include "cj_want_ffi.h"
#include <gtest/gtest.h>

#include <iostream>
#include <cstring>
#include <string>
#include <vector>

#include "cj_utils_ffi.h"
#include "want.h"
#include "want_params_wrapper.h"
#include "securec.h"

using namespace testing;
using namespace testing::ext;
using OHOS::AAFwk::Want;
using OHOS::AppExecFwk::ElementName;

class CjWantFfiTest : public testing::Test {
public:
    CjWantFfiTest()
    {}
    ~CjWantFfiTest()
    {}
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void CjWantFfiTest::SetUpTestCase()
{}

void CjWantFfiTest::TearDownTestCase()
{}

void CjWantFfiTest::SetUp()
{}

void CjWantFfiTest::TearDown()
{}

/**
 * @tc.name: CjWantFfiTestFFICJWantCreateWithWantInfo_0100
 * @tc.desc: CjWantFfiTest test for FFICJWantCreateWithWantInfo.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantFfiTest, CjWantFfiTestFFICJWantCreateWithWantInfo_0100, TestSize.Level1)
{
    const char* deviceId = "deviceId";
    const char* bundleName = "bundleName";
    const char* abilityName = "abilityName";
    const char* moduleName = "moduleName";
    ElementNameHandle elementNameHandle = new ElementName(deviceId, bundleName, abilityName, moduleName);

    CJWantParams params;
    params.elementName = elementNameHandle;
    params.flags = 123;

    params.uri = new char[9];
    strcpy_s(params.uri, 9, "deviceId");

    params.action = new char[11];
    strcpy_s(params.action, 11, "bundleName");

    params.wantType = new char[12];
    strcpy_s(params.wantType, 12, "abilityName");

    params.parameters = new char[11];
    strcpy_s(params.parameters, 11, "moduleName");

    WantHandle want = FFICJWantCreateWithWantInfo(params);
    EXPECT_NE(want, nullptr);
}

/**
 * @tc.name: CjWantFfiTestFFICJWantDelete_0100
 * @tc.desc: CjWantFfiTest test for FFICJWantDelete.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantFfiTest, CjWantFfiTestFFICJWantDelete_0100, TestSize.Level1)
{
    const char* deviceId = "deviceId";
    const char* bundleName = "bundleName";
    const char* abilityName = "abilityName";
    const char* moduleName = "moduleName";
    ElementNameHandle elementNameHandle = new ElementName(deviceId, bundleName, abilityName, moduleName);

    CJWantParams params;
    params.elementName = elementNameHandle;
    params.flags = 123;

    params.uri = new char[9];
    strcpy_s(params.uri, 9, "deviceId");

    params.action = new char[11];
    strcpy_s(params.action, 11, "bundleName");

    params.wantType = new char[12];
    strcpy_s(params.wantType, 12, "abilityName");

    params.parameters = new char[11];
    strcpy_s(params.parameters, 11, "moduleName");
    WantHandle want = FFICJWantCreateWithWantInfo(params);
    FFICJWantDelete(want);
}

/**
 * @tc.name: CjWantFfiTestFFICJWantGetWantInfo_0100
 * @tc.desc: CjWantFfiTest test for OnCreate.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantFfiTest, CjWantFfiTestFFICJWantGetWantInfo_0100, TestSize.Level1)
{
    const char* deviceId = "deviceId";
    const char* bundleName = "bundleName";
    const char* abilityName = "abilityName";
    const char* moduleName = "moduleName";
    ElementNameHandle elementNameHandle = new ElementName(deviceId, bundleName, abilityName, moduleName);

    CJWantParams params;
    params.elementName = elementNameHandle;
    params.flags = 123;

    params.uri = new char[9];
    strcpy_s(params.uri, 9, "deviceId");

    params.action = new char[11];
    strcpy_s(params.action, 11, "bundleName");

    params.wantType = new char[12];
    strcpy_s(params.wantType, 12, "abilityName");

    params.parameters = new char[11];
    strcpy_s(params.parameters, 11, "moduleName");
    WantHandle want = FFICJWantCreateWithWantInfo(params);

    CJWantParams* paramsResult = FFICJWantGetWantInfo(want);
    EXPECT_NE(paramsResult, nullptr);
    FFICJWantParamsDelete(paramsResult);
    FFICJWantDelete(reinterpret_cast<WantHandle>(want));
}

/**
 * @tc.name: CjWantFfiTestFFICJWantAddEntity_0100
 * @tc.desc: CjWantFfiTest test for FFICJWantAddEntity.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantFfiTest, CjWantFfiTestFFICJWantAddEntity_0100, TestSize.Level1)
{
    const char* deviceId = "deviceId";
    const char* bundleName = "bundleName";
    const char* abilityName = "abilityName";
    const char* moduleName = "moduleName";
    ElementNameHandle elementNameHandle = new ElementName(deviceId, bundleName, abilityName, moduleName);

    CJWantParams params;
    params.elementName = elementNameHandle;
    params.flags = 123;

    params.uri = new char[9];
    strcpy_s(params.uri, 9, "deviceId");

    params.action = new char[11];
    strcpy_s(params.action, 11, "bundleName");

    params.wantType = new char[12];
    strcpy_s(params.wantType, 12, "abilityName");

    params.parameters = new char[11];
    strcpy_s(params.parameters, 11, "moduleName");
    WantHandle want = FFICJWantCreateWithWantInfo(params);

    const char* entity = "test_entity";
    FFICJWantAddEntity(want, entity);
}

/**
 * @tc.name: CjWantFfiTestFFICJWantParseUri_0100
 * @tc.desc: CjWantFfiTest test for FFICJWantParseUri.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantFfiTest, CjWantFfiTestFFICJWantParseUri_0100, TestSize.Level1)
{
    const char* uri = "test_uri";
    WantHandle want = FFICJWantParseUri(uri);
}