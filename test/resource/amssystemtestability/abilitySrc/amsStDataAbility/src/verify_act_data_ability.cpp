/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSERETURN_ERROR2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "abs_shared_result_set.h"
#include "ability_loader.h"
#include "app_log_wrapper.h"
#include "data_ability_helper.h"
#include "data_ability_predicates.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "values_bucket.h"
#include "verify_act_data_ability.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
static std::shared_ptr<NativeRdb::RdbStore> testStore;
static const std::string RDB_TEST_PATH = "/data/test/";
static const std::string DATABASE_FILE_NAME = "insert_test.db";
static const std::string DATABASE_NAME = RDB_TEST_PATH + "insert_test.db";
static int defaultReturn = 1;
static const double ageI = 24;
static const double salaryI = 2024.20;
static const int returnError = 0;
static const int returnError1 = -1;
static const int returnError2 = -2;
static const int returnError3 = -3;
static const int value1 = 0;
static const int value2 = 1;
static const int value3 = 2;
static const int value4 = 3;
}  // namespace

class InsertTestOpenCallback : public NativeRdb::RdbOpenCallback {
public:
    int OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
    static const std::string CREATE_TABLE_TEST;
};

const std::string InsertTestOpenCallback::CREATE_TABLE_TEST =
    std::string("CREATE TABLE IF NOT EXISTS test ") + std::string("(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                                  "name TEXT NOT NULL, age INTEGER, salary "
                                                                  "REAL, blobType BLOB)");

int InsertTestOpenCallback::OnCreate(NativeRdb::RdbStore &store)
{
    APP_LOGI("VerifyActDataAbility OnCreate");
    return store.ExecuteSql(CREATE_TABLE_TEST);
}

int InsertTestOpenCallback::OnUpgrade(NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    APP_LOGI("VerifyActDataAbility OnUpgrade");
    return returnError;
}

void VerifyActDataAbility::OnStart(const Want &want)
{
    APP_LOGI("VerifyActDataAbility OnStart");
    sharedList_.clear();
    Ability::OnStart(want);
}

