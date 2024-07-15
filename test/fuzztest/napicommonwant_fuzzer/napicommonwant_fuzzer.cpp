/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "napicommonwant_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "napi_common_want.h"
#undef protected
#undef private

#include "ability_record.h"
#include "array_wrapper.h"
#include "bool_wrapper.h"
#include "byte_wrapper.h"
#include "double_wrapper.h"
#include "float_wrapper.h"
#include "int_wrapper.h"
#include "long_wrapper.h"
#include "short_wrapper.h"
#include "string_wrapper.h"
#include "zchar_wrapper.h"
#include "remote_object_wrapper.h"
#include "native_runtime_impl.h"
#include "js_environment.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_THREE = 3;
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
}

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[ENABLE] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

void NapiCommonWantFuzztest1(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    napi_env env = nullptr;
    ElementName elementName;
    elementName.SetDeviceID(stringParam);
    elementName.SetBundleName(stringParam);
    elementName.SetAbilityName(stringParam);
    elementName.SetModuleName(stringParam);
    WrapElementName(env, elementName); // branch failed
    napi_value param = nullptr;
    UnwrapElementName(env, param, elementName); // branch failed
    AAFwk::WantParams wantParams1;
    WrapWantParams(env, wantParams1); // branch failed
    wantParams1.SetParam("intf1", String::Box(stringParam));
    wantParams1.SetParam("intf2", Long::Box(int32Param));
    wantParams1.SetParam("intf3", Boolean::Box(boolParam));
    wantParams1.SetParam("intf4", Integer::Box(int32Param));
    wantParams1.SetParam("intf5", Float::Box(int32Param));
    wantParams1.SetParam("intf5", RemoteObjectWrap::Box(nullptr));
    wantParams1.SetParam("intf6", Char::Box(int32Param));
    wantParams1.SetParam("intf7", Double::Box(int32Param));
    wantParams1.SetParam("intf8", Byte::Box(int32Param));
    std::size_t size = 3; // 3 means arraysize.
    sptr<IArray> ao = new (std::nothrow) Array(size, g_IID_IBoolean);
    if (ao != nullptr) {
        for (std::size_t i = 0; i < size; i++) {
            ao->Set(i, Boolean::Box(boolParam));
        }
        wantParams1.SetParam("intf8", ao);
    }
    WrapWantParams(env, wantParams1); // branch failed
    UnwrapWantParams(env, param, wantParams1); // branch failed
    BlackListFilter(Want::PARAM_RESV_WINDOW_MODE); // branch
    BlackListFilter(Want::PARAM_RESV_DISPLAY_ID); // branch
    BlackListFilter(stringParam); // branch
    Want want;
    WrapWant(env, want); // branch
    UnwrapWant(env, param, want); // branch
    int resultCode = 0;
    WrapAbilityResult(env, resultCode, want); // branch
    UnWrapAbilityResult(env, param, resultCode, want); // branch
    napi_value jsProValue = nullptr;
    HandleNapiObject(env, param, jsProValue, stringParam, wantParams1); // branch
    IsSpecialObject(env, param, stringParam, stringParam, static_cast<napi_valuetype>(int32Param)); // branch
    HandleFdObject(env, param, stringParam, wantParams1); // branch
    HandleRemoteObject(env, param, stringParam, wantParams1); // branch
    CreateJsWant(env, want); // branch
    CreateJsWantParams(env, wantParams1); // branch
}

void NapiCommonWantFuzztest2(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    napi_env env = nullptr;
    AAFwk::WantParams wantParams1;
    napi_value object = nullptr;
    InnerWrapJsWantParamsWantParams(env, object, stringParam, wantParams1); // failed
    std::size_t size = 3; // 3 means arraysize.
    sptr<IArray> ao = new (std::nothrow) Array(size, g_IID_IBoolean);
    if (ao != nullptr) {
        for (std::size_t i = 0; i < size; i++) {
            ao->Set(i, Boolean::Box(boolParam));
        }
    }
    WrapJsWantParamsArray(env, object, stringParam, ao); // branch
}

