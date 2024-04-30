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

#define private public
#include "uri_permission_manager_client.h"
#include "uri_permission_load_callback.h"
#undef private
#include "ability_manager_errors.h"
#include "mock_sa_call.h"
#include "want.h"
using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace AAFwk {
class UriPermissionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UriPermissionManagerTest::SetUpTestCase() {}

void UriPermissionManagerTest::TearDownTestCase() {}

void UriPermissionManagerTest::SetUp() {}

void UriPermissionManagerTest::TearDown() {}

/*
 * Feature: UriPermissionManagerClient
 * Function: ConnectUriPermService
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerClient ConnectUriPermService
 */
HWTEST_F(UriPermissionManagerTest, ConnectUriPermService_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    upmc.saLoadFinished_ = true;
    EXPECT_EQ(upmc.GetUriPermMgr(), nullptr);
    auto ret = upmc.ConnectUriPermService();
}

/*
 * Feature: UriPermissionManagerClient
 * Function: ConnectUriPermService
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerClient ConnectUriPermService
 */
HWTEST_F(UriPermissionManagerTest, ConnectUriPermService_002, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    sptr<IRemoteObject> remoteObject = new (std::nothrow) UriPermissionLoadCallback();
    upmc.SetUriPermMgr(remoteObject);
    EXPECT_EQ(upmc.GetUriPermMgr(), nullptr);
    auto ret = upmc.ConnectUriPermService();
}

/*
 * Feature: UriPermissionManagerClient
 * Function: ConnectUriPermService
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerClient ConnectUriPermService
 */
HWTEST_F(UriPermissionManagerTest, ConnectUriPermService_003, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    sptr<IRemoteObject> remoteObject = nullptr;
    upmc.SetUriPermMgr(remoteObject);
    EXPECT_EQ(upmc.GetUriPermMgr(), nullptr);
    auto ret = upmc.ConnectUriPermService();
}

/*
 * Feature: UriPermissionManagerClient
 * Function: LoadUriPermService
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerClient LoadUriPermService
 */
HWTEST_F(UriPermissionManagerTest, LoadUriPermService_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    upmc.saLoadFinished_ = true;
    auto ret = upmc.LoadUriPermService();
    EXPECT_TRUE(ret);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermission
 * SubFunction: SingleGrantUriPermission
 * FunctionPoints: NA.
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermission
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_GrantUriPermission_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    auto uri = Uri("file://com.example.test1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::string bundleName = "com.example.test1001";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    auto ret = upmc.GrantUriPermission(uri, flag, bundleName, 0, 0);
    EXPECT_NE(ret, ERR_OK);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermission
 * SubFunction: BatchGrantUriPermission
 * FunctionPoints: Size of uris is 0
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermission
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_BatchGrantUriPermission_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::vector<Uri> uriVec;
    std::string bundleName = "com.example.test1001";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    auto ret = upmc.GrantUriPermission(uriVec, flag, bundleName, 0, 0);
    EXPECT_EQ(ret, ERR_URI_LIST_OUT_OF_RANGE);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermission
 * SubFunction: BatchGrantUriPermission
 * FunctionPoints: Size of uris is more than 500
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermission
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_BatchGrantUriPermission_002, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    Uri uri = Uri("file://com.example.test1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::vector<Uri> uriVec(501, uri);
    std::string bundleName = "com.example.test1001";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    auto ret = upmc.GrantUriPermission(uriVec, flag, bundleName, 0, 0);
    EXPECT_EQ(ret, ERR_URI_LIST_OUT_OF_RANGE);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermission
 * SubFunction: BatchGrantUriPermission
 * FunctionPoints: Size of uris is betweent 1 and 500
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermission
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_BatchGrantUriPermission_003, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    Uri uri = Uri("file://com.example.test1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::vector<Uri> uriVec(500, uri);
    std::string bundleName = "com.example.test1001";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    auto ret = upmc.GrantUriPermission(uriVec, flag, bundleName, 0, 0);
    EXPECT_NE(ret, ERR_OK);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: RevokeUriPermissionManually
 * SubFunction: RevokeUriPermissionManually
 * FunctionPoints: Uri is invalid.
 * CaseDescription: Verify UriPermissionManagerClient RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_RevokeUriPermissionManually_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    auto uri = Uri("http://com.example.test1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::string bundleName = "com.example.test1001";
    auto ret = upmc.RevokeUriPermissionManually(uri, bundleName);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: RevokeUriPermissionManually
 * SubFunction: RevokeUriPermissionManually
 * FunctionPoints: Uri is valid.
 * CaseDescription: Verify UriPermissionManagerClient RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_RevokeUriPermissionManually_002, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    auto uri = Uri("file://com.example.test1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::string bundleName = "com.example.test1001";
    auto ret = upmc.RevokeUriPermissionManually(uri, bundleName);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: RevokeUriPermission
 * SubFunction: RevokeUriPermission
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerClient RevokeUriPermission
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_RevokeUriPermission_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    auto uri = Uri("file://com.example.test1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    uint32_t targetTokenId = 1001;
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    upmc.RevokeUriPermission(targetTokenId);
    bool ret = upmc.VerifyUriPermission(uri, flag, targetTokenId);
    EXPECT_EQ(ret, false);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: RevokeAllUriPermissions
 * SubFunction: RevokeAllUriPermissions
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerClient RevokeAllUriPermissions
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_RevokeAllUriPermissions_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    uint32_t targetTokenId = 1001;
    auto ret = upmc.RevokeAllUriPermissions(targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: VerifyUriPermission
 * SubFunction: VerifyUriPermission
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerClient VerifyUriPermission
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_VerifyUriPermission_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string bundleName = "com.example.test";
    uint32_t targetTokenId = 100002;
    Uri uri(uriStr);
    bool res = upmc.VerifyUriPermission(uri, flag, targetTokenId);
    EXPECT_EQ(res, false);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: CheckUriAuthorization
 * SubFunction: CheckUriAuthorization
 * FunctionPoints: Size of uris is 0
 * CaseDescription: Verify UriPermissionManagerClient CheckUriAuthorization
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_CheckUriAuthorization_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::vector<std::string> uriVec;
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;
    auto res = upmc.CheckUriAuthorization(uriVec, flag, tokenId);
    std::vector<bool> expectRes;
    EXPECT_EQ(res, expectRes);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: CheckUriAuthorization
 * SubFunction: CheckUriAuthorization
 * FunctionPoints: Size of uris is between 1 and 500
 * CaseDescription: Verify UriPermissionManagerClient CheckUriAuthorization
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_CheckUriAuthorization_002, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::vector<std::string> uriVec(1, uriStr);
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;
    auto res = upmc.CheckUriAuthorization(uriVec, flag, tokenId);
    std::vector<bool> expectRes(1, false);
    EXPECT_EQ(res, expectRes);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: CheckUriAuthorization
 * SubFunction: CheckUriAuthorization
 * FunctionPoints: Size of uris is more than 500
 * CaseDescription: Verify UriPermissionManagerClient CheckUriAuthorization
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_CheckUriAuthorization_003, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::vector<std::string> uriVec(501, uriStr);
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;
    auto res = upmc.CheckUriAuthorization(uriVec, flag, tokenId);
    std::vector<bool> expectRes(501, false);
    EXPECT_EQ(res, expectRes);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermissionPrivileged
 * SubFunction: GrantUriPermissionPrivileged
 * FunctionPoints: Size of uris is 0
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermissionPrivileged
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_GrantUriPermissionPrivileged_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::vector<Uri> uriVec;
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string bundleName = "com.example.test1001";
    int32_t appIndex = 0;
    auto res = upmc.GrantUriPermissionPrivileged(uriVec, flag, bundleName, appIndex);
    EXPECT_EQ(res, ERR_URI_LIST_OUT_OF_RANGE);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermissionPrivileged
 * SubFunction: GrantUriPermissionPrivileged
 * FunctionPoints: Size of uris is more than 500
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermissionPrivileged
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_GrantUriPermissionPrivileged_002, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::vector<Uri> uriVec(501, Uri(uriStr));
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string bundleName = "com.example.test1001";
    int32_t appIndex = 0;
    auto res = upmc.GrantUriPermissionPrivileged(uriVec, flag, bundleName, appIndex);
    EXPECT_EQ(res, ERR_URI_LIST_OUT_OF_RANGE);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermissionPrivileged
 * SubFunction: GrantUriPermissionPrivileged
 * FunctionPoints: size of uri is between 1 and 500
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermissionPrivileged
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_GrantUriPermissionPrivileged_003, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::vector<Uri> uriVec(1, Uri(uriStr));
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string bundleName = "com.example.test1001";
    int32_t appIndex = 0;
    auto res = upmc.GrantUriPermissionPrivileged(uriVec, flag, bundleName, appIndex);
    EXPECT_EQ(res, CHECK_PERMISSION_FAILED);
}
}  // namespace AAFwk
}  // namespace OHOS