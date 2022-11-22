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

#ifndef OHOS_ABILITY_RUNTIME_IQUICK_FIX_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_IQUICK_FIX_CALLBACK_H

#include "iremote_broker.h"

namespace OHOS {
namespace AppExecFwk {
class IQuickFixCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.QuickFixCallback");

    virtual void OnLoadPatchDone(int32_t resultCode) = 0;
    virtual void OnUnloadPatchDone(int32_t resultCode) = 0;
    virtual void OnReloadPageDone(int32_t resultCode) = 0;

    enum QuickFixCallbackCmd {
        ON_NOTIFY_LOAD_PATCH = 0,   // ipc id for OnLoadPatchDone
        ON_NOTIFY_UNLOAD_PATCH = 1, // ipc id for OnUnloadPatchDone
        ON_NOTIFY_RELOAD_PAGE = 2,  // ipc id for OnReloadPageDone
    };
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_IQUICK_FIX_CALLBACK_H
