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

#include "js_api_utils.h"

#include <vector>

#include "hilog_wrapper.h"
#include "string_wrapper.h"
#include "bool_wrapper.h"
#include "double_wrapper.h"
#include "int_wrapper.h"
#include "array_wrapper.h"
#include "remote_object_wrapper.h"
#include "want_params_wrapper.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"
#include "napi_remote_object.h"
#include "ability_business_error.h"
#include "js_error_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace JsApiUtils {
bool UnWrapAbilityResult(NativeEngine &engine, NativeValue* argv, int &resultCode, AAFwk::Want &want)
{
    if (!IsNarmalObject(argv)) {
        HILOG_WARN("invalid argv");
        return false;
    }

    NativeObject* jObj = ConvertNativeValueTo<NativeObject>(argv);
    if (!UnwrapNumberValue(jObj->GetProperty("resultCode"), resultCode)) {
        HILOG_WARN("resultCode invalid");
        return false;
    }

    return UnWrapWant(engine, jObj->GetProperty("want"), want);
}

bool UnWrapWant(NativeEngine &engine, NativeValue* param, AAFwk::Want &want)
{
    if (!IsNarmalObject(param)) {
        HILOG_WARN("param is invalid.");
        return false;
    }

    NativeObject* wantObj = ConvertNativeValueTo<NativeObject>(param);
    AAFwk::WantParams wantParams;
    if (UnwrapWantParams(engine, wantObj->GetProperty("parameters"), wantParams)) {
        want.SetParams(wantParams);
    }

    std::string natValueString;
    if (UnwrapStringValue(wantObj->GetProperty("action"), natValueString)) {
        want.SetAction(natValueString);
    }

    std::vector<std::string> natValueStringList;
    if (UnwrapArrayStringValue(wantObj->GetProperty("entities"), natValueStringList)) {
        for (size_t i = 0; i < natValueStringList.size(); i++) {
            want.AddEntity(natValueStringList[i]);
        }
    }

    natValueString.clear();
    if (UnwrapStringValue(wantObj->GetProperty("uri"), natValueString)) {
        want.SetUri(natValueString);
    }

    int32_t flags = 0;
    if (UnwrapNumberValue(wantObj->GetProperty("flags"), flags)) {
        want.SetFlags(flags);
    }

    std::string deviceId, bundleName, abilityName, moduleName;
    UnwrapStringValue(wantObj->GetProperty("deviceId"), deviceId);
    UnwrapStringValue(wantObj->GetProperty("bundleName"), bundleName);
    UnwrapStringValue(wantObj->GetProperty("abilityName"), abilityName);
    UnwrapStringValue(wantObj->GetProperty("moduleName"), moduleName);
    want.SetElementName(deviceId, bundleName, abilityName, moduleName);

    natValueString.clear();
    if (UnwrapStringValue(wantObj->GetProperty("type"), natValueString)) {
        want.SetType(natValueString);
    }

    return true;
}
namespace {
bool BlackListFilter(const std::string &strProName)
{
    if (strProName == AAFwk::Want::PARAM_RESV_WINDOW_MODE) {
        return true;
    }
    if (strProName == AAFwk::Want::PARAM_RESV_DISPLAY_ID) {
        return true;
    }
    return false;
}

bool UnwrapFDObject(const std::string &propName, NativeValue* jsProp, AAFwk::WantParams &wantParams)
{
    // This is used inner, so no nullptr check
    NativeObject* propObj = ConvertNativeValueTo<NativeObject>(jsProp);
    std::string strType;
    if (!UnwrapStringValue(propObj->GetProperty(AAFwk::TYPE_PROPERTY), strType) ||
        strType != AAFwk::FD) {
        return false;
    }

    int32_t nativeFd = 0;
    if (UnwrapNumberValue(propObj->GetProperty(AAFwk::VALUE_PROPERTY), nativeFd)) {
        AAFwk::WantParams wp;
        wp.SetParam(AAFwk::TYPE_PROPERTY, AAFwk::String::Box(AAFwk::FD));
        wp.SetParam(AAFwk::VALUE_PROPERTY, AAFwk::Integer::Box(nativeFd));
        sptr<AAFwk::IWantParams> pWantParams = AAFwk::WantParamWrapper::Box(wp);
        wantParams.SetParam(propName, pWantParams);
    } else {
        HILOG_INFO("parse FD failed.");
    }
    return true;
}

bool UnwrapRemoteObject(NativeEngine &engine, const std::string &propName,
    NativeValue* jsProp, AAFwk::WantParams &wantParams)
{
    // This is used inner, so no nullptr check
    NativeObject* propObj = ConvertNativeValueTo<NativeObject>(jsProp);
    std::string strType;
    if (!UnwrapStringValue(propObj->GetProperty(AAFwk::TYPE_PROPERTY), strType) ||
        strType != AAFwk::REMOTE_OBJECT) {
        return false;
    }

    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        HILOG_WARN("not system app, REMOTE_OBJECT is FORIBBED IN WANT.");
        return true;
    }

