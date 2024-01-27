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

#include "native_rumtime.h"

#include "js_environment.h"
#include "hilog_wrapper.h"
#include "native_runtime_impl.h"

using NativeRuntimeImpl = OHOS::AbilityRuntime::NativeRuntimeImpl;

int32_t OH_NativeAbility_Create_NapiEnv(napi_env *env)
{
    HILOG_INFO("zhuhan call OH_NativeAbility_Create_NapiEnv");
    auto options = OHOS::AbilityRuntime::JsRuntime::GetChildOptions();
    if (options == nullptr) {
        HILOG_ERROR("zhuhan options is null, it maybe application startup failed!");
        return OHOS::AbilityRuntime::NATIVE_RUNTIME_INNER_ERROR;
    }
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto errCode = NativeRuntimeImpl::GetNativeRuntimeImpl().CreateJsEnv(*options, jsEnv);
    if (errCode != OHOS::AbilityRuntime::NATIVE_RUNTIME_ERR_OK) {
        return errCode;
    }
    HILOG_INFO("zhuhan CreateJsEnv ok");
    *env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    if (env == nullptr) {
        HILOG_ERROR("zhuhan CreateJsEnv failed");
        return OHOS::AbilityRuntime::NATIVE_RUNTIME_INNER_ERROR;
    }
    return NativeRuntimeImpl::GetNativeRuntimeImpl().Init(*options, *env);
}

int32_t OH_NativeAbility_Destory_NapiEnv(napi_env *env)
{
    HILOG_INFO("zhuhan call OH_NativeAbility_Destory_NapiEnv");
    auto errCode = NativeRuntimeImpl::GetNativeRuntimeImpl().RemoveJsEnv(*env);
    if (errCode == OHOS::AbilityRuntime::NATIVE_RUNTIME_ERR_OK) {
        *env = nullptr;
    }
    return errCode;
}