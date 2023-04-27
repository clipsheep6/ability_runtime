/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "app_recovery_parcel_allocator.h"

namespace OHOS {
namespace AppExecFwk {
AppRecoveryParcelAllocator::AppRecoveryParcelAllocator()
{
}

AppRecoveryParcelAllocator::~AppRecoveryParcelAllocator()
{
}

void* AppRecoveryParcelAllocator::Realloc(void* data __attribute__((unused)), size_t newSize __attribute__((unused)))
{
    return nullptr;
}

void* AppRecoveryParcelAllocator::Alloc(size_t size __attribute__((unused)))
{
    return nullptr;
}

void AppRecoveryParcelAllocator::Dealloc(void* data __attribute__((unused)))
{
    // we do not alloc any new memory, thus no need to dealloc.
}
}  // namespace AbilityRuntime
}  // namespace OHOS