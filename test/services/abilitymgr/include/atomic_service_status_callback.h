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

#ifndef OHOS_ABILITY_RUNTIME_ATOMIC_SERVICE_STATUS_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_ATOMIC_SERVICE_STATUS_CALLBACK_H

#include <iremote_object.h>
#include <iremote_stub.h>

#include "ability_info.h"
#include "atomic_service_status_callback_stub.h"

namespace OHOS {
namespace AAFwk {
class FreeInstallManager;
/**
 * @class AtomicServiceStatusCallback
 * AtomicServiceStatusCallback.
 */
class AtomicServiceStatusCallback : public AtomicServiceStatusCallbackStub {
public:
    AtomicServiceStatusCallback(const std::weak_ptr<FreeInstallManager> &server, bool isAsync);
    virtual ~AtomicServiceStatusCallback() = default;

    /**
     * OnInstallFinished, FreeInstall is complete.
     *
     * @param resultCode, ERR_OK on success, others on failure.
     * @param want, installed ability.
     * @param userId, user`s id.
     */
    void OnInstallFinished(int resultCode, const Want &want, int32_t userId) override;

    /**
     * OnRemoteInstallFinished, DMS has finished.
     *
     * @param resultCode, ERR_OK on success, others on failure.
     * @param want, installed ability.
     * @param userId, user`s id.
     */
    void OnRemoteInstallFinished(int resultCode, const Want &want, int32_t userId) override;

    /**
     * OnRemoveTimeoutTask, BMS has connected AG.
     *
     * @param want, installed ability
     */
    void OnRemoveTimeoutTask(const Want &want) override;

private:
    std::weak_ptr<FreeInstallManager> server_;
    bool isAsync_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ATOMIC_SERVICE_STATUS_CALLBACK_H
