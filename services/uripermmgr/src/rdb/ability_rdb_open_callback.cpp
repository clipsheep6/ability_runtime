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
#include "rdb_errno.h"
#include "ability_rdb_open_callback.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
AbilityRdbOpenCallback::AbilityRdbOpenCallback(const RdbConfig &rdbConfig)
    : rdbConfig_(rdbConfig) {}

int32_t AbilityRdbOpenCallback::OnCreate(NativeRdb::RdbStore &rdbStore)
{
    HILOG_INFO("OnCreate");
    return NativeRdb::E_OK;
}

int32_t AbilityRdbOpenCallback::OnUpgrade(
    NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    HILOG_INFO("OnUpgrade currentVersion: %{plubic}d, targetVersion: %{plubic}d",
        currentVersion, targetVersion);
    return NativeRdb::E_OK;
}

int32_t AbilityRdbOpenCallback::OnDowngrade(
    NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    HILOG_INFO("OnDowngrade  currentVersion: %{plubic}d, targetVersion: %{plubic}d",
        currentVersion, targetVersion);
    return NativeRdb::E_OK;
}

int32_t AbilityRdbOpenCallback::OnOpen(NativeRdb::RdbStore &rdbStore)
{
    HILOG_INFO("OnOpen");
    return NativeRdb::E_OK;
}

int32_t AbilityRdbOpenCallback::onCorruption(std::string databaseFile)
{
    return NativeRdb::E_OK;
}
}  // namespace AAFwk
}  // namespace OHOS
