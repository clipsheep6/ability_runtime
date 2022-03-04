/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "iremote_object.h"

#include "ability_manager_client.h"
#include "ability_controller_stub.h"
#include "ability_connection.h"
#include "want.h"
#include <unistd.h>

using OHOS::AAFwk::AbilityManagerClient;
using OHOS::sptr;
using OHOS::IRemoteObject;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
class MyAbilityControllerProxy : public OHOS::AppExecFwk::AbilityControllerStub {
public:
    virtual bool AllowAbilityStart(const OHOS::AAFwk::Want &want, const std::string &bundleName) override
    {
        return false;
    }

    virtual bool AllowAbilityBackground(const std::string &bundleName) override
    {
        return false;
    }

};

int main(int argc, char *argv[])
{
    std::string cmd = argv[1];
    std::string result;
    if (cmd == "setACTrue") {
        int res = AbilityManagerClient::GetInstance()->SetAbilityController(new MyAbilityControllerProxy(), true);
        result = (res == 0) ? "success" : "fail";
        sleep(300);
    } else if (cmd == "setACFalse") {
        int res = AbilityManagerClient::GetInstance()->SetAbilityController(new MyAbilityControllerProxy(), false);
        result = (res == 0) ? "success" : "fail";
        sleep(300);
    } else if (cmd == "doForeground") {
        sptr<IRemoteObject> token = nullptr;
        AbilityManagerClient::GetInstance()->GetCurrentTopAbility(token);
        if (token == nullptr) {
            std::cout << "token null";
            return 0;
        }
        int res = AbilityManagerClient::GetInstance()->DoAbilityForeground(token, 1);
        result = (res == 0) ? "success" : "fail";
    } else if (cmd == "doBackground") {
        sptr<IRemoteObject> token = nullptr;
        AbilityManagerClient::GetInstance()->GetCurrentTopAbility(token);
        if (token == nullptr) {
            std::cout << "token null";
            return 0;
        }
        int res = AbilityManagerClient::GetInstance()->DoAbilityBackground(token, 1);
        result = (res == 0) ? "success" : "fail";
    } else if (cmd == "connect") {
        Want want;
        ElementName element("", "ohos.dms.jsDemo", "ohos.dms.jsDemo.ServiceAbility");
        want.SetElement(element);
        int res = AbilityManagerClient::GetInstance()->ConnectAbility(want,
            new OHOS::AbilityRuntime::AbilityConnection(), new MyAbilityControllerProxy(), 100);
        result = (res == 0) ? "success" : "fail";
    }
    std::cout << result;
    return 0;
}