/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONTEXT_H

#include <map>

#include "ability_connect_callback.h"
#include "extension_context.h"
#include "start_options.h"
#include "want.h"
#include "window.h"

namespace OHOS {
namespace Ace {
class UIContent;
}
namespace AbilityRuntime {
using RuntimeTask = std::function<void(int, const AAFwk::Want &, bool)>;
/**
 * @brief context supply for UIExtension
 *
 */
class UIExtensionContext : public ExtensionContext {
public:
    UIExtensionContext() = default;
    virtual ~UIExtensionContext() = default;

    /**
     * @brief Starts a new ability.
     * An ability using the AbilityInfo.AbilityType.EXTENSION or AbilityInfo.AbilityType.PAGE template uses this method
     * to start a specific ability. The system locates the target ability from installed abilities based on the value
     * of the want parameter and then starts it. You can specify the ability to start using the want parameter.
     *
     * @param want Indicates the Want containing information about the target ability to start.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode StartAbility(const AAFwk::Want &want) const;
    virtual ErrCode StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const;

    /**
     * @brief Destroys the current ui extension ability.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode TerminateSelf();
        /**
     * @brief Connects the current ability to an ability using the AbilityInfo.AbilityType.SERVICE template.
     *
     * @param want Indicates the want containing information about the ability to connect
     *
     * @param conn Indicates the callback object when the target ability is connected.
     *
     * @return Returns zero on success, others on failure.
     */
    virtual ErrCode ConnectAbility(
        const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const;

    /**
     * @brief Disconnects the current ability from an ability.
     *
     * @param conn Indicates the IAbilityConnection callback object passed by connectAbility after the connection
     * is set up. The IAbilityConnection object uniquely identifies a connection between two abilities.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode DisconnectAbility(
        const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const;
    /**
     * Start other ability for result.
     *
     * @param want Information of other ability.
     * @param startOptions Indicates the StartOptions containing service side information about the target ability to
     * start.
     * @param requestCode Request code for abilityMS to return result.
     * @param task Represent std::function<void(int, const AAFwk::Want &, bool)>.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode StartAbilityForResult(const AAFwk::Want &want, int requestCode, RuntimeTask &&task);
    virtual ErrCode StartAbilityForResult(
        const AAFwk::Want &want, const AAFwk::StartOptions &startOptions, int requestCode, RuntimeTask &&task);

    /**
     * Starts a new ability for result using the original caller information.
     *
     * @param want Information of other ability.
     * @param requestCode Request code for abilityMS to return result.
     * @param task Represent std::function<void(int, const AAFwk::Want &, bool)>.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode StartAbilityForResultAsCaller(const AAFwk::Want &want, int requestCode, RuntimeTask &&task);

    /**
     * Starts a new ability for result using the original caller information.
     *
     * @param want Information of other ability.
     * @param startOptions Indicates the StartOptions containing service side information about the target ability to
     * start.
     * @param requestCode Request code for abilityMS to return result.
     * @param task Represent std::function<void(int, const AAFwk::Want &, bool)>.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode StartAbilityForResultAsCaller(
        const AAFwk::Want &want, const AAFwk::StartOptions &startOptions, int requestCode, RuntimeTask &&task);

    /**
     * @brief Called when startAbilityForResult(ohos.aafwk.content.Want,int) is called to start an extension ability
     * and the result is returned.
     * @param requestCode Indicates the request code returned after the ability is started. You can define the request
     * code to identify the results returned by abilities. The value ranges from 0 to 65535.
     * @param resultCode Indicates the result code returned after the ability is started. You can define the result
     * code to identify an error.
     * @param resultData Indicates the data returned after the ability is started. You can define the data returned. The
     * value can be null.
     */
    virtual void OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData);

    virtual int GenerateCurRequestCode();

    virtual ErrCode ReportDrawnCompleted();

    void SetWindow(sptr<Rosen::Window> window);

    sptr<Rosen::Window> GetWindow();

    Ace::UIContent* GetUIContent();

    ErrCode OpenAtomicService(AAFwk::Want& want, const AAFwk::StartOptions &options, int requestCode,
        RuntimeTask &&task);

    using SelfType = UIExtensionContext;
    static const size_t CONTEXT_TYPE_ID;

protected:
    bool IsContext(size_t contextTypeId) override
    {
        return contextTypeId == CONTEXT_TYPE_ID || ExtensionContext::IsContext(contextTypeId);
    }

private:
    static int ILLEGAL_REQUEST_CODE;
    std::map<int, RuntimeTask> resultCallbacks_;

    int curRequestCode_ = 0;

    sptr<Rosen::Window> window_ = nullptr;

    std::mutex mutexlock_;
    /**
     * @brief Get Current Ability Type
     *
     * @return Current Ability Type
     */
    OHOS::AppExecFwk::AbilityType GetAbilityInfoType() const;

    void OnAbilityResultInner(int requestCode, int resultCode, const AAFwk::Want &resultData);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONTEXT_H
