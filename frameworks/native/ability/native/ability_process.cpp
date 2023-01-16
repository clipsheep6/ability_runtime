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
#ifdef _ARM64_
constexpr char SHARED_LIBRARY_FEATURE_ABILITY[] = "/system/lib64/module/ability/libfeatureability.z.so";
#else
constexpr char SHARED_LIBRARY_FEATURE_ABILITY[] = "/system/lib/module/ability/libfeatureability.z.so";
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
std::mutex AbilityProcess::permissionRequestMutex_;
std::map<int, PermissionRequestTask> AbilityProcess::permissionRequestCallbackMap_;
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
    HILOG_INFO("AbilityProcess::StartAbility begin");
    if (ability == nullptr) {
        HILOG_ERROR("AbilityProcess::StartAbility ability is nullptr");
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
            HILOG_INFO("%{public}s param.setting == nullptr call StartAbilityForResult.", __func__);
            param.want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
            err = ability->StartAbilityForResult(param.want, param.requestCode);
        } else {
            HILOG_INFO("%{public}s param.setting != nullptr call StartAbilityForResult.", __func__);
            err = ability->StartAbilityForResult(param.want, param.requestCode, *(param.setting));
        }

        std::lock_guard<std::mutex> lock_l(mutex_);

        std::map<int, CallbackInfo> map;
        auto it = abilityResultMap_.find(ability);
        if (it == abilityResultMap_.end()) {
            HILOG_INFO("AbilityProcess::StartAbility ability: is not in the abilityResultMap_");
        } else {
            HILOG_INFO("AbilityProcess::StartAbility ability: is in the abilityResultMap_");
            map = it->second;
        }
        callback.errCode = err;
        map[param.requestCode] = callback;
        abilityResultMap_[ability] = map;
    } else {
        if (param.setting == nullptr) {
            HILOG_INFO("%{public}s param.setting == nullptr call StartAbility.", __func__);
            err = ability->StartAbility(param.want);
        } else {
            HILOG_INFO("%{public}s param.setting != nullptr call StartAbility.", __func__);
            err = ability->StartAbility(param.want, *(param.setting));
        }
    }
    HILOG_INFO("AbilityProcess::StartAbility end");
    return err;
}

void AbilityProcess::AddAbilityResultCallback(Ability *ability, CallAbilityParam &param, int32_t errCode,
                                              CallbackInfo &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::map<int, CallbackInfo> map;
    auto it = abilityResultMap_.find(ability);
    if (it == abilityResultMap_.end()) {
        HILOG_INFO("AbilityProcess::StartAbility ability: is not in the abilityResultMap_");
    } else {
        HILOG_INFO("AbilityProcess::StartAbility ability: is in the abilityResultMap_");
        map = it->second;
    }
    callback.errCode = errCode;
    map[param.requestCode] = callback;
    abilityResultMap_[ability] = map;
}

void AbilityProcess::OnAbilityResult(Ability *ability, int requestCode, int resultCode, const Want &resultData)
{
    HILOG_INFO("AbilityProcess::OnAbilityResult begin");

    std::lock_guard<std::mutex> lock_l(mutex_);

    auto it = abilityResultMap_.find(ability);
    if (it == abilityResultMap_.end()) {
        HILOG_ERROR("AbilityProcess::OnAbilityResult ability: is not in the abilityResultMap");
        return;
    }
    std::map<int, CallbackInfo> map = it->second;

    auto callback = map.find(requestCode);
    if (callback == map.end()) {
        HILOG_ERROR("AbilityProcess::OnAbilityResult requestCode: %{public}d is not in the map", requestCode);
        return;
    }
    CallbackInfo callbackInfo = callback->second;
#ifdef SUPPORT_GRAPHICS
    // start open featureability lib
    if (g_handle == nullptr) {
        g_handle = dlopen(SHARED_LIBRARY_FEATURE_ABILITY, RTLD_LAZY);
        if (g_handle == nullptr) {
            HILOG_ERROR("%{public}s, dlopen failed %{public}s. %{public}s",
                __func__,
                SHARED_LIBRARY_FEATURE_ABILITY,
                dlerror());
            return;
        }
    }
#endif
    // get function
    auto func = reinterpret_cast<NAPICallOnAbilityResult>(dlsym(g_handle, FUNC_CALL_ON_ABILITY_RESULT));
    if (func == nullptr) {
        HILOG_ERROR(
            "%{public}s, dlsym failed %{public}s. %{public}s", __func__, FUNC_CALL_ON_ABILITY_RESULT, dlerror());
        dlclose(g_handle);
        g_handle = nullptr;
        return;
    }
    func(requestCode, resultCode, resultData, callbackInfo);

    map.erase(requestCode);

    abilityResultMap_[ability] = map;
    HILOG_INFO("AbilityProcess::OnAbilityResult end");
}

