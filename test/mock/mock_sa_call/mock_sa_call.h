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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_IS_SA_CALL_TEST_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_IS_SA_CALL_TEST_H

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS::AAFwk {
class IsMockSaCall {
public:
    static void IsMockSaCallWithPermission()
    {
        uint64_t tokenId;
        const char* perms[] = {
            perms[0] = "ohos.permission.ACCESS_DLP_FILE",
            perms[1] = "ohos.permission.CLEAN_APPLICATION_DATA",
            perms[2] = "ohos.permission.CLEAN_BACKGROUND_PROCESSES",
            perms[3] = "ohos.permission.GET_RUNNING_INFO",
            perms[4] = "ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS",
            perms[5] = "ohos.permission.MANAGE_MISSIONS",
            perms[6] = "ohos.permission.RUNNING_STATE_OBSERVER",
            perms[7] = "ohos.permission.SET_ABILITY_CONTROLLER",
            perms[8] = "ohos.permission.UPDATE_CONFIGURATION",
            perms[9] = "ohos.permission.INSTALL_BUNDLE",
            perms[10] = "ohos.permission.GET_BUNDLE_INFO_PRIVILEGED",
            perms[11] = "ohos.permission.START_INVISIBLE_ABILITY",
            perms[12] = "ohos.permission.START_ABILITIES_FROM_BACKGROUND",
            perms[13] = "ohos.permission.START_ABILIIES_FROM_BACKGROUND",
            perms[14] = "ohos.permission.ABILITY_BACKGROUND_COMMUNICATION",
            perms[15] = "ohos.permission.MANAGER_ABILITY_FROM_GATEWAY",
            perms[16] = "ohos.permission.PROXY_AUTHORIZATION_URI",
            perms[17] = "ohos.permission.EXEMPT_AS_CALLER",
            perms[18] = "ohos.permission.EXEMPT_AS_TARGET",
            perms[19] = "ohos.permission.PREPARE_APP_TERMINATE",
            perms[20] = "ohos.permission.START_ABILITY_WITH_ANIMATION",
            perms[21] = "ohos.permission.MANAGE_APP_BOOT_INTERNAL",
            perms[22] = "ohos.permission.CONNECT_UI_EXTENSION_ABILITY",
            perms[23] = "ohos.permission.START_RECENT_ABILITY"
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = static_cast<int32_t>(sizeof(perms)/sizeof(perms[0])),
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .aplStr = "system_core",
        };
        infoInstance.processName = "distributedsched";
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    }

    static void IsMockCheckObserverCallerPermission()
    {
        uint64_t tokenId;
        const char* perms[] = {
            perms[0] = "ohos.permission.RUNNING_STATE_OBSERVER",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = static_cast<int32_t>(sizeof(perms)/sizeof(perms[0])),
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .aplStr = "system_core",
        };
        infoInstance.processName = "memmgrservice";
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    }

    static void IsMockProcessCachePermission()
    {
        uint64_t tokenId;
        const char* perms[] = {
            perms[0] = "ohos.permission.SET_PROCESS_CACHE_STATE",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = static_cast<int32_t>(sizeof(perms)/sizeof(perms[0])),
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .aplStr = "system_core",
        };
        infoInstance.processName = "distributedsched";
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    }
};
}  // namespace OHOS::AAFwk
#endif // UNITTEST_OHOS_ABILITY_RUNTIME_IS_SA_CALL_TEST_H