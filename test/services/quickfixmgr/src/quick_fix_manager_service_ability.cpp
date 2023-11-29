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

#include "quick_fix_manager_service_ability.h"

#include "hilog_wrapper.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
REGISTER_SYSTEM_ABILITY_BY_ID(QuickFixManagerServiceAbility, QUICK_FIX_MGR_SERVICE_ID, true);

QuickFixManagerServiceAbility::QuickFixManagerServiceAbility(const int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate), service_(nullptr)
{
    HILOG_DEBUG("function called.");
}

QuickFixManagerServiceAbility::~QuickFixManagerServiceAbility()
{
    HILOG_DEBUG("function called.");
}

void QuickFixManagerServiceAbility::OnStart()
{
    HILOG_INFO("function called.");
    if (service_ != nullptr) {
        HILOG_DEBUG("Quick fix manager service has started.");
        return;
    }

    service_ = QuickFixManagerService::GetInstance();
    if (service_ == nullptr) {
        HILOG_ERROR("instance is nullptr.");
        return;
    }

    if (!service_->Init()) {
        HILOG_ERROR("init failed.");
        return;
    }

    if (!Publish(service_)) {
        HILOG_ERROR("Publish failed.");
        return;
    }

    HILOG_INFO("Quick fix manager service start succeed.");
}

void QuickFixManagerServiceAbility::OnStop()
{
    HILOG_INFO("function called.");
    service_ = nullptr;
}
}  // namespace AAFwk
}  // namespace OHOS
