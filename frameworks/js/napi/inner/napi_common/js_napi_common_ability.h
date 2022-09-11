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

#ifndef OHOS_ABILITY_RUNTIME_JS_NAPI_COMMON_ABILITY_H
#define OHOS_ABILITY_RUNTIME_JS_NAPI_COMMON_ABILITY_H
#include "ability_connect_callback_stub.h"
#include "ability_info.h"
#include "ability_manager_errors.h"
#include "application_info.h"
#include "feature_ability_common.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AppExecFwk {
class JsNapiCommon {
public:
    JsNapiCommon();
    virtual ~JsNapiCommon() = default;

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

    NativeValue* JsGetContext(NativeEngine &engine, NativeCallbackInfo &info, const AbilityType abilityType);
    NativeValue* JsGetDisplayOrientation(
        NativeEngine &engine, NativeCallbackInfo &info, const AbilityType abilityType);
    bool CheckAbilityType(const AbilityType typeWant);
    bool UnwarpVerifyPermissionParams(NativeEngine &engine, NativeCallbackInfo &info, JsPermissionOptions &options);
    bool GetStringsValue(NativeEngine &engine, NativeValue *object, std::vector<std::string> &strList);
    bool GetPermissionOptions(NativeEngine &engine, NativeValue *object, JsPermissionOptions &strVal);
    std::string ConvertErrorCode(int32_t errCode);

    Ability *ability_;
    int32_t value_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_NAPI_COMMON_ABILITY_H