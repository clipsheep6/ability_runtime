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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_COMMON_WANT_H
#define OHOS_ABILITY_RUNTIME_NAPI_COMMON_WANT_H

#include <map>
#include <string>
#include <vector>

#include "js_runtime_utils.h"
#include "native_engine/native_engine.h"
#include "napi_common_data.h"
#include "want.h"
#include "want_params.h"

namespace OHOS {
namespace AppExecFwk {

napi_value WrapElementName(napi_env env, const ElementName &elementName);
bool UnwrapElementName(napi_env env, napi_value param, ElementName &elementName);

napi_value WrapWantParams(napi_env env, const AAFwk::WantParams &wantParams);
bool UnwrapWantParams(napi_env env, napi_value param, AAFwk::WantParams &wantParams);
bool BlackListFilter(const std::string &strProName);

napi_value WrapWant(napi_env env, const Want &want);
bool UnwrapWant(napi_env env, napi_value param, Want &want);

void HandleNapiObject(napi_env env, napi_value param, napi_value jsProValue, std::string &strProName,
    AAFwk::WantParams &wantParams);

bool IsSpecialObject(napi_env env, napi_value param, std::string &strProName, std::string type,
    napi_valuetype jsValueProType);

bool HandleFdObject(napi_env env, napi_value param, std::string &strProName, AAFwk::WantParams &wantParams);

bool HandleRemoteObject(napi_env env, napi_value param, std::string &strProName, AAFwk::WantParams &wantParams);

NativeValue* CreateJsWant(NativeEngine &engine, const Want &want);
NativeValue* CreateJsWantParams(NativeEngine &engine, const AAFwk::WantParams &wantParams);
template<class TBase, class T, class NativeT>
bool InnerWrapJsWantParams(
    NativeEngine &engine, NativeObject* object, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    TBase *ao = TBase::Query(value);
    if (ao != nullptr) {
        NativeT natValue = T::Unbox(ao);
        object->SetProperty(key.c_str(), OHOS::AbilityRuntime::CreateJsValue(engine, natValue));
        return true;
    }
    return false;
}

bool InnerWrapJsWantParamsWantParams(
    NativeEngine &engine, NativeObject* object, const std::string &key, const AAFwk::WantParams &wantParams);

bool WrapJsWantParamsArray(
    NativeEngine &engine, NativeObject* object, const std::string &key, sptr<AAFwk::IArray> &ao);

template<class TBase, class T, class NativeT>
bool InnerWrapWantParamsArray(
    NativeEngine &engine, NativeObject* object, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }
    std::vector<NativeT> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            TBase *iValue = TBase::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(T::Unbox(iValue));
            }
        }
    }
    object->SetProperty(key.c_str(), OHOS::AbilityRuntime::CreateNativeArray(engine, natArray));
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NAPI_COMMON_WANT_H
