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

#include "foundation/ability/ability_runtime/frameworks/cj/mock/cj_delegator_ffi.cpp"

using namespace testing;
using namespace testing::ext;

class CjDelegatorFfiMockTest : public testing::Test {
public:
    CjDelegatorFfiMockTest()
    {}
    ~CjDelegatorFfiMockTest()
    {}
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void CjDelegatorFfiMockTest::SetUpTestCase()
{}

void CjDelegatorFfiMockTest::TearDownTestCase()
{}

void CjDelegatorFfiMockTest::SetUp()
{}

void CjDelegatorFfiMockTest::TearDown()
{}

/**
 * @tc.name: CjDelegatorFfiMockTestRegisterCJTestRunnerFuncs_0100
 * @tc.desc: CjDelegatorFfiMockTest test for RegisterCJTestRunnerFuncs.
 * @tc.type: FUNC
 */
HWTEST_F(CjDelegatorFfiMockTest, CjDelegatorFfiMockTestRegisterCJTestRunnerFuncs_0100, TestSize.Level1)
{
    RegisterCJTestRunnerFuncs();
}