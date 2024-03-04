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

#include "extension_record_factory.h"
#include "ability_util.h"
#include "app_utils.h"
#include "extension_record_manager.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
uint32_t GetPreCheckFlag(ExtensionAbilityType type)
{
    auto iter = EXTENSION_RECORD_CONFIG_MAP.find(type);
    if (iter == EXTENSION_RECORD_CONFIG_MAP.end()) {
        return PRE_CHECK_FLAG_NONE;
    }
    return iter->second.preCheckFlag;
}
}

ExtensionRecordFactory::ExtensionRecordFactory() = default;

ExtensionRecordFactory::~ExtensionRecordFactory() = default;

bool ExtensionRecordFactory::NeedReuse(const AAFwk::AbilityRequest &abilityRequest, int32_t &extensionRecordId)
{
    return false;
}

int32_t ExtensionRecordFactory::PreCheck(const AAFwk::AbilityRequest &abilityRequest, const std::string &hostBundleName)
{
    uint32_t preCheckFlag = GetPreCheckFlag(abilityRequest.extensionType);
    if (preCheckFlag == 0) {
        return ERR_OK;
    }
    if (preCheckFlag & PRE_CHECK_FLAG_CALLED_WITHIN_THE_BUNDLE) {
        if (hostBundleName != abilityRequest.abilityInfo.applicationName) {
            HILOG_WARN("not called within bundle.");
            return ERR_INVALID_VALUE;
        }
        CHECK_POINTER_AND_RETURN(abilityRequest.sessionInfo, ERR_INVALID_VALUE);
        auto callerToken = abilityRequest.sessionInfo->callerToken;
        auto callerAbilityRecord = AAFwk::Token::GetAbilityRecordByToken(callerToken);
        CHECK_POINTER_AND_RETURN(callerAbilityRecord, ERR_INVALID_VALUE);
        AppExecFwk::AbilityInfo abilityInfo = callerAbilityRecord->GetAbilityInfo();
        if (abilityInfo.type != AbilityType::PAGE) {
            HILOG_WARN("caller ability is not UIAbility.");
            return ERR_INVALID_VALUE;
        }
    }
    if (preCheckFlag & PRE_CHECK_FLAG_MULTIPLE_PROCESSES) {
        if (!AppUtils::GetInstance().IsMultiProcessModel()) {
            HILOG_WARN("not multi process model.");
            return ERR_INVALID_VALUE;
        }
    }
    return ERR_OK;
}

uint32_t ExtensionRecordFactory::GetExtensionProcessMode(
    const AAFwk::AbilityRequest &abilityRequest, bool &isHostSpecified)
{
    ExtensionRecordConfig config;
    auto iter = EXTENSION_RECORD_CONFIG_MAP.find(abilityRequest.extensionType);
    if (iter != EXTENSION_RECORD_CONFIG_MAP.end()) {
        config = iter->second;
    }

    // check host specified
    isHostSpecified = false;
    if (config.processModeSupport & PROCESS_MODE_HOST_SPECIFIED) {
        if (abilityRequest.want.HasParameter(PROCESS_MODE_HOST_SPECIFIED_KEY)) {
            isHostSpecified = true;
            return PROCESS_MODE_HOST_SPECIFIED;
        }
    }

    if (config.processModeSupport & PROCESS_MODE_HOST_INSTANCE) {
        if (abilityRequest.want.HasParameter(PROCESS_MODE_HOST_INSTANCE_KEY)) {
            bool hostInstance = abilityRequest.want.GetBoolParam(PROCESS_MODE_HOST_INSTANCE_KEY, false);
            if (hostInstance) {
                return PROCESS_MODE_INSTANCE;
            }
        }
    }

    if (abilityRequest.extensionProcessMode == ExtensionProcessMode::UNDEFINED) {
        return config.processModeDefault;
    }
    uint32_t inputProcessMode = 1 << static_cast<int32_t>(abilityRequest.extensionProcessMode);
    if (config.processModeSupport & inputProcessMode) {
        return inputProcessMode;
    }
    return config.processModeDefault;
}

int32_t ExtensionRecordFactory::CreateRecord(
    const AAFwk::AbilityRequest &abilityRequest, std::shared_ptr<ExtensionRecord> &extensionRecord)
{
    std::shared_ptr<AAFwk::AbilityRecord> abilityRecord = AAFwk::AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("Failed to create ability record");
        return ERR_NULL_OBJECT;
    }
    extensionRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extensionRecord->processMode_ = GetExtensionProcessMode(abilityRequest, extensionRecord->isHostSpecified_);
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
