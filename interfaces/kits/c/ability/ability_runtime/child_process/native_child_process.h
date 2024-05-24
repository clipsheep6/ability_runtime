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

#ifndef OHOS_C_ABILITY_RUNTIME_NATIVE_CHILD_PROCESS_H
#define OHOS_C_ABILITY_RUNTIME_NATIVE_CHILD_PROCESS_H

#include "ipc_cparcel.h"

/**
 * @file native_child_process.h
 *
 * @brief Defines the functions for native child process management.
 * @library libchild_process.so
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 12
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief native child process error code
 * @since 12
 */
enum Ability_NativeChildProcess_ErrCode {
    /**
     * @error The operation completed successfully
     */
    NCP_NOERROR = 0,

    /**
     * @error Invalid param
     */
    NCP_ERR_INVALID_PARAM = 401,

    /**
     * @error Unsupport start native process
     */
    NCP_ERR_NOT_SUPPORTED = 801,

    /**
     * @error Internal error
     */
    NCP_ERR_INTERNAL = 16000050,

    /**
     * @error Can not start another during child process startup, try again after current child process started
     */
    NCP_ERR_BUSY = 16010001,

    /**
     * @error Start native child time out
     */
    NCP_ERR_TIMEOUT = 16010002,

    /**
     * @error Service process error
     */
    NCP_ERR_SERVICE = 16010003,

    /**
     * @error Multi process disabled, can not start child process
     */
    NCP_ERR_MULTI_PROCESS_DISABLED = 16010004,

    /**
     * @error Already in child process, only main process can start child
     */
    NCP_ERR_ALREADY_IN_CHILD = 16010005,

    /**
     * @error Max native child processes reached, can not start another
     */
    NCP_ERR_MAX_CHILD_PROCESSES_REACHED = 16010006,

    /**
     * @error Child process load library failed
     */
    NCP_ERR_CHILD_PROCESS_LOAD_LIB = 16010007,

    /**
     * @error Faild to invoke OnConnect method in library
     */
    NCP_ERR_CHILD_PROCESS_CONNECT = 16010008,
};


/**
 * @brief callback function for notify the child process start result, see <b>OH_Ability_CreateNativeChildProcess</b>
 *
 * @param errCode Zero if successful, an error otherwise, see <v>Ability_NativeChildProcess_ErrCode</b> for detail
 * @param remoteProxy IPC object implemented in the sharded lib loaded by child process; will be nullptr when failed
 * @since 12
 */
typedef void (*OH_Ability_OnNativeChildProcessStarted)(int errCode, OHIPCRemoteProxy *remoteProxy);

/**
 * @brief Create native child process for app and load shared library specified by param,
 * process startup result is asynchronously notified via callback
 * Lib file must be implemented and exported follow functions:
 *   1. OHIPCRemoteStub* NativeChildProcess_OnConnect()
 *   2. void NativeChildProcess_MainProc()
 *
 * Processing logic be like follows:
 *   Main Process:
 *     1. Call OH_Ability_CreateNativeChildProcess(libName, onProcessStartedCallback)
 *   Child Process:
 *     2. dlopen(libName)
 *     3. dlsym("NativeChildProcess_OnConnect") & dlsym("NativeChildProcess_MainProc")
 *     4. ipcRemote = NativeChildProcess_OnConnect()
 *     5. NativeChildProcess_MainProc()
 *   Main Process:
 *     6. onProcessStartedCallback(ipcRemote, errCode)
 *   Child Process:
 *     7. Process exit after NativeChildProcess_MainProc() method returned
 *
 * @param libName Name of the library file loaded by child process, can not be nullptr
 * @param onProcessStarted Callback for notify the child process start result
 * @return Zero if successful, an error otherwise, see <b>Ability_NativeChildProcess_ErrCode</b> for detail
 * @see OH_Ability_OnNativeChildProcessStarted
 * @since 12
 */
int OH_Ability_CreateNativeChildProcess(const char* libName,
                                        OH_Ability_OnNativeChildProcessStarted onProcessStarted);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // OHOS_C_ABILITY_RUNTIME_NATIVE_CHILD_PROCESS_H
