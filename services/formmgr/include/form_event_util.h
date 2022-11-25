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
#ifndef OHOS_FORM_FWK_FORM_EVENT_UTIL_H
#define OHOS_FORM_FWK_FORM_EVENT_UTIL_H

#include <set>
#include <string>

#include "form_record.h"
#include "form_info.h"
#include "form_id_key.h"
#include "form_timer.h"

namespace OHOS {
namespace AppExecFwk {
const std::string KEY_UID = "uid";
const std::string KEY_USER_ID = "userId";
class FormEventUtil {
public:

    FormEventUtil() = default;

    void HandleBundleFormInfoChanged(const std::string &bundleName, int32_t userId);
    void HandleProviderUpdated(const std::string &bundleName, const int userId);
    void HandleBundleFormInfoRemoved(const std::string &bundleName, int32_t userId);
    void HandleProviderRemoved(const std::string &bundleName, const int32_t userId);
    void HandleBundleDataCleared(const std::string &bundleName, const int uid);
    void HandleFormHostDataCleared(const int uid);
    bool ProviderFormUpdated(const int64_t formId, const FormRecord &formRecord,
        const std::vector<FormInfo> &targetForms);
    void ClearFormDBRecordData(const int uid, std::map<int64_t, bool> &removedFormsMap);
    void ClearTempFormRecordData(const int uid, std::map<int64_t, bool> &removedFormsMap);
    void BatchDeleteNoHostTempForms(const int uid, std::map<FormIdKey, std::set<int64_t>> &noHostTempFormsMap,
        std::map<int64_t, bool> &foundFormsMap);
    void GetTimerCfg(const bool updateEnabled, const int updateDuration, const std::string &configUpdateAt,
        FormTimerCfg &cfg);
    void HandleTimerUpdate(const int64_t formId, const FormRecord &record, const FormTimerCfg &timerCfg);
    void ReCreateForm(const int64_t formId);
    void BatchDeleteNoHostDBForms(const int uid, std::map<FormIdKey, std::set<int64_t>> &noHostFormDbMap,
        std::map<int64_t, bool> &removedFormsMap);
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_FORM_FWK_FORM_EVENT_UTIL_H