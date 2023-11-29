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

#ifndef OHOS_ABILITY_RUNTIME_URI_PERMISSION_RDB_H
#define OHOS_ABILITY_RUNTIME_URI_PERMISSION_RDB_H


#include <string>
#include <vector>

#include "istorage_manager.h"
#include "rdb_data_manager.h"


namespace OHOS {
namespace AAFwk {

struct RdbGrantInfo {
    std::string uri;
    uint32_t flag;
    const uint32_t fromTokenId;
    const uint32_t targetTokenId;
};

class UriPermissionRdb {
public:
    UriPermissionRdb();
    int32_t AddGrantInfo(const std::string& uri, uint32_t flag, uint32_t fromTokenId, uint32_t targetTokenId);
    int32_t RemoveGrantInfo(uint32_t tokenId, sptr<StorageManager::IStorageManager> storageManager);
    int32_t RemoveGrantInfo(const std::string& uri, uint32_t tokenId,
        sptr<StorageManager::IStorageManager> storageManager);
    int32_t RemoveGrantInfo(const NativeRdb::AbsRdbPredicates& absRdbPredicates,
        sptr<StorageManager::IStorageManager> storageManager);
    bool CheckPersistableUriPermissionProxy(const std::string& uri, uint32_t flag, uint32_t tokenId);
    void ShowAllGrantInfo();

private:
    bool QueryData(const NativeRdb::AbsRdbPredicates& absRdbPredicates, std::vector<RdbGrantInfo>& rdbGrantInfoList,
        int& rowCount);
    bool InsertData(const std::vector<RdbGrantInfo>& rdbGrantInfoList);
    bool UpdateData(const NativeRdb::AbsRdbPredicates& absRdbPredicates, const NativeRdb::ValuesBucket& valuesBucket);
    bool DeleteData(const NativeRdb::AbsRdbPredicates& absRdbPredicates);
    bool GetGrantInfo(std::shared_ptr<NativeRdb::AbsSharedResultSet> absSharedResultSet,
        std::vector<RdbGrantInfo> &rdbGrantInfoList);

private:
    std::shared_ptr<RdbDataManager> rdbDataManager_ = nullptr;
};

void PrintRdbGrantInfo(const RdbGrantInfo& info);
}
}
#endif