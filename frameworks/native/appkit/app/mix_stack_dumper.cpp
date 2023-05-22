/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "mix_stack_dumper.h"

#include <ctime>
#include <mutex>

#include <dirent.h>
#include <securec.h>
#include <unistd.h>
#include <sigchain.h>

#include "dfx_dump_catcher.h"
#include "dfx_dump_res.h"
#include "faultloggerd_client.h"
#include "file_ex.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"

namespace OHOS {
namespace AppExecFwk {
std::weak_ptr<EventHandler> MixStackDumper::signalHandler_;
std::weak_ptr<OHOSApplication> MixStackDumper::application_;
static std::mutex g_mutex;
using OHOS::HiviewDFX::NativeFrame;
namespace {
static const char PID_STR_NAME[] = "Pid:";
static const char PPID_STR_NAME[] = "PPid:";
static const char NSPID_STR_NAME[] = "NSpid:";
static const char PROC_SELF_STATUS_PATH[] = "/proc/self/status";
static const std::string PROC_SELF_CMDLINE_PATH = "/proc/self/cmdline";
static constexpr int STATUS_LINE_SIZE = 1024;
static constexpr int FRAME_BUF_LEN = 1024;
static constexpr int HEADER_BUF_LEN = 512;
static constexpr int NAMESPACE_MATCH_NUM = 2;
static bool hasInstalled = false;
typedef struct ProcInfo {
    int tid;
    int pid;
    int ppid;
    bool ns;
} ProcInfo;
}

typedef void (*DumpSignalHandlerFunc) (int sig, siginfo_t *si, void *context);
static DumpSignalHandlerFunc g_dumpSignalHandlerFunc = nullptr;
static pid_t g_targetDumpTid = -1;
static struct ProcInfo g_procInfo = {0};

static std::string PrintJsFrame(const JsFrames& jsFrame)
{
    return "  at " + jsFrame.functionName + " (" + jsFrame.fileName + ":" + jsFrame.pos + ")\n";
}

static std::string PrintNativeFrame(const NativeFrame& frame)
{
    char buf[FRAME_BUF_LEN] = {0};
    std::string mapName = frame.binaryName;
    if (mapName.empty()) {
        mapName = "Unknown";
    }

#ifdef __LP64__
    char frameFormatWithMapName[] = "#%02zu pc %016" PRIx64 " %s\n";
    char frameFormatWithFuncName[] = "#%02zu pc %016" PRIx64 " %s(%s+%" PRIu64 ")\n";
#else
    char frameFormatWithMapName[] = "#%02zu pc %08" PRIx64 " %s\n";
    char frameFormatWithFuncName[] = "#%02zu pc %08" PRIx64 " %s(%s+%" PRIu64 ")\n";
#endif

    if (frame.funcName.empty()) {
        int ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, frameFormatWithMapName, \
            frame.index, frame.relativePc, mapName.c_str());
        if (ret <= 0) {
            HILOG_ERROR("DfxMixStackDumper::PrintNativeFrame snprintf_s failed.");
        }
        return std::string(buf);
    }

    int ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, frameFormatWithFuncName, \
        frame.index, frame.relativePc, mapName.c_str(),\
        frame.funcName.c_str(), frame.funcOffset);
    if (ret <= 0) {
        HILOG_ERROR("DfxMixStackDumper::PrintNativeFrame snprintf_s failed.");
    }
    return std::string(buf);
}

static pid_t GetPid()
{
    return g_procInfo.pid;
}

static bool HasNameSpace()
{
    return g_procInfo.ns;
}

static int GetProcStatus(struct ProcInfo* procInfo)
{
    if (procInfo->pid >= 1) {
        return 0;
    }

    procInfo->pid = getpid();
    procInfo->tid = gettid();
    procInfo->ppid = getppid();
    procInfo->ns = false;
    char buf[STATUS_LINE_SIZE];
    FILE *fp = fopen(PROC_SELF_STATUS_PATH, "r");
    if (fp == nullptr) {
        return -1;
    }
    int p = 0, pp = 0, t = 0;
    while (!feof(fp)) {
        if (fgets(buf, STATUS_LINE_SIZE, fp) == nullptr) {
            fclose(fp);
            return -1;
        }
        if (strncmp(buf, PID_STR_NAME, strlen(PID_STR_NAME)) == 0) {
            // Pid:    1892
            if (sscanf_s(buf, "%*[^0-9]%d", &p) != 1) {
                perror("sscanf_s failed.");
            }
            procInfo->pid = p;
            if (procInfo->pid == getpid()) {
                procInfo->ns = false;
                break;
            }
            procInfo->ns = true;
            continue;
        }
        if (strncmp(buf, PPID_STR_NAME, strlen(PPID_STR_NAME)) == 0) {
            // PPid:   240
            if (sscanf_s(buf, "%*[^0-9]%d", &pp) != 1) {
                perror("sscanf_s failed.");
            }
            procInfo->ppid = pp;
            continue;
        }
        // NSpid:  1892    1
        if (strncmp(buf, NSPID_STR_NAME, strlen(NSPID_STR_NAME)) == 0) {
            if (sscanf_s(buf, "%*[^0-9]%d%*[^0-9]%d", &p, &t) != NAMESPACE_MATCH_NUM) {
                perror("sscanf_s failed.");
            }
            procInfo->tid = t;
            break;
        }
    }
    (void)fclose(fp);
    return 0;
}