    sptr<IRemoteObject> remoteObject = NAPI_ohos_rpc_getNativeRemoteObject(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(propObj->GetProperty(AAFwk::VALUE_PROPERTY)));
    if (!remoteObject) {
        HILOG_WARN("failed to transfer to remoteObject");
        return true;
    }

    AAFwk::WantParams wp;
    wp.SetParam(AAFwk::TYPE_PROPERTY, AAFwk::String::Box(AAFwk::REMOTE_OBJECT));
    wp.SetParam(AAFwk::VALUE_PROPERTY, AAFwk::RemoteObjectWrap::Box(remoteObject));
    sptr<AAFwk::IWantParams> pWantParams = AAFwk::WantParamWrapper::Box(wp);
    wantParams.SetParam(propName, pWantParams);
    return true;
}

void UnwrapStringArrayParams(const std::string &propName, NativeArray* array,
    AAFwk::WantParams &wantParams, size_t size)
{
    std::vector<sptr<AAFwk::IString>> tempResult;
    tempResult.reserve(size);
    for (size_t i = 0; i < size; i++) {
        std::string value;
        if (UnwrapStringValue(array->GetElement(i), value)) {
            tempResult.push_back(AAFwk::String::Box(value));
        }
    }
    sptr<AAFwk::IArray> arrayProp = new AAFwk::Array(tempResult.size(), AAFwk::g_IID_IString);
    for (size_t i = 0; i < tempResult.size(); i++) {
        arrayProp->Set(i, tempResult[i]);
    }
    wantParams.SetParam(propName, arrayProp);
}

void UnwrapNumberArrayParams(const std::string &propName, NativeArray* array,
    AAFwk::WantParams &wantParams, size_t size)
{
    std::vector<sptr<AAFwk::IDouble>> tempResult;
    tempResult.reserve(size);
    for (size_t i = 0; i < size; i++) {
        double value;
        if (UnwrapNumberValue(array->GetElement(i), value)) {
            tempResult.push_back(AAFwk::Double::Box(value));
        }
    }
    sptr<AAFwk::IArray> arrayProp = new AAFwk::Array(tempResult.size(), AAFwk::g_IID_IDouble);
    for (size_t i = 0; i < tempResult.size(); i++) {
        arrayProp->Set(i, tempResult[i]);
    }
    wantParams.SetParam(propName, arrayProp);
}

void UnwrapBoolArrayParams(const std::string &propName, NativeArray* array,
    AAFwk::WantParams &wantParams, size_t size)
{
    std::vector<sptr<AAFwk::IBoolean>> tempResult;
    tempResult.reserve(size);
    for (size_t i = 0; i < size; i++) {
        auto element = array->GetElement(i);
        if (element && element->TypeOf() == NATIVE_BOOLEAN) {
            tempResult.push_back(AAFwk::Boolean::Box(bool(*ConvertNativeValueTo<NativeBoolean>(element))));
        }
    }
    sptr<AAFwk::IArray> arrayProp = new AAFwk::Array(tempResult.size(), AAFwk::g_IID_IBoolean);
    for (size_t i = 0; i < tempResult.size(); i++) {
        arrayProp->Set(i, tempResult[i]);
    }
    wantParams.SetParam(propName, arrayProp);
}

void UnwrapObjectArrayParams(NativeEngine &engine, const std::string &propName, NativeArray* array,
    AAFwk::WantParams &wantParams, size_t size)
{
    std::vector<sptr<AAFwk::IWantParams>> tempResult;
    tempResult.reserve(size);
    for (size_t i = 0; i < size; i++) {
        AAFwk::WantParams value;
        if (UnwrapWantParams(engine, array->GetElement(i), value)) {
            value.DumpInfo(0);
            tempResult.push_back(AAFwk::WantParamWrapper::Box(value));
        }
    }
    sptr<AAFwk::IArray> arrayProp = new AAFwk::Array(tempResult.size(), AAFwk::g_IID_IWantParams);
    for (size_t i = 0; i < tempResult.size(); i++) {
        arrayProp->Set(i, tempResult[i]);
    }
    wantParams.SetParam(propName, arrayProp);
}

bool UnwrapArrayParams(NativeEngine &engine, const std::string &propName,
    NativeValue* jsProp, AAFwk::WantParams &wantParams)
{
    // This is used inner, so no nullptr check
    if (!jsProp->IsArray()) {
        return false;
    }
    // There should be only one type in the array.
    auto nativeArray = ConvertNativeValueTo<NativeArray>(jsProp);
    auto arrayLen = nativeArray->GetLength();
    if (arrayLen == 0) {
        return true;
    }

    auto firstElemnt = nativeArray->GetElement(0);
    switch (firstElemnt->TypeOf()) {
        case NATIVE_STRING: {
            UnwrapStringArrayParams(propName, nativeArray, wantParams, arrayLen);
            break;
        }
        case NATIVE_NUMBER: {
            UnwrapNumberArrayParams(propName, nativeArray, wantParams, arrayLen);
            break;
        }
        case NATIVE_BOOLEAN: {
            UnwrapBoolArrayParams(propName, nativeArray, wantParams, arrayLen);
            break;
        }
        case NATIVE_OBJECT: {
            UnwrapObjectArrayParams(engine, propName, nativeArray, wantParams, arrayLen);
            break;
        }
        default: {
            HILOG_WARN("Unsupported prop: %{public}s, %{public}d", propName.c_str(), jsProp->TypeOf());
            break;
        }
    }

    return true;
}

void UnwrapObjPropForWantParams(NativeEngine &engine, const std::string &propName,
    NativeValue* jsProp, AAFwk::WantParams &wantParams)
{
    if (UnwrapFDObject(propName, jsProp, wantParams)) {
        return;
    }

    if (UnwrapRemoteObject(engine, propName, jsProp, wantParams)) {
        return;
    }

    if (UnwrapArrayParams(engine, propName, jsProp, wantParams)) {
        return;
    }

    AAFwk::WantParams wp;
    if (UnwrapWantParams(engine, jsProp, wp)) {
        wantParams.SetParam(propName, AAFwk::WantParamWrapper::Box(wp));
    }
}
}

bool UnwrapWantParams(NativeEngine &engine, NativeValue* param, AAFwk::WantParams &wantParams)
{
    if (!IsNarmalObject(param)) {
        HILOG_INFO("param is invalid.");
        return false;
    }

    NativeObject* paramObj = ConvertNativeValueTo<NativeObject>(param);
    std::vector<std::string> propNames;
    if (!UnwrapArrayStringValue(paramObj->GetPropertyNames(), propNames)) {
        HILOG_WARN("Get prop names failed.");
        return false;
    }

    for (const auto &propName : propNames) {
        if (BlackListFilter(propName)) {
            HILOG_INFO("%{public}s is filtered.", propName.c_str());
            continue;
        }
        auto jsProp = paramObj->GetProperty(propName.c_str());
        if (!jsProp) {
            HILOG_WARN("Prop is invalid: %{public}s", propName.c_str());
            continue;
        }
        switch (jsProp->TypeOf()) {
            case NATIVE_STRING: {
                std::string value;
                UnwrapStringValue(jsProp, value);
                wantParams.SetParam(propName, AAFwk::String::Box(value));
                break;
            }
            case NATIVE_BOOLEAN: {
                bool value = *ConvertNativeValueTo<NativeBoolean>(jsProp);
                wantParams.SetParam(propName, AAFwk::Boolean::Box(value));
                break;
            }
            case NATIVE_NUMBER: {
                // In js, all is double and the user should make a desision.
                double value = *ConvertNativeValueTo<NativeNumber>(jsProp);
                wantParams.SetParam(propName, AAFwk::Double::Box(value));
                break;
            }
            case NATIVE_OBJECT: {
                UnwrapObjPropForWantParams(engine, propName, jsProp, wantParams);
                break;
            }
            default: {
                HILOG_WARN("Unsupported prop: %{public}s, %{public}d", propName.c_str(), jsProp->TypeOf());
                break;
            }
        };
    }

    return true;
}

bool UnwrapStringValue(NativeValue* param, std::string &value)
{
    if (param == nullptr) {
        HILOG_INFO("param is nullptr!");
        return false;
    }
    if (param->TypeOf() != NativeValueType::NATIVE_STRING) {
        HILOG_INFO("invalid type!");
        return false;
    }

    auto nativeString = ConvertNativeValueTo<NativeString>(param);
    size_t size = 0;
    nativeString->GetCString(nullptr, 0, &size);
    if (size == 0) {
        return true;
    }

    value.resize(size + 1);
    nativeString->GetCString(value.data(), size + 1, &size);
    value.pop_back();

    return true;
}

bool UnwrapArrayStringValue(NativeValue* param, std::vector<std::string> &value)
{
    if (param == nullptr) {
        HILOG_INFO("param is nullptr!");
        return false;
    }
    if (!param->IsArray()) {
        HILOG_INFO("invalid type!");
        return false;
    }

    auto nativeArray = ConvertNativeValueTo<NativeArray>(param);
    auto arrayLen = nativeArray->GetLength();
    if (arrayLen == 0) {
        return true;
    }

    value.reserve(arrayLen);
    for (uint32_t i = 0; i < arrayLen; i++) {
        std::string strItem;
        if (UnwrapStringValue(nativeArray->GetElement(i), strItem)) {
            value.emplace_back(std::move(strItem));
        }
    }
    return true;
}

bool IsNarmalObject(NativeValue* value)
{
    if (value == nullptr) {
        HILOG_DEBUG("value is nullptr!");
        return false;
    }
    if (value->TypeOf() == NativeValueType::NATIVE_UNDEFINED) {
        HILOG_DEBUG("value is undefined!");
        return true;
    }
    if (value->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_DEBUG("invalid type of value!");
        return false;
    }
    return true;
}

#ifdef ENABLE_ERRCODE
void HandleTooFewParam(NativeEngine &engine, AsyncTask::CompleteCallback &completeCb)
{
    ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
}
void HandleInvalidParam(NativeEngine &engine, AsyncTask::CompleteCallback &completeCb, const std::string &paramName)
{
    ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
}
void HandleTaskResolve(NativeEngine &engine, AsyncTask &task, NativeValue* value)
{
    task.ResolveWithNoError(engine, value != nullptr? value : engine.CreateUndefined());
}
void HandleTaskReject(NativeEngine &engine, AsyncTask &task, int errcode)
{
    task.Reject(engine, CreateJsError(engine, GetJsErrorCodeByNativeError(errcode)));
}
#else
void HandleTooFewParam(NativeEngine &engine, AsyncTask::CompleteCallback &completeCb)
{
    completeCb = [](NativeEngine &engine, AsyncTask &task, int32_t status) {
        task.Reject(engine, CreateJsError(engine, ERR_INVALID_VALUE, "not enough params."));
    };
}
void HandleInvalidParam(NativeEngine &engine, AsyncTask::CompleteCallback &completeCb, const std::string &paramName)
{
    completeCb = [paramName](NativeEngine &engine, AsyncTask &task, int32_t status) {
        task.Reject(engine, CreateJsError(engine, ERR_INVALID_VALUE, paramName + " is invalid."));
    };
}
void HandleTaskResolve(NativeEngine &engine, AsyncTask &task, NativeValue* value)
{
    task.Resolve(engine, value != nullptr? value : engine.CreateUndefined());
}
void HandleTaskReject(NativeEngine &engine, AsyncTask &task, int errcode)
{
    task.Reject(engine, CreateJsError(engine, errcode));
}
#endif
}
}  // namespace AbilityRuntime
}  // namespace OHOS

