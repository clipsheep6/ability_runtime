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

#include "gtest/gtest.h"

// Add the following two macro definitions to test the private and protected method.
#define private public
#define protected public

#include "frameworks/core/accessibility/js_inspector/inspect_toolbar.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::Framework {
class InspectToolbarTest : public testing::Test {
public:
    static void SetUpTestSuite() {};
    static void TearDownTestSuite() {};
};

/**
 * @tc.name: InspectToolbarTest001
 * @tc.desc: InspectToolbar::InspectToolbar
 * @tc.type: FUNC
 */
HWTEST_F(InspectToolbarTest, InspectToolbarTest001, TestSize.Level1)
{
    NodeId nodeId = -1;
    std::string tag = "tagTest";
    InspectToolbar inspectToolbar(nodeId, tag);
    EXPECT_EQ(inspectToolbar.nodeId_, nodeId);
    EXPECT_EQ(inspectToolbar.tag_, tag);
}
} // namespace OHOS::Ace::Framework
