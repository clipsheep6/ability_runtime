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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ABILITY_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_CJ_ABILITY_CONTEXT_H

#include "ability.h"
#include "ability_context_impl.h"
#include "ffi_remote_data.h"

namespace OHOS {
namespace AbilityRuntime {
class CJAbilityContext : public FFI::FFIData {
public:
    explicit CJAbilityContext(const std::shared_ptr<AbilityRuntime::AbilityContext>& abilityContext)
        : context_(abilityContext) {};

    std::shared_ptr<AbilityRuntime::AbilityContext> GetAbilityContext();
    sptr<IRemoteObject> GetToken();
    std::string GetPreferencesDir();
    std::string GetDatabaseDir();
    std::string GetBundleName();
    int32_t GetArea();
    std::shared_ptr<AppExecFwk::AbilityInfo> GetAbilityInfo();
    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo();
    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration();
    int32_t StartAbility(const AAFwk::Want& want);
    int32_t StartAbility(const AAFwk::Want& want, const AAFwk::StartOptions& startOptions);
    int32_t StartAbilityWithAccount(const AAFwk::Want& want, int32_t accountId);
    int32_t StartAbilityWithAccount(
        const AAFwk::Want& want, int32_t accountId, const AAFwk::StartOptions& startOptions);
    int32_t StartServiceExtensionAbility(const AAFwk::Want& want);
    int32_t StartServiceExtensionAbilityWithAccount(const AAFwk::Want& want, int32_t accountId);
    int32_t StopServiceExtensionAbility(const AAFwk::Want& want);
    int32_t StopServiceExtensionAbilityWithAccount(const AAFwk::Want& want, int32_t accountId);
    int32_t TerminateSelf();
    int32_t TerminateSelfWithResult(const AAFwk::Want& want, int32_t resultCode);
    std::tuple<int32_t, bool> IsTerminating();
    bool ConnectAbility(const AAFwk::Want& want, int64_t connectionId);
    int32_t ConnectAbilityWithAccount(
        const AAFwk::Want& want, int32_t accountId, int64_t connectionId);
    void DisconnectAbility(const AAFwk::Want& want, int64_t connectionId);
    int32_t StartAbilityForResult(const AAFwk::Want& want, int32_t requestCode, RuntimeTask&& task);
    int32_t StartAbilityForResult(
        const AAFwk::Want& want, const AAFwk::StartOptions& startOptions, int32_t requestCode, RuntimeTask&& task);
    int32_t StartAbilityForResultWithAccount(
        const AAFwk::Want& want, int32_t accountId, int32_t requestCode, RuntimeTask&& task);
    int32_t StartAbilityForResultWithAccount(const AAFwk::Want& want, int32_t accountId,
        const AAFwk::StartOptions& startOptions, int32_t requestCode, RuntimeTask&& task);
    int32_t RequestPermissionsFromUser(
        AppExecFwk::Ability* ability, std::vector<std::string>& permissions, PermissionRequestTask&& task);
    void InheritWindowMode(AAFwk::Want& want);
    int32_t RequestDialogService(AAFwk::Want& want, RequestDialogResultTask&& task);

#ifdef SUPPORT_GRAPHICS
    int32_t SetMissionLabel(const std::string& label);
    int32_t SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap>& icon);
#endif

private:
    std::shared_ptr<AbilityContext> context_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CJ_ABILITY_CONTEXT_H
