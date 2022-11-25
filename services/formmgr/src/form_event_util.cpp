/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "form_event_util.h"

#include "form_bms_helper.h"
#include "form_cache_mgr.h"
#include "form_data_mgr.h"
#include "form_db_cache.h"
#include "form_info_mgr.h"
#include "form_timer_mgr.h"
#include "form_util.h"
#include "form_provider_mgr.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {


void FormEventUtil::HandleBundleFormInfoChanged(const std::string &bundleName, int32_t userId)
{
    FormInfoMgr::GetInstance().Update(bundleName, userId);
}


void FormEventUtil::HandleProviderUpdated(const std::string &bundleName, const int userId)
{
    HILOG_INFO("%{public}s, bundleName:%{public}s, userId:%{public}d.", __func__, bundleName.c_str(), userId);
    std::vector<FormRecord> formInfos;
    if (!FormDataMgr::GetInstance().GetFormRecord(bundleName, formInfos)) {
        HILOG_INFO("%{public}s, no form info.", __func__);
        return;
    }

    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        // GetBundleMgr() has error log
        return;
    }

    std::vector<FormInfo> targetForms;
    if (!IN_PROCESS_CALL(iBundleMgr->GetFormsInfoByApp(bundleName, targetForms))) {
        HILOG_ERROR("%{public}s error, failed to get forms info.", __func__);
        return;
    }

    std::vector<int64_t> removedForms;
    std::vector<int64_t> updatedForms;
    for (FormRecord& formRecord : formInfos) {
        HILOG_INFO("%{public}s, provider update, formName:%{public}s", __func__, formRecord.formName.c_str());
        int64_t formId = formRecord.formId;
        if (ProviderFormUpdated(formId, formRecord, targetForms)) {
            updatedForms.emplace_back(formId);
            continue;
        }

        HILOG_INFO("%{public}s, no such form anymore, delete it:%{public}s", __func__, formRecord.formName.c_str());
        (!formRecord.formTempFlg) ? FormDbCache::GetInstance().DeleteFormInfo(formId) :
            FormDataMgr::GetInstance().DeleteTempForm(formId);
        removedForms.emplace_back(formId);
        FormDataMgr::GetInstance().DeleteFormRecord(formId);
    }

    if (!removedForms.empty()) {
        HILOG_INFO("%{public}s, clean removed forms", __func__);
        FormDataMgr::GetInstance().CleanHostRemovedForms(removedForms);
    }

    HILOG_INFO("%{public}s, remove form timer", __func__);
    for (const int64_t id : removedForms) {
        FormTimerMgr::GetInstance().RemoveFormTimer(id);
    }

    HILOG_INFO("%{public}s, refresh form", __func__);
    Want want;
    want.SetParam(Constants::PARAM_FORM_USER_ID, userId);
    for (const int64_t id : updatedForms) {
        FormProviderMgr::GetInstance().RefreshForm(id, want, true);
    }
}

void FormEventUtil::HandleBundleFormInfoRemoved(const std::string &bundleName, int32_t userId)
{
    FormInfoMgr::GetInstance().Remove(bundleName, userId);
}

void FormEventUtil::HandleProviderRemoved(const std::string &bundleName, const int32_t userId)
{
    HILOG_INFO("GET into HandleProviderRemoved with bundleName : %{public}s, userId : %{public}d",
        bundleName.c_str(), userId);
    // clean removed form in DB
    std::set<int64_t> removedForms;
    {
        std::vector<FormDBInfo> removedDBForm;
        FormDbCache::GetInstance().DeleteFormInfoByBundleName(bundleName, userId, removedDBForm);
        for (const auto &dbForm : removedDBForm) {
            removedForms.emplace(dbForm.formId);
            int32_t matchCount = FormDbCache::GetInstance().GetMatchCount(dbForm.bundleName, dbForm.moduleName);
            if (matchCount == 0) {
                FormBmsHelper::GetInstance().NotifyModuleRemovable(dbForm.bundleName, dbForm.moduleName);
            }
        }
    }
    // clean removed form in FormRecords
    FormDataMgr::GetInstance().CleanRemovedFormRecords(bundleName, removedForms);
    // clean removed temp form in FormRecords
    FormDataMgr::GetInstance().CleanRemovedTempFormRecords(bundleName, userId, removedForms);
    // clean removed forms in FormHostRecords
    std::vector<int64_t> vRemovedForms;
    vRemovedForms.assign(removedForms.begin(), removedForms.end());
    FormDataMgr::GetInstance().CleanHostRemovedForms(vRemovedForms);

    // clean removed form timers
    for (auto &formId : removedForms) {
        FormTimerMgr::GetInstance().RemoveFormTimer(formId);
    }
}

