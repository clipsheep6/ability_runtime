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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_JS_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_NAPI_JS_CONTEXT_H

#include "js_napi_common_ability.h"

#define MODE 0771
namespace OHOS {
namespace AppExecFwk {
struct CallAbilityPermissionParam {
    std::vector<std::string> permission_list;
    int requestCode = 0;
    int syncOption = false;
};
class NapiJsContext : public JsNapiCommon {
public:
    NapiJsContext() = default;
    ~NapiJsContext() override = default;

    static void Finalizer(napi_env env, void *data, void *hint);

    static napi_value JsRequestPermissionsFromUser(napi_env env, napi_callback_info info);
    static napi_value JsGetBundleName(napi_env env, napi_callback_info info);
    static napi_value JsVerifyPermission(napi_env env, napi_callback_info info);
    static napi_value JsGetApplicationInfo(napi_env env, napi_callback_info info);
    static napi_value JsGetProcessInfo(napi_env env, napi_callback_info info);
    static napi_value JsGetElementName(napi_env env, napi_callback_info info);
    static napi_value JsGetProcessName(napi_env env, napi_callback_info info);
    static napi_value JsGetCallingBundle(napi_env env, napi_callback_info info);
    static napi_value JsGetOrCreateLocalDir(napi_env env, napi_callback_info info);
    static napi_value JsGetFilesDir(napi_env env, napi_callback_info info);
    static napi_value JsIsUpdatingConfigurations(napi_env env, napi_callback_info info);
    static napi_value JsPrintDrawnCompleted(napi_env env, napi_callback_info info);
    static napi_value JsGetCacheDir(napi_env env, napi_callback_info info);
    static napi_value JsGetCtxAppType(napi_env env, napi_callback_info info);
    static napi_value JsGetCtxHapModuleInfo(napi_env env, napi_callback_info info);
    static napi_value JsGetAppVersionInfo(napi_env env, napi_callback_info info);
    static napi_value JsGetApplicationContext(napi_env env, napi_callback_info info);
    static napi_value JsGetCtxAbilityInfo(napi_env env, napi_callback_info info);
    static napi_value JsSetShowOnLockScreen(napi_env env, napi_callback_info info);
    static napi_value JsGetOrCreateDistributedDir(napi_env env, napi_callback_info info);
    static napi_value JsSetWakeUpScreen(napi_env env, napi_callback_info info);
    static napi_value JsSetDisplayOrientation(napi_env env, napi_callback_info info);
    static napi_value JsGetDisplayOrientation(napi_env env, napi_callback_info info);
    static napi_value JsGetExternalCacheDir(napi_env env, napi_callback_info info);

    bool DataInit(napi_env env);

private:
#ifdef SUPPORT_GRAPHICS
    napi_value OnSetShowOnLockScreen(napi_env env, napi_callback_info info);
    napi_value OnSetWakeUpScreen(napi_env env, napi_callback_info info);
    napi_value OnSetDisplayOrientation(napi_env env, napi_callback_info info);
#endif

    napi_value OnRequestPermissionsFromUser(napi_env env, napi_callback_info info);
    napi_value OnGetBundleName(napi_env env, napi_callback_info info);
    napi_value OnVerifyPermission(napi_env env, napi_callback_info info);
    napi_value OnGetApplicationInfo(napi_env env, napi_callback_info info);
    napi_value OnGetProcessInfo(napi_env env, napi_callback_info info);
    napi_value OnGetElementName(napi_env env, napi_callback_info info);
    napi_value OnGetProcessName(napi_env env, napi_callback_info info);
    napi_value OnGetCallingBundle(napi_env env, napi_callback_info info);
    napi_value OnGetOrCreateLocalDir(napi_env env, napi_callback_info info);
};
} // AppExecFwk
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_NAPI_JS_CONTEXT_H