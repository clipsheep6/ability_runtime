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

#include "quick_fix_manager_service.h"

#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<QuickFixManagerService>::GetInstance().get());
sptr<QuickFixManagerService> QuickFixManagerService::instance_;

QuickFixManagerService::QuickFixManagerService() : SystemAbility(QUICK_FIX_MGR_SERVICE_ID, true) {}

QuickFixManagerService::~QuickFixManagerService()
{
    if (quickFixStubImpl_ != nullptr) {
        quickFixStubImpl_ = nullptr;
    }
}

void QuickFixManagerService::OnStart()
{
    HILOG_INFO("OnStart is called.");
    if (!Init()) {
        HILOG_ERROR("Quick fix manager service init failed.");
        return;
    }

    instance_ = DelayedSingleton<QuickFixManagerService>::GetInstance().get();
    if (instance_ == nullptr) {
        HILOG_ERROR("Quick fix manager service instance is nullptr.");
        return;
    }

    bool ret = Publish(instance_);
    if (!ret) {
        HILOG_ERROR("Publish quick fix manager service failed.");
        return;
    }

    HILOG_INFO("Quick fix manager service start success.");
}

void QuickFixManagerService::OnStop()
{
    HILOG_INFO("OnStop is called.");
}

bool QuickFixManagerService::Init()
{
    if (running_) {
        HILOG_WARN("Quick fix manager service has been inited.");
        return true;
    }

    if (quickFixStubImpl_ == nullptr) {
        quickFixStubImpl_ = std::make_shared<QuickFixManagerStubImpl>();
    }

    running_ = true;
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
