/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "connect_server_manager.h"

#include <dlfcn.h>
#include <unistd.h>

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS::AbilityRuntime {
namespace {
std::string GetInstanceMapMessage(
    const std::string& messageType, int32_t instanceId, const std::string& instanceName, int32_t tid)
{
    std::string message;
    message.append("{\"type\":\"");
    message.append(messageType);
    message.append("\",\"instanceId\":");
    message.append(std::to_string(instanceId));
    message.append(",\"name\":\"");
    message.append(instanceName);
    message.append("\",\"tid\":");
    message.append(std::to_string(tid));
    message.append(",\"apiType\":\"");
    message.append("stageMode\"");
    message.append(",\"language\":\"");
    message.append("ets\"");
    message.append("}");
    return message;
}
}

using StartServer = void (*)(const std::string&);
using StartServerForSocketPair = void (*)(int);
using SendMessage = void (*)(const std::string&);
using SendLayoutMessage = void (*)(const std::string&);
using StopServer = void (*)(const std::string&);
using StoreMessage = void (*)(int32_t, const std::string&);
using StoreInspectorInfo = void (*)(const std::string&, const std::string&);
using SetSwitchCallBack = void (*)(const std::function<void(bool)> &setStatus,
    const std::function<void(int32_t)> &createLayoutInfo, int32_t instanceId);
using SetConnectCallback = void (*)(const std::function<void(bool)>);
using RemoveMessage = void (*)(int32_t);
using WaitForConnection = bool (*)();

std::mutex g_debuggerMutex;
std::mutex g_loadsoMutex;
std::mutex ConnectServerManager::instanceMutex_;
std::unordered_map<int, std::pair<void*, const DebuggerPostTask>> g_debuggerInfo;

ConnectServerManager::~ConnectServerManager()
{
    StopConnectServer();
}

ConnectServerManager& ConnectServerManager::Get()
{
    std::lock_guard<std::mutex> lock(instanceMutex_);
    static ConnectServerManager connectServerManager;
    return connectServerManager;
}

void ConnectServerManager::LoadConnectServerDebuggerSo()
{
    std::lock_guard<std::mutex> lock(g_loadsoMutex);
    if (handlerConnectServerSo_ == nullptr) {
        handlerConnectServerSo_ = dlopen("libconnectserver_debugger.z.so", RTLD_LAZY);
        if (handlerConnectServerSo_ == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "ConnectServerManager::StartConnectServer failed to open register library");
            return;
        }
    }
}

void ConnectServerManager::StartConnectServer(const std::string& bundleName, int socketFd, bool isLocalAbstract)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "ConnectServerManager::StartConnectServer Start connect server");
    
    LoadConnectServerDebuggerSo();
    bundleName_ = bundleName;
    if (isLocalAbstract) {
        auto startServer = reinterpret_cast<StartServer>(dlsym(handlerConnectServerSo_, "StartServer"));
        if (startServer == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "ConnectServerManager::StartServer failed to find symbol 'StartServer'");
            return;
        }
        startServer(bundleName_);
        return;
    }
    auto startServerForSocketPair =
        reinterpret_cast<StartServerForSocketPair>(dlsym(handlerConnectServerSo_, "StartServerForSocketPair"));
    if (startServerForSocketPair == nullptr) {
        TAG_LOGE(
            AAFwkTag::JSRUNTIME, "ConnectServerManager::StartServerForSocketPair failed to find symbol 'StartServer'");
        return;
    }
    startServerForSocketPair(socketFd);
}

void ConnectServerManager::StopConnectServer(bool isCloseSo)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "ConnectServerManager::StopConnectServer Stop connect server");
    if (handlerConnectServerSo_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "ConnectServerManager::StopConnectServer handlerConnectServerSo_ is nullptr");
        return;
    }
    auto stopServer = reinterpret_cast<StopServer>(dlsym(handlerConnectServerSo_, "StopServer"));
    if (stopServer != nullptr) {
        stopServer(bundleName_);
    } else {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "ConnectServerManager::StopConnectServer failed to find symbol 'StopServer'");
    }
    if (isCloseSo) {
        dlclose(handlerConnectServerSo_);
        handlerConnectServerSo_ = nullptr;
    }
}


bool ConnectServerManager::StoreInstanceMessage(int32_t tid, int32_t instanceId, const std::string& instanceName)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto result = instanceMap_.try_emplace(instanceId, std::make_pair(instanceName, tid));
        if (!result.second) {
            TAG_LOGW(AAFwkTag::JSRUNTIME,
                "ConnectServerManager::StoreInstanceMessage Instance %{public}d already added", instanceId);
            return false;
        }
    }
    return true;
}