static void TidToNstid(const int tid, int& nstid)
{
    char path[NAME_LEN];
    (void)memset_s(path, sizeof(path), '\0', sizeof(path));
    if (snprintf_s(path, sizeof(path), sizeof(path) - 1, "/proc/%d/task/%d/status", GetPid(), tid) <= 0) {
        HILOG_WARN("snprintf_s error.");
        return;
    }

    char buf[STATUS_LINE_SIZE];
    FILE *fp = fopen(path, "r");
    if (fp == nullptr) {
        return;
    }

    int p = 0, t = 0;
    while (!feof(fp)) {
        if (fgets(buf, STATUS_LINE_SIZE, fp) == nullptr) {
            fclose(fp);
            return;
        }

        // NSpid:  1892    1
        if (strncmp(buf, NSPID_STR_NAME, strlen(NSPID_STR_NAME)) == 0) {
            if (sscanf_s(buf, "%*[^0-9]%d%*[^0-9]%d", &p, &t) != NAMESPACE_MATCH_NUM) {
                HILOG_ERROR("sscanf_s failed.");
            }
            nstid = t;
            break;
        }
    }
    (void)fclose(fp);
}

static std::string GetCurrentTimeStr(uint64_t current = 0)
{
    time_t now = time(nullptr);
    uint64_t millisecond = 0;
    const uint64_t ratio = 1000;
    if (current > static_cast<uint64_t>(now)) {
        millisecond = current % ratio;
        now = static_cast<time_t>(current / ratio);
    }

    auto tm = std::localtime(&now);
    char seconds[128] = { 0 }; // 128 : time buffer size
    if (tm == nullptr || strftime(seconds, sizeof(seconds) - 1, "%Y-%m-%d %H:%M:%S", tm) == 0) {
        return "invalid timestamp\n";
    }

    char formatTimeBuf[256] = { 0 }; // 256 : buffer size
    int ret = snprintf_s(formatTimeBuf, sizeof(formatTimeBuf), sizeof(formatTimeBuf) - 1,
        "%s.%03u\n", seconds, millisecond);
    if (ret <= 0) {
        return "invalid timestamp\n";
    }
    return std::string(formatTimeBuf, strlen(formatTimeBuf));
}

bool MixStackDumper::Dump_SignalHandler(int sig, siginfo_t *si, void *context)
{
    HILOG_INFO("Dump_SignalHandler.");
    bool ret = false;
    if (si->si_code != DUMP_TYPE_MIX) {
        return ret;
    }

    HILOG_INFO("Received mix stack dump request.");
    if (signalHandler_.expired()) {
        HILOG_WARN("signalHandler is expired.");
        return ret;
    }
    auto handler = signalHandler_.lock();
    if (handler == nullptr) {
        return ret;
    }
    g_targetDumpTid = si->si_value.sival_int;
    handler->PostTask(&MixStackDumper::HandleMixDumpRequest);
    return ret;
}

bool MixStackDumper::IsInstalled()
{
    return hasInstalled;
}

void MixStackDumper::InstallDumpHandler(std::shared_ptr<OHOSApplication> application,
    std::shared_ptr<EventHandler> handler)
{
    if (!hasInstalled) {
        signalHandler_ = handler;
        application_ = application;
        struct signal_chain_action sigchain = {
            .sca_sigaction = MixStackDumper::Dump_SignalHandler,
            .sca_mask = {},
            .sca_flags = 0,
        };
        add_special_signal_handler(SIGDUMP, &sigchain);
        hasInstalled = true;
    }
}

bool MixStackDumper::IsJsNativePcEqual(uintptr_t *jsNativePointer, uint64_t nativePc, uint64_t nativeOffset)
{
    uint64_t jsPc_ = (uint64_t)jsNativePointer;
    if (nativePc - nativeOffset == jsPc_) {
        return true;
    }
    return false;
}

