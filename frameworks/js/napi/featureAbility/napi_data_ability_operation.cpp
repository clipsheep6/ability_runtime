/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "napi_data_ability_operation.h"

#include <cstring>
#include <map>

#include "data_ability_predicates.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "napi_common_want.h"
#include "napi_data_ability_helper.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
napi_value DataAbilityOperationInit(napi_env env, napi_value exports)
{
    const int INSERT = 1;
    const int UPDATE = 2;
    const int DELETE = 3;
    const int ASSERT = 4;
    TAG_LOGD(AAFwkTag::FA, "called");

    napi_value dataAbilityOperationType = nullptr;
    napi_create_object(env, &dataAbilityOperationType);
    SetNamedProperty(env, dataAbilityOperationType, "TYPE_INSERT", INSERT);
    SetNamedProperty(env, dataAbilityOperationType, "TYPE_UPDATE", UPDATE);
    SetNamedProperty(env, dataAbilityOperationType, "TYPE_DELETE", DELETE);
    SetNamedProperty(env, dataAbilityOperationType, "TYPE_ASSERT", ASSERT);

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_PROPERTY("DataAbilityOperationType", dataAbilityOperationType),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties));

    return exports;
}

napi_value UnwrapDataAbilityOperation(
    std::shared_ptr<DataAbilityOperation> &dataAbilityOperation, napi_env env, napi_value param)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s called.", __func__);
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Params is invalid.", __func__);
        return nullptr;
    }

    napi_value result = BuildDataAbilityOperation(dataAbilityOperation, env, param);
    return result;
}

napi_value BuildDataAbilityOperation(
    std::shared_ptr<DataAbilityOperation> &dataAbilityOperation, napi_env env, napi_value param)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s start.", __func__);

    // get uri property
    std::string uriStr("");
    if (!UnwrapStringByPropertyName(env, param, "uri", uriStr)) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, uri is not exist.", __func__);
        return nullptr;
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s, uri:%{public}s", __func__, uriStr.c_str());
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(uriStr);

    // get type property
    int type = 0;
    if (!UnwrapInt32ByPropertyName(env, param, "type", type)) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, type:%{public}d is not exist.", __func__, type);
        return nullptr;
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s, type:%{public}d", __func__, type);

    std::shared_ptr<DataAbilityOperationBuilder> builder = nullptr;
    if (!GetDataAbilityOperationBuilder(builder, type, uri)) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, GetDataAbilityOperationBuilder failed.", __func__);
        return nullptr;
    }

    // get valuesBucket property
    std::shared_ptr<NativeRdb::ValuesBucket> valuesBucket = std::make_shared<NativeRdb::ValuesBucket>();
    valuesBucket->Clear();
    napi_value jsValueBucket = GetPropertyValueByPropertyName(env, param, "valuesBucket", napi_object);
    UnwrapValuesBucket(valuesBucket, env, jsValueBucket);
    builder->WithValuesBucket(valuesBucket);

    // get dataAbilityPredicates property
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates = std::make_shared<NativeRdb::DataAbilityPredicates>();
    napi_value jsPredicates = GetPropertyValueByPropertyName(env, param, "predicates", napi_object);
    UnwrapDataAbilityPredicates(*predicates, env, jsPredicates);
    builder->WithPredicates(predicates);

    // get expectedcount property
    int expectedCount = 0;
    UnwrapInt32ByPropertyName(env, param, "expectedCount", expectedCount);
    TAG_LOGI(AAFwkTag::FA, "%{public}s, expectedCount:%{public}d", __func__, expectedCount);
    if (expectedCount > 0) {
        builder->WithExpectedCount(expectedCount);
    }

    // get PredicatesBackReferences property
    napi_value jsPredicatesBackReferences =
        GetPropertyValueByPropertyName(env, param, "PredicatesBackReferences", napi_object);
    UnwrapDataAbilityPredicatesBackReferences(builder, env, jsPredicatesBackReferences);

    // get interrupted property
    bool interrupted = false;
    UnwrapBooleanByPropertyName(env, param, "interrupted", interrupted);
    builder->WithInterruptionAllowed(interrupted);

    // get backReferences
    std::shared_ptr<NativeRdb::ValuesBucket> backReferences = std::make_shared<NativeRdb::ValuesBucket>();
    backReferences->Clear();
    napi_value jsBackReferences = GetPropertyValueByPropertyName(env, param, "valueBackReferences", napi_object);
    UnwrapValuesBucket(backReferences, env, jsBackReferences);
    builder->WithValueBackReferences(backReferences);

    if (builder != nullptr) {
        TAG_LOGI(AAFwkTag::FA, "%{public}s, builder is not nullptr", __func__);
        dataAbilityOperation = builder->Build();
    }
    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    TAG_LOGI(AAFwkTag::FA, "%{public}s end.", __func__);
    return result;
}

bool GetDataAbilityOperationBuilder(
    std::shared_ptr<DataAbilityOperationBuilder> &builder, const int type, const std::shared_ptr<Uri> &uri)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s called.", __func__);
    switch (type) {
        case DataAbilityOperation::TYPE_INSERT:
            builder = DataAbilityOperation::NewInsertBuilder(uri);
            break;
        case DataAbilityOperation::TYPE_UPDATE:
            builder = DataAbilityOperation::NewUpdateBuilder(uri);
            break;
        case DataAbilityOperation::TYPE_DELETE:
            builder = DataAbilityOperation::NewDeleteBuilder(uri);
            break;
        case DataAbilityOperation::TYPE_ASSERT:
            builder = DataAbilityOperation::NewAssertBuilder(uri);
            break;
        default:
            TAG_LOGE(AAFwkTag::FA, "%{public}s, type:%{public}d is invalid.", __func__, type);
            return false;
    }
    return true;
}

napi_value UnwrapValuesBucket(const std::shared_ptr<NativeRdb::ValuesBucket> &param, napi_env env,
    napi_value valueBucketParam)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s called.", __func__);
    napi_value result;

    if (param == nullptr) {
        TAG_LOGI(AAFwkTag::FA, "%{public}s input param is nullptr.", __func__);
        NAPI_CALL(env, napi_create_int32(env, 0, &result));
        return result;
    }
    AnalysisValuesBucket(*param, env, valueBucketParam);

    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    return result;
}

napi_value UnwrapDataAbilityPredicatesBackReferences(
    std::shared_ptr<DataAbilityOperationBuilder> &builder, napi_env env, napi_value predicatesBackReferencesParam)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s called.", __func__);

    if (!IsTypeForNapiValue(env, predicatesBackReferencesParam, napi_object)) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, predicatesBackReferencesParam is invalid.", __func__);
        return nullptr;
    }

    napi_valuetype jsValueType = napi_undefined;
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;

    NAPI_CALL(env, napi_get_property_names(env, predicatesBackReferencesParam, &jsProNameList));
    NAPI_CALL(env, napi_get_array_length(env, jsProNameList, &jsProCount));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, Property size=%{public}d.", __func__, jsProCount);

    napi_value jsProName = nullptr;
    napi_value jsProValue = nullptr;
    for (uint32_t index = 0; index < jsProCount; index++) {
        NAPI_CALL(env, napi_get_element(env, jsProNameList, index, &jsProName));
        std::string strProName = UnwrapStringFromJS(env, jsProName);
        int intProName = std::atoi(strProName.c_str());
        TAG_LOGI(AAFwkTag::FA, "%{public}s, Property name=%{public}d.", __func__, intProName);
        NAPI_CALL(env, napi_get_property(env, predicatesBackReferencesParam, jsProName, &jsProValue));
        NAPI_CALL(env, napi_typeof(env, jsProValue, &jsValueType));
        int32_t natValue32 = 0;
        if (napi_get_value_int32(env, jsProValue, &natValue32) == napi_ok) {
            TAG_LOGI(AAFwkTag::FA, "%{public}s, Property value=%{public}d.", __func__, natValue32);
            builder->WithPredicatesBackReference(intProName, natValue32);
        }
    }
    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    return result;
}

void SetNamedProperty(napi_env env, napi_value obj, const char *propName, int propValue)
{
    napi_value prop = nullptr;
    napi_create_int32(env, propValue, &prop);
    napi_set_named_property(env, obj, propName, prop);
}
}  // namespace AppExecFwk
}  // namespace OHOS
