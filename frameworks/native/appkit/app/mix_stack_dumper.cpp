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

#include "mix_stack_dumper.h"

#include <dirent.h>
#include <securec.h>
#include <unistd.h>

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
namespace {
constexpr int SIGDUMP = 35;
constexpr int FRAME_BUF_LEN = 1024;
constexpr int NATIVE_DUMP = -1;
constexpr int MIX_DUMP = -2;
}

typedef void (*DumpSignalHandlerFunc) (int sig, siginfo_t *si, void *context);
static DumpSignalHandlerFunc g_dumpSignalHandlerFunc = nullptr;
static pid_t g_targetDumpTid = -1;

static std::string PrintJsFrame(JsFrames& jsFrame)
{
    return "  at " + jsFrame.functionName + " (" + jsFrame.fileName + ":" + jsFrame.pos + ")\n";
}

static std::string PrintNativeFrame(std::shared_ptr<OHOS::HiviewDFX::DfxFrame> frame)
{
    if (frame == nullptr) {
        return "";
    }
    char buf[FRAME_BUF_LEN] = {0};
    std::string mapName = frame->GetFrameMapName();
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

    if (frame->GetFrameFuncName().empty()) {
        int ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, frameFormatWithMapName, \
            frame->GetFrameIndex(), frame->GetFrameRelativePc(), mapName.c_str());
        if (ret <= 0) {
            HILOG_ERROR("DfxMixStackDumper::PrintNativeFrame snprintf_s failed.");
        }
        return std::string(buf);
    }

    int ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, frameFormatWithFuncName, \
        frame->GetFrameIndex(), frame->GetFrameRelativePc(), mapName.c_str(),\
        frame->GetFrameFuncName().c_str(), frame->GetFrameFuncOffset());
    if (ret <= 0) {
        HILOG_ERROR("DfxMixStackDumper::PrintNativeFrame snprintf_s failed.");
    }
    return std::string(buf);
}

void MixStackDumper::Dump_SignalHandler(int sig, siginfo_t *si, void *context)
{
    switch (si->si_code) {
        case NATIVE_DUMP: {
            if (g_dumpSignalHandlerFunc != nullptr) {
                g_dumpSignalHandlerFunc(sig, si, context);
            }
            break;
        }
        case MIX_DUMP: {
            auto handler = signalHandler_.lock();
            if (handler == nullptr) {
                return;
            }
            g_targetDumpTid = si->si_value.sival_int;
            handler->PostTask(&MixStackDumper::HandleMixDumpRequest);
            break;
        }
        default:
            break;
    }
}

