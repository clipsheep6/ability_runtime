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

#ifndef OHOS_ABILITY_RUNTIME_FEATURE_ABILITY_COMMON_H
#define OHOS_ABILITY_RUNTIME_FEATURE_ABILITY_COMMON_H
#include "ability.h"
#include "ability_info.h"
#include "application_info.h"
#include "hap_module_info.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "napi_common.h"
#include "napi_common_util.h"
#include "pac_map.h"
#include "want.h"

using Want = OHOS::AAFwk::Want;
using Ability = OHOS::AppExecFwk::Ability;
using AbilityStartSetting = OHOS::AppExecFwk::AbilityStartSetting;

namespace OHOS {
namespace AppExecFwk {
class FeatureAbility;

struct CallAbilityParam {
    Want want;
    int requestCode = 0;
    bool forResultOption = false;
    std::shared_ptr<AbilityStartSetting> setting = nullptr;
};

struct OnAbilityCallback {
    int requestCode = 0;
    int resultCode = 0;
    Want resultData;
    CallbackInfo cb;
};

struct ContinueAbilityOptionsInfo {
    bool reversible = false;
    std::string deviceId;
};

struct AsyncCallbackInfo {
    CallbackInfo cbInfo;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    Ability *ability = nullptr;
    AbilityRuntime::WantAgent::WantAgent *wantAgent = nullptr;
    CallAbilityParam param;
    CallbackInfo aceCallback;
    bool native_result;
    AbilityType abilityType = AbilityType::UNKNOWN;
    int errCode = 0;
    ContinueAbilityOptionsInfo optionInfo;
#ifdef SUPPORT_GRAPHICS
    sptr<OHOS::Rosen::Window> window;
#endif
};

struct CBBase {
    CallbackInfo cbInfo;
    napi_async_work asyncWork;
    napi_deferred deferred;
    Ability *ability = nullptr;
    AbilityType abilityType = AbilityType::UNKNOWN;
    int errCode = 0;
};

struct AppInfoCB {
    CBBase cbBase;
    ApplicationInfo appInfo;
};

struct AppTypeCB {
    CBBase cbBase;
    std::string name;
};
struct AbilityInfoCB {
    CBBase cbBase;
    AbilityInfo abilityInfo;
};

struct AbilityNameCB {
    CBBase cbBase;
    std::string name;
};

struct ProcessInfoCB {
    CBBase cbBase;
    pid_t pid = 0;
    std::string processName;
};

struct ProcessNameCB {
    CBBase cbBase;
    std::string processName;
};

struct CallingBundleCB {
    CBBase cbBase;
    std::string callingBundleName;
};

struct GetOrCreateLocalDirCB {
    CBBase cbBase;
    std::string rootDir;
};

struct DatabaseDirCB {
    CBBase cbBase;
    std::string dataBaseDir;
};

struct PreferencesDirCB {
    CBBase cbBase;
    std::string preferencesDir;
};

struct ElementNameCB {
    CBBase cbBase;
    std::string deviceId;
    std::string bundleName;
    std::string abilityName;
    std::string uri;
    std::string shortName;
};

struct HapModuleInfoCB {
    CBBase cbBase;
    HapModuleInfo hapModuleInfo;
};

struct AppVersionInfo {
    std::string appName;
    std::string versionName;
    int32_t versionCode = 0;
};

struct AppVersionInfoCB {
    CBBase cbBase;
    AppVersionInfo appVersionInfo;
};

struct DataAbilityHelperCB {
    CBBase cbBase;
    napi_ref uri = nullptr;
    napi_value result = nullptr;
};

class NAPIAbilityConnection;
struct AbilityConnectionCB {
    napi_env env;
    napi_ref callback[3] = {nullptr};  // onConnect/onDisconnect/onFailed
    int resultCode = 0;
    ElementName elementName;
    sptr<IRemoteObject> connection;
};
struct ConnectAbilityCB {
    CBBase cbBase;
    Want want;
    sptr<NAPIAbilityConnection> abilityConnection;
    AbilityConnectionCB abilityConnectionCB;
    int64_t id;
    bool result;
    int errCode = 0;
};

struct ShowOnLockScreenCB {
    CBBase cbBase;
    bool isShow;
};

struct SetWakeUpScreenCB {
    CBBase cbBase;
    bool wakeUp;
};

static inline std::string NapiValueToStringUtf8(napi_env env, napi_value value)
{
    std::string result = "";
    return UnwrapStringFromJS(env, value, result);
}

static inline bool NapiValueToArrayStringUtf8(napi_env env, napi_value param, std::vector<std::string> &result)
{
    return UnwrapArrayStringFromJS(env, param, result);
}
}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* OHOS_ABILITY_RUNTIME_FEATURE_ABILITY_COMMON_H */