void AbilityProcess::RequestPermissionsFromUser(
    Ability *ability, CallAbilityPermissionParam &param, CallbackInfo callbackInfo)
{
    HILOG_INFO("AbilityProcess::RequestPermissionsFromUser begin");
    if (ability == nullptr) {
        HILOG_ERROR("AbilityProcess::RequestPermissionsFromUser ability is nullptr");
        return;
    }

    std::vector<PermissionListState> permList;
    for (auto permission : param.permission_list) {
        HILOG_DEBUG("%{public}s. permission: %{public}s.", __func__, permission.c_str());
        PermissionListState permState;
        permState.permissionName = permission;
        permState.state = -1;
        permList.emplace_back(permState);
    }
    HILOG_DEBUG("%{public}s. permList size: %{public}zu, permissions size: %{public}zu.",
        __func__, permList.size(), param.permission_list.size());

    auto ret = AccessTokenKit::GetSelfPermissionsState(permList);
    if (permList.size() != param.permission_list.size()) {
        HILOG_ERROR("%{public}s. Returned permList size: %{public}zu.", __func__, permList.size());
        return;
    }

    std::vector<int> permissionsState;
    for (auto permState : permList) {
        HILOG_DEBUG("%{public}s. permissions: %{public}s. permissionsState: %{public}u",
            __func__, permState.permissionName.c_str(), permState.state);
        permissionsState.emplace_back(permState.state);
    }
    HILOG_DEBUG("%{public}s. permissions size: %{public}zu. permissionsState size: %{public}zu",
        __func__, param.permission_list.size(), permissionsState.size());

    auto requestCode = param.requestCode;
    if (ret != TypePermissionOper::DYNAMIC_OPER) {
        HILOG_DEBUG("%{public}s. No dynamic popup required.", __func__);
        (void)CaullFunc(requestCode, param.permission_list, permissionsState, callbackInfo);
        return;
    }

    auto task = [self = GetInstance(), requestCode, callbackInfo]
        (const std::vector<std::string> &permissions, const std::vector<int> &grantResults) mutable {
        if (!self) {
            HILOG_ERROR("%{public}s: self is nullptr.", __func__);
            return;
        }
        if (!self->CaullFunc(requestCode, permissions, grantResults, callbackInfo)) {
            HILOG_ERROR("%{public}s: call function failed.", __func__);
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
            HILOG_ERROR("%{public}s, dlopen failed %{public}s. %{public}s",
                __func__, SHARED_LIBRARY_FEATURE_ABILITY, dlerror());
            return false;
        }
    }
#endif
    // get function
    auto func = reinterpret_cast<NAPICallOnRequestPermissionsFromUserResult>(
        dlsym(g_handle, FUNC_CALL_ON_REQUEST_PERMISSIONS_FROM_USERRESULT));
    if (func == nullptr) {
        HILOG_ERROR("%{public}s, dlsym failed %{public}s. %{public}s",
            __func__, FUNC_CALL_ON_REQUEST_PERMISSIONS_FROM_USERRESULT, dlerror());
        dlclose(g_handle);
        g_handle = nullptr;
        return false;
    }
    func(requestCode, permissions, permissionsState, callbackInfo);
    return true;
}

