/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "appfreeze_manager.h"

#include <fcntl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>

#include "file_ex.h"
#include "directory_ex.h"
#include "dfx_dump_catcher.h"
#include "parameter.h"
#include "faultloggerd_client.h"
#include "hiperf_client.h"
#include "hisysevent.h"

#include "app_mgr_client.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr char EVENT_KEY_UID[] = "UID";
constexpr char EVENT_KEY_PID[] = "PID";
constexpr char EVENT_KEY_MESSAGE[] = "MSG";
constexpr char EVENT_KEY_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_KEY_PROCESS_NAME[] = "PROCESS_NAME";
constexpr char EVENT_KEY_STACK[] = "STACK";
}

AppfreezeManager::AppfreezeManager()
{}

AppfreezeManager::~AppfreezeManager()
{}

uint64_t AppfreezeManager::GetMilliseconds()
{
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return millisecs.count();
}

bool AppfreezeManager::IsHandleAppfreeze(const std::string bundleName)
{
    if (bundleName.empty()) {
        return true;
    }
    const int buffSize = 128;
    char paramOutBuff[buffSize] = {0};
    GetParameter("appfreezeFiltration", "", paramOutBuff, buffSize - 1);

    std::string str(paramOutBuff);
    if (str.find(bundleName) != std::string::npos) {
        HILOG_WARN("appfreeze filtration %{public}s.", bundleName.c_str());
        return false;
    }
    return true;
}

int AppfreezeManager::AppfreezeHandle(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo)
{
    if (!IsHandleAppfreeze(appInfo.bundleName)) {
        return -1;
    }
    if (faultData.errorObject.name == "THREAD_BLOCK_3S"||
        faultData.errorObject.name == "LIFECYCLE_TIMEOUT_HALF"||
        faultData.errorObject.name == "APP_INPUT_BLOCK") {
        AcquireStack(faultData, appInfo);
    } else {
        NotifyANR(faultData, appInfo);
    }
    return 0;
}

int AppfreezeManager::TimeoutHandle(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo)
{
    if (!IsHandleAppfreeze(appInfo.bundleName)) {
        return -1;
    }
    FaultData faultNotifyData;
    faultNotifyData.errorObject.name = faultData.errorObject.name;
    faultNotifyData.errorObject.message = faultData.errorObject.message;
    faultNotifyData.errorObject.stack = faultData.errorObject.stack + "\n";
    faultNotifyData.faultType = FaultDataType::APP_FREEZE;
    faultNotifyData.errorObject.stack += CatcherStacktrace(appInfo.pid);
    return AppfreezeHandle(faultNotifyData, appInfo);
    
}

int AppfreezeManager::AcquireStack(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo)
{
    HILOG_INFO("called");
    int ret = 0;
    int pid = appInfo.pid;
    FaultData faultNotifyData;
    faultNotifyData.errorObject.name = faultData.errorObject.name;
    faultNotifyData.errorObject.message = faultData.errorObject.message;
    faultNotifyData.errorObject.stack = faultData.errorObject.stack + "\n";
    faultNotifyData.faultType = FaultDataType::APP_FREEZE;
    std::string& stack = faultNotifyData.errorObject.stack;
    std::set<int> pids = GetBinderPeerPids(faultNotifyData.errorObject.stack, pid);

    if (pids.empty()) {
        stack += "PeerBinder pids is empty\n";
    }
    ForkToDumpHiperf(pids, pid);
    for (auto& pidTemp : pids) {
        HILOG_INFO("pidTemp pids:%{public}d", pidTemp);
        if (pidTemp != pid) {
            std::string content = "\nPeerBinder catcher stacktrace for pid : " + std::to_string(pidTemp) + "\n";
            content += CatcherStacktrace(pidTemp);
            stack += content;
        }
    }

    ret = NotifyANR(faultNotifyData, appInfo);
    return ret;
}

int AppfreezeManager::NotifyANR(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo)
{
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK, faultData.errorObject.name,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, EVENT_KEY_UID, appInfo.uid,
        EVENT_KEY_PID, appInfo.pid, EVENT_KEY_PACKAGE_NAME, appInfo.bundleName,
        EVENT_KEY_PROCESS_NAME, appInfo.processName, EVENT_KEY_MESSAGE,
        faultData.errorObject.message, EVENT_KEY_STACK, faultData.errorObject.stack);

    HILOG_INFO("reportEvent:%{public}s, pid:%{public}d, bundleName:%{public}s. success",
        faultData.errorObject.name.c_str(), appInfo.pid, appInfo.bundleName.c_str());
    return 0;
}

