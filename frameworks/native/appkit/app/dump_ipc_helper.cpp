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

#include "dump_ipc_helper.h"

#include <vector>
#include <sys/types.h>
#include <unistd.h>

#include "string_ex.h"
#include "ipc_payload_statistics.h"

namespace OHOS {
namespace AppExecFwk {
const std::string DUMP_IPC_CMD_SUCCESS = "success";
const std::string DUMP_IPC_CMD_FAIL = "fail";

void DumpIpcHelper::DumpIpcStart(std::string& result)
{
    result += "StartIpcStatistics pid: " + std::to_string(getpid()) + "\t";
    if (IPCPayloadStatistics::StartStatistics()) {
        result += DUMP_IPC_CMD_SUCCESS;
        return;
    }
    result += DUMP_IPC_CMD_FAIL;
}

void DumpIpcHelper::DumpIpcStop(std::string& result)
{
    result += "StopIpcStatistics pid: " + std::to_string(getpid()) + "\t";
    if (IPCPayloadStatistics::StopStatistics()) {
        result += DUMP_IPC_CMD_SUCCESS;
        return;
    }
    result += DUMP_IPC_CMD_FAIL;
}

void DumpIpcHelper::DumpIpcStat(std::string& result)
{
    result += "********************************GlobalStatisticsInfo********************************";
    result += "\nCurrentPid:";
    result += std::to_string(getpid());
    result += "\nTotalCount:";
    result += std::to_string(IPCPayloadStatistics::GetTotalCount());
    result += "\nTotalTimeCost:";
    result += std::to_string(IPCPayloadStatistics::GetTotalCost());
    std::vector<int32_t> pids = IPCPayloadStatistics::GetPids();
    for (int32_t pid : pids) {
        result += "\n--------------------------------ProcessStatisticsInfo-------------------------------";
        result += "\nCallingPid:";
        result += std::to_string(pid);
        result += "\nCallingPidTotalCount:";
        result += std::to_string(IPCPayloadStatistics::GetCount(pid));
        result += "\nCallingPidTotalTimeCost:";
        result += std::to_string(IPCPayloadStatistics::GetCost(pid));
        std::vector<OHOS::IPCInterfaceInfo> ipcInterfaceInfos = IPCPayloadStatistics::GetDescriptorCodes(pid);
        for (const auto& ipcInterfaceInfo : ipcInterfaceInfos) {
            result += "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~InterfaceStatisticsInfo~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
            result += "\nDescriptorCode:";
            result += Str16ToStr8(ipcInterfaceInfo.desc) + std::string("_") + std::to_string(ipcInterfaceInfo.code);
            result += "\nDescriptorCodeCount:";
            result += std::to_string(
                IPCPayloadStatistics::GetDescriptorCodeCount(pid, ipcInterfaceInfo.desc, ipcInterfaceInfo.code));
            result += "\nDescriptorCodeTimeCost:";
            result += "\nTotal:";
            OHOS::IPCPayloadCost descriptorCodeCost = IPCPayloadStatistics::GetDescriptorCodeCost(
                pid, ipcInterfaceInfo.desc, ipcInterfaceInfo.code);
            result += std::to_string(descriptorCodeCost.totalCost);
            result += " | Max:";
            result += std::to_string(descriptorCodeCost.maxCost);
            result += " | Min:";
            result += std::to_string(descriptorCodeCost.minCost);
            result += " | Avg:";
            result += std::to_string(descriptorCodeCost.averCost);
            result += "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
        }
        result += "\n------------------------------------------------------------------------------------";
    }
    result += "\n************************************************************************************\n";
}
}  // namespace AppExecFwk
}  // namespace OHOS
