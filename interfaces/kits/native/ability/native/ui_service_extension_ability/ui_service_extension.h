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

#ifndef OHOS_ABILITY_RUNTIME_UI_SERVICE_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_UI_SERVICE_EXTENSION_H

#include "extension_base.h"
//#include "ui_ability.h"
#ifdef SUPPORT_GRAPHICS
#include "js_window_stage.h"
#endif
#include "foundation/ability/ability_runtime/utils/global/freeze/include/freeze_util.h"
#include "js_runtime.h"

#include "context/context.h"
namespace OHOS {
#ifdef SUPPORT_GRAPHICS
enum class WindowStageAttribute : int8_t {
    SYSTEM_WINDOW_STAGE,
    SUB_WINDOW_STAGE,
    UNKNOWN
};

struct WindowStageConfig {
    WindowStageAttribute windowStageAttribute = WindowStageAttribute::UNKNOWN;
    OHOS::Rosen::Rect rect;
};
#endif
} // OHOS

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
class UIServiceExtensionContext;
class Runtime;

using Ability = AppExecFwk::Ability;
using AbilityHandler = AppExecFwk::AbilityHandler;
using AbilityInfo = AppExecFwk::AbilityInfo;
using OHOSApplication = AppExecFwk::OHOSApplication;
using AbilityStartSetting = AAFwk::AbilityStartSetting;
using Configuration = AppExecFwk::Configuration;

/**
 * @brief Basic ui extension components.
 */
class UIServiceExtension : public ExtensionBase<UIServiceExtensionContext> {
public:
    UIServiceExtension() = default;
    virtual ~UIServiceExtension() = default;

    /**
     * @brief Create and init context.
     *
     * @param record the ui service extension record.
     * @param application the application info.
     * @param handler the ui service extension handler.
     * @param token the remote token.
     * @return The created context.
     */
    virtual std::shared_ptr<UIServiceExtensionContext> CreateAndInitContext(
        const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Init the ui service extension.
     *
     * @param record the ui service extension record.
     * @param application the application info.
     * @param handler the ui service extension handler.
     * @param token the remote token.
     */
    virtual void Init(const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;
    
    /**
     * @brief Create ui service extension.
     *
     * @param runtime The runtime.
     * @return The ui service extension instance.
     */
    static UIServiceExtension* Create(const std::unique_ptr<Runtime>& runtime);

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_ = nullptr;
    sptr<Rosen::IWindowLifeCycle> sceneListener_ = nullptr;
    // std::shared_ptr<Rosen::WindowScene> scene_ = nullptr;
    sptr<Rosen::IDisplayMoveListener> abilityDisplayMoveListener_ = nullptr;
#ifdef SUPPORT_GRAPHICS
    /**
     * @brief Called before instantiating WindowScene.
     * You can override this function to implement your own processing logic.
     */
    void OnSceneWillCreated(WindowStageConfig &windowStageConfig);

    /**
     * @brief Called after instantiating WindowScene.
     * You can override this function to implement your own processing logic.
     */
    void OnSceneDidCreated();
private:
    void UpdateJsWindowStage(napi_value windowStage);
#endif
private:
    napi_value CallObjectMethod(const char *name, napi_value const *argv = nullptr, size_t argc = 0,
        bool withResult = false, bool showMethodNotFoundLog = true);
    std::unique_ptr<NativeReference> CreateAppWindowStage();
    // void AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState state, const std::string &methodName) const;
    // void AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState state, const std::string &methodName) const;
    // std::shared_ptr<AppExecFwk::ADelegatorAbilityProperty> CreateADelegatorAbilityProperty();

    // JsRuntime &jsRuntime_;
    JsRuntime jsRuntime_;
    std::shared_ptr<AppExecFwk::Ability> ability_;
    std::shared_ptr<Context> context_;
    std::shared_ptr<NativeReference> shellContextRef_;
    std::shared_ptr<NativeReference> jsAbilityObj_;
    std::shared_ptr<NativeReference> jsWindowStageObj_;
    std::shared_ptr<int32_t> screenModePtr_;
    sptr<IRemoteObject> remoteCallee_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_SERVICE_EXTENSION_H
