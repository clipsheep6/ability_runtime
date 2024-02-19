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

#include "connect_server_manager.h"

#include <dlfcn.h>
#include <unistd.h>

#include "hilog_wrapper.h"

namespace OHOS::AbilityRuntime {
namespace {
std::string GetInstanceMapMessage(
    const std::string& messageType, int32_t instanceId, const std::string& instanceName, bool isWorker)
{
    std::string message;
    message.append("{\"type\":\"");
    message.append(messageType);
    message.append("\",\"instanceId\":");
    message.append(std::to_string(instanceId));
    message.append(",\"name\":\"");
    message.append(instanceName);
    message.append("\",\"tid\":");
    if (isWorker) {
        message.append(std::to_string(instanceId));
    } else {
        message.append(std::to_string(gettid()));
    }
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

std::mutex debuggerMutex_;
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
    if (handlerConnectServerSo_ == nullptr) {
        handlerConnectServerSo_ = dlopen("libconnectserver_debugger.z.so", RTLD_LAZY);
        if (handlerConnectServerSo_ == nullptr) {
            HILOG_ERROR("ConnectServerManager::StartConnectServer failed to open register library");
            return;
        }
    }
}

void ConnectServerManager::StartConnectServer(const std::string& bundleName, int socketFd, bool isLocalAbstract)
{
    HILOG_DEBUG("ConnectServerManager::StartConnectServer Start connect server");
    
    LoadConnectServerDebuggerSo();
    bundleName_ = bundleName;
    if (isLocalAbstract) {
        auto startServer = reinterpret_cast<StartServer>(dlsym(handlerConnectServerSo_, "StartServer"));
        if (startServer == nullptr) {
            HILOG_ERROR("ConnectServerManager::StartServer failed to find symbol 'StartServer'");
            return;
        }
        startServer(bundleName_);
        return;
    }
    auto startServerForSocketPair =
        reinterpret_cast<StartServerForSocketPair>(dlsym(handlerConnectServerSo_, "StartServerForSocketPair"));
    if (startServerForSocketPair == nullptr) {
        HILOG_ERROR("ConnectServerManager::StartServerForSocketPair failed to find symbol 'StartServer'");
        return;
    }
    startServerForSocketPair(socketFd);
}

void ConnectServerManager::StopConnectServer(bool isCloseSo)
{
    HILOG_DEBUG("ConnectServerManager::StopConnectServer Stop connect server");
    if (handlerConnectServerSo_ == nullptr) {
        HILOG_ERROR("ConnectServerManager::StopConnectServer handlerConnectServerSo_ is nullptr");
        return;
    }
    auto stopServer = reinterpret_cast<StopServer>(dlsym(handlerConnectServerSo_, "StopServer"));
    if (stopServer != nullptr) {
        stopServer(bundleName_);
    } else {
        HILOG_ERROR("ConnectServerManager::StopConnectServer failed to find symbol 'StopServer'");
    }
    if (isCloseSo) {
        dlclose(handlerConnectServerSo_);
        handlerConnectServerSo_ = nullptr;
    }
}


bool ConnectServerManager::StoreInstanceMessage(int32_t instanceId, const std::string& instanceName, bool isWorker)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto result = instanceMap_.try_emplace(instanceId, std::make_pair(instanceName, isWorker));
        if (!result.second) {
            HILOG_WARN("ConnectServerManager::StoreInstanceMessage Instance %{public}d already added", instanceId);
            return false;
        }
    }
    return true;
}

void ConnectServerManager::StoreDebuggerInfo(int tid, void* vm, const panda::JSNApi::DebugOption& debugOption,
    const DebuggerPostTask& debuggerPostTask, bool isDebugApp)
{
    std::lock_guard<std::mutex> lock(debuggerMutex_);
    if (g_debuggerInfo.find(tid) == g_debuggerInfo.end()) {
        g_debuggerInfo.emplace(tid, std::make_pair(vm, debuggerPostTask));
    }

    if (!isConnected_) {
        HILOG_WARN("ConnectServerManager::StoreDebuggerInfo not Connected");
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
        auto isWorker = instance.second.second;

        panda::EcmaVM* vm = reinterpret_cast<panda::EcmaVM*>(g_debuggerInfo[instanceId].first);
        std::unique_lock<std::mutex> lock(debuggerMutex_);
        const auto &debuggerPoskTask = g_debuggerInfo[instanceId].second;
        if (!debuggerPoskTask) {
            continue;
        }
        ConnectServerManager::Get().SendInstanceMessage(instanceId, instanceName, isWorker);
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
        dlsym(handlerConnectServerSo_, "SetConnectCallBack"));
    if (setConnectCallBack == nullptr) {
        HILOG_ERROR("ConnectServerManager::SetConnectedCallback failed to find symbol 'SetConnectCallBack'");
        return;
    }

    setConnectCallBack([](bool isConnected) {
        ConnectServerManager::Get().isConnected_ = isConnected;
    });
}

bool ConnectServerManager::SendInstanceMessage(int32_t instanceId, const std::string& instanceName, bool isWorker)
{
    HILOG_INFO("ConnectServerManager::SendInstanceMessage Add instance to connect server");
    LoadConnectServerDebuggerSo();

    auto setSwitchCallBack = reinterpret_cast<SetSwitchCallBack>(
    dlsym(handlerConnectServerSo_, "SetSwitchCallBack"));
    if (setSwitchCallBack == nullptr) {
        HILOG_INFO("ConnectServerManager::SendInstanceMessage failed to find symbol 'setSwitchCallBack'");
        return false;
    }
 
    auto storeMessage = reinterpret_cast<StoreMessage>(dlsym(handlerConnectServerSo_, "StoreMessage"));
    if (storeMessage == nullptr) {
        HILOG_ERROR("ConnectServerManager::SendInstanceMessage failed to find symbol 'StoreMessage'");
        return false;
    }
    
    setSwitchCallBack([this](bool status) { setStatus_(status); },
        [this](int32_t containerId) { createLayoutInfo_(containerId); }, instanceId);

    std::string message = GetInstanceMapMessage("addInstance", instanceId, instanceName, isWorker);
    storeMessage(instanceId, message);
 
    return true;
}

 
bool ConnectServerManager::AddInstance(int32_t instanceId, const std::string& instanceName, bool isWorker)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto result = instanceMap_.try_emplace(instanceId, std::make_pair(instanceName, isWorker));
        if (!result.second) {
            HILOG_WARN("ConnectServerManager::AddInstance Instance %{public}d already added", instanceId);
            return false;
        }
    }

    if (!isConnected_) {
        HILOG_WARN("ConnectServerManager::AddInstance not Connected");
        return false;
    }

    HILOG_DEBUG("ConnectServerManager::AddInstance Add instance to connect server");
    LoadConnectServerDebuggerSo();

    auto setSwitchCallBack = reinterpret_cast<SetSwitchCallBack>(
        dlsym(handlerConnectServerSo_, "SetSwitchCallBack"));
    if (setSwitchCallBack == nullptr) {
        HILOG_ERROR("ConnectServerManager::AddInstance failed to find symbol 'setSwitchCallBack'");
        return false;
    }
    setSwitchCallBack([this](bool status) { setStatus_(status); },
        [this](int32_t containerId) { createLayoutInfo_(containerId); }, instanceId);

    // Get the message including information of new instance, which will be send to IDE.
    std::string message = GetInstanceMapMessage("addInstance", instanceId, instanceName, isWorker);

    auto storeMessage = reinterpret_cast<StoreMessage>(dlsym(handlerConnectServerSo_, "StoreMessage"));
    if (storeMessage == nullptr) {
        HILOG_ERROR("ConnectServerManager::AddInstance failed to find symbol 'StoreMessage'");
        return false;
    }
    storeMessage(instanceId, message);

    // WaitForConnection() means the connection state of the connect server
    auto sendMessage = reinterpret_cast<SendMessage>(dlsym(handlerConnectServerSo_, "SendMessage"));
    if (sendMessage == nullptr) {
        HILOG_ERROR("ConnectServerManager::AddInstance failed to find symbol 'SendMessage'");
        return false;
    }
    // if connected, message will be sent immediately.
    sendMessage(message);
    return true;
}

void ConnectServerManager::RemoveInstance(int32_t instanceId)
{
    HILOG_DEBUG("ConnectServerManager::RemoveInstance Remove instance to connect server");
    if (handlerConnectServerSo_ == nullptr) {
        HILOG_ERROR("ConnectServerManager::RemoveInstance handlerConnectServerSo_ is nullptr");
        return;
    }

    std::string instanceName;
    bool isWorker;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = instanceMap_.find(instanceId);
        if (it == instanceMap_.end()) {
            HILOG_WARN("ConnectServerManager::RemoveInstance Instance %{public}d is not found", instanceId);
            return;
        }

        instanceName = std::move(it->second.first);
        isWorker = std::move(it->second.second);
        instanceMap_.erase(it);
    }

    auto waitForConnection = reinterpret_cast<WaitForConnection>(dlsym(handlerConnectServerSo_, "WaitForConnection"));
    if (waitForConnection == nullptr) {
        HILOG_ERROR("ConnectServerManager::RemoveInstance failed to find symbol 'WaitForConnection'");
        return;
    }

    // Get the message including information of deleted instance, which will be send to IDE.
    std::string message = GetInstanceMapMessage("destroyInstance", instanceId, instanceName, isWorker);

    auto removeMessage = reinterpret_cast<RemoveMessage>(dlsym(handlerConnectServerSo_, "RemoveMessage"));
    if (removeMessage == nullptr) {
        HILOG_ERROR("ConnectServerManager::RemoveInstance failed to find symbol 'RemoveMessage'");
        return;
    }
    removeMessage(instanceId);

    if (waitForConnection()) {
        return;
    }

    auto sendMessage = reinterpret_cast<SendMessage>(dlsym(handlerConnectServerSo_, "SendMessage"));
    if (sendMessage == nullptr) {
        HILOG_ERROR("ConnectServerManager::RemoveInstance failed to find symbol 'SendMessage'");
        return;
    }
    sendMessage(message);
}

void ConnectServerManager::SendInspector(const std::string& jsonTreeStr, const std::string& jsonSnapshotStr)
{
    HILOG_INFO("ConnectServerManager SendInspector Start");
    auto sendLayoutMessage = reinterpret_cast<SendMessage>(dlsym(handlerConnectServerSo_, "SendLayoutMessage"));
    if (sendLayoutMessage == nullptr) {
        HILOG_ERROR("ConnectServerManager::AddInstance failed to find symbol 'sendLayoutMessage'");
        return;
    }

    sendLayoutMessage(jsonTreeStr);
    sendLayoutMessage(jsonSnapshotStr);
    auto storeInspectorInfo = reinterpret_cast<StoreInspectorInfo>(
        dlsym(handlerConnectServerSo_, "StoreInspectorInfo"));
    if (storeInspectorInfo == nullptr) {
        HILOG_ERROR("ConnectServerManager::AddInstance failed to find symbol 'StoreInspectorInfo'");
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