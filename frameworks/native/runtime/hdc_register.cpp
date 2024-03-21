/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "hdc_register.h"

#include <dlfcn.h>
#include <unistd.h>

#include "hilog_wrapper.h"

namespace OHOS::AbilityRuntime {
using StartRegister = void (*)(const std::string& processName, const std::string& pkgName, bool isDebug,
    const HdcRegisterCallback& callback);
using StopRegister = void (*)();

HdcRegister::~HdcRegister()
{
    StopHdcRegister();
}

HdcRegister& HdcRegister::Get()
{
    static HdcRegister hdcRegister;
    return hdcRegister;
}

void HdcRegister::StartHdcRegister(const std::string& bundleName, const std::string& processName, bool debugApp,
    HdcRegisterCallback callback)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "HdcRegister::StartHdcRegister begin");

    registerHandler_ = dlopen("libhdc_register.z.so", RTLD_LAZY);
    if (registerHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "HdcRegister::StartHdcRegister failed to open register library");
        return;
    }
    auto startRegister = reinterpret_cast<StartRegister>(dlsym(registerHandler_, "StartConnect"));
    if (startRegister == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "HdcRegister::StartHdcRegister failed to find symbol 'StartConnect'");
        return;
    }
    startRegister(processName, bundleName, debugApp, callback);
    
    TAG_LOGD(AAFwkTag::JSRUNTIME, "HdcRegister::StartHdcRegister end");
}

void HdcRegister::StopHdcRegister()
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "HdcRegister::StopHdcRegister begin");
    if (registerHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "HdcRegister::StopHdcRegister registerHandler_ is nullptr");
        return;
    }
    auto stopRegister = reinterpret_cast<StopRegister>(dlsym(registerHandler_, "StopConnect"));
    if (stopRegister != nullptr) {
        stopRegister();
    } else {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "HdcRegister::StopHdcRegister failed to find symbol 'StopConnect'");
    }
    dlclose(registerHandler_);
    registerHandler_ = nullptr;
    TAG_LOGD(AAFwkTag::JSRUNTIME, "HdcRegister::StopHdcRegister end");
}
} // namespace OHOS::AbilityRuntime
