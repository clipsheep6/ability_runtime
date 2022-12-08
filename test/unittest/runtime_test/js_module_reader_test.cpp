/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "js_module_reader.h"
#include "extractor.h"

using namespace testing;
using namespace testing::ext;
using JsModuleReader = OHOS::AbilityRuntime::JsModuleReader;
using Extractor = OHOS::AbilityBase::Extractor;

namespace OHOS {
namespace AAFwk {
class JsModuleReaderTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void JsModuleReaderTest::SetUpTestCase()
{}

void JsModuleReaderTest::TearDownTestCase()
{}

void JsModuleReaderTest::SetUp()
{}

void JsModuleReaderTest::TearDown()
{}

/**
 * @tc.name: JsModuleReaderTest_0100
 * @tc.desc: JsModuleReaderTest Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(JsModuleReaderTest, JsModuleReaderTest_0100, TestSize.Level0)
{
    JsModuleReader jsModuleReader("JsModuleReader", "", nullptr);
    std::vector<uint8_t> result = jsModuleReader("", "");
    EXPECT_EQ(result.size(), 0);
}

/**
 * @tc.name: JsModuleReaderTest_0200
 * @tc.desc: JsModuleReaderTest Test
 * @tc.type: FUNC
 * @tc.require: issueI581RO
 */
HWTEST_F(JsModuleReaderTest, JsModuleReaderTest_0200, TestSize.Level0)
{
    auto extractor = std::make_shared<Extractor>("");
    if (extractor == nullptr) {
        EXPECT_TRUE(extractor == nullptr);
        return;
    }
    JsModuleReader jsModuleReader("JsModuleReader", "", extractor);
    std::vector<uint8_t> result = jsModuleReader("", "");
    EXPECT_EQ(result.size(), 0);
}

/**
 * @tc.name: JsModuleReaderTest_0300
 * @tc.desc: JsModuleReaderTest test
 * @tc.type: FUNC
 * @tc.require: issueI581RO
 */
HWTEST_F(JsModuleReaderTest, JsModuleReaderTest_0300, TestSize.Level0)
{
    auto extractor = std::make_shared<Extractor>("");
    if (extractor == nullptr) {
        EXPECT_TRUE(extractor == nullptr);
        return;
    }
    JsModuleReader jsModuleReader("JsModuleReader", "", extractor);
    std::vector<uint8_t> result = jsModuleReader("/test", "/test2");
    EXPECT_EQ(result.size(), 0);
}
}  // namespace AAFwk
}  // namespace OHOS
