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
#include "session/host/include/zidl/session_interface.h"

namespace OHOS {
namespace AbilityRuntime {
UIExtensionRecord::UIExtensionRecord(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord)
    : ExtensionRecord(abilityRecord)
{}

UIExtensionRecord::~UIExtensionRecord() = default;

bool UIExtensionRecord::ContinueToGetCallerToken()
{
    return true;
}

void UIExtensionRecord::Update(const AAFwk::AbilityRequest &abilityRequest)
{
    if (abilityRecord_ == nullptr) {
        HILOG_ERROR("abilityRecord_ is null");
        return;
    }
    abilityRecord_->SetSessionInfo(abilityRequest.sessionInfo);
}

void UIExtensionRecord::HandleNotifyUIExtensionTimeout(ErrorCode code)
{
    CHECK_POINTER(abilityRecord_);
    auto sessionInfo = abilityRecord_->GetSessionInfo();
    CHECK_POINTER(sessionInfo);
    sptr<Rosen::ISession> sessionProxy = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    if (sessionProxy == nullptr) {
        HILOG_DEBUG("Parsing session failed, is nullptr.");
        return;
    }
    sessionProxy->NotifyExtensionTimeout(code);
}

void UIExtensionRecord::LoadTimeout()
{
    HILOG_DEBUG("Called.");
    HandleNotifyUIExtensionTimeout(ErrorCode::LOAD_TIMEOUT);
    HILOG_DEBUG("Notify wms, the uiextension load time out.");
}

void UIExtensionRecord::ForegroundTimeout()
{
    HILOG_DEBUG("Called.");
    HandleNotifyUIExtensionTimeout(ErrorCode::FOREGROUND_TIMEOUT);
    HILOG_DEBUG("Notify wms, the uiextension move foreground time out.");
}

void UIExtensionRecord::BackgroundTimeout()
{
    HILOG_DEBUG("Called.");
    HandleNotifyUIExtensionTimeout(ErrorCode::BACKGROUND_TIMEOUT);
    HILOG_DEBUG("Notify wms, the uiextension move background time out.");
}

void UIExtensionRecord::TerminateTimeout()
{
    HILOG_DEBUG("Called.");
    HandleNotifyUIExtensionTimeout(ErrorCode::TERMINATE_TIMEOUT);
    HILOG_DEBUG("Notify wms, the uiextension terminate time out.");
}
} // namespace AbilityRuntime
} // namespace OHOS