int VerifyActDataAbility::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    APP_LOGI("VerifyActDataAbility <<<<Insert>>>>");
    std::shared_ptr<Uri> dataUri = std::make_shared<Uri>("dataability:///com.ix.VerifyActDataAbility");
    auto notify = DataAbilityHelper::Creator(GetContext(), dataUri);
    if (notify == nullptr) {
        APP_LOGE("VerifyActDataAbility <<<<Insert Creator nullptr>>>>");
        return returnError;
    }
    notify->NotifyChange(*dataUri);

    APP_LOGI("VerifyActDataAbility Insert uri.ToString   = %{public}s", uri.ToString().c_str());
    APP_LOGI("VerifyActDataAbility Insert dataUri.ToString   = %{public}s", dataUri->ToString().c_str());
    APP_LOGI("VerifyActDataAbility uri.Equals     = %{public}d", uri.Equals(*dataUri));
    if (uri.Equals(*dataUri) != 1) {
        return returnError1;
    }

    defaultReturn = 1;
    std::map<std::string, NativeRdb::ValueObject> valuesMap;
    value.GetAll(valuesMap);
    int valuesMapSize = valuesMap.size();
    APP_LOGI("VerifyActDataAbility value = %{public}d", valuesMapSize);
    if (!valuesMapSize)
        return returnError2;
    for (auto item = valuesMap.begin(); item != valuesMap.end(); item++) {
        auto valueFirst = item->first;
        auto valueSecond = item->second;
        int iEOK = 0;
        APP_LOGI("VerifyActDataAbility valueSecond.GetType[%{public}d]]", valueSecond.GetType());
        if (valueFirst.compare("name") == 0) {
            std::string val;
            iEOK = valueSecond.GetString(val);
            APP_LOGI("VerifyActDataAbility it iEOK[%{public}d]]", iEOK);
            APP_LOGI("VerifyActDataAbility it[%{public}s],[%{public}s]", valueFirst.c_str(), val.c_str());
            if (val.compare("ActsDataAbilityHelperTest")) {
                return returnError2;
                break;
            }
        } else if (valueFirst.compare("age") == 0) {
            double age;
            iEOK = valueSecond.GetDouble(age);
            APP_LOGI("VerifyActDataAbility it iEOK[%{public}d]", iEOK);
            APP_LOGI("VerifyActDataAbility it[%{public}s],[%{public}f]", valueFirst.c_str(), age);
            if (age != ageI) {
                return returnError2;
                break;
            }
        } else if (valueFirst.compare("salary") == 0) {
            double salary;
            iEOK = valueSecond.GetDouble(salary);
            APP_LOGI("VerifyActDataAbility it iEOK[%{public}d]]", iEOK);
            APP_LOGI("VerifyActDataAbility it[%{public}s],[%{public}f]", valueFirst.c_str(), salary);
            if (salary != salaryI) {
                return returnError2;
                break;
            }
        } else {
            APP_LOGI("VerifyActDataAbility it default[%{public}s]", valueFirst.c_str());
            break;
        }
    }

    int errCode = 0;
    std::string dbDir = GetDatabaseDir();
    NativeRdb::RdbStoreConfig config(dbDir + "/" + DATABASE_FILE_NAME);
    InsertTestOpenCallback helper;
    testStore = NativeRdb::RdbHelper::GetRdbStore(config, 1, helper, errCode);
    APP_LOGI("VerifyActDataAbility Insert Patch %{public}s", (dbDir + "/" + DATABASE_FILE_NAME).c_str());

    if (testStore == nullptr) {
        APP_LOGI("VerifyActDataAbility <<<<Insert>>>> testStore is nullptr, data will be empty");
        return returnError3;
    }

    int64_t id;
    int insertError;
    if ((insertError = testStore->Insert(id, "test", value)) != 0) {
        APP_LOGE("VerifyActDataAbility <<<<Insert>>>> store->Insert Error %{public}d", insertError);
        return insertError;
    }

    return defaultReturn;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> VerifyActDataAbility::Query(
    const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("VerifyActDataAbility <<<<Query>>>>");
    std::shared_ptr<Uri> dataUri = std::make_shared<Uri>("dataability:///com.ix.VerifyActDataAbility");

    APP_LOGI("VerifyActDataAbility Query uri.ToString   = %{public}s", uri.ToString().c_str());
    APP_LOGI("VerifyActDataAbility Query dataUri.ToString   = %{public}s", dataUri->ToString().c_str());
    APP_LOGI("VerifyActDataAbility uri.Equals     = %{public}d", uri.Equals(*dataUri));
    if (uri.Equals(*dataUri) != 1) {
        return nullptr;
    }
    int indexI = 0;
    for (const auto &item : columns) {
        switch (indexI++) {
            case value1:
                if (item.compare("value1") != 0)
                    return nullptr;
                break;
            case value2:
                if (item.compare("value2") != 0)
                    return nullptr;
                break;
            case value3:
                if (item.compare("value3") != 0)
                    return nullptr;
                break;
            case value4:
                if (item.compare("value4") != 0)
                    return nullptr;
                break;
            default:
                APP_LOGI("VerifyActDataAbility indexI = [%{public}d]]", indexI);
        }
    }

    APP_LOGI("VerifyActDataAbility Query predicates");
    APP_LOGI("VerifyActDataAbility Query predicates.Is = %{public}d", predicates.IsRawSelection());
    APP_LOGI("VerifyActDataAbility Query predicates.Get = %{public}d", predicates.GetJudgeSource());

    int errCode = 0;
    std::string dbDir = GetDatabaseDir();
    NativeRdb::RdbStoreConfig config(dbDir + "/" + DATABASE_FILE_NAME);
    InsertTestOpenCallback helper;
    testStore = NativeRdb::RdbHelper::GetRdbStore(config, 1, helper, errCode);
    APP_LOGI("VerifyActDataAbility Query Patch %{public}s", (dbDir + "/" + DATABASE_FILE_NAME).c_str());

    if (testStore == nullptr) {
        APP_LOGI("VerifyActDataAbility <<<<Query>>>> testStore is nullptr, data will be empty");
        return nullptr;
    }

    std::unique_ptr<NativeRdb::AbsSharedResultSet> rresultSet =
        testStore->QuerySql("SELECT * FROM test WHERE name = ?" ,  
            std::vector<std::string> {"ActsDataAbilityHelperTest"});
    if (rresultSet == nullptr) {
        APP_LOGE("VerifyActDataAbility <<<<Query>>>> rresultSet is nullptr");
        return nullptr;
    }

    APP_LOGE("VerifyActDataAbility <<<<Query>>>> ret(rresultSet.release()");
    std::shared_ptr<NativeRdb::AbsSharedResultSet> ret(rresultSet.release());
    sharedList_.push_back(ret);
    return ret;
}

