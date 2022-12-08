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

#include "js_error_manager.h"
#include "native_engine/native_engine.h"

extern "C" __attribute__((constructor))
#ifdef ENABLE_ERRCODE
void NAPI_app_ability_ErrorManager_AutoRegister()
{
    NativeModule newModuleInfo = {
        .name = "app.ability.errorManager",
        .fileName = "app/ability/errormanager_napi.so/error_manager.js",
        .registerCallback = OHOS::AbilityRuntime::JsErrorManagerInit,
    };

    auto moduleManager = NativeModuleManager::GetInstance();
    moduleManager->Register(&newModuleInfo);
}
#else
void NAPI_application_ErrorManager_AutoRegister()
{
    NativeModule newModuleInfo = {
        .name = "application.errorManager",
        .fileName = "application/errormanager_napi.so/error_manager.js",
        .registerCallback = OHOS::AbilityRuntime::JsErrorManagerInit,
    };

    auto moduleManager = NativeModuleManager::GetInstance();
    moduleManager->Register(&newModuleInfo);
}
#endif