void MixStackDumper::InstallDumpHandler(std::shared_ptr<OHOSApplication> application,
    std::shared_ptr<EventHandler> handler)
{
    MixStackDumper::signalHandler_ = handler;
    MixStackDumper::application_ = application;
    struct sigaction newDumpAction;
    struct sigaction oldDumpAction;
    (void)memset_s(&newDumpAction, sizeof(newDumpAction), 0, sizeof(newDumpAction));
    (void)memset_s(&oldDumpAction, sizeof(oldDumpAction), 0, sizeof(oldDumpAction));
    sigfillset(&newDumpAction.sa_mask);
    newDumpAction.sa_sigaction = Dump_SignalHandler;
    newDumpAction.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
    sigaction(SIGDUMP, &newDumpAction, &oldDumpAction);
    if (oldDumpAction.sa_sigaction != nullptr) {
        g_dumpSignalHandlerFunc = oldDumpAction.sa_sigaction;
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
    std::vector<std::shared_ptr<OHOS::HiviewDFX::DfxFrame>>& nativeFrames)
{
    uint32_t jsIdx = 0;
    uint32_t nativeIdx = 0;
    std::string mixStackStr = "";
    while (jsIdx < jsFrames.size() && jsFrames[jsIdx].nativePointer == nullptr) {
        jsIdx++;
    }
    while (jsIdx < jsFrames.size() && nativeIdx < nativeFrames.size()) {
        if (jsFrames[jsIdx].nativePointer == nullptr) {
            mixStackStr += PrintJsFrame(jsFrames[jsIdx]);
            jsIdx++;
            continue;
        }
        if (IsJsNativePcEqual(jsFrames[jsIdx].nativePointer, nativeFrames[nativeIdx]->GetFramePc(),
            nativeFrames[nativeIdx]->GetFrameFuncOffset())) {
            HILOG_DEBUG("DfxMixStackDumper::BuildJsNativeMixStack pc register values matched.");
            mixStackStr += PrintNativeFrame(nativeFrames[nativeIdx]);
            mixStackStr += PrintJsFrame(jsFrames[jsIdx]);
            nativeIdx++;
            jsIdx++;
        } else {
            mixStackStr += PrintNativeFrame(nativeFrames[nativeIdx]);
            nativeIdx++;
        }
    }
    while (nativeIdx < nativeFrames.size()) {
        mixStackStr +=  PrintNativeFrame(nativeFrames[nativeIdx]);
        nativeIdx++;
    }
    write(fd, mixStackStr.c_str(), mixStackStr.size());
}

std::string MixStackDumper::GetThreadStackTraceLabel(pid_t tid)
{
    std::ostringstream result;
    result << "Tid:" << tid;
    std::string path = "/proc/self/task/" + std::to_string(tid) + "/comm";
    std::string threadComm;
    if (LoadStringFromFile(path, threadComm)) {
        result << " comm:" << threadComm;
    } else {
        result << std::endl;
    }
    return result.str();
}

void MixStackDumper::PrintNativeFrames(int fd, std::vector<std::shared_ptr<OHOS::HiviewDFX::DfxFrame>>& nativeFrames)
{
    for (auto& frame : nativeFrames) {
        std::string nativeFrameStr = PrintNativeFrame(frame);
        write(fd, nativeFrameStr.c_str(), nativeFrameStr.size());
    }
}

bool MixStackDumper::DumpMixFrame(int fd, pid_t tid)
{
    if (catcher_ == nullptr) {
        HILOG_ERROR("No FrameCatcher? call init first.");
        return false;
    }

    std::string threadComm = GetThreadStackTraceLabel(tid);
    write(fd, threadComm.c_str(), threadComm.size());
    if (!catcher_->RequestCatchFrame(tid)) {
        std::string result = "Failed to suspend thread(" + std::to_string(tid) + ").\n";
        HILOG_ERROR("%{public}s", result.c_str());
        write(fd, result.c_str(), result.size());
        return false;
    }

    bool onlyDumpNative = false;
    std::vector<JsFrames> jsFrames;
    auto application = application_.lock();
    if (application != nullptr && application->GetRuntime() != nullptr) {
        bool ret = application->GetRuntime()->BuildJsStackInfoList(tid, jsFrames);
        if (!ret || jsFrames.size() == 0) {
            onlyDumpNative = true;
        }
    }

    std::vector<std::shared_ptr<OHOS::HiviewDFX::DfxFrame>> nativeFrames;
    if (catcher_->CatchFrame(tid, nativeFrames) == false) {
        HILOG_ERROR("DfxMixStackDumper::DumpMixFrame Capture thread(%{public}d) native frames failed.", tid);
    }

    if (onlyDumpNative) {
        PrintNativeFrames(fd, nativeFrames);
        return true;
    }

    BuildJsNativeMixStack(fd, jsFrames, nativeFrames);
    write(fd, "\n", 1);
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
    catcher_->InitFrameCatcher();
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
    do {
        fd = RequestPipeFd(getpid(), FaultLoggerPipeType::PIPE_FD_WRITE_BUF);
        resFd = RequestPipeFd(getpid(), FaultLoggerPipeType::PIPE_FD_WRITE_RES);
        if (fd < 0 || resFd < 0) {
            HILOG_ERROR("DfxMixStackDumper::HandleProcessMixDumpRequest request pipe fd failed");
            dumpRes = OHOS::HiviewDFX::ProcessDumpRes::DUMP_EGETFD;
            break;
        }
        MixStackDumper mixDumper;
        mixDumper.Init(getpid());
        if (g_targetDumpTid > 0) {
            mixDumper.DumpMixFrame(fd, g_targetDumpTid);
            g_targetDumpTid = -1;
            mixDumper.Destroy();
            break;
        }
        std::vector<pid_t> threads;
        mixDumper.GetThreadList(threads);
        for (auto& tid : threads) {
            if (tid == gettid()) {
                continue;
            }
            mixDumper.DumpMixFrame(fd, tid);
        }
        mixDumper.Destroy();
    } while (false);

    OHOS::HiviewDFX::DumpResMsg dumpResMsg;
    dumpResMsg.res = dumpRes;
    const char* strRes = OHOS::HiviewDFX::DfxDumpRes::GetInstance().GetResStr(dumpRes);
    if (strncpy_s(dumpResMsg.strRes, sizeof(dumpResMsg.strRes), strRes, sizeof(dumpResMsg.strRes) - 1) != 0) {
        HILOG_ERROR("DfxMixStackDumper::HandleProcessMixDumpRequest strncpy_s failed.");
    }
    if (resFd != -1) {
        write(resFd, &dumpResMsg, sizeof(struct OHOS::HiviewDFX::DumpResMsg));
        close(resFd);
    }
    if (fd != -1) {
        close(fd);
    }
}
} // AppExecFwk
} // OHOS
