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

#include "application_context_manager.h"

namespace OHOS {
namespace AbilityRuntime {
ApplicationContextManager::ApplicationContextManager()
{}

ApplicationContextManager::~ApplicationContextManager()
{
    if (!applicationContextObj_) {
        applicationContextObj_.reset();
    }
}

ApplicationContextManager& ApplicationContextManager::GetApplicationContextManager()
{
    static ApplicationContextManager resoureManager;
    return resoureManager;
}

void ApplicationContextManager::AddGlobalObject(std::shared_ptr<NativeReference> applicationContextObj)
{
    applicationContextObj_ = applicationContextObj;
}

std::shared_ptr<NativeReference> ApplicationContextManager::GetGlobalObject()
{
    return applicationContextObj_;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