void MixStackDumper::BuildJsNativeMixStack(int fd, std::vector<JsFrames>& jsFrames,
    std::vector<NativeFrame>& nativeFrames)
{
    uint32_t jsIdx = 0;
    uint32_t nativeIdx = 0;
    std::string mixStackStr = "";
    bool matchJsFrame = false;
    while (jsIdx < jsFrames.size() && jsFrames[jsIdx].nativePointer == nullptr) {
        jsIdx++;
    }

    while (jsIdx < jsFrames.size() && nativeIdx < nativeFrames.size()) {
        if (jsFrames[jsIdx].nativePointer == nullptr) {
            mixStackStr += PrintJsFrame(jsFrames[jsIdx]);
            jsIdx++;
            continue;
        }

        if (IsJsNativePcEqual(jsFrames[jsIdx].nativePointer, nativeFrames[nativeIdx].pc,
            nativeFrames[nativeIdx].funcOffset)) {
            HILOG_DEBUG("DfxMixStackDumper::BuildJsNativeMixStack pc register values matched.");
            mixStackStr += PrintNativeFrame(nativeFrames[nativeIdx]);
            mixStackStr += PrintJsFrame(jsFrames[jsIdx]);
            nativeIdx++;
            jsIdx++;
            matchJsFrame = true;
        } else {
            mixStackStr += PrintNativeFrame(nativeFrames[nativeIdx]);
            nativeIdx++;
        }
    }

    std::string jsStack;
    if (!matchJsFrame && jsFrames.size() > 0) {
        jsIdx = 0;
        while (jsIdx < jsFrames.size()) {
            jsStack += PrintJsFrame(jsFrames[jsIdx]);
            jsIdx++;
        }
    }

    while (nativeIdx < nativeFrames.size()) {
        mixStackStr += PrintNativeFrame(nativeFrames[nativeIdx]);
        nativeIdx++;
    }

    jsStack += mixStackStr;
    Write(fd, jsStack);
    Write(fd, "\n");
}

std::string MixStackDumper::GetThreadStackTraceLabel(pid_t tid)
{
    std::ostringstream result;
    result << "Tid:" << tid;
    std::string path = "/proc/self/task/" + std::to_string(tid) + "/comm";
    std::string threadComm;
    if (LoadStringFromFile(path, threadComm)) {
        result << " Name:" << threadComm;
    } else {
        result << std::endl;
    }
    return result.str();
}

void MixStackDumper::PrintNativeFrames(int fd, std::vector<NativeFrame>& nativeFrames)
{
    for (const auto& frame : nativeFrames) {
        std::string nativeFrameStr = PrintNativeFrame(frame);
        Write(fd, nativeFrameStr);
    }
    Write(fd, "\n");
}

void MixStackDumper::PrintProcessHeader(int fd, pid_t pid, uid_t uid)
{
    char headerBuf[HEADER_BUF_LEN] = { 0 };
    std::string processName = "";
    int ret = 1;
    if (LoadStringFromFile(PROC_SELF_CMDLINE_PATH, processName)) {
        ret = snprintf_s(headerBuf, HEADER_BUF_LEN, HEADER_BUF_LEN - 1,
                         "Timestamp:%sPid:%d\nUid:%d\nProcess name:%s\n",
                         GetCurrentTimeStr().c_str(), pid, uid, processName.c_str());
    } else {
        ret = snprintf_s(headerBuf, HEADER_BUF_LEN, HEADER_BUF_LEN - 1,
                         "Timestamp:%sPid:%d\nUid:%d\nProcess name:unknown\n",
                         GetCurrentTimeStr().c_str(), pid, uid);
    }
    if (ret <= 0) {
        HILOG_ERROR("snprintf_s process mix stack header failed.");
        return;
    }
    Write(fd, std::string(headerBuf));
}

bool MixStackDumper::DumpMixFrame(int fd, pid_t nstid, pid_t tid)
{
    if (catcher_ == nullptr) {
        HILOG_ERROR("No FrameCatcher? call init first.");
        return false;
    }

    std::vector<NativeFrame> nativeFrames;
    bool hasNativeFrame = true;
    if (!catcher_->CatchFrame(nstid, nativeFrames)) {
        hasNativeFrame = false;
    }

    bool hasJsFrame = true;
    std::vector<JsFrames> jsFrames;
    auto application = application_.lock();
    // if we failed to get native frame, target thread may not be seized
    if (application != nullptr && application->GetRuntime() != nullptr && hasNativeFrame) {
        hasJsFrame = application->GetRuntime()->BuildJsStackInfoList(nstid, jsFrames);
    }
    catcher_->ReleaseThread(nstid);

    if (jsFrames.size() == 0) {
        hasJsFrame = false;
    }

    if (!hasNativeFrame && !hasJsFrame) {
        std::string result = "Failed to frames for " + std::to_string(nstid) + ".\n";
        HILOG_ERROR("%{public}s", result.c_str());
        return false;
    }

    std::string threadComm = GetThreadStackTraceLabel(tid);
    Write(fd, threadComm);
    if (hasNativeFrame && !hasJsFrame) {
        PrintNativeFrames(fd, nativeFrames);
        return true;
    }

    BuildJsNativeMixStack(fd, jsFrames, nativeFrames);
    return true;
}