void FormEventUtil::HandleBundleDataCleared(const std::string &bundleName, const int uid)
{
    HILOG_DEBUG("%{public}s, bundleName:%{public}s, uid:%{public}d", __func__, bundleName.c_str(), uid);
    // as provider data is cleared
    std::set<int64_t> reCreateForms;
    FormDataMgr::GetInstance().GetReCreateFormRecordsByBundleName(bundleName, reCreateForms);
    if (!reCreateForms.empty()) {
        for (int64_t formId : reCreateForms) {
            ReCreateForm(formId);
        }
    }

    // as form host data is cleared
    HandleFormHostDataCleared(uid);
}

void FormEventUtil::HandleFormHostDataCleared(const int uid)
{
    HILOG_DEBUG("%{public}s, uid:%{public}d", __func__, uid);
    std::map<int64_t, bool> removedFormsMap;
    // clear formDBRecord
    ClearFormDBRecordData(uid, removedFormsMap);

    // clear temp form
    ClearTempFormRecordData(uid, removedFormsMap);

    // clear host data
    FormDataMgr::GetInstance().ClearHostDataByUId(uid);

    // delete forms timer
    for (const auto &removedForm : removedFormsMap) {
        if (removedForm.second) {
            FormTimerMgr::GetInstance().RemoveFormTimer(removedForm.first);
        }
    }
}

bool FormEventUtil::ProviderFormUpdated(const int64_t formId,
    const FormRecord &formRecord, const std::vector<FormInfo> &targetForms)
{
    HILOG_INFO("%{public}s start", __func__);
    if (targetForms.empty()) {
        HILOG_ERROR("%{public}s error, targetForms is empty", __func__);
        return false;
    }

    FormInfo updatedForm;
    bool bGetForm = FormDataMgr::GetInstance().GetUpdatedForm(formRecord, targetForms, updatedForm);
    if (bGetForm) {
        HILOG_INFO("%{public}s, form is still exist,form:%{public}s", __func__, formRecord.formName.c_str());
        // update resource
        FormDataMgr::GetInstance().SetNeedRefresh(formId, true);
        FormCacheMgr::GetInstance().DeleteData(formId);

        FormBmsHelper::GetInstance().NotifyModuleNotRemovable(formRecord.bundleName, formRecord.moduleName);
        FormTimerCfg timerCfg;
        GetTimerCfg(updatedForm.updateEnabled, updatedForm.updateDuration,
            updatedForm.scheduledUpdateTime, timerCfg);
        HandleTimerUpdate(formId, formRecord, timerCfg);
        FormDataMgr::GetInstance().SetVersionUpgrade(formId, true);
        return true;
    }
    HILOG_INFO("%{public}s, no updated form.", __func__);
    return false;
}

void FormEventUtil::ClearFormDBRecordData(const int uid, std::map<int64_t, bool> &removedFormsMap)
{
    std::map<int64_t, bool> foundFormsMap;
    std::map<FormIdKey, std::set<int64_t>> noHostFormDbMap;
    FormDbCache::GetInstance().GetNoHostDBForms(uid, noHostFormDbMap, foundFormsMap);
    if (foundFormsMap.size() > 0) {
        for (const auto &element : foundFormsMap) {
            FormDataMgr::GetInstance().DeleteFormUserUid(element.first, uid);
        }
    }

    HILOG_DEBUG("%{public}s, noHostFormDbMap size:%{public}zu", __func__, noHostFormDbMap.size());
    if (noHostFormDbMap.size() > 0) {
        BatchDeleteNoHostDBForms(uid, noHostFormDbMap, foundFormsMap);
    }

    if (!foundFormsMap.empty()) {
        removedFormsMap.insert(foundFormsMap.begin(), foundFormsMap.end());
    }
}

void FormEventUtil::ClearTempFormRecordData(const int uid, std::map<int64_t, bool> &removedFormsMap)
{
    std::map<int64_t, bool> foundFormsMap;
    std::map<FormIdKey, std::set<int64_t>> noHostTempFormsMap;
    FormDataMgr::GetInstance().GetNoHostTempForms(uid, noHostTempFormsMap, foundFormsMap);
    HILOG_DEBUG("%{public}s, noHostTempFormsMap size:%{public}zu", __func__, noHostTempFormsMap.size());
    if (noHostTempFormsMap.size() > 0) {
        BatchDeleteNoHostTempForms(uid, noHostTempFormsMap, foundFormsMap);
    }
    if (!foundFormsMap.empty()) {
        removedFormsMap.insert(foundFormsMap.begin(), foundFormsMap.end());
    }
}

