/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_IPC_TEST_SERVICE_CLIENT_H
#define OHOS_IPC_TEST_SERVICE_CLIENT_H

#include "ipc_debug.h"
#include "log_tags.h"
#include "zidl_test_service_proxy.h"

namespace OHOS {
class TestClient {
public:
    int ConnectService();
    void StartIntTransaction();
    void StartStringTransaction();
private:
    static constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_ID_IPC, "TestClient" };
    sptr<ZidlTestServiceProxy> testService_;
};
} // namespace OHOS
#endif // OHOS_IPC_TEST_SERVICE_CLIENT_H
