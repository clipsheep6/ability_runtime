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

#include "mix_stack_dumper.h"

#include <ctime>
#include <mutex>

#include <dirent.h>
#include <securec.h>
#include <unistd.h>
#include <sigchain.h>

#include "dfx_dump_res.h"
#include "dfx_frame_format.h"
#include "dfx_util.h"
#include "faultloggerd_client.h"
#include "file_ex.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "procinfo.h"

using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace AppExecFwk {
std::weak_ptr<EventHandler> MixStackDumper::signalHandler_;
std::weak_ptr<OHOSApplication> MixStackDumper::application_;
static std::mutex g_mutex;

namespace {
static const std::string PROC_SELF_CMDLINE_PATH = "/proc/self/cmdline";
static constexpr int SUSPEND_VM_FRAME_SKIP_NUM = 2;
static constexpr int HEADER_BUF_LEN = 512;
static bool hasInstalled = false;
static pid_t g_targetDumpTid = -1;
static struct ProcInfo g_procInfo = {0};

static std::string PrintJsFrame(const JsFrames& jsFrame)
{
    return "  at " + jsFrame.functionName + " (" + jsFrame.fileName + ":" + jsFrame.pos + ")\n";
}

static pid_t GetPid()
{
    return g_procInfo.pid;
}

static bool HasNameSpace()
{
    return g_procInfo.ns;
}
}

bool MixStackDumper::Dump_SignalHandler(int sig, siginfo_t *si, void *context)
{
    TAG_LOGI(AAFwkTag::BASE, "Dump_SignalHandler.");
    bool ret = false;
    if (si->si_code != DUMP_TYPE_MIX) {
        return ret;
    }

    TAG_LOGI(AAFwkTag::BASE, "Received mix stack dump request.");
    if (signalHandler_.expired()) {
        TAG_LOGW(AAFwkTag::BASE, "signalHandler is expired.");
        return ret;
    }
    auto handler = signalHandler_.lock();
    if (handler == nullptr) {
        return ret;
    }
    g_targetDumpTid = si->si_value.sival_int;
    handler->PostTask(&MixStackDumper::HandleMixDumpRequest, "MixStackDumper::MixDump");
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
    std::vector<DfxFrame>& nativeFrames)
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
            TAG_LOGD(AAFwkTag::BASE, "DfxMixStackDumper::BuildJsNativeMixStack pc register values matched.");
            mixStackStr += HiviewDFX::DfxFrameFormat::GetFrameStr(nativeFrames[nativeIdx]);
            mixStackStr += PrintJsFrame(jsFrames[jsIdx]);
            nativeIdx++;
            jsIdx++;
            matchJsFrame = true;
        } else {
            mixStackStr += HiviewDFX::DfxFrameFormat::GetFrameStr(nativeFrames[nativeIdx]);
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
        mixStackStr += HiviewDFX::DfxFrameFormat::GetFrameStr(nativeFrames[nativeIdx]);
        nativeIdx++;
    }

    jsStack += mixStackStr;
    Write(fd, jsStack);
    Write(fd, "\n");
}

std::string MixStackDumper::GetThreadStackTraceLabel(pid_t tid)
{
    std::ostringstream ss;
    std::string threadName = "";
    ReadThreadName(tid, threadName);
    ss << "Tid:" << tid << ", Name:" << threadName << std::endl;
    return ss.str();
}

void MixStackDumper::PrintProcessHeader(int fd, pid_t pid, uid_t uid)
{
    char headerBuf[HEADER_BUF_LEN] = { 0 };
    std::string processName = "";
    int ret = 1;
    ReadProcessName(getpid(), processName);
    if (!processName.empty()) {
        ret = snprintf_s(headerBuf, HEADER_BUF_LEN, HEADER_BUF_LEN - 1,
                         "Timestamp:%sPid:%d\nUid:%d\nProcess name:%s\n",
                         GetCurrentTimeStr().c_str(), pid, uid, processName.c_str());
    } else {
        ret = snprintf_s(headerBuf, HEADER_BUF_LEN, HEADER_BUF_LEN - 1,
                         "Timestamp:%sPid:%d\nUid:%d\nProcess name:unknown\n",
                         GetCurrentTimeStr().c_str(), pid, uid);
    }
    if (ret <= 0) {
        TAG_LOGE(AAFwkTag::BASE, "snprintf_s process mix stack header failed.");
        return;
    }
    Write(fd, std::string(headerBuf));
}

