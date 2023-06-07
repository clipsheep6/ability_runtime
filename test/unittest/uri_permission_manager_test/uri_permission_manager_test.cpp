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
#include "uri_permission_manager_client.h"
#include "uri_permission_load_callback.h"
#undef private
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
    auto ret = upmc.ConnectUriPermService();
    EXPECT_TRUE(ret == nullptr);
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
    auto ret = upmc.ConnectUriPermService();
    EXPECT_TRUE(ret == nullptr);
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
    auto ret = upmc.ConnectUriPermService();
    EXPECT_TRUE(ret == nullptr);
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
}  // namespace AAFwk
}  // namespace OHOS