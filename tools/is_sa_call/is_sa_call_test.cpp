/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "is_sa_call_test.h"
#include "ability_manager_errors.h"
#include "hilog_wrapper.h"
#include "parameters.h"
#define private public
#define protected public
#include "permission_verification.h"
#undef private
#undef protected

using namespace std;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
using namespace OHOS;

bool MockSaWithNoPermission()
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission(false);
    auto permission = AAFwk::PermissionVerification::GetInstance();
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;

    return permission->VerifyRunningInfoPerm() &&
        permission->VerifyControllerPerm() &&
        permission->VerifyAccountPermission() == ERR_OK &&
        permission->VerifyMissionPermission() &&
        permission->VerifyAppStateObserverPermission() == ERR_OK &&
        permission->JudgeInvisibleAndBackground(verificationInfo) == ERR_OK &&
        permission->VerifyPrepareTerminatePermission();
}

bool MockSaWithPermission()
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission(true);
    auto permission = AAFwk::PermissionVerification::GetInstance();
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;

    return permission->VerifyRunningInfoPerm() &&
        permission->VerifyControllerPerm() &&
        permission->VerifyAccountPermission() == ERR_OK &&
        permission->VerifyMissionPermission() &&
        permission->VerifyAppStateObserverPermission() == ERR_OK &&
        permission->JudgeInvisibleAndBackground(verificationInfo) == ERR_OK &&
        permission->VerifyPrepareTerminatePermission();
}

int main()
{
    cout << "===========BEGIN============" << endl;
    cout << "is_sa_call_0100 : " << (MockSaWithNoPermission() ? "true" : "false") << endl;
    cout << "is_sa_call_0200 : " << (MockSaWithPermission() ? "true" : "false") << endl;
    cout << "============END=============" << endl;
    return 0;
}