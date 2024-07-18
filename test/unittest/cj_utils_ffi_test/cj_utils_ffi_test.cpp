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

#include "cj_utils_ffi.h"

#include "securec.h"
#include <cstdlib>

using namespace testing;
using namespace testing::ext;

class CjUtilsFfiTest : public testing::Test {
public:
    CjUtilsFfiTest()
    {}
    ~CjUtilsFfiTest()
    {}
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void CjUtilsFfiTest::SetUpTestCase()
{}

void CjUtilsFfiTest::TearDownTestCase()
{}

void CjUtilsFfiTest::SetUp()
{}

void CjUtilsFfiTest::TearDown()
{}

/**
 * @tc.name: CjElementNameFfiTestContext_0100
 * @tc.desc: CjUtilsFfiTest test for CreateCStringFromString.
 * @tc.type: FUNC
 */
HWTEST_F(CjUtilsFfiTest, CjUtilsFfiTestCreateCStringFromString_0100, TestSize.Level1)
{
    // 测试用例1：空字符串
    std::string emptyStr = "";
    const char* result1 = CreateCStringFromString(emptyStr);
    EXPECT_TRUE(result1 == nullptr);

    // 测试用例2：正常字符串
    std::string normalStr = "Hello, world!";
    const char* result2 = CreateCStringFromString(normalStr);
    EXPECT_TRUE(result2 != nullptr);

    // 测试用例3：包含特殊字符的字符串
    std::string specialStr = "Hello, \0world!";
    const char* result3 = CreateCStringFromString(specialStr);
    EXPECT_TRUE(result3 != nullptr);
}