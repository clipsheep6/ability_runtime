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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_FREE_INSTALL_OPERATOR_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_FREE_INSTALL_OPERATOR_H

#include <future>
#include "free_install_status_callback_stub.h"
#include "event_handler.h"

namespace OHOS {
namespace AppExecFwk {
class FreeInstallStatusCallBack;
/**
 * @class FormFreeInstallOperator
 * Free install operator class.
 */
class FormFreeInstallOperator final : public std::enable_shared_from_this<FormFreeInstallOperator> {
public:
    FormFreeInstallOperator(const std::string &formShareInfoKey,
        const std::shared_ptr<AppExecFwk::EventHandler> &handler);
    ~FormFreeInstallOperator();

    int StartFreeInstall(const std::string &bundleName, const std::string &abilityName);
    void OnInstallFinished(int resultCode);

private:
    std::string formShareInfoKey_;
    sptr<FreeInstallStatusCallBack> freeInstallStatusCallBack_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
};

class FreeInstallStatusCallBack final : public FreeInstallStatusCallBackStub {
public:
    FreeInstallStatusCallBack(const std::weak_ptr<FormFreeInstallOperator> &freeInstallOperator);
    virtual ~FreeInstallStatusCallBack() = default;
    /**
     * OnInstallFinished, FreeInstall is complete.
     *
     * @param resultCode, ERR_OK on success, others on failure.
     * @param want, installed ability.
     * @param userId, user`s id.
     */
    virtual void OnInstallFinished(int resultCode, const Want &want, int32_t userId) override;

private:
    std::weak_ptr<FormFreeInstallOperator> formFreeInstallOperator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_FREE_INSTALL_OPERATOR_H
