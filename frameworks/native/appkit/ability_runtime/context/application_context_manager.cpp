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

#include "application_context_manager.h"

namespace OHOS {
namespace AbilityRuntime {
ApplicationContextManager::ApplicationContextManager()
{}

ApplicationContextManager::~ApplicationContextManager()
{
    std::lock_guard<std::mutex> lock(applicationContextMutex_);
    for (auto &iter : applicationContextMap_) {
        iter.second.reset();
    }
}

ApplicationContextManager& ApplicationContextManager::GetApplicationContextManager()
{
    static ApplicationContextManager applicationContextManager;
    return applicationContextManager;
}

void ApplicationContextManager::AddGlobalObject(NativeEngine &engine,
    std::shared_ptr<NativeReference> applicationContextObj)
{
    std::lock_guard<std::mutex> lock(applicationContextMutex_);
    auto key = reinterpret_cast<napi_env>(&engine);
    auto iter = applicationContextMap_.find(key);
    if (iter == applicationContextMap_.end()) {
        applicationContextMap_[key] = applicationContextObj;
        return;
    }
    if (iter->second != nullptr) {
        iter->second.reset();
        iter->second = nullptr;
    }
    iter->second = applicationContextObj;
}

std::shared_ptr<NativeReference> ApplicationContextManager::GetGlobalObject(NativeEngine &engine)
{
    std::lock_guard<std::mutex> lock(applicationContextMutex_);
    auto key = reinterpret_cast<napi_env>(&engine);
    return applicationContextMap_[key];
}
}  // namespace AbilityRuntime
}  // namespace OHOS
