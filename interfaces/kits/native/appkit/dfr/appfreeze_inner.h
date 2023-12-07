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
#ifndef OHOS_ABILITY_ABILITY_APPFREEZE_LOG_CLIENT_H
#define OHOS_ABILITY_ABILITY_APPFREEZE_LOG_CLIENT_H
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <set>
#include <sys/types.h>

#include "refbase.h"
#include "singleton.h"

#include "event_handler.h"
#include "app_mgr_client.h"
#include "app_mgr_interface.h"
#include "application_impl.h"
#include "fault_data.h"

namespace OHOS {
namespace AppExecFwk {
class AppfreezeInner {
public:
    AppfreezeInner();
    ~AppfreezeInner();
    static std::shared_ptr<AppfreezeInner> GetInstance();
    static void DestroyInstance();
    static void SetMainHandler(const std::shared_ptr<EventHandler>& eventHandler);
    void SetApplicationInfo(const std::shared_ptr<ApplicationInfo>& applicationInfo);
    void ThreadBlock(std::atomic_bool& isSixSecondEvent);
    int AppfreezeHandle(const FaultData& faultInfo, bool onlyMainThread);
    int AcquireStack(const FaultData& faultInfo, bool onlyMainThread);
    void SetAppDebug(bool isAppDebug);
private:
    static std::weak_ptr<EventHandler> appMainHandler_;
    std::weak_ptr<ApplicationInfo> applicationInfo_;
    void AppFreezeRecovery();
    int NotifyANR(const FaultData& faultData);
    bool IsExitApp(const std::string& name);
    bool IsHandleAppfreeze();

    static std::mutex singletonMutex_;
    static std::shared_ptr<AppfreezeInner> instance_;
    bool isAppDebug_ = false;
    std::mutex handlingMutex_;
    std::list<FaultData> handlinglist_;
};

class MainHandlerDumper : public Dumper {
public:
    virtual void Dump(const std::string &message) override;
    virtual std::string GetTag() override;
    std::string GetDumpInfo();
private:
    std::string dumpInfo;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_ABILITY_APPFREEZE_LOG_CLIENT_H