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

#ifndef FOUNDATION_ACE_ACE_ENGINE_ADAPTER_OHOS_CPP_ACE_SERVICE_ABILITY_H
#define FOUNDATION_ACE_ACE_ENGINE_ADAPTER_OHOS_CPP_ACE_SERVICE_ABILITY_H

#include <string>
#include <vector>
#include "ability.h"
#include "ability_loader.h"
#include "frameworks/bridge/pa_backend/pa_backend.h"
#include "want.h"
#include "iremote_object.h"

namespace OHOS::Ace {
class AceServiceAbility final : public OHOS::AppExecFwk::Ability {
public:
    AceServiceAbility()
    {
        abilityId_ = instanceId_;
        instanceId_++;
    }
    virtual ~AceServiceAbility() = default;

    void OnStart(const OHOS::AAFwk::Want& want) override;
    void OnStop() override;
    sptr<IRemoteObject> OnConnect(const OHOS::AAFwk::Want &want) override;
    void OnDisconnect(const OHOS::AAFwk::Want &want) override;

    virtual OHOS::AppExecFwk::FormProviderInfo OnCreate(const OHOS::AAFwk::Want &want) override;
    virtual void OnDelete(const int64_t formId) override;
    virtual void OnTriggerEvent(const int64_t formId, const std::string &message) override;
    virtual void OnUpdate(const int64_t formId) override;
    virtual void OnCastTemptoNormal(const int64_t formId) override;
    virtual void OnVisibilityChanged(const std::map<int64_t, int32_t>& formEventsMap) override;
    virtual void OnAcquireState(const OHOS::AAFwk::Want &want);

private:
    int32_t abilityId_ = 100000;

    static int32_t instanceId_;
    static const std::string START_PARAMS_KEY;
    static const std::string URI;
    BackendType type_ = BackendType::FORM;
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_ACE_ENGINE_ADAPTER_OHOS_CPP_ACE_SERVICE_ABILITY_H
