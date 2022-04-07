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

#include "free_install_status_callback.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
FreeInstallStatusCallback::FreeInstallStatusCallback()
{
    HILOG_INFO("%{public}s", __func__);
}

void FreeInstallStatusCallback::OnInstallFinished(int resultCode)
{
    HILOG_INFO("%{public}s", __func__);
    resultCodeSignal_.set_value(resultCode);
}

int32_t FreeInstallStatusCallback::GetResultCode()
{
    auto future = resultCodeSignal_.get_future();
    return future.get();
}
}  // namespace AppExecFwk
}  // namespace OHOS
