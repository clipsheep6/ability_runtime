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

#ifndef OHOS_ABILITY_RUNTIME_CONNECT_SERVER_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CONNECT_SERVER_MANAGER_H

#include <mutex>
#include <unordered_map>

namespace OHOS::AbilityRuntime {
class ConnectServerManager final {
public:
    static ConnectServerManager& Get();

    void StartConnectServer(const std::string& bundleName);
    void StopConnectServer();
    void SetLayoutInspectorStatus(bool status)
    {
        layoutInspectorStatus_ = status;
    }
    bool GetlayoutInspectorStatus() const
    {
        return layoutInspectorStatus_;
    }
    bool AddInstance(int32_t instanceId, const std::string& instanceName = "PandaDebugger");
    void RemoveInstance(int32_t instanceId);
    void SendInspector(const std::string& jsonTreeStr, const std::string& jsonSnapshotStr);

private:
    ConnectServerManager() = default;
    ~ConnectServerManager();

    void* handlerConnectServerSo_ = nullptr;
    std::string bundleName_;

    std::mutex mutex_;
    std::unordered_map<int32_t, std::string> instanceMap_;
    bool layoutInspectorStatus_ = false;
    ConnectServerManager(const ConnectServerManager&) = delete;
    ConnectServerManager(ConnectServerManager&&) = delete;
    ConnectServerManager& operator=(const ConnectServerManager&) = delete;
    ConnectServerManager& operator=(ConnectServerManager&&) = delete;
};
} // namespace OHOS::AbilityRuntime

#endif // OHOS_ABILITY_RUNTIME_CONNECT_SERVER_MANAGER_H