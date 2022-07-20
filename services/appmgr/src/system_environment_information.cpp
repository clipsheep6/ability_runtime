/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "system_environment_information.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <string>

#include "hilog_wrapper.h"
#include "kernel_system_memory_info.h"
#include "securec.h"

namespace OHOS {
namespace AppExecFwk {
namespace SystemEnv {
namespace {
static const int BYTES_KB = 1024;
}
void KernelSystemMemoryInfo::Init(std::map<std::string, std::string> &memInfo)
{
    auto findData = [&] (const std::string& key) -> std::string {
        auto iter = memInfo.find(key);
        if (iter != memInfo.end()) {
            HILOG_DEBUG("key[%{public}s] data[%{public}s]", key.c_str(), iter->second.c_str());
            return iter->second;
        } else {
            HILOG_ERROR("key[%{public}s]", key.c_str());
            return std::string("");
        }
    };

    memTotal_ = ParseMemInfo(findData(std::string("MemTotal")).c_str());
    memFree_ = ParseMemInfo(findData(std::string("MemFree")).c_str());
    memAvailable_ = ParseMemInfo(findData(std::string("MemAvailable")).c_str());
    buffers_ = ParseMemInfo(findData(std::string("Buffers")).c_str());
    cached_ = ParseMemInfo(findData(std::string("Cached")).c_str());
    swapCached_ = ParseMemInfo(findData(std::string("SwapCached")).c_str());
}

int64_t KernelSystemMemoryInfo::ParseMemInfo(const char* data) const
{
    int32_t base = 10; // 10 means decimal
    char* end = nullptr;
    errno = 0;
    auto result = strtoll(data, &end, base);
    if (end == data || errno == ERANGE || result > LLONG_MAX || result < LLONG_MIN) {
        HILOG_WARN("input str is: %{public}s!", data);
        return 0;
    }
    return result * BYTES_KB;
}

int64_t KernelSystemMemoryInfo::GetMemTotal() const
{
    return memTotal_;
}

int64_t KernelSystemMemoryInfo::GetMemFree() const
{
    return memFree_;
}

int64_t KernelSystemMemoryInfo::GetMemAvailable() const
{
    return memAvailable_;
}

int64_t KernelSystemMemoryInfo::GetBuffers() const
{
    return buffers_;
}

int64_t KernelSystemMemoryInfo::GetCached() const
{
    return cached_;
}

int64_t KernelSystemMemoryInfo::GetSwapCached() const
{
    return swapCached_;
}

static void RequestSystemMemoryInfo(std::map<std::string, std::string> &memInfo)
{
    std::regex rLabel("[\\w()]+");
    std::regex rData("\\d+");
    const int buffsize = 1024;
    char buff[buffsize] = {0};

    FILE *fp = popen("cat /proc/meminfo", "r");
    if (fp == nullptr) {
        HILOG_ERROR("open meminfo failed");
        return;
    }

    while (fgets(buff, sizeof(buff), fp) != nullptr) {
        std::string strbuf(buff);
        (void)memset_s(buff, sizeof(buff), 0x00, sizeof(buff));
        std::smatch sm;
        std::smatch smData;
        bool flag = false;
        flag = std::regex_search(strbuf, sm, rLabel);
        if (!flag) {
            HILOG_ERROR("open meminfo failed");
            continue;
        }
        std::string strLabel = sm[0];
        strbuf = sm.suffix().str();
        flag = std::regex_search(strbuf, sm, rData);
        if (!flag) {
            HILOG_ERROR("open meminfo failed");
            continue;
        }
        std::string strData = sm[0];
        memInfo[strLabel] = strData;
    }

    pclose(fp);
    fp = nullptr;
}

void GetMemInfo(KernelSystemMemoryInfo &memInfo)
{
    std::map<std::string, std::string> memListInfo;
    RequestSystemMemoryInfo(memListInfo);
    memInfo.Init(memListInfo);
}
}  // namespace SystemEnv
}  // namespace AppExecFwk
}  // namespace OHOS
