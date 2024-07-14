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
#ifndef SANDBOXMANAGER_KIT_H
#define SANDBOXMANAGER_KIT_H

#include <sys/types.h>
#include <map>
#include <mutex>
#include <unordered_set>
#include <vector>

#include "ability_manager_errors.h"
#include "policy_info.h"

namespace OHOS {
namespace AccessControl {
namespace SandboxManager {

class SandboxManagerKit {
public:
    static int32_t SetPolicy(uint32_t tokenid, const std::vector<PolicyInfo> &policys, uint64_t policyFlag,
        std::vector<uint32_t> &results);

    static int32_t CheckPolicy(uint32_t tokenid, const std::vector<PolicyInfo> &policys, std::vector<bool> &results);

    static int32_t CheckPersistPolicy(uint32_t tokenid, const std::vector<PolicyInfo> &policys,
        std::vector<bool> &results);

    static int32_t PersistPolicy(uint32_t tokenid, const std::vector<PolicyInfo> &policys,
        std::vector<uint32_t> &results);

    static int32_t UnSetPolicy(uint32_t tokenid, const PolicyInfo &policy);

    static int32_t StartAccessingByTokenId(uint32_t tokenid);

    static void Init();

public:
    static std::mutex mutex_;
    static std::mutex persistMutex_;
    static std::map<std::string, int32_t> policyMap_;
    static std::map<std::string, int32_t> persistPolicyMap_;

    static int32_t SetPolicyRet_;
    static int32_t UnSetPolicyRet_;
};

} // OHOS
} // AccessControl
} // SandboxManager
#endif // SANDBOXMANAGER_KIT_H