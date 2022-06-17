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

#include "ui_service_mgr_client_mock.h"

#include <csignal>

#include "dialog_callback_stub.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

// external dependence
#include "ability_manager_client.h"
#include "display_manager.h"

namespace OHOS {
namespace Ace {
std::shared_ptr<UIServiceMgrClientMock> UIServiceMgrClientMock::instance_ = nullptr;
int UIServiceMgrClientMock::pid_ = 0;
std::string UIServiceMgrClientMock::code_ = "0";
std::mutex UIServiceMgrClientMock::mutex_;
namespace {
const std::string EVENT_WAITING_CODE = "0";
const std::string EVENT_CLOSE_CODE = "1";
}

std::shared_ptr<UIServiceMgrClientMock> UIServiceMgrClientMock::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<UIServiceMgrClientMock>();
        }
    }
    return instance_;
}

UIServiceMgrClientMock::UIServiceMgrClientMock()
{}

UIServiceMgrClientMock::~UIServiceMgrClientMock()
{}

ErrCode UIServiceMgrClientMock::RegisterCallBack(const AAFwk::Want& want, const sptr<IUIService>& uiService)
{
    return 0;
}

ErrCode UIServiceMgrClientMock::UnregisterCallBack(const AAFwk::Want& want)
{
    return 0;
}

ErrCode UIServiceMgrClientMock::Push(const AAFwk::Want& want, const std::string& name, const std::string& jsonPath,
    const std::string& data, const std::string& extraData)
{
    return 0;
}

ErrCode UIServiceMgrClientMock::Request(const AAFwk::Want& want, const std::string& name, const std::string& data)
{
    return 0;
}

ErrCode UIServiceMgrClientMock::ReturnRequest(const AAFwk::Want& want, const std::string& source,
    const std::string& data, const std::string& extraData)
{
    return 0;
}

ErrCode UIServiceMgrClientMock::ShowDialog(const std::string& name, const std::string& params,
    OHOS::Rosen::WindowType windowType, int x, int y, int width, int height, DialogCallback callback, int* id)
{
    if (code_ == EVENT_WAITING_CODE) {
        return 0;
    }
    if (code_ == EVENT_CLOSE_CODE) {
        kill(pid_, SIGKILL);
    }
    return 0;
}

ErrCode UIServiceMgrClientMock::CancelDialog(int32_t id)
{
    return 0;
}

ErrCode UIServiceMgrClientMock::UpdateDialog(int32_t id, const std::string& data)
{
    return 0;
}

ErrCode UIServiceMgrClientMock::ShowAppPickerDialog(
    const AAFwk::Want& want, const std::vector<AppExecFwk::AbilityInfo>& abilityInfos, int32_t userId)
{
    return 0;
}

ErrCode UIServiceMgrClientMock::Connect()
{
    return 0;
}

const std::string UIServiceMgrClientMock::GetPickerDialogParam(
    const AAFwk::Want& want, const std::vector<AppExecFwk::AbilityInfo>& abilityInfos, bool wideScreen) const
{
    return "";
}

void UIServiceMgrClientMock::GetDisplayPosition(
    int32_t& offsetX, int32_t& offsetY, int32_t& width, int32_t& height, bool& wideScreen)
{
    return;
}
}  // namespace Ace
}  // namespace OHOS
