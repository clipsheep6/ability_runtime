/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "application_context_manager.h"
#include "running_process_info.h"
#include "want.h"
#include "configuration_convertor.h"
#include "ability_manager_errors.h"
#include "js_runtime.h"
#include "ability_lifecycle_callback.h"
#include "event_runner.h"
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class ApplicationContextManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::unique_ptr<JsRuntime> Runtime_ = nullptr;
    napi_env napi_env_ = nullptr;
    ApplicationContextManager* AppMgr_ = nullptr;
};

void ApplicationContextManagerTest::SetUpTestCase(void)
{}

void ApplicationContextManagerTest::TearDownTestCase(void)
{}

void ApplicationContextManagerTest::SetUp()
{
    auto eventRunner = AppExecFwk::EventRunner::Create("ApplicationContextManagerTest");
    Runtime::Options options;
    options.preload = true;
    options.eventRunner = eventRunner;
    Runtime_ = JsRuntime::Create(options);
    napi_env_ = (static_cast<AbilityRuntime::JsRuntime&>(*Runtime_)).GetNapiEnv();
    AppMgr_ = &ApplicationContextManager::GetApplicationContextManager();
}

void ApplicationContextManagerTest::TearDown()
{}


/**
 * @tc.number: AddGlobalObject_0100
 * @tc.name: AddGlobalObject
 * @tc.desc: AddGlobalObject
 */
HWTEST_F(ApplicationContextManagerTest, AddGlobalObject_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddGlobalObject_0100 start";
    AppMgr_->AddGlobalObject(napi_env_, nullptr);
    GTEST_LOG_(INFO) << "AddGlobalObject_0100 end";
}

/**
 * @tc.number: GetGlobalObject_0100
 * @tc.name: GetGlobalObject
 * @tc.desc: GetGlobalObject Callback
 */
HWTEST_F(ApplicationContextManagerTest, GetGlobalObject_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetGlobalObject_0100 start";
    std::shared_ptr<NativeReference> ptrNRef = AppMgr_->GetGlobalObject(napi_env_);
    EXPECT_EQ(ptrNRef, nullptr);
    GTEST_LOG_(INFO) << "GetGlobalObject_0100 end";
}

/**
 * @tc.number: RemoveGlobalObject_0100
 * @tc.name: RemoveGlobalObject
 * @tc.desc: RemoveGlobalObject
 */
HWTEST_F(ApplicationContextManagerTest, RemoveGlobalObject_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplicationContextManager_0100 start";
    AppMgr_->RemoveGlobalObject(napi_env_);
    GTEST_LOG_(INFO) << "ApplicationContextManager_0100 end";
}
}
}