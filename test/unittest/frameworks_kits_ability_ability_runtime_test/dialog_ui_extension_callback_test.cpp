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
#include "dialog_ui_extension_callback.h"
#include "mock_ui_content.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;
namespace OHOS {
namespace AAFwk {
class MyAbilityCallback : public IAbilityCallback {
public:
    virtual int GetCurrentWindowMode()
    {
        return 0;
    }

    virtual ErrCode SetMissionLabel(const std::string& label)
    {
        return 0;
    }

    virtual ErrCode SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap>& icon)
    {
        GTEST_LOG_(INFO) << "========AbilityCallback SetMissionIcon------------------------.";
        return 0;
    }

    virtual void GetWindowRect(int32_t &left, int32_t &top, int32_t &width, int32_t &height)
    {
        return;
    }

    virtual Ace::UIContent* GetUIContent()
    {
        return nullptr;
    }

    void EraseUIExtension(int32_t sessionId)
    {
        return;
    }

    void RegisterAbilityLifecycleObserver(const std::shared_ptr<ILifecycleObserver> &observer)
    {
    }

    void UnregisterAbilityLifecycleObserver(const std::shared_ptr<ILifecycleObserver> &observer)
    {
    }
};

class DialogUIExtensionCallbackTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DialogUIExtensionCallbackTest::SetUpTestCase(void) {}
void DialogUIExtensionCallbackTest::TearDownTestCase(void) {}
void DialogUIExtensionCallbackTest::TearDown() {}
void DialogUIExtensionCallbackTest::SetUp() {}

/**
 * @tc.name: DialogUIExtensionCallbackTest_OnRelease_0100
 * @tc.desc: Test the state of OnRelease
 * @tc.type: FUNC
 */
HWTEST_F(DialogUIExtensionCallbackTest, OnRelease_0100, TestSize.Level1)
{
    Ace::MockUIContent *uicontent = new Ace::MockUIContent();
    EXPECT_CALL(*uicontent, CloseModalUIExtension(_)).Times(1).WillOnce(Return());
    auto abilityCallback_ = std::make_shared<MyAbilityCallback>();
    auto dialogUIExtensionCallback_ =
    std::make_shared<DialogUIExtensionCallback>(std::weak_ptr<AppExecFwk::IAbilityCallback>(abilityCallback_));
    dialogUIExtensionCallback_->SetUIContent(uicontent);
    dialogUIExtensionCallback_->SetSessionId(1);
    dialogUIExtensionCallback_->OnRelease();
    delete uicontent;
}

/**
 * @tc.name: DialogUIExtensionCallbackTest_OnError_0100
 * @tc.desc: Test the state of OnError
 * @tc.type: FUNC
 */
HWTEST_F(DialogUIExtensionCallbackTest, OnError_0100, TestSize.Level1)
{
    Ace::MockUIContent *uicontent = new Ace::MockUIContent();
    EXPECT_CALL(*uicontent, CloseModalUIExtension(_)).Times(1).WillOnce(Return());
    auto abilityCallback_ = std::make_shared<MyAbilityCallback>();
    auto dialogUIExtensionCallback_ =
    std::make_shared<DialogUIExtensionCallback>(std::weak_ptr<AppExecFwk::IAbilityCallback>(abilityCallback_));
    dialogUIExtensionCallback_->SetUIContent(uicontent);
    dialogUIExtensionCallback_->SetSessionId(1);
    dialogUIExtensionCallback_->OnError();
    delete uicontent;
}

/**
 * @tc.name: DialogUIExtensionCallbackTest_OnDestroy_0100
 * @tc.desc: Test the state of OnDestroy
 * @tc.type: FUNC
 */
HWTEST_F(DialogUIExtensionCallbackTest, OnDestroy_0100, TestSize.Level1)
{
    auto abilityCallback = std::make_shared<MyAbilityCallback>();
    auto dialogUIExtensionCallback =
    std::make_shared<DialogUIExtensionCallback>(std::weak_ptr<AppExecFwk::IAbilityCallback>(abilityCallback));
    dialogUIExtensionCallback->OnDestroy();
}

} // namespace AAFwk
} // namespace OHOS
