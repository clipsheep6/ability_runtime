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

#include "native_child_callback.h"
#include "hilog_tag_wrapper.h"
#include "ipc_inner_object.h"
#include "child_process_manager_error_utils.h"

namespace OHOS {
namespace AbilityRuntime {

NativeChildCallback::NativeChildCallback(OH_Ability_OnNativeChildProcessStarted cb)
    : NativeChildNotifyStub(), callback_(cb)
{
}

void NativeChildCallback::OnNativeChildStarted(const sptr<IRemoteObject> &nativeChild)
{
    if (callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Native child process started, but callback is null?");
        return;
    }

    if (!nativeChild) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Native child process ipc object is null");
        return;
    }
    
    TAG_LOGI(AAFwkTag::PROCESSMGR, "Native child process started");
    sptr<IRemoteObject> ipcRemote = nativeChild;
    OHIPCRemoteProxy *ipcProxy = CreateIPCRemoteProxy(ipcRemote);
    if (ipcProxy == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Convert inner ipc object to OHIPCRemoteProxy point failed");
        callback_(NCP_ERR_INTERNAL, nullptr);
        return;
    }

    callback_(static_cast<int32_t>(ChildProcessManagerErrorCode::ERR_OK), ipcProxy);
}

void NativeChildCallback::OnError(int32_t errCode)
{
    if (callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Native child process start failed, but callback is null?");
        return;
    }

    TAG_LOGI(AAFwkTag::PROCESSMGR, "Native child process start failed, err %{public}d", errCode);
    callback_(errCode, nullptr);
}

} // namespace AbilityRuntime
} // namespace OHOS
