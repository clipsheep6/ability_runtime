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
#include "napi_data_ability_operation.h"

#include <cstring>
#include <map>

#include "data_ability_predicates.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
#include "napi_data_ability_helper.h"
#include "values_bucket.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AppExecFwk {
napi_value DataAbilityOperationInit(napi_env env, napi_value exports)
{
    const int INSERT = 1;
    const int UPDATE = 2;
    const int DELETE = 3;
    const int ASSERT = 4;
    HILOG_INFO("%{public}s called.", __func__);

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
    HILOG_INFO("%{public}s called.", __func__);
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        HILOG_ERROR("%{public}s, Params is invalid.", __func__);
        return nullptr;
    }

    napi_value result = BuildDataAbilityOperation(dataAbilityOperation, env, param);
    return result;
}

napi_value BuildDataAbilityOperation(
    std::shared_ptr<DataAbilityOperation> &dataAbilityOperation, napi_env env, napi_value param)
{
    HILOG_INFO("%{public}s start.", __func__);

    // get uri property
    std::string uriStr("");
    if (!UnwrapStringByPropertyName(env, param, "uri", uriStr)) {
        HILOG_ERROR("%{public}s, uri is not exist.", __func__);
        return nullptr;
    }
    HILOG_INFO("%{public}s, uri:%{public}s", __func__, uriStr.c_str());
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(uriStr);

    // get type property
    int type = 0;
    if (!UnwrapInt32ByPropertyName(env, param, "type", type)) {
        HILOG_ERROR("%{public}s, type:%{public}d is not exist.", __func__, type);
        return nullptr;
    }
    HILOG_INFO("%{public}s, type:%{public}d", __func__, type);

    std::shared_ptr<DataAbilityOperationBuilder> builder = nullptr;
    if (!GetDataAbilityOperationBuilder(builder, type, uri)) {
        HILOG_ERROR("%{public}s, GetDataAbilityOperationBuilder failed.", __func__);
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
    HILOG_INFO("%{public}s, expectedCount:%{public}d", __func__, expectedCount);
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
        HILOG_INFO("%{public}s, builder is not nullptr", __func__);
        dataAbilityOperation = builder->Build();
    }
    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    HILOG_INFO("%{public}s end.", __func__);
    return result;
}

bool GetDataAbilityOperationBuilder(
    std::shared_ptr<DataAbilityOperationBuilder> &builder, const int type, const std::shared_ptr<Uri> &uri)
{
    HILOG_INFO("%{public}s called.", __func__);
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
            HILOG_ERROR("%{public}s, type:%{public}d is invalid.", __func__, type);
            return false;
    }
    return true;
}

