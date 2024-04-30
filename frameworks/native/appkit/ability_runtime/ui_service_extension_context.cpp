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

#include "ui_service_extension_context.h"

#include "ability_connection.h"
#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {

ErrCode UIServiceExtensionContext::StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const
{
	ErrCode err = ERR_OK;
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Start ability begin, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    return err;
}

ErrCode UIServiceExtensionContext::TerminateSelf()
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "begin.");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "ServiceExtensionContext::TerminateAbility is failed %{public}d", err);
    }
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "%{public}s end.", __func__);
    return err;
}

ErrCode UIServiceExtensionContext::startAbilityByType(const std::string &abilityType,
    const AAFwk::Want &want, napi_callback_info info)
{
    //todo StartAbilityByType
    ErrCode err = ERR_OK;    
    return err;
}

}  // namespace AbilityRuntime
}  // namespace OHOS