void ConnectServerManager::StoreDebuggerInfo(int32_t tid, void* vm, const panda::JSNApi::DebugOption& debugOption,
    const DebuggerPostTask& debuggerPostTask, bool isDebugApp)
{
    std::lock_guard<std::mutex> lock(g_debuggerMutex);
    if (g_debuggerInfo.find(tid) == g_debuggerInfo.end()) {
        g_debuggerInfo.emplace(tid, std::make_pair(vm, debuggerPostTask));
    }

    if (!isConnected_) {
        TAG_LOGW(AAFwkTag::JSRUNTIME, "ConnectServerManager::StoreDebuggerInfo not Connected");
        return;
    }

    panda::JSNApi::StoreDebugInfo(tid, reinterpret_cast<panda::EcmaVM*>(vm), debugOption, debuggerPostTask, isDebugApp);
}

void ConnectServerManager::SendDebuggerInfo(bool needBreakPoint, bool isDebugApp)
{
    ConnectServerManager::Get().SetConnectedCallback();
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& instance : instanceMap_) {
        auto instanceId = instance.first;
        auto instanceName = instance.second.first;
        auto tid = instance.second.second;

        panda::EcmaVM* vm = reinterpret_cast<panda::EcmaVM*>(g_debuggerInfo[instanceId].first);
        std::lock_guard<std::mutex> lock(g_debuggerMutex);
        const auto &debuggerPoskTask = g_debuggerInfo[instanceId].second;
        if (!debuggerPoskTask) {
            continue;
        }
        ConnectServerManager::Get().SendInstanceMessage(tid, instanceId, instanceName);
        auto storeDebugInfoTask = [needBreakPoint, isDebugApp, instanceId, vm, debuggerPoskTask, instanceName]() {
            panda::JSNApi::DebugOption debugOption = {ARK_DEBUGGER_LIB_PATH, isDebugApp ? needBreakPoint : false};
            panda::JSNApi::StoreDebugInfo(gettid(), vm, debugOption, debuggerPoskTask, isDebugApp);
        };

        debuggerPoskTask(storeDebugInfoTask);
    }
}

void ConnectServerManager::SetConnectedCallback()
{
    LoadConnectServerDebuggerSo();

    auto setConnectCallBack = reinterpret_cast<SetConnectCallback>(
        dlsym(handlerConnectServerSo_, "SetConnectCallback"));
    if (setConnectCallBack == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME,
            "ConnectServerManager::SetConnectedCallback failed to find symbol 'SetConnectCallBack'");
        return;
    }

    setConnectCallBack([](bool isConnected) {
        ConnectServerManager::Get().isConnected_ = isConnected;
    });
}

bool ConnectServerManager::SendInstanceMessage(int32_t tid, int32_t instanceId, const std::string& instanceName)
{
    TAG_LOGI(AAFwkTag::JSRUNTIME, "ConnectServerManager::SendInstanceMessage Add instance to connect server");
    LoadConnectServerDebuggerSo();

    auto setSwitchCallBack = reinterpret_cast<SetSwitchCallBack>(
    dlsym(handlerConnectServerSo_, "SetSwitchCallBack"));
    if (setSwitchCallBack == nullptr) {
        TAG_LOGI(
            AAFwkTag::JSRUNTIME, "ConnectServerManager::SendInstanceMessage failed to find symbol 'setSwitchCallBack'");
        return false;
    }
 
    auto storeMessage = reinterpret_cast<StoreMessage>(dlsym(handlerConnectServerSo_, "StoreMessage"));
    if (storeMessage == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "ConnectServerManager::SendInstanceMessage failed to find symbol 'StoreMessage'");
        return false;
    }
    
    setSwitchCallBack([this](bool status) { setStatus_(status); },
        [this](int32_t containerId) { createLayoutInfo_(containerId); }, instanceId);

    std::string message = GetInstanceMapMessage("addInstance", instanceId, instanceName, tid);
    storeMessage(instanceId, message);
 
    return true;
}

 
bool ConnectServerManager::AddInstance(int32_t tid, int32_t instanceId, const std::string& instanceName)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto result = instanceMap_.try_emplace(instanceId, std::make_pair(instanceName, tid));
        if (!result.second) {
            TAG_LOGW(
                AAFwkTag::JSRUNTIME, "ConnectServerManager::AddInstance Instance %{public}d already added", instanceId);
            return false;
        }
    }

    if (!isConnected_) {
        TAG_LOGW(AAFwkTag::JSRUNTIME, "ConnectServerManager::AddInstance not Connected");
        return false;
    }

    TAG_LOGD(AAFwkTag::JSRUNTIME, "ConnectServerManager::AddInstance Add instance to connect server");
    LoadConnectServerDebuggerSo();

    auto setSwitchCallBack = reinterpret_cast<SetSwitchCallBack>(
        dlsym(handlerConnectServerSo_, "SetSwitchCallBack"));
    if (setSwitchCallBack == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "ConnectServerManager::AddInstance failed to find symbol 'setSwitchCallBack'");
        return false;
    }
    setSwitchCallBack([this](bool status) { setStatus_(status); },
        [this](int32_t containerId) { createLayoutInfo_(containerId); }, instanceId);

    // Get the message including information of new instance, which will be send to IDE.
    std::string message = GetInstanceMapMessage("addInstance", instanceId, instanceName, tid);

    auto storeMessage = reinterpret_cast<StoreMessage>(dlsym(handlerConnectServerSo_, "StoreMessage"));
    if (storeMessage == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "ConnectServerManager::AddInstance failed to find symbol 'StoreMessage'");
        return false;
    }
    storeMessage(instanceId, message);

    // WaitForConnection() means the connection state of the connect server
    auto sendMessage = reinterpret_cast<SendMessage>(dlsym(handlerConnectServerSo_, "SendMessage"));
    if (sendMessage == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "ConnectServerManager::AddInstance failed to find symbol 'SendMessage'");
        return false;
    }
    // if connected, message will be sent immediately.
    sendMessage(message);
    return true;
}

