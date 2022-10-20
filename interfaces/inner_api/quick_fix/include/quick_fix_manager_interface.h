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

#ifndef OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_INTERFACE_H

#include "iremote_broker.h"
#include "quick_fix_info.h"

namespace OHOS {
namespace AAFwk {
class IQuickFixManager : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.QuickFixManager");

    /**
     * @brief Apply quick fix.
     *
     * @param quickFixFiles quick fix files need to apply, this value should include file path and file name.
     * @return returns 0 on success, error code on failure.
     */
    virtual int32_t ApplyQuickFix(const std::vector<std::string> &quickFixFiles) = 0;

    /**
     * @brief Get applyed quick fix info.
     *
     * @param bundleName bundle name of quick fix info.
     * @param quickFixInfo quick fix info, including bundleName, bundleVersion and so on.
     * @return int32_t returns 0 on success, error code on failure.
     */
    virtual int32_t GetApplyedQuickFixInfo(const std::string &bundleName, ApplicationQuickFixInfo &quickFixInfo) = 0;

    enum QuickFixMgrCmd {
        ON_APPLY_QUICK_FIX = 0,             // ipc id for ApplyQuickFix
        ON_GET_APPLYED_QUICK_FIX_INFO = 1,  // ipc id for GetApplyedQuickFixInfo
    };
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_INTERFACE_H
