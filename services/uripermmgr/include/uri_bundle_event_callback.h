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

#include "bundle_event_callback_host.h"
#include "common_event_support.h"
#include "hilog_wrapper.h"
#include "uri_permission_manager_stub_impl.h"

namespace OHOS {
namespace AAFwk {
/**
 * @brief This class is a callback class that will be registered to BundleManager.
 * This class will be called by BundleManager when install, uninstall, updates of haps happens,
 * and executes corresponding functionalities of Uri Permission manager.
 */
class UriBundleEventCallback : public AppExecFwk::BundleEventCallbackHost {
public:
    UriBundleEventCallback() = default;
    explicit UriBundleEventCallback(sptr<UriPermissionManagerStubImpl> impl);
    ~UriBundleEventCallback() = default;
    /**
     * @brief The main callback function that will be called by BundleManager
     * when install, uninstall, updates of haps happens to notify UriPermissionManger.
     * @param eventData the data passed from BundleManager that includes bundleName, change type of haps
     * etc. More can be found from BundleCommonEventMgr::NotifyBundleStatus()
     */
    void OnReceiveEvent(const EventFwk::CommonEventData eventData) override;
private:
    sptr<UriPermissionManagerStubImpl> upms_;
};
} // namespace OHOS
} // namespace AAFwk