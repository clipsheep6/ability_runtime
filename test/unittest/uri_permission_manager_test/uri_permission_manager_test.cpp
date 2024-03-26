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
 * Function: RevokeAllUriPermissions
 * SubFunction: RevokeAllUriPermissions
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerClient RevokeAllUriPermissions
 */
HWTEST_F(UriPermissionManagerTest, RevokeAllUriPermissions_001, TestSize.Level1)
{
    // AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::string bundleName = "com.example.test";
    uint32_t targetTokenId = 100002;
    Uri uri(uriStr);
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
HWTEST_F(UriPermissionManagerTest, VerifyUriPermission_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int perReadFlag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    std::string bundleName = "com.example.test";
    uint32_t targetTokenId = 100002;
    Uri uri(uriStr);
    bool res = upmc.VerifyUriPermission(uri, perReadFlag, targetTokenId);
    EXPECT_EQ(res, false);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: RevokeUriPermissionManually
 * SubFunction: RevokeUriPermissionManually
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerClient RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionManagerTest, RevokeUriPermissionManually_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    auto uri = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::string targetBundleName = "com.example.app1002";
    auto ret = upmc.RevokeUriPermissionManually(uri, targetBundleName);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: RevokeUriPermissionManually
 * SubFunction: RevokeUriPermissionManually
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerClient RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionManagerTest, RevokeUriPermissionManually_002, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uriStrA = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_A.txt";
    std::string uriStrB = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_B.txt";
    std::vector<Uri> uriVec = { Uri(uriStrA), Uri(uriStrB) };
    std::string targetBundleName = "com.example.app1002";
    auto ret = upmc.RevokeUriPermissionManually(uriVec, targetBundleName);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: RevokeUriPermissionManually
 * SubFunction: RevokeUriPermissionManually
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerClient RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionManagerTest, RevokeUriPermissionManually_003, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    auto uri = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::vector<Uri> uriVec(501, uri);
    std::string targetBundleName = "com.example.app1002";
    auto ret = upmc.RevokeUriPermissionManually(uriVec, targetBundleName);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
}
}  // namespace AAFwk
}  // namespace OHOS