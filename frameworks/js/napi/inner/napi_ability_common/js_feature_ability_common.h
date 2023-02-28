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

#ifndef OHOS_ABILITY_RUNTIME_JS_FEATURE_ABILITY_COMMON_H
#define OHOS_ABILITY_RUNTIME_JS_FEATURE_ABILITY_COMMON_H
#include "ability.h"
#include "ability_info.h"
#include "abs_shared_result_set.h"
#include "application_info.h"
#include "data_ability_predicates.h"
#include "hap_module_info.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "napi_common.h"
#include "napi_common_util.h"
#include "pac_map.h"
#include "values_bucket.h"
#include "want.h"

using Want = OHOS::AAFwk::Want;
using Ability = OHOS::AppExecFwk::Ability;
using AbilityStartSetting = OHOS::AppExecFwk::AbilityStartSetting;
using DataAbilityHelper = OHOS::AppExecFwk::DataAbilityHelper;
using CallbackInfo = OHOS::AppExecFwk::CallbackInfo;
using AbilityType = OHOS::AppExecFwk::AbilityType;
using DataAbilityOperation = OHOS::AppExecFwk::DataAbilityOperation;
using DataAbilityResult = OHOS::AppExecFwk::DataAbilityResult;

namespace OHOS {
namespace AbilityRuntime {
struct CallAbilityParam {
    Want want;
    int requestCode = 0;
    bool forResultOption = false;
    std::shared_ptr<AbilityStartSetting> setting = nullptr;
};

struct OnAbilityCallback {
    int requestCode = 0;
    int resultCode = 0;
    Want resultData;
    CallbackInfo cb;
};

struct ContinueAbilityOptionsInfo {
    bool reversible = false;
    std::string deviceId;
};

struct AsyncCallbackInfo {
    CallbackInfo cbInfo;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    Ability *ability = nullptr;
    AbilityRuntime::WantAgent::WantAgent *wantAgent = nullptr;
    CallAbilityParam param;
    CallbackInfo aceCallback;
    bool native_result;
    AbilityType abilityType = AbilityType::UNKNOWN;
    int errCode = 0;
    ContinueAbilityOptionsInfo optionInfo;
#ifdef SUPPORT_GRAPHICS
    sptr<OHOS::Rosen::Window> window;
#endif
};

struct CBBase {
    CallbackInfo cbInfo;
    napi_async_work asyncWork;
    napi_deferred deferred;
    Ability *ability = nullptr;
    AbilityType abilityType = AbilityType::UNKNOWN;
    int errCode = 0;
};

struct DataAbilityHelperCB {
    CBBase cbBase;
    napi_ref uri = nullptr;
    napi_value result = nullptr;
};

struct DAHelperInsertCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    NativeRdb::ValuesBucket valueBucket;
    int result = 0;
    int execResult;
};

struct DAHelperNotifyChangeCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    int execResult;
};

class NAPIDataAbilityObserver;
struct DAHelperOnOffCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    sptr<NAPIDataAbilityObserver> observer;
    std::string uri;
    int result = 0;
    std::vector<DAHelperOnOffCB *> NotifyList;
    std::vector<DAHelperOnOffCB *> DestroyList;
};

struct DAHelperGetTypeCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    std::string result = "";
    int execResult;
};

struct DAHelperGetFileTypesCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    std::string mimeTypeFilter;
    std::vector<std::string> result;
    int execResult;
};

struct DAHelperNormalizeUriCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    std::string result = "";
    int execResult;
};
struct DAHelperDenormalizeUriCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    std::string result = "";
    int execResult;
};

struct DAHelperDeleteCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    NativeRdb::DataAbilityPredicates predicates;
    int result = 0;
    int execResult;
};

struct DAHelperQueryCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> result;
    int execResult;
};

struct DAHelperUpdateCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    NativeRdb::ValuesBucket valueBucket;
    NativeRdb::DataAbilityPredicates predicates;
    int result = 0;
    int execResult;
};

struct DAHelperCallCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    std::string method;
    std::string arg;
    AppExecFwk::PacMap pacMap;
    std::shared_ptr<AppExecFwk::PacMap> result;
    int execResult;
};

struct DAHelperBatchInsertCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    std::vector<NativeRdb::ValuesBucket> values;
    int result = 0;
    int execResult;
};
struct DAHelperOpenFileCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    std::string mode;
    int result = 0;
    int execResult;
};

struct DAHelperExecuteBatchCB {
    CBBase cbBase;
    std::string uri;
    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::vector<std::shared_ptr<DataAbilityResult>> result;
    int execResult;
};

static inline std::string NapiValueToStringUtf8(napi_env env, napi_value value)
{
    std::string result = "";
    return AppExecFwk::UnwrapStringFromJS(env, value, result);
}

static inline bool NapiValueToArrayStringUtf8(napi_env env, napi_value param, std::vector<std::string> &result)
{
    return AppExecFwk::UnwrapArrayStringFromJS(env, param, result);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_FEATURE_ABILITY_COMMON_H

