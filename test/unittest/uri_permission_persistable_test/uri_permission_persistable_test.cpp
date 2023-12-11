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
#include "ability_manager_errors.h"
#include "event_report.h"
#include "mock_permission_verification.h"
#include "mock_native_token.h"
#include "system_ability_definition.h"
#include "system_ability_manager_client.h"
#include "tokenid_kit.h"
#define private public
#include "uri_permission_manager_stub_impl.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

namespace {
const std::string URI_PERMISSION_TABLE_NAME = "uri_permission";
}

class UriPermissionPersistableTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UriPermissionPersistableTest::SetUpTestCase()
{
    AppExecFwk::MockNativeToken::SetNativeToken();
}

void UriPermissionPersistableTest::TearDownTestCase() {}

void UriPermissionPersistableTest::SetUp() {}

void UriPermissionPersistableTest::TearDown() {}

/*
 * Feature: UriPermissionRdb
 * Function: AddGrantInfo
 * SubFunction: AddGrantInfo
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionRdb AddGrantInfo
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_001, TestSize.Level1)
{
    auto uprdb = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(uprdb, nullptr);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    uprdb->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    uint32_t ret = uprdb->AddGrantInfo(uriStr, flag, fromTokenId, targetTokenId);
    ASSERT_EQ(ret, ERR_OK);
}

/*
 * Feature: UriPermissionRdb
 * Function: AddGrantInfo
 * SubFunction: AddGrantInfo
 * FunctionPoints: NA
 * CaseDescription: UriPermissionRdb AddGrantInfo
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_002, TestSize.Level1)
{
    auto uprdb = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(uprdb, nullptr);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    uprdb->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int flag1 = Want::FLAG_AUTH_WRITE_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    unsigned int flag2 = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    RdbGrantInfo info = { uriStr, flag1, fromTokenId, targetTokenId };
    std::vector<RdbGrantInfo> rdbGrantInfoList = { info };
    uprdb->InsertData(rdbGrantInfoList);
    uint32_t ret = uprdb->AddGrantInfo(uriStr, flag2, fromTokenId, targetTokenId);
    ASSERT_EQ(ret, ERR_OK);
    int rowCount = 0;
    rdbGrantInfoList.clear();
    uprdb->QueryData(absRdbPredicates, rdbGrantInfoList, rowCount);
    bool condition = rowCount == 1 && rdbGrantInfoList[0].flag == flag2;
    ASSERT_EQ(condition, true);
}

/*
 * Feature: UriPermissionRdb
 * Function: AddGrantInfo
 * SubFunction: AddGrantInfo
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionRdb AddGrantInfo
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_003, TestSize.Level1)
{
    auto uprdb = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(uprdb, nullptr);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    uprdb->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int flag1 = Want::FLAG_AUTH_WRITE_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    unsigned int flag2 = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    RdbGrantInfo info1 = { uriStr, flag1, fromTokenId, targetTokenId };
    RdbGrantInfo info2 = { uriStr, flag2, fromTokenId, targetTokenId };
    std::vector<RdbGrantInfo> rdbGrantInfoList = { info1, info2 };
    uprdb->InsertData(rdbGrantInfoList);
    uint32_t ret = uprdb->AddGrantInfo(uriStr, flag2, fromTokenId, targetTokenId);
    ASSERT_EQ(ret, INNER_ERR);
}

/*
 * Feature: UriPermissionRdb
 * Function: AddGrantInfo
 * SubFunction: AddGrantInfo
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionRdb RemoveGrantInfo
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_004, TestSize.Level1)
{
    auto uprdb = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(uprdb, nullptr);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    uprdb->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    RdbGrantInfo info = { uriStr, flag, fromTokenId, targetTokenId };
    std::vector<RdbGrantInfo> rdbGrantInfoList = { info };
    uprdb->InsertData(rdbGrantInfoList);
    sptr<StorageManager::IStorageManager> storageManager = new StorageManager::StorageManagerServiceMock();
    uint32_t ret = uprdb->RemoveGrantInfo(uriStr, targetTokenId, storageManager);
    ASSERT_EQ(ret, ERR_OK);
    int rowCount = 0;
    uprdb->QueryData(absRdbPredicates, rdbGrantInfoList, rowCount);
    ASSERT_EQ(rowCount, 0);
}

/*
 * Feature: UriPermissionRdb
 * Function: AddGrantInfo
 * SubFunction: AddGrantInfo
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionRdb CheckPersistableUriPermissionProxy
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_005, TestSize.Level1)
{
    auto uprdb = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(uprdb, nullptr);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    uprdb->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int perReadFlag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    RdbGrantInfo info = { uriStr, perReadFlag, fromTokenId, targetTokenId };
    unsigned int flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::vector<RdbGrantInfo> rdbGrantInfoList = { info };
    uprdb->InsertData(rdbGrantInfoList);
    bool ret = uprdb->CheckPersistableUriPermissionProxy(uriStr, flag, targetTokenId);
    ASSERT_EQ(ret, true);
}

/*
 * Feature: UriPermissionRdb
 * Function: AddGrantInfo
 * SubFunction: AddGrantInfo
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionRdb CheckPersistableUriPermissionProxy
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_006, TestSize.Level1)
{
    auto uprdb = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(uprdb, nullptr);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    uprdb->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int perWriteFlag = Want::FLAG_AUTH_WRITE_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    RdbGrantInfo info = { uriStr, perWriteFlag, fromTokenId, targetTokenId };
    unsigned int flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::vector<RdbGrantInfo> rdbGrantInfoList = { info };
    uprdb->InsertData(rdbGrantInfoList);
    bool ret = uprdb->CheckPersistableUriPermissionProxy(uriStr, flag, targetTokenId);
    ASSERT_EQ(ret, true);
}

/*
 * Feature: UriPermissionRdb
 * Function: AddGrantInfo
 * SubFunction: AddGrantInfo
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionRdb CheckPersistableUriPermissionProxy
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_007, TestSize.Level1)
{
    auto uprdb = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(uprdb, nullptr);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    uprdb->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int perReadFlag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    RdbGrantInfo info = { uriStr, perReadFlag, fromTokenId, targetTokenId };
    unsigned int flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    std::vector<RdbGrantInfo> rdbGrantInfoList = { info };
    uprdb->InsertData(rdbGrantInfoList);
    bool ret = uprdb->CheckPersistableUriPermissionProxy(uriStr, flag, targetTokenId);
    ASSERT_EQ(ret, false);
}

/*
 * Feature: UriPermissionRdb
 * Function: AddGrantInfo
 * SubFunction: AddGrantInfo
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionRdb CheckPersistableUriPermissionProxy
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_008, TestSize.Level1)
{
    auto uprdb = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(uprdb, nullptr);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    uprdb->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int perWriteFlag = Want::FLAG_AUTH_WRITE_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    RdbGrantInfo info = { uriStr, perWriteFlag, fromTokenId, targetTokenId };
    unsigned int flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    std::vector<RdbGrantInfo> rdbGrantInfoList = { info };
    uprdb->InsertData(rdbGrantInfoList);
    bool ret = uprdb->CheckPersistableUriPermissionProxy(uriStr, flag, targetTokenId);
    ASSERT_EQ(ret, true);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: VerifyUriPermission
 * SubFunction: VerifyUriPermission
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerStubImpl VerifyUriPermission
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_009, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    upms->uriPermissionRdb_ = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(upms->uriPermissionRdb_, nullptr);
    upms->isGrantPersistableUriPermissionEnable_ = true;
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    bool res = upms->uriPermissionRdb_->DeleteData(absRdbPredicates);
    ASSERT_EQ(res, true);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int perReadFlag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    uint32_t ret = upms->uriPermissionRdb_->AddGrantInfo(uriStr, perReadFlag, fromTokenId, targetTokenId);
    ASSERT_EQ(ret, ERR_OK);
    Uri uri(uriStr);
    res = upms->VerifyUriPermission(uri, perReadFlag, targetTokenId);
    ASSERT_EQ(res, true);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: VerifyUriPermission
 * SubFunction: VerifyUriPermission
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerStubImpl VerifyUriPermission
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_010, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    upms->uriPermissionRdb_ = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(upms->uriPermissionRdb_, nullptr);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int tmpReadFlag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    int autoremove = 0;
    upms->uriMap_.clear();
    GrantInfo info = { tmpReadFlag, fromTokenId, targetTokenId, autoremove };
    std::list<GrantInfo> infoList = { info };
    upms->uriMap_.emplace(uriStr, infoList);
    Uri uri(uriStr);
    bool res = upms->VerifyUriPermission(uri, tmpReadFlag, targetTokenId);
    upms->uriMap_.clear();
    ASSERT_EQ(res, true);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: VerifyUriPermission
 * SubFunction: VerifyUriPermission
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerStubImpl VerifyUriPermission
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_011, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    upms->uriPermissionRdb_ = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(upms->uriPermissionRdb_, nullptr);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    upms->uriPermissionRdb_->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int tmpReadFlag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t targetTokenId = 100001;
    Uri uri(uriStr);
    bool res = upms->VerifyUriPermission(uri, tmpReadFlag, targetTokenId);
    ASSERT_EQ(res, false);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GetUriPermissionFlag
 * SubFunction: GetUriPermissionFlag
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerStubImpl GetUriPermissionFlag
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_012, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    upms->uriPermissionRdb_ = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(upms->uriPermissionRdb_, nullptr);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    upms->uriPermissionRdb_->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int perReadFlag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    upms->isGrantPersistableUriPermissionEnable_ = false;
    uint32_t newFlag = 0;
    Uri uri(uriStr);
    uint32_t ret = upms->GetUriPermissionFlag(uri, perReadFlag, fromTokenId, targetTokenId, newFlag);
    ASSERT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GetUriPermissionFlag
 * SubFunction: GetUriPermissionFlag
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerStubImpl GetUriPermissionFlag
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_013, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    upms->uriPermissionRdb_ = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(upms->uriPermissionRdb_, nullptr);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    upms->uriPermissionRdb_->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int perReadFlag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    upms->isGrantPersistableUriPermissionEnable_ = false;
    uint32_t newFlag = 0;
    MyFlag::flag_ = 1;
    Uri uri(uriStr);
    uint32_t ret = upms->GetUriPermissionFlag(uri, perReadFlag, fromTokenId, targetTokenId, newFlag);
    MyFlag::flag_ = 0;
    bool condition = ret == ERR_OK && newFlag == 1;
    ASSERT_EQ(condition, true);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GetUriPermissionFlag
 * SubFunction: GetUriPermissionFlag
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerStubImpl GetUriPermissionFlag
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_014, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    upms->uriPermissionRdb_ = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(upms->uriPermissionRdb_, nullptr);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    upms->uriPermissionRdb_->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int perReadFlag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    uint32_t tmpReadFlag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    upms->isGrantPersistableUriPermissionEnable_ = true;
    uint32_t ret = upms->uriPermissionRdb_->AddGrantInfo(uriStr, perReadFlag, fromTokenId, targetTokenId);
    ASSERT_EQ(ret, ERR_OK);
    uint32_t newFlag = 0;
    Uri uri(uriStr);
    ret = upms->GetUriPermissionFlag(uri, tmpReadFlag, fromTokenId, targetTokenId, newFlag);
    bool condition = ret == ERR_OK && newFlag == 0;
    ASSERT_EQ(condition, true);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GetUriPermissionFlag
 * SubFunction: GetUriPermissionFlag
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerStubImpl GetUriPermissionFlag
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_015, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    upms->uriPermissionRdb_ = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(upms->uriPermissionRdb_, nullptr);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    upms->uriPermissionRdb_->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int perReadFlag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    upms->isGrantPersistableUriPermissionEnable_ = true;
    uint32_t newFlag = 0;
    MyFlag::flag_ = 1;
    Uri uri(uriStr);
    uint32_t ret = upms->GetUriPermissionFlag(uri, perReadFlag, fromTokenId, targetTokenId, newFlag);
    MyFlag::flag_ = 0;
    bool condition = ret == ERR_OK && newFlag == 65;
    ASSERT_EQ(condition, true);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GetUriPermissionFlag
 * SubFunction: GetUriPermissionFlag
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerStubImpl GetUriPermissionFlag
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_016, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    upms->uriPermissionRdb_ = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(upms->uriPermissionRdb_, nullptr);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    upms->uriPermissionRdb_->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    uint32_t tmpReadFlag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    upms->isGrantPersistableUriPermissionEnable_ = true;
    uint32_t newFlag = 0;
    Uri uri(uriStr);
    uint32_t ret = upms->GetUriPermissionFlag(uri, tmpReadFlag, fromTokenId, targetTokenId, newFlag);
    ASSERT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GetUriPermissionFlag
 * SubFunction: GetUriPermissionFlag
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerStubImpl GetUriPermissionFlag
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_017, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    upms->uriPermissionRdb_ = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(upms->uriPermissionRdb_, nullptr);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    upms->uriPermissionRdb_->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int perReadFlag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    uint32_t tmpReadFlag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    upms->isGrantPersistableUriPermissionEnable_ = true;
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    uint32_t ret = upms->uriPermissionRdb_->AddGrantInfo(uriStr, perReadFlag, fromTokenId, callerTokenId);
    ASSERT_EQ(ret, ERR_OK);
    uint32_t newFlag = 0;
    Uri uri(uriStr);
    ret = upms->GetUriPermissionFlag(uri, tmpReadFlag, fromTokenId, targetTokenId, newFlag);
    bool condition = ret == ERR_OK && newFlag == 65;
    ASSERT_EQ(condition, true);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GetUriPermissionFlag
 * SubFunction: GetUriPermissionFlag
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerStubImpl GrantUriPermissionImpl
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_018, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    upms->uriPermissionRdb_ = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(upms->uriPermissionRdb_, nullptr);
    upms->storageManager_ = new StorageManager::StorageManagerServiceMock();
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    upms->uriPermissionRdb_->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int perReadFlag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    std::string targetBundleName = "com.example.test";
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    Uri uri(uriStr);
    uint32_t ret = upms->GrantUriPermissionImpl(uri, perReadFlag, fromTokenId, targetTokenId, 0);
    ASSERT_EQ(ret, ERR_OK);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: RevokeAllUriPermissions
 * SubFunction: RevokeAllUriPermissions
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerStubImpl RevokeAllUriPermissions
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_019, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    upms->uriPermissionRdb_ = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(upms->uriPermissionRdb_, nullptr);
    upms->storageManager_ = new StorageManager::StorageManagerServiceMock();
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    upms->uriPermissionRdb_->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    unsigned int perReadFlag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    uint32_t fromTokenId = 100001;
    uint32_t targetTokenId = 100002;
    upms->isGrantPersistableUriPermissionEnable_ = true;
    uint32_t ret = upms->uriPermissionRdb_->AddGrantInfo(uriStr, perReadFlag, fromTokenId, targetTokenId);
    ASSERT_EQ(ret, ERR_OK);
    ret = upms->RevokeAllUriPermissions(targetTokenId);
    ASSERT_EQ(ret, ERR_OK);
    std::vector<RdbGrantInfo> rdbGrantInfoList;
    int rowCount = 0;
    bool res = upms->uriPermissionRdb_->QueryData(absRdbPredicates, rdbGrantInfoList, rowCount);
    bool condition = res && rowCount == 0;
    ASSERT_EQ(condition, true);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: RevokeUriPermissionManually
 * SubFunction: RevokeUriPermissionManually
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerStubImpl RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_020, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    upms->uriPermissionRdb_ = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(upms->uriPermissionRdb_, nullptr);
    upms->storageManager_ = new StorageManager::StorageManagerServiceMock();
    upms->isGrantPersistableUriPermissionEnable_ = true;
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    upms->uriPermissionRdb_->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::string bundleName = "com.example.test";
    Uri uri(uriStr);
    uint32_t ret = upms->RevokeUriPermissionManually(uri, bundleName);
    ASSERT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: RevokeUriPermissionManually
 * SubFunction: RevokeUriPermissionManually
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerStubImpl RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionPersistableTest, UriPermissionPersistableTest_021, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    upms->uriPermissionRdb_ = std::make_shared<UriPermissionRdb>();
    ASSERT_NE(upms->uriPermissionRdb_, nullptr);
    upms->storageManager_ = new StorageManager::StorageManagerServiceMock();
    upms->isGrantPersistableUriPermissionEnable_ = true;
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    upms->uriPermissionRdb_->DeleteData(absRdbPredicates);
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::string bundleName = "com.example.test";
    MyFlag::flag_ = 1;
    Uri uri(uriStr);
    uint32_t ret = upms->RevokeUriPermissionManually(uri, bundleName);
    MyFlag::flag_ = 0;
    ASSERT_EQ(ret, ERR_OK);
}
}  // namespace AAFwk
}  // namespace OHOS
