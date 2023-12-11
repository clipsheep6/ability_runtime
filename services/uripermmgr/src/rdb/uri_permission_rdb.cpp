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

#include "uri_permission_rdb.h"

#include <string>
#include <vector>

#include "scope_guard.h"
#include "ability_manager_errors.h"
#include "hilog_wrapper.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {

namespace {
const std::string URI_PERMISSION_RDB_NAME = "/uripmdb.db";
const std::string URI_PERMISSION_TABLE_NAME = "uri_permission";
const std::string COLUMN_URI = "URI";
const std::string COLUMN_FLAG = "FLAG";
const std::string COLUMN_FROM_TOKEN_ID = "FROM_TOKEN_ID";
const std::string COLUMN_TARGET_TOKEN_ID = "TARGET_TOKEN_ID";
const int32_t COLUMN_URI_INDEX = 1;
const int32_t COLUMN_FLAG_INDEX = 2;
const int32_t COLUMN_FROM_TOKEN_ID_INDEX = 3;
const int32_t COLUMN_TARGET_TOKEN_ID_INDEX = 4;
}

void PrintRdbGrantInfo(const RdbGrantInfo &info)
{
    HILOG_DEBUG("uri: %{private}s, flag: %{public}u, fromTokenId: %{public}u, targetTokenId: %{public}u.",
        info.uri.c_str(), info.flag, info.fromTokenId, info.targetTokenId);
}

UriPermissionRdb::UriPermissionRdb()
{
    HILOG_DEBUG("UriPermissionRdb: Create DataBase");
    RdbConfig rdbConfig;
    rdbConfig.dbName = URI_PERMISSION_RDB_NAME;
    rdbConfig.tableName = URI_PERMISSION_TABLE_NAME;
    // create database
    rdbConfig.createTableSql = std::string("CREATE TABLE IF NOT EXISTS " + URI_PERMISSION_TABLE_NAME +
        "(ID INTEGER PRIMARY KEY AUTOINCREMENT, URI TEXT NOT NULL, " +
        "FLAG INTEGER, FROM_TOKEN_ID INTEGER, TARGET_TOKEN_ID INTEGER);");
    HILOG_DEBUG("CreateTableSql: %{public}s", rdbConfig.createTableSql.c_str());
    rdbDataManager_ = std::make_shared<RdbDataManager>(rdbConfig);
    bool ret = rdbDataManager_->CreateTable();
    if (!ret) {
        HILOG_ERROR("Failed to createTable");
    }
}

int32_t UriPermissionRdb::AddGrantInfo(const std::string &uri, uint32_t flag, uint32_t fromTokenId,
    uint32_t targetTokenId)
{
    HILOG_INFO("AddGrantInfo uri=%{private}s, flag=%{public}u, fromTokenId=%{public}u, targetTokenId=%{public}u.",
        uri.c_str(), flag, fromTokenId, targetTokenId);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    absRdbPredicates.EqualTo(COLUMN_FROM_TOKEN_ID, std::to_string(fromTokenId));
    absRdbPredicates.EqualTo(COLUMN_TARGET_TOKEN_ID, std::to_string(targetTokenId));
    absRdbPredicates.EqualTo(COLUMN_URI, uri);
    std::vector<RdbGrantInfo> rdbGrantInfoList;
    int rowCount;
    bool ret = QueryData(absRdbPredicates, rdbGrantInfoList, rowCount);
    if (!ret) {
        HILOG_ERROR("QueryData failed");
        return INNER_ERR;
    }
    HILOG_DEBUG("rowCount = %{public}d", rowCount);
    // should query no more than one uri permission
    if (rowCount > 1) {
        HILOG_ERROR("Query more than one uri permission grant info!");
        for (const auto &info : rdbGrantInfoList) {
            PrintRdbGrantInfo(info);
        }
        return INNER_ERR;
    }
    if (rowCount == 0) {
        HILOG_INFO("Add a new uri permission info");
        RdbGrantInfo grantInfo = { uri, flag, static_cast<uint32_t>(fromTokenId),
                                   static_cast<uint32_t>(targetTokenId) };
        rdbGrantInfoList.push_back(grantInfo);
        ret = InsertData(rdbGrantInfoList);
        if (!ret) {
            HILOG_ERROR("InsertData failed");
            return INNER_ERR;
        }
        return ERR_OK;
    }
    // update flag
    if ((rdbGrantInfoList[0].flag & flag) == Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION) {
        HILOG_INFO("Update an uri permission info");
        NativeRdb::ValuesBucket valuesBucket;
        valuesBucket.PutInt(COLUMN_FLAG, flag);
        ret = UpdateData(absRdbPredicates, valuesBucket);
        if (!ret) {
            HILOG_ERROR("UpdateData failed");
            return INNER_ERR;
        }
        return ERR_OK;
    }
    HILOG_INFO("Uri has been granted");
    return ERR_OK;
}

