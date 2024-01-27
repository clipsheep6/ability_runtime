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

/**
 * @addtogroup Ability_Runtime
 * @{
 *
 * @brief Provides ArkTS runtime environment.

 *
 * @since 12
 * @version 1.0
 */

/**
 * @file native_rumtime.h
 *
 * @brief Declare interfaces for creating and destroying Arkts runtime environments.
 *
 * @library libruntime.z.so
 * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
 * @since 12
 * @version 1.0
 */
#ifndef ABILITY_ABILITY_RUNTIME_NATIVE_RUNTIME_H
#define ABILITY_ABILITY_RUNTIME_NATIVE_RUNTIME_H

#include <stdint.h>
#include "napi/native_api.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Obtains the application info based on the The current bundle.
 *
 * @param env: The environment that the API is invoked under.
 * @return 0 - success.
 *         1 
 * @since 12
 * @version 1.0
 */
int32_t OH_NativeAbility_Create_NapiEnv(napi_env *env);


/**
 * @brief Obtains the application info based on the The current bundle.
 *
 * @param env: The environment that the API is invoked under.
 * @return 0 - success.
 *         1
 * @since 12
 * @version 1.0
 */
int32_t OH_NativeAbility_Destory_NapiEnv(napi_env *env);

#ifdef __cplusplus
};
#endif
/** @} */
#endif  // ABILITY_ABILITY_RUNTIME_RUNTIME_H