void FormEventUtil::BatchDeleteNoHostTempForms(const int uid, std::map<FormIdKey,
    std::set<int64_t>> &noHostTempFormsMap, std::map<int64_t, bool> &foundFormsMap)
{
    for (const auto &element : noHostTempFormsMap) {
        std::set<int64_t> formIds = element.second;
        FormIdKey formIdKey = element.first;
        std::string bundleName = formIdKey.bundleName;
        std::string abilityName = formIdKey.abilityName;
        int result = FormProviderMgr::GetInstance().NotifyProviderFormsBatchDelete(bundleName, abilityName, formIds);
        if (result != ERR_OK) {
            HILOG_ERROR("%{public}s error, NotifyProviderFormsBatchDelete failed! bundleName:%{public}s,\
            abilityName:%{public}s",
                __func__, bundleName.c_str(), abilityName.c_str());
            for (int64_t formId : formIds) {
                FormDataMgr::GetInstance().AddFormUserUid(formId, uid);
            }
        } else {
            for (int64_t formId : formIds) {
                foundFormsMap.emplace(formId, true);
                FormDataMgr::GetInstance().DeleteFormRecord(formId);
                FormDataMgr::GetInstance().DeleteTempForm(formId);
            }
        }
    }
}

void FormEventUtil::GetTimerCfg(const bool updateEnabled,
    const int updateDuration, const std::string &configUpdateAt, FormTimerCfg& cfg)
{
    HILOG_INFO("%{public}s start", __func__);
    if (!updateEnabled) {
        HILOG_INFO("%{public}s, update disable", __func__);
        return;
    }

    if (updateDuration > 0) {
        // interval timer
        HILOG_INFO("%{public}s,interval timer updateDuration:%{public}d", __func__, updateDuration);
        if (updateDuration <= Constants::MIN_CONFIG_DURATION) {
            cfg.updateDuration = Constants::MIN_PERIOD;
        } else if (updateDuration >= Constants::MAX_CONFIG_DURATION) {
            cfg.updateDuration = Constants::MAX_PERIOD;
        } else {
            cfg.updateDuration = updateDuration * Constants::TIME_CONVERSION;
        }
        cfg.enableUpdate = true;
        return;
    } else {
        // updateAtTimer
        if (configUpdateAt.empty()) {
            HILOG_INFO("%{public}s, configUpdateAt is empty", __func__);
            return;
        }
        HILOG_INFO("%{public}s,update at timer updateAt:%{public}s", __func__, configUpdateAt.c_str());
        std::vector<std::string> temp = FormUtil::StringSplit(configUpdateAt, Constants::TIME_DELIMETER);
        if (temp.size() != Constants::UPDATE_AT_CONFIG_COUNT) {
            HILOG_ERROR("%{public}s, invalid config", __func__);
            return;
        }
        int hour = std::stoi(temp[0]);
        int min = std::stoi(temp[1]);
        if (hour < Constants::MIN_TIME || hour > Constants::MAX_HOUR || min < Constants::MIN_TIME || min >
            Constants::MAX_MINUTE) {
            HILOG_ERROR("%{public}s, time is invalid", __func__);
            return;
        }

        cfg.updateAtHour = hour;
        cfg.updateAtMin = min;
        cfg.enableUpdate = true;
        return;
    }
}

void FormEventUtil::HandleTimerUpdate(const int64_t formId,
    const FormRecord &record, const FormTimerCfg &timerCfg)
{
    // both disable
    if (!record.isEnableUpdate && !timerCfg.enableUpdate) {
        return;
    }

    // enable to disable
    if (record.isEnableUpdate && !timerCfg.enableUpdate) {
        FormDataMgr::GetInstance().SetEnableUpdate(formId, false);
        FormTimerMgr::GetInstance().RemoveFormTimer(formId);
        return;
    }

    // disable to enable
    if (!record.isEnableUpdate && timerCfg.enableUpdate) {
        FormDataMgr::GetInstance().SetUpdateInfo(formId, true,
            timerCfg.updateDuration, timerCfg.updateAtHour, timerCfg.updateAtMin);
        if (timerCfg.updateDuration > 0) {
            HILOG_INFO("%{public}s, add interval timer:%{public}" PRId64 "", __func__, timerCfg.updateDuration);
            FormTimerMgr::GetInstance().AddFormTimer(formId, timerCfg.updateDuration, record.userId);
        } else {
            HILOG_INFO("%{public}s, add at timer:%{public}d, %{public}d", __func__,
                timerCfg.updateAtHour, timerCfg.updateAtMin);
            FormTimerMgr::GetInstance().AddFormTimer(formId, timerCfg.updateAtHour,
                                                     timerCfg.updateAtMin, record.userId);
        }
        return;
    }

    // both enable
    UpdateType type;
    if (record.updateDuration > 0) {
        if (timerCfg.updateDuration > 0) {
            // no change
            if (record.updateDuration == timerCfg.updateDuration) {
                return;
            }
            // interval change
            type = TYPE_INTERVAL_CHANGE;
        } else {
            // interval to updateat
            type = TYPE_INTERVAL_TO_ATTIME;
        }
    } else {
        if (timerCfg.updateDuration > 0) {
            // updateat to interval
            type = TYPE_ATTIME_TO_INTERVAL;
        } else {
            // no change;
            if (record.updateAtHour == timerCfg.updateAtHour && record.updateAtMin == timerCfg.updateAtMin) {
                return;
            }
            // updateat change
            type = TYPE_ATTIME_CHANGE;
        }
    }

    FormDataMgr::GetInstance().SetUpdateInfo(formId, true,
        timerCfg.updateDuration, timerCfg.updateAtHour, timerCfg.updateAtMin);
    FormTimerMgr::GetInstance().UpdateFormTimer(formId, type, timerCfg);
}

