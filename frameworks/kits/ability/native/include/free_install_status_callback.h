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

#ifndef FOUNDATION_APPEXECFWK_OHOS_FREE_INSTALL_STATUS_CALLBACK_H
#define FOUNDATION_APPEXECFWK_OHOS_FREE_INSTALL_STATUS_CALLBACK_H

#include <future>

#include "iremote_object.h"
#include "iremote_stub.h"

#include "free_install_status_callback_stub.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FreeInstallStatusCallback
 * FreeInstallStatusCallback.
 */
class FreeInstallStatusCallback : public FreeInstallStatusCallbackStub {
public:
    FreeInstallStatusCallback();
    virtual ~FreeInstallStatusCallback() = default;

    /**
     * OnInstallFinished, FreeInstall is complete.
     *
     * @param resultCode, ERR_OK on success, others on failure.
     */
    void OnInstallFinished(int resultCode) override;

    int32_t GetResultCode();

private:
    std::promise<int32_t> resultCodeSignal_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_FREE_INSTALL_STATUS_CALLBACK_H
