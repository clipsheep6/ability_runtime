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

#include <cstdint>

extern "C" {
int FFICJWantDelete = 0;
int FFICJWantGetWantInfo = 0;
int FFICJWantParamsDelete = 0;
int FFICJWantCreateWithWantInfo = 0;
int FFICJWantParseUri = 0;
int FFICJWantAddEntity = 0;
int FFICJElementNameCreateWithContent = 0;
int FFICJElementNameDelete = 0;
int FFICJElementNameGetElementNameInfo = 0;
int FFICJElementNameParamsDelete = 0;
int FFIAbilityGetAbilityContext = 0;
int FFIAbilityContextGetFilesDir = 0;
int FFIGetContext = 0;
int FFICreateNapiValue = 0;
int FFIGetArea  = 0;
int FFICJApplicationInfo = 0;
int FFIAbilityDelegatorRegistryGetAbilityDelegator = 0;
int FFIAbilityDelegatorStartAbility = 0;
int FFIAbilityDelegatorExecuteShellCommand = 0;
int FFIGetExitCode = 0;
int FFIGetStdResult = 0;
int FFIDump = 0;
int FFIAbilityDelegatorApplicationContext = 0;

struct AbilityContextBroker {
    int64_t isAbilityContextExisted = 1;
    int64_t getSizeOfStartOptions = 1;

    int64_t getAbilityInfo = 1;
    int64_t getHapModuleInfo = 1;
    int64_t getConfiguration = 1;

    int64_t startAbility = 1;
    int64_t startAbilityWithOption = 1;
    int64_t startAbilityWithAccount = 1;
    int64_t startAbilityWithAccountAndOption = 1;
    int64_t startServiceExtensionAbility = 1;
    int64_t startServiceExtensionAbilityWithAccount = 1;
    int64_t stopServiceExtensionAbility = 1;
    int64_t stopServiceExtensionAbilityWithAccount = 1;

    int64_t terminateSelf = 1;
    int64_t terminateSelfWithResult = 1;
    int64_t isTerminating = 1;

    int64_t connectAbility = 1;
    int64_t connectAbilityWithAccount = 1;
    int64_t disconnectAbility = 1;
    int64_t startAbilityForResult = 1;
    int64_t startAbilityForResultWithOption = 1;
    int64_t startAbilityForResultWithAccount = 1;
    int64_t startAbilityForResultWithAccountAndOption = 1;
    int64_t requestPermissionsFromUser = 1;

    int64_t setMissionLabel = 1;
    int64_t setMissionIcon = 1;
};

AbilityContextBroker* FFIAbilityContextGetBroker()
{
    static AbilityContextBroker globalBroker;
    return &globalBroker;
}

void RegisterCJAbilityStageFuncs() {}
void RegisterCJAbilityConnectCallbackFuncs() {}
void RegisterCJAbilityCallbacks() {}
void RegisterCJAbilityFuncs() {}
void FFIAbilityContextRequestDialogService() {}
}