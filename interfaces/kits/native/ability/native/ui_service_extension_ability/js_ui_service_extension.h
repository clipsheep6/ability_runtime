/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_UI_SERVICE_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_JS_UI_SERVICE_EXTENSION_H

#include "ui_service_extension.h"
#include "configuration.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_execute_result.h"
#include "insight_intent_executor_info.h"
#include "ui_service_extension_context.h"
#ifdef SUPPORT_GRAPHICS
#include "display_manager.h"
#include "system_ability_status_change_stub.h"
#endif
#include "ui_service_extension.h"
#include "ui_service_host_stub.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
class UIServiceExtension;
class Runtime;
class UIServiceExtensionContext;
class JsUIServiceExtension;


/**
 * @brief Basic service components.
 */
class JsUIServiceExtension : public UIServiceExtension {
public:
    explicit JsUIServiceExtension(JsRuntime& jsRuntime);
    virtual ~JsUIServiceExtension() override;

		/**
     * @brief Called when this extension is started. You must override this function if you want to perform some
     *        initialization operations during extension startup.
     *
     * This function can be called only once in the entire lifecycle of an extension.
     * @param Want Indicates the {@link Want} structure containing startup information about the extension.
     */
    virtual void OnStart(const AAFwk::Want &want) override;

#ifdef SUPPORT_GRAPHICS
    /**
     * @brief Called before instantiating WindowScene.
     * You can override this function to implement your own processing logic.
     */
    void OnSceneWillCreated(std::shared_ptr<Rosen::WindowStageConfig> windowStageConfig);

    /**
     * @brief Called after instantiating WindowScene.
     * You can override this function to implement your own processing logic.
     */
    void OnSceneDidCreated();

    void UpdateJsWindowStage(napi_value windowStage);
private:
    std::shared_ptr<NativeReference> jsWindowStageObj_;
#endif
private:
    napi_value CallObjectMethod(const char* name, napi_value const *argv = nullptr, size_t argc = 0);

    JsRuntime& jsRuntime_;
    std::shared_ptr<NativeReference> jsObj_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_SERVICE_EXTENSION_H
