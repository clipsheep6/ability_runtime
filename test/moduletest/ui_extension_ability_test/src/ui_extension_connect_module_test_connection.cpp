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

#include "ui_extension_connect_module_test_connection.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
void UIExtensionConnectModuleTestConnection::OnAbilityConnectDone(const AppExecFwk::ElementName& element,
    const sptr<IRemoteObject>& remoteObject, int resultCode)
{
    TAG_LOGI(AAFwkTag::TEST, "element: %{public}s", element.GetURI().c_str());
    std::unique_lock<std::mutex> lock(connectMutex_);
    connectFinished_ = true;
    connectCondation_.notify_one();
}

void UIExtensionConnectModuleTestConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName& element,
    int resultCode)
{
    TAG_LOGI(AAFwkTag::TEST, "element: %{public}s", element.GetURI().c_str());
    std::unique_lock<std::mutex> lock(connectMutex_);
    disConnectFinished_ = true;
    connectCondation_.notify_one();
}
} // namespace AAFwk
} // namespace OHOS
