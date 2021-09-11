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

#ifndef _VERIFY_IO_SEVENTH_ABILITY_
#define _VERIFY_IO_SEVENTH_ABILITY_
#include "ability_loader.h"
#include "common_event.h"
namespace OHOS {
namespace AppExecFwk {
class VerifyIoSeventhAbility : public Ability {
public:
    virtual void OnStart(const Want &want) override;
    virtual void OnStop() override;
    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void OnBackground() override;

private:
    OHOS::Parcel parcelForCommonEventData_;
    OHOS::Parcel parcelForCommonEventSubscribeInfo_;
    OHOS::Parcel parcelForCommonEventPublishInfo_;
    OHOS::Parcel parcelForMatchingSkills_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  //_VERIFY_IO_SEVENTH_ABILITY_