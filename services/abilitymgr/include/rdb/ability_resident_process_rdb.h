/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_RDB_ABILITY_RESIDENT_PROCESS_RDB_H
#define OHOS_ABILITY_RUNTIME_RDB_ABILITY_RESIDENT_PROCESS_RDB_H

#include "rdb_data_manager.h"

namespace OHOS {
namespace AbilityRuntime {
enum RdbResult : int32_t {
    Rdb_OK = 0,
    /* Representative database initialization failed */
    Rdb_Init_Err,
    /* Failed to parse initialization file */
    Rdb_Parse_File_Err,
    /* Parameter check failed */
    Rdb_Parameter_Err,
    /* Failed to query permission settings for resident processes */
    Rdb_Permissions_Err,
    /* Database query failed, key may not exist */
    Rdb_Search_Record_Err
};

class ScopeGuard final {
public:
    using Function = std::function<void()>;
    explicit ScopeGuard(Function fn) : fn_(fn), dismissed(false) {}

    ~ScopeGuard()
    {
        if (!dismissed) {
            fn_();
        }
    }

    void Dismiss()
    {
        dismissed = true;
    }

private:
    Function fn_;
    bool dismissed;
};

class AmsResidentProcessRdbCallBack : public NativeRdb::RdbOpenCallback {
public:
    AmsResidentProcessRdbCallBack(const AmsRdbConfig &rdbConfig);
    int32_t OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int32_t OnUpgrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;
    int32_t OnDowngrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;
    int32_t OnOpen(NativeRdb::RdbStore &rdbStore) override;
    int32_t onCorruption(std::string databaseFile) override;

private:
    AmsRdbConfig rdbConfig_;
};

class AmsResidentProcessRdb final {
public:
    AmsResidentProcessRdb() {}
    ~AmsResidentProcessRdb() {}
    static AmsResidentProcessRdb &GetInstance();
    int32_t Init();
    int32_t VerifyConfigurationPermissions(const std::string &bundleName, const std::string &callerName);
    int32_t GetResidentProcessEnable(const std::string &bundleName, bool &enable);
    int32_t UpdateResidentProcessEnable(const std::string &bundleName, bool enable);
    int32_t RemoveData(std::string &bundleName);

private:
    std::unique_ptr<RdbDataManager> rdbMgr_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_RDB_ABILITY_RESIDENT_PROCESS_RDB_H