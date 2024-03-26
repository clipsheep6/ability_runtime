/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include <vector>

#include "mock_ipc_skeleton.h"
#include "mock_native_token.h"
#include "mock_bundle_mgr_helper.h"
#include "system_ability_manager_client_mock.h"

#include "ability_manager_errors.h"
#include "event_report.h"
#include "system_ability_definition.h"
#include "tokenid_kit.h"
#define private public
#include "uri_permission_manager_stub_impl.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class UriPermissionImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UriPermissionImplTest::SetUpTestCase()
{
    AppExecFwk::MockNativeToken::SetNativeToken();
}

void UriPermissionImplTest::TearDownTestCase() {}

void UriPermissionImplTest::SetUp() {}

void UriPermissionImplTest::TearDown() {}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    Uri uri(uriStr);
    unsigned int flag = 0;
    std::string targetBundleName = "name2";
    upms->GrantUriPermission(uri, flag, targetBundleName);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_002, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    Uri uri(uriStr);
    unsigned int flag = 1;
    std::string targetBundleName = "name2";
    upms->GrantUriPermission(uri, flag, targetBundleName);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_003, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    Uri uri(uriStr);
    unsigned int flag = 2;
    MockSystemAbilityManager::isNullptr = false;
    std::string targetBundleName = "name2";
    upms->GrantUriPermission(uri, flag, targetBundleName);
    MockSystemAbilityManager::isNullptr = true;
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_004, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    Uri uri(uriStr);
    unsigned int flag = 2;
    std::string targetBundleName = "name2";
    MockSystemAbilityManager::isNullptr = false;
    StorageManager::StorageManagerServiceMock::isZero = false;
    upms->GrantUriPermission(uri, flag, targetBundleName);
    MockSystemAbilityManager::isNullptr = true;
    StorageManager::StorageManagerServiceMock::isZero = true;
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_005, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    unsigned int tmpFlag = 1;
    uint32_t fromTokenId = 2;
    uint32_t targetTokenId = 3;
    std::string targetBundleName = "name2";
    int autoremove = 1;
    GrantInfo info = { tmpFlag, fromTokenId, targetTokenId, autoremove };
    std::list<GrantInfo> infoList = { info };
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    upms->uriMap_.emplace(uriStr, infoList);
    Uri uri(uriStr);
    MockSystemAbilityManager::isNullptr = false;
    upms->GrantUriPermission(uri, tmpFlag, targetBundleName);
    MockSystemAbilityManager::isNullptr = true;
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_006, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    unsigned int tmpFlag = 1;
    uint32_t fromTokenId = 2;
    uint32_t targetTokenId = 3;
    std::string targetBundleName = "name2";
    int autoremove = 1;
    GrantInfo info = { tmpFlag, fromTokenId, targetTokenId, autoremove };
    std::list<GrantInfo> infoList = { info };
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    upms->uriMap_.emplace(uriStr, infoList);
    Uri uri(uriStr);
    MockSystemAbilityManager::isNullptr = false;
    unsigned int flag = 2;
    upms->GrantUriPermission(uri, flag, targetBundleName);
    MockSystemAbilityManager::isNullptr = true;
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_007, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    unsigned int tmpFlag = 1;
    uint32_t fromTokenId = 2;
    uint32_t targetTokenId = 3;
    std::string targetBundleName = "name2";
    int autoremove = 1;
    GrantInfo info = { tmpFlag, fromTokenId, targetTokenId, autoremove };
    std::list<GrantInfo> infoList = { info };
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    upms->uriMap_.emplace(uriStr, infoList);
    Uri uri(uriStr);
    MockSystemAbilityManager::isNullptr = false;
    unsigned int flag = 2;
    upms->GrantUriPermission(uri, flag, targetBundleName);
    MockSystemAbilityManager::isNullptr = true;
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService RevokeUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermission_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    unsigned int tmpFlag = 1;
    uint32_t fromTokenId = 2;
    uint32_t targetTokenId = 3;
    GrantInfo info = { tmpFlag, fromTokenId, targetTokenId };
    std::list<GrantInfo> infoList = { info };
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    upms->uriMap_.emplace(uriStr, infoList);
    upms->RevokeUriPermission(targetTokenId);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService RevokeUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermission_002, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    unsigned int tmpFlag = 1;
    uint32_t fromTokenId = 2;
    uint32_t targetTokenId = 3;
    GrantInfo info = { tmpFlag, fromTokenId, targetTokenId };
    std::list<GrantInfo> infoList = { info };
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    upms->uriMap_.emplace(uriStr, infoList);
    uint32_t tokenId = 4;
    upms->RevokeUriPermission(tokenId);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GetTokenIdByBundleName
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GetTokenIdByBundleName
 */
