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

#include "jsruntimeutils_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "js_runtime_utils.h"
#undef private
#include "parcel.h"
#include "securec.h"

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_TWO = 2;
constexpr int INPUT_THREE = 3;
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
constexpr uint8_t ENABLE = 2;
} // namespace

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    napi_env env = nullptr;
    int32_t errCode = static_cast<int32_t>(GetU32Data(data));
    std::string strParam(data, size);
    AbilityRuntime::CreateJsError(env, errCode, strParam);
    napi_value object = nullptr;
    const char* name = "some_name";
    const char* moduleName = "some_module_name";
    napi_callback func = nullptr;
    AbilityRuntime::BindNativeFunction(env, object, name, moduleName, func);
    napi_callback getter = nullptr;
    AbilityRuntime::BindNativeProperty(env, object, name, getter);
    napi_callback_info info = nullptr;
    AbilityRuntime::GetNativePointerFromCallbackInfo(env, info, name);
    napi_value* argv = nullptr;
    size_t* argc = nullptr;
    AbilityRuntime::GetCbInfoFromCallbackInfo(env, info, argc, argv);
    void* ptr = nullptr;
    napi_finalize func1 = nullptr;
    AbilityRuntime::SetNamedNativePointer(env, object, name, ptr, func1);
    AbilityRuntime::GetNamedNativePointer(env, object, name);
    napi_value param = nullptr;
    napi_valuetype expectType = napi_valuetype::napi_undefined;
    AbilityRuntime::CheckTypeForNapiValue(env, param, expectType);

    AbilityRuntime::JsRuntime runtime;
    auto handleScope = std::make_shared<AbilityRuntime::HandleScope>(runtime);
    auto handleScope1 = std::make_shared<AbilityRuntime::HandleScope>(env);
    auto handleEscape = std::make_shared<AbilityRuntime::HandleEscape>(runtime);
    auto handleEscape1 = std::make_shared<AbilityRuntime::HandleEscape>(env);

    napi_value lastParam = nullptr;
    napi_value* result = nullptr;
    AbilityRuntime::CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, result);
    AbilityRuntime::CreateEmptyAsyncTask(env, lastParam, result);

    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    /* Validate the length of size */
    if (size > OHOS::FOO_MAX_LEN || size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = (char*)malloc(size + 1);
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
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

