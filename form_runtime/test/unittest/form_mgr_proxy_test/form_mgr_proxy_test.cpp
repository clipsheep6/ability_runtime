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
#include "form_mgr_proxy.h"
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
class FormMgrProxyTest : public testing::Test {
public:
    void SetUp() override
    {
        mockFormMgrService = new MockFormMgrService();
        formMgrProxy = new FormMgrProxy(mockFormMgrService);
    }
    // TearDown() is unnecessary.
    sptr<MockFormMgrService> mockFormMgrService;
    sptr<FormMgrProxy> formMgrProxy;
};
/**
 * @tc.name: FormMgrProxyTest_0001
 * @tc.desc: Verify GetFormsInfo
 * @tc.type: FUNC
 * @tc.require: #I59O23
 */
HWTEST_F(FormMgrProxyTest, FormMgrProxyTest_0001, TestSize.Level1) {
    GTEST_LOG_(INFO) << "FormMgrProxyTest_001 starts";
    // initialize input parameters.
    std::string moduleName = "";
    std::vector<FormInfo> formInfos;
    // setup expectations.
    std::vector<FormInfo> expectFormInfos;
    FormInfo formInfo = {};
    formInfo.bundleName = "ohos.samples.FormApplication";
    expectFormInfos.push_back(formInfo);
    EXPECT_CALL(*mockFormMgrService, GetFormsInfo(_, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(expectFormInfos), Return(ERR_OK)));
    // test.
    formMgrProxy->GetFormsInfo(moduleName, formInfos);
    // expect result.
    EXPECT_THAT(formInfos, ContainerEq(expectFormInfos));
    GTEST_LOG_(INFO) << "FormMgrProxyTest_0001 test ends";
}

/**
 * @tc.name: FormMgrProxyTest_0002
 * @tc.desc: Verify GetFormsInfo
 * @tc.type: FUNC
 * @tc.require: #I59O23
 */
HWTEST_F(FormMgrProxyTest, FormMgrProxyTest_0002, TestSize.Level1) {
    GTEST_LOG_(INFO) << "FormMgrProxyTest_0002 starts";
    // initialize input parameters.
    std::string moduleName = "empty";
    std::vector<FormInfo> formInfos;
    // setup expectations.
    std::vector<FormInfo> expectFormInfos;
    FormInfo formInfo = {};
    formInfo.bundleName = "ohos.samples.FormApplication";
    formInfo.moduleName = "entry";
    expectFormInfos.push_back(formInfo);
    EXPECT_CALL(*mockFormMgrService, GetFormsInfo(_, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));
    // test.
    formMgrProxy->GetFormsInfo(moduleName, formInfos);
    // expect result.
    EXPECT_EQ(formInfos.size(), 0);
    GTEST_LOG_(INFO) << "FormMgrProxyTest_0002 test ends";
}
}