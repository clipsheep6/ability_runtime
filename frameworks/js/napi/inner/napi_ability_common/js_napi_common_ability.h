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

#ifndef OHOS_ABILITY_RUNTIME_JS_NAPI_COMMON_ABILITY_H
#define OHOS_ABILITY_RUNTIME_JS_NAPI_COMMON_ABILITY_H

#include "ability_connect_callback_stub.h"
#include "ability_info.h"
#include "ability_manager_errors.h"
#include "application_info.h"
#include "feature_ability_common.h"
#include "js_free_install_observer.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AppExecFwk {
class JsNapiCommon {
public:
    JsNapiCommon();
    virtual ~JsNapiCommon();

    struct JsPermissionOptions {
        bool uidFlag = false;
        bool pidFlag = false;
        int32_t uid = 0;
        int32_t pid = 0;
    };

    struct JsApplicationInfo {
        ApplicationInfo appInfo;
    };

    struct JsBundleName {
        std::string name = "";
    };
    typedef JsBundleName JsProcessName;
    typedef JsBundleName JsCallingBundleName;
    typedef JsBundleName JsOrCreateLocalDir;
    typedef JsBundleName JsFilesDir;
    typedef JsBundleName JsCacheDir;
    typedef JsBundleName JsCtxAppType;
    typedef JsBundleName JsOrCreateDistributedDir;
    typedef JsBundleName JsAppType;

    struct JsElementName {
        std::string deviceId = "";
        std::string bundleName = "";
        std::string abilityName = "";
        std::string uri = "";
        std::string shortName = "";
    };

    struct JsProcessInfo {
        std::string processName = "";
        pid_t pid = 0;
    };

    struct JsConfigurations {
        bool status;
    };
    typedef JsConfigurations JsDrawnCompleted;

    struct JsHapModuleInfo {
        HapModuleInfo hapModInfo;
    };

    struct JsAbilityInfoInfo {
        AbilityInfo abilityInfo;
    };

    struct JsWant {
        Want want;
    };

    bool CheckAbilityType(const AbilityType typeWant);
    std::string ConvertErrorCode(int32_t errCode);
    sptr<NAPIAbilityConnection> FindConnectionLocked(const Want &want, int64_t &id);
    void RemoveAllCallbacksLocked();
    bool CreateConnectionAndConnectAbilityLocked(
        std::shared_ptr<ConnectionCallback> callback, const Want &want, int64_t &id);
    void RemoveConnectionLocked(const Want &want);
    napi_value HandleJsConnectAbilityError(napi_env env, std::shared_ptr<ConnectionCallback> &connectionCallback,
        const Want &want, int32_t errorVal);

    napi_value OnFindAbilityConnection(napi_env env, sptr<NAPIAbilityConnection> &abilityConnection,
        std::shared_ptr<ConnectionCallback> &connectionCallback, const Want &want, int64_t id);
    napi_value JsConnectAbility(napi_env env, napi_callback_info info, const AbilityType abilityType);