void ConnectServerManager::RemoveInstance(int32_t instanceId)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "ConnectServerManager::RemoveInstance Remove instance to connect server");
    std::string instanceName;
    int32_t tid;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = instanceMap_.find(instanceId);
        if (it == instanceMap_.end()) {
            TAG_LOGW(AAFwkTag::JSRUNTIME, "ConnectServerManager::RemoveInstance Instance %{public}d is not found",
                instanceId);
            return;
        }

        instanceName = std::move(it->second.first);
        tid = std::move(it->second.second);
        instanceMap_.erase(it);
    }

    if (!isConnected_) {
        TAG_LOGW(AAFwkTag::JSRUNTIME, "ConnectServerManager::RemoveInstance not Connected");
        return;
    }

    LoadConnectServerDebuggerSo();
    auto waitForConnection = reinterpret_cast<WaitForConnection>(dlsym(handlerConnectServerSo_, "WaitForConnection"));
    if (waitForConnection == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "ConnectServerManager::RemoveInstance failed to find symbol 'WaitForConnection'");
        return;
    }

    // Get the message including information of deleted instance, which will be send to IDE.
    std::string message = GetInstanceMapMessage("destroyInstance", instanceId, instanceName, tid);

    auto removeMessage = reinterpret_cast<RemoveMessage>(dlsym(handlerConnectServerSo_, "RemoveMessage"));
    if (removeMessage == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "ConnectServerManager::RemoveInstance failed to find symbol 'RemoveMessage'");
        return;
    }
    removeMessage(instanceId);

    if (waitForConnection()) {
        return;
    }

    auto sendMessage = reinterpret_cast<SendMessage>(dlsym(handlerConnectServerSo_, "SendMessage"));
    if (sendMessage == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "ConnectServerManager::RemoveInstance failed to find symbol 'SendMessage'");
        return;
    }
    sendMessage(message);
}

void ConnectServerManager::SendInspector(const std::string& jsonTreeStr, const std::string& jsonSnapshotStr)
{
    TAG_LOGI(AAFwkTag::JSRUNTIME, "ConnectServerManager SendInspector Start");
    auto sendLayoutMessage = reinterpret_cast<SendMessage>(dlsym(handlerConnectServerSo_, "SendLayoutMessage"));
    if (sendLayoutMessage == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "ConnectServerManager::AddInstance failed to find symbol 'sendLayoutMessage'");
        return;
    }

    sendLayoutMessage(jsonTreeStr);
    sendLayoutMessage(jsonSnapshotStr);
    auto storeInspectorInfo = reinterpret_cast<StoreInspectorInfo>(
        dlsym(handlerConnectServerSo_, "StoreInspectorInfo"));
    if (storeInspectorInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "ConnectServerManager::AddInstance failed to find symbol 'StoreInspectorInfo'");
        return;
    }
    storeInspectorInfo(jsonTreeStr, jsonSnapshotStr);
}

void ConnectServerManager::SetLayoutInspectorCallback(
    const std::function<void(int32_t)>& createLayoutInfo, const std::function<void(bool)>& setStatus)
{
    createLayoutInfo_ = createLayoutInfo;
    setStatus_ = setStatus;
}

std::function<void(int32_t)> ConnectServerManager::GetLayoutInspectorCallback()
{
    return createLayoutInfo_;
}

} // namespace OHOS::AbilityRuntime