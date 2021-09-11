/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef _MAIN_DATA_ABILITY_H_
#define _MAIN_DATA_ABILITY_H_
#include "ability_loader.h"
#include "app_log_wrapper.h"
#include "ability.h"
#include "matching_skills.h"
#include "common_event_subscribe_info.h"
#include "common_event_subscriber.h"
#include "common_event.h"
#include "common_event_data.h"
#include "iremote_object.h"
#include "common_event_publish_info.h"

namespace OHOS {
namespace AppExecFwk {
class VerifyActDataFirstAbility : public Ability {
public:
    virtual void OnStart(const Want &want) override;
    virtual int Insert(const Uri &uri, const ValuesBucket &value) override;
    virtual int Delete(const Uri &uri, const DataAbilityPredicates &predicates) override;
    virtual int Update(const Uri &uri, const ValuesBucket &value, const DataAbilityPredicates &predicates) override;
    virtual std::shared_ptr<ResultSet> Query(
        const Uri &uri, const std::vector<std::string> &columns, const DataAbilityPredicates &predicates) override;
    virtual std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter) override;
    virtual int OpenFile(const Uri &uri, const std::string &mode) override;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  //_MAIN_ABILITY_H_