void AbilityProcess::RequestPermissionsFromUser(NativeEngine &engine, Ability *ability,
    const RequestPermissionsParamInfo &param, PermissionRequestTask &&task)
{
    HILOG_DEBUG("AbilityProcess::RequestPermissionsFromUser begin");
    if (ability == nullptr) {
        HILOG_ERROR("AbilityProcess::RequestPermissionsFromUser ability is nullptr");
        return;
    }

    std::vector<PermissionListState> permList;
    for (auto permission : param.permissionList) {
        HILOG_DEBUG("permission: %{public}s.", permission.c_str());
        PermissionListState permState;
        permState.permissionName = permission;
        permState.state = -1;
        permList.emplace_back(permState);
    }
    HILOG_DEBUG("permList size: %{public}zu, permissions size: %{public}zu.", permList.size(),
        param.permissionList.size());

    auto ret = AccessTokenKit::GetSelfPermissionsState(permList);
    if (permList.size() != param.permissionList.size()) {
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
        param.permissionList.size(), permissionsState.size());

    if (ret != TypePermissionOper::DYNAMIC_OPER) {
        HILOG_DEBUG("No dynamic popup required.");
        if (task) {
            task(param.permissionList, permissionsState);
        }
        return;
    }
    RequestPermissionsParamInfo paramInfo;
    paramInfo.permissionList = param.permissionList;
    paramInfo.permissionsState = permissionsState;
    paramInfo.callbackKey = param.callbackKey;
    StartGrantExtension(engine, ability, paramInfo, std::move(task));
    HILOG_DEBUG("AbilityProcess::RequestPermissionsFromUser end");
}

void AbilityProcess::StartGrantExtension(NativeEngine &engine, Ability *ability,
    const RequestPermissionsParamInfo &param, PermissionRequestTask &&task)
{
    HILOG_DEBUG("AbilityProcess::StartGrantExtension begin");
    {
        std::lock_guard<std::mutex> lock(permissionRequestMutex_);
        permissionRequestCallbackMap_.insert(make_pair(param.callbackKey, std::move(task)));
    }
    auto resultTask = [&engine, callbackKey = param.callbackKey]
        (const std::vector<std::string> &permissions, const std::vector<int> &grantResults) {
        auto retCB = new RequestPermissionsResultCallback();
        retCB->permissions = permissions;
        retCB->grantResults = grantResults;
        retCB->callbackKey = callbackKey;

        auto loop = engine.GetUVLoop();
        if (loop == nullptr) {
            HILOG_ERROR("StartGrantExtension, fail to get uv loop.");
            return;
        }
        auto work = new uv_work_t;
        work->data = static_cast<void *>(retCB);
        int rev = uv_queue_work(
            loop,
            work,
            [](uv_work_t *work) {},
            ResultCallbackJSThreadWorker);
        if (rev != 0) {
            if (retCB != nullptr) {
                delete retCB;
                retCB = nullptr;
            }
            if (work != nullptr) {
                delete work;
                work = nullptr;
            }
        }
    };
    std::vector<std::string> permissionList = param.permissionList;
    std::vector<int> permissionsState = param.permissionsState;
    ability->RequestPermissionsFromUser(permissionList, permissionsState, std::move(resultTask));
    HILOG_DEBUG("AbilityProcess::StartGrantExtension end");
}

void AbilityProcess::ResultCallbackJSThreadWorker(uv_work_t* work, int status)
{
    HILOG_DEBUG("ResultCallbackJSThreadWorker is called.");
    if (work == nullptr) {
        HILOG_ERROR("ResultCallbackJSThreadWorker, uv_queue_work input work is nullptr");
        return;
    }
    RequestPermissionsResultCallback* retCB = static_cast<RequestPermissionsResultCallback *>(work->data);
    if (retCB == nullptr) {
        HILOG_ERROR("ResultCallbackJSThreadWorker, retCB is nullptr");
        delete work;
        work = nullptr;
        return;
    }

    std::lock_guard<std::mutex> lock(permissionRequestMutex_);
    auto callbackKey = retCB->callbackKey;
    auto iter = permissionRequestCallbackMap_.find(callbackKey);
    if (iter != permissionRequestCallbackMap_.end() && iter->second) {
        auto task = iter->second;
        if (task) {
            HILOG_DEBUG("calling js task.");
            task(retCB->permissions, retCB->grantResults);
        }
        permissionRequestCallbackMap_.erase(iter);
    }

    delete retCB;
    retCB = nullptr;
    delete work;
    work = nullptr;
}
}  // namespace AppExecFwk
}  // namespace OHOS