napi_value UnwrapValuesBucket(const std::shared_ptr<NativeRdb::ValuesBucket> &param, napi_env env,
    napi_value valueBucketParam)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value result;

    if (param == nullptr) {
        HILOG_INFO("%{public}s input param is nullptr.", __func__);
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
    HILOG_INFO("%{public}s called.", __func__);

    if (!IsTypeForNapiValue(env, predicatesBackReferencesParam, napi_object)) {
        HILOG_ERROR("%{public}s, predicatesBackReferencesParam is invalid.", __func__);
        return nullptr;
    }

    napi_valuetype jsValueType = napi_undefined;
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;

    NAPI_CALL(env, napi_get_property_names(env, predicatesBackReferencesParam, &jsProNameList));
    NAPI_CALL(env, napi_get_array_length(env, jsProNameList, &jsProCount));
    HILOG_INFO("%{public}s, Property size=%{public}d.", __func__, jsProCount);

    napi_value jsProName = nullptr;
    napi_value jsProValue = nullptr;
    for (uint32_t index = 0; index < jsProCount; index++) {
        NAPI_CALL(env, napi_get_element(env, jsProNameList, index, &jsProName));
        std::string strProName = UnwrapStringFromJS(env, jsProName);
        int intProName = std::atoi(strProName.c_str());
        HILOG_INFO("%{public}s, Property name=%{public}d.", __func__, intProName);
        NAPI_CALL(env, napi_get_property(env, predicatesBackReferencesParam, jsProName, &jsProValue));
        NAPI_CALL(env, napi_typeof(env, jsProValue, &jsValueType));
        int32_t natValue32 = 0;
        if (napi_get_value_int32(env, jsProValue, &natValue32) == napi_ok) {
            HILOG_INFO("%{public}s, Property value=%{public}d.", __func__, natValue32);
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

void UnwrapValuesBucket(NativeEngine& engine, NativeValue* jsParam, std::shared_ptr<NativeRdb::ValuesBucket> &param)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (param == nullptr || jsParam == nullptr || jsParam->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("%{public}s input param is error.", __func__);
        return;
    }

    AnalysisValuesBucket(engine, *jsParam, *param);
}

void UnwrapDataAbilityPredicatesBackReferences(
    NativeEngine& engine, NativeValue* jsParam, std::shared_ptr<DataAbilityOperationBuilder> &param)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (param == nullptr || jsParam == nullptr || jsParam->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("%{public}s, input params is error.", __func__);
        return;
    }

    auto object = ConvertNativeValueTo<NativeObject>(jsParam);
    if (object == nullptr) {
        HILOG_ERROR("{%{public}s Convert Native Value params error", __func__);
        return;
    }

    auto array = ConvertNativeValueTo<NativeArray>(object->GetPropertyNames());
    for (uint32_t i = 0; i < array->GetLength(); i++) {
        auto itemName = array->GetElement(i);
        std::string strName("");
        ConvertFromJsValue(engine, itemName, strName);
        auto itemValue = object->GetProperty(itemName);
        int32_t value = 0;
        if (ConvertFromJsValue(engine, itemValue, value)) {
            param->WithPredicatesBackReference(std::stoi(strName), value);
        }
    }
}

NativeValue* BuildDataAbilityOperation(
    NativeEngine &engine, NativeValue &jsParam, std::shared_ptr<DataAbilityOperation> &dataAbilityOperation)
{
    HILOG_DEBUG("called.");
    auto object = ConvertNativeValueTo<NativeObject>(&jsParam);
    // get uri
    std::string uriStr("");
    if (!object->HasProperty("uri")) {
        HILOG_ERROR("param no uri.");
        return nullptr;
    }
    NativeValue *jsUri = object->GetProperty("uri");
    if (jsUri->TypeOf() != NativeValueType::NATIVE_STRING) {
        HILOG_ERROR("uri type is error.");
        return nullptr;
    }
    if (!ConvertFromJsValue(engine, jsUri, uriStr)) {
        HILOG_ERROR("convert from JsValue failed.");
        return nullptr;
    }

    // get type
    int32_t type = 0;
    if (!object->HasProperty("type")) {
        HILOG_ERROR("param no type.");
        return nullptr;
    }
    NativeValue *jsType = object->GetProperty("type");
    if (jsType->TypeOf() != NativeValueType::NATIVE_NUMBER) {
        HILOG_ERROR("param type is error.");
        return nullptr;
    }
    if (!ConvertFromJsValue(engine, jsType, type)) {
        HILOG_ERROR("convert from JsValue failed.");
        return nullptr;
    }

    // carate builder
    HILOG_DEBUG("GetDataAbilityOperationBuilder, type=%{public}d, uri=%{public}s", type, uriStr.c_str());
    std::shared_ptr<DataAbilityOperationBuilder> builder = nullptr;
    if (!GetDataAbilityOperationBuilder(builder, type, std::make_shared<Uri>(uriStr))) {
        HILOG_ERROR("GetDataAbilityOperationBuilder failed.");
        return nullptr;
    }

    // get expectedCount if it has
    int32_t expectedCount = 0;
    if (object->HasProperty("expectedCount")) {
        NativeValue *jsExpectedCount = object->GetProperty("expectedCount");
        if (jsExpectedCount->TypeOf() == NativeValueType::NATIVE_NUMBER) {
            ConvertFromJsValue(engine, jsExpectedCount, expectedCount);
        }
    }
    if (expectedCount > 0) {
        builder->WithExpectedCount(expectedCount);
    }

    // get interrupted if it has
    bool interrupted = false;
    if (object->HasProperty("interrupted")) {
        NativeValue *jsInterrupted = object->GetProperty("interrupted");
        if (jsInterrupted->TypeOf() == NativeValueType::NATIVE_BOOLEAN) {
            ConvertFromJsValue(engine, jsInterrupted, interrupted);
        }
    }
    builder->WithInterruptionAllowed(interrupted);

    // get valuesBucket if it has
    auto valuesBucket = std::make_shared<NativeRdb::ValuesBucket>();
    valuesBucket->Clear();
    if (object->HasProperty("valuesBucket")) {
        NativeValue *jsValuesBucket = object->GetProperty("valuesBucket");
        if (jsValuesBucket->TypeOf() == NativeValueType::NATIVE_OBJECT) {
            UnwrapValuesBucket(engine, jsValuesBucket, valuesBucket);
        }
    }
    builder->WithValuesBucket(valuesBucket);

    // get valueBackReferences if it has
    auto backReferences = std::make_shared<NativeRdb::ValuesBucket>();
    backReferences->Clear();
    if (object->HasProperty("valueBackReferences")) {
        NativeValue *jsBackReferences = object->GetProperty("valueBackReferences");
        if (jsBackReferences->TypeOf() == NativeValueType::NATIVE_OBJECT) {
            UnwrapValuesBucket(engine, jsBackReferences, backReferences);
        }
    }
    builder->WithValueBackReferences(backReferences);

    // get predicates if it has
    auto predicates = std::make_shared<NativeRdb::DataAbilityPredicates>();
    if (object->HasProperty("predicates")) {
        NativeValue *jsPredicates = object->GetProperty("predicates");
        if (jsPredicates->TypeOf() == NativeValueType::NATIVE_OBJECT) {
            UnwrapDataAbilityPredicates(engine, jsPredicates, predicates);
        }
    }
    builder->WithPredicates(predicates);

    // get predicatesBackReferences if it has
    if (object->HasProperty("PredicatesBackReferences")) {
        NativeValue *jsPredicatesBackReferences = object->GetProperty("PredicatesBackReferences");
        if (jsPredicatesBackReferences->TypeOf() == NativeValueType::NATIVE_OBJECT) {
            UnwrapDataAbilityPredicatesBackReferences(engine, jsPredicatesBackReferences, builder);
        }
    }

    if (builder != nullptr) {
        HILOG_DEBUG("builder is not nullptr");
        dataAbilityOperation = builder->Build();
    }
    return CreateJsValue(engine, 1);
}

NativeValue* UnwrapDataAbilityOperation(
    NativeEngine &engine, NativeValue *jsParam, std::shared_ptr<DataAbilityOperation> &operation)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (jsParam == nullptr || jsParam->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("%{public}s, input params error.", __func__);
        return nullptr;
    }

    return BuildDataAbilityOperation(engine, *jsParam, operation);
}
}  // namespace AppExecFwk
}  // namespace OHOS
