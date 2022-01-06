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

#ifndef INTERFACES_INNERKITS_JS_WINDOW_STAGE_H
#define INTERFACES_INNERKITS_JS_WINDOW_STAGE_H

#include <map>
#include <native_engine/native_reference.h>
#include <native_engine/native_value.h>
#include <refbase.h>
#include "window_scene.h"

class NativeEngine;
class NativeValue;

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsWindowStage(NativeEngine& engine, std::shared_ptr<Rosen::WindowScene> windowScene);

class JsWindowStage : Rosen::IWindowLifeCycle {
public:
    JsWindowStage(const std::shared_ptr<Rosen::WindowScene>& windowScene, NativeValue* object)
        : windowScene_(windowScene), object_(object) {}
    ~JsWindowStage() = default;
    static void Finalizer(NativeEngine* engine, void* data, void* hint);
    static NativeValue* SetUIContent(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetMainWindow(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* On(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* Off(NativeEngine* engine, NativeCallbackInfo* info);
    virtual void AfterForeground() override;
    virtual void AfterBackground() override;
    virtual void AfterFocused() override;
    virtual void AfterUnfocused() override;

private:
    NativeValue* OnSetUIContent(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetMainWindow(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnEvent(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OffEvent(NativeEngine& engine, NativeCallbackInfo& info);

    enum WindowStageEventType {
        VISIBLE = 1,
        FOCUSED,
        UNFOCUSED,
        INVISIBLE,
    };
    void LifeCycleCallBack(WindowStageEventType type);

    std::shared_ptr<Rosen::WindowScene> windowScene_;
    NativeValue* object_;
    NativeEngine* engine_ = nullptr;
    sptr<IWindowLifeCycle> lifecycleListener_ = nullptr;
    std::map<std::shared_ptr<NativeReference>, int> eventCallbackMap_;
    bool regLifeCycleListenerFlag_ = false;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // INTERFACES_INNERKITS_JS_WINDOW_STAGE_H