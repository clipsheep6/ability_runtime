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

#include "native_child_process.h"
#include <map>
#include <mutex>
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "native_child_callback.h"
#include "child_process_manager.h"

using namespace OHOS;
using namespace OHOS::AbilityRuntime;

namespace {

std::mutex g_MutexCallBackObj;
sptr<IRemoteObject> g_CallbackStub;
OH_Ability_OnNativeChildProcessStarted g_Callback = nullptr;

const std::map<ChildProcessManagerErrorCode, Ability_NativeChildProcess_ErrCode> CPM_ERRCODE_MAP = {
    { ChildProcessManagerErrorCode::ERR_OK, NCP_NOERROR },
    { ChildProcessManagerErrorCode::ERR_MULTI_PROCESS_MODEL_DISABLED, NCP_ERR_MULTI_PROCESS_DISABLED },
    { ChildProcessManagerErrorCode::ERR_ALREADY_IN_CHILD_PROCESS, NCP_ERR_ALREADY_IN_CHILD },
    { ChildProcessManagerErrorCode::ERR_GET_APP_MGR_FAILED, NCP_ERR_SERVICE },
    { ChildProcessManagerErrorCode::ERR_GET_APP_MGR_START_PROCESS_FAILED, NCP_ERR_SERVICE },
    { ChildProcessManagerErrorCode::ERR_UNSUPPORT_NATIVE_CHILD_PROCESS, NCP_ERR_NOT_SUPPORTED },
    { ChildProcessManagerErrorCode::ERR_MAX_NATIVE_CHILD_PROCESSES, NCP_ERR_MAX_CHILD_PROCESSES_REACHED },
    { ChildProcessManagerErrorCode::ERR_NATIVE_CHILD_PROCESS_LOAD_LIB, NCP_ERR_CHILD_PROCESS_LOAD_LIB },
    { ChildProcessManagerErrorCode::ERR_NATIVE_CHILD_PROCESS_CONNECT, NCP_ERR_CHILD_PROCESS_CONNECT },
};

int CvtChildProcessManagerErrCode(ChildProcessManagerErrorCode cpmErr)
{
    auto it = CPM_ERRCODE_MAP.find(cpmErr);
    if (it == CPM_ERRCODE_MAP.end()) {
        return NCP_ERR_INTERNAL;
    }

    return it->second;
}

void OnNativeChildProcessStartedWapper(int errCode, OHIPCRemoteProxy *ipcProxy)
{
    std::unique_lock autoLock(g_MutexCallBackObj);
    if (g_Callback != nullptr) {
        g_Callback(CvtChildProcessManagerErrCode(static_cast<ChildProcessManagerErrorCode>(errCode)), ipcProxy);
        g_Callback = nullptr;
    } else {
        TAG_LOGW(AAFwkTag::PROCESSMGR, "Remote call twice?");
    }

    g_CallbackStub.clear();
}

} // Anonymous namespace

int OH_Ability_CreateNativeChildProcess(const char* libName, OH_Ability_OnNativeChildProcessStarted onProcessStarted)
{
    if (libName == nullptr || *libName == '\0' || onProcessStarted == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Invalid libname or callback");
        return NCP_ERR_INVALID_PARAM;
    }

    std::string strLibName(libName);
    if (strLibName.find("../") != std::string::npos) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Do not allow use relative path");
        return NCP_ERR_INVALID_PARAM;
    }
    
    std::unique_lock autoLock(g_MutexCallBackObj);
    if (g_Callback != nullptr || g_CallbackStub != nullptr) {
        TAG_LOGW(AAFwkTag::PROCESSMGR, "Another native process process starting, try again later");
        return NCP_ERR_BUSY;
    }
    
    sptr<IRemoteObject> callbackStub(new (std::nothrow) NativeChildCallback(OnNativeChildProcessStartedWapper));
    if (!callbackStub) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Alloc callback ipc stub object faild.");
        return NCP_ERR_INTERNAL;
    }
    
    ChildProcessManager &mgr = ChildProcessManager::GetInstance();
    auto cpmErr = mgr.StartNativeChildProcessByAppSpawnFork(strLibName, callbackStub);
    if (cpmErr != ChildProcessManagerErrorCode::ERR_OK) {
        return CvtChildProcessManagerErrCode(cpmErr);
    }

    g_Callback = onProcessStarted;
    g_CallbackStub = callbackStub;
    return NCP_NOERROR;
}
