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

#include "ability_local_record.h"

#include "ability_impl.h"
#include "ability_thread.h"

namespace OHOS {
namespace AppExecFwk {
AbilityLocalRecord::AbilityLocalRecord(const std::shared_ptr<AbilityInfo> &info, const sptr<IRemoteObject> &token)
    : abilityInfo_(info), token_(token) {}

AbilityLocalRecord::~AbilityLocalRecord() {}

const std::shared_ptr<AbilityInfo> &AbilityLocalRecord::GetAbilityInfo()
{
    return abilityInfo_;
}

const std::shared_ptr<EventHandler> &AbilityLocalRecord::GetEventHandler()
{
    return handler_;
}

void AbilityLocalRecord::SetEventHandler(const std::shared_ptr<EventHandler> &handler)
{
    handler_ = handler;
}

const std::shared_ptr<EventRunner> &AbilityLocalRecord::GetEventRunner()
{
    return runner_;
}

void AbilityLocalRecord::SetEventRunner(const std::shared_ptr<EventRunner> &runner)
{
    runner_ = runner;
}

const sptr<IRemoteObject> &AbilityLocalRecord::GetToken()
{
    return token_;
}

const std::shared_ptr<AbilityImpl> &AbilityLocalRecord::GetAbilityImpl()
{
    return abilityImpl_;
}

void AbilityLocalRecord::SetAbilityImpl(const std::shared_ptr<AbilityImpl> &abilityImpl)
{
    abilityImpl_ = abilityImpl;
}

const sptr<AbilityThread> &AbilityLocalRecord::GetAbilityThread()
{
    return abilityThread_;
}

void AbilityLocalRecord::SetAbilityThread(const sptr<AbilityThread> &abilityThread)
{
    abilityThread_ = abilityThread;
}

void AbilityLocalRecord::SetWant(const std::shared_ptr<AAFwk::Want> &want)
{
    want_ = want;
}

const std::shared_ptr<AAFwk::Want> &AbilityLocalRecord::GetWant()
{
    return want_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
