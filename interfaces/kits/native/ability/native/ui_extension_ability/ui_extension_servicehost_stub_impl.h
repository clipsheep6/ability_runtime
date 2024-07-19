/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_SERVICEHOST_STUB_IMPL_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_SERVICEHOST_STUB_IMPL_H

#include "js_ui_extension_context.h"

#include "ui_service_host_stub.h"

namespace OHOS {
namespace AbilityRuntime {

class UIExtensionServiceHostStubImpl : public AAFwk::UIServiceHostStub {
public:
    UIExtensionServiceHostStubImpl(wptr<JSUIServiceUIExtConnection> conn);
    ~UIExtensionServiceHostStubImpl() = default;
    virtual int32_t SendData(OHOS::AAFwk::WantParams &data) override;

protected:
    wptr<JSUIServiceUIExtConnection> conn_;
};

}
}
#endif

