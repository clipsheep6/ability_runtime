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

#include <cstdint>

extern "C" {
int FFICJWantDelete();
int FFICJWantGetWantInfo();
int FFICJWantParamsDelete();
int FFICJWantCreateWithWantInfo();
int FFICJWantParseUri();
int FFICJWantAddEntity();
int FFICJElementNameCreateWithContent();
int FFICJElementNameDelete();
int FFICJElementNameGetElementNameInfo();
int FFICJElementNameParamsDelete();
int FFIAbilityGetAbilityContext();
int FFIAbilityContextGetFilesDir();
int FFIGetContext();
int FFICreateNapiValue();
int FFIGetArea();
int FFICJApplicationInfo();
int FFIAbilityDelegatorRegistryGetAbilityDelegator();
int FFIAbilityDelegatorStartAbility();
int FFIAbilityDelegatorExecuteShellCommand();
int FFIGetExitCode();
int FFIGetStdResult();
int FFIDump();
int FFIAbilityDelegatorApplicationContext();

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