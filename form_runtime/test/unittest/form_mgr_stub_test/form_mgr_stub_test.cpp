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
#include <memory>
#define private public
#include "form_mgr_stub.h"
#undef private
#include "gmock/gmock.h"
#include "mock_form_mgr_service.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::ContainerEq;
using ::testing::_;

// overload operator for ContainerEq
namespace OHOS::AppExecFwk {
    bool operator==(const FormInfo& lhs, const FormInfo& rhs)
    {
        if (lhs.bundleName != rhs.bundleName) {
            return false;
        }
        if (lhs.moduleName != rhs.moduleName) {
            return false;
        }
        if (lhs.name != rhs.name) {
            return false;
        }
        // to be continued...
        return true;
    }
}

namespace {
class FormMgrStubTest : public testing::Test {
public:
    void SetUp() override
    {
        mockFormMgrService = new (std::nothrow) MockFormMgrService();
    }
    sptr<MockFormMgrService> mockFormMgrService;
};

void GetParcelableInfos(MessageParcel &reply, std::vector<FormInfo> &parcelableInfos)
    {
    int32_t infoSize = reply.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<FormInfo> info(reply.ReadParcelable<FormInfo>());
        parcelableInfos.emplace_back(*info);
    }
}

/**
 * @tc.name: FormMgrStubTest_0001
 * @tc.desc: Verify HandleGetFormsInfo
 * @tc.type: FUNC
 * @tc.require: #I59O23
 */
HWTEST_F(FormMgrStubTest, FormMgrStubTest_0001, TestSize.Level1) {
    GTEST_LOG_(INFO) << "FormMgrTest_0001 starts";
    // initialize input parameters.
    MessageParcel data;
    std::string moduleName = "";
    data.WriteString(moduleName);
    MessageParcel reply;
    // set up return vector.
    std::vector<FormInfo> returnInfos;
    // set up 2 infos from different modules.
    FormInfo info1 = {};
    info1.bundleName = "bundle1";
    info1.moduleName = "module1";
    info1.name = "widget1";
    FormInfo info2 = {};
    info2.bundleName = "bundle1";
    info2.moduleName = "module2";
    info2.name = "widget2";
    returnInfos.push_back(info1);
    returnInfos.push_back(info2);
    // Set Expectations
    EXPECT_CALL(*mockFormMgrService, GetFormsInfo(_, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(returnInfos), Return(ERR_OK)));
    // test.
    mockFormMgrService->HandleGetFormsInfo(data, reply);
    // check errorcode
    EXPECT_EQ(ERR_OK, reply.ReadInt32());
    // check resulting infos.
    std::vector<FormInfo> resultInfos;
    GetParcelableInfos(reply, resultInfos);
    EXPECT_THAT(resultInfos, ContainerEq(returnInfos));
    GTEST_LOG_(INFO) << "FormMgrStubTest_0001 ends";
}

/**
 * @tc.name: FormMgrStubTest_0002
 * @tc.desc: Verify HandleGetFormsInfo
 * @tc.type: FUNC
 * @tc.require: #I59O23
 */
HWTEST_F(FormMgrStubTest, FormMgrStubTest_0002, TestSize.Level1) {
    GTEST_LOG_(INFO) << "FormMgrStubTest_0002 starts";
    // initialize input parameters.
    MessageParcel data;
    std::string moduleName = "";
    data.WriteString(moduleName);
    MessageParcel reply;
    EXPECT_CALL(*mockFormMgrService, GetFormsInfo(_, _))
        .Times(1)
        .WillOnce(Return(-1));
    // test.
    mockFormMgrService->HandleGetFormsInfo(data, reply);
    // check errorcode
    EXPECT_NE(ERR_OK, reply.ReadInt32());
    // check resulting infos.
    std::vector<FormInfo> resultInfos;
    GetParcelableInfos(reply, resultInfos);
    EXPECT_EQ(0, resultInfos.size());
    GTEST_LOG_(INFO) << "FormMgrStubTest_0002 ends";
}
}