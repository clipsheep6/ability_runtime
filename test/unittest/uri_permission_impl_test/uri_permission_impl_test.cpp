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
#include "mock_app_mgr_service.h"
#include "mock_bundle_mgr_helper.h"
#include "mock_sandbox_manager.h"
#include "mock_ipc_skeleton.h"
#include "mock_my_flag.h"
#include "mock_native_token.h"
#include "mock_permission_verification.h"
#include "mock_system_ability_manager_client.h"
#include "ability_manager_errors.h"
#include "file_permission_manager.h"
#include "hilog_tag_wrapper.h"
#include "event_report.h"
#include "system_ability_definition.h"
#include "tokenid_kit.h"
#define private public
#include "batch_uri.h"
#include "uri_permission_manager_stub_impl.h"
#include "uri_permission_utils.h"
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

void UriPermissionImplTest::SetUp()
{
    MyFlag::Init();
    SandboxManagerKit::Init();
    IPCSkeleton::callerTokenId = 0;
    StorageManager::StorageManagerServiceMock::isZero = true;
    MockSystemAbilityManager::isNullptr = false;
    AccessControl::SandboxManager::SandboxManagerKit::SetPolicyRet_ = ERR_OK;
    AccessControl::SandboxManager::SandboxManagerKit::UnSetPolicyRet_ = ERR_OK;
}