int32_t UriPermissionRdb::RemoveGrantInfo(uint32_t tokenId, sptr<StorageManager::IStorageManager> storageManager)
{
    HILOG_INFO("RemoveGrantInfo, TokenId = %{public}u", tokenId);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    absRdbPredicates.EqualTo(COLUMN_FROM_TOKEN_ID, std::to_string(tokenId));
    absRdbPredicates.Or();
    absRdbPredicates.EqualTo(COLUMN_TARGET_TOKEN_ID, std::to_string(tokenId));
    int ret = RemoveGrantInfo(absRdbPredicates, storageManager);
    return ret;
}

int32_t UriPermissionRdb::RemoveGrantInfo(const std::string &uri, uint32_t tokenId,
    sptr<StorageManager::IStorageManager> storageManager)
{
    HILOG_INFO("RemoveGrantInfo, uri = %{private}s, TokenId = %{public}u", uri.c_str(), tokenId);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    absRdbPredicates.EqualTo(COLUMN_URI, uri);
    absRdbPredicates.EqualTo(COLUMN_TARGET_TOKEN_ID, std::to_string(tokenId));
    int ret = RemoveGrantInfo(absRdbPredicates, storageManager);
    return ret;
}

int32_t UriPermissionRdb::RemoveGrantInfo(const NativeRdb::AbsRdbPredicates &absRdbPredicates,
    sptr<StorageManager::IStorageManager> storageManager)
{
    if (storageManager == nullptr) {
        HILOG_ERROR("storageManager is nullptr!");
        return INNER_ERR;
    }
    std::map<unsigned int, std::vector<std::string>> uriLists;
    int rowCount;
    std::vector<RdbGrantInfo> rdbGrantInfoList;
    bool ret = QueryData(absRdbPredicates, rdbGrantInfoList, rowCount);
    if (!ret) {
        return INNER_ERR;
    }
    for (const auto &info : rdbGrantInfoList) {
        uriLists[info.targetTokenId].emplace_back(info.uri);
    }
    // 1. delete share file
    for (auto iter = uriLists.begin(); iter != uriLists.end(); iter++) {
        storageManager->DeleteShareFile(iter->first, iter->second);
    }
    // 2. delete rdb data
    HILOG_DEBUG("total %{public}u uri permissions info to be removed", rowCount);
    ret = DeleteData(absRdbPredicates);
    if (!ret) {
        HILOG_ERROR("RemoveGrantInfo failed");
        return INNER_ERR;
    }
    return ERR_OK;
}

bool UriPermissionRdb::CheckPersistableUriPermissionProxy(const std::string& uri, uint32_t flag, uint32_t tokenId)
{
    // check if the uri has flag permission
    HILOG_DEBUG(
        "CheckPersistablekUriPermissionProxy: uri = %{private}s, flag = %{public}i, tokenId = %{public}i",
        uri.c_str(), flag, tokenId);
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    absRdbPredicates.EqualTo(COLUMN_URI, uri);
    absRdbPredicates.EqualTo(COLUMN_TARGET_TOKEN_ID, std::to_string(tokenId));
    int rowCount;
    std::vector<RdbGrantInfo> rdbGrantInfoList;
    bool ret = QueryData(absRdbPredicates, rdbGrantInfoList, rowCount);
    flag &= (~Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION);
    if (ret && rowCount > 0) {
        for (const auto &info : rdbGrantInfoList) {
            if (((info.flag | Want::FLAG_AUTH_READ_URI_PERMISSION) & flag) != 0) {
                HILOG_DEBUG("CheckUriPermissionProxy ok.");
                return true;
            }
        }
    }
    HILOG_DEBUG("CheckUriPermissionProxy failed.");
    return false;
}

void UriPermissionRdb::ShowAllGrantInfo()
{
    NativeRdb::AbsRdbPredicates absRdbPredicates(URI_PERMISSION_TABLE_NAME);
    std::vector<RdbGrantInfo> rdbGrantInfoList;
    int rowCount;
    bool ret = QueryData(absRdbPredicates, rdbGrantInfoList, rowCount);
    if (!ret) {
        HILOG_WARN("failed to query");
    }
}

