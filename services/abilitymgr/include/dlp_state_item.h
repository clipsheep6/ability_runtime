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

#ifndef OHOS_AAFWK_DLP_STATE_ITME_H
#define OHOS_AAFWK_DLP_STATE_ITME_H

#include <list>

#include "ability_record.h"
#include "dlp_state_data.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class DlpStateItem
 * DlpStateItem,This class is used to record connection state of a process.
 */
class DlpStateItem : public std::enable_shared_from_this<DlpStateItem> {
public:
    DlpStateItem(int32_t dlpUid, int32_t dlpPid);
    virtual ~DlpStateItem();

    /**
     * add an opened dlp ability.
     *
     * @param record target dlp ability.
     * @param data output relationship data.
     * @return Returns true if need report relationship.
     */
    bool AddDlpConnectionState(const std::shared_ptr<AbilityRecord> &record, AbilityRuntime::DlpStateData &data);

    /**
     * remove an closed dlp ability.
     *
     * @param record target dlp ability.
     * @param data output relationship data.
     * @return Returns true if need report relationship.
     */
    bool RemoveDlpConnectionState(const std::shared_ptr<AbilityRecord> &record, AbilityRuntime::DlpStateData &data);

    int32_t GetDlpUid() const;
    int32_t GetOpenedAbilitySize() const;

private:
    DISALLOW_COPY_AND_MOVE(DlpStateItem);

    bool HandleDlpConnectionState(const std::shared_ptr<AbilityRecord> &record, bool isAdd,
        AbilityRuntime::DlpStateData &data);
    void GenerateDlpStateData(const std::shared_ptr<AbilityRecord> &dlpAbility, AbilityRuntime::DlpStateData &data);

    int32_t dlpUid_ = 0;
    int32_t dlpPid_ = 0;
    std::list<sptr<IRemoteObject>> dlpAbilities_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_DLP_STATE_ITME_H
