/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <dlfcn.h>
#include <gtest/gtest.h>
#include <mutex>
#include <unordered_map>
#define private public
#define protected public
#include "connect_server_manager.h"
#undef private
#undef protected
#include "hilog_tag_wrapper.h"
using namespace testing::ext;
using namespace testing;
using namespace OHOS::AbilityRuntime;
namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t ONE = 1;
constexpr int32_t TWO = 2;
}
class ConnectServerManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/*
 * @tc.number    : ConnectServerManagerTest_0100
 * @tc.name      : ConnectServerManager
 * @tc.desc      : Test Function ConnectServerManager::Get and ConnectServerManager::~ConnectServerManager
 */
HWTEST_F(ConnectServerManagerTest, ConnectServerManagerTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0100 is start");
    std::shared_ptr<ConnectServerManager> connectServerManager = std::make_shared<ConnectServerManager>();
    EXPECT_TRUE(connectServerManager != nullptr);
    connectServerManager.reset();
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0100 is end");
}

/*
 * @tc.number    : ConnectServerManagerTest_0200
 * @tc.name      : ConnectServerManager
 * @tc.desc      : Test Function ConnectServerManager::StartConnectServer
 */
HWTEST_F(ConnectServerManagerTest, ConnectServerManagerTest_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0200 is start");
    ConnectServerManager &connectServerManager = AbilityRuntime::ConnectServerManager::Get();
    const std::string bundleName = "StartServer";
    uint32_t socketFd = 0;
    connectServerManager.StartConnectServer(bundleName, socketFd, true);
    EXPECT_TRUE(connectServerManager.bundleName_ == "StartServer");
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0200 is end");
}

/*
 * @tc.number    : ConnectServerManagerTest_0300
 * @tc.name      : ConnectServerManager
 * @tc.desc      : Test Function ConnectServerManager::StopConnectServer
 */
HWTEST_F(ConnectServerManagerTest, ConnectServerManagerTest_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0300 is start");
    ConnectServerManager &connectServerManager = AbilityRuntime::ConnectServerManager::Get();
    connectServerManager.StopConnectServer();
    EXPECT_FALSE(connectServerManager.handlerConnectServerSo_);
    connectServerManager.handlerConnectServerSo_ = nullptr;
    connectServerManager.StopConnectServer();
    EXPECT_FALSE(connectServerManager.handlerConnectServerSo_);
    char data[] = "StopServer";
    char *dptr = data;
    connectServerManager.handlerConnectServerSo_ = dptr;
    connectServerManager.StopConnectServer();
    EXPECT_FALSE(connectServerManager.handlerConnectServerSo_);
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0300 is end");
}

/*
 * @tc.number    : ConnectServerManagerTest_0400
 * @tc.name      : ConnectServerManager
 * @tc.desc      : Test Function ConnectServerManager::AddInstance
 */
HWTEST_F(ConnectServerManagerTest, ConnectServerManagerTest_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0400 is start");
    ConnectServerManager &connectServerManager = AbilityRuntime::ConnectServerManager::Get();
    auto createTest = [] (int32_t val) {};
    auto setTest = [] (bool flag) {};
    connectServerManager.SetLayoutInspectorCallback(createTest, setTest);
    const std::string instanceName = "test";
    connectServerManager.handlerConnectServerSo_ = nullptr;
    EXPECT_FALSE(connectServerManager.AddInstance(gettid(), ONE, instanceName));
    char data[] = "WaitForConnection";
    char *dptr = data;
    connectServerManager.handlerConnectServerSo_ = dptr;
    EXPECT_FALSE(connectServerManager.AddInstance(gettid(), ONE, instanceName));
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0400 is end");
}

/*
 * @tc.number    : ConnectServerManagerTest_0500
 * @tc.name      : ConnectServerManager
 * @tc.desc      : Test Function ConnectServerManager::RemoveInstance
 */
HWTEST_F(ConnectServerManagerTest, ConnectServerManagerTest_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0500 is start");
    ConnectServerManager &connectServerManager = AbilityRuntime::ConnectServerManager::Get();
    connectServerManager.handlerConnectServerSo_ = nullptr;
    EXPECT_FALSE(connectServerManager.handlerConnectServerSo_);
    connectServerManager.RemoveInstance(ONE);
    const std::string instanceName = "test";
    char data[] = "WaitForConnection";
    char *dptr = data;
    connectServerManager.handlerConnectServerSo_ = dptr;
    connectServerManager.instanceMap_.clear();
    auto res = connectServerManager.instanceMap_.try_emplace(ONE, instanceName, gettid());
    EXPECT_TRUE(res.second);
    connectServerManager.RemoveInstance(ONE);
    EXPECT_TRUE(connectServerManager.handlerConnectServerSo_);
    connectServerManager.RemoveInstance(ONE);
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0500 is end");
}

/*
 * @tc.number    : ConnectServerManagerTest_0600
 * @tc.name      : ConnectServerManager
 * @tc.desc      : Test Function ConnectServerManager::SendInspector
 */
