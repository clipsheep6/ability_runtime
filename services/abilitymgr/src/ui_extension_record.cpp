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

#include "ui_extension_record.h"
#include "ability_util.h"
#include "extension_record_manager.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string UIEXTENSION_ABILITY_ID = "ability.want.params.uiExtensionAbilityId";
}

UIExtensionRecord::UIExtensionRecord(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord,
    const std::string &hostBundleName, int32_t extensionRecordId)
    : ExtensionRecord(abilityRecord, hostBundleName, extensionRecordId)
{}

UIExtensionRecord::~UIExtensionRecord() = default;

bool UIExtensionRecord::ContinueToGetCallerToken()
{
    return true;
}

int32_t UIExtensionRecord::NeedReuse(const AAFwk::AbilityRequest &abilityRequest)
{
    int32_t uiExtensionAbilityId = abilityRequest.sessionInfo->want.GetIntParam(UIEXTENSION_ABILITY_ID,
        INVALID_EXTENSION_RECORD_ID);
    if (uiExtensionAbilityId == INVALID_EXTENSION_RECORD_ID) {
        HILOG_DEBUG("UIEXTENSION_ABILITY_ID is not config, no reuse");
        return uiExtensionAbilityId;
    }
    HILOG_INFO("UIExtensionAbility id: %{public}d.", uiExtensionAbilityId);
    return uiExtensionAbilityId;
}

void UIExtensionRecord::Update(const AAFwk::AbilityRequest &abilityRequest)
{
    if (abilityRecord_ == nullptr) {
        HILOG_ERROR("abilityRecord_ is null");
        return;
    }
    abilityRecord_->SetSessionInfo(abilityRequest.sessionInfo);
}
} // namespace AbilityRuntime
} // namespace OHOS
