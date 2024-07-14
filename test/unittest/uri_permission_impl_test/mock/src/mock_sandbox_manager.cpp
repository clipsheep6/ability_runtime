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

#include "mock_sandbox_manager.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AccessControl {
namespace SandboxManager {
namespace {
constexpr uint32_t MOCK_READ_MODE = 1;
constexpr uint32_t MOCK_WRITE_MODE = 2;
constexpr uint32_t MOCK_READ_WRITE_MODE = 3;
constexpr uint32_t MOCK_FLAG_PERSIST_URI = 64;
}

int32_t SandboxManagerKit::SetPolicy(uint32_t tokenid, const std::vector<PolicyInfo> &policys, uint64_t policyFlag,
    std::vector<uint32_t> &results)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "SetPolicy called, size of policys is %{public}zu", policys.size());
    if (SetPolicyRet_ != ERR_OK) {
        return SetPolicyRet_;
    }
    results.clear();
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto &policyInfo: policys) {
        auto key = std::to_string(tokenid) + ":" + policyInfo.path;
        auto keySearchIter = policyMap_.find(key);
        uint32_t mode = policyInfo.mode;
        if (policyFlag > 0) {
            mode |= MOCK_FLAG_PERSIST_URI;
        }
        if (keySearchIter == policyMap_.end()) {
            policyMap_.emplace(key, mode);
        } else {
            keySearchIter->second |= mode;
        }
    }
    results = std::vector<uint32_t>(policys.size(), ERR_OK);
    return ERR_OK;
}

int32_t SandboxManagerKit::CheckPolicy(uint32_t tokenid, const std::vector<PolicyInfo> &policys,
    std::vector<bool> &results)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckPolicy called.");
    std::lock_guard<std::mutex> guard(mutex_);
    results.clear();
    for (auto &policyInfo: policys) {
        auto key = std::to_string(tokenid) + ":" + policyInfo.path;
        TAG_LOGI(AAFwkTag::URIPERMMGR, "key is %{public}s.", key.c_str());
        auto keySearchIter = policyMap_.find(key);
        if (keySearchIter == policyMap_.end()) {
            TAG_LOGI(AAFwkTag::URIPERMMGR, "no policy record.");
            results.emplace_back(false);
            continue;
        }
        auto itemMode = (keySearchIter->second & (~MOCK_FLAG_PERSIST_URI));
        // rw
        if (itemMode == MOCK_READ_WRITE_MODE) {
            results.emplace_back(true);
            continue;
        }
        // w
        if (itemMode == MOCK_WRITE_MODE) {
            results.emplace_back(policyInfo.mode == MOCK_WRITE_MODE);
            continue;
        }
        // r
        if (itemMode == MOCK_READ_MODE) {
            results.emplace_back(policyInfo.mode == MOCK_READ_MODE);
            continue;
        }
        results.emplace_back(false);
    }
    return ERR_OK;
}

int32_t SandboxManagerKit::CheckPersistPolicy(uint32_t tokenid, const std::vector<PolicyInfo> &policys,
    std::vector<bool> &results)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckPersistPolicy called.");
    std::lock_guard<std::mutex> guard(persistMutex_);
    results.clear();
    for (auto &policyInfo: policys) {
        auto key = std::to_string(tokenid) + ":" + policyInfo.path;
        auto keySearchIter = persistPolicyMap_.find(key);
        if (keySearchIter == persistPolicyMap_.end()) {
            results.emplace_back(false);
            continue;
        }
        // rw
        if (keySearchIter->second == MOCK_READ_WRITE_MODE) {
            results.emplace_back(true);
            continue;
        }
        // w
        if (keySearchIter->second == MOCK_WRITE_MODE) {
            results.emplace_back(policyInfo.mode == MOCK_WRITE_MODE);
            continue;
        }
        // r
        if (keySearchIter->second == MOCK_READ_MODE) {
            results.emplace_back(policyInfo.mode == MOCK_READ_MODE);
            continue;
        }
        results.emplace_back(false);
    }
    return ERR_OK;
}

int32_t SandboxManagerKit::PersistPolicy(uint32_t tokenid, const std::vector<PolicyInfo> &policys,
    std::vector<uint32_t> &results)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "PersistPolicy called, size of policys is %{public}zu", policys.size());
    results.clear();
    std::lock_guard<std::mutex> guard(persistMutex_);
    for (auto &policyInfo: policys) {
        auto key = std::to_string(tokenid) + ":" + policyInfo.path;
        auto keySearchIter = persistPolicyMap_.find(key);
        if (keySearchIter == persistPolicyMap_.end()) {
            persistPolicyMap_.emplace(key, policyInfo.mode);
        } else {
            keySearchIter->second |= policyInfo.mode;
        }
    }
    results = std::vector<uint32_t>(policys.size(), ERR_OK);
    return ERR_OK;
}

int32_t SandboxManagerKit::UnSetPolicy(uint32_t tokenid, const PolicyInfo &policy)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "UnSetPolicy called.");
    if (UnSetPolicyRet_ != ERR_OK) {
        return UnSetPolicyRet_;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    auto key = std::to_string(tokenid) + ":" + policy.path;
    auto keySearchIter = policyMap_.find(key);
    if (keySearchIter == policyMap_.end()) {
        return ERR_OK;
    }
    policyMap_.erase(keySearchIter);
    return ERR_OK;
}

int32_t SandboxManagerKit::StartAccessingByTokenId(uint32_t tokenid)
{
    return ERR_OK;
}

void SandboxManagerKit::Init()
{
    {
        std::lock_guard<std::mutex> guard(mutex_);
        policyMap_.clear();
    }
    {
        std::lock_guard<std::mutex> guard(persistMutex_);
        persistPolicyMap_.clear();
    }
}

std::mutex SandboxManagerKit::mutex_;
std::map<std::string, int32_t> SandboxManagerKit::policyMap_;
std::mutex SandboxManagerKit::persistMutex_;
std::map<std::string, int32_t> SandboxManagerKit::persistPolicyMap_;
int32_t SandboxManagerKit::SetPolicyRet_ = ERR_OK;
int32_t SandboxManagerKit::UnSetPolicyRet_ = ERR_OK;
} // OHOS
} // AccessControl
} // SandboxManager