    void SetJsDisConnectAbilityCallback(std::shared_ptr<int32_t> &errorVal, const AbilityType &abilityType,
        sptr<NAPIAbilityConnection> &abilityConnection, AbilityRuntime::NapiAsyncTask::ExecuteCallback &execute,
        AbilityRuntime::NapiAsyncTask::CompleteCallback &complete);
    napi_value JsDisConnectAbility(napi_env env, napi_callback_info info, const AbilityType abilityType);
    napi_value JsGetContext(napi_env env, const napi_callback_info info, const AbilityType abilityType);
    napi_value JsGetFilesDir(napi_env env, napi_callback_info info, const AbilityType abilityType);
    napi_value JsIsUpdatingConfigurations(napi_env env, napi_callback_info info, const AbilityType abilityType);
    napi_value JsPrintDrawnCompleted(napi_env env, napi_callback_info info, const AbilityType abilityType);
    napi_value JsGetCacheDir(napi_env env, napi_callback_info info, const AbilityType abilityType);
    napi_value JsGetCtxAppType(napi_env env, napi_callback_info info, const AbilityType abilityType);
    napi_value JsGetCtxHapModuleInfo(napi_env env, napi_callback_info info, const AbilityType abilityType);
    napi_value JsGetAppVersionInfo(napi_env env, napi_callback_info info, const AbilityType abilityType);
    napi_value JsGetApplicationContext(
        napi_env env, napi_callback_info info, const AbilityType abilityType);
    napi_value JsGetCtxAbilityInfo(napi_env env, napi_callback_info info, const AbilityType abilityType);
    napi_value JsGetOrCreateDistributedDir(
        napi_env env, napi_callback_info info, const AbilityType abilityType);
#ifdef SUPPORT_GRAPHICS
    napi_value JsGetDisplayOrientation(
        napi_env env, napi_callback_info info, const AbilityType abilityType);
#endif
    napi_value JsGetWant(napi_env env, napi_callback_info info, const AbilityType abilityType);
    napi_value JsTerminateAbility(napi_env env, AbilityRuntime::NapiCallbackInfo& info);
    void SetJsStartAbilityExecuteCallback(std::shared_ptr<int32_t> &errorVal, AbilityType &abilityType,
        std::shared_ptr<CallAbilityParam> &param, AbilityRuntime::NapiAsyncTask::ExecuteCallback &execute);
    napi_value JsStartAbility(napi_env env, napi_callback_info info, AbilityType abilityType);
    napi_value JsGetExternalCacheDir(napi_env env, napi_callback_info info, AbilityType abilityType);

    napi_value CreateProcessInfo(napi_env env, const std::shared_ptr<JsProcessInfo> &processInfo);
    napi_value CreateElementName(napi_env env, const std::shared_ptr<JsElementName> &elementName);
    napi_value CreateHapModuleInfo(napi_env env, const std::shared_ptr<JsHapModuleInfo> &hapModInfo);
    napi_value CreateModuleInfo(napi_env env, const ModuleInfo &modInfo);
    napi_value CreateModuleInfos(napi_env env, const std::vector<ModuleInfo> &moduleInfos);
    napi_value CreateAppInfo(napi_env env, const ApplicationInfo &appInfo);
    napi_value CreateAppInfo(napi_env env, const std::shared_ptr<JsApplicationInfo> &appInfo);
    napi_value CreateAbilityInfo(napi_env env, const AbilityInfo &abilityInfo);
    napi_value CreateAbilityInfo(napi_env env, const std::shared_ptr<JsAbilityInfoInfo> &abilityInfo);
    napi_value CreateAbilityInfos(napi_env env, const std::vector<AbilityInfo> &abilityInfos);
    napi_value CreateAppVersionInfo(napi_env env, const std::shared_ptr<JsApplicationInfo> &appInfo);
    napi_value CreateWant(napi_env env, const std::shared_ptr<JsWant> &want);
    bool UnwrapVerifyPermissionParams(napi_env env, napi_callback_info info, JsPermissionOptions &options);
    bool GetStringsValue(napi_env env, napi_value object, std::vector<std::string> &strList);
    bool GetPermissionOptions(napi_env env, napi_value object, JsPermissionOptions &options);
    void AddFreeInstallObserver(napi_env env, const AAFwk::Want &want, napi_value callback, napi_value* result);
    Ability *ability_;
    sptr<AbilityRuntime::JsFreeInstallObserver> freeInstallObserver_ = nullptr;
};

enum {
    CONNECTION_STATE_DISCONNECTED = -1,

    CONNECTION_STATE_CONNECTED = 0,

    CONNECTION_STATE_CONNECTING = 1
};

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
    void Reset();

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
    size_t RemoveAllCallbacks(ConnectRemoveKeyType key);

private:
    std::list<std::shared_ptr<ConnectionCallback>> callbacks_;
    AppExecFwk::ElementName element_;
    sptr<IRemoteObject> serviceRemoteObject_ = nullptr;
    int connectionState_ = CONNECTION_STATE_DISCONNECTED;
    mutable std::mutex lock_;
};

struct ConnectionKey {
    Want want;
    int64_t id;
};

struct key_compare {
    bool operator()(const ConnectionKey &key1, const ConnectionKey &key2) const
    {
        if (key1.id < key2.id) {
            return true;
        }
        return false;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_NAPI_COMMON_ABILITY_H
