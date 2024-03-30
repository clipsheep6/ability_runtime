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

#include "mock_native_token.h"

namespace OHOS {
namespace AppExecFwk {
void MockNativeToken::SetNativeToken()
{
    int permission = 0;
    int permission1 = 1;
    int permission2 = 2;
    int permission3 = 3;
    int permission4 = 4;
    int permission5 = 5;
    int permission6 = 6;
    uint64_t tokenId;
    const char** perms = new const char* [7];
    perms[permission] = "ohos.permission.DISTRIBUTED_DATASYNC";
    perms[permission1] = "ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS";
    perms[permission2] = "ohos.permission.UPDATE_CONFIGURATION";
    perms[permission3] = "ohos.permission.GET_RUNNING_INFO";
    perms[permission4] = "ohos.permission.MANAGE_MISSIONS";
    perms[permission5] = "ohos.permission.CLEAN_BACKGROUND_PROCESSES";
    perms[permission6] = "ohos.permission.RUNNING_STATE_OBSERVER";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 7,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .aplStr = "system_core",
    };

    infoInstance.processName = "SetUpTestCase";
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    delete[] perms;
}
}  // namespace AppExecFwk
}  // namespace OHOS