void NapiCommonWantFuzztest3(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    AbilityRuntime::JsRuntime::Options options;
    auto err = NativeRuntimeImpl::GetNativeRuntimeImpl().CreateJsEnv(options, jsEnv);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    ElementName elementName1;
    elementName1.SetDeviceID(stringParam);
    elementName1.SetBundleName(stringParam);
    elementName1.SetAbilityName(stringParam);
    elementName1.SetModuleName(stringParam);
    napi_value jsObject = WrapElementName(env, elementName1); // branch

    napi_value param = nullptr;
    ElementName elementName2;
    UnwrapElementName(env, param, elementName2); // branch null param
    ElementName elementName3;
    UnwrapElementName(env, jsObject, elementName3); //  branch not null param

    AAFwk::WantParams wantParams1;
    WrapWantParams(env, wantParams1);
    wantParams1.SetParam("intf1", String::Box(stringParam));
    wantParams1.SetParam("intf2", Long::Box(int32Param));
    wantParams1.SetParam("intf3", Boolean::Box(boolParam));
    wantParams1.SetParam("intf4", Integer::Box(int32Param));
    wantParams1.SetParam("intf5", Float::Box(int32Param));
    wantParams1.SetParam("intf5", RemoteObjectWrap::Box(nullptr));
    wantParams1.SetParam("intf6", Char::Box(int32Param));
    wantParams1.SetParam("intf7", Double::Box(int32Param));
    wantParams1.SetParam("intf8", Byte::Box(int32Param));
    std::size_t size = 3; // 3 means arraysize.
    sptr<IArray> ao = new (std::nothrow) Array(size, g_IID_IBoolean);
    if (ao != nullptr) {
        for (std::size_t i = 0; i < size; i++) {
            ao->Set(i, Boolean::Box(boolParam));
        }
        wantParams1.SetParam("intf8", ao);
    }
    WrapWantParams(env, wantParams1); // branch null param
    UnwrapWantParams(env, param, wantParams1); // branch null param
    UnwrapWantParams(env, jsObject, wantParams1); // branch not null param
}

void NapiCommonWantFuzztest4(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    napi_value param = nullptr;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    AbilityRuntime::JsRuntime::Options options;
    auto err = NativeRuntimeImpl::GetNativeRuntimeImpl().CreateJsEnv(options, jsEnv);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    Want want;
    want.SetElementName(stringParam, stringParam, stringParam, stringParam);
    WrapWant(env, want); // wrap

    UnwrapWant(env, param, want); // branch null param
    ElementName elementName1;
    elementName1.SetDeviceID(stringParam);
    elementName1.SetBundleName(stringParam);
    elementName1.SetAbilityName(stringParam);
    elementName1.SetModuleName(stringParam);
    napi_value jsObject = WrapElementName(env, elementName1); // branch
    UnwrapWant(env, jsObject, want); // branch not null param

    int resultCode = 0;
    napi_value jsonObject1 = WrapAbilityResult(env, resultCode, want); // env not null
    UnWrapAbilityResult(env, param, resultCode, want); // null param
    UnWrapAbilityResult(env, jsonObject1, resultCode, want); // null param

    napi_value jsProValue = nullptr;
    AAFwk::WantParams wantParams1;
    HandleNapiObject(env, param, jsProValue, stringParam, wantParams1); // param null
    HandleNapiObject(env, jsObject, jsProValue, stringParam, wantParams1); // param not null jsProValue null.

    IsSpecialObject(env, param, stringParam, stringParam, static_cast<napi_valuetype>(int32Param)); // param null
    IsSpecialObject(env, jsObject, stringParam, stringParam, static_cast<napi_valuetype>(int32Param)); // param not null

    HandleFdObject(env, param, stringParam, wantParams1); // branch null param
    HandleRemoteObject(env, param, stringParam, wantParams1); // branch null param
    CreateJsWant(env, want); // branch
    CreateJsWantParams(env, wantParams1); // branch
    napi_value object = nullptr;
    InnerWrapJsWantParamsWantParams(env, object, stringParam, wantParams1); // branch null object
    napi_value jsObject2 = nullptr;
    napi_create_object(env, &jsObject2);
    InnerWrapJsWantParamsWantParams(env, jsObject2, stringParam, wantParams1); // branch object, key not exist.
    AAFwk::WantParams wantParams2;
    wantParams2.SetParam("intf1", String::Box(stringParam));
    InnerWrapJsWantParamsWantParams(env, jsObject2, "intf1", wantParams2); // branch object, key exist.
}