void UriPermissionImplTest::TearDown() {}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: Not called by SA or SystemApp.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    Uri uri("file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt");
    unsigned int flag = 0;
    std::string targetBundleName = "name2";
    auto ret = upms->GrantUriPermission(uri, flag, targetBundleName);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: JudgeSandboxByPid called failed.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_002, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    Uri uri("file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt");
    auto mockAppMgr = new AppExecFwk::MockAppMgrService();
    mockAppMgr->judgeSandboxByPidRet_ = INNER_ERR;
    upms->appMgr_ = mockAppMgr;
    uint32_t flag = 1;
    std::string targetBundleName = "name2";
    auto ret = upms->GrantUriPermission(uri, flag, targetBundleName);
    EXPECT_EQ(ret, INNER_ERR);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: JudgeSandboxByPid called with isSandbox = true.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_003, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    Uri uri("file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt");
    auto mockAppMgr = new AppExecFwk::MockAppMgrService();
    mockAppMgr->isSandbox_ = true;
    upms->appMgr_ = mockAppMgr;
    uint32_t flag = 1;
    std::string targetBundleName = "name2";
    auto ret = upms->GrantUriPermission(uri, flag, targetBundleName);
    EXPECT_EQ(ret, ERR_CODE_GRANT_URI_PERMISSION);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: flag is invalid.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_004, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto mockAppMgr = new AppExecFwk::MockAppMgrService();
    upms->appMgr_ = mockAppMgr;
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    Uri uri("file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt");
    unsigned int flag = 0;
    std::string targetBundleName = "name2";
    auto ret = upms->GrantUriPermission(uri, flag, targetBundleName);
    EXPECT_EQ(ret, ERR_CODE_INVALID_URI_FLAG);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: get target tokenId by bundleName failed.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_005, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto mockAppMgr = new AppExecFwk::MockAppMgrService();
    upms->appMgr_ = mockAppMgr;
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    Uri uri("file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt");
    unsigned int flag = 1;
    std::string targetBundleName = "name2";
    auto ret = upms->GrantUriPermission(uri, flag, targetBundleName);
    EXPECT_EQ(ret, GET_BUNDLE_INFO_FAILED);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: uri is invalid.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_006, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto mockAppMgr = new AppExecFwk::MockAppMgrService();
    upms->appMgr_ = mockAppMgr;
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    Uri uri("invalid://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt");
    unsigned int flag = 1;
    std::string targetBundleName = "com.example.app1001";
    auto ret = upms->GrantUriPermission(uri, flag, targetBundleName);
    EXPECT_EQ(ret, ERR_CODE_INVALID_URI_TYPE);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: check uri permission failed.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_007, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto mockAppMgr = new AppExecFwk::MockAppMgrService();
    upms->appMgr_ = mockAppMgr;
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    Uri uri("file://com.example.app1002/data/storage/el2/base/haps/entry/files/test_A.txt");
    unsigned int flag = 1;
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1001", "com.example.app1001");
    std::string targetBundleName = "com.example.app1003";
    auto ret = upms->GrantUriPermission(uri, flag, targetBundleName);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: create share file failed.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_008, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto mockAppMgr = new AppExecFwk::MockAppMgrService();
    upms->appMgr_ = mockAppMgr;
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    upms->storageManager_ = new StorageManager::StorageManagerServiceMock();
    StorageManager::StorageManagerServiceMock::isZero = false;
    
    unsigned int flag = 1;
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    std::string targetBundleName = "com.example.app1003";

    // content uri
    Uri uri1("content://data/storage/el2/base/haps/entry/files/test_A.txt");
    auto ret = upms->GrantUriPermission(uri1, flag, targetBundleName);
    EXPECT_EQ(ret, INNER_ERR);

    // media uri
    Uri uri2("file://media/Photo/1/IMG_001/test_001.jpg");
    MyFlag::permissionAllMedia_ = true;
    ret = upms->GrantUriPermission(uri2, flag, targetBundleName);
    EXPECT_EQ(ret, INNER_ERR);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: create share file success.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_009, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto mockAppMgr = new AppExecFwk::MockAppMgrService();
    upms->appMgr_ = mockAppMgr;
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    
    unsigned int flag = 1;
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    std::string targetBundleName = "com.example.app1003";
    upms->storageManager_ = new StorageManager::StorageManagerServiceMock();
    
    // content uri
    Uri uri1("content://data/storage/el2/base/haps/entry/files/test_A.txt");
    auto ret = upms->GrantUriPermission(uri1, flag, targetBundleName);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(upms->uriMap_.empty(), false);
    
    // media uri
    MyFlag::permissionAllMedia_ = true;
    Uri uri2("file://media/Photo/1/IMG_001/test_001.jpg");
    ret = upms->GrantUriPermission(uri2, flag, targetBundleName);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(upms->uriMap_.empty(), false);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: SetPolicy failed.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_010, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto mockAppMgr = new AppExecFwk::MockAppMgrService();
    upms->appMgr_ = mockAppMgr;
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    
    unsigned int flag = 1;
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1001", "com.example.app1001");
    std::string targetBundleName = "com.example.app1002";
    AccessControl::SandboxManager::SandboxManagerKit::SetPolicyRet_ = INNER_ERR;
    
    // bundle uri
    Uri uri1("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    auto ret = upms->GrantUriPermission(uri1, flag, targetBundleName);
    EXPECT_EQ(ret, INNER_ERR);

    // docs uri
    MyFlag::permissionFileAccessManager_ = true;
    Uri uri2("file://docs/Photo/1/IMG_001/test_001.jpg");
    ret = upms->GrantUriPermission(uri2, flag, targetBundleName);
    EXPECT_EQ(ret, INNER_ERR);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: SetPolicy Success and is not systemAppCall.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_011, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto mockAppMgr = new AppExecFwk::MockAppMgrService();
    upms->appMgr_ = mockAppMgr;
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    unsigned int flag = 1;
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1001", "com.example.app1001");
    std::string targetBundleName = "com.example.app1002";
    MyFlag::IsSystempAppCall_ = false;

    // bundle uri
    Uri uri1("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    auto ret = upms->GrantUriPermission(uri1, flag, targetBundleName);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(upms->policyMap_.empty(), true);

    // docs uri
    MyFlag::permissionFileAccessManager_ = true;
    Uri uri2("file://docs/Photo/1/IMG_001/test_001.jpg");
    ret = upms->GrantUriPermission(uri2, flag, targetBundleName);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(upms->policyMap_.empty(), true);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: SetPolicy Success and is systemAppCall.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_012, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto mockAppMgr = new AppExecFwk::MockAppMgrService();
    upms->appMgr_ = mockAppMgr;
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    unsigned int flag = 1;
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1001", "com.example.app1001");
    std::string targetBundleName = "com.example.app1002";
    MyFlag::IsSystempAppCall_ = true;

    // bundle uri
    Uri uri1("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    auto ret = upms->GrantUriPermission(uri1, flag, targetBundleName);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(upms->policyMap_.empty(), false);

    // docs uri
    MyFlag::permissionFileAccessManager_ = true;
    Uri uri2("file://docs/Photo/1/IMG_001/test_001.jpg");
    ret = upms->GrantUriPermission(uri2, flag, targetBundleName);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(upms->policyMap_.empty(), false);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: caller by foundation.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_013, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto mockAppMgr = new AppExecFwk::MockAppMgrService();
    upms->appMgr_ = mockAppMgr;
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    unsigned int flag = 1;
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    MyFlag::tokenInfos[1002] = TokenInfo(1002, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1002", "com.example.app1002");
    MyFlag::tokenInfos[1003] = TokenInfo(1003, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1003", "com.example.app1003");
    std::string targetBundleName = "com.example.app1004";
    
    // uri belong to initialTokenId
    Uri uri("file://com.example.app1002/data/storage/el2/base/haps/entry/files/test_A.txt");
    auto ret = upms->GrantUriPermission(uri, flag, targetBundleName, 0, 1002);
    EXPECT_EQ(ret, ERR_OK);
    
    // uri do not belong to initialTokenId
    ret = upms->GrantUriPermission(uri, flag, targetBundleName, 0, 1003);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: Grant batch uris permission.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_014, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto mockAppMgr = new AppExecFwk::MockAppMgrService();
    upms->appMgr_ = mockAppMgr;
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    unsigned int flag = 1;
    IPCSkeleton::callerTokenId = 1000;
    MyFlag::tokenInfos[1000] = TokenInfo(1000, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1001", "com.example.app1001");
    MyFlag::tokenInfos[1002] = TokenInfo(1002, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1002", "com.example.app1002");
    MyFlag::tokenInfos[1003] = TokenInfo(1003, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1003", "com.example.app1003");
    std::string targetBundleName = "com.example.app1002";
    uint32_t initialTokenId = 1001;
    uint32_t targetTokenId = 1002;

    std::vector<Uri> uriVec = {
        // invalid
        Uri("invalid://batchuri_001.txt"),
        // content
        Uri("content://batchuri_001.txt"),
        // docs
        Uri("file://docs/batchuri_001.txt"),
        // media
        Uri("file://media/Photo/1/IMG_001/test_001.jpg"),
        // caller
        Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt"),
        // target uri
        Uri("file://com.example.app1002/data/storage/el2/base/haps/entry/files/test_002.txt"),
        // other
        Uri("file://com.example.app1003/data/storage/el2/base/haps/entry/files/test_003.txt")
    };

    MyFlag::permissionFileAccessManager_ = true;
    MyFlag::permissionAllMedia_ = true;
    upms->storageManager_ = new StorageManager::StorageManagerServiceMock();
    auto ret = upms->GrantUriPermission(uriVec, flag, targetBundleName, 0, initialTokenId);
    EXPECT_EQ(ret, ERR_OK);

    auto verifyRet = upms->VerifyUriPermission(uriVec[0], flag, targetTokenId);
    EXPECT_EQ(verifyRet, false);

    verifyRet = upms->VerifyUriPermission(uriVec[1], flag, targetTokenId);
    EXPECT_EQ(verifyRet, true);

    verifyRet = upms->VerifyUriPermission(uriVec[2], flag, targetTokenId);
    EXPECT_EQ(verifyRet, true);

    verifyRet = upms->VerifyUriPermission(uriVec[3], flag, targetTokenId);
    EXPECT_EQ(verifyRet, true);

    verifyRet = upms->VerifyUriPermission(uriVec[4], flag, targetTokenId);
    EXPECT_EQ(verifyRet, true);
    
    // grant target's uri to target
    verifyRet = upms->VerifyUriPermission(uriVec[5], flag, targetTokenId);
    EXPECT_EQ(verifyRet, false);

    verifyRet = upms->VerifyUriPermission(uriVec[6], flag, targetTokenId);
    EXPECT_EQ(verifyRet, true);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: Grant bundle uri permission.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_015, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto mockAppMgr = new AppExecFwk::MockAppMgrService();
    upms->appMgr_ = mockAppMgr;
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    unsigned int flag = 1;
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1001", "com.example.app1001");
    MyFlag::tokenInfos[1002] = TokenInfo(1002, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1002", "com.example.app1002");

    Uri uri1("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    Uri uri2("file://com.example.app1002/data/storage/el2/base/haps/entry/files/test_B.txt");

    // caller grant it's uri to itself
    SandboxManagerKit::Init();
    std::string targetBundleName = "com.example.app1001";
    auto ret = upms->GrantUriPermission(uri1, flag, targetBundleName);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(SandboxManagerKit::policyMap_.empty(), true);

    // CheckUriPermission success
    SandboxManagerKit::Init();
    targetBundleName = "com.example.app1002";
    ret = upms->GrantUriPermission(uri1, flag, targetBundleName);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(SandboxManagerKit::policyMap_.empty(), false);

    // CheckUriPermission failed
    targetBundleName = "com.example.app1002";
    ret = upms->GrantUriPermission(uri2, flag, targetBundleName);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    // grant uri of target app to target app
    SandboxManagerKit::Init();
    MyFlag::permissionFileAccessManager_ = true;
    targetBundleName = "com.example.app1002";
    ret = upms->GrantUriPermission(uri2, flag, targetBundleName);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(SandboxManagerKit::policyMap_.empty(), true);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: Grant uri permission with FLAG_AUTH_PERSISTABLE_URI_PERMISSION.
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_016, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto mockAppMgr = new AppExecFwk::MockAppMgrService();
    upms->appMgr_ = mockAppMgr;
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    uint32_t FLAG_PERSIST_URI = 64;
    unsigned int flag = (1 | FLAG_PERSIST_URI);
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1001", "com.example.app1001");

    // bundle uri
    SandboxManagerKit::Init();
    Uri uri1("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    auto targetBundleName = "com.example.app1002";
    auto ret = upms->GrantUriPermission(uri1, flag, targetBundleName);
    EXPECT_EQ(ret, ERR_OK);
    auto policyRecord = SandboxManagerKit::policyMap_.begin();
    EXPECT_EQ((policyRecord->second & FLAG_PERSIST_URI), FLAG_PERSIST_URI);

    // bundle uri
    SandboxManagerKit::Init();
    MyFlag::permissionFileAccessManager_ = true;
    Uri uri2("file://docs/Photo/1/IMG_001/test_001.jpg");
    ret = upms->GrantUriPermission(uri2, flag, targetBundleName);
    EXPECT_EQ(ret, ERR_OK);
    policyRecord = SandboxManagerKit::policyMap_.begin();
    EXPECT_EQ((policyRecord->second & FLAG_PERSIST_URI), FLAG_PERSIST_URI);
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
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: not called by SA or SystemApp
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermissionManually_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    int32_t appIndex = 0;
    std::string targetBundleName = "com.example.testB1002";
    auto uri = Uri("file://com.example.testA/data/storage/el2/base/haps/entry/files/test_A.txt");
    auto ret = upms->RevokeUriPermissionManually(uri, targetBundleName, appIndex);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: uri is invalid.
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermissionManually_002, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    int32_t appIndex = 0;
    std::string targetBundleName = "com.example.testB1002";
    auto uri = Uri("invalid://com.example.testA/data/storage/el2/base/haps/entry/files/test_A.txt");
    auto ret = upms->RevokeUriPermissionManually(uri, targetBundleName, appIndex);
    EXPECT_EQ(ret, ERR_CODE_INVALID_URI_TYPE);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: GetTokenIdByBundleName failed.
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermissionManually_003, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    int32_t appIndex = 0;
    std::string targetBundleName = "invalidName";
    auto uri = Uri("file://com.example.testA/data/storage/el2/base/haps/entry/files/test_A.txt");
    auto ret = upms->RevokeUriPermissionManually(uri, targetBundleName, appIndex);
    EXPECT_EQ(ret, INNER_ERR);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: revoke docs uri not permissioned.
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermissionManually_004, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    int32_t appIndex = 0;
    auto docsUri = Uri("file://docs/DestTop/Text/test_001.txt");
    auto path = "/DestTop/Text/test_001.txt";
    auto callerTokenId = 1001;
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1001", "com.example.app1001");
    auto targetTokenId = 1003;
    upms->AddPolicyRecordCache(callerTokenId, targetTokenId, path);
    bool recordExists = upms->policyMap_.find(path) != upms->policyMap_.end();
    EXPECT_EQ(recordExists, true);
    
    std::string targetBundleName = "com.example.test1002";
    auto ret = upms->RevokeUriPermissionManually(docsUri, targetBundleName, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    recordExists = upms->policyMap_.find(path) != upms->policyMap_.end();
    EXPECT_EQ(recordExists, false);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: revoke docs uri permissioned but UnSetPolicy Failed.
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermissionManually_005, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    int32_t appIndex = 0;
    std::string targetBundleName = "com.example.test1002";
    auto docsUri = Uri("file://docs/DestTop/Text/test_001.txt");
    auto path = "/DestTop/Text/test_001.txt";
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1001", "com.example.app1001");
    auto callerTokenId = 1001;
    auto targetTokenId = 1002;
    upms->AddPolicyRecordCache(callerTokenId, targetTokenId, path);
    bool recordExists = (upms->policyMap_.find(path) != upms->policyMap_.end());
    EXPECT_EQ(recordExists, true);
    
    SandboxManagerKit::UnSetPolicyRet_ = INNER_ERR;
    auto ret = upms->RevokeUriPermissionManually(docsUri, targetBundleName, appIndex);
    EXPECT_EQ(ret, INNER_ERR);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: revoke docs uri permissioned success by caller.
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermissionManually_006, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    int32_t appIndex = 0;
    auto docsUri = Uri("file://docs/DestTop/Text/test_001.txt");
    auto path = "/DestTop/Text/test_001.txt";
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.test1002", "com.example.test1002");
    auto callerTokenId = 1001;
    auto targetTokenId = 1002;
    upms->AddPolicyRecordCache(callerTokenId, targetTokenId, path);
    bool recordExists = (upms->policyMap_.find(path) != upms->policyMap_.end());
    EXPECT_EQ(recordExists, true);
    
    std::string targetBundleName = "com.example.test1002";
    auto ret = upms->RevokeUriPermissionManually(docsUri, targetBundleName, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    recordExists = (upms->policyMap_.find(path) != upms->policyMap_.end());
    EXPECT_EQ(recordExists, false);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: revoke docs uri permissioned success by target.
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermissionManually_007, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    int32_t appIndex = 0;
    auto docsUri = Uri("file://docs/DestTop/Text/test_001.txt");
    auto path = "/DestTop/Text/test_001.txt";
    IPCSkeleton::callerTokenId = 1002;
    MyFlag::tokenInfos[1002] = TokenInfo(1002, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.test1002", "com.example.test1002");
    auto callerTokenId = 1001;
    auto targetTokenId = 1002;
    upms->AddPolicyRecordCache(callerTokenId, targetTokenId, path);
    bool recordExists = (upms->policyMap_.find(path) != upms->policyMap_.end());
    EXPECT_EQ(recordExists, true);
    
    std::string targetBundleName = "com.example.test1002";
    auto ret = upms->RevokeUriPermissionManually(docsUri, targetBundleName, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    recordExists = (upms->policyMap_.find(path) != upms->policyMap_.end());
    EXPECT_EQ(recordExists, false);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: revoke docs uri permissioned success by caller when .
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermissionManually_008, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    int32_t appIndex = 0;
    auto docsUri = Uri("file://docs/DestTop/Text/test_001.txt");
    auto path = "/DestTop/Text/test_001.txt";
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::tokenInfos[1002] = TokenInfo(1002, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.test1002", "com.example.test1002");
    auto callerTokenId = 1001;
    auto callerTokenId2 = 1003;
    auto targetTokenId = 1002;
    upms->AddPolicyRecordCache(callerTokenId, targetTokenId, path);
    upms->AddPolicyRecordCache(callerTokenId2, targetTokenId, path);
    bool recordExists = (upms->policyMap_.find(path) != upms->policyMap_.end());
    EXPECT_EQ(recordExists, true);
    
    std::string targetBundleName = "com.example.test1002";
    auto ret = upms->RevokeUriPermissionManually(docsUri, targetBundleName, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    recordExists = (upms->policyMap_.find(path) != upms->policyMap_.end());
    EXPECT_EQ(recordExists, false);
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
    EXPECT_EQ(storageManager, nullptr);
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
    EXPECT_EQ(storageManager, nullptr);
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
    std::string uri = "file://media/Photo/1/IMG_001/test_001.jpg";
    auto flagRead = 1;
    auto flagWrite = 2;
    auto flagReadWrite = 3;

    // read
    upms->uriMap_.clear();
    upms->AddTempUriPermission(uri, flagRead, callerTokenId, targetTokenId, 0);
    auto ret = upms->VerifyUriPermission(Uri(uri), flagRead, targetTokenId);
    EXPECT_EQ(ret, true);
    ret = upms->VerifyUriPermission(Uri(uri), flagWrite, targetTokenId);
    EXPECT_EQ(ret, false);
    ret = upms->VerifyUriPermission(Uri(uri), flagReadWrite, targetTokenId);
    EXPECT_EQ(ret, false);
    upms->uriMap_.clear();
    
    // write
    upms->uriMap_.clear();
    upms->AddTempUriPermission(uri, flagWrite, callerTokenId, targetTokenId, 0);
    ret = upms->VerifyUriPermission(Uri(uri), flagRead, targetTokenId);
    EXPECT_EQ(ret, true);
    ret = upms->VerifyUriPermission(Uri(uri), flagWrite, targetTokenId);
    EXPECT_EQ(ret, true);
    ret = upms->VerifyUriPermission(Uri(uri), flagReadWrite, targetTokenId);
    EXPECT_EQ(ret, true);
    upms->uriMap_.clear();

    // flagReadWrite
    upms->AddTempUriPermission(uri, flagReadWrite, callerTokenId, targetTokenId, 0);
    ret = upms->VerifyUriPermission(Uri(uri), flagRead, targetTokenId);
    EXPECT_EQ(ret, true);
    ret = upms->VerifyUriPermission(Uri(uri), flagWrite, targetTokenId);
    EXPECT_EQ(ret, true);
    ret = upms->VerifyUriPermission(Uri(uri), flagReadWrite, targetTokenId);
    EXPECT_EQ(ret, true);
    upms->uriMap_.clear();
    
    // no permission record
    ret = upms->VerifyUriPermission(Uri(uri), flagRead, invalidTokenId);
    EXPECT_EQ(ret, false);
    upms->uriMap_.clear();
}

/*
 * Feature: URIPermissionManagerService
 * Function: VerifyUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService VerifyUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_VerifyUriPermission_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    auto targetTokenId = 1002;
    auto invalidTokenId = 1003;
    auto uri = Uri("file://docs/Photo/1/IMG_001/test_001.jpg");
    auto flagRead = 1;
    auto flagWrite = 2;
    auto flagReadWrite = 3;

    std::vector<uint32_t> setPolicyResult;
    auto docsPolicyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uri, 0);
    docsPolicyInfo.mode = flagRead;
    std::vector<PolicyInfo> policys = { docsPolicyInfo };

    // read
    SandboxManagerKit::Init();
    SandboxManagerKit::SetPolicy(targetTokenId, policys, 0, setPolicyResult);
    auto ret = upms->VerifyUriPermission(uri, flagRead, targetTokenId);
    EXPECT_EQ(ret, true);
    ret = upms->VerifyUriPermission(uri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, false);
    ret = upms->VerifyUriPermission(uri, flagReadWrite, targetTokenId);
    EXPECT_EQ(ret, false);

    // write
    SandboxManagerKit::Init();
    docsPolicyInfo.mode = flagWrite;
    policys = { docsPolicyInfo };
    SandboxManagerKit::SetPolicy(targetTokenId, policys, 0, setPolicyResult);
    ret = upms->VerifyUriPermission(uri, flagRead, targetTokenId);
    EXPECT_EQ(ret, false);
    ret = upms->VerifyUriPermission(uri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, true);
    ret = upms->VerifyUriPermission(uri, flagReadWrite, targetTokenId);
    EXPECT_EQ(ret, false);
    
    // read write
    SandboxManagerKit::Init();
    docsPolicyInfo.mode = flagReadWrite;
    policys = { docsPolicyInfo };
    SandboxManagerKit::SetPolicy(targetTokenId, policys, 0, setPolicyResult);
    ret = upms->VerifyUriPermission(uri, flagRead, targetTokenId);
    EXPECT_EQ(ret, true);
    ret = upms->VerifyUriPermission(uri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, true);
    ret = upms->VerifyUriPermission(uri, flagReadWrite, targetTokenId);
    EXPECT_EQ(ret, true);
    
    // no permission record
    ret = upms->VerifyUriPermission(uri, flagRead, invalidTokenId);
    EXPECT_EQ(ret, false);
}

/*
 * Feature: URIPermissionManagerService
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService SendSystemAppGrantUriPermissionEvent
 */
HWTEST_F(UriPermissionImplTest, Upms_SendSystemAppGrantUriPermissionEvent_001, TestSize.Level1)
{
    std::vector<Uri> uriVec = { Uri("file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt") };
    const std::vector<bool> resVec = { true };
    auto ret = UPMSUtils::SendSystemAppGrantUriPermissionEvent(1001, 1002, uriVec, resVec);
    EXPECT_EQ(ret, false);
}

/*
 * Feature: URIPermissionManagerService
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService SendShareUnPrivilegeUriEvent
 */
HWTEST_F(UriPermissionImplTest, Upms_SendShareUnPrivilegeUriEvent_001, TestSize.Level1)
{
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    auto ret = UPMSUtils::SendShareUnPrivilegeUriEvent(1001, 1002);
    EXPECT_EQ(ret, false);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check uri permission of media\photo uri.
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_Media_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto mediaPhotoUri = Uri("file://media/Photo/1/IMG_001/test_001.jpg");
    std::vector<Uri> uriVec = { mediaPhotoUri };
    BatchUri batchUri;
    batchUri.Init(uriVec);
    
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    uint32_t flagRead = 1;
    uint32_t flagWrite = 2;

    auto ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    
    // read
    MyFlag::permissionReadImageVideo_ = true;
    ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    MyFlag::permissionReadImageVideo_ = false;
   
    // write
    MyFlag::permissionWriteImageVideo_ = true;
    ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    MyFlag::permissionWriteImageVideo_ = false;

    // proxy uri permision
    MyFlag::permissionProxyAuthorization_ = true;
    // no record
    ret = upms->CheckUriPermission(batchUri, flagRead, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    // read
    upms->AddTempUriPermission(mediaPhotoUri.ToString(), flagRead, callerTokenId, targetTokenId, false);
    ret = upms->CheckUriPermission(batchUri, flagRead, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    upms->uriMap_.clear();
    
    // write
    upms->AddTempUriPermission(mediaPhotoUri.ToString(), flagWrite, callerTokenId, targetTokenId, false);
    ret = upms->CheckUriPermission(batchUri, flagRead, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    upms->uriMap_.clear();
    MyFlag::permissionProxyAuthorization_ = false;
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check uri permission of media\audio uri.
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_Media_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto mediaAudioUri = Uri("file://media/Audio/1/Record_001/test_001.mp3");
    std::vector<Uri> uriVec = { mediaAudioUri };
    BatchUri batchUri;
    batchUri.Init(uriVec);
    
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    uint32_t flagRead = 1;
    uint32_t flagWrite = 2;

    auto ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    
    // read
    MyFlag::permissionReadAudio_ = true;
    ret = upms->CheckUriPermission(batchUri, flagRead, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    MyFlag::permissionReadAudio_ = false;
   
    // write
    MyFlag::permissionWriteAudio_ = true;
    ret = upms->CheckUriPermission(batchUri, flagRead, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    MyFlag::permissionWriteAudio_ = false;
    
    // proxy uri permission
    MyFlag::permissionProxyAuthorization_ = true;
    // no record
    ret = upms->CheckUriPermission(batchUri, flagRead, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    // read
    upms->AddTempUriPermission(mediaAudioUri.ToString(), flagRead, callerTokenId, targetTokenId, 0);
    ret = upms->CheckUriPermission(batchUri, flagRead, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    upms->uriMap_.clear();
    
    // write
    upms->AddTempUriPermission(mediaAudioUri.ToString(), flagWrite, callerTokenId, targetTokenId, 0);
    ret = upms->CheckUriPermission(batchUri, flagRead, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    upms->uriMap_.clear();
    MyFlag::permissionProxyAuthorization_ = false;
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check uri permission of docs uri.
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_Docs_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto docsUri = Uri("file://docs/DestTop/Text/test_001.txt");
    std::vector<Uri> uriVec = { docsUri };
    BatchUri batchUri;
    batchUri.Init(uriVec);

    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    uint32_t flagRead = 1;
    uint32_t flagWrite = 2;

    auto ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    
    // have FILE_ACCESS_MANAGER permission
    MyFlag::permissionFileAccessManager_ = true;
    ret = upms->CheckUriPermission(batchUri, flagRead, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    MyFlag::permissionFileAccessManager_ = false;
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check uri permission of docs uri with proxy permission.
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_Docs_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto docsUri = Uri("file://docs/DestTop/Text/test_001.txt");
    auto docsPolicyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(docsUri, 0);
    std::vector<Uri> uriVec = { docsUri };
    BatchUri batchUri;
    batchUri.Init(uriVec);

    uint32_t targetTokenId = 1002;
    uint32_t flagRead = 1;
    uint32_t flagWrite = 2;
    
    // proxy uri permision
    MyFlag::permissionProxyAuthorization_ = true;
    // no record
    auto ret = upms->CheckUriPermission(batchUri, flagRead, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    // read
    docsPolicyInfo.mode = flagRead;
    std::vector<uint32_t> setPolicyResult;
    std::vector<PolicyInfo> policys = { docsPolicyInfo };
    SandboxManagerKit::Init();
    SandboxManagerKit::SetPolicy(targetTokenId, policys, 0, setPolicyResult);
    ret = upms->CheckUriPermission(batchUri, flagRead, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    
    // write
    docsPolicyInfo.mode = flagWrite;
    policys = { docsPolicyInfo };
    SandboxManagerKit::Init();
    SandboxManagerKit::SetPolicy(targetTokenId, policys, 0, setPolicyResult);
    ret = upms->CheckUriPermission(batchUri, flagRead, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    // read write
    docsPolicyInfo.mode = flagRead | flagWrite;
    policys = { docsPolicyInfo };
    SandboxManagerKit::Init();
    SandboxManagerKit::SetPolicy(targetTokenId, policys, 0, setPolicyResult);
    ret = upms->CheckUriPermission(batchUri, flagRead, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    MyFlag::permissionProxyAuthorization_ = false;
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check uri permission of docs uri with persist permission
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_Docs_003, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto docsUri = Uri("file://docs/DestTop/Text/test_001.txt");
    auto docsPolicyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(docsUri, 0);
    std::vector<Uri> uriVec = { docsUri };
    BatchUri batchUri;
    batchUri.Init(uriVec);

    uint32_t callerTokenId = 1001;
    uint32_t flagRead = 1;
    uint32_t flagWrite = 2;

    // persist read
    docsPolicyInfo.mode = flagRead;
    std::vector<uint32_t> setPolicyResult;
    std::vector<PolicyInfo> policys = { docsPolicyInfo };
    SandboxManagerKit::Init();
    SandboxManagerKit::PersistPolicy(callerTokenId, policys, setPolicyResult);
    auto ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    
    // persist write
    docsPolicyInfo.mode = flagWrite;
    policys = { docsPolicyInfo };
    SandboxManagerKit::Init();
    SandboxManagerKit::PersistPolicy(callerTokenId, policys, setPolicyResult);
    ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri, flagWrite, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    // persist read write
    docsPolicyInfo.mode = flagRead | flagWrite;
    policys = { docsPolicyInfo };
    SandboxManagerKit::Init();
    SandboxManagerKit::PersistPolicy(callerTokenId, policys, setPolicyResult);
    ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check uri permission of download docs uri
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_Docs_004, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto docsUri = Uri("file://docs/storage/Users/currentUser/Download/test.txt");
    std::vector<Uri> uriVec = { docsUri };
    BatchUri batchUri;
    batchUri.Init(uriVec);

    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    uint32_t flagRead = 1;
    uint32_t flagWrite = 2;

    auto ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    
    // have PERMISSION_READ_WRITE_DOWNLOAD permission
    MyFlag::permissionReadWriteDownload_ = true;
    ret = upms->CheckUriPermission(batchUri, flagRead, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    MyFlag::permissionReadWriteDownload_ = false;
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check uri permission of desktop docs uri
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_Docs_005, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto docsUri = Uri("file://docs/storage/Users/currentUser/Desktop/test.txt");
    std::vector<Uri> uriVec = { docsUri };
    BatchUri batchUri;
    batchUri.Init(uriVec);

    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    uint32_t flagRead = 1;
    uint32_t flagWrite = 2;

    auto ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    
    // have PERMISSION_READ_WRITE_DESKTON permission
    MyFlag::permissionReadWriteDesktop_ = true;
    ret = upms->CheckUriPermission(batchUri, flagRead, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    MyFlag::permissionReadWriteDesktop_ = false;
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check uri permission of download docs uri
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_Docs_006, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto docsUri = Uri("file://docs/storage/Users/currentUser/Documents/test.txt");
    std::vector<Uri> uriVec = { docsUri };
    BatchUri batchUri;
    batchUri.Init(uriVec);

    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    uint32_t flagRead = 1;
    uint32_t flagWrite = 2;

    auto ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    
    // have PERMISSION_READ_WRITE_DOCUMENTS permission
    MyFlag::permissionReadWriteDocuments_ = true;
    ret = upms->CheckUriPermission(batchUri, flagRead, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    MyFlag::permissionReadWriteDocuments_ = false;
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check uri permission of bunldename uri.
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_Bundle_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1001", "com.example.app1001");
    MyFlag::tokenInfos[1002] = TokenInfo(1002, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1002", "com.example.app1002");

    auto uri1 = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    auto uri2 = Uri("file://com.example.app1002/data/storage/el2/base/haps/entry/files/test_002.txt");
    std::vector<Uri> uriVec1 = { uri1 };
    BatchUri batchUri1;
    std::vector<Uri> uriVec2 = { uri2 };
    BatchUri batchUri2;

    uint32_t callerTokenId = 1001;
    uint32_t flagRead = 1;
    uint32_t flagWrite = 2;
    
    batchUri1.Init(uriVec1, 0, "com.example.app1001");
    auto ret = upms->CheckUriPermission(batchUri1, flagRead, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri1, flagWrite, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri1, flagRead | flagWrite, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    
    batchUri2.Init(uriVec2, 0, "com.example.app1001");
    ret = upms->CheckUriPermission(batchUri2, flagRead, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri2, flagWrite, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri2, flagRead | flagWrite, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check uri permission of bunldename uri with FILE_ACCESS_MANAGER
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_Bundle_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1001", "com.example.app1001");
    auto uri = Uri("file://com.example.app1002/data/storage/el2/base/haps/entry/files/test_002.txt");
    std::vector<Uri> uriVec = { uri };
    BatchUri batchUri;
    batchUri.Init(uriVec, 0, "com.example.app1001");

    uint32_t callerTokenId = 1001;
    uint32_t flagRead = 1;
    uint32_t flagWrite = 2;
    
    // have FILE_ACCESS_MANAGER permission
    MyFlag::permissionFileAccessManager_ = true;
    auto ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    MyFlag::permissionFileAccessManager_ = false;
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check uri permission of bunldename uri with proxy permission
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_Bundle_003, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1001", "com.example.app1001");
    MyFlag::tokenInfos[1002] = TokenInfo(1002, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1002", "com.example.app1002");

    auto uri1 = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    uint32_t flagRead = 1;
    uint32_t flagWrite = 2;
    auto uri1PolicyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uri1, flagRead);
    std::vector<Uri> uriVec1 = { uri1 };
    BatchUri batchUri1;
    uint32_t targetTokenId = 1002;
    
    // proxy uri permision
    batchUri1.Init(uriVec1, 0, "com.example.app1002");
    MyFlag::permissionProxyAuthorization_ = true;
    // no record
    auto ret = upms->CheckUriPermission(batchUri1, flagRead, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri1, flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    
    // read
    std::vector<uint32_t> setPolicyResult;
    std::vector<PolicyInfo> policys = { uri1PolicyInfo };
    SandboxManagerKit::Init();
    SandboxManagerKit::SetPolicy(targetTokenId, policys, 0, setPolicyResult);
    ret = upms->CheckUriPermission(batchUri1, flagRead, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri1, flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri1, flagRead | flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    
    // write
    policys[0].mode = flagWrite;
    SandboxManagerKit::Init();
    SandboxManagerKit::SetPolicy(targetTokenId, policys, 0, setPolicyResult);
    ret = upms->CheckUriPermission(batchUri1, flagRead, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri1, flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri1, flagRead | flagWrite, targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    // read write
    policys[0].mode = flagRead | flagWrite;
    SandboxManagerKit::Init();
    SandboxManagerKit::SetPolicy(targetTokenId, policys, 0, setPolicyResult);
    ret = upms->CheckUriPermission(batchUri1, flagRead, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri1, flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri1, flagRead | flagWrite, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check uri permission of bunldename uri with persist permission
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_Bundle_004, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1001", "com.example.app1001");
    MyFlag::tokenInfos[1002] = TokenInfo(1002, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1002", "com.example.app1002");

    auto uri = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    auto uri1PolicyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uri, 0);
    std::vector<Uri> uriVec = { uri };
    BatchUri batchUri;
    batchUri.Init(uriVec, 0, "com.example.app1002");

    uint32_t callerTokenId = 1002;
    uint32_t flagRead = 1;
    uint32_t flagWrite = 2;
    
    // persist read
    std::vector<uint32_t> setPolicyResult;
    uri1PolicyInfo.mode = flagRead;
    std::vector<PolicyInfo> policys = { uri1PolicyInfo };
    SandboxManagerKit::Init();
    SandboxManagerKit::PersistPolicy(callerTokenId, policys, setPolicyResult);
    auto ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    
    // persist write
    uri1PolicyInfo.mode = flagWrite;
    policys = { uri1PolicyInfo };
    SandboxManagerKit::Init();
    SandboxManagerKit::PersistPolicy(callerTokenId, policys, setPolicyResult);
    ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    ret = upms->CheckUriPermission(batchUri, flagWrite, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, callerTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    // persist read write
    uri1PolicyInfo.mode = flagRead | flagWrite;
    policys = { uri1PolicyInfo };
    SandboxManagerKit::Init();
    SandboxManagerKit::PersistPolicy(callerTokenId, policys, setPolicyResult);
    ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagWrite, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
    ret = upms->CheckUriPermission(batchUri, flagRead | flagWrite, callerTokenId);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: Check content uri.
*/
HWTEST_F(UriPermissionImplTest, Upms_CheckUriPermission_Content_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    auto uri = Uri("content://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    std::vector<Uri> uriVec = { uri };
    BatchUri batchUri;
    batchUri.Init(uriVec);

    uint32_t flagRead = 1;
    uint32_t callerTokenId1 = 1001;
    IPCSkeleton::callerTokenId = callerTokenId1;
    MyFlag::tokenInfos[callerTokenId1] = TokenInfo(callerTokenId1, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    auto ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId1);
    EXPECT_EQ(ret, ERR_OK);

    uint32_t callerTokenId2 = 1002;
    IPCSkeleton::callerTokenId = callerTokenId2;
    MyFlag::tokenInfos[callerTokenId2] = TokenInfo(callerTokenId2, MyATokenTypeEnum::TOKEN_NATIVE, "testProcess");
    ret = upms->CheckUriPermission(batchUri, flagRead, callerTokenId2);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
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
    MyFlag::permissionAllMedia_ = true;

    auto uri1 = Uri("file://media/Photo/1/IMG_001/test_001.jpg");
    std::string targetBundleName = "com.example.app1002";
    uint32_t flag = 1;
    const std::vector<Uri> uris = { uri1 };
    upms->storageManager_ = new StorageManager::StorageManagerServiceMock();
    StorageManager::StorageManagerServiceMock::isZero = false;
    auto ret = upms->GrantUriPermissionPrivileged(uris, flag, targetBundleName, 0);
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
    auto ret = upms->GrantUriPermissionPrivileged(uris, flag, targetBundleName, 0);
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
    MyFlag::tokenInfos[2001] = TokenInfo(2001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app2001", "com.example.app2001");
    std::string uri = "file://com.example.app2001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    uint32_t flag = 1;
    uint32_t tokenId = 2001;
    auto res = upms->CheckUriAuthorization(uris, flag, tokenId);
    std::vector<bool> expectRes(1, true);
    EXPECT_EQ(res, expectRes);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: check batch uri authorization success.
*/
HWTEST_F(UriPermissionImplTest, CheckUriAuthorization_006, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    MyFlag::tokenInfos[2001] = TokenInfo(2001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app2001", "com.example.app2001");
    const std::vector<std::string> uris = {
        "invalid://test_001.txt",
        "content://test_001.txt",
        "file://docs/test_001.txt",
        "file://media/Photo/1/IMG_001/test_001.jpg",
        "file://com.example.app2001/batchuri_001.txt",
        "file://com.example.app2002/batchuri_001.txt",
        "file://com.example.app2003/batchuri_001.txt",
    };
    TAG_LOGI(AAFwkTag::URIPERMMGR, "size of uris is %{public}zu.", uris.size());
    uint32_t flag = 1;
    uint32_t tokenId = 2001;
    auto res = upms->CheckUriAuthorization(uris, flag, tokenId);
    std::vector<bool> expectRes = { false, false, false, false, true, false, false };
    EXPECT_EQ(res, expectRes);
    
    // docs uri permission
    MyFlag::permissionFileAccessManager_ = true;
    res = upms->CheckUriAuthorization(uris, flag, tokenId);
    expectRes = { false, false, true, false, true, true, true };
    EXPECT_EQ(res, expectRes);
    MyFlag::permissionFileAccessManager_ = false;

    // media uri permission
    MyFlag::permissionAllMedia_ = true;
    res = upms->CheckUriAuthorization(uris, flag, tokenId);
    expectRes = { false, false, false, true, true, false, false };
    EXPECT_EQ(res, expectRes);
    MyFlag::permissionAllMedia_ = false;

    // content uri permission
    uint32_t callerTokenId = 1001;
    MyFlag::tokenInfos[callerTokenId] = TokenInfo(tokenId, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    IPCSkeleton::callerTokenId = callerTokenId;
    res = upms->CheckUriAuthorization(uris, flag, tokenId);
    expectRes = { false, true, false, false, true, false, false };
    EXPECT_EQ(res, expectRes);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: BatchUri::Init
 * SubFunction: NA
 * FunctionPoints: uriVec is empty, init failed.
*/
HWTEST_F(UriPermissionImplTest, UPMS_BatchUriTest_Init_001, TestSize.Level1)
{
    std::string callerBundleName = "com.example.testA";
    std::string targetBundleName = "com.example.testB";
    bool mode = 0;
    std::vector<Uri> uriVec;
    BatchUri batchUri;
    auto validCount = batchUri.Init(uriVec, mode, callerBundleName, targetBundleName);
    // invalid uri
    EXPECT_EQ(validCount, 0);
    // total uri
    EXPECT_EQ(batchUri.totalUriCount, 0);
    // content uri
    EXPECT_EQ(batchUri.contentUris.size(), 0);
    EXPECT_EQ(batchUri.contentIndexs.size(), 0);
    // media uri
    EXPECT_EQ(batchUri.mediaUris.size(), 0);
    EXPECT_EQ(batchUri.mediaIndexs.size(), 0);
    // other uri
    EXPECT_EQ(batchUri.otherIndexs.size(), 0);
    EXPECT_EQ(batchUri.otherUris.size(), 0);
    // docs uri
    EXPECT_EQ(batchUri.isDocsUriVec.size(), 0);
    // targetBundle
    EXPECT_EQ(batchUri.targetBundleUriCount, 0);
    // selfBundlePolicyInfos
    EXPECT_EQ(batchUri.selfBundlePolicyInfos.size(), 0);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: BatchUri::Init
 * SubFunction: NA
 * FunctionPoints: Init with mode is 0.
*/
HWTEST_F(UriPermissionImplTest, UPMS_BatchUriTest_Init_002, TestSize.Level1)
{
    std::string callerBundleName = "com.example.testA";
    std::string targetBundleName = "com.example.testB";
    bool mode = 0;
    std::vector<Uri> uriVec = {
        // invalid
        Uri("invalid://batchuri_001.txt"),
        // content
        Uri("content://batchuri_001.txt"),
        // docs
        Uri("file://docs/batchuri_001.txt"),
        // media
        Uri("file://media/Photo/1/IMG_001/test_001.jpg"),
        // caller
        Uri("file://com.example.testA/batchuri_001.txt"),
        // target
        Uri("file://com.example.testB/batchuri_001.txt"),
        // other
        Uri("file://com.example.testC/batchuri_001.text")
    };
    BatchUri batchUri;
    auto validCount = batchUri.Init(uriVec, mode, callerBundleName, targetBundleName);
    // invalid uri
    EXPECT_EQ(validCount, 6);
    // total uri
    EXPECT_EQ(batchUri.totalUriCount, 7);
    // content uri
    EXPECT_EQ(batchUri.contentUris.size(), 1);
    EXPECT_EQ(batchUri.contentIndexs.size(), 1);
    // media uri
    EXPECT_EQ(batchUri.mediaUris.size(), 1);
    EXPECT_EQ(batchUri.mediaIndexs.size(), 1);
    // other uri
    EXPECT_EQ(batchUri.otherIndexs.size(), 3);
    EXPECT_EQ(batchUri.otherUris.size(), 3);
    // docs uri
    EXPECT_EQ(batchUri.isDocsUriVec[2], true);
    // targetBundle
    EXPECT_EQ(batchUri.targetBundleUriCount, 0);
    // selfBundlePolicyInfos
    EXPECT_EQ(batchUri.selfBundlePolicyInfos.size(), 0);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: BatchUri::Init
 * SubFunction: NA
 * FunctionPoints: Init with mode is 1.
*/
HWTEST_F(UriPermissionImplTest, UPMS_BatchUriTest_Init_003, TestSize.Level1)
{
    std::string callerBundleName = "com.example.testA";
    std::string targetBundleName = "com.example.testB";
    bool mode = 1;
    std::vector<Uri> uriVec = {
        // invalid
        Uri("invalid://batchuri_001.txt"),
        // content
        Uri("content://batchuri_001.txt"),
        // docs
        Uri("file://docs/batchuri_001.txt"),
        // media
        Uri("file://media/batchuri_001.txt"),
        // caller
        Uri("file://com.example.testA/batchuri_001.txt"),
        // target
        Uri("file://com.example.testB/batchuri_001.txt"),
        // other
        Uri("file://com.example.testC/batchuri_001.text")
    };
    BatchUri batchUri;
    auto validCount = batchUri.Init(uriVec, mode, callerBundleName, targetBundleName);
    // invalid uri
    EXPECT_EQ(validCount, 6);
    // total uri
    EXPECT_EQ(batchUri.totalUriCount, 7);
    // content uri
    EXPECT_EQ(batchUri.contentUris.size(), 1);
    EXPECT_EQ(batchUri.contentIndexs.size(), 1);
    // media uri
    EXPECT_EQ(batchUri.mediaUris.size(), 1);
    EXPECT_EQ(batchUri.mediaIndexs.size(), 1);
    // other uri
    EXPECT_EQ(batchUri.otherIndexs.size(), 3);
    EXPECT_EQ(batchUri.otherUris.size(), 3);
    // docs uri
    EXPECT_EQ(batchUri.isDocsUriVec[2], true);
    // targetBundle
    EXPECT_EQ(batchUri.targetBundleUriCount, 0);
    // selfBundlePolicyInfos
    EXPECT_EQ(batchUri.selfBundlePolicyInfos.size(), 1);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: BatchUri::Init
 * SubFunction: NA
 * FunctionPoints: Init with mode is 1 and 500 uris
*/
HWTEST_F(UriPermissionImplTest, UPMS_BatchUriTest_Init_004, TestSize.Level1)
{
    std::string callerBundleName = "com.example.testA";
    std::string targetBundleName = "com.example.testB";
    bool mode = 1;
    std::vector<Uri> uriVec;
    auto batchSize = 75;
    for (int i = 0; i < batchSize; i++) {
        uriVec.emplace_back("invalid://batchuri_001.txt");
        uriVec.emplace_back("content://batchuri_001.txt");
        uriVec.emplace_back("file://docs/batchuri_001.txt");
        uriVec.emplace_back("file://media/batchuri_001.txt");
        uriVec.emplace_back("file://com.example.testA/batchuri_001.txt");
        uriVec.emplace_back("file://com.example.testB/batchuri_001.txt");
        uriVec.emplace_back("file://com.example.testC/batchuri_001.text");
    }
    BatchUri batchUri;
    auto validCount = batchUri.Init(uriVec, mode, callerBundleName, targetBundleName);
    // invalid uri
    EXPECT_EQ(validCount, 6 * batchSize);
    // total uri
    EXPECT_EQ(batchUri.totalUriCount, 7 * batchSize);
    // content uri
    EXPECT_EQ(batchUri.contentUris.size(), 1 * batchSize);
    EXPECT_EQ(batchUri.contentIndexs.size(), 1 * batchSize);
    // media uri
    EXPECT_EQ(batchUri.mediaUris.size(), 1 * batchSize);
    EXPECT_EQ(batchUri.mediaIndexs.size(), 1 * batchSize);
    // other uri
    EXPECT_EQ(batchUri.otherIndexs.size(), 3 * batchSize);
    EXPECT_EQ(batchUri.otherUris.size(), 3 * batchSize);
    // docs uri
    EXPECT_EQ(batchUri.isDocsUriVec[2], true);
    // targetBundle
    EXPECT_EQ(batchUri.targetBundleUriCount, 0 * batchSize);
    // selfBundlePolicyInfos
    EXPECT_EQ(batchUri.selfBundlePolicyInfos.size(), 1 * batchSize);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: BatchUri::SetContentUriCheckResult
 * SubFunction: NA
 * FunctionPoints: SetContentUriCheckResult.
*/
HWTEST_F(UriPermissionImplTest, UPMS_BatchUriTest_SetCheckResult_001, TestSize.Level1)
{
    std::string callerBundleName = "com.example.testA";
    std::string targetBundleName = "com.example.testB";
    bool mode = 1;
    std::vector<Uri> uriVec = {
        // invalid
        Uri("invalid://batchuri_001.txt"),
        // content
        Uri("content://batchuri_001.txt"),
        // docs
        Uri("file://docs/batchuri_001.txt"),
        // media
        Uri("file://media/batchuri_001.txt"),
        // caller
        Uri("file://com.example.testA/batchuri_001.txt"),
        // target
        Uri("file://com.example.testB/batchuri_001.txt"),
        // other
        Uri("file://com.example.testC/batchuri_001.text")
    };
    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerBundleName, targetBundleName);
    EXPECT_EQ(batchUri.contentIndexs.size(), 1);

    std::vector<bool> contentUriResult = { true };
    batchUri.SetContentUriCheckResult(contentUriResult);
    EXPECT_EQ(batchUri.result[1], true);

    contentUriResult = { false };
    batchUri.SetContentUriCheckResult(contentUriResult);
    EXPECT_EQ(batchUri.result[1], false);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: BatchUri::SetMediaUriCheckResult
 * SubFunction: NA
 * FunctionPoints: SetMediaUriCheckResult.
*/
HWTEST_F(UriPermissionImplTest, UPMS_BatchUriTest_SetCheckResult_002, TestSize.Level1)
{
    std::string callerBundleName = "com.example.testA";
    std::string targetBundleName = "com.example.testB";
    bool mode = 1;
    std::vector<Uri> uriVec = {
        // invalid
        Uri("invalid://batchuri_001.txt"),
        // content
        Uri("content://batchuri_001.txt"),
        // docs
        Uri("file://docs/batchuri_001.txt"),
        // media
        Uri("file://media/batchuri_001.txt"),
        // caller
        Uri("file://com.example.testA/batchuri_001.txt"),
        // target
        Uri("file://com.example.testB/batchuri_001.txt"),
        // other
        Uri("file://com.example.testC/batchuri_001.text")
    };
    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerBundleName, targetBundleName);
    EXPECT_EQ(batchUri.mediaIndexs.size(), 1);

    std::vector<bool> mediaUriResult = { true };
    batchUri.SetMediaUriCheckResult(mediaUriResult);
    EXPECT_EQ(batchUri.result[3], true);

    mediaUriResult = { false };
    batchUri.SetMediaUriCheckResult(mediaUriResult);
    EXPECT_EQ(batchUri.result[3], false);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: BatchUri::SetOtherUriCheckResult
 * SubFunction: NA
 * FunctionPoints: SetOtherUriCheckResult.
*/
HWTEST_F(UriPermissionImplTest, UPMS_BatchUriTest_SetCheckResult_003, TestSize.Level1)
{
    std::string callerBundleName = "com.example.testA";
    std::string targetBundleName = "com.example.testB";
    bool mode = 1;
    std::vector<Uri> uriVec = {
        // invalid
        Uri("invalid://batchuri_001.txt"),
        // content
        Uri("content://batchuri_001.txt"),
        // docs
        Uri("file://docs/batchuri_001.txt"),
        // media
        Uri("file://media/batchuri_001.txt"),
        // caller
        Uri("file://com.example.testA/batchuri_001.txt"),
        // target
        Uri("file://com.example.testB/batchuri_001.txt"),
        // other
        Uri("file://com.example.testC/batchuri_001.text")
    };
    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerBundleName, targetBundleName);
    EXPECT_EQ(batchUri.otherIndexs.size(), 3);
    EXPECT_EQ(batchUri.result.size(), 7);

    std::vector<bool> otherUriResult = { true, true, true };
    batchUri.SetOtherUriCheckResult(otherUriResult);
    EXPECT_EQ(batchUri.result[2], true);
    EXPECT_EQ(batchUri.result[5], true);
    EXPECT_EQ(batchUri.result[6], true);
    EXPECT_EQ(batchUri.targetBundleUriCount, 1);
    
    otherUriResult = { false, false, false};
    batchUri.targetBundleUriCount = 0;
    batchUri.SetOtherUriCheckResult(otherUriResult);
    EXPECT_EQ(batchUri.result[2], false);
    EXPECT_EQ(batchUri.result[5], false);
    EXPECT_EQ(batchUri.result[6], false);
    EXPECT_EQ(batchUri.targetBundleUriCount, 0);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: BatchUri::GetNeedCheckProxyPermissionURI
 * SubFunction: NA
 * FunctionPoints: all uri is permissioned.
*/
HWTEST_F(UriPermissionImplTest, UPMS_BatchUriTest_GetProxyUri_001, TestSize.Level1)
{
    std::string callerBundleName = "com.example.testA";
    std::string targetBundleName = "com.example.testB";
    bool mode = 1;
    std::vector<Uri> uriVec = {
        // invalid
        Uri("invalid://batchuri_001.txt"),
        // content
        Uri("content://batchuri_001.txt"),
        // docs
        Uri("file://docs/batchuri_001.txt"),
        // media
        Uri("file://media/batchuri_001.txt"),
        // caller
        Uri("file://com.example.testA/batchuri_001.txt"),
        // target
        Uri("file://com.example.testB/batchuri_001.txt"),
        // other
        Uri("file://com.example.testC/batchuri_001.text")
    };
    BatchUri batchUri;
    auto validCount = batchUri.Init(uriVec, mode, callerBundleName, targetBundleName);
    EXPECT_EQ(validCount, 6);

    PolicyInfo policyInfo1, policyInfo2;
    batchUri.otherPolicyInfos = { policyInfo1, policyInfo2 };
    // all is permissioned
    std::vector<PolicyInfo> proxyUrisByPolicy;
    std::vector<Uri> proxyUrisByMap;
    batchUri.result = { false, true, true, true, true, true, true };
    batchUri.GetNeedCheckProxyPermissionURI(proxyUrisByPolicy, proxyUrisByMap);
    EXPECT_EQ(proxyUrisByPolicy.size(), 0);
    EXPECT_EQ(proxyUrisByMap.size(), 0);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: BatchUri::GetNeedCheckProxyPermissionURI
 * SubFunction: NA
 * FunctionPoints: all uri is not permissioned.
*/
HWTEST_F(UriPermissionImplTest, UPMS_BatchUriTest_GetProxyUri_002, TestSize.Level1)
{
    std::string callerBundleName = "com.example.testA";
    std::string targetBundleName = "com.example.testB";
    bool mode = 1;
    std::vector<Uri> uriVec = {
        // invalid
        Uri("invalid://batchuri_001.txt"),
        // content
        Uri("content://batchuri_001.txt"),
        // docs
        Uri("file://docs/batchuri_001.txt"),
        // media
        Uri("file://media/batchuri_001.txt"),
        // caller
        Uri("file://com.example.testA/batchuri_001.txt"),
        // target
        Uri("file://com.example.testB/batchuri_001.txt"),
        // other
        Uri("file://com.example.testC/batchuri_001.text")
    };
    BatchUri batchUri;
    auto validCount = batchUri.Init(uriVec, mode, callerBundleName, targetBundleName);
    EXPECT_EQ(validCount, 6);

    PolicyInfo policyInfo1, policyInfo2;
    batchUri.otherPolicyInfos = { policyInfo1, policyInfo2 };
    
    std::vector<PolicyInfo> proxyUrisByPolicy;
    std::vector<Uri> proxyUrisByMap;
    batchUri.result = { false, false, false, false, true, true, false };
    batchUri.GetNeedCheckProxyPermissionURI(proxyUrisByPolicy, proxyUrisByMap);
    EXPECT_EQ(proxyUrisByPolicy.size(), 2);
    EXPECT_EQ(proxyUrisByMap.size(), 1);
    
    EXPECT_EQ(batchUri.proxyIndexsByPolicy[0], 2);
    EXPECT_EQ(batchUri.proxyIndexsByPolicy[1], 6);
    EXPECT_EQ(batchUri.proxyIndexsByMap[0], 3);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: BatchUri::GetNeedCheckProxyPermissionURI
 * SubFunction: NA
 * FunctionPoints: specified uri is permissioned.
*/
HWTEST_F(UriPermissionImplTest, UPMS_BatchUriTest_GetProxyUri_003, TestSize.Level1)
{
    std::string callerBundleName = "com.example.testA";
    std::string targetBundleName = "com.example.testB";
    bool mode = 1;
    std::vector<Uri> uriVec = {
        // invalid
        Uri("invalid://batchuri_001.txt"),
        // content
        Uri("content://batchuri_001.txt"),
        // docs
        Uri("file://docs/batchuri_001.txt"),
        // media
        Uri("file://media/batchuri_001.txt"),
        // caller
        Uri("file://com.example.testA/batchuri_001.txt"),
        // target
        Uri("file://com.example.testB/batchuri_001.txt"),
        // other
        Uri("file://com.example.testC/batchuri_001.text")
    };
    BatchUri batchUri;
    auto validCount = batchUri.Init(uriVec, mode, callerBundleName, targetBundleName);
    EXPECT_EQ(validCount, 6);

    PolicyInfo policyInfo1, policyInfo2;
    batchUri.otherPolicyInfos = { policyInfo1, policyInfo2 };
    
    std::vector<PolicyInfo> proxyUrisByPolicy;
    std::vector<Uri> proxyUrisByMap;
    // docs and other bundle is false
    batchUri.result = { false, true, false, true, true, true, false };
    batchUri.GetNeedCheckProxyPermissionURI(proxyUrisByPolicy, proxyUrisByMap);
    EXPECT_EQ(proxyUrisByPolicy.size(), 2);
    EXPECT_EQ(proxyUrisByMap.size(), 0);

    // media is false
    proxyUrisByPolicy.clear();
    proxyUrisByMap.clear();
    batchUri.result = { false, true, true, false, true, true, true };
    batchUri.GetNeedCheckProxyPermissionURI(proxyUrisByPolicy, proxyUrisByMap);
    EXPECT_EQ(proxyUrisByPolicy.size(), 0);
    EXPECT_EQ(proxyUrisByMap.size(), 1);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: SetProxyResult
 * SubFunction: NA
 * FunctionPoints: SetProxyResult
*/
HWTEST_F(UriPermissionImplTest, UPMS_BatchUriTest_SetCheckProxyResult_001, TestSize.Level1)
{
    std::string callerBundleName = "com.example.testA";
    std::string targetBundleName = "com.example.testB";
    bool mode = 1;
    std::vector<Uri> uriVec = {
        // invalid
        Uri("invalid://batchuri_001.txt"),
        // content
        Uri("content://batchuri_001.txt"),
        // docs
        Uri("file://docs/batchuri_001.txt"),
        // media
        Uri("file://media/batchuri_001.txt"),
        // caller
        Uri("file://com.example.testA/batchuri_001.txt"),
        // target
        Uri("file://com.example.testB/batchuri_001.txt"),
        // other
        Uri("file://com.example.testC/batchuri_001.text")
    };
    BatchUri batchUri;
    auto validCount = batchUri.Init(uriVec, mode, callerBundleName, targetBundleName);
    EXPECT_EQ(validCount, 6);

    std::vector<bool> proxyResultByPolicy = {true, true};
    batchUri.proxyIndexsByPolicy = {2, 6};
    batchUri.SetCheckProxyByPolicyResult(proxyResultByPolicy);
    EXPECT_EQ(batchUri.result[2], true);
    EXPECT_EQ(batchUri.result[6], true);

    std::vector<bool> proxyResultByMap = {true};
    batchUri.proxyIndexsByMap = { 3 };
    batchUri.SetCheckProxyByMapResult(proxyResultByMap);
    EXPECT_EQ(batchUri.result[3], true);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: BatchUri::GetUriToGrantByMap and GetUriToGrantByPolicy
 * SubFunction: NA
 * FunctionPoints: GetUriToGrantByMap and GetUriToGrantByPolicy
*/
HWTEST_F(UriPermissionImplTest, UPMS_BatchUriTest_GetGrantUri_001, TestSize.Level1)
{
    std::string callerBundleName = "com.example.testA";
    std::string targetBundleName = "com.example.testB";
    bool mode = 1;
    std::vector<Uri> uriVec = {
        // invalid
        Uri("invalid://batchuri_001.txt"),
        // content
        Uri("content://batchuri_001.txt"),
        // docs
        Uri("file://docs/batchuri_001.txt"),
        // media
        Uri("file://media/batchuri_001.txt"),
        // caller
        Uri("file://com.example.testA/batchuri_001.txt"),
        // target
        Uri("file://com.example.testB/batchuri_001.txt"),
        // other
        Uri("file://com.example.testC/batchuri_001.text")
    };
    BatchUri batchUri;
    auto validCount = batchUri.Init(uriVec, mode, callerBundleName, targetBundleName);
    EXPECT_EQ(validCount, 6);
    
    // all is ok
    PolicyInfo policyInfo1, policyInfo2;
    batchUri.otherPolicyInfos = { policyInfo1, policyInfo2 };
    batchUri.selfBundlePolicyInfos = { policyInfo1 };
    batchUri.result = { false, true, true, true, true, true, true };
    std::vector<std::string> uriStrVec;
    EXPECT_EQ(batchUri.GetUriToGrantByMap(uriStrVec), 2);

    std::vector<PolicyInfo> docsPolicyInfoVec, bundlePolicyInfoVec;
    EXPECT_EQ(batchUri.GetUriToGrantByPolicy(docsPolicyInfoVec, bundlePolicyInfoVec), 2);
    EXPECT_EQ(docsPolicyInfoVec.size(), 1);
    EXPECT_EQ(bundlePolicyInfoVec.size(), 1);
    
    // by map failed.
    batchUri.result = { false, false, true, false, true, true, true };
    uriStrVec.clear();
    EXPECT_EQ(batchUri.GetUriToGrantByMap(uriStrVec), 0);

    // by policy failed.
    docsPolicyInfoVec.clear();
    bundlePolicyInfoVec.clear();
    batchUri.otherPolicyInfos = { policyInfo1, policyInfo2 };
    batchUri.selfBundlePolicyInfos = { policyInfo1 };
    batchUri.result = { false, true, false, true, true, true, false };
    EXPECT_EQ(batchUri.GetUriToGrantByPolicy(docsPolicyInfoVec, bundlePolicyInfoVec), 1);
    EXPECT_EQ(docsPolicyInfoVec.size(), 0);
    EXPECT_EQ(bundlePolicyInfoVec.size(), 1);
}

}  // namespace AAFwk
}  // namespace OHOS
