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

#include "verify_io_third_ability.h"
#include "app_log_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
void VerifyIoThirdAbility::OnStart(const Want &want)
{
    Ability::OnStart(want);
}
void VerifyIoThirdAbility::OnStop()
{
    Ability::OnStop();
}
void VerifyIoThirdAbility::OnActive()
{
    Ability::OnActive();
}
void VerifyIoThirdAbility::OnInactive()
{
    Ability::OnInactive();
}
void VerifyIoThirdAbility::OnBackground()
{
    Ability::OnBackground();
}

REGISTER_AA(VerifyIoThirdAbility)
void CleanCacheCallbackDerived::OnCleanCacheFinished(bool succeeded)
{}
void BundleStatusCallbackDerived::OnBundleStateChanged(
    const uint8_t installType, const int32_t resultCode, const std::string &resultMsg, const std::string &bundleName)
{}
sptr<IRemoteObject> CleanCacheCallbackDerived::AsObject()
{
    return nullptr;
}
sptr<IRemoteObject> BundleStatusCallbackDerived::AsObject()
{
    return nullptr;
}

}  // namespace AppExecFwk
}  // namespace OHOS