void FormEventUtil::ReCreateForm(const int64_t formId)
{
    HILOG_INFO("%{public}s start, formId:%{public}" PRId64 "", __func__, formId);
    FormRecord reCreateRecord;
    FormRecord record;
    bool isGetForm = FormDataMgr::GetInstance().GetFormRecord(formId, record);
    if (!isGetForm) {
        HILOG_ERROR("%{public}s error, not exist such form:%{public}" PRId64 "", __func__, formId);
        return;
    }
    FormCacheMgr::GetInstance().DeleteData(formId);
    reCreateRecord.bundleName = record.bundleName;
    reCreateRecord.abilityName = record.abilityName;
    reCreateRecord.formName = record.formName;
    reCreateRecord.specification = record.specification;
    reCreateRecord.formTempFlg = record.formTempFlg;
    reCreateRecord.isInited = record.isInited;
    reCreateRecord.versionUpgrade = record.versionUpgrade;

    Want want;
    FormUtil::CreateFormWant(reCreateRecord.formName, reCreateRecord.specification, reCreateRecord.formTempFlg, want);
    want.SetParam(Constants::RECREATE_FORM_KEY, true);
    FormProviderMgr::GetInstance().ConnectAmsForRefresh(formId, reCreateRecord, want, false);
}

void FormEventUtil::BatchDeleteNoHostDBForms(const int uid, std::map<FormIdKey,
    std::set<int64_t>> &noHostFormDbMap, std::map<int64_t, bool> &removedFormsMap)
{
    std::set<FormIdKey> removableModuleSet;
    for (const auto &element: noHostFormDbMap) {
        std::set<int64_t> formIds = element.second;
        FormIdKey formIdKey = element.first;
        std::string bundleName = formIdKey.bundleName;
        std::string abilityName = formIdKey.abilityName;
        int result = FormProviderMgr::GetInstance().NotifyProviderFormsBatchDelete(bundleName, abilityName, formIds);
        if (result != ERR_OK) {
            HILOG_ERROR("%{public}s error, NotifyProviderFormsBatchDelete failed! bundleName:%{public}s,\
            abilityName:%{public}s",
                __func__, bundleName.c_str(), abilityName.c_str());
            for (int64_t formId : formIds) {
                FormDBInfo dbInfo;
                int errCode = FormDbCache::GetInstance().GetDBRecord(formId, dbInfo);
                if (errCode == ERR_OK) {
                    dbInfo.formUserUids.emplace_back(uid);
                    FormDbCache::GetInstance().SaveFormInfo(dbInfo);
                }
            }
        } else {
            for (const int64_t formId : formIds) {
                removedFormsMap.emplace(formId, true);
                FormDBInfo dbInfo;
                int errCode = FormDbCache::GetInstance().GetDBRecord(formId, dbInfo);
                if (errCode == ERR_OK) {
                    FormIdKey removableModuleFormIdKey(dbInfo.bundleName, dbInfo.moduleName);
                    removableModuleSet.emplace(removableModuleFormIdKey);
                    FormDbCache::GetInstance().DeleteFormInfo(formId);
                }
                FormDataMgr::GetInstance().DeleteFormRecord(formId);
            }
        }
    }

    for (const FormIdKey &item : removableModuleSet) {
        int32_t matchCount = FormDbCache::GetInstance().GetMatchCount(item.bundleName, item.moduleName);
        if (matchCount == 0) {
            FormBmsHelper::GetInstance().NotifyModuleRemovable(item.bundleName, item.moduleName);
        }
    }
}

} // namespace AppExecFwk
} // namespace OHOS