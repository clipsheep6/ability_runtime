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

#ifndef FORM_DISTRIBUTED_CLIENT_H
#define FORM_DISTRIBUTED_CLIENT_H

#include "want_params.h"
#include "want.h"
#include "iremote_broker.h"
#include "form_share_info.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormDistributedClient
 * form Distributed Client.
 */
class FormDistributedClient {
public:
    FormDistributedClient() = default;
    virtual ~FormDistributedClient() = default;

    int ShareForm(const std::string &remoteDeviceId, const FormShareInfo &formShareInfo);

    enum {
        START_REMOTE_SHARE_FORM = 220,
    };
private:
    void GetDmsServiceProxy();
    void SetDmsProxy(const sptr<IRemoteObject> &dmsProxy);
    sptr<IRemoteObject> dmsProxy_ = nullptr;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FORM_DISTRIBUTED_CLIENT_H
