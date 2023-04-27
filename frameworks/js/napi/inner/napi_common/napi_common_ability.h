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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_H
#define OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_H

#include <memory>
#include <mutex>
#include <list>

#include "ability_connect_callback_stub.h"
#include "ability_info.h"
#include "ability_manager_errors.h"
#include "application_info.h"
#include "feature_ability_common.h"

namespace OHOS {
namespace AppExecFwk {
const std::int32_t STR_MAX_SIZE = 128;
napi_status SetGlobalClassContext(napi_env env, napi_value constructor);
napi_value GetGlobalClassContext(napi_env env);

napi_status SaveGlobalDataAbilityHelper(napi_env env, napi_value constructor);
napi_value GetGlobalDataAbilityHelper(napi_env env);
bool& GetDataAbilityHelperStatus();

napi_value WrapAppInfo(napi_env env, const ApplicationInfo &appInfo);
napi_value WrapProperties(napi_env env, const std::vector<std::string> properties, const std::string &proName,
    napi_value &result);
napi_value WrapModuleInfos(napi_env env, const ApplicationInfo &appInfo, napi_value &result);
int32_t GetStartAbilityErrorCode(ErrCode innerErrorCode);

/**
 * @brief Get Files Dir.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetFilesDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Get OrCreateDistribute Dir.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetOrCreateDistributedDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

napi_value NAPI_GetCacheDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

napi_value NAPI_GetExternalCacheDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

napi_value NAPI_IsUpdatingConfigurationsCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

napi_value NAPI_PrintDrawnCompletedCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Obtains the type of this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAppTypeCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Get the display orientation of the main window.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
#ifdef SUPPORT_GRAPHICS
napi_value NAPI_GetDisplayOrientationCommon(napi_env env, napi_callback_info info, AbilityType abilityType);
bool UnwrapParamGetDisplayOrientationWrap(napi_env env, size_t argc, napi_value *argv,
    AsyncJSCallbackInfo *asyncCallbackInfo);
void GetDisplayOrientationExecuteCallback(napi_env env, void *data);
#endif

/**
 * @brief Obtains information about the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Obtains the HapModuleInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetHapModuleInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Obtains the AppVersionInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAppVersionInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AsyncCallbackInfo on success, nullptr on failure
 */
AsyncCallbackInfo *CreateAsyncCallbackInfo(napi_env env);
/**
 * @brief Get context.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetContextCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Get want.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetWantCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Obtains the class name in this ability name, without the prefixed bundle name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityNameCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief startAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_StartAbilityCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief stopAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_StopAbilityCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Obtains the continue ability Info this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param value The value passed into the info.
 * @param info The continue ability options info
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value GetContinueAbilityOptionsInfoCommon(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info);

/**
 * @brief Obtains the continue ability can reversible or not
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param value The value passed into the info.
 * @param info The continue ability options info
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value GetContinueAbilityOptionsReversible(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info);

/**
 * @brief Obtains the continue ability Info this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param value The value passed into the info.
 * @param info The continue ability options info
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value GetContinueAbilityOptionsDeviceID(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info);

bool UnwrapAbilityStartSetting(napi_env env, napi_value param, AAFwk::AbilityStartSetting &setting);

/**
 * @brief terminateAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_TerminateAbilityCommon(napi_env env, napi_callback_info info);

/**
 * @brief TerminateAbility processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value TerminateAbilityWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo);
napi_value TerminateAbilityAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo);
napi_value TerminateAbilityPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo);

enum {
    CONNECTION_STATE_DISCONNECTED = -1,

    CONNECTION_STATE_CONNECTED = 0,

    CONNECTION_STATE_CONNECTING = 1
};

class JsNapiCommon;
using ConnectRemoveKeyType = JsNapiCommon*;
struct ConnectionCallback {
    ConnectionCallback(napi_env env, napi_value cbInfo, ConnectRemoveKeyType key)
    {
        this->env = env;
        napi_value jsMethod = nullptr;
        napi_get_named_property(env, cbInfo, "onConnect", &jsMethod);
        napi_create_reference(env, jsMethod, 1, &connectCallbackRef);
        napi_get_named_property(env, cbInfo, "onDisconnect", &jsMethod);
        napi_create_reference(env, jsMethod, 1, &disconnectCallbackRef);
        napi_get_named_property(env, cbInfo, "onFailed", &jsMethod);
        napi_create_reference(env, jsMethod, 1, &failedCallbackRef);
        removeKey = key;
    }
    ConnectionCallback(ConnectionCallback &) = delete;
    ConnectionCallback(ConnectionCallback &&other)
        : env(other.env), connectCallbackRef(other.connectCallbackRef),
        disconnectCallbackRef(other.disconnectCallbackRef), failedCallbackRef(other.failedCallbackRef),
        removeKey(other.removeKey)
    {
        other.env = nullptr;
        other.connectCallbackRef = nullptr;
        other.disconnectCallbackRef = nullptr;
        other.failedCallbackRef = nullptr;
        other.removeKey = nullptr;
    }
    const ConnectionCallback &operator=(ConnectionCallback &) = delete;
    const ConnectionCallback &operator=(ConnectionCallback &&other)
    {
        Reset();
        env = other.env;
        connectCallbackRef = other.connectCallbackRef;
        disconnectCallbackRef = other.disconnectCallbackRef;
        failedCallbackRef = other.failedCallbackRef;
        other.env = nullptr;
        other.connectCallbackRef = nullptr;
        other.disconnectCallbackRef = nullptr;
        other.failedCallbackRef = nullptr;
        other.removeKey = nullptr;
        return *this;
    }
    ~ConnectionCallback()
    {
        Reset();
    }
    void Reset()
    {
        if (env) {
            if (connectCallbackRef) {
                napi_delete_reference(env, connectCallbackRef);
                connectCallbackRef = nullptr;
            }
            if (disconnectCallbackRef) {
                napi_delete_reference(env, disconnectCallbackRef);
                disconnectCallbackRef = nullptr;
            }
            if (failedCallbackRef) {
                napi_delete_reference(env, failedCallbackRef);
                failedCallbackRef = nullptr;
            }
            env = nullptr;
        }
        removeKey = nullptr;
    }

    napi_env env = nullptr;
    napi_ref connectCallbackRef = nullptr;
    napi_ref disconnectCallbackRef = nullptr;
    napi_ref failedCallbackRef = nullptr;
    ConnectRemoveKeyType removeKey = nullptr;
};

class NAPIAbilityConnection : public AAFwk::AbilityConnectionStub {
public:
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    void AddConnectionCallback(std::shared_ptr<ConnectionCallback> callback);
    void HandleOnAbilityConnectDone(ConnectionCallback &callback, int resultCode);
    void HandleOnAbilityDisconnectDone(ConnectionCallback &callback, int resultCode);
    int GetConnectionState() const;
    void SetConnectionState(int connectionState);
    size_t GetCallbackSize();
    size_t ReomveAllCallbacks(ConnectRemoveKeyType key);

private:
    std::list<std::shared_ptr<ConnectionCallback>> callbacks_;
    AppExecFwk::ElementName element_;
    sptr<IRemoteObject> serviceRemoteObject_ = nullptr;
    int connectionState_ = CONNECTION_STATE_DISCONNECTED;
    mutable std::mutex lock_;
};

/**
 * @brief acquireDataAbilityHelper processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param dataAbilityHelperCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value AcquireDataAbilityHelperWrap(
    napi_env env, napi_callback_info info, DataAbilityHelperCB *dataAbilityHelperCB);

/**
 * @brief AcquireDataAbilityHelper.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_AcquireDataAbilityHelperCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

napi_value ConvertAbilityInfo(napi_env env, const AbilityInfo &abilityInfo);

/**
 * @brief start background running.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_StartBackgroundRunningCommon(napi_env env, napi_callback_info info);

/**
 * @brief cancel background running.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_CancelBackgroundRunningCommon(napi_env env, napi_callback_info info);

bool CheckAbilityType(const CBBase *cbBase);

enum ErrorCode {
    NO_ERROR = 0,
    INVALID_PARAMETER = -1,
    ABILITY_NOT_FOUND = -2,
    PERMISSION_DENY = -3,
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_H
