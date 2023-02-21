/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "uri_permission_manager_stub_impl.h"
#undef private
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class UriPermissionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UriPermissionTest::SetUpTestCase() {}

void UriPermissionTest::TearDownTestCase() {}

void UriPermissionTest::SetUp() {}

void UriPermissionTest::TearDown() {}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionTest, Upms_GrantUriPermission_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    Uri uri(uriStr);
    unsigned int flag = 1;
    uint32_t fromTokenId = 2;
    uint32_t targetTokenId = 3;
    upms->GrantUriPermission(uri, flag, fromTokenId, targetTokenId);
}

/*
 * Feature: URIPermissionManagerService
 * Function: ConnectBundleManager
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService ConnectBundleManager
 */
HWTEST_F(UriPermissionTest, Upms_ConnectBundleManager_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    (void)upms->ConnectBundleManager();
}

/*
 * Feature: URIPermissionManagerService
 * Function: ConnectStorageManager
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService ConnectStorageManager
 */
HWTEST_F(UriPermissionTest, Upms_ConnectStorageManager_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    (void)upms->ConnectStorageManager();
}

/*
 * Feature: URIPermissionManagerService
 * Function: RemoveUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService RemoveUriPermission
 */
HWTEST_F(UriPermissionTest, Upms_RemoveUriPermission_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    unsigned int tmpFlag = 1;
    uint32_t fromTokenId = 2;
    uint32_t targetTokenId = 3;
    GrantInfo info = { tmpFlag, fromTokenId, targetTokenId };
    std::list<GrantInfo> infoList = { info };
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    upms->uriMap_.emplace(uriStr, infoList);
    upms->RemoveUriPermission(targetTokenId);
}

/*
 * Feature: URIPermissionManagerService
 * Function: ClearBMSProxy
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService ClearBMSProxy
 */
HWTEST_F(UriPermissionTest, Upms_ClearBMSProxy_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    upms->ClearBMSProxy();
}

/*
 * Feature: URIPermissionManagerService
 * Function: ClearSMProxy
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService ClearSMProxy
 */
HWTEST_F(UriPermissionTest, Upms_ClearBMSProxy_002, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    upms->ClearSMProxy();
}

/*
 * Feature: URIPermissionManagerService
 * Function: OnRemoteDied
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService OnRemoteDied
 */
HWTEST_F(UriPermissionTest, Upms_OnRemoteDied_001, TestSize.Level1)
{
    const auto& onClearProxyCallback = [](const wptr<IRemoteObject>& remote) {};
    sptr<UriPermissionManagerStubImpl::BMSOrSMDeathRecipient> object =
        new UriPermissionManagerStubImpl::BMSOrSMDeathRecipient(onClearProxyCallback);
    object->OnRemoteDied(nullptr);
}

/*
 * Feature: URIPermissionManagerService
 * Function: OnRemoteDied
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService OnRemoteDied
 */
HWTEST_F(UriPermissionTest, Upms_OnRemoteDied_002, TestSize.Level1)
{
    sptr<UriPermissionManagerStubImpl::BMSOrSMDeathRecipient> object =
        new UriPermissionManagerStubImpl::BMSOrSMDeathRecipient(nullptr);
    object->OnRemoteDied(nullptr);
}
}  // namespace AAFwk
}  // namespace OHOS
