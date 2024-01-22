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

#include "js_quickfix_callback.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class JsQuickfixCallbackTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void JsQuickfixCallbackTest::SetUpTestCase()
{}

void JsQuickfixCallbackTest::TearDownTestCase()
{}

void JsQuickfixCallbackTest::SetUp()
{}

void JsQuickfixCallbackTest::TearDown()
{}

/**
 * @tc.name: JsQuickfixCallbackTest_0100
 * @tc.desc: JsQuickfixCallbackTest Test
 * @tc.type: FUNC
 */
HWTEST_F(JsQuickfixCallbackTest, JsQuickfixCallbackTest_0100, TestSize.Level0)
{
    std::string moudel = "<moudelName>";
    std::string hqfFile = "<hqfFile>";
    std::map<std::string, std::string> moduleAndPath;
    moduleAndPath.insert(std::make_pair(moudel, hqfFile));
    AbilityRuntime::JsQuickfixCallback jsQuickfixCallback(moduleAndPath);

    std::string baseFileName = "<baseFileName>";
    std::string patchFileName;
    void* patchBuffer = nullptr;
    size_t patchSize = 0;
    bool res = jsQuickfixCallback(baseFileName, patchFileName, &patchBuffer, patchSize);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: JsQuickfixCallbackTest_0200
 * @tc.desc: JsQuickfixCallbackTest Test
 * @tc.type: FUNC
 */
HWTEST_F(JsQuickfixCallbackTest, JsQuickfixCallbackTest_0200, TestSize.Level0)
{
    std::string moudel = "<moudelName>";
    std::string hqfFile = "<hqfFile>";
    std::map<std::string, std::string> moduleAndPath;
    moduleAndPath.insert(std::make_pair(moudel, hqfFile));
    AbilityRuntime::JsQuickfixCallback jsQuickfixCallback(moduleAndPath);

    std::string baseFileName = "baseFileName.abc";
    std::string patchFileName;
    void* patchBuffer = nullptr;
    size_t patchSize = 0;
    bool res = jsQuickfixCallback(baseFileName, patchFileName, &patchBuffer, patchSize);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: JsQuickfixCallbackTest_0300
 * @tc.desc: JsQuickfixCallbackTest Test
 * @tc.type: FUNC
 */
HWTEST_F(JsQuickfixCallbackTest, JsQuickfixCallbackTest_0300, TestSize.Level0)
{
    std::string moudel = "<moudelName>";
    std::string hqfFile = "<hqfFile>";
    std::map<std::string, std::string> moduleAndPath;
    moduleAndPath.insert(std::make_pair(moudel, hqfFile));
    AbilityRuntime::JsQuickfixCallback jsQuickfixCallback(moduleAndPath);

    std::string baseFileName = "/data/storage/el1/bundle/entry/ets/modules.abc";
    std::string patchFileName;
    void* patchBuffer = nullptr;
    size_t patchSize = 0;
    bool res = jsQuickfixCallback(baseFileName, patchFileName, &patchBuffer, patchSize);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: JsQuickfixCallbackTest_0400
 * @tc.desc: JsQuickfixCallbackTest Test
 * @tc.type: FUNC
 */
HWTEST_F(JsQuickfixCallbackTest, JsQuickfixCallbackTest_0400, TestSize.Level0)
{
    std::string moudel = "bundle";
    std::string hqfFile = "<hqfFile>";
    std::map<std::string, std::string> moduleAndPath;
    moduleAndPath.insert(std::make_pair(moudel, hqfFile));
    AbilityRuntime::JsQuickfixCallback jsQuickfixCallback(moduleAndPath);

    std::string baseFileName = "/data/storage/el1/bundle/bundle/ets/modules.abc";
    std::string patchFileName;
    void* patchBuffer = nullptr;
    size_t patchSize = 0;
    bool res = jsQuickfixCallback(baseFileName, patchFileName, &patchBuffer, patchSize);
    EXPECT_FALSE(res);
}
}  // namespace AAFwk
}  // namespace OHOS