void NapiCommonWantFuzztest5(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    napi_value param = nullptr;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    AbilityRuntime::JsRuntime::Options options;
    auto err = NativeRuntimeImpl::GetNativeRuntimeImpl().CreateJsEnv(options, jsEnv);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    napi_value nullObject = nullptr;
    std::size_t size = 3; // 3 means arraysize.
    sptr<IArray> ao1 = new (std::nothrow) Array(size, g_IID_IBoolean);
    if (ao1 != nullptr) {
        for (std::size_t i = 0; i < size; i++) {
            ao1->Set(i, Boolean::Box(boolParam));
        }
    }
    WrapJsWantParamsArray(env, nullObject, stringParam, ao1); // null object.
    napi_value jsObject1 = nullptr;
    napi_create_object(env, &jsObject1);
    WrapJsWantParamsArray(env, jsObject1, stringParam, ao1); // not null object.

    sptr<IArray> ao2 = new (std::nothrow) Array(size, g_IID_IChar);
    if (ao2 != nullptr) {
        for (std::size_t i = 0; i < size; i++) {
            ao2->Set(i, Char::Box(int32Param));
        }
    }
    WrapJsWantParamsArray(env, nullObject, stringParam, ao2); // null object.
    napi_value jsObject2 = nullptr;
    napi_create_object(env, &jsObject2);
    WrapJsWantParamsArray(env, jsObject2, stringParam, ao2); // not null object.

    sptr<IArray> ao3 = new (std::nothrow) Array(size, g_IID_IByte);
    if (ao3 != nullptr) {
        for (std::size_t i = 0; i < size; i++) {
            ao3->Set(i, Byte::Box(int32Param));
        }
    }
    WrapJsWantParamsArray(env, nullObject, stringParam, ao3); // null object.
    napi_value jsObject3 = nullptr;
    napi_create_object(env, &jsObject3);
    WrapJsWantParamsArray(env, jsObject3, stringParam, ao3); // not null object.

    sptr<IArray> ao4 = new (std::nothrow) Array(size, g_IID_IShort);
    if (ao4 != nullptr) {
        for (std::size_t i = 0; i < size; i++) {
            ao4->Set(i, Short::Box(int32Param));
        }
    }
    WrapJsWantParamsArray(env, nullObject, stringParam, ao4); // null object.
    napi_value jsObject4 = nullptr;
    napi_create_object(env, &jsObject4);
    WrapJsWantParamsArray(env, jsObject4, stringParam, ao4); // not null object.
}

void NapiCommonWantFuzztest6(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    napi_value param = nullptr;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    AbilityRuntime::JsRuntime::Options options;
    auto err = NativeRuntimeImpl::GetNativeRuntimeImpl().CreateJsEnv(options, jsEnv);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    napi_value nullObject = nullptr;
    std::size_t size = 3; // 3 means arraysize.
    sptr<IArray> ao1 = new (std::nothrow) Array(size, g_IID_ILong);
    if (ao1 != nullptr) {
        for (std::size_t i = 0; i < size; i++) {
            ao1->Set(i, Long::Box(int32Param));
        }
    }
    WrapJsWantParamsArray(env, nullObject, stringParam, ao1); // null object.
    napi_value jsObject1 = nullptr;
    napi_create_object(env, &jsObject1);
    WrapJsWantParamsArray(env, jsObject1, stringParam, ao1); // not null object.

    sptr<IArray> ao2 = new (std::nothrow) Array(size, g_IID_IFloat);
    if (ao2 != nullptr) {
        for (std::size_t i = 0; i < size; i++) {
            ao2->Set(i, Float::Box(int32Param));
        }
    }
    WrapJsWantParamsArray(env, nullObject, stringParam, ao2); // null object.
    napi_value jsObject2 = nullptr;
    napi_create_object(env, &jsObject2);
    WrapJsWantParamsArray(env, jsObject2, stringParam, ao2); // not null object.

    sptr<IArray> ao3 = new (std::nothrow) Array(size, g_IID_IDouble);
    if (ao3 != nullptr) {
        for (std::size_t i = 0; i < size; i++) {
            ao3->Set(i, Double::Box(int32Param));
        }
    }
    WrapJsWantParamsArray(env, nullObject, stringParam, ao3); // null object.
    napi_value jsObject3 = nullptr;
    napi_create_object(env, &jsObject3);
    WrapJsWantParamsArray(env, jsObject3, stringParam, ao3); // not null object.

    sptr<IArray> ao4 = new (std::nothrow) Array(size, g_IID_IString);
    if (ao4 != nullptr) {
        for (std::size_t i = 0; i < size; i++) {
            ao4->Set(i, String::Box(stringParam));
        }
    }
    WrapJsWantParamsArray(env, nullObject, stringParam, ao4); // null object.
    napi_value jsObject4 = nullptr;
    napi_create_object(env, &jsObject4);
    WrapJsWantParamsArray(env, jsObject4, stringParam, ao4); // not null object.
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    bool boolParam = *data % ENABLE;
    std::string stringParam(data, size);
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    NapiCommonWantFuzztest1(boolParam, stringParam, int32Param);
    NapiCommonWantFuzztest2(boolParam, stringParam, int32Param);
    NapiCommonWantFuzztest3(boolParam, stringParam, int32Param);
    NapiCommonWantFuzztest4(boolParam, stringParam, int32Param);
    NapiCommonWantFuzztest5(boolParam, stringParam, int32Param);
    NapiCommonWantFuzztest6(boolParam, stringParam, int32Param);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE || size > OHOS::FOO_MAX_LEN) {
        return 0;
    }

    char* ch = (char*)malloc(size + 1);
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}

