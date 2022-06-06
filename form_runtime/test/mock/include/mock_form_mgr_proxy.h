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
#ifndef FOUNDATION_APPEXECFWK_OHOS_MOCK_FORM_MGR_PROXY_H
#define FOUNDATION_APPEXECFWK_OHOS_MOCK_FORM_MGR_PROXY_H

#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "form_mgr_proxy.h"

namespace OHOS {
namespace AppExecFwk {
class MockFormMgrProxy : public FormMgrProxy {
public:
    MockFormMgrProxy(const sptr<IRemoteObject> &impl) : FormMgrProxy(impl) {};
    MOCK_METHOD2(GetFormsInfo, int(const std::string &moduleName, std::vector<FormInfo> &formInfos));
};
}
}
#endif // FOUNDATION_APPEXECFWK_OHOS_MOCK_FORM_MGR_PROXY_H