HWTEST_F(UriPermissionImplTest, Upms_GetTokenIdByBundleName_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    std::string bundleName = "com.example.app1001";
    int appIndex = 0;
    uint32_t tokenId = 0;
    auto ret = upms->GetTokenIdByBundleName(bundleName, appIndex, tokenId);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermissionManually_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    upms->storageManager_ = new StorageManager::StorageManagerServiceMock();

    std::string uriStr = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_A.txt";
    std::string targetBundleName = "com.example.app1002";
    uint32_t fromTokenId = 1001;
    uint32_t targetTokenId = 1002;
    uint32_t flag = 1;

    auto errorCode = upms->AddTempUriPermission(uriStr, flag, fromTokenId, targetTokenId, 0);
    EXPECT_EQ(errorCode, ERR_OK);

    auto verifyRet = upms->VerifyUriPermission(Uri(uriStr), flag, targetTokenId);
    EXPECT_EQ(verifyRet, true);

    IPCID::tokenId = fromTokenId;
    std::vector<Uri> uriVec = { Uri(uriStr) };
    errorCode = upms->DeleteTempUriPermission(uriVec, fromTokenId, targetTokenId);
    EXPECT_EQ(errorCode, ERR_OK);
    verifyRet = upms->VerifyUriPermission(Uri(uriStr), flag, targetTokenId);
    EXPECT_EQ(verifyRet, false);
    IPCID::tokenId = 0;
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermissionManually_002, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    upms->storageManager_ = new StorageManager::StorageManagerServiceMock();

    std::string uriStr = "unknow://com.example.app1001/data/storage/el2/base/haps/entry/files/test_A.txt";
    std::string targetBundleName = "com.example.app1002";
    auto errorCode = upms->RevokeUriPermissionManually(Uri(uriStr), targetBundleName);
    EXPECT_NE(errorCode, ERR_OK);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService RevokeUriPermissionManually for batch uris
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermissionManually_003, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    upms->storageManager_ = new StorageManager::StorageManagerServiceMock();

    std::string uriStrA = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_A.txt";
    std::string uriStrB = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_B.txt";
    std::vector<Uri> uriVec = { Uri(uriStrA), Uri(uriStrB) };
    std::string targetBundleName = "com.example.app1002";
    uint32_t fromTokenId = 1001;
    uint32_t targetTokenId = 1002;
    uint32_t flag = 1;

    upms->AddTempUriPermission(uriStrA, flag, fromTokenId, targetTokenId, 0);
    upms->AddTempUriPermission(uriStrB, flag, fromTokenId, targetTokenId, 0);
    
    IPCID::tokenId = fromTokenId;
    auto ret = upms->DeleteTempUriPermission(uriVec, fromTokenId, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    
    auto verifyRet = upms->VerifyUriPermission(Uri(uriStrA), flag, targetTokenId);
    EXPECT_EQ(verifyRet, false);
    verifyRet = upms->VerifyUriPermission(Uri(uriStrB), flag, targetTokenId);
    EXPECT_EQ(verifyRet, false);

    IPCID::tokenId = 0;
}

/*
 * Feature: URIPermissionManagerService
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService ConnectManager
 */
HWTEST_F(UriPermissionImplTest, Upms_ConnectManager_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    SystemAbilityManagerClient::nullptrFlag = true;
    sptr<StorageManager::IStorageManager> storageManager = nullptr;
    upms->ConnectManager(storageManager, STORAGE_MANAGER_MANAGER_ID);
    SystemAbilityManagerClient::nullptrFlag = false;
    ASSERT_EQ(storageManager, nullptr);
}

/*
 * Feature: URIPermissionManagerService
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService ConnectManager
 */
HWTEST_F(UriPermissionImplTest, Upms_ConnectManager_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MockSystemAbilityManager::isNullptr = true;
    sptr<StorageManager::IStorageManager> storageManager = nullptr;
    upms->ConnectManager(storageManager, STORAGE_MANAGER_MANAGER_ID);
    MockSystemAbilityManager::isNullptr = false;
    ASSERT_EQ(storageManager, nullptr);
}

/*
 * Feature: URIPermissionManagerService
 * Function: VerifyUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService VerifyUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_VerifyUriPermission_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto callerTokenId = 1;
    auto targetTokenId = 2;
    auto invalidTokenId = 3;
    std::string uri = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    auto flagRead = 1;
    auto flagWrite = 2;
    upms->AddTempUriPermission(uri, flagRead, callerTokenId, targetTokenId, false);
    auto ret = upms->VerifyUriPermission(Uri(uri), flagRead, targetTokenId);
    EXPECT_EQ(ret, true);
    ret = upms->VerifyUriPermission(Uri(uri), flagWrite, targetTokenId);
    EXPECT_EQ(ret, false);
    ret = upms->VerifyUriPermission(Uri(uri), flagRead, invalidTokenId);
    EXPECT_EQ(ret, false);
}

/*
 * Feature: URIPermissionManagerService
 * Function: SendEvent
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService SendEvent
 */
HWTEST_F(UriPermissionImplTest, Upms_SendEvent_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    std::string uri = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    std::string targetBundleName = "com.example.test";
    auto ret = upms->SendEvent(1, 2, uri);
    ASSERT_EQ(ret, false);
}
}  // namespace AAFwk
}  // namespace OHOS