bool MixStackDumper::DumpMixFrame(int fd, pid_t nstid, pid_t tid)
{
    if (catcher_ == nullptr) {
        TAG_LOGE(AAFwkTag::BASE, "No FrameCatcher? call init first.");
        return false;
    }

    std::vector<DfxFrame> nativeFrames;
    bool hasNativeFrame = true;
    bool isVmSuspended = false;
    auto application = application_.lock();
    if (application != nullptr && application->GetRuntime() != nullptr) {
        isVmSuspended = application->GetRuntime()->SuspendVM(nstid);
    }
    int skipframes = isVmSuspended ? SUSPEND_VM_FRAME_SKIP_NUM : 0;
    if (!catcher_->CatchFrame(nstid, nativeFrames, skipframes)) {
        hasNativeFrame = false;
    }

    bool hasJsFrame = true;
    std::vector<JsFrames> jsFrames;
    // if we failed to get native frame, target thread may not be seized
    if (application != nullptr && application->GetRuntime() != nullptr && isVmSuspended) {
        hasJsFrame = application->GetRuntime()->BuildJsStackInfoList(nstid, jsFrames);
    }
    catcher_->ReleaseThread(nstid);
    if (application != nullptr && application->GetRuntime() != nullptr && isVmSuspended) {
        application->GetRuntime()->ResumeVM(nstid);
    }

    if (jsFrames.size() == 0) {
        hasJsFrame = false;
    }

    Write(fd, GetThreadStackTraceLabel(tid));
    if (!hasNativeFrame && !hasJsFrame) {
        TAG_LOGE(AAFwkTag::BASE, "Failed to unwind frames for %{public}d.", nstid);
        std::string wchan;
        ReadThreadWchan(wchan, tid, true);
        Write(fd, wchan);
        Write(fd, "\n");
        return false;
    }

    if (hasNativeFrame && !hasJsFrame) {
        Write(fd, HiviewDFX::DfxFrameFormat::GetFramesStr(nativeFrames));
        Write(fd, "\n");
        return true;
    }
    BuildJsNativeMixStack(fd, jsFrames, nativeFrames);
    return true;
}

void MixStackDumper::Init(pid_t pid)
{
    catcher_ = std::make_unique<OHOS::HiviewDFX::DfxCatchFrameLocal>(pid);
    if (!catcher_->InitFrameCatcher()) {
        TAG_LOGE(AAFwkTag::BASE, "Init DumpCatcher Failed.");
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
    int dumpRes = OHOS::HiviewDFX::DumpErrorCode::DUMP_ESUCCESS;
    std::unique_lock<std::mutex> lock(g_mutex);
    TAG_LOGI(AAFwkTag::BASE, "Current process is ready to dump stack trace.");
    (void)GetProcStatus(g_procInfo);
    do {
        fd = RequestPipeFd(GetPid(), FaultLoggerPipeType::PIPE_FD_WRITE_BUF);
        resFd = RequestPipeFd(GetPid(), FaultLoggerPipeType::PIPE_FD_WRITE_RES);
        if (fd < 0 || resFd < 0) {
            TAG_LOGE(AAFwkTag::BASE, "request pid(%{public}d) pipe fd failed", GetPid());
            dumpRes = OHOS::HiviewDFX::DumpErrorCode::DUMP_EGETFD;
            break;
        }
        MixStackDumper mixDumper;
        mixDumper.DumpMixStackLocked(fd, g_targetDumpTid);
        g_targetDumpTid = -1;
    } while (false);

    if (resFd >= 0) {
        write(resFd, &dumpRes, sizeof(dumpRes));
        close(resFd);
    }
    if (fd >= 0) {
        close(fd);
    }
    TAG_LOGI(AAFwkTag::BASE, "Finish dumping stack trace.");
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
            TidToNstid(GetPid(), targetTid, targetNsTid);
        }
        DumpMixFrame(fd, targetNsTid, targetTid);
    } else {
        std::vector<pid_t> tids;
        std::function<bool(int)> func = [&](int tid) {
            pid_t nstid = tid;
            if (HasNameSpace()) {
                TidToNstid(GetPid(), tid, nstid);
            }
            if (nstid != gettid()) {
                DumpMixFrame(fd, nstid, tid);
            }
            return true;
        };
        GetTidsByPidWithFunc(GetPid(), tids, func);
    }
    Destroy();
    return outputStr_;
}

std::string MixStackDumper::GetMixStack(bool onlyMainThread)
{
    std::unique_lock<std::mutex> lock(g_mutex);
    (void)GetProcStatus(g_procInfo);
    MixStackDumper mixDumper;
    return mixDumper.DumpMixStackLocked(-1, onlyMainThread ? GetPid() : -1);
}
} // namespace AppExecFwk
} // namespace OHOS
