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

#ifndef OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_SERVICE_H
#define OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_SERVICE_H

#include <mutex>

#include "event_runner.h"
#include "event_handler.h"
#include "quick_fix/quick_fix_manager_interface.h"
#include "quick_fix_manager_apply_task.h"
#include "quick_fix_manager_stub.h"

namespace OHOS {
namespace AAFwk {
class QuickFixManagerService : public QuickFixManagerStub,
                               public std::enable_shared_from_this<QuickFixManagerService> {
public:
    QuickFixManagerService() = default;
    virtual ~QuickFixManagerService() = default;

    /**
     * @brief Get the instance of quick fix manager service.
     *
     * @return Returns the instance.
     */
    static sptr<QuickFixManagerService> GetInstance();

    /**
     * @brief Init quick fix manager service, such as event runner and event handler.
     *
     * @return Returns true when init succeed, false when init failed.
     */
    bool Init();

    /**
     * @brief Apply quick fix.
     *
     * @param quickFixFiles Quick fix files need to apply, this value should include file path and file name.
     * @return Returns 0 on success, error code on failure.
     */
    int32_t ApplyQuickFix(const std::vector<std::string> &quickFixFiles) override;

    /**
     * @brief Get applyed quick fix info.
     *
     * @param bundleName Bundle name of quick fix info.
     * @param quickFixInfo Quick fix info, including bundleName, bundleVersion and so on.
     * @return Returns 0 on success, error code on failure.
     */
    int32_t GetApplyedQuickFixInfo(const std::string &bundleName, ApplicationQuickFixInfo &quickFixInfo) override;

    /**
     * @brief Remove quick fix apply task.
     *
     */
    void RemoveApplyTask(std::shared_ptr<QuickFixManagerApplyTask> applyTask);

private:
    void AddApplyTask(std::shared_ptr<QuickFixManagerApplyTask> applyTask);

    static std::mutex mutex_;
    static sptr<QuickFixManagerService> instance_;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    std::vector<std::shared_ptr<QuickFixManagerApplyTask>> applyTasks_;

    DISALLOW_COPY_AND_MOVE(QuickFixManagerService);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_SERVICE_H
