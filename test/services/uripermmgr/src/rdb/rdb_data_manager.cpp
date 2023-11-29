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

#include "rdb_data_manager.h"

#include "scope_guard.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {

RdbDataManager::RdbDataManager(const RdbConfig &rdbConfig) : rdbConfig_(rdbConfig) {}

RdbDataManager::~RdbDataManager() {}

void RdbDataManager::ClearCache()
{
    NativeRdb::RdbHelper::ClearCache();
}

std::shared_ptr<NativeRdb::RdbStore> RdbDataManager::GetRdbStore()
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ != nullptr) {
        return rdbStore_;
    }
    NativeRdb::RdbStoreConfig rdbStoreConfig(rdbConfig_.dbPath + rdbConfig_.dbName);
    rdbStoreConfig.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    int32_t errCode = NativeRdb::E_OK;
    AbilityRdbOpenCallback abilityRdbOpenCallback(rdbConfig_);
    rdbStore_ = NativeRdb::RdbHelper::GetRdbStore(rdbStoreConfig, rdbConfig_.version, abilityRdbOpenCallback, errCode);
    return rdbStore_;
}

bool RdbDataManager::InsertData(const NativeRdb::ValuesBucket &valuesBucket)
{
    HILOG_DEBUG("InsertData start");
    auto rdbStore = GetRdbStore();
    if (rdbStore == nullptr) {
        HILOG_ERROR("RdbStore is null");
        return false;
    }

    int64_t rowId = -1;
    auto ret = rdbStore->InsertWithConflictResolution(rowId, rdbConfig_.tableName, valuesBucket,
        NativeRdb::ConflictResolution::ON_CONFLICT_REPLACE);
    return ret == NativeRdb::E_OK;
}

bool RdbDataManager::BatchInsert(int64_t &outInsertNum, const std::vector<NativeRdb::ValuesBucket> &valuesBuckets)
{
    auto rdbStore = GetRdbStore();
    if (rdbStore == nullptr) {
        HILOG_ERROR("RdbStore is null");
        return false;
    }
    auto ret = rdbStore->BatchInsert(outInsertNum, rdbConfig_.tableName, valuesBuckets);
    return ret == NativeRdb::E_OK;
}

bool RdbDataManager::UpdateData(const NativeRdb::ValuesBucket &valuesBucket,
    const NativeRdb::AbsRdbPredicates &absRdbPredicates)
{
    auto rdbStore = GetRdbStore();
    if (rdbStore == nullptr) {
        HILOG_ERROR("RdbStore is null");
        return false;
    }
    if (absRdbPredicates.GetTableName() != rdbConfig_.tableName) {
        HILOG_ERROR("RdbStore table is invalid");
        return false;
    }
    int32_t rowId = -1;
    auto ret = rdbStore->Update(rowId, valuesBucket, absRdbPredicates);
    return ret == NativeRdb::E_OK;
}

bool RdbDataManager::DeleteData(const NativeRdb::AbsRdbPredicates &absRdbPredicates)
{
    auto rdbStore = GetRdbStore();
    if (rdbStore == nullptr) {
        HILOG_ERROR("RdbStore is null");
        return false;
    }
    if (absRdbPredicates.GetTableName() != rdbConfig_.tableName) {
        HILOG_ERROR("RdbStore table is invalid");
        return false;
    }
    int32_t rowId = -1;
    auto ret = rdbStore->Delete(rowId, absRdbPredicates);
    return ret == NativeRdb::E_OK;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> RdbDataManager::QueryData(
    const NativeRdb::AbsRdbPredicates &absRdbPredicates)
{
    auto rdbStore = GetRdbStore();
    if (rdbStore == nullptr) {
        HILOG_ERROR("RdbStore is null");
        return nullptr;
    }
    if (absRdbPredicates.GetTableName() != rdbConfig_.tableName) {
        HILOG_ERROR("RdbStore table is invalid");
        return nullptr;
    }
    auto absSharedResultSet = rdbStore->Query(absRdbPredicates, std::vector<std::string>());
    if (absSharedResultSet == nullptr || !absSharedResultSet->HasBlock()) {
        HILOG_ERROR("absSharedResultSet failed");
        return nullptr;
    }
    return absSharedResultSet;
}

bool RdbDataManager::CreateTable()
{
    std::string createTableSql;
    if (rdbConfig_.createTableSql.empty()) {
        createTableSql = std::string("CREATE TABLE IF NOT EXISTS " + rdbConfig_.tableName +
            "(KEY TEXT NOT NULL PRIMARY KEY, VALUE TEXT NOT NULL);");
    } else {
        createTableSql = rdbConfig_.createTableSql;
    }

    auto rdbStore = GetRdbStore();
    if (rdbStore == nullptr) {
        HILOG_ERROR("RdbStore is null");
        return false;
    }
    int ret = rdbStore->ExecuteSql(createTableSql);
    if (ret != NativeRdb::E_OK) {
        HILOG_ERROR("CreateTable failed, ret: %{public}d", ret);
        return false;
    }
    for (const auto &sql : rdbConfig_.insertColumnSql) {
        int32_t insertRet = rdbStore->ExecuteSql(sql);
        if (insertRet != NativeRdb::E_OK) {
            HILOG_WARN("ExecuteSql insertColumnSql failed, insertRet: %{public}d", insertRet);
        }
    }
    return true;
}
} // namespace AAFwk
} // namespace OHOS