std::map<int, std::set<int>> AppfreezeManager::BinderParser(std::ifstream& fin, std::string& stack) const
{
    std::map<int, std::set<int>> binderInfo;
    const int decimal = 10;
    std::string line;
    bool isBinderMatchup = false;
    stack += "BinderCatcher --\n\n";
    while (getline(fin, line)) {
        stack += line + "\n";
        if (isBinderMatchup) {
            continue;
        }

        if (line.find("async") != std::string::npos) {
            continue;
        }

        std::istringstream lineStream(line);
        std::vector<std::string> strList;
        std::string tmpstr;
        while (lineStream >> tmpstr) {
            strList.push_back(tmpstr);
        }

        auto SplitPhase = [](const std::string& str, uint16_t index) -> std::string {
            std::vector<std::string> strings;
            SplitStr(str, ":", strings);
            if (index < strings.size()) {
                return strings[index];
            }
            return "";
        };

        if (strList.size() == 7) { // 7: valid array size
            // 2: binder peer id,
            std::string server = SplitPhase(strList[2], 0);
            // 0: binder local id,
            std::string client = SplitPhase(strList[0], 0);
            // 5: binder wait time, s
            std::string wait = SplitPhase(strList[5], 1);
            if (server == "" || client == "" || wait == "") {
                continue;
            }
            int serverNum = std::strtol(server.c_str(), nullptr, decimal);
            int clientNum = std::strtol(client.c_str(), nullptr, decimal);
            int waitNum = std::strtol(wait.c_str(), nullptr, decimal);
            HILOG_INFO("server:%{public}d, client:%{public}d, wait:%{public}d", serverNum, clientNum, waitNum);
            binderInfo[clientNum].insert(serverNum);
        }
        if (line.find("context") != line.npos) {
            isBinderMatchup = true;
        }
    }
    HILOG_INFO("binderInfo size: %{public}zu", binderInfo.size());
    return binderInfo;
}

std::set<int> AppfreezeManager::GetBinderPeerPids(std::string& stack, int pid) const
{
    std::set<int> pids;
    std::ifstream fin;
    std::string path = LOGGER_BINDER_DEBUG_PROC_PATH;
    fin.open(path.c_str());
    if (!fin.is_open()) {
        HILOG_ERROR("open binder file failed, %{public}s.", path.c_str());
        stack += "open binder file failed :" + path + "\r\n";
        return pids;
    }

    stack += "\n\nPeerBinderCatcher -- pid==" + std::to_string(pid) + "\n\n";
    std::map<int, std::set<int>> binderInfo = BinderParser(fin, stack);
    fin.close();

    if (binderInfo.size() == 0 || binderInfo.find(pid) == binderInfo.end()) {
        return pids;
    }

    ParseBinderPids(binderInfo, pids, pid);
    for (auto& each : pids) {
        HILOG_INFO("each pids:%{public}d", each);
    }
    return pids;
}

void AppfreezeManager::ParseBinderPids(const std::map<int, std::set<int>>& binderInfo,
    std::set<int>& pids, int pid) const
{
    auto it = binderInfo.find(pid);
    if (it != binderInfo.end()) {
        for (auto& each : it->second) {
            pids.insert(each);
            ParseBinderPids(binderInfo, pids, each);
        }
    }
    
}

std::string AppfreezeManager::CatcherStacktrace(int pid) const
{
    HiviewDFX::DfxDumpCatcher dumplog;
    std::string ret;
    std::string msg;
    if (!dumplog.DumpCatch(pid, 0, msg)) {
        ret = "Failed to dump stacktrace for " + std::to_string(pid) + "\n" + msg;
    } else {
        ret = msg;
    }
    return ret;
}

void AppfreezeManager::DoExecHiperf(const std::string& fileName, const std::set<int>& pids, int pid)
{
    Developtools::HiPerf::HiperfClient::RecordOption opt;
    opt.SetOutputFilename(fileName);
    constexpr int collectTime = 1;
    opt.SetTimeStopSec(collectTime);
    opt.SetFrequency(1000); // 1000 : 1kHz
    opt.SetCallGraph("fp");
    opt.SetOffCPU(true);
    std::vector<pid_t> selectPids;
    selectPids.push_back(pid);
    for (const auto& pidTmp : pids) {
        if (pidTmp > 0) {
            selectPids.push_back(pidTmp);
        }
    }
    opt.SetSelectPids(selectPids);
    std::unique_ptr<Developtools::HiPerf::HiperfClient::Client> perfClient_ =
        std::make_unique<Developtools::HiPerf::HiperfClient::Client>(PEERBINDER_DIR);
    perfClient_->Start(opt);
}

void AppfreezeManager::ForkToDumpHiperf(const std::set<int>& pids, int pid)
{
#if defined(__aarch64__)
    std::string fileName = "hiperf-" + std::to_string(pid) + ".data";
    std::string fullPath = std::string(PEERBINDER_DIR) + "/" + fileName;
    if (access(fullPath.c_str(), F_OK) == 0) {
        struct stat statBuf;
        auto now = time(nullptr);
        if (stat(fullPath.c_str(), &statBuf) == -1) {
            HILOG_INFO("Failed to stat file, error:%{public}d.", errno);
            RemoveFile(fullPath);
        } else if (now - statBuf.st_mtime < PERF_LOG_EXPIRE_TIME) {
            HILOG_INFO("Target log has exist, reuse it.");
            return;
        } else {
            RemoveFile(fullPath);
        }
    }

    pid_t child = fork();
    if (child < 0) {
        // failed to fork child
        return;
    } else if (child == 0) {
        pid_t grandChild = fork();
        if (grandChild == 0) {
            DoExecHiperf(fileName, pids, pid);
        }
        _exit(0);
    } else {
        // do not left a zombie
        waitpid(child, nullptr, 0);
    }
#endif
}
}  // namespace AAFwk
}  // namespace OHOS