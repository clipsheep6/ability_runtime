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

    napi_value JsConnectAbility(napi_env env, napi_callback_info info, const AbilityType abilityType);
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
    bool UnwarpVerifyPermissionParams(napi_env env, napi_callback_info info, JsPermissionOptions &options);
    bool GetStringsValue(napi_env env, napi_value object, std::vector<std::string> &strList);
    bool GetPermissionOptions(napi_env env, napi_value object, JsPermissionOptions &options);
    void AddFreeInstallObserver(napi_env env, const AAFwk::Want &want, napi_value callback);
    Ability *ability_;
    sptr<AbilityRuntime::JsFreeInstallObserver> freeInstallObserver_ = nullptr;
private:
    int32_t JsStartAbilityInner(std::shared_ptr<CallAbilityParam> paramObj, AbilityType abilityType);
    int32_t JsDisConnectAbilityInner(AbilityType abilityType, sptr<NAPIAbilityConnection>);
    bool WrapJsConnectFailedResult(napi_env env, const Want &want,
        int32_t errorVal, std::shared_ptr<ConnectionCallback> cb);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_NAPI_COMMON_ABILITY_H