HWTEST_F(ConnectServerManagerTest, ConnectServerManagerTest_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0600 is start");
    ConnectServerManager &connectServerManager = AbilityRuntime::ConnectServerManager::Get();
    const std::string jsonTreeStr = "jsonTreeStr";
    const std::string jsonSnapshotStr = "jsonSnapshotStr";
    connectServerManager.handlerConnectServerSo_ = nullptr;
    EXPECT_FALSE(connectServerManager.handlerConnectServerSo_);
    connectServerManager.SendInspector(jsonTreeStr, jsonSnapshotStr);
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0600 is end");
}

/*
 * @tc.number    : ConnectServerManagerTest_0700
 * @tc.name      : ConnectServerManager
 * @tc.desc      : Test Function ConnectServerManager::AddInstance
 */
HWTEST_F(ConnectServerManagerTest, ConnectServerManagerTest_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0700 is start");
    ConnectServerManager &connectServerManager = AbilityRuntime::ConnectServerManager::Get();
    auto createTest = [] (int32_t val) {};
    auto setTest = [] (bool flag) {};
    connectServerManager.SetLayoutInspectorCallback(createTest, setTest);
    const std::string instanceName = "test02";
    connectServerManager.handlerConnectServerSo_ = nullptr;
    const std::string bundleName = "StartServer";
    uint32_t socketFd = 0;
    connectServerManager.StartConnectServer(bundleName, socketFd, true);
    EXPECT_FALSE(connectServerManager.AddInstance(gettid(), TWO, instanceName));
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0700 is end");
}

/*
 * @tc.number    : ConnectServerManagerTest_0800
 * @tc.name      : ConnectServerManager
 * @tc.desc      : Test Function ConnectServerManager::RemoveInstance
 */
HWTEST_F(ConnectServerManagerTest, ConnectServerManagerTest_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0800 is start");
    ConnectServerManager &connectServerManager = AbilityRuntime::ConnectServerManager::Get();
    const std::string instanceName = "test02";
    connectServerManager.handlerConnectServerSo_ = nullptr;
    const std::string bundleName = "StartServer";
    uint32_t socketFd = 0;
    connectServerManager.StartConnectServer(bundleName, socketFd, true);
    connectServerManager.RemoveInstance(TWO);
    EXPECT_TRUE(connectServerManager.instanceMap_.find(TWO) == connectServerManager.instanceMap_.end());
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0800 is end");
}

/*
 * @tc.number    : ConnectServerManagerTest_0900
 * @tc.name      : ConnectServerManager
 * @tc.desc      : Test Function ConnectServerManager::SendInspector
 */
HWTEST_F(ConnectServerManagerTest, ConnectServerManagerTest_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0900 is start");
    ConnectServerManager &connectServerManager = AbilityRuntime::ConnectServerManager::Get();
    const std::string instanceName = "test02";
    connectServerManager.handlerConnectServerSo_ = nullptr;
    const std::string bundleName = "StartServer";
    uint32_t socketFd = 0;
    connectServerManager.StartConnectServer(bundleName, socketFd, true);
    const std::string jsonTreeStr = "jsonTreeStr";
    const std::string jsonSnapshotStr = "jsonSnapshotStr";
    connectServerManager.SendInspector(jsonTreeStr, jsonSnapshotStr);
    EXPECT_TRUE(connectServerManager.handlerConnectServerSo_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_0900 is end");
}

/*
 * @tc.number    : ConnectServerManagerTest_1000
 * @tc.name      : ConnectServerManager
 * @tc.desc      : Test Function ConnectServerManager::GetLayoutInspectorCallback
 */
HWTEST_F(ConnectServerManagerTest, ConnectServerManagerTest_1000, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_1000 is start");
    ConnectServerManager &connectServerManager = AbilityRuntime::ConnectServerManager::Get();
    connectServerManager.SetLayoutInspectorCallback(nullptr, nullptr);
    auto resulftSetTest = connectServerManager.GetLayoutInspectorCallback();
    EXPECT_TRUE(resulftSetTest == nullptr);
    
    auto createTest = [] (int32_t val) {};
    auto setTest = [] (bool flag) {};
    connectServerManager.SetLayoutInspectorCallback(createTest, setTest);
    resulftSetTest = connectServerManager.GetLayoutInspectorCallback();
    EXPECT_TRUE(resulftSetTest != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_1000 is end");
}

/*
 * @tc.number    : ConnectServerManagerTest_1100
 * @tc.name      : ConnectServerManager
 * @tc.desc      : Test Function ConnectServerManager::SetRecordCallback
 */
HWTEST_F(ConnectServerManagerTest, ConnectServerManagerTest_1100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_1100 is start");
    ConnectServerManager &connectServerManager = AbilityRuntime::ConnectServerManager::Get();
    auto startRecordFunc = []() {};
    auto stopRecordFunc = []() {};
    bool result = connectServerManager.SetRecordCallback(startRecordFunc, stopRecordFunc);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "ConnectServerManagerTest_1100 is end");
}
} // namespace AAFwk
} // namespace OHOS
