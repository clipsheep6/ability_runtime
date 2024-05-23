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

#include "cj_ability_connect_callback_object.h"

#include "cj_remote_object_ffi.h"
#include "hilog_wrapper.h"

using namespace OHOS::AbilityRuntime;

namespace {
CJAbilityConnectCallbackFuncs* g_cjAbilityConnectCallbackFuncs = nullptr;
}

void RegisterCJAbilityConnectCallbackFuncs(void (*registerFunc)(CJAbilityConnectCallbackFuncs* result))
{
    HILOG_INFO("RegisterCJAbilityConnectCallbackFuncs start.");
    if (g_cjAbilityConnectCallbackFuncs != nullptr) {
        HILOG_ERROR("Repeated registration for cangjie functions of CJAbilityConnectCallback.");
        return;
    }

    g_cjAbilityConnectCallbackFuncs = new CJAbilityConnectCallbackFuncs();
    registerFunc(g_cjAbilityConnectCallbackFuncs);
}

CJAbilityConnectCallback::~CJAbilityConnectCallback()
{
    if (g_cjAbilityConnectCallbackFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbilityConnectCallbacks are not registered");
        return;
    }
    g_cjAbilityConnectCallbackFuncs->release(callbackId_);
}

void CJAbilityConnectCallback::OnAbilityConnectDone(
    const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode)
{
    HILOG_INFO("OnAbilityConnectDone begin, resultCode:%{public}d", resultCode);
    if (g_cjAbilityConnectCallbackFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbilityConnectCallbacks are not registered");
        return;
    }

    ElementNameHandle elementNameHandle = const_cast<AppExecFwk::ElementName*>(&element);
    // The cj side is responsible for the release.
    auto cjRemoteObj = FFI::FFIData::Create<AppExecFwk::CJRemoteObject>(remoteObject);
    g_cjAbilityConnectCallbackFuncs->onConnect(callbackId_, elementNameHandle, cjRemoteObj->GetID(), resultCode);
}

void CJAbilityConnectCallback::OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode)
{
    HILOG_INFO("OnAbilityDisconnectDone begin, resultCode:%{public}d", resultCode);
    if (g_cjAbilityConnectCallbackFuncs == nullptr) {
        HILOG_ERROR("Cangjie functions for CJAbilityConnectCallbacks are not registered");
        return;
    }

    ElementNameHandle elementNameHandle = const_cast<AppExecFwk::ElementName*>(&element);
    g_cjAbilityConnectCallbackFuncs->onDisconnect(callbackId_, elementNameHandle, resultCode);
}
