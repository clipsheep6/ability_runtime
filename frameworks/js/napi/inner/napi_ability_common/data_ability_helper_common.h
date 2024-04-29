/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_DATA_ABILITY_HELPER_COMMON_H
#define OHOS_ABILITY_RUNTIME_DATA_ABILITY_HELPER_COMMON_H

#include <vector>
#include "abs_shared_result_set.h"
#include "data_ability_helper.h"
#include "data_ability_predicates.h"
#include "feature_ability_common.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
struct DAHelperInsertCB {
    CBBase cbBase;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper;
    std::string uri;
    NativeRdb::ValuesBucket valueBucket;
    int result = 0;
    int execResult;
};

struct DAHelperNotifyChangeCB {
    CBBase cbBase;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper;
    std::string uri;
    int execResult;
};

class NAPIDataAbilityObserver;
struct DAHelperOnOffCB {
    CBBase cbBase;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper;
    sptr<NAPIDataAbilityObserver> observer;
    std::string uri;
    int result = 0;
    std::vector<DAHelperOnOffCB *> NotifyList;
    std::vector<DAHelperOnOffCB *> DestroyList;
};

struct DAHelperGetTypeCB {
    CBBase cbBase;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper;
    std::string uri;
    std::string result = "";
    int execResult;
};

struct DAHelperGetFileTypesCB {
    CBBase cbBase;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper;
    std::string uri;
    std::string mimeTypeFilter;
    std::vector<std::string> result;
    int execResult;
};

struct DAHelperNormalizeUriCB {
    CBBase cbBase;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper;
    std::string uri;
    std::string result = "";
    int execResult;
};
struct DAHelperDenormalizeUriCB {
    CBBase cbBase;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper;
    std::string uri;
    std::string result = "";
    int execResult;
};

struct DAHelperDeleteCB {
    CBBase cbBase;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper;
    std::string uri;
    NativeRdb::DataAbilityPredicates predicates;
    int result = 0;
    int execResult;
};

struct DAHelperQueryCB {
    CBBase cbBase;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper;
    std::string uri;
    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> result;
    int execResult;
};

struct DAHelperUpdateCB {
    CBBase cbBase;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper;
    std::string uri;
    NativeRdb::ValuesBucket valueBucket;
    NativeRdb::DataAbilityPredicates predicates;
    int result = 0;
    int execResult;
};

struct DAHelperCallCB {
    CBBase cbBase;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper;
    std::string uri;
    std::string method;
    std::string arg;
    AppExecFwk::PacMap pacMap;
    std::shared_ptr<AppExecFwk::PacMap> result;
    int execResult;
};

struct DAHelperErrorCB {
    CBBase cbBase;
    int execResult;
};
struct DAHelperBatchInsertCB {
    CBBase cbBase;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper;
    std::string uri;
    std::vector<NativeRdb::ValuesBucket> values;
    int result = 0;
    int execResult;
};
struct DAHelperOpenFileCB {
    CBBase cbBase;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper;
    std::string uri;
    std::string mode;
    int result = 0;
    int execResult;
};

struct DAHelperExecuteBatchCB {
    CBBase cbBase;
    std::string uri;
    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper;
    std::vector<std::shared_ptr<DataAbilityResult>> result;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif /* OHOS_ABILITY_RUNTIME_DATA_ABILITY_HELPER_COMMON_H */
