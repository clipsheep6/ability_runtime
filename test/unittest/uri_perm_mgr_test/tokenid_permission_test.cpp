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
#define private public
#define protected public
#include "tokenid_permission.h"
#undef private
#undef protected
using namespace testing::ext;
namespace OHOS {
namespace AAFwk {
class TokenIdPermissionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void TokenIdPermissionTest::SetUpTestCase()
{}

void TokenIdPermissionTest::TearDownTestCase()
{}

void TokenIdPermissionTest::SetUp()
{}

void TokenIdPermissionTest::TearDown()
{}

/**
 * @tc.number: VerifyProxyAuthorizationUriPermission_0100
 * @tc.name: VerifyProxyAuthorizationUriPermission
 * @tc.desc: Test whether VerifyProxyAuthorizationUriPermission and are called normally.
 */
HWTEST_F(TokenIdPermissionTest, VerifyProxyAuthorizationUriPermission_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "TokenIdPermissionTest VerifyProxyAuthorizationUriPermission_0100 start";
    uint32_t callerTokenId = 1001;
    auto tokenIdPermission = std::make_shared<TokenIdPermission>(callerTokenId);
    bool res = tokenIdPermission->VerifyProxyAuthorizationUriPermission();
    EXPECT_EQ(res, false);
    GTEST_LOG_(INFO) << "TokenIdPermissionTest VerifyProxyAuthorizationUriPermission_0100 end";
}

/**
 * @tc.number: VerifyFileAccessManagerPermission_0100
 * @tc.name: VerifyFileAccessManagerPermission
 * @tc.desc: Test whether VerifyFileAccessManagerPermission and are called normally.
 */
HWTEST_F(TokenIdPermissionTest, VerifyFileAccessManagerPermission_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "TokenIdPermissionTest VerifyFileAccessManagerPermission_0100 start";
    uint32_t callerTokenId = 1001;
    auto tokenIdPermission = std::make_shared<TokenIdPermission>(callerTokenId);
    bool res = tokenIdPermission->VerifyFileAccessManagerPermission();
    EXPECT_EQ(res, false);
    GTEST_LOG_(INFO) << "TokenIdPermissionTest VerifyFileAccessManagerPermission_0100 end";
}

/**
 * @tc.number: VerifyReadImageVideoPermission_0100
 * @tc.name: VerifyReadImageVideoPermission
 * @tc.desc: Test whether VerifyReadImageVideoPermission and are called normally.
 */
HWTEST_F(TokenIdPermissionTest, VerifyReadImageVideoPermission_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "TokenIdPermissionTest VerifyReadImageVideoPermission_0100 start";
    uint32_t callerTokenId = 1001;
    auto tokenIdPermission = std::make_shared<TokenIdPermission>(callerTokenId);
    bool res = tokenIdPermission->VerifyReadImageVideoPermission();
    EXPECT_EQ(res, false);
    GTEST_LOG_(INFO) << "TokenIdPermissionTest VerifyReadImageVideoPermission_0100 end";
}

/**
 * @tc.number: VerifyWriteImageVideoPermission_0100
 * @tc.name: VerifyWriteImageVideoPermission
 * @tc.desc: Test whether VerifyWriteImageVideoPermission and are called normally.
 */
HWTEST_F(TokenIdPermissionTest, VerifyWriteImageVideoPermission_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "TokenIdPermissionTest VerifyWriteImageVideoPermission_0100 start";
    uint32_t callerTokenId = 1001;
    auto tokenIdPermission = std::make_shared<TokenIdPermission>(callerTokenId);
    bool res = tokenIdPermission->VerifyWriteImageVideoPermission();
    EXPECT_EQ(res, false);
    GTEST_LOG_(INFO) << "TokenIdPermissionTest VerifyWriteImageVideoPermission_0100 end";
}

/**
 * @tc.number: VerifyReadAudioPermission_0100
 * @tc.name: VerifyReadAudioPermission
 * @tc.desc: Test whether VerifyReadAudioPermission and are called normally.
 */
HWTEST_F(TokenIdPermissionTest, VerifyReadAudioPermission_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "TokenIdPermissionTest VerifyReadAudioPermission_0100 start";
    uint32_t callerTokenId = 1001;
    auto tokenIdPermission = std::make_shared<TokenIdPermission>(callerTokenId);
    bool res = tokenIdPermission->VerifyReadAudioPermission();
    EXPECT_EQ(res, false);
    GTEST_LOG_(INFO) << "TokenIdPermissionTest VerifyReadAudioPermission_0100 end";
}

/**
 * @tc.number: VerifyWriteAudioPermission_0100
 * @tc.name: VerifyWriteAudioPermission
 * @tc.desc: Test whether VerifyWriteAudioPermission and are called normally.
 */
HWTEST_F(TokenIdPermissionTest, VerifyWriteAudioPermission_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "TokenIdPermissionTest VerifyWriteAudioPermission_0100 start";
    uint32_t callerTokenId = 1001;
    auto tokenIdPermission = std::make_shared<TokenIdPermission>(callerTokenId);
    bool res = tokenIdPermission->VerifyWriteAudioPermission();
    EXPECT_EQ(res, false);
    GTEST_LOG_(INFO) << "TokenIdPermissionTest VerifyWriteAudioPermission_0100 end";
}
}
}
