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

#include "advanced_security_mode_manager.h"

#include <string>

#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "parameters.h"
#include "time_util.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string PARAM_GLOBAL_ADVANCED_MODE_STATE = "ohos.boot.advsecmode.state";
}
AdvancedSecurityModeManager::AdvancedSecurityModeManager()
{
    TAG_LOGD(AAFwkTag::APPMGR, "ASMM constructor");
}

AdvancedSecurityModeManager::~AdvancedSecurityModeManager()
{
    TAG_LOGD(AAFwkTag::APPMGR, "ASMM deconstructor");
}

void AdvancedSecurityModeManager::Init()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    int32_t state = OHOS::system::GetIntParameter<int32_t>(PARAM_GLOBAL_ADVANCED_MODE_STATE, 0);
    isAdvSecModeEnabled_ = state > 0;
    TAG_LOGI(AAFwkTag::APPMGR, "ASMM Init isAdvSecModeEnabled:%{public}d.", isAdvSecModeEnabled_);
}

bool AdvancedSecurityModeManager::IsJITEnabled()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "ASMM IsJITEnabled %{public}d", !isAdvSecModeEnabled_);
    return !isAdvSecModeEnabled_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
