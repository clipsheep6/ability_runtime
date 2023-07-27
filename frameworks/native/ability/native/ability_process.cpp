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

#include "ability_process.h"

#include <dlfcn.h>

#include "accesstoken_kit.h"
#include "hilog_wrapper.h"
#include "permission_list_state.h"

using OHOS::Security::AccessToken::AccessTokenKit;
using OHOS::Security::AccessToken::PermissionListState;
using OHOS::Security::AccessToken::TypePermissionOper;

namespace OHOS {
namespace AppExecFwk {
static void *g_handle = nullptr;
#ifdef SUPPORT_GRAPHICS
#ifdef APP_USE_ARM
constexpr char SHARED_LIBRARY_FEATURE_ABILITY[] = "/system/lib/module/ability/libfeatureability.z.so";
#else
constexpr char SHARED_LIBRARY_FEATURE_ABILITY[] = "/system/lib64/module/ability/libfeatureability.z.so";
#endif
#endif
constexpr char FUNC_CALL_ON_ABILITY_RESULT[] = "CallOnAbilityResult";
using NAPICallOnAbilityResult = void (*)(int requestCode, int resultCode, const Want &resultData, CallbackInfo cb);
constexpr char FUNC_CALL_ON_REQUEST_PERMISSIONS_FROM_USERRESULT[] = "CallOnRequestPermissionsFromUserResult";
using NAPICallOnRequestPermissionsFromUserResult = void (*)(int requestCode,
    const std::vector<std::string> &permissions, const std::vector<int> &grantResults, CallbackInfo callbackInfo);

std::shared_ptr<AbilityProcess> AbilityProcess::instance_ = nullptr;
std::map<Ability *, std::map<int, CallbackInfo>> AbilityProcess::abilityResultMap_;
std::mutex AbilityProcess::mutex_;
std::shared_ptr<AbilityProcess> AbilityProcess::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<AbilityProcess>();
        }
    }
    return instance_;
}

AbilityProcess::AbilityProcess()
{}

AbilityProcess::~AbilityProcess()
{}

ErrCode AbilityProcess::StartAbility(Ability *ability, CallAbilityParam param, CallbackInfo callback)
{
    HILOG_DEBUG("begin");
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return ERR_NULL_OBJECT;
    }
#ifdef SUPPORT_GRAPHICS
    // inherit split mode
    auto windowMode = ability->GetCurrentWindowMode();
    if (windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        param.want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }
    HILOG_INFO("window mode is %{public}d", windowMode);
#endif
    ErrCode err = ERR_OK;
    if (param.forResultOption == true) {
        if (param.setting == nullptr) {
            HILOG_INFO("param.setting == nullptr call StartAbilityForResult.");
            param.want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
            err = ability->StartAbilityForResult(param.want, param.requestCode);
        } else {
            HILOG_INFO("param.setting != nullptr call StartAbilityForResult.");
            err = ability->StartAbilityForResult(param.want, param.requestCode, *(param.setting));
        }

        std::lock_guard<std::mutex> lock_l(mutex_);

        std::map<int, CallbackInfo> map;
        auto it = abilityResultMap_.find(ability);
        if (it == abilityResultMap_.end()) {
            HILOG_INFO("ability is not in the abilityResultMap_");
        } else {
            HILOG_INFO("ability is in the abilityResultMap_");
            map = it->second;
        }
        callback.errCode = err;
        map[param.requestCode] = callback;
        abilityResultMap_[ability] = map;
    } else {
        if (param.setting == nullptr) {
            HILOG_INFO("param.setting == nullptr call StartAbility.");
            err = ability->StartAbility(param.want);
        } else {
            HILOG_INFO("param.setting != nullptr call StartAbility.");
            err = ability->StartAbility(param.want, *(param.setting));
        }
    }
    HILOG_DEBUG("end");
    return err;
}

void AbilityProcess::AddAbilityResultCallback(Ability *ability, CallAbilityParam &param, int32_t errCode,
                                              CallbackInfo &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::map<int, CallbackInfo> map;
    auto it = abilityResultMap_.find(ability);
    if (it == abilityResultMap_.end()) {
        HILOG_INFO("ability is not in the abilityResultMap_");
    } else {
        HILOG_INFO("ability is in the abilityResultMap_");
        map = it->second;
    }
    callback.errCode = errCode;
    map[param.requestCode] = callback;
    abilityResultMap_[ability] = map;
}

void AbilityProcess::OnAbilityResult(Ability *ability, int requestCode, int resultCode, const Want &resultData)
{
    HILOG_DEBUG("begin");

    std::lock_guard<std::mutex> lock_l(mutex_);

    auto it = abilityResultMap_.find(ability);
    if (it == abilityResultMap_.end()) {
        HILOG_ERROR("ability is not in the abilityResultMap");
        return;
    }
    std::map<int, CallbackInfo> map = it->second;

    auto callback = map.find(requestCode);
    if (callback == map.end()) {
        HILOG_ERROR("requestCode: %{public}d is not in the map", requestCode);
        return;
    }
    CallbackInfo callbackInfo = callback->second;
#ifdef SUPPORT_GRAPHICS
    // start open featureability lib
    if (g_handle == nullptr) {
        g_handle = dlopen(SHARED_LIBRARY_FEATURE_ABILITY, RTLD_LAZY);
        if (g_handle == nullptr) {
            HILOG_ERROR("dlopen failed %{public}s. %{public}s",
                SHARED_LIBRARY_FEATURE_ABILITY,
                dlerror());
            return;
        }
    }
#endif
    // get function
    auto func = reinterpret_cast<NAPICallOnAbilityResult>(dlsym(g_handle, FUNC_CALL_ON_ABILITY_RESULT));
    if (func == nullptr) {
        HILOG_ERROR("dlsym failed %{public}s. %{public}s", FUNC_CALL_ON_ABILITY_RESULT, dlerror());
        dlclose(g_handle);
        g_handle = nullptr;
        return;
    }
    func(requestCode, resultCode, resultData, callbackInfo);

    map.erase(requestCode);

    abilityResultMap_[ability] = map;
    HILOG_DEBUG("end");
}

void AbilityProcess::RequestPermissionsFromUser(
    Ability *ability, CallAbilityPermissionParam &param, CallbackInfo callbackInfo)
{
    HILOG_DEBUG("begin");
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }

    std::vector<PermissionListState> permList;
    for (auto permission : param.permission_list) {
        HILOG_DEBUG("permission: %{public}s.", permission.c_str());
        PermissionListState permState;
        permState.permissionName = permission;
        permState.state = -1;
        permList.emplace_back(permState);
    }
    HILOG_DEBUG("permList size: %{public}zu, permissions size: %{public}zu.",
        permList.size(), param.permission_list.size());

    auto ret = AccessTokenKit::GetSelfPermissionsState(permList);
    if (permList.size() != param.permission_list.size()) {
        HILOG_ERROR("Returned permList size: %{public}zu.", permList.size());
        return;
    }

    std::vector<int> permissionsState;
    for (auto permState : permList) {
        HILOG_DEBUG("permissions: %{public}s. permissionsState: %{public}u",
            permState.permissionName.c_str(), permState.state);
        permissionsState.emplace_back(permState.state);
    }
    HILOG_DEBUG("permissions size: %{public}zu. permissionsState size: %{public}zu",
        param.permission_list.size(), permissionsState.size());

    auto requestCode = param.requestCode;
    if (ret != TypePermissionOper::DYNAMIC_OPER) {
        HILOG_DEBUG("No dynamic popup required.");
        (void)CaullFunc(requestCode, param.permission_list, permissionsState, callbackInfo);
        return;
    }

    auto task = [self = GetInstance(), requestCode, callbackInfo]
        (const std::vector<std::string> &permissions, const std::vector<int> &grantResults) mutable {
        if (!self) {
            HILOG_ERROR("self is nullptr.");
            return;
        }
        if (!self->CaullFunc(requestCode, permissions, grantResults, callbackInfo)) {
            HILOG_ERROR("call function failed.");
            return;
        }
    };

    ability->RequestPermissionsFromUser(param.permission_list, permissionsState, std::move(task));
}

bool AbilityProcess::CaullFunc(int requestCode, const std::vector<std::string> &permissions,
    const std::vector<int> &permissionsState, CallbackInfo &callbackInfo)
{
#ifdef SUPPORT_GRAPHICS
    // start open featureability lib
    if (g_handle == nullptr) {
        g_handle = dlopen(SHARED_LIBRARY_FEATURE_ABILITY, RTLD_LAZY);
        if (g_handle == nullptr) {
            HILOG_ERROR("dlopen failed %{public}s. %{public}s",
                SHARED_LIBRARY_FEATURE_ABILITY, dlerror());
            return false;
        }
    }
#endif
    // get function
    auto func = reinterpret_cast<NAPICallOnRequestPermissionsFromUserResult>(
        dlsym(g_handle, FUNC_CALL_ON_REQUEST_PERMISSIONS_FROM_USERRESULT));
    if (func == nullptr) {
        HILOG_ERROR("dlsym failed %{public}s. %{public}s",
            FUNC_CALL_ON_REQUEST_PERMISSIONS_FROM_USERRESULT, dlerror());
        dlclose(g_handle);
        g_handle = nullptr;
        return false;
    }
    func(requestCode, permissions, permissionsState, callbackInfo);
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
