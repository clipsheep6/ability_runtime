/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <gtest/hwext/gtest-multithread.h>
#include <string>
#define private public
#define protected public
#include "cj_environment.h"
#include "cj_invoker.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;

namespace OHOS {

class CjEnvironmentTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void CjEnvironmentTest::SetUpTestCase() {}

void CjEnvironmentTest::TearDownTestCase() {}

void CjEnvironmentTest::SetUp() {}

void CjEnvironmentTest::TearDown() {}

void RegisterCJUncaughtExceptionHandlerTest(const CJUncaughtExceptionInfo &handle) {}

/**
 * @tc.name: CJEnvironment_GetInstance_0001
 * @tc.desc: JsRuntime test for UpdatePkgContextInfoJson.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, CJEnvironment_GetInstance_0100, TestSize.Level1)
{
    auto cJEnvironment = std::make_shared<CJEnvironment>();
    CJEnvironment *ret = nullptr;
    ret = cJEnvironment->GetInstance();
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: CJEnvironment_IsRuntimeStarted_0001
 * @tc.desc: JsRuntime test for IsRuntimeStarted.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, CJEnvironment_IsRuntimeStarted_0100, TestSize.Level1)
{
    auto cJEnvironment = std::make_shared<CJEnvironment>();
    bool ret = cJEnvironment->IsRuntimeStarted();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CJEnvironment_SetSanitizerKindRuntimeVersion_0001
 * @tc.desc: JsRuntime test for SetSanitizerKindRuntimeVersion.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, CJEnvironment_SetSanitizerKindRuntimeVersion_0100, TestSize.Level1)
{
    auto cJEnvironment = std::make_shared<CJEnvironment>();
    SanitizerKind kind = SanitizerKind::ASAN;

    cJEnvironment->SetSanitizerKindRuntimeVersion(kind);
    EXPECT_NE(cJEnvironment->sanitizerKind_, SanitizerKind::NONE);
}

/**
 * @tc.name: CJEnvironment_InitCJAppNS_0001
 * @tc.desc: JsRuntime test for InitCJAppNS.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, CJEnvironment_InitCJAppNS_0100, TestSize.Level1)
{
    auto cJEnvironment = std::make_shared<CJEnvironment>();
    std::string path = "ability_runtime/CjEnvironmentTest";

    cJEnvironment->InitCJAppNS(path);
    EXPECT_NE(cJEnvironment->cjAppNSName, nullptr);
}

/**
 * @tc.name: CJEnvironment_InitCJSDKNS_0001
 * @tc.desc: JsRuntime test for InitCJSDKNS.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, CJEnvironment_InitCJSDKNS_0100, TestSize.Level1)
{
    auto cJEnvironment = std::make_shared<CJEnvironment>();
    std::string path = "ability_runtime/CjEnvironmentTest";
    cJEnvironment->InitCJSDKNS(path);
    EXPECT_NE(cJEnvironment->cjAppNSName, nullptr);
}

/**
 * @tc.name: CJEnvironment_InitCJSysNS_0001
 * @tc.desc: JsRuntime test for InitCJSysNS.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, CJEnvironment_InitCJSysNS_0100, TestSize.Level1)
{
    auto cJEnvironment = std::make_shared<CJEnvironment>();
    std::string path = "ability_runtime/CjEnvironmentTest";
    cJEnvironment->InitCJSysNS(path);
    std::string getTempCjAppNSName = cJEnvironment->cjAppNSName;
    EXPECT_NE(cJEnvironment->cjAppNSName, nullptr);
}

/**
 * @tc.name: CJEnvironment_InitCJChipSDKNS_0001
 * @tc.desc: JsRuntime test for InitCJChipSDKNS.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, CJEnvironment_InitCJChipSDKNS_0100, TestSize.Level1)
{
    auto cJEnvironment = std::make_shared<CJEnvironment>();
    std::string path = "ability_runtime/CjEnvironmentTest";
    cJEnvironment->InitCJChipSDKNS(path);
    EXPECT_NE(cJEnvironment->cjAppNSName, nullptr);
}

/**
 * @tc.name: CJEnvironment_StartRuntime_0001
 * @tc.desc: JsRuntime test for StartRuntime.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, CJEnvironment_StartRuntime_0100, TestSize.Level1)
{
    auto cJEnvironment = std::make_shared<CJEnvironment>();

    bool ret = cJEnvironment->StartRuntime();
    EXPECT_EQ(ret, false);

    cJEnvironment->isRuntimeStarted_ = true;
    ret = cJEnvironment->StartRuntime();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CJEnvironment_StopRuntime_0001
 * @tc.desc: JsRuntime test for StopRuntime.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, CJEnvironment_StopRuntime_0100, TestSize.Level1)
{
    auto cJEnvironment = std::make_shared<CJEnvironment>();
    cJEnvironment->StopRuntime();
    EXPECT_EQ(cJEnvironment->isRuntimeStarted_, false);

    cJEnvironment->isUISchedulerStarted_ = true;
    EXPECT_EQ(cJEnvironment->isRuntimeStarted_, false);
}

/**
 * @tc.name: CJEnvironment_RegisterCJUncaughtExceptionHandler_0001
 * @tc.desc: JsRuntime test for RegisterCJUncaughtExceptionHandler.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, CJEnvironment_RegisterCJUncaughtExceptionHandler_0100, TestSize.Level1)
{
    // using RegisterUncaughtExceptionType = void (*)(const CJUncaughtExceptionInfo& handle);
    CJEnvironment cJEnvironment;
    CJUncaughtExceptionInfo handle;
    handle.hapPath = "/test1/";
    handle.uncaughtTask = [](const char* summary, const CJErrorObject errorObj) {};

    CJRuntimeAPI api {
        .InitCJRuntime = nullptr,
        .InitUIScheduler = nullptr,
        .RunUIScheduler = nullptr,
        .FiniCJRuntime = nullptr,
        .InitCJLibrary = nullptr,
        .RegisterEventHandlerCallbacks = nullptr,
        .RegisterCJUncaughtExceptionHandler = RegisterCJUncaughtExceptionHandlerTest,
    };

    CJEnvironment::lazyApis_ = api;

    cJEnvironment.RegisterCJUncaughtExceptionHandler(handle);

    EXPECT_NE(cJEnvironment.lazyApis_.RegisterCJUncaughtExceptionHandler, nullptr);
}

/**
 * @tc.name: CJEnvironment_IsUISchedulerStarted_0001
 * @tc.desc: JsRuntime test for IsUISchedulerStarted.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, CJEnvironment_IsUISchedulerStarted_0100, TestSize.Level1)
{
    auto cJEnvironment = std::make_shared<CJEnvironment>();
    bool ret = cJEnvironment->IsUISchedulerStarted();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: StartUIScheduler_0100
 * @tc.desc: Test when isUISchedulerStarted_ is true.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, StartUIScheduler_0100, TestSize.Level1)
{
    auto cjEnv = std::make_shared<CJEnvironment>();
    cjEnv->isUISchedulerStarted_ = true;
    auto res = cjEnv->StartUIScheduler();
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: StopUIScheduler_0100
 * @tc.desc: Test StopUIScheduler.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, StopUIScheduler_0100, TestSize.Level1)
{
    auto cjEnv = std::make_shared<CJEnvironment>();
    EXPECT_NE(cjEnv, nullptr);
    cjEnv->StopUIScheduler();
}

/**
 * @tc.name: LoadCJLibrary_0100
 * @tc.desc: Test LoadCJLibrary.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, LoadCJLibrary_0100, TestSize.Level1)
{
    auto cjEnv = std::make_shared<CJEnvironment>();
    char dlNames[5] = "Name";
    char* dlName = dlNames;
    auto res = cjEnv->LoadCJLibrary(dlName);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: LoadCJLibrary_0200
 * @tc.desc: Test LoadCJLibrary.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, LoadCJLibrary_0200, TestSize.Level1)
{
    auto cjEnv = std::make_shared<CJEnvironment>();
    CJEnvironment::LibraryKind kind = CJEnvironment::SYSTEM;
    char dlNames[] = "Name";
    char* dlName = dlNames;
    auto res = cjEnv->LoadCJLibrary(kind, dlName);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: UnLoadCJLibrary_0100
 * @tc.desc: Test LoadCJLibrary.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, UnLoadCJLibrary_0100, TestSize.Level1)
{
    auto cjEnv = std::make_shared<CJEnvironment>();
    EXPECT_NE(cjEnv, nullptr);
    cjEnv->UnLoadCJLibrary(nullptr);
}

/**
 * @tc.name: GetUIScheduler_0100
 * @tc.desc: Test GetUIScheduler.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, GetUIScheduler_0100, TestSize.Level1)
{
    auto cjEnv = std::make_shared<CJEnvironment>();
    cjEnv->isUISchedulerStarted_ = true;
    auto res = cjEnv->GetUIScheduler();
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: GetSymbol_0100
 * @tc.desc: Test GetSymbol.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, GetSymbol_0100, TestSize.Level1)
{
    auto cjEnv = std::make_shared<CJEnvironment>();
    EXPECT_NE(cjEnv, nullptr);
    void* dso = nullptr;
    char symbols[] = "symbol";
    char* symbol = symbols;
    auto res = cjEnv->GetSymbol(dso, symbol);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: StartDebugger_0100
 * @tc.desc: Test StartDebugger.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, StartDebugger_0100, TestSize.Level1)
{
    auto cjEnv = std::make_shared<CJEnvironment>();
    EXPECT_NE(cjEnv, nullptr);
    auto res = cjEnv->StartDebugger();
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: PostTask_0100
 * @tc.desc: Test PostTask.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, PostTask_0100, TestSize.Level1)
{
    auto cjEnv = std::make_shared<CJEnvironment>();
    TaskFuncType task = nullptr;
    auto res = cjEnv->PostTask(task);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: HasHigherPriorityTask_0100
 * @tc.desc: Test HasHigherPriorityTask.
 * @tc.type: FUNC
 */
HWTEST_F(CjEnvironmentTest, HasHigherPriorityTask_0100, TestSize.Level1)
{
    auto cjEnv = std::make_shared<CJEnvironment>();
    EXPECT_NE(cjEnv, nullptr);
    auto res = cjEnv->HasHigherPriorityTask();
    EXPECT_EQ(res, false);
}
} // namespace OHOS