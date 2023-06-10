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

#include <cstring>
#include <unistd.h>

#include "ability_command.h"
#include "ability_tool_command.h"
#include "hilog_wrapper.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#ifdef A11Y_ENABLE
#include "accessibility_ability_command.h"
#endif // A11Y_ENABLE
int main(int argc, char* argv[])
{
    uint64_t tokenId;
    const char** perms = new const char* [6];
    perms[0] = "ohos.permission.DISTRIBUTED_DATASYNC";
    perms[1] = "ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS";
    perms[2] = "ohos.permission.UPDATE_CONFIGURATION";
    perms[3] = "ohos.permission.GET_RUNNING_INFO";
    perms[4] = "ohos.permission.MANAGE_MISSIONS";
    perms[5] = "ohos.permission.CLEAN_BACKGROUND_PROCESSES";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 6,
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

    if (strstr(argv[0], "aa") != nullptr) {
        OHOS::AAFwk::AbilityManagerShellCommand cmd(argc, argv);
        std::cout << cmd.ExecCommand();
    } else if (strstr(argv[0], "ability_tool") != nullptr) {
        OHOS::AAFwk::AbilityToolCommand cmd(argc, argv);
        std::cout << cmd.ExecCommand();
    } else {
#ifdef A11Y_ENABLE
        if (strstr(argv[0], "accessibility") != nullptr) {
            OHOS::AAFwk::AccessibilityAbilityShellCommand cmd(argc, argv);
            std::cout << cmd.ExecCommand();
        }
#endif // A11Y_ENABLE
    }
    fflush(stdout);
    _exit(0);
}
