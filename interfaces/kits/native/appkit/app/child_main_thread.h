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

#ifndef OHOS_ABILITY_RUNTIME_CHILD_MAIN_THREAD_H
#define OHOS_ABILITY_RUNTIME_CHILD_MAIN_THREAD_H

#include <string>
#include <vector>
#include <memory>

#include "app_mgr_interface.h"
#include "bundle_info.h"
#include "bundle_mgr_interface.h"
#include "child_scheduler_interface.h"
#include "child_scheduler_stub.h"
#include "child_process_info.h"
#include "event_handler.h"
#include "ipc_singleton.h"
#include "js_runtime.h"

namespace OHOS {
namespace AppExecFwk {
class ChildMainThread : public ChildSchedulerStub {
    DECLARE_DELAYED_IPCSINGLETON(ChildMainThread);
    
public:
    static void Start(const ChildProcessInfo &processInfo);
    void ScheduleLoadJs() override;
    void ScheduleExitProcessSafely() override;

private:
    bool Init(const std::shared_ptr<EventRunner> &runner, const ChildProcessInfo &processInfo);
    bool Attach();
    void HandleLoadJs();
    void InitNativeLib(const BundleInfo &bundleInfo);
    void HandleExitProcessSafely();
    void ExitProcessSafely();
    void GetNativeLibPath(const BundleInfo &bundleInfo, AppLibPathMap &appLibPaths);
    void GetHapSoPath(const HapModuleInfo &hapInfo, AppLibPathMap &appLibPaths, bool isPreInstallApp);
    std::string GetLibPath(const std::string &hapPath, bool isPreInstallApp);

    sptr<IAppMgr> appMgr_ = nullptr;
    std::shared_ptr<EventHandler> mainHandler_ = nullptr;
    std::shared_ptr<BundleInfo> bundleInfo_ = nullptr;
    std::shared_ptr<ChildProcessInfo> processInfo_ = nullptr;
    std::unique_ptr<AbilityRuntime::Runtime> runtime_ = nullptr;

    DISALLOW_COPY_AND_MOVE(ChildMainThread);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CHILD_MAIN_THREAD_H