int VerifyActDataAbility::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("VerifyActDataAbility <<<<Update>>>>");
    std::shared_ptr<Uri> dataUri = std::make_shared<Uri>("dataability:///com.ix.VerifyActDataAbility");
    auto notify = DataAbilityHelper::Creator(GetContext(), dataUri);
    if (notify == nullptr) {
        APP_LOGE("VerifyActDataAbility <<<<Update Creator nullptr>>>>");
        return returnError;
    }
    notify->NotifyChange(*dataUri);

    APP_LOGI("VerifyActDataAbility Update uri.ToString   = %{public}s", uri.ToString().c_str());
    APP_LOGI("VerifyActDataAbility Update dataUri.ToString   = %{public}s", dataUri->ToString().c_str());
    APP_LOGI("VerifyActDataAbility uri.Equals     = %{public}d", uri.Equals(*dataUri));
    if (uri.Equals(*dataUri) != 1) {
        return returnError1;
    }

    defaultReturn = 1;
    std::map<std::string, NativeRdb::ValueObject> valuesMap;
    value.GetAll(valuesMap);
    int valuesMapSize = valuesMap.size();
    APP_LOGI("VerifyActDataAbility value = %{public}d", valuesMapSize);
    if (!valuesMapSize)
        return returnError2;
    for (auto item = valuesMap.begin(); item != valuesMap.end(); item++) {
        auto valueFirst = item->first;
        auto valueSecond = item->second;
        APP_LOGI("VerifyActDataAbility valueSecond.GetType[%{public}d]]", valueSecond.GetType());
        if (valueFirst.compare("name") == 0) {
            std::string val;
            valueSecond.GetString(val);
            APP_LOGI("VerifyActDataAbility it[%{public}s],[%{public}s]", valueFirst.c_str(), val.c_str());
            if (val.compare("ActsDataAbilityHelperTest")) {
                return returnError2;
                break;
            }
        } else if (valueFirst.compare("age") == 0) {
            double age;
            valueSecond.GetDouble(age);
            APP_LOGI("VerifyActDataAbility it[%{public}s],[%{public}f]", valueFirst.c_str(), age);
            if (age != ageI) {
                return returnError2;
                break;
            }
        } else if (valueFirst.compare("salary") == 0) {
            double salary;
            valueSecond.GetDouble(salary);
            APP_LOGI("VerifyActDataAbility it[%{public}s],[%{public}f]", valueFirst.c_str(), salary);
            if (salary != salaryI) {
                return returnError2;
                break;
            }
        } else {
            APP_LOGI("VerifyActDataAbility it default[%{public}s]", valueFirst.c_str());
            break;
        }
    }
    APP_LOGI("VerifyActDataAbility Update predicates = %{public}d", predicates.IsRawSelection());
    APP_LOGI("VerifyActDataAbility Update predicates.Get = %{public}d", predicates.GetJudgeSource());

    int errCode = 0;
    std::string dbDir = GetDatabaseDir();
    NativeRdb::RdbStoreConfig config(dbDir + "/" + DATABASE_FILE_NAME);
    InsertTestOpenCallback helper;
    testStore = NativeRdb::RdbHelper::GetRdbStore(config, 1, helper, errCode);
    APP_LOGI("VerifyActDataAbility Update Patch %{public}s", (dbDir + "/" + DATABASE_FILE_NAME).c_str());

    if (testStore == nullptr) {
        APP_LOGI("VerifyActDataAbility <<<<Update>>>> testStore is nullptr, data will be empty");
        return returnError3;
    }

    int changedRows;
    int updatError;
    if ((updatError = testStore->Update(changedRows, "test", value)) != 0) {
        APP_LOGE("VerifyActDataAbility <<<<Update>>>> store->Replace changedRows %{public}d Error %{public}d",
            changedRows,
            updatError);
        return updatError;
    }
    return defaultReturn;
}
int VerifyActDataAbility::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("VerifyActDataAbility <<<<Delete>>>>");
    std::shared_ptr<Uri> dataUri = std::make_shared<Uri>("dataability:///com.ix.VerifyActDataAbility");
    auto notify = DataAbilityHelper::Creator(GetContext(), dataUri);
    if (notify == nullptr) {
        APP_LOGE("VerifyActDataAbility <<<<Delete Creator nullptr>>>>");
        return returnError;
    }
    notify->NotifyChange(*dataUri);

    defaultReturn = 1;
    APP_LOGI("VerifyActDataAbility Delete uri.ToString   = %{public}s", uri.ToString().c_str());
    APP_LOGI("VerifyActDataAbility Delete dataUri.ToString   = %{public}s", dataUri->ToString().c_str());
    APP_LOGI("VerifyActDataAbility uri.Equals  is  = %{public}d", uri.Equals(*dataUri));
    if (uri.Equals(*dataUri) != 1) {
        return returnError;
    }
    APP_LOGI("VerifyActDataAbility Delete predicates.Is = %{public}d", predicates.IsRawSelection());
    APP_LOGI("VerifyActDataAbility Delete predicates.Get = %{public}d", predicates.GetJudgeSource());

    int errCode = 0;
    std::string dbDir = GetDatabaseDir();
    NativeRdb::RdbStoreConfig config(dbDir + "/" + DATABASE_FILE_NAME);
    InsertTestOpenCallback helper;
    testStore = NativeRdb::RdbHelper::GetRdbStore(config, 1, helper, errCode);
    APP_LOGI("VerifyActDataAbility Delete Patch %{public}s", (dbDir + "/" + DATABASE_FILE_NAME).c_str());

    if (testStore == nullptr) {
        APP_LOGI("VerifyActDataAbility <<<<Delete>>>> testStore is nullptr, data will be empty");
        return returnError3;
    }

    int changedRows;
    int deleteError;
    if ((deleteError = testStore->Delete(changedRows, "test")) != 0) {
        APP_LOGE("VerifyActDataAbility <<<<Delete>>>> store->Delete changedRows  %{public}d Error %{public}d",
            changedRows,
            deleteError);
        return deleteError;
    }
    return defaultReturn;
}
std::string VerifyActDataAbility::GetType(const Uri &uri)
{
    APP_LOGI("VerifyActDataAbility <<<<GetType>>>>");
    APP_LOGI("VerifyActDataAbility GetType uri = %{public}s", uri.ToString().c_str());
    std::shared_ptr<Uri> dataUri = std::make_shared<Uri>("dataability:///com.ix.VerifyActDataAbility3");
    APP_LOGI("VerifyActDataAbility GetType dataUri.ToString   = %{public}s", dataUri->ToString().c_str());
    APP_LOGI("VerifyActDataAbility uri.Equals     = %{public}d", uri.Equals(*dataUri));
    if (uri.Equals(*dataUri) == 1) {
        return nullptr;
    }
    std::string retval(uri.ToString());
    return retval;
}
int VerifyActDataAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    APP_LOGI("VerifyActDataAbility <<<<OpenFile>>>>");
    std::shared_ptr<Uri> dataUri = std::make_shared<Uri>("dataability:///com.ix.VerifyActDataAbility");
    defaultReturn = 1;
    APP_LOGI("VerifyActDataAbility Delete uri.ToString   = %{public}s", uri.ToString().c_str());
    APP_LOGI("VerifyActDataAbility Delete dataUri.ToString   = %{public}s", dataUri->ToString().c_str());
    APP_LOGI("VerifyActDataAbility uri.Equals     = %{public}d", uri.Equals(*dataUri));
    if (uri.Equals(*dataUri) != 1) {
        return returnError1;
    }
    APP_LOGI("VerifyActDataAbility OpenFile mode = %{public}s", mode.c_str());
    if (!(mode.compare("r") == 0 || mode.compare("w") == 0 || mode.compare("wt") == 0 || mode.compare("wa") == 0 ||
            mode.compare("rw") == 0 || mode.compare("rwt") == 0)) {
        if (mode.compare("A1@k#4%$,.<>)(oioiu*((*&(&*giIGT^%&^Ug;sdfk;losd*7873iug8%&^$&%]ERFUy&^%&&R7") == 0) {
            defaultReturn = returnError2;
        } else
            defaultReturn = returnError3;
    }
    APP_LOGI("VerifyActDataAbility OpenFile defaultReturn = %{public}d", defaultReturn);
    return defaultReturn;
}
int VerifyActDataAbility::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    APP_LOGI("VerifyActDataAbility <<<<BatchInsert>>>>");
    std::shared_ptr<Uri> dataUri = std::make_shared<Uri>("dataability:///com.ix.VerifyActDataAbility");
    defaultReturn = 1;
    APP_LOGI("VerifyActDataAbility BatchInsert uri.ToString   = %{public}s", uri.ToString().c_str());
    APP_LOGI("VerifyActDataAbility BatchInsert dataUri.ToString   = %{public}s", dataUri->ToString().c_str());
    APP_LOGI("VerifyActDataAbility uri.Equals     = %{public}d", uri.Equals(*dataUri));
    if (uri.Equals(*dataUri) != 1) {
        return returnError1;
    }

    defaultReturn = 1;
    std::map<std::string, NativeRdb::ValueObject> valuesMap;
    for (auto it = values.begin(); it != values.end(); it++) {
        it->GetAll(valuesMap);
        int valuesMapSize = values.size();
        APP_LOGI("VerifyActDataAbility value = %{public}d", valuesMapSize);
        if (!valuesMapSize)
            return returnError2;
        for (auto item = valuesMap.begin(); item != valuesMap.end(); item++) {
            auto valueFirst = item->first;
            auto valueSecond = item->second;
            APP_LOGI("VerifyActDataAbility valueSecond.GetType[%{public}d]]", valueSecond.GetType());
            if (valueFirst.compare("name") == 0) {
                std::string val;
                valueSecond.GetString(val);
                APP_LOGI("VerifyActDataAbility it[%{public}s],[%{public}s]", valueFirst.c_str(), val.c_str());
                if (val.compare("ActsDataAbilityHelperTest")) {
                    return returnError2;
                    break;
                }
            } else if (valueFirst.compare("age") == 0) {
                double age;
                valueSecond.GetDouble(age);
                APP_LOGI("VerifyActDataAbility it[%{public}s],[%{public}f]", valueFirst.c_str(), age);
                if (age != ageI) {
                    return returnError2;
                    break;
                }
            } else if (valueFirst.compare("salary") == 0) {
                double salary;
                valueSecond.GetDouble(salary);
                APP_LOGI("VerifyActDataAbility it[%{public}s],[%{public}f]", valueFirst.c_str(), salary);
                if (salary != salaryI) {
                    return returnError2;
                    break;
                }
            } else {
                APP_LOGI("VerifyActDataAbility it default[%{public}s]", valueFirst.c_str());
                break;
            }
        }
    }

    int retAbilityBatchInsert = Ability::BatchInsert(uri, values);
    APP_LOGI("VerifyActDataAbility BatchInsert retAbility = %{public}d", retAbilityBatchInsert);
    return retAbilityBatchInsert;
}
std::vector<std::string> VerifyActDataAbility::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    APP_LOGI("VerifyActDataAbility <<<<GetFileTypes>>>>");
    std::shared_ptr<Uri> dataUri = std::make_shared<Uri>("dataability:///com.ix.VerifyActDataAbility");
    defaultReturn = 1;
    APP_LOGI("VerifyActDataAbility Delete uri.ToString   = %{public}s", uri.ToString().c_str());
    APP_LOGI("VerifyActDataAbility Delete dataUri.ToString   = %{public}s", dataUri->ToString().c_str());
    APP_LOGI("VerifyActDataAbility uri.Equals     = %{public}d", uri.Equals(*dataUri));
    std::vector<std::string> vectorFileTypes;
    if (uri.Equals(*dataUri) != 1) {
        APP_LOGE("VerifyActDataAbility uri.Equals(*dataUri) != 1");
        vectorFileTypes.push_back("Uri error");
        return vectorFileTypes;
    }

    if (mimeTypeFilter.compare("*/*") == 0) {
        vectorFileTypes.push_back("hap");
        vectorFileTypes.push_back("jpg");
        vectorFileTypes.push_back("image/png");
    } else if (mimeTypeFilter.compare("image/*") == 0) {
        vectorFileTypes.push_back("image/png");
    } else if (mimeTypeFilter.compare("*/jpg") == 0) {
        vectorFileTypes.push_back("jpg");
    } else if (mimeTypeFilter.compare("returnUndefined") == 0) {
        return {};
    }

    return vectorFileTypes;
}

Uri VerifyActDataAbility::NormalizeUri(const Uri &uri)
{
    APP_LOGI("VerifyActDataAbility <<<<NormalizeUri>>>>");
    APP_LOGI("VerifyActDataAbility NormalizeUri uri = %{public}s", uri.ToString().c_str());
    std::shared_ptr<Uri> dataUri = std::make_shared<Uri>("dataability:///com.ix.VerifyActDataAbility");
    std::shared_ptr<Uri> dataUri3 = std::make_shared<Uri>("dataability:///com.ix.VerifyActDataAbility3");
    Uri dataUriNull("");
    Uri dataUriError("errorUri");
    APP_LOGI("VerifyActDataAbility NormalizeUri dataUri.ToString   = %{public}s", dataUri->ToString().c_str());
    APP_LOGI("VerifyActDataAbility uri.Equals     = %{public}d", uri.Equals(*dataUri));
    if (uri.Equals(*dataUri) != 1) {
        if (uri.Equals(*dataUri3) == 1) {
            return dataUriNull;
        }
        return dataUriError;
    }
    return uri;
}

Uri VerifyActDataAbility::DenormalizeUri(const Uri &uri)
{
    APP_LOGI("VerifyActDataAbility <<<<DenormalizeUri>>>>");
    APP_LOGI("VerifyActDataAbility DenormalizeUri uri = %{public}s", uri.ToString().c_str());
    std::shared_ptr<Uri> dataUri = std::make_shared<Uri>("dataability:///com.ix.VerifyActDataAbility");
    std::shared_ptr<Uri> dataUri3 = std::make_shared<Uri>("dataability:///com.ix.VerifyActDataAbility3");
    Uri dataUriNull("");
    Uri dataUriError("errorUri");
    APP_LOGI("VerifyActDataAbility DenormalizeUri dataUri.ToString   = %{public}s", dataUri->ToString().c_str());
    APP_LOGI("VerifyActDataAbility uri.Equals     = %{public}d", uri.Equals(*dataUri));

    if (uri.Equals(*dataUri) != 1) {
        if (uri.Equals(*dataUri3) == 1) {
            return dataUriNull;
        }
        return dataUriError;
    }
    return uri;
}
REGISTER_AA(VerifyActDataAbility);
}  // namespace AppExecFwk
}  // namespace OHOS
