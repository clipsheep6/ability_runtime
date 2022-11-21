/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

// #define OHOS_ABILITY_RUNTIME_JS_RUNTIME_UTILS_H

#include <gtest/gtest.h>
#include "mock_js_runtime.h"
#define private public
#define protected public
#include "js_ability.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
using OHOS::AppExecFwk::ElementName;

namespace OHOS {
namespace AbilityRuntime {
class JsAbilityTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void JsAbilityTest::SetUpTestCase()
{}

void JsAbilityTest::TearDownTestCase()
{}

void JsAbilityTest::SetUp()
{}

void JsAbilityTest::TearDown()
{}

/*
 * @tc.name: JsAbilityTest_0100
 * @tc.desc: test if JsAbilityTest::Init function executed as expected configJson.
 * @tc.type: FUNC
 * @tc.require: #I5SJ62
 *
 */
HWTEST_F(JsAbilityTest, JsAbilityTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityTest_0100 begin";
    JsRuntime jsRuntime;
    JsAbility jsAbility(jsRuntime);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->isModuleJson = false;
    abilityInfo->srcPath = "";
    std::shared_ptr<AbilityHandler> handler;
    sptr<IRemoteObject> token;
    jsAbility.Init(abilityInfo, nullptr, handler, token);
    GTEST_LOG_(INFO) << "JsAbilityTest_0100 end";

}

/*
 * @tc.name: JsAbilityTest_0200
 * @tc.desc: test if JsAbilityTest::Init function executed as expected configJson.
 * @tc.type: FUNC
 * @tc.require: #I5SJ62
 *
 */
HWTEST_F(JsAbilityTest, JsAbilityTest_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityTest_0200 begin";
    JsRuntime jsRuntime;
    JsAbility jsAbility(jsRuntime);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->isModuleJson = false;
    abilityInfo->srcPath = "JsAbilityTest";
    std::shared_ptr<AbilityHandler> handler;
    sptr<IRemoteObject> token;
    jsAbility.Init(abilityInfo, nullptr, handler, token);
    GTEST_LOG_(INFO) << "JsAbilityTest_0200 end";
}

/*
 * @tc.name: JsAbilityTest_0300
 * @tc.desc: test if JsAbilityTest::Init function executed as expected moduleJson.
 * @tc.type: FUNC
 * @tc.require: #I5SJ62
 *
 */
HWTEST_F(JsAbilityTest, JsAbilityTest_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityTest_0300 begin";
    JsRuntime jsRuntime;
    JsAbility jsAbility(jsRuntime);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->isModuleJson = true;
    abilityInfo->srcEntrance = "";
    std::shared_ptr<AbilityHandler> handler;
    sptr<IRemoteObject> token;
    jsAbility.Init(abilityInfo, nullptr, handler, token);
    GTEST_LOG_(INFO) << "JsAbilityTest_0300 end";
}

/*
 * @tc.name: JsAbilityTest_0400
 * @tc.desc: test if JsAbilityTest::Init function executed as expected moduleJson.
 * @tc.type: FUNC
 * @tc.require: #I5SJ62
 *
 */
HWTEST_F(JsAbilityTest, JsAbilityTest_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityTest_0400 begin";
    JsRuntime jsRuntime;
    JsAbility jsAbility(jsRuntime);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->isModuleJson = true;
    abilityInfo->srcEntrance = "JsAbilityTest";
    std::shared_ptr<AbilityHandler> handler;
    sptr<IRemoteObject> token;
    jsAbility.Init(abilityInfo, nullptr, handler, token);
    GTEST_LOG_(INFO) << "JsAbilityTest_0400 end";
}
} // namespace AAFwk
} // namespace OHOS