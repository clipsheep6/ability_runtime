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

#include "exit_resident_process_info.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
}
ExitResidentProcessInfo::~ExitResidentProcessInfo() {}

ExitResidentProcessInfo::ExitResidentProcessInfo() {}

ExitResidentProcessInfo &ExitResidentProcessInfo::GetInstance()
{
    static ExitResidentProcessInfo utils;
    return utils;
}

bool ExitResidentProcessInfo::IsMemorySizeSufficent() const
{
    return currentMemorySizeState_ == MemorySizeState::MEMORY_SIZE_SUFFICENT;
}

void ExitResidentProcessInfo::SetCurrentMemorySizeState(MemorySizeState memorySizeState)
{
    currentMemorySizeState_ = memorySizeState;
}

void ExitResidentProcessInfo::ClearExitResidentBundleNames()
{
    exitResidentBundleNames_.clear();
}

void ExitResidentProcessInfo::AddExitResidentBundleName(std::string bundleName)
{
    exitResidentBundleNames_.emplace_back(bundleName);
}

bool ExitResidentProcessInfo::GetExitResidentBundleNames(std::vector<std::string> &bundleNames) const
{
    if (exitResidentBundleNames_.empty()) {
        HILOG_DEBUG("No Resident Process exit");
        return false;
    }
    bundleNames = exitResidentBundleNames_;
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
