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

#include "app_module_checker.h"

#include "module_checker_delegate.h"
#include "utils/log.h"

bool AppModuleChecker::CheckModuleLoadable(const char *moduleName,
                                           std::unique_ptr<ApiAllowListChecker> &apiAllowListChecker)
{
    apiAllowListChecker = nullptr;
    HILOG_DEBUG("check blocklist, moduleName = %{public}s, processExtensionType_ = %{public}d",
        moduleName, static_cast<int32_t>(processExtensionType_));
    const auto& blockListIter = moduleBlocklist_.find(processExtensionType_);
    if (blockListIter == moduleBlocklist_.end()) {
        return true;
    }
    auto blockList = blockListIter->second;
    if (blockList.find(moduleName) == blockList.end()) {
        return true;
    }
    return false;
}

bool AppModuleChecker::DiskCheckOnly()
{
    return false;
}