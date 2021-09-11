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

#include "verify_act_data_first_ability.h"
#include "app_log_wrapper.h"
#include "common_event_manager.h"
#include "unistd.h"
#include <future>
namespace OHOS {
namespace AppExecFwk {
#define RETURN_DEFAULT 1
void VerifyActDataFirstAbility::OnStart(const Want &want)
{
    Ability::OnStart(want);
}

int VerifyActDataFirstAbility::Insert(const Uri &uri, const ValuesBucket &value)
{
    return RETURN_DEFAULT;
}

int VerifyActDataFirstAbility::Delete(const Uri &uri, const DataAbilityPredicates &predicates)
{
    return RETURN_DEFAULT;
}

int VerifyActDataFirstAbility::Update(
    const Uri &uri, const ValuesBucket &value, const DataAbilityPredicates &predicates)
{
    return RETURN_DEFAULT;
}

std::shared_ptr<ResultSet> VerifyActDataFirstAbility::Query(
    const Uri &uri, const std::vector<std::string> &columns, const DataAbilityPredicates &predicates)
{
    return nullptr;
}

std::vector<std::string> VerifyActDataFirstAbility::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    std::vector<std::string> value;
    return value;
}

int VerifyActDataFirstAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    return RETURN_DEFAULT;
}
REGISTER_AA(VerifyActDataFirstAbility)
}  // namespace AppExecFwk
}  // namespace OHOS
