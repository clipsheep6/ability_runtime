/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "service_router_mgr_helper.h"

#include <unistd.h>

#include "bundle_constants.h"
#include "hilog_tag_wrapper.h"
#include "iservice_registry.h"
#include "service_router_load_callback.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
static const int LOAD_SA_TIMEOUT_MS = 60000;
}
ServiceRouterMgrHelper::ServiceRouterMgrHelper()
{}

ServiceRouterMgrHelper::~ServiceRouterMgrHelper()
{}

void ServiceRouterMgrHelper::OnRemoteDiedHandle()
{
    TAG_LOGE(AAFwkTag::SER_ROUTER, "Remove service died.");
    SetServiceRouterMgr(nullptr);
    std::unique_lock<std::mutex> lock(cvLock_);
    isReady = false;
}

void ServiceRouterMgrHelper::SetServiceRouterMgr(const sptr<IServiceRouterManager> &serviceRouterMgr)
{
    std::unique_lock<std::mutex> lock(mgrMutex_);
    routerMgr_ = serviceRouterMgr;
}

sptr<IServiceRouterManager> ServiceRouterMgrHelper::InnerGetServiceRouterMgr()
{
    std::unique_lock<std::mutex> lock(mgrMutex_);
    return routerMgr_;
}

void ServiceRouterMgrHelper::LoadSA()
{
    {
        std::unique_lock<std::mutex> lock(cvLock_);
        isReady = false;
    }
    sptr<ISystemAbilityManager> saManager = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "LoadSA, GetSystemAbilityManager is null.");
        return;
    }

    sptr<ServiceRouterLoadCallback> loadCallback = new (std::nothrow) ServiceRouterLoadCallback();
    if (loadCallback == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "LoadSA, new ServiceRouterLoadCallback return null.");
        return;
    }
    int32_t result = saManager->LoadSystemAbility(OHOS::SERVICE_ROUTER_MGR_SERVICE_ID, loadCallback);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "LoadSA, LoadSystemAbility result: %{public}d", result);
        return;
    }
}

void ServiceRouterMgrHelper::FinishStartSASuccess(const sptr<IRemoteObject> &remoteObject)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "FinishStartSASuccess.");
    SetServiceRouterMgr(OHOS::iface_cast<IServiceRouterManager>(remoteObject));

    {
        std::unique_lock<std::mutex> lock(cvLock_);
        isReady = true;
    }
    mgrConn_.notify_one();

    serviceDeathObserver_ = new (std::nothrow) ServiceRouterDeathRecipient();
    if (serviceDeathObserver_ != nullptr) {
        remoteObject->AddDeathRecipient(serviceDeathObserver_);
    }
}

void ServiceRouterMgrHelper::FinishStartSAFail()
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "FinishStartSAFail.");
    SetServiceRouterMgr(nullptr);

    {
        std::unique_lock<std::mutex> lock(cvLock_);
        isReady = false;
    }
    mgrConn_.notify_one();
}

sptr<IServiceRouterManager> ServiceRouterMgrHelper::GetServiceRouterMgr()
{
    auto routerMgr = InnerGetServiceRouterMgr();
    if (routerMgr != nullptr) {
        return routerMgr;
    }

    LoadSA();

    {
        std::unique_lock<std::mutex> lock(cvLock_);
        auto waitState = mgrConn_.wait_for(lock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS),
            [this]() {
                return isReady;
            });
        if (!waitState) {
            return nullptr;
        }
    }

    routerMgr = InnerGetServiceRouterMgr();
    if (routerMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetServiceRouterMgr, after load  routerMgr_ is null");
    }
    return routerMgr;
}
} // namespace AbilityRuntime
} // namespace OHOS
