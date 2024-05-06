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

#include "mock_accesstoken_kit.h"
#include "mock_bundle_mgr_helper.h"
#include "mock_ipc_skeleton.h"
#include "mock_my_flag.h"
#include "mock_native_token.h"
#include "mock_permission_verification.h"
#include "mock_system_ability_manager_client.h"

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
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
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
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
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
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
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
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
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
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
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
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
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
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
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
 * FunctionPoints: GrantUriPermission not called by SA or SystemApp.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_008, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ &= (~MyFlag::IS_SA_CALL);
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    Uri uri(uriStr);
    uint32_t flag = 1;
    std::string targetBundleName = "name1001";
    auto ret = upms->GrantUriPermission(uri, flag, targetBundleName);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
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
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
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
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
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
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService ConnectManager
 */
HWTEST_F(UriPermissionImplTest, Upms_ConnectManager_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
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
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
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
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    auto callerTokenId = 1001;
    auto targetTokenId = 1002;
    auto invalidTokenId = 1003;
    std::string uri = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    auto flagRead = 1;
    auto flagWrite = 2;
    
    upms->AddTempUriPermission(uri, flagRead, callerTokenId, targetTokenId, false);
    auto ret = upms->VerifyUriPermission(Uri(uri), flagRead, targetTokenId);
    ASSERT_EQ(ret, true);
    ret = upms->VerifyUriPermission(Uri(uri), flagWrite, targetTokenId);
    ASSERT_EQ(ret, false);
    
    upms->AddTempUriPermission(uri, flagWrite, callerTokenId, targetTokenId, false);
    ret = upms->VerifyUriPermission(Uri(uri), flagRead, targetTokenId);
    ASSERT_EQ(ret, true);
    ret = upms->VerifyUriPermission(Uri(uri), flagWrite, targetTokenId);
    ASSERT_EQ(ret, true);

    ret = upms->VerifyUriPermission(Uri(uri), flagRead, invalidTokenId);
    ASSERT_EQ(ret, false);
}

/*
 * Feature: URIPermissionManagerService
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService SendEvent
 */
HWTEST_F(UriPermissionImplTest, Upms_SendEvent_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    std::string uri = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    std::string targetBundleName = "com.example.test";
    auto ret = upms->SendEvent(1001, 1002, uri);
    ASSERT_EQ(ret, false);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check uri permission of media\photo uri.
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    auto mediaPhotoUri = Uri("file://media/Photo/1/IMG_001/test_001.jpg");
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    uint32_t flagRead = 1;
    uint32_t flagWrite = 2;

    TokenIdPermission tokenIdPermission(callerTokenId);
    auto ret = upms->CheckUriPermission(mediaPhotoUri, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, false);
    
    // read
    MyFlag::permissionReadImageVideo_ = true;
    tokenIdPermission = TokenIdPermission(callerTokenId);
    ret = upms->CheckUriPermission(mediaPhotoUri, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, true);
    ret = upms->CheckUriPermission(mediaPhotoUri, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, false);
    MyFlag::permissionReadImageVideo_ = false;
   
    // write
    MyFlag::permissionWriteImageVideo_ = true;
    tokenIdPermission = TokenIdPermission(callerTokenId);
    ret = upms->CheckUriPermission(mediaPhotoUri, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, true);
    ret = upms->CheckUriPermission(mediaPhotoUri, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, true);
    MyFlag::permissionWriteImageVideo_ = false;

    // proxy uri permision
    MyFlag::permissionProxyAuthorization_ = true;
    tokenIdPermission = TokenIdPermission(targetTokenId);
    // no record
    ret = upms->CheckUriPermission(mediaPhotoUri, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, false);
    ret = upms->CheckUriPermission(mediaPhotoUri, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, false);

    // read
    upms->AddTempUriPermission(mediaPhotoUri.ToString(), flagRead, callerTokenId, targetTokenId, false);
    ret = upms->CheckUriPermission(mediaPhotoUri, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, true);
    ret = upms->CheckUriPermission(mediaPhotoUri, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, false);
    
    // write
    upms->AddTempUriPermission(mediaPhotoUri.ToString(), flagWrite, callerTokenId, targetTokenId, false);
    ret = upms->CheckUriPermission(mediaPhotoUri, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, true);
    MyFlag::permissionProxyAuthorization_ = false;
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check uri permission of media\audio uri.
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    auto mediaAudioUri = Uri("file://media/Audio/1/Record_001/test_001.mp3");
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    uint32_t flagRead = 1;
    uint32_t flagWrite = 2;

    TokenIdPermission tokenIdPermission(callerTokenId);
    auto ret = upms->CheckUriPermission(mediaAudioUri, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, false);
    
    // read
    MyFlag::permissionReadAudio_ = true;
    tokenIdPermission = TokenIdPermission(targetTokenId);
    ret = upms->CheckUriPermission(mediaAudioUri, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, true);
    ret = upms->CheckUriPermission(mediaAudioUri, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, false);
    MyFlag::permissionReadAudio_ = false;
   
    // write
    MyFlag::permissionWriteAudio_ = true;
    tokenIdPermission = TokenIdPermission(targetTokenId);
    ret = upms->CheckUriPermission(mediaAudioUri, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, true);
    ret = upms->CheckUriPermission(mediaAudioUri, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, true);
    MyFlag::permissionWriteAudio_ = false;
    
    // proxy uri permission
    MyFlag::permissionProxyAuthorization_ = true;
    tokenIdPermission = TokenIdPermission(targetTokenId);
    // no record
    ret = upms->CheckUriPermission(mediaAudioUri, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, false);
    ret = upms->CheckUriPermission(mediaAudioUri, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, false);

    // read
    upms->AddTempUriPermission(mediaAudioUri.ToString(), flagRead, callerTokenId, targetTokenId, false);
    ret = upms->CheckUriPermission(mediaAudioUri, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, true);
    ret = upms->CheckUriPermission(mediaAudioUri, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, false);
    
    // write
    upms->AddTempUriPermission(mediaAudioUri.ToString(), flagWrite, callerTokenId, targetTokenId, false);
    ret = upms->CheckUriPermission(mediaAudioUri, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, true);
    MyFlag::permissionProxyAuthorization_ = false;
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check uri permission of docs uri.
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_003, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    auto docsUri = Uri("file://docs/DestTop/Text/test_001.txt");
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    uint32_t flagRead = 1;
    uint32_t flagWrite = 2;

    TokenIdPermission tokenIdPermission(callerTokenId);
    auto ret = upms->CheckUriPermission(docsUri, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, false);
    
    // have FILE_ACCESS_MANAGER permission
    MyFlag::permissionFileAccessManager_ = true;
    tokenIdPermission = TokenIdPermission(targetTokenId);
    ret = upms->CheckUriPermission(docsUri, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, true);
    ret = upms->CheckUriPermission(docsUri, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, true);
    MyFlag::permissionFileAccessManager_ = false;
    
    // proxy uri permision
    MyFlag::permissionProxyAuthorization_ = true;
    tokenIdPermission = TokenIdPermission(targetTokenId);
    // no record
    ret = upms->CheckUriPermission(docsUri, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, false);
    ret = upms->CheckUriPermission(docsUri, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, false);

    // read
    upms->AddTempUriPermission(docsUri.ToString(), flagRead, callerTokenId, targetTokenId, false);
    ret = upms->CheckUriPermission(docsUri, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, true);
    ret = upms->CheckUriPermission(docsUri, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, false);
    
    // write
    upms->AddTempUriPermission(docsUri.ToString(), flagWrite, callerTokenId, targetTokenId, false);
    ret = upms->CheckUriPermission(docsUri, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, true);
    MyFlag::permissionProxyAuthorization_ = false;
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check uri permission of bunldename uri.
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_004, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    auto uri1 = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    auto uri2 = Uri("file://com.example.app1002/data/storage/el2/base/haps/entry/files/test_002.txt");
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    uint32_t flagRead = 1;
    uint32_t flagWrite = 2;

    TokenIdPermission tokenIdPermission(callerTokenId);
    auto ret = upms->CheckUriPermission(uri1, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, true);
    ret = upms->CheckUriPermission(uri1, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, true);

    ret = upms->CheckUriPermission(uri2, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, false);
    ret = upms->CheckUriPermission(uri2, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, false);

    // proxy uri permision
    MyFlag::permissionProxyAuthorization_ = true;
    tokenIdPermission = TokenIdPermission(targetTokenId);
    // no record
    ret = upms->CheckUriPermission(uri1, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, false);
    ret = upms->CheckUriPermission(uri1, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, false);
    
    // read
    upms->AddTempUriPermission(uri1.ToString(), flagRead, callerTokenId, targetTokenId, false);
    ret = upms->CheckUriPermission(uri1, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, true);
    ret = upms->CheckUriPermission(uri1, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, false);
    
    // write
    upms->AddTempUriPermission(uri1.ToString(), flagWrite, callerTokenId, targetTokenId, false);
    ret = upms->CheckUriPermission(uri1, flagRead, tokenIdPermission);
    ASSERT_EQ(ret, true);
    ret = upms->CheckUriPermission(uri1, flagWrite, tokenIdPermission);
    ASSERT_EQ(ret, true);
    MyFlag::permissionProxyAuthorization_ = false;
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: RevokeAllUriPermission
 * SubFunction: NA
 * FunctionPoints: RevokeAllUriPermission called by SA or SystemApp.
*/
HWTEST_F(UriPermissionImplTest, RevokeAllUriPermission_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    IPCSkeleton::callerTokenId = 1001;
    auto ret = upms->RevokeAllUriPermissions(1002);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: RevokeAllUriPermission
 * SubFunction: NA
 * FunctionPoints: RevokeAllUriPermission not called by SA or SystemApp.
*/
HWTEST_F(UriPermissionImplTest, RevokeAllUriPermission_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ &= (~MyFlag::IS_SA_CALL);
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "tempProcess");
    IPCSkeleton::callerTokenId = 1001;
    auto ret = upms->RevokeAllUriPermissions(1002);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: do not have permission to call GrantUriPermissionPrivileged.
*/
HWTEST_F(UriPermissionImplTest, GrantUriPermissionPrivileged_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);

    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "tempProcess");
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::permissionPrivileged_ = false;

    auto uri1 = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    std::string targetBundleName = "com.example.app1002";
    uint32_t flag = 1;
    const std::vector<Uri> uris = { uri1 };
    auto ret = upms->GrantUriPermissionPrivileged(uris, flag, targetBundleName, 0);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: flag is 0.
*/
HWTEST_F(UriPermissionImplTest, GrantUriPermissionPrivileged_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);

    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::permissionPrivileged_ = true;

    auto uri1 = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    std::string targetBundleName = "com.example.app1002";
    uint32_t flag = 0;
    const std::vector<Uri> uris = { uri1 };
    auto ret = upms->GrantUriPermissionPrivileged(uris, flag, targetBundleName, 0);
    MyFlag::permissionPrivileged_ = false;
    EXPECT_EQ(ret, ERR_CODE_INVALID_URI_FLAG);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: targetBundleName is invalid.
*/
HWTEST_F(UriPermissionImplTest, GrantUriPermissionPrivileged_003, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);

    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::permissionPrivileged_ = true;

    auto uri1 = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    std::string targetBundleName = "com.example.invalid";
    uint32_t flag = 1;
    const std::vector<Uri> uris = { uri1 };
    auto ret = upms->GrantUriPermissionPrivileged(uris, flag, targetBundleName, 0);
    MyFlag::permissionPrivileged_ = false;
    EXPECT_EQ(ret, GET_BUNDLE_INFO_FAILED);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: type of uri is invalid.
*/
HWTEST_F(UriPermissionImplTest, GrantUriPermissionPrivileged_004, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);

    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::permissionPrivileged_ = true;

    auto uri1 = Uri("http://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    std::string targetBundleName = "com.example.app1002";
    uint32_t flag = 1;
    const std::vector<Uri> uris = { uri1 };
    auto ret = upms->GrantUriPermissionPrivileged(uris, flag, targetBundleName, 0);
    MyFlag::permissionPrivileged_ = false;
    EXPECT_EQ(ret, ERR_CODE_INVALID_URI_TYPE);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: Create Share File failed.
*/
HWTEST_F(UriPermissionImplTest, GrantUriPermissionPrivileged_005, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);

    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::permissionPrivileged_ = true;

    auto uri1 = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    std::string targetBundleName = "com.example.app1002";
    uint32_t flag = 1;
    const std::vector<Uri> uris = { uri1 };
    upms->storageManager_ = new StorageManager::StorageManagerServiceMock();
    StorageManager::StorageManagerServiceMock::isZero = false;
    auto ret = upms->GrantUriPermissionPrivileged(uris, flag, targetBundleName, 0);
    MyFlag::permissionPrivileged_ = false;
    EXPECT_EQ(ret, INNER_ERR);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: Grant Uri permission success.
*/
HWTEST_F(UriPermissionImplTest, GrantUriPermissionPrivileged_006, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);

    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::permissionPrivileged_ = true;

    auto uri1 = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    std::string targetBundleName = "com.example.app1002";
    uint32_t flag = 1;
    const std::vector<Uri> uris = { uri1 };
    upms->storageManager_ = new StorageManager::StorageManagerServiceMock();
    StorageManager::StorageManagerServiceMock::isZero = true;
    auto ret = upms->GrantUriPermissionPrivileged(uris, flag, targetBundleName, 0);
    MyFlag::permissionPrivileged_ = false;
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: CheckUriAuthorization not called by SA or SystemApp.
*/
HWTEST_F(UriPermissionImplTest, CheckUriAuthorization_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ &= (~MyFlag::IS_SA_CALL);
    std::string uri = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    uint32_t flag = 1;
    uint32_t tokenId = 1001;
    auto res = upms->CheckUriAuthorization(uris, flag, tokenId);
    std::vector<bool> expectRes(1, false);
    EXPECT_EQ(res, expectRes);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: flag is 0.
*/
HWTEST_F(UriPermissionImplTest, CheckUriAuthorization_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    std::string uri = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    uint32_t flag = 0;
    uint32_t tokenId = 1001;
    auto res = upms->CheckUriAuthorization(uris, flag, tokenId);
    std::vector<bool> expectRes(1, false);
    EXPECT_EQ(res, expectRes);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: uri is invalid.
*/
HWTEST_F(UriPermissionImplTest, CheckUriAuthorization_003, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    std::string uri = "http://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    uint32_t flag = 1;
    uint32_t tokenId = 1001;
    auto res = upms->CheckUriAuthorization(uris, flag, tokenId);
    std::vector<bool> expectRes(1, false);
    EXPECT_EQ(res, expectRes);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: check uri authorization failed, have no permission.
*/
HWTEST_F(UriPermissionImplTest, CheckUriAuthorization_004, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    std::string uri = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    uint32_t flag = 1;
    uint32_t tokenId = 1002;
    auto res = upms->CheckUriAuthorization(uris, flag, tokenId);
    std::vector<bool> expectRes(1, false);
    EXPECT_EQ(res, expectRes);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: check uri authorization success.
*/
HWTEST_F(UriPermissionImplTest, CheckUriAuthorization_005, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    std::string uri = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    uint32_t flag = 1;
    uint32_t tokenId = 1001;
    auto res = upms->CheckUriAuthorization(uris, flag, tokenId);
    std::vector<bool> expectRes(1, true);
    EXPECT_EQ(res, expectRes);
}
}  // namespace AAFwk
}  // namespace OHOS