void MixStackDumper::GetThreadList(std::vector<pid_t>& threadList)
{
    char realPath[PATH_MAX] = {'\0'};
    if (realpath("/proc/self/task", realPath) == nullptr) {
        HILOG_ERROR("DfxMixStackDumper::GetThreadList return false as realpath failed.");
        return;
    }
    DIR *dir = opendir(realPath);
    if (dir == nullptr) {
        HILOG_ERROR("DfxMixStackDumper::GetThreadList return false as opendir failed.");
        return;
    }
    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        pid_t tid = atoi(ent->d_name);
        if (tid == 0) {
            continue;
        }
        threadList.emplace_back(tid);
    }
    if (closedir(dir) == -1) {
        HILOG_ERROR("MixStackDumper::GetThreadList closedir failed.");
    }
}

void MixStackDumper::Init(pid_t pid)
{
    catcher_ = std::make_unique<OHOS::HiviewDFX::DfxDumpCatcher>(pid);
    if (!catcher_->InitFrameCatcher()) {
        HILOG_ERROR("Init DumpCatcher Failed.");
    }
}

void MixStackDumper::Destroy()
{
    if (catcher_ != nullptr) {
        catcher_->DestroyFrameCatcher();
        catcher_ = nullptr;
    }
}

void MixStackDumper::HandleMixDumpRequest()
{
    int fd = -1;
    int resFd = -1;
    int dumpRes = OHOS::HiviewDFX::ProcessDumpRes::DUMP_ESUCCESS;
    std::unique_lock<std::mutex> lock(g_mutex);
    HILOG_INFO("Current process is ready to dump stack trace.");
    (void)GetProcStatus(&g_procInfo);
    do {
        fd = RequestPipeFd(GetPid(), FaultLoggerPipeType::PIPE_FD_WRITE_BUF);
        resFd = RequestPipeFd(GetPid(), FaultLoggerPipeType::PIPE_FD_WRITE_RES);
        if (fd < 0 || resFd < 0) {
            HILOG_ERROR("request pid(%{public}d) pipe fd failed", GetPid());
            dumpRes = OHOS::HiviewDFX::ProcessDumpRes::DUMP_EGETFD;
            break;
        }
        MixStackDumper mixDumper;
        mixDumper.DumpMixStackLocked(fd, g_targetDumpTid);
        g_targetDumpTid = -1;
    } while (false);

    OHOS::HiviewDFX::DumpResMsg dumpResMsg;
    dumpResMsg.res = dumpRes;
    const char* strRes = OHOS::HiviewDFX::DfxDumpRes::GetInstance().GetResStr(dumpRes);
    if (strncpy_s(dumpResMsg.strRes, sizeof(dumpResMsg.strRes), strRes, sizeof(dumpResMsg.strRes) - 1) != 0) {
        HILOG_ERROR("DfxMixStackDumper::HandleProcessMixDumpRequest strncpy_s failed.");
    }
    if (resFd >= 0) {
        write(resFd, &dumpResMsg, sizeof(struct OHOS::HiviewDFX::DumpResMsg));
        close(resFd);
    }
    if (fd >= 0) {
        close(fd);
    }
    HILOG_INFO("Finish dumping stack trace.");
}

void MixStackDumper::Write(int fd, const std::string& outStr)
{
    if (fd < 0) {
        outputStr_.append(outStr);
    } else {
        write(fd, outStr.c_str(), outStr.size());
    }
}

std::string MixStackDumper::DumpMixStackLocked(int fd, pid_t requestTid)
{
    if (fd < 0) {
        outputStr_.clear();
    }

    Init(GetPid());
    PrintProcessHeader(fd, GetPid(), getuid());
    if (requestTid > 0) {
        pid_t targetNsTid = requestTid;
        pid_t targetTid = requestTid;
        if (HasNameSpace()) {
            TidToNstid(targetTid, targetNsTid);
        }
        DumpMixFrame(fd, targetNsTid, targetTid);
    } else {
        std::vector<pid_t> threads;
        GetThreadList(threads);
        for (auto& tid : threads) {
            pid_t nstid = tid;
            if (HasNameSpace()) {
                TidToNstid(tid, nstid);
            }
            if (nstid == gettid()) {
                continue;
            }
            DumpMixFrame(fd, nstid, tid);
        }
    }
    Destroy();
    return outputStr_;
}

std::string MixStackDumper::GetMixStack(bool onlyMainThread)
{
    std::unique_lock<std::mutex> lock(g_mutex);
    (void)GetProcStatus(&g_procInfo);
    MixStackDumper mixDumper;
    return mixDumper.DumpMixStackLocked(-1, onlyMainThread ? GetPid() : -1);
}
} // AppExecFwk
} // OHOS