bool UriPermissionRdb::GetGrantInfo(std::shared_ptr<NativeRdb::AbsSharedResultSet> absSharedResultSet,
    std::vector<RdbGrantInfo> &rdbGrantInfoList)
{
    if (absSharedResultSet == nullptr) {
        return false;
    }
    std::string uri;
    bool ret = absSharedResultSet->GetString(COLUMN_URI_INDEX, uri);
    if (ret != NativeRdb::E_OK) {
        HILOG_ERROR("Get COLUMN_URI_INDEX  failed");
        return false;
    }
    int flag;
    ret = absSharedResultSet->GetInt(COLUMN_FLAG_INDEX, flag);
    if (ret != NativeRdb::E_OK) {
        HILOG_ERROR("Get COLUMN_TARGET_TOKEN_ID_INDEX failed");
        return false;
    }
    int targetTokenId;
    ret = absSharedResultSet->GetInt(COLUMN_TARGET_TOKEN_ID_INDEX, targetTokenId);
    if (ret != NativeRdb::E_OK) {
        HILOG_ERROR("Get COLUMN_FLAG_INDEX failed");
        return false;
    }
    int fromTokenId;
    ret = absSharedResultSet->GetInt(COLUMN_FROM_TOKEN_ID_INDEX, fromTokenId);
    if (ret != NativeRdb::E_OK) {
        HILOG_ERROR("Get COLUMN_FROM_TOKEN_ID_INDEX failed");
        return false;
    }
    RdbGrantInfo grantInfo = { uri, flag, static_cast<uint32_t>(fromTokenId), static_cast<uint32_t>(targetTokenId) };
    rdbGrantInfoList.push_back(grantInfo);
    return true;
}

bool UriPermissionRdb::QueryData(const NativeRdb::AbsRdbPredicates &absRdbPredicates,
    std::vector<RdbGrantInfo> &rdbGrantInfoList, int &rowCount)
{
    auto absSharedResultSet = rdbDataManager_->QueryData(absRdbPredicates);
    if (absSharedResultSet == nullptr) {
        HILOG_ERROR("UriPermissionRdb::QueryData failed");
        return false;
    }
    ScopeGuard stateGuard([&] { absSharedResultSet->Close(); });
    int ret = absSharedResultSet->GetRowCount(rowCount);
    if (ret != NativeRdb::E_OK) {
        HILOG_ERROR("GetRowCount failed");
        return false;
    }
    if (rowCount == 0) {
        HILOG_DEBUG("Query Result, total %{public}i uri", rowCount);
        return true;
    }
    ret = absSharedResultSet->GoToFirstRow();
    if (ret != NativeRdb::E_OK) {
        HILOG_ERROR("GoToFirstRow failed");
        return false;
    }
    do {
        // ger grant info from query result
        bool result = GetGrantInfo(absSharedResultSet, rdbGrantInfoList);
        if (!result) {
            return false;
        }
    } while (absSharedResultSet->GoToNextRow() == NativeRdb::E_OK);
    HILOG_DEBUG("Query Result, total %{public}i uri", rowCount);
    for (const auto &info : rdbGrantInfoList) {
        PrintRdbGrantInfo(info);
    }
    return true;
}

bool UriPermissionRdb::InsertData(const std::vector<RdbGrantInfo> &rdbGrantInfoList)
{
    std::vector<NativeRdb::ValuesBucket> valuesBuckets;
    int64_t grantInfoNum = static_cast<int64_t>(rdbGrantInfoList.size());
    for (int i = 0; i < grantInfoNum; i++) {
        NativeRdb::ValuesBucket valuesBucket;
        valuesBucket.PutString(COLUMN_URI, rdbGrantInfoList[i].uri);
        valuesBucket.PutInt(COLUMN_FLAG, rdbGrantInfoList[i].flag);
        valuesBucket.PutInt(COLUMN_FROM_TOKEN_ID, rdbGrantInfoList[i].fromTokenId);
        valuesBucket.PutInt(COLUMN_TARGET_TOKEN_ID, rdbGrantInfoList[i].targetTokenId);
        valuesBuckets.push_back(valuesBucket);
    }
    bool ret = rdbDataManager_->BatchInsert(grantInfoNum, valuesBuckets);
    return ret;
}

bool UriPermissionRdb::UpdateData(const NativeRdb::AbsRdbPredicates &absRdbPredicates,
    const NativeRdb::ValuesBucket &valuesBucket)
{
    bool ret = rdbDataManager_->UpdateData(valuesBucket, absRdbPredicates);
    return ret;
}

bool UriPermissionRdb::DeleteData(const NativeRdb::AbsRdbPredicates &absRdbPredicates)
{
    bool ret = rdbDataManager_->DeleteData(absRdbPredicates);
    return ret;
}
}
}
