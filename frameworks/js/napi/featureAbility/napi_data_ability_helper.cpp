/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "napi_data_ability_helper.h"

#include <cstring>
#include <uv.h>
#include <vector>
#include <string>

#include "data_ability_helper.h"
#include "data_ability_observer_interface.h"
#include "uri.h"
#include "../inner/napi_common/napi_common_ability.h"
#include "data_ability_operation.h"
#include "data_ability_result.h"
#include "js_runtime_utils.h"
#include "js_napi_common_ability.h"
#include "hilog_wrapper.h"
#include "message_parcel.h"
#include "napi_base_context.h"
#include "napi_data_ability_operation.h"
#include "napi_data_ability_predicates.h"
#include "napi_rdb_predicates.h"
#include "napi_result_set.h"
#include "securec.h"

#ifndef SUPPORT_GRAPHICS
#define DBL_MIN ((double)2.22507385850720138309e-308L)
#define DBL_MAX ((double)2.22507385850720138309e-308L)
#endif

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AppExecFwk {
std::vector<DAHelperOnOffCB *> g_registerInstances;

void AnalysisValuesBucket(NativeRdb::ValuesBucket &valuesBucket, const napi_env &env, const napi_value &arg)
{
    napi_value keys = nullptr;
    napi_get_property_names(env, arg, &keys);
    uint32_t arrLen = 0;
    napi_status status = napi_get_array_length(env, keys, &arrLen);
    if (status != napi_ok) {
        HILOG_ERROR("ValuesBucket errr");
        return;
    }
    HILOG_INFO("ValuesBucket num:%{public}d ", arrLen);
    for (size_t i = 0; i < arrLen; ++i) {
        napi_value key = nullptr;
        (void)napi_get_element(env, keys, i, &key);
        std::string keyStr = UnwrapStringFromJS(env, key);
        napi_value value = nullptr;
        napi_get_property(env, arg, key, &value);

        SetValuesBucketObject(valuesBucket, env, keyStr, value);
    }
}

void SetValuesBucketObject(
    NativeRdb::ValuesBucket &valuesBucket, const napi_env &env, std::string keyStr, napi_value value)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType == napi_string) {
        std::string valueString = UnwrapStringFromJS(env, value);
        HILOG_INFO("ValueObject type:%{public}d, key:%{public}s, value:%{private}s",
            valueType,
            keyStr.c_str(),
            valueString.c_str());
        valuesBucket.PutString(keyStr, valueString);
    } else if (valueType == napi_number) {
        double valueNumber = 0;
        napi_get_value_double(env, value, &valueNumber);
        valuesBucket.PutDouble(keyStr, valueNumber);
        HILOG_INFO(
            "ValueObject type:%{public}d, key:%{public}s, value:%{private}lf", valueType, keyStr.c_str(), valueNumber);
    } else if (valueType == napi_boolean) {
        bool valueBool = false;
        napi_get_value_bool(env, value, &valueBool);
        HILOG_INFO(
            "ValueObject type:%{public}d, key:%{public}s, value:%{private}d", valueType, keyStr.c_str(), valueBool);
        valuesBucket.PutBool(keyStr, valueBool);
    } else if (valueType == napi_null) {
        valuesBucket.PutNull(keyStr);
        HILOG_INFO("ValueObject type:%{public}d, key:%{public}s, value:null", valueType, keyStr.c_str());
    } else if (valueType == napi_object) {
        HILOG_INFO("ValueObject type:%{public}d, key:%{public}s, value:Uint8Array", valueType, keyStr.c_str());
        valuesBucket.PutBlob(keyStr, ConvertU8Vector(env, value));
    } else {
        HILOG_ERROR("valuesBucket error");
    }
}

void FindRegisterObs(DAHelperOnOffCB *data)
{
    HILOG_INFO("FindRegisterObs main event thread execute.");
    if (data == nullptr || data->dataAbilityHelper == nullptr) {
        HILOG_ERROR("NAPI_UnRegister, param is null.");
        return;
    }

    HILOG_INFO("NAPI_UnRegister, uri=%{public}s.", data->uri.c_str());
    if (!data->uri.empty()) {
        // if match uri, unregister all observers corresponding the uri
        std::string strUri = data->uri;
        auto iter = g_registerInstances.begin();
        while (iter != g_registerInstances.end()) {
            DAHelperOnOffCB *helper = *iter;
            if (helper == nullptr || helper->uri != strUri) {
                iter++;
                continue;
            }
            data->NotifyList.emplace_back(helper);
            iter = g_registerInstances.erase(iter);
            HILOG_INFO("NAPI_UnRegister Instances erase size = %{public}zu", g_registerInstances.size());
        }
    } else {
        HILOG_ERROR("NAPI_UnRegister, error: uri is null.");
    }
    HILOG_INFO("NAPI_UnRegister, FindRegisterObs main event thread execute.end %{public}zu", data->NotifyList.size());
}

void NAPIDataAbilityObserver::ReleaseJSCallback()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (nativeRef_ == nullptr) {
        HILOG_ERROR("NAPIDataAbilityObserver::ReleaseJSCallback, ref_ is null.");
        return;
    }

    if (isCallingback_) {
        needRelease_ = true;
        HILOG_WARN("%{public}s, ref_ is calling back.", __func__);
        return;
    }

    SafeReleaseJSCallback();
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called. end", __func__);
}

void NAPIDataAbilityObserver::SafeReleaseJSCallback()
{
    if (engine_ == nullptr) {
        HILOG_ERROR("NAPIDataAbilityObserver::SafeReleaseJSCallback, engine_ is null.");
        return;
    }
    uv_loop_t *loop = nullptr;
    loop = engine_->GetUVLoop();
    if (loop == nullptr) {
        HILOG_ERROR("%{public}s, loop == nullptr.", __func__);
        return;
    }

    struct DelRefCallbackInfo {
        NativeEngine *engine_ = nullptr;
        NativeReference *nativeRef_ = nullptr;
    };

    DelRefCallbackInfo *delRefCallbackInfo = new DelRefCallbackInfo {
        .engine_ = engine_,
        .nativeRef_ = nativeRef_,
    };

    uv_work_t *work = new uv_work_t;
    work->data = static_cast<void*>(delRefCallbackInfo);
    int ret = uv_queue_work(
        loop, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            // JS Thread
            if (work == nullptr) {
                HILOG_ERROR("uv_queue_work input work is nullptr");
                return;
            }
            auto delRefCallbackInfo =  reinterpret_cast<DelRefCallbackInfo*>(work->data);
            if (delRefCallbackInfo == nullptr) {
                HILOG_ERROR("uv_queue_work delRefCallbackInfo is nullptr");
                delete work;
                work = nullptr;
                return;
            }

            delete delRefCallbackInfo->nativeRef_;
            delRefCallbackInfo->nativeRef_ = nullptr;
            delete delRefCallbackInfo;
            delRefCallbackInfo = nullptr;
            delete work;
            work = nullptr;
        });
    if (ret != 0) {
        if (delRefCallbackInfo != nullptr) {
            delete delRefCallbackInfo;
            delRefCallbackInfo = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
    nativeRef_ = nullptr;
}

void NAPIDataAbilityObserver::SetEnv(NativeEngine *engine)
{
    engine_ = engine;
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called. end", __func__);
}

void NAPIDataAbilityObserver::SetCallbackRef(NativeReference *ref)
{
    nativeRef_ = ref;
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called. end", __func__);
}

static void OnChangeJSThreadWorker(uv_work_t *work, int status)
{
    HILOG_INFO("OnChange, uv_queue_work");
    if (work == nullptr) {
        HILOG_ERROR("OnChange, uv_queue_work input work is nullptr");
        return;
    }
    DAHelperOnOffCB *onCB = (DAHelperOnOffCB *)work->data;
    if (onCB == nullptr) {
        HILOG_ERROR("OnChange, uv_queue_work onCB is nullptr");
        delete work;
        work = nullptr;
        return;
    }

    if (onCB->observer != nullptr) {
        onCB->observer->CallJsMethod();
    }

    delete onCB;
    onCB = nullptr;
    delete work;
    work = nullptr;
    HILOG_INFO("OnChange, uv_queue_work. end");
}

void NAPIDataAbilityObserver::CallJsMethod()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (nativeRef_ == nullptr || engine_ == nullptr) {
            HILOG_WARN("%{public}s observer is invalid.", __func__);
            return;
        }
        isCallingback_ = true;
    }
    NativeValue *result[ARGS_TWO] = {0};

    NativeValue *objValue = engine_->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);

    object->SetProperty("code", CreateJsValue(*(engine_), NO_ERROR));
    result[PARAM0] = objValue;

    NativeValue *value = nativeRef_->Get();
    NativeValue *callback = nativeRef_->Get();
    engine_->CallFunction(value, callback, result, ARGS_TWO);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (needRelease_ && nativeRef_ != nullptr) {
            HILOG_INFO("%{public}s to delete callback.", __func__);
            delete nativeRef_;
            nativeRef_ = nullptr;
            needRelease_ = false;
        }
        isCallingback_ = false;
    }
}

void NAPIDataAbilityObserver::OnChange()
{
    if (nativeRef_ == nullptr) {
        HILOG_ERROR("OnChange ref is nullptr.");
        return;
    }
    if (engine_ == nullptr) {
        HILOG_ERROR("OnChange engine_ is nullptr.");
        return;
    }

    uv_loop_t *loop = nullptr;
    loop = engine_->GetUVLoop();
    if (loop == nullptr) {
        HILOG_ERROR("OnChange loop is nullptr.");
        return;
    }

    uv_work_t *work = new uv_work_t;
    DAHelperOnOffCB *onCB = new DAHelperOnOffCB;
    onCB->observer = this;
    work->data = static_cast<void *>(onCB);
    int rev = uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        OnChangeJSThreadWorker);
    if (rev != 0) {
        if (onCB != nullptr) {
            delete onCB;
            onCB = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
    HILOG_INFO("%{public}s, called. end", __func__);
}

void UnwrapDataAbilityPredicates(NativeRdb::DataAbilityPredicates &predicates, napi_env env, napi_value value)
{
    auto tempPredicates = DataAbilityJsKit::DataAbilityPredicatesProxy::GetNativePredicates(env, value);
    if (tempPredicates == nullptr) {
        HILOG_ERROR("%{public}s, GetNativePredicates retval Marshalling failed.", __func__);
        return;
    }
    predicates = *tempPredicates;
}

static std::string ExcludeTag(const std::string& jsonString, const std::string& tagString)
{
    size_t pos = jsonString.find(tagString);
    if (pos == std::string::npos) {
        return jsonString;
    }
    std::string valueString = jsonString.substr(pos);
    pos = valueString.find(":");
    if (pos == std::string::npos) {
        return "";
    }
    size_t valuePos = pos + 1;
    while (valuePos < valueString.size()) {
        if (valueString.at(valuePos) != ' ' && valueString.at(valuePos) != '\t') {
            break;
        }
        valuePos++;
    }
    if (valuePos >= valueString.size()) {
        return "";
    }
    valueString = valueString.substr(valuePos);
    return valueString.substr(0, valueString.size() - 1);
}

std::vector<NativeRdb::ValuesBucket> NapiValueObject(napi_env env, napi_value param)
{
    HILOG_INFO("%{public}s,called", __func__);
    std::vector<NativeRdb::ValuesBucket> result;
    UnwrapArrayObjectFromJS(env, param, result);
    return result;
}

bool UnwrapArrayObjectFromJS(napi_env env, napi_value param, std::vector<NativeRdb::ValuesBucket> &value)
{
    HILOG_INFO("%{public}s,called", __func__);
    uint32_t arraySize = 0;
    napi_value jsValue = nullptr;
    std::string strValue = "";

    if (!IsArrayForNapiValue(env, param, arraySize)) {
        HILOG_INFO("%{public}s, IsArrayForNapiValue is false", __func__);
        return false;
    }

    value.clear();
    for (uint32_t i = 0; i < arraySize; i++) {
        jsValue = nullptr;
        if (napi_get_element(env, param, i, &jsValue) != napi_ok) {
            HILOG_INFO("%{public}s, napi_get_element is false", __func__);
            return false;
        }

        NativeRdb::ValuesBucket valueBucket;
        valueBucket.Clear();
        AnalysisValuesBucket(valueBucket, env, jsValue);

        value.push_back(valueBucket);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return true;
}

void EraseMemberProperties(DAHelperOnOffCB* onCB)
{
    HILOG_DEBUG("called");
    if (onCB->observer) {
        HILOG_DEBUG("EraseMemberProperties, call ReleaseJSCallback");
        onCB->observer->ReleaseJSCallback();
        onCB->observer = nullptr;
    }
    if (onCB->dataAbilityHelper) {
        HILOG_DEBUG("EraseMemberProperties, call Release");
        onCB->dataAbilityHelper->Release();
        onCB->dataAbilityHelper = nullptr;
    }
}

bool NeedErase(std::vector<DAHelperOnOffCB*>::iterator& iter, const std::shared_ptr<DataAbilityHelper> &objectInfo)
{
    if ((*iter) == nullptr) {
        return false;
    }
    if ((*iter)->dataAbilityHelper == objectInfo) {
        EraseMemberProperties(*iter);
        delete (*iter);
        (*iter) = nullptr;
        iter = g_registerInstances.erase(iter);
    } else {
        ++iter;
    }
    return true;
}

void DeleteDAHelperOnOffCB(DAHelperOnOffCB *onCB)
{
    HILOG_DEBUG("called");
    if (!onCB) {
        HILOG_DEBUG("DeleteDAHelperOnOffCB, onCB is nullptr, no need delete");
        return;
    }
    EraseMemberProperties(onCB);

    auto end = remove(g_registerInstances.begin(), g_registerInstances.end(), onCB);
    if (end != g_registerInstances.end()) {
        (void)g_registerInstances.erase(end);
    }
    delete onCB;
    onCB = nullptr;
}

void UnwrapDataAbilityPredicates(
    NativeEngine &engine, NativeValue* jsParam, std::shared_ptr<NativeRdb::DataAbilityPredicates> &predicates)
{
    if (jsParam == nullptr || predicates == nullptr) {
        HILOG_ERROR("input params error.");
        return;
    }

    auto tempPredicates =
        DataAbilityJsKit::DataAbilityPredicatesProxy::GetNativePredicates(
            reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(jsParam));
    if (tempPredicates == nullptr) {
        HILOG_ERROR("GetNativePredicates retval Marshalling failed.");
        return;
    }

    *predicates = *tempPredicates;
}

void SetValuesBucketObject(
    NativeEngine &engine, NativeValue* jsParam, std::string &keyStr, NativeRdb::ValuesBucket &valuesBucket)
{
    if (jsParam == nullptr) {
        HILOG_ERROR("input params error");
        return;
    }

    auto paramType = jsParam->TypeOf();
    if (paramType == NativeValueType::NATIVE_STRING) {
        std::string valueStr("");
        if (!ConvertFromJsValue(engine, jsParam, valueStr)) {
            HILOG_ERROR("valuesBucket string error");
            return;
        }
        HILOG_DEBUG("ValueObject type:%{public}d, key:%{public}s, value:%{private}s",
            paramType,
            keyStr.c_str(),
            valueStr.c_str());
        valuesBucket.PutString(keyStr, valueStr);
    } else if (paramType == NativeValueType::NATIVE_NUMBER) {
        double valueNum = 0.0;
        if (!ConvertFromJsValue(engine, jsParam, valueNum)) {
            HILOG_ERROR("valuesBucket number error");
            return;
        }
        HILOG_DEBUG("ValueObject type:%{public}d, key:%{public}s, value:%{private}lf",
            paramType,
            keyStr.c_str(),
            valueNum);
        valuesBucket.PutDouble(keyStr, valueNum);
    } else if (paramType == NativeValueType::NATIVE_BOOLEAN) {
        bool valueBool = false;
        if (!ConvertFromJsValue(engine, jsParam, valueBool)) {
            HILOG_ERROR("valuesBucket bool error");
            return;
        }
        HILOG_DEBUG("ValueObject type:%{public}d, key:%{public}s, value:%{private}d",
            paramType,
            keyStr.c_str(),
            valueBool);
        valuesBucket.PutBool(keyStr, valueBool);
    } else if (paramType == NativeValueType::NATIVE_NULL) {
        valuesBucket.PutNull(keyStr);
        HILOG_DEBUG("ValueObject type:%{public}d, key:%{public}s, value:null", paramType, keyStr.c_str());
    } else if (paramType == NativeValueType::NATIVE_OBJECT) {
        valuesBucket.PutBlob(keyStr, ConvertU8Vector(engine, jsParam));
        HILOG_DEBUG("ValueObject type:%{public}d, key:%{public}s, value:Uint8Array", paramType, keyStr.c_str());
    } else {
        HILOG_ERROR("valuesBucket error");
    }
}

void AnalysisValuesBucket(NativeEngine &engine, NativeValue &jsParam, NativeRdb::ValuesBucket &valuesBucket)
{
    auto object = ConvertNativeValueTo<NativeObject>(&jsParam);
    if (object == nullptr) {
        HILOG_ERROR("convert native value error");
        return;
    }

    auto array = ConvertNativeValueTo<NativeArray>(object->GetPropertyNames());
    for (uint32_t i = 0; i < array->GetLength(); i++) {
        auto itemKey = array->GetElement(i);
        std::string strKey("");
        if (!ConvertFromJsValue(engine, itemKey, strKey)) {
            HILOG_ERROR("convert native value error");
            return;
        }

        SetValuesBucketObject(engine, object->GetProperty(itemKey), strKey, valuesBucket);
    }
}

NativeValue* CreateJsDataAbilityHelper(NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s is called", __FUNCTION__);
    auto objValue = engine.CreateObject();
    if (objValue == nullptr) {
        HILOG_ERROR("CreateJsDataAbilityHelper failed");
        return nullptr;
    }
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = nullptr;
    bool stageMode = false;
    auto env = reinterpret_cast<napi_env>(&engine);
    auto arg0 = reinterpret_cast<napi_value>(info.argv[0]);
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, arg0, stageMode);
    if (status != napi_ok) {
        HILOG_DEBUG("argv[0] is not a context");
        auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
        if (ability == nullptr) {
            HILOG_ERROR("Failed to get native context instance");
            return nullptr;
        }
        std::string strUri;
        ConvertFromJsValue(engine, info.argv[PARAM0], strUri);
        HILOG_DEBUG("FA Model: strUri = %{public}s", strUri.c_str());
        dataAbilityHelper = DataAbilityHelper::Creator(ability->GetContext(), std::make_shared<Uri>(strUri));
    } else {
        HILOG_DEBUG("argv[0] is a context");
        if (stageMode) {
            auto context = OHOS::AbilityRuntime::GetStageModeContext(env, arg0);
            if (context == nullptr) {
                HILOG_ERROR("Failed to get native context instance");
                return nullptr;
            }
            std::string strUri;
            ConvertFromJsValue(engine, info.argv[PARAM1], strUri);
            HILOG_DEBUG("Stage Model: strUri = %{public}s", strUri.c_str());
            dataAbilityHelper = DataAbilityHelper::Creator(context, std::make_shared<Uri>(strUri));
        } else {
            auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
            if (ability == nullptr) {
                HILOG_ERROR("Failed to get native context instance");
                return nullptr;
            }
            std::string strUri;
            ConvertFromJsValue(engine, info.argv[PARAM1], strUri);
            HILOG_DEBUG("FA Model: strUri = %{public}s", strUri.c_str());
            dataAbilityHelper = DataAbilityHelper::Creator(ability->GetContext(), std::make_shared<Uri>(strUri));
        }
    }

    if (dataAbilityHelper == nullptr) {
        HILOG_ERROR("%{public}s, dataAbilityHelper is nullptr", __func__);
        return nullptr;
    }
    dataAbilityHelper->SetCallFromJs();

    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("invalid object.");
        return objValue;
    }

    std::unique_ptr<JsDataAbilityHelper> jsDataAbilityHelper = std::make_unique<JsDataAbilityHelper>(dataAbilityHelper);
    object->SetNativePointer(jsDataAbilityHelper.release(), JsDataAbilityHelper::Finalizer, nullptr);

    HILOG_DEBUG("JsDataAbilityHelper BindNativeFunction called");
    const char *moduleName = "JsDataAbilityHelper";
    BindNativeFunction(engine, *object, "insert", moduleName, JsDataAbilityHelper::JsInsert);
    BindNativeFunction(engine, *object, "notifyChange", moduleName, JsDataAbilityHelper::JsNotifyChange);
    BindNativeFunction(engine, *object, "on", moduleName, JsDataAbilityHelper::JsRegister);
    BindNativeFunction(engine, *object, "off", moduleName, JsDataAbilityHelper::JsUnRegister);
    BindNativeFunction(engine, *object, "delete", moduleName, JsDataAbilityHelper::JsDelete);
    BindNativeFunction(engine, *object, "query", moduleName, JsDataAbilityHelper::JsQuery);
    BindNativeFunction(engine, *object, "update", moduleName, JsDataAbilityHelper::JsUpdate);
    BindNativeFunction(engine, *object, "batchInsert", moduleName, JsDataAbilityHelper::JsBatchInsert);
    BindNativeFunction(engine, *object, "openFile", moduleName, JsDataAbilityHelper::JsOpenFile);
    BindNativeFunction(engine, *object, "getType", moduleName, JsDataAbilityHelper::JsGetType);
    BindNativeFunction(engine, *object, "getFileTypes", moduleName, JsDataAbilityHelper::JsGetFileTypes);
    BindNativeFunction(engine, *object, "normalizeUri", moduleName, JsDataAbilityHelper::JsNormalizeUri);
    BindNativeFunction(engine, *object, "denormalizeUri", moduleName, JsDataAbilityHelper::JsDenormalizeUri);
    BindNativeFunction(engine, *object, "executeBatch", moduleName, JsDataAbilityHelper::JsExecuteBatch);
    BindNativeFunction(engine, *object, "call", moduleName, JsDataAbilityHelper::JsCall);

    HILOG_DEBUG("JsDataAbilityHelper end");
    return objValue;
}

void JsDataAbilityHelper::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_DEBUG("JsDataAbilityHelper::Finalizer is called");
    auto jsDataAbilityHelper = std::unique_ptr<JsDataAbilityHelper>(static_cast<JsDataAbilityHelper*>(data));
    HILOG_DEBUG("DAHelper finalize_cb regInstances_.size = %{public}zu", g_registerInstances.size());
    for (auto iter = g_registerInstances.begin(); iter != g_registerInstances.end();) {
        if (!NeedErase(iter, jsDataAbilityHelper->GetDataAbilityHelper())) {
            iter = g_registerInstances.erase(iter);
        }
    }
    HILOG_DEBUG("DAHelper finalize_cb regInstances_.size = %{public}zu", g_registerInstances.size());
}

NativeValue* JsDataAbilityHelper::JsInsert(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("input parameters %{public}s is nullptr", ((engine == nullptr) ? "engine" : "info"));
        return nullptr;
    }
    auto object = CheckParamsAndGetThis<JsDataAbilityHelper>(engine, info);
    if (object == nullptr) {
        HILOG_ERROR("CheckParamsAndGetThis return nullptr");
        return nullptr;
    }
    return object->OnInsert(*engine, *info);
}

NativeValue* JsDataAbilityHelper::JsNotifyChange(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("input parameters %{public}s is nullptr", ((engine == nullptr) ? "engine" : "info"));
        return nullptr;
    }
    auto object = CheckParamsAndGetThis<JsDataAbilityHelper>(engine, info);
    if (object == nullptr) {
        HILOG_ERROR("CheckParamsAndGetThis return nullptr");
        return nullptr;
    }
    return object->OnNotifyChange(*engine, *info);
}

NativeValue* JsDataAbilityHelper::JsRegister(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("input parameters %{public}s is nullptr", ((engine == nullptr) ? "engine" : "info"));
        return nullptr;
    }
    auto object = CheckParamsAndGetThis<JsDataAbilityHelper>(engine, info);
    if (object == nullptr) {
        HILOG_ERROR("CheckParamsAndGetThis return nullptr");
        return nullptr;
    }
    return object->OnRegister(*engine, *info);
};

NativeValue* JsDataAbilityHelper::JsUnRegister(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("input parameters %{public}s is nullptr", ((engine == nullptr) ? "engine" : "info"));
        return nullptr;
    }
    auto object = CheckParamsAndGetThis<JsDataAbilityHelper>(engine, info);
    if (object == nullptr) {
        HILOG_ERROR("CheckParamsAndGetThis return nullptr");
        return nullptr;
    }
    return object->OnUnRegister(*engine, *info);
};

NativeValue* JsDataAbilityHelper::JsDelete(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("input parameters %{public}s is nullptr", ((engine == nullptr) ? "engine" : "info"));
        return nullptr;
    }
    auto object = CheckParamsAndGetThis<JsDataAbilityHelper>(engine, info);
    if (object == nullptr) {
        HILOG_ERROR("CheckParamsAndGetThis return nullptr");
        return nullptr;
    }
    return object->OnDelete(*engine, *info);
}

NativeValue* JsDataAbilityHelper::JsQuery(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("input parameters %{public}s is nullptr", ((engine == nullptr) ? "engine" : "info"));
        return nullptr;
    }
    auto object = CheckParamsAndGetThis<JsDataAbilityHelper>(engine, info);
    if (object == nullptr) {
        HILOG_ERROR("CheckParamsAndGetThis return nullptr");
        return nullptr;
    }
    return object->OnQuery(*engine, *info);
}

NativeValue* JsDataAbilityHelper::JsUpdate(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("input parameters %{public}s is nullptr", ((engine == nullptr) ? "engine" : "info"));
        return nullptr;
    }
    auto object = CheckParamsAndGetThis<JsDataAbilityHelper>(engine, info);
    if (object == nullptr) {
        HILOG_ERROR("CheckParamsAndGetThis return nullptr");
        return nullptr;
    }
    return object->OnUpdate(*engine, *info);
}

NativeValue* JsDataAbilityHelper::JsBatchInsert(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("input parameters %{public}s is nullptr", ((engine == nullptr) ? "engine" : "info"));
        return nullptr;
    }
    auto object = CheckParamsAndGetThis<JsDataAbilityHelper>(engine, info);
    if (object == nullptr) {
        HILOG_ERROR("CheckParamsAndGetThis return nullptr");
        return nullptr;
    }
    return object->OnBatchInsert(*engine, *info);
}

NativeValue* JsDataAbilityHelper::JsOpenFile(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("input parameters %{public}s is nullptr", ((engine == nullptr) ? "engine" : "info"));
        return nullptr;
    }
    auto object = CheckParamsAndGetThis<JsDataAbilityHelper>(engine, info);
    if (object == nullptr) {
        HILOG_ERROR("CheckParamsAndGetThis return nullptr");
        return nullptr;
    }
    return object->OnOpenFile(*engine, *info);
}

NativeValue* JsDataAbilityHelper::JsGetType(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("input parameters %{public}s is nullptr", ((engine == nullptr) ? "engine" : "info"));
        return nullptr;
    }
    auto object = CheckParamsAndGetThis<JsDataAbilityHelper>(engine, info);
    if (object == nullptr) {
        HILOG_ERROR("CheckParamsAndGetThis return nullptr");
        return nullptr;
    }
    return object->OnGetType(*engine, *info);
}

NativeValue* JsDataAbilityHelper::JsGetFileTypes(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("input parameters %{public}s is nullptr", ((engine == nullptr) ? "engine" : "info"));
        return nullptr;
    }
    auto object = CheckParamsAndGetThis<JsDataAbilityHelper>(engine, info);
    if (object == nullptr) {
        HILOG_ERROR("CheckParamsAndGetThis return nullptr");
        return nullptr;
    }
    return object->OnGetFileTypes(*engine, *info);
}

NativeValue* JsDataAbilityHelper::JsNormalizeUri(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("input parameters %{public}s is nullptr", ((engine == nullptr) ? "engine" : "info"));
        return nullptr;
    }

    auto object = CheckParamsAndGetThis<JsDataAbilityHelper>(engine, info);
    if (object == nullptr) {
        HILOG_ERROR("CheckParamsAndGetThis return nullptr");
        return engine->CreateUndefined();
    }

    return object->OnNormalizeUri(*engine, *info);
}

NativeValue* JsDataAbilityHelper::JsDenormalizeUri(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("input parameters %{public}s is nullptr", ((engine == nullptr) ? "engine" : "info"));
        return nullptr;
    }

    auto object = CheckParamsAndGetThis<JsDataAbilityHelper>(engine, info);
    if (object == nullptr) {
        HILOG_ERROR("CheckParamsAndGetThis return nullptr");
        return engine->CreateUndefined();
    }

    return object->OnDenormalizeUri(*engine, *info);
}

NativeValue* JsDataAbilityHelper::JsExecuteBatch(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("input parameters %{public}s is nullptr", ((engine == nullptr) ? "engine" : "info"));
        return nullptr;
    }

    auto object = CheckParamsAndGetThis<JsDataAbilityHelper>(engine, info);
    if (object == nullptr) {
        HILOG_ERROR("CheckParamsAndGetThis return nullptr");
        return engine->CreateUndefined();
    }

    return object->OnExecuteBatch(*engine, *info);
}

NativeValue* JsDataAbilityHelper::JsCall(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("input parameters %{public}s is nullptr", ((engine == nullptr) ? "engine" : "info"));
        return nullptr;
    }

    auto object = CheckParamsAndGetThis<JsDataAbilityHelper>(engine, info);
    if (object == nullptr) {
        HILOG_ERROR("CheckParamsAndGetThis return nullptr");
        return engine->CreateUndefined();
    }

    return object->OnCall(*engine, *info);
}

NativeValue* JsDataAbilityHelper::OnInsert(NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s called", __func__);
    if (info.argc < ARGS_TWO || info.argc > ARGS_THREE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }
    std::shared_ptr<JsInsertData> data = std::make_shared<JsInsertData>();

    if (info.argv[PARAM0]->TypeOf() == NativeValueType::NATIVE_STRING) {
        ConvertFromJsValue(engine, info.argv[PARAM0], data->uri);
        HILOG_DEBUG("uri=%{public}s", data->uri.c_str());
    }

    data->valueBucket.Clear();
    AnalysisValuesBucket(data->valueBucket, reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[PARAM1]));
    auto execute = [dataAbilityHelper = dataAbilityHelper_, getData = data] () {
        if (dataAbilityHelper == nullptr) {
            HILOG_ERROR("task execute error, the dataAbilityHelper is nullptr");
            return;
        }
        if (getData == nullptr) {
            HILOG_ERROR("task execute error, getData is nullptr");
            return;
        }
        if (getData->uri.empty()) {
            HILOG_ERROR("task execute error, uri is emptynullptr");
            getData->errorVal = INVALID_PARAMETER;
            return;
        }
        OHOS::Uri uriValue(getData->uri);
        getData->result = dataAbilityHelper->Insert(uriValue, getData->valueBucket);
    };
    auto complete = [getData = data] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        task.ResolveWithCustomize(engine, CreateJsError(engine, getData->errorVal),
            CreateJsValue(engine, getData->result));
    };
    auto callback = (info.argc == ARGS_THREE) ? info.argv[PARAM2] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsDataAbilityHelper::OnInsert",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsDataAbilityHelper::OnNotifyChange(NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s called", __func__);
    std::shared_ptr<JsNotifyChangeData> data = std::make_shared<JsNotifyChangeData>();
    if (info.argc == ARGS_ZERO || info.argc > ARGS_TWO) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateNull();
    }
    if (info.argv[PARAM0]->TypeOf() == NativeValueType::NATIVE_STRING) {
        ConvertFromJsValue(engine, info.argv[PARAM0], data->uri);
        HILOG_DEBUG("uri=%{public}s", data->uri.c_str());
    }
    auto execute = [dataAbilityHelper = dataAbilityHelper_, getData = data] () {
        if (dataAbilityHelper == nullptr) {
            HILOG_ERROR("task execute error, the dataAbilityHelper is nullptr");
            return;
        }
        if (getData == nullptr) {
            HILOG_ERROR("task execute error, getData is nullptr");
            return;
        }
        if (getData->uri.empty()) {
            HILOG_ERROR("task execute error, uri is emptynullptr");
            getData->errorVal = INVALID_PARAMETER;
            return;
        }
        OHOS::Uri uriValue(getData->uri);
        dataAbilityHelper->NotifyChange(uriValue);
    };
    auto complete = [getData = data] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        task.ResolveWithCustomize(engine, CreateJsError(engine, getData->errorVal),
            CreateJsValue(engine, 0));
    };
    auto callback = (info.argc == ARGS_TWO) ? info.argv[PARAM1] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsDataAbilityHelper::OnNotifyChange",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsDataAbilityHelper::OnRegister(NativeEngine& engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s called", __func__);
    if (info.argc != ARGS_THREE) {
        HILOG_ERROR("Input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }
    DAHelperOnOffCB *onCB = new DAHelperOnOffCB;
    onCB->result = NO_ERROR;
    std::string firstParam = ""; 
    if ((info.argv[PARAM0])->TypeOf() == NativeValueType::NATIVE_STRING){
        if (ConvertFromJsValue(engine, info.argv[PARAM0], firstParam)) {
            if (firstParam != "dataChange") {
                HILOG_ERROR("Wrong argument firstparam is %{public}s.", firstParam.c_str());
                onCB->result = INVALID_PARAMETER;
            }
        } else {
            HILOG_ERROR("Input first param convert error");
            onCB->result = INVALID_PARAMETER;
        }
    } else {
        HILOG_ERROR("Input first param type error");
        onCB->result = INVALID_PARAMETER;
    }

    if ((info.argv[PARAM1])->TypeOf() == NativeValueType::NATIVE_STRING){
        if (!ConvertFromJsValue(engine, info.argv[PARAM1], onCB->uri)) {
            HILOG_ERROR("input second param convert error");
            onCB->result = INVALID_PARAMETER;
        }
    } else {
        HILOG_ERROR("Input second param type error");
        onCB->result = INVALID_PARAMETER;
    }
    if (info.argv[PARAM2]->IsCallable()) {
        onCB->cbBase.cbInfo.nativeRef = engine.CreateReference(info.argv[PARAM2], 1);
    } else {
        HILOG_ERROR("Input third param type error");
        onCB->result = INVALID_PARAMETER;
    }
    sptr<NAPIDataAbilityObserver> observer(new NAPIDataAbilityObserver());
    observer->SetEnv(&engine);
    observer->SetCallbackRef(onCB->cbBase.cbInfo.nativeRef);
    onCB->observer = observer;
    onCB->dataAbilityHelper = GetDataAbilityHelper();
    if (onCB->result == NO_ERROR) {
        g_registerInstances.emplace_back(onCB);
    }

    auto execute = [dataAbilityHelper = dataAbilityHelper_, data = onCB] () {
        if (dataAbilityHelper == nullptr) {
            HILOG_ERROR("dataAbilityHelper_ is nullptr.");
            data->result = ABILITY_NOT_FOUND;
            return;
        }

        if (data->result != INVALID_PARAMETER && !data->uri.empty() && data->cbBase.cbInfo.nativeRef != nullptr) {
            HILOG_DEBUG("%{public}s,uri=%{public}s", __func__, data->uri.c_str());
            OHOS::Uri uri(data->uri);
            dataAbilityHelper->RegisterObserver(uri, data->observer);
        } else {
            HILOG_ERROR("OnRegister check param failed.");
            data->result = INVALID_PARAMETER;
        }
    };
    auto complete = [data = onCB] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        auto onCBIter = std::find(g_registerInstances.begin(), g_registerInstances.end(), data);
        if (onCBIter == g_registerInstances.end()) {
            // onCB is invalid or onCB has been delete
            HILOG_ERROR("onCB is invalid.");
            return;
        }

        if (data->result != NO_ERROR) {
            HILOG_ERROR("OnRegister error delete object. result=%{public}d", data->result);
            DeleteDAHelperOnOffCB(data);
        }
        task.Resolve(engine, engine.CreateNull());
    };
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsDataAbilityHelper::OnRegister",
        engine, CreateAsyncTaskWithLastParam(engine, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsDataAbilityHelper::OnUnRegister(NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s called", __func__);
    DAHelperOnOffCB *offCB = new DAHelperOnOffCB;
    offCB->result = NAPI_ERR_NO_ERROR;
    if (info.argc < ARGS_TWO|| info.argc > ARGS_THREE) {
        HILOG_ERROR("Input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }
    std::string firstParam = "";
    if ((info.argv[PARAM0])->TypeOf() == NativeValueType::NATIVE_STRING){
        if (ConvertFromJsValue(engine, info.argv[PARAM0], firstParam)) {
            if (firstParam != "dataChange") {
                HILOG_ERROR("Wrong argument firstparam is %{public}s.", firstParam.c_str());
                offCB->result = NAPI_ERR_PARAM_INVALID;
            }
        } else {
            HILOG_ERROR("Input first param convert error");
            offCB->result = NAPI_ERR_PARAM_INVALID;
        }
    } else {
        HILOG_ERROR("Input first param type error");
        offCB->result = NAPI_ERR_PARAM_INVALID;
    }

    if (info.argc > ARGS_TWO) {
        if ((info.argv[PARAM1])->TypeOf() == NativeValueType::NATIVE_STRING){
            if (!ConvertFromJsValue(engine, info.argv[PARAM1], offCB->uri)) {
                HILOG_ERROR("input second param convert error");
                offCB->result = NAPI_ERR_PARAM_INVALID;
            }
        } else {
            HILOG_ERROR("Input second param type error");
            offCB->result = NAPI_ERR_PARAM_INVALID;
        }
    
        if (info.argv[PARAM2]->IsCallable()) {
            offCB->cbBase.cbInfo.nativeRef = engine.CreateReference(info.argv[PARAM2], 1);
        } else {
            HILOG_ERROR("Input third param type error");
            offCB->result = NAPI_ERR_PARAM_INVALID;
        }
    } else {
        if ((info.argv[PARAM1])->TypeOf() == NativeValueType::NATIVE_STRING){
            if (!ConvertFromJsValue(engine, info.argv[PARAM1], offCB->uri)) {
                HILOG_ERROR("input second param convert error");
                offCB->result = NAPI_ERR_PARAM_INVALID;
            }
        } else if (info.argv[PARAM1]->IsCallable()) {
            offCB->cbBase.cbInfo.nativeRef = engine.CreateReference(info.argv[PARAM1], 1);
        }else {
            HILOG_ERROR("Input second param type error");
            offCB->result = NAPI_ERR_PARAM_INVALID;
        }
    }
    offCB->dataAbilityHelper = GetDataAbilityHelper();

    if (offCB->result == NO_ERROR) {
        FindRegisterObs(offCB);
    }

    HILOG_DEBUG("OnUnRegister, offCB->NotifyList size is %{public}zu", offCB->NotifyList.size());
    for (auto &iter : offCB->NotifyList) {
        if (iter != nullptr && iter->observer != nullptr) {
            OHOS::Uri uri(iter->uri);
            iter->dataAbilityHelper->UnregisterObserver(uri, iter->observer);
            offCB->DestroyList.emplace_back(iter);
        }
    }
    offCB->NotifyList.clear();

    HILOG_DEBUG("OnUnRegister, offCB->DestroyList size is %{public}zu", offCB->DestroyList.size());
    for (auto &iter : offCB->DestroyList) {
        if (iter->observer != nullptr) {
            iter->observer->ReleaseJSCallback();
            delete iter;
            iter = nullptr;
            HILOG_DEBUG("OnUnRegister ReleaseJSCallback end.");
        }
    }

    offCB->DestroyList.clear();
    delete offCB;
    offCB = nullptr;
    return engine.CreateNull();
}

NativeValue* JsDataAbilityHelper::OnDelete(NativeEngine& engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s called", __func__);
    std::shared_ptr<JsDeleteData> data = std::make_shared<JsDeleteData>();
    if (info.argc < ARGS_TWO || info.argc > ARGS_THREE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateNull();
    }
    if (info.argv[PARAM0]->TypeOf() == NativeValueType::NATIVE_STRING) {
        ConvertFromJsValue(engine, info.argv[PARAM0], data->uri);
        HILOG_DEBUG("uri=%{public}s", data->uri.c_str());
    }
    UnwrapDataAbilityPredicates(data->predicates, reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[PARAM1]));
    auto execute = [dataAbilityHelper = dataAbilityHelper_, getData = data] () {
        if (dataAbilityHelper == nullptr) {
            HILOG_ERROR("task execute error, the dataAbilityHelper is nullptr");
            return;
        }
        if (getData == nullptr) {
            HILOG_ERROR("task execute error, getData == nullptr");
            return;
        }
        if (getData->uri.empty()) {
            HILOG_ERROR(" task execute error, uri is empty.");
            getData->errorVal = INVALID_PARAMETER;
            return;
        }
        OHOS::Uri uriValue(getData->uri);
        getData->result = dataAbilityHelper->Delete(uriValue, getData->predicates);
    };
    auto complete = [getData = data] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        task.ResolveWithCustomize(engine, CreateJsError(engine, getData->errorVal), CreateJsValue(engine, getData->result));
    };
    auto callback = (info.argc == ARGS_THREE) ? info.argv[PARAM2] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsDataAbilityHelper::OnDelete",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsDataAbilityHelper::OnQuery(NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s called", __func__);
    if (info.argc < ARGS_THREE || info.argc > ARGS_FOUR) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateNull();
    }
    std::shared_ptr<JsQueryData> data = std::make_shared<JsQueryData>();
    auto env = reinterpret_cast<napi_env>(&engine);
    auto arg1 = reinterpret_cast<napi_value>(info.argv[PARAM1]);
    auto arg2 = reinterpret_cast<napi_value>(info.argv[PARAM2]);
    if (info.argv[PARAM0]->TypeOf() == NativeValueType::NATIVE_STRING) {
        ConvertFromJsValue(engine, info.argv[PARAM0], data->uri);
        HILOG_DEBUG("uri=%{public}s", data->uri.c_str());
    }

    bool arrayStringbool = NapiValueToArrayStringUtf8(env, arg1, data->columns);
    if (!arrayStringbool) {
        HILOG_ERROR("The return value of arraystringbool is false");
    }

    UnwrapDataAbilityPredicates(data->predicates, env, arg2);
    auto execute = [dataAbilityHelper = dataAbilityHelper_, getData = data] () {
        if (dataAbilityHelper == nullptr) {
            HILOG_ERROR("task execute error, the dataAbilityHelper is nullptr");
            return;
        }
        if (getData == nullptr) {
            HILOG_ERROR("task execute error, getData == nullptr");
            return;
        }
        if (getData->uri.empty()) {
            HILOG_ERROR(" task execute error, uri is empty.");
            getData->errorVal = INVALID_PARAMETER;
            return;
        }
        OHOS::Uri uriValue(getData->uri);
        getData->result = dataAbilityHelper->Query(uriValue, getData->columns, getData->predicates);
    };
    auto complete = [getData = data] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        NativeValue *jsResult = engine.CreateNull();
        if (getData->result != nullptr) {
            auto result = RdbJsKit::ResultSetProxy::NewInstance(reinterpret_cast<napi_env>(&engine), getData->result);
            jsResult = reinterpret_cast<NativeValue*>(result);
        }
        task.ResolveWithCustomize(engine, CreateJsError(engine, getData->errorVal), jsResult);
    };
    auto callback = info.argc == ARGS_FOUR ? info.argv[PARAM3] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsDataAbilityHelper::OnQuery",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsDataAbilityHelper::OnUpdate(NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s called", __func__);
    std::shared_ptr<JsUpdateData> data = std::make_shared<JsUpdateData>();
    if (info.argc < ARGS_THREE || info.argc > ARGS_FOUR) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateNull();
    }

    if (info.argv[PARAM0]->TypeOf() == NativeValueType::NATIVE_STRING) {
        ConvertFromJsValue(engine, info.argv[PARAM0], data->uri);
        HILOG_DEBUG("uri=%{public}s", data->uri.c_str());
    }

    data->valueBucket.Clear();
    AnalysisValuesBucket(data->valueBucket, reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[PARAM1]));
    UnwrapDataAbilityPredicates(data->predicates, reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[PARAM2]));
    auto execute = [dataAbilityHelper = dataAbilityHelper_, getData = data] () {
        if (dataAbilityHelper == nullptr) {
            HILOG_ERROR("task execute error, the dataAbilityHelper is nullptr");
            return;
        }
        if (getData == nullptr) {
            HILOG_ERROR("task execute error, getData == nullptr");
            return;
        }
        if (getData->uri.empty()) {
            HILOG_ERROR(" task execute error, uri is empty.");
            getData->errorVal = INVALID_PARAMETER;
            return;
        }
        OHOS::Uri uriValue(getData->uri);
        getData->result = dataAbilityHelper->Update(uriValue, getData->valueBucket, getData->predicates);
    };
    auto complete = [getData = data] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        task.ResolveWithCustomize(engine, CreateJsError(engine, getData->errorVal),
            CreateJsValue(engine, getData->result));
    };
    auto callback = info.argc == ARGS_FOUR ? info.argv[PARAM3] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsDataAbilityHelper::OnUpdate",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsDataAbilityHelper::OnBatchInsert(NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s called", __func__);
    std::shared_ptr<JsBatchInsertData> data = std::make_shared<JsBatchInsertData>();
    if (info.argc < ARGS_TWO || info.argc > ARGS_THREE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateNull();
    }

    if (info.argv[PARAM0]->TypeOf() == NativeValueType::NATIVE_STRING) {
        ConvertFromJsValue(engine, info.argv[PARAM0], data->uri);
        HILOG_DEBUG("uri=%{public}s", data->uri.c_str());
    }
    data->values = NapiValueObject(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[PARAM1]));
    auto execute = [dataAbilityHelper = dataAbilityHelper_, getData = data] () {
        if (dataAbilityHelper == nullptr) {
            HILOG_ERROR("task execute error, the dataAbilityHelper is nullptr");
            return;
        }
        if (getData == nullptr) {
            HILOG_ERROR("task execute error, getData == nullptr");
            return;
        }
        if (getData->uri.empty()) {
            HILOG_ERROR(" task execute error, uri is empty.");
            getData->errorVal = INVALID_PARAMETER;
            return;
        }
        OHOS::Uri uriValue(getData->uri);
        getData->result = dataAbilityHelper->BatchInsert(uriValue, getData->values);
    };
    auto complete = [getData = data] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        task.ResolveWithCustomize(engine, CreateJsError(engine, getData->errorVal), CreateJsValue(engine, getData->result));
    };
    auto callback = (info.argc == ARGS_THREE) ? info.argv[PARAM2] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsDataAbilityHelper::OnBatchInsert",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsDataAbilityHelper::OnOpenFile(NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s called", __func__);
    std::shared_ptr<JsOpenFileData> data = std::make_shared<JsOpenFileData>();
    if (info.argc < ARGS_TWO || info.argc > ARGS_THREE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateNull();
    }
    if (info.argv[PARAM0]->TypeOf() == NativeValueType::NATIVE_STRING) {
        ConvertFromJsValue(engine, info.argv[PARAM0], data->uri);
        HILOG_DEBUG("uri=%{public}s", data->uri.c_str());
    }
    if (info.argv[PARAM1]->TypeOf() == NativeValueType::NATIVE_STRING) {
        ConvertFromJsValue(engine, info.argv[PARAM1], data->mode);
        HILOG_DEBUG("uri=%{public}s", data->mode.c_str());
    }

    auto execute = [dataAbilityHelper = dataAbilityHelper_, getData = data] () {
        if (dataAbilityHelper == nullptr) {
            HILOG_ERROR("task execute error, the dataAbilityHelper is nullptr");
            return;
        }
        if (getData == nullptr) {
            HILOG_ERROR("task execute error, getData == nullptr");
            return;
        }
        if (getData->uri.empty()) {
            HILOG_ERROR(" task execute error, uri is empty.");
            getData->errorVal = INVALID_PARAMETER;
            return;
        }

        OHOS::Uri uriValue(getData->uri);
        getData->result = dataAbilityHelper->OpenFile(uriValue, getData->mode);
    };
    auto complete = [getData = data] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        task.ResolveWithCustomize(engine, CreateJsError(engine, getData->errorVal),
            CreateJsValue(engine, getData->result));
    };
    auto callback = (info.argc == ARGS_THREE) ? info.argv[PARAM2] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsDataAbilityHelper::OnOpenFile",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsDataAbilityHelper::OnGetType(NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s called", __func__);
    std::shared_ptr<JsGetTypeData> data = std::make_shared<JsGetTypeData>();
    if (info.argc == ARGS_ZERO || info.argc > ARGS_TWO) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }
    if (info.argv[PARAM0]->TypeOf() == NativeValueType::NATIVE_STRING) {
        ConvertFromJsValue(engine, info.argv[PARAM0], data->uri);
        HILOG_DEBUG("uri=%{public}s", data->uri.c_str());
    }
    auto execute = [dataAbilityHelper = dataAbilityHelper_, getData = data] () {
        if (dataAbilityHelper == nullptr) {
            HILOG_ERROR("task execute error, the dataAbilityHelper is nullptr");
            return;
        }
        if (getData == nullptr) {
            HILOG_ERROR("task execute error, getData == nullptr");
            return;
        }
        if (getData->uri.empty()) {
            HILOG_ERROR(" task execute error, uri is empty.");
            getData->errorVal = INVALID_PARAMETER;
            return;
        }

        OHOS::Uri uriValue(getData->uri);
        getData->result = dataAbilityHelper->GetType(uriValue);
    };
    auto complete = [getData = data] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        task.ResolveWithCustomize(engine, CreateJsError(engine, getData->errorVal), 
            CreateJsValue(engine, getData->result));
    };
    auto callback = (info.argc == ARGS_TWO) ? info.argv[PARAM1] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsDataAbilityHelper::OnGetType",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsDataAbilityHelper::OnGetFileTypes(NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("%{public}s called", __func__);
    auto errorVal = std::make_shared<int32_t>(NO_ERROR);
    if (info.argc < ARGS_TWO || info.argc > ARGS_THREE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateNull();
    }
    std::shared_ptr<JsGetFileTypesData> data = std::make_shared<JsGetFileTypesData>();
    if (info.argv[PARAM0]->TypeOf() == NativeValueType::NATIVE_STRING) {
        ConvertFromJsValue(engine, info.argv[PARAM0], data->uri);
        HILOG_DEBUG("uri=%{public}s", data->uri.c_str());
    }

    if (info.argv[PARAM1]->TypeOf() == NativeValueType::NATIVE_STRING) {
        ConvertFromJsValue(engine, info.argv[PARAM1], data->mimeTypeFilter);
        HILOG_DEBUG("mimeTypeFilter=%{public}s", data->mimeTypeFilter.c_str());
    }

    auto execute = [dataAbilityHelper = dataAbilityHelper_, getData = data] () {
        if (dataAbilityHelper != nullptr) {
            if (getData == nullptr) {
                HILOG_ERROR("task execute error, getData is nullptr");
                return;
            }
            if (getData->uri.empty()) {
                HILOG_ERROR(" task execute error, uri is empty.");
                getData->errorVal = INVALID_PARAMETER;
                return;
            }

            OHOS::Uri uriValue(getData->uri);
            getData->result = dataAbilityHelper->GetFileTypes(uriValue, getData->mimeTypeFilter);
        } else {
            HILOG_DEBUG("task execute error, the dataAbilityHelper is nullptr");
        }
    };
    auto complete = [getData = data] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_DEBUG("OnGetFileTypes, result.size:%{public}zu", getData->result.size());
        task.ResolveWithCustomize(engine, CreateJsError(engine, getData->errorVal),
            CreateNativeArray(engine, getData->result));
    };
    auto callback = (info.argc == ARGS_THREE) ? info.argv[PARAM2] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsDataAbilityHelper::OnGetFileTypes",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsDataAbilityHelper::OnNormalizeUri(NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("called");
    if (info.argc == ARGS_ZERO || info.argc > ARGS_TWO) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }
    std::shared_ptr<JsNormalizeUriData> data = std::make_shared<JsNormalizeUriData>();
    if (info.argv[PARAM0]->TypeOf() == NativeValueType::NATIVE_STRING) {
        ConvertFromJsValue(engine, info.argv[PARAM0], data->uri);
        HILOG_DEBUG("uri=%{public}s", data->uri.c_str());
    }

    auto execute = [dataAbilityHelper = dataAbilityHelper_, getData = data] () {
        if (dataAbilityHelper == nullptr) {
            HILOG_ERROR("task execute error, the dataAbilityHelper is nullptr");
            return;
        }
        if (getData == nullptr) {
            HILOG_ERROR("task execute error, getData is nullptr");
            return;
        }
        if (getData->uri.empty()) {
            getData->errorVal = INVALID_PARAMETER;
            HILOG_ERROR("task execute error, uri is empty");
            return;
        }
        OHOS::Uri uriValue(getData->uri);
        OHOS::Uri result = dataAbilityHelper->NormalizeUri(uriValue);
        getData->result = result.ToString();
    };
    auto complete = [getData = data] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        task.ResolveWithCustomize(engine, CreateJsError(engine, getData->errorVal),
            CreateJsValue(engine, getData->result));
    };

    auto callback = (info.argc == ARGS_TWO) ? info.argv[PARAM1] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsDataAbilityHelper::OnNormalizeUri",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsDataAbilityHelper::OnDenormalizeUri(NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("called");
    if (info.argc == ARGS_ZERO || info.argc > ARGS_TWO) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }
    std::shared_ptr<JsDenormalizeUriData> data = std::make_shared<JsDenormalizeUriData>();
    if (info.argv[PARAM0]->TypeOf() == NativeValueType::NATIVE_STRING) {
        ConvertFromJsValue(engine, info.argv[PARAM0], data->uri);
        HILOG_DEBUG("uri=%{public}s", data->uri.c_str());
    }
    auto execute = [dataAbilityHelper = dataAbilityHelper_, getData = data] () {
        if (dataAbilityHelper == nullptr) {
            HILOG_ERROR("task execute error, the dataAbilityHelper is nullptr");
            return;
        }
        if (getData == nullptr) {
            HILOG_ERROR("task execute error, getData is nullptr");
            return;
        }
        if (getData->uri.empty()) {
            getData->errorVal = INVALID_PARAMETER;
            HILOG_ERROR("task execute error, uri is empty");
            return;
        }
        OHOS::Uri uriValue(getData->uri);
        OHOS::Uri result = dataAbilityHelper->DenormalizeUri(uriValue);
        getData->result = result.ToString();
    };
    auto complete = [getData = data] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        task.ResolveWithCustomize(engine, CreateJsError(engine, getData->errorVal),
            CreateJsValue(engine, getData->result));
    };

    auto callback = (info.argc == ARGS_TWO) ? info.argv[PARAM1] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsDataAbilityHelper::OnDenormalizeUri",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsDataAbilityHelper::OnExecuteBatch(NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("called");
    if (info.argc < ARGS_TWO || info.argc > ARGS_THREE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    std::shared_ptr<JsExecuteBatchData> data = std::make_shared<JsExecuteBatchData>();
    if (!UnWarpExecuteBatchParams(engine, info, data)) {
        HILOG_ERROR("unwarp input params error");
    }
    HILOG_DEBUG("operations size=%{public}zu", data->operations.size());
    auto execute = [dataAbilityHelper = dataAbilityHelper_, getData = data] () {
        if (dataAbilityHelper == nullptr) {
            HILOG_ERROR("task execute error, the dataAbilityHelper is nullptr");
            return;
        }
        if (getData == nullptr) {
            HILOG_ERROR("task execute error, getData is nullptr");
            return;
        }
        OHOS::Uri uri(getData->uri);
        getData->results = dataAbilityHelper->ExecuteBatch(uri, getData->operations);
    };
    auto complete = [this, getData = data] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        task.ResolveWithCustomize(engine, CreateJsError(engine, getData->errorVal),
            CreateExecuteBatchResult(engine, getData));
    };

    auto callback = (info.argc == ARGS_THREE) ? info.argv[PARAM2] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsDataAbilityHelper::OnExecuteBatch",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsDataAbilityHelper::OnCall(NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("called");
    if (info.argc < ARGS_FOUR || info.argc > ARGS_FIVE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateNull();
    }
    std::shared_ptr<JsCallData> data = std::make_shared<JsCallData>();
    if (!UnWarpCallParams(engine, info, data)) {
        HILOG_ERROR("unwarp input params error");
        return engine.CreateNull();
    }
    auto execute = [dataAbilityHelper = dataAbilityHelper_, getData = data] () {
        if (dataAbilityHelper == nullptr) {
            HILOG_ERROR("task execute error, the dataAbilityHelper is nullptr");
            return;
        }
        if (getData == nullptr) {
            HILOG_ERROR("task execute error, getData is nullptr");
            return;
        }
        if (getData->uri.empty()) {
            HILOG_ERROR("task execute error, uri is empty");
            getData->errorVal = INVALID_PARAMETER;
            return;
        }

        OHOS::Uri uri(getData->uri);
        getData->result = dataAbilityHelper->Call(uri, getData->method, getData->arg, getData->extras);
    };
    auto complete = [this, getData = data] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        task.ResolveWithCustomize(engine, CreateJsError(engine, getData->errorVal),
            CreateCallResult(engine, getData));
    };

    auto callback = (info.argc == ARGS_FIVE) ? info.argv[PARAM4] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsDataAbilityHelper::OnCall",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));
    return result;
}

bool JsDataAbilityHelper::UnWarpCallParams(
    NativeEngine &engine, const NativeCallbackInfo &info, std::shared_ptr<JsCallData> &data)
{
    HILOG_DEBUG("called");
    if (data == nullptr || info.argc < ARGS_FOUR || info.argc > ARGS_FIVE) {
        HILOG_ERROR("input params error");
        return false;
    }

    if (!ConvertFromJsValue(engine, info.argv[PARAM0], data->uri)) {
        HILOG_ERROR("SetPacMapObject convert from JsValue error");
        return false;
    }

    if (!ConvertFromJsValue(engine, info.argv[PARAM1], data->method)) {
        HILOG_ERROR("SetPacMapObject convert from JsValue error");
        return false;
    }

    if (!ConvertFromJsValue(engine, info.argv[PARAM2], data->arg)) {
        HILOG_ERROR("SetPacMapObject convert from JsValue error");
        return false;
    }

    AnalysisPacMap(engine, info.argv[PARAM3], data->extras);
    return true;
}

bool JsDataAbilityHelper::UnWarpExecuteBatchParams(
    NativeEngine &engine, const NativeCallbackInfo &info, std::shared_ptr<JsExecuteBatchData> &data)
{
    if (data == nullptr) {
        HILOG_ERROR("input params is nullptr");
        return false;
    }

    if (info.argv[PARAM0]->TypeOf() != NativeValueType::NATIVE_STRING) {
        HILOG_ERROR("uri type is error");
        return false;
    }

    if (!ConvertFromJsValue(engine, info.argv[PARAM0], data->uri)) {
        HILOG_ERROR("convert from JsValue error");
        return false;
    }

    if (!info.argv[PARAM1]->IsArray()) {
        HILOG_ERROR("operations type is error");
        return false;
    }

    auto array = ConvertNativeValueTo<NativeArray>(info.argv[PARAM1]);
    if (array == nullptr) {
        HILOG_ERROR("convert native value error");
        return false;
    }

    for (uint32_t i = 0; i < array->GetLength(); i++) {
        auto item = array->GetElement(i);
        if (item == nullptr) {
            HILOG_ERROR("get element is nullptr");
            return false;
        }

        std::shared_ptr<DataAbilityOperation> operation = nullptr;
        UnwrapDataAbilityOperation(engine, item, operation);
        if (operation == nullptr) {
            HILOG_WARN("data operation is nullptr, index=%{public}d", i);
        }
        data->operations.push_back(operation);
    }
    return true;
}

NativeValue* JsDataAbilityHelper::CreateDataAbilityResult(
    NativeEngine &engine, const std::shared_ptr<DataAbilityResult> &data)
{
    HILOG_DEBUG("called");
    if (data == nullptr) {
        HILOG_ERROR("input params error");
        return engine.CreateUndefined();
    }

    auto objContext = engine.CreateObject();
    if (objContext == nullptr) {
        HILOG_ERROR("CreateObject failed");
        return engine.CreateUndefined();
    }

    auto object = ConvertNativeValueTo<NativeObject>(objContext);
    if (object == nullptr) {
        HILOG_ERROR("ConvertNativeValueTo object failed");
        return engine.CreateUndefined();
    }

    object->SetProperty("uri", CreateJsValue(engine, data->GetUri().ToString()));
    object->SetProperty("count", CreateJsValue(engine, data->GetCount()));

    return objContext;
}

NativeValue* JsDataAbilityHelper::CreateExecuteBatchResult(
    NativeEngine &engine, const std::shared_ptr<JsExecuteBatchData> &data)
{
    HILOG_DEBUG("called");
    if (data == nullptr) {
        HILOG_ERROR("input params error");
        return engine.CreateUndefined();
    }

    NativeValue* arrayValue = engine.CreateArray(data->results.size());
    NativeArray* array = ConvertNativeValueTo<NativeArray>(arrayValue);

    uint32_t index = 0;
    for (const auto &resultInfo : data->results) {
        array->SetElement(index++, CreateDataAbilityResult(engine, resultInfo));
    }
    return arrayValue;
}

NativeValue* JsDataAbilityHelper::CreateCallResult(NativeEngine &engine, const std::shared_ptr<JsCallData> &data)
{
    HILOG_DEBUG("called");
    if (data == nullptr || data->result == nullptr) {
        HILOG_ERROR("input params error");
        return engine.CreateUndefined();
    }

    auto objContext = engine.CreateObject();
    if (objContext == nullptr) {
        HILOG_ERROR("CreateObject failed");
        return engine.CreateUndefined();
    }

    auto object = ConvertNativeValueTo<NativeObject>(objContext);
    if (object == nullptr) {
        HILOG_ERROR("ConvertNativeValueTo object failed");
        return engine.CreateUndefined();
    }
    std::string resultWithoutTag = ExcludeTag(data->result->ToString(), "pacmap");
    object->SetProperty("result", CreateJsValue(engine, resultWithoutTag));

    return objContext;
}

void JsDataAbilityHelper::SetPacMapObject(
    NativeEngine &engine, NativeValue* jsParam, const std::string strKey, AppExecFwk::PacMap &pacMap)
{
    if (jsParam == nullptr) {
        HILOG_ERROR("SetPacMapObject input params error");
        return;
    }

    auto type = jsParam->TypeOf();
    if (type == NativeValueType::NATIVE_STRING) {
        std::string valueStr("");
        if (!ConvertFromJsValue(engine, jsParam, valueStr)) {
            HILOG_ERROR("SetPacMapObject convert string type error");
            return;
        }
        pacMap.PutStringValue(strKey, valueStr);
    } else if (type == NativeValueType::NATIVE_NUMBER) {
        double valueNum = 0.0;
        if (!ConvertFromJsValue(engine, jsParam, valueNum)) {
            HILOG_ERROR("SetPacMapObject convert number type error");
            return;
        }
        pacMap.PutDoubleValue(strKey, valueNum);
    } else if (type == NativeValueType::NATIVE_BOOLEAN) {
        bool valueBool = false;
        if (!ConvertFromJsValue(engine, jsParam, valueBool)) {
            HILOG_ERROR("SetPacMapObject convert boot type error");
            return;
        }
        pacMap.PutBooleanValue(strKey, valueBool);
    } else if (type == NativeValueType::NATIVE_NULL) {
        pacMap.PutObject(strKey, nullptr);
    } else if (type == NativeValueType::NATIVE_OBJECT) {
        pacMap.PutStringValueArray(strKey, ConvertStringVector(engine, jsParam));
    } else {
        HILOG_ERROR("SetPacMapObject pacMap type error");
    }
}

void JsDataAbilityHelper::AnalysisPacMap(NativeEngine &engine, NativeValue* jsParam, AppExecFwk::PacMap &pacMap)
{
    auto object = ConvertNativeValueTo<NativeObject>(jsParam);
    if (object == nullptr) {
        HILOG_ERROR("Convert Native Value params error");
        return;
    }

    auto array = ConvertNativeValueTo<NativeArray>(object->GetPropertyNames());
    for (uint32_t i = 0; i < array->GetLength(); i++) {
        auto itemName = array->GetElement(i);
        std::string strName("");
        ConvertFromJsValue(engine, itemName, strName);
        auto itemValue = object->GetProperty(itemName);
        SetPacMapObject(engine, itemValue, strName, pacMap);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
