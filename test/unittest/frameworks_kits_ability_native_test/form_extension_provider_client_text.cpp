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

#include "accesstoken_kit.h"
#include "appexecfwk_errors.h"
#include "form_constants.h"
#include "form_mgr_errors.h"
#include "mock_form_supply_callback.h"

#define private public
#include "form_runtime/form_extension_provider_client.h"
#undef private

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Security;

const std::string FORM_MANAGER_SERVICE_BUNDLE_NAME = "com.form.fms.app";
const std::string FORM_SUPPLY_INFO = "com.form.supply.info.test";

class FormExtensionProviderClientTest : public testing::Test {
public:
    FormExtensionProviderClientTest()
    {}
    ~FormExtensionProviderClientTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FormExtensionProviderClientTest::SetUpTestCase(void)
{}

void FormExtensionProviderClientTest::TearDownTestCase(void)
{}

void FormExtensionProviderClientTest::SetUp(void)
{}

void FormExtensionProviderClientTest::TearDown(void)
{}

/**
 * @tc.number: formExtensionProviderClient_0100
 * @tc.name: NotifyFormExtensionDelete
 * @tc.desc: Test NotifyFormExtensionDelete function with hostToken is nullptr.
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0100 start";

    Want want;
    // don't set Constants::PARAM_FORM_HOST_TOKEN then hostToken is nullptr.
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 103)
        .SetParam(Constants::FORM_CONNECT_ID, 103L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 103L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    int64_t formId = 723L;
    const sptr<IRemoteObject> callerToken = nullptr;
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    formExtensionProviderClient.NotifyFormExtensionDelete(formId, want, callerToken);

    GTEST_LOG_(INFO) << "formExtensionProviderClient_0100 end";
}

/**
 * @tc.number: formExtensionProviderClient_0200
 * @tc.name: NotifyFormExtensionDelete
 * @tc.desc: Test NotifyFormExtensionDelete function with hostToken is not nullptr.
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0200 start";

    // callerToken
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    Want want;
    // set Constants::PARAM_FORM_HOST_TOKEN hostToken pointer can be obtained.
    want.SetParam(Constants::PARAM_FORM_HOST_TOKEN, callerToken);

    int64_t formId = 723L;
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    formExtensionProviderClient.NotifyFormExtensionDelete(formId, want, callerToken);

    GTEST_LOG_(INFO) << "formExtensionProviderClient_0200 end";
}

/**
 * @tc.number: formExtensionProviderClient_0300
 * @tc.name: NotifyFormExtensionUpdate
 * @tc.desc: Test NotifyFormExtensionUpdate function with HasParameter(Constants::FORM_CONNECT_ID) is true .
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0300 start";

    Want want;
    // set Constants::FORM_CONNECT_ID
    want.SetParam(Constants::FORM_CONNECT_ID, 103L);

    int64_t formId = 723L;
    const sptr<IRemoteObject> callerToken = nullptr;
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    formExtensionProviderClient.NotifyFormExtensionUpdate(formId, want, callerToken);

    GTEST_LOG_(INFO) << "formExtensionProviderClient_0300 end";
}

/**
 * @tc.number: formExtensionProviderClient_0400
 * @tc.name: NotifyFormExtensionUpdate
 * @tc.desc: Test NotifyFormExtensionUpdate function with HasParameter(Constants::FORM_CONNECT_ID) is false .
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0400 start";

    Want want;
    // don't set Constants::FORM_CONNECT_ID
    want.SetParam(Constants::ACQUIRE_TYPE, 103L);

    int64_t formId = 723L;
    const sptr<IRemoteObject> callerToken = nullptr;
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    formExtensionProviderClient.NotifyFormExtensionUpdate(formId, want, callerToken);

    GTEST_LOG_(INFO) << "formExtensionProviderClient_0400 end";
}

/**
 * @tc.number: formExtensionProviderClient_0500
 * @tc.name: FireFormExtensionEvent
 * @tc.desc: Test FireFormExtensionEvent function with HasParameter(Constants::FORM_CONNECT_ID) is false .
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0500 start";

    Want want;
    // don't set Constants::FORM_CONNECT_ID
    want.SetParam(Constants::ACQUIRE_TYPE, 103L);

    int64_t formId = 723L;
    std::string message = "event message";
    const sptr<IRemoteObject> callerToken = nullptr;
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    formExtensionProviderClient.FireFormExtensionEvent(formId, message, want, callerToken);

    GTEST_LOG_(INFO) << "formExtensionProviderClient_0500 end";
}

/**
 * @tc.number: formExtensionProviderClient_0600
 * @tc.name: FireFormExtensionEvent
 * @tc.desc: Test FireFormExtensionEvent function with HasParameter(Constants::FORM_CONNECT_ID) is true .
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0600 start";

    Want want;
    //set Constants::FORM_CONNECT_ID
    want.SetParam(Constants::FORM_CONNECT_ID, 103L);

    int64_t formId = 723L;
    std::string message = "event message";
    const sptr<IRemoteObject> callerToken = nullptr;
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    formExtensionProviderClient.FireFormExtensionEvent(formId, message, want, callerToken);

    GTEST_LOG_(INFO) << "formExtensionProviderClient_0600 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
