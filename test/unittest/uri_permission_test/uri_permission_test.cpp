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

#include "event_report.h"
#include "istorage_manager.h"
#include "storage_manager_proxy.h"
#include "system_ability_definition.h"
#include "tokenid_kit.h"
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
    EXPECT_NE(upms, nullptr);
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    Uri uri(uriStr);
    unsigned int flag = 1;
    std::string targetBundleName = "name2";
    upms->GrantUriPermission(uri, flag, targetBundleName);
}

/*
 * Feature: URIPermissionManagerService
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService ConnectManager
 */
HWTEST_F(UriPermissionTest, Upms_ConnectManager_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    sptr<StorageManager::IStorageManager> storageManager = nullptr;
    upms->ConnectManager(storageManager, STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(storageManager, nullptr);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService RevokeUriPermission
 */
HWTEST_F(UriPermissionTest, Upms_RevokeUriPermission_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    EXPECT_NE(upms, nullptr);
    unsigned int tmpFlag = 1;
    uint32_t fromTokenId = 2;
    uint32_t targetTokenId = 3;
    int autoremove = 1;
    GrantInfo info = { tmpFlag, fromTokenId, targetTokenId, autoremove };
    std::list<GrantInfo> infoList = { info };
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    upms->uriMap_.emplace(uriStr, infoList);
    upms->RevokeUriPermission(targetTokenId);
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
    sptr<UriPermissionManagerStubImpl::ProxyDeathRecipient> object =
        new UriPermissionManagerStubImpl::ProxyDeathRecipient(onClearProxyCallback);
    EXPECT_NE(object, nullptr);
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
    sptr<UriPermissionManagerStubImpl::ProxyDeathRecipient> object =
        new UriPermissionManagerStubImpl::ProxyDeathRecipient(nullptr);
    EXPECT_NE(object, nullptr);
    object->OnRemoteDied(nullptr);
}
}  // namespace AAFwk
}  // namespace OHOS
