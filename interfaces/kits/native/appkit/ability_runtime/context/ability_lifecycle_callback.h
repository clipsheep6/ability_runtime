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

#ifndef OHOS_ABILITY_RUNTIME_CONTEXT_ABILITY_LIFECYCLE_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_CONTEXT_ABILITY_LIFECYCLE_CALLBACK_H

#include <map>
#include <memory>

class NativeReference;
typedef struct napi_env__* napi_env;
typedef struct napi_value__* napi_value;

namespace OHOS {
namespace AbilityRuntime {
class AbilityLifecycleCallback {
public:
    virtual ~AbilityLifecycleCallback() {}
    /**
     * Called back when the ability is started for initialization.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    virtual void OnAbilityCreate(const std::shared_ptr<NativeReference> &ability) = 0;

    /**
     * Called back when the window stage is created.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @param windowStage: Indicates the window stage to create.
     * @StageModelOnly
     */
    virtual void OnWindowStageCreate(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) = 0;

    /**
     * Called back when the window stage is destroy.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @param windowStage: Indicates the window stage to destroy.
     * @StageModelOnly
     */
    virtual void OnWindowStageDestroy(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) = 0;

    /**
     * Called back when the window stage is active.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @param windowStage: Indicates the window stage to active.
     * @StageModelOnly
     */
    virtual void OnWindowStageActive(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) = 0;

    /**
     * Called back when the window stage is inactive.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @param windowStage: Indicates the window stage to inactive.
     * @StageModelOnly
     */
    virtual void OnWindowStageInactive(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) = 0;

    /**
     * Called back when the ability is destroy.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    virtual void OnAbilityDestroy(const std::shared_ptr<NativeReference> &ability) = 0;

    /**
     * Called back after the UIAbility called onNewWant.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    virtual void OnNewWant(const std::shared_ptr<NativeReference> &ability)
    {}

    /**
     * Called back before the UIAbility will called onNewWant.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    virtual void OnWillNewWant(const std::shared_ptr<NativeReference> &ability)
    {}

    /**
     * Called back before an ability is started for initialization.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    virtual void OnAbilityWillCreate(const std::shared_ptr<NativeReference> &ability)
    {}

    /**
     * Called back before a window stage is created.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @param windowStage: Indicates the window stage to active.
     * @StageModelOnly
     */
    virtual void OnWindowStageWillCreate(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) {}

    /**
     * Called back before a window stage is destroyed.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @param windowStage: Indicates the window stage to active.
     * @StageModelOnly
     */
    virtual void OnWindowStageWillDestroy(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) {}

    /**
     * Called back before an ability is destroyed.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    virtual void OnAbilityWillDestroy(const std::shared_ptr<NativeReference> &ability) {}

    /**
     * Called back before the state of an ability changes to foreground.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    virtual void OnAbilityWillForeground(const std::shared_ptr<NativeReference> &ability) {}

    /**
     * Called back before the state of an ability changes to background.
     *
     * @since 12
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    virtual void OnAbilityWillBackground(const std::shared_ptr<NativeReference> &ability) {}

    /**
     * Called back when the ability is foreground.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    virtual void OnAbilityForeground(const std::shared_ptr<NativeReference> &ability) = 0;

    /**
     * Called back when the ability is background.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    virtual void OnAbilityBackground(const std::shared_ptr<NativeReference> &ability) = 0;

    /**
     * Called back when the ability is continue.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    virtual void OnAbilityContinue(const std::shared_ptr<NativeReference> &ability) = 0;

    virtual void OnAbilityWillContinue(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnWindowStageWillRestore(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) {}
    virtual void OnWindowStageRestore(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) {}
    virtual void OnAbilityWillSaveState(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnAbilitySaveState(const std::shared_ptr<NativeReference> &ability) {}
};

class JsAbilityLifecycleCallback : public AbilityLifecycleCallback,
                                   public std::enable_shared_from_this<JsAbilityLifecycleCallback> {
public:
    explicit JsAbilityLifecycleCallback(napi_env env);
    void OnAbilityCreate(const std::shared_ptr<NativeReference> &ability) override;
    void OnWindowStageCreate(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) override;
    void OnWindowStageDestroy(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) override;
    void OnWindowStageActive(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) override;
    void OnWindowStageInactive(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) override;
    void OnAbilityWillCreate(const std::shared_ptr<NativeReference> &ability) override;
    void OnWindowStageWillCreate(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) override;
    void OnWindowStageWillDestroy(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) override;
    void OnAbilityWillDestroy(const std::shared_ptr<NativeReference> &ability) override;
    void OnAbilityWillForeground(const std::shared_ptr<NativeReference> &ability) override;
    void OnAbilityWillBackground(const std::shared_ptr<NativeReference> &ability) override;
    void OnNewWant(const std::shared_ptr<NativeReference> &ability) override;
    void OnWillNewWant(const std::shared_ptr<NativeReference> &ability) override;
    void OnAbilityDestroy(const std::shared_ptr<NativeReference> &ability) override;
    void OnAbilityForeground(const std::shared_ptr<NativeReference> &ability) override;
    void OnAbilityBackground(const std::shared_ptr<NativeReference> &ability) override;
    void OnAbilityContinue(const std::shared_ptr<NativeReference> &ability) override;
    void OnAbilityWillContinue(const std::shared_ptr<NativeReference> &ability) override;
    void OnWindowStageWillRestore(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) override;
    void OnWindowStageRestore(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) override;
    void OnAbilityWillSaveState(const std::shared_ptr<NativeReference> &ability) override;
    void OnAbilitySaveState(const std::shared_ptr<NativeReference> &ability) override;
    int32_t Register(napi_value jsCallback, bool isSync = false);
    bool UnRegister(int32_t callbackId, bool isSync = false);
    bool IsEmpty() const;
    static int32_t serialNumber_;

private:
    napi_env env_ = nullptr;
    std::shared_ptr<NativeReference> jsCallback_;
    std::map<int32_t, std::shared_ptr<NativeReference>> callbacks_;
    std::map<int32_t, std::shared_ptr<NativeReference>> callbacksSync_;
    void CallJsMethod(const std::string &methodName, const std::shared_ptr<NativeReference> &ability);
    void CallWindowStageJsMethod(const std::string &methodName, const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage);
    void CallJsMethodInnerCommon(const std::string &methodName, const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage,
        const std::map<int32_t, std::shared_ptr<NativeReference>> callbacks);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CONTEXT_ABILITY_LIFECYCLE_CALLBACK_H
