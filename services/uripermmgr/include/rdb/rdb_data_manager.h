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

#ifndef OHOS_ABILITY_RUNTIME_RDB_RDB_DATA_MANAGER_H
#define OHOS_ABILITY_RUNTIME_RDB_RDB_DATA_MANAGER_H

#include <mutex>

#include "rdb_config.h"
#include "ability_rdb_open_callback.h"
#include "rdb_helper.h"

namespace OHOS {
namespace AAFwk {
class RdbDataManager {
public:
    RdbDataManager(const RdbConfig &rdbConfig);
    ~RdbDataManager();

    static void ClearCache();
    bool InsertData(const NativeRdb::ValuesBucket &valuesBucket);
    bool BatchInsert(int64_t &outInsertNum, const std::vector<NativeRdb::ValuesBucket> &valuesBuckets);
    bool UpdateData(const NativeRdb::ValuesBucket &valuesBucket,
        const NativeRdb::AbsRdbPredicates &absRdbPredicates);
    bool DeleteData(const NativeRdb::AbsRdbPredicates &absRdbPredicates);
    std::shared_ptr<NativeRdb::AbsSharedResultSet> QueryData(
        const NativeRdb::AbsRdbPredicates &absRdbPredicates);
    bool CreateTable();

private:
    std::shared_ptr<NativeRdb::RdbStore> GetRdbStore();
    std::mutex rdbMutex_;
    std::shared_ptr<NativeRdb::RdbStore> rdbStore_;

    RdbConfig rdbConfig_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
