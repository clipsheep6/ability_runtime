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
namespace {
void HandleClean(void *data)
{
    auto env = reinterpret_cast<napi_env>(data);
    ApplicationContextManager::GetApplicationContextManager().RemoveGlobalObject(env);
}
}
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

void ApplicationContextManager::AddGlobalObject(napi_env env,
    std::shared_ptr<NativeReference> applicationContextObj)
{
    napi_add_env_cleanup_hook(env, HandleClean, env);
    std::lock_guard<std::mutex> lock(applicationContextMutex_);
    auto iter = applicationContextMap_.find(env);
    if (iter == applicationContextMap_.end()) {
        applicationContextMap_[env] = applicationContextObj;
        return;
    }
    if (iter->second != nullptr) {
        iter->second.reset();
        iter->second = nullptr;
    }
    iter->second = applicationContextObj;
}

std::shared_ptr<NativeReference> ApplicationContextManager::GetGlobalObject(napi_env env)
{
    std::lock_guard<std::mutex> lock(applicationContextMutex_);
    return applicationContextMap_[env];
}

void ApplicationContextManager::RemoveGlobalObject(napi_env env)
{
    std::lock_guard<std::mutex> lock(applicationContextMutex_);
    auto iter = applicationContextMap_.find(env);
    if (iter != applicationContextMap_.end() && iter->second != nullptr) {
        iter->second.reset();
        iter->second = nullptr;
        applicationContextMap_.erase(env);
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS
