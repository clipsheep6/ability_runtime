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

#include "js_quick_fix_manager.h"

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
class JsQuickFixManager {
public:
    JsQuickFixManager() = default;
    ~JsQuickFixManager() = default;

    static void Finalizer(NativeEngine *engine, void *data, void *hint)
    {
        HILOG_INFO("JsQuickFixManager::Finalizer is called.");
        std::unique_ptr<JsQuickFixManager>(static_cast<JsQuickFixManager*>(data));
    }

    static NativeValue *ApplyQuickFix(NativeEngine *engine, NativeCallbackInfo *info)
    {
        JsQuickFixManager *me = CheckParamsAndGetThis<JsQuickFixManager>(engine, info);
        return (me != nullptr) ? me->OnApplyQuickFix(*engine, *info) : nullptr;
    }

private:
    NativeValue *OnApplyQuickFix(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_DEBUG("OnApplyQuickFix is called.");

        // TODO: parameter parse
        // TODO: according to promise or callback, call different value

        NativeValue *result = nullptr;
        HILOG_DEBUG("OnApplyQuickFix is finished.");
        return result;
    }
};

NativeValue *CreateJsQuickFixManager(NativeEngine *engine, NativeValue *exportObj)
{
    HILOG_INFO("CreateJsQuickFixManager is called.");
    if (engine == nullptr || exportObj == nullptr) {
        HILOG_ERROR("Input parameter is invalid.");
        return nullptr;
    }

    NativeObject *object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr.");
        return nullptr;
    }

    std::unique_ptr<JsQuickFixManager> quickFixManager = std::make_unique<JsQuickFixManager>();
    object->SetNativePointer(quickFixManager.release(), JsQuickFixManager::Finalizer, nullptr);

    BindNativeFunction(*engine, *object, "applyQuickFix", JsQuickFixManager::ApplyQuickFix);
    return engine->CreateUndefined();
}
} // namespace AbilityRuntime
} // namespace OHOS
