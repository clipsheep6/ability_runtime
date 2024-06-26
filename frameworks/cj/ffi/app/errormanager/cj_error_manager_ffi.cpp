/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstring.h>
#include "cj_error_manager_ffi.h"
#include "cj_error_observer.h"
#include "application_data_manager.h"
#include "hilog_tag_wrapper.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS::FFI;

namespace {
    constexpr int ERR_PARAM = 401;
    constexpr int ERROR_CODE_INVALID_ID = 16000003;
    constexpr const char* ON_OFF_TYPE = "error";
    int32_t g_serialNumber = 0;
    std::shared_ptr<ErrorObserver> g_observer;
}

extern "C" {
RetDataI32 FfiOHOSErrorManagerOn(char* onType, CErrorObserver observer)
{
    TAG_LOGI(AAFwkTag::APPKIT, "FfiOHOSErrorManagerOn.");
    RetDataI32 ret = { .code = ERR_PARAM, .data = 0 };
    if (strcmp(onType, ON_OFF_TYPE) != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "FfiOHOSErrorManagerOn failed.");
        return ret;
    }
    int32_t observerId = g_serialNumber;
    if (g_serialNumber < INT32_MAX) {
        g_serialNumber++;
    } else {
        g_serialNumber = 0;
    }

    if (g_observer == nullptr) {
        TAG_LOGI(AAFwkTag::APPKIT, "g_observer is null.");
        g_observer = std::make_shared<ErrorObserver>();
        OHOS::AppExecFwk::ApplicationDataManager::GetInstance().AddErrorObserver(g_observer);
    }

    g_observer->AddObserverObject(observerId, observer);
    TAG_LOGI(AAFwkTag::APPKIT, "FfiOHOSErrorManagerOn success.");
    ret.code = SUCCESS_CODE;
    ret.data = observerId;
    return ret;
}

int FfiOHOSErrorManagerOff(char* offType, int observerId)
{
    TAG_LOGI(AAFwkTag::APPKIT, "FfiOHOSErrorManagerOff.");
    int ret = ERR_PARAM;
    TAG_LOGI(AAFwkTag::APPKIT, "unregister errorObserver called, observer:%{public}d", observerId);
    if (strcmp(offType, ON_OFF_TYPE) != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "FfiOHOSErrorManagerOff failed.");
        return ret;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "Unregister errorObserver called.");
    if (g_observer == nullptr || !g_observer->RemoveObserverObject(observerId)) {
        ret = ERROR_CODE_INVALID_ID;
        return ret;
    }

    if (g_observer && g_observer->IsEmpty()) {
        OHOS::AppExecFwk::ApplicationDataManager::GetInstance().RemoveErrorObserver();
        g_observer = nullptr;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "FfiOHOSErrorManagerOff success.");
    ret = SUCCESS_CODE;
    return ret;
}
}