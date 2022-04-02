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

#include <cstdio>

#include "ability_manager_client.h"
#include "want.h"

using OHOS::AppExecFwk::ElementName;
using OHOS::AppExecFwk::Want;
using OHOS::AAFwk::AbilityManagerClient;
using OHOS::IRemoteObject;

namespace {
static ElementName g_testAbility1("", "com.ix.hiMusic", "MusicAbility");
}  // namespace

int main()
{
    std::unique_ptr<AbilityManagerClient> client(new AbilityManagerClient());
    auto ret = client->Connect();
    if (ret != 0) {
        printf("ConnectAppMgrService failed\n");
        return -1;
    }

    Want want;
    printf("Set free install flag\n");
    want.SetFlags(Want::FLAG_INSTALL_ON_DEMAND);
    want.SetElement(g_testAbility1);
    client->StartAbility(want, nullptr);
    return 0;
}
