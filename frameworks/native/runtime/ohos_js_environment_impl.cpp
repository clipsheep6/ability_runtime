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

#include "ohos_js_environment_impl.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
OHOSJsEnvironmentImpl::OHOSJsEnvironmentImpl()
{
    HILOG_DEBUG("called");
}

OHOSJsEnvironmentImpl::~OHOSJsEnvironmentImpl()
{
    HILOG_DEBUG("called");
}

void OHOSJsEnvironmentImpl::PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime)
{
    HILOG_DEBUG("called");
}

void OHOSJsEnvironmentImpl::RemoveTask(const std::string& name)
{
    HILOG_DEBUG("called");
}

void OHOSJsEnvironmentImpl::InitTimerModule()
{
    HILOG_DEBUG("called");
}

void OHOSJsEnvironmentImpl::InitConsoleLogModule()
{
    HILOG_DEBUG("called");
}

void OHOSJsEnvironmentImpl::InitWorkerModule()
{
    HILOG_DEBUG("called");
}

void OHOSJsEnvironmentImpl::InitSyscapModule()
{
    HILOG_DEBUG("called");
}
} // namespace AbilityRuntime
} // namespace OHOS
