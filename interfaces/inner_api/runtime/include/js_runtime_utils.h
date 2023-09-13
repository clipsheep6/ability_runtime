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

#ifndef OHOS_ABILITY_RUNTIME_JS_RUNTIME_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_RUNTIME_UTILS_H

#include <cstdint>
#include <memory>
#include <sstream>
#include <type_traits>

#include "native_engine/native_engine.h"

#include "js_runtime.h"

namespace OHOS {
namespace AbilityRuntime {
template<class T>
inline T* ConvertNativeValueTo(NativeValue* value)
{
    return (value != nullptr) ? static_cast<T*>(value->GetInterface(T::INTERFACE_ID)) : nullptr;
}

template<class T>
inline NativeValue* CreateJsValue(NativeEngine& engine, const T& value)
{
    using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (std::is_same_v<ValueType, bool>) {
        return engine.CreateBoolean(value);
    } else if constexpr (std::is_arithmetic_v<ValueType>) {
        return engine.CreateNumber(value);
    } else if constexpr (std::is_same_v<ValueType, std::string>) {
        return engine.CreateString(value.c_str(), value.length());
    } else if constexpr (std::is_enum_v<ValueType>) {
        return engine.CreateNumber(static_cast<std::make_signed_t<ValueType>>(value));
    } else if constexpr (std::is_same_v<ValueType, const char*>) {
        return (value != nullptr) ? engine.CreateString(value, strlen(value)) : engine.CreateUndefined();
    }
    return engine.CreateUndefined();
}

template<class T>
inline bool ConvertFromJsValue(NativeEngine& engine, NativeValue* jsValue, T& value)
{
    if (jsValue == nullptr) {
        return false;
    }

    using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (std::is_same_v<ValueType, bool>) {
        auto boolValue = ConvertNativeValueTo<NativeBoolean>(jsValue);
        if (boolValue == nullptr) {
            return false;
        }
        value = *boolValue;
        return true;
    } else if constexpr (std::is_arithmetic_v<ValueType>) {
        auto numberValue = ConvertNativeValueTo<NativeNumber>(jsValue);
        if (numberValue == nullptr) {
            return false;
        }
        value = *numberValue;
        return true;
    } else if constexpr (std::is_same_v<ValueType, std::string>) {
        auto stringValue = ConvertNativeValueTo<NativeString>(jsValue);
        if (stringValue == nullptr) {
            return false;
        }
        size_t len = stringValue->GetLength() + 1;
        auto buffer = std::make_unique<char[]>(len);
        size_t strLength = 0;
        stringValue->GetCString(buffer.get(), len, &strLength);
        value = buffer.get();
        return true;
    } else if constexpr (std::is_enum_v<ValueType>) {
        auto numberValue = ConvertNativeValueTo<NativeNumber>(jsValue);
        if (numberValue == nullptr) {
            return false;
        }
        value = static_cast<ValueType>(static_cast<std::make_signed_t<ValueType>>(*numberValue));
        return true;
    }
}

template<class T>
NativeValue* CreateNativeArray(NativeEngine& engine, const std::vector<T>& data)
{
    NativeValue* arrayValue = engine.CreateArray(data.size());
    NativeArray* array = ConvertNativeValueTo<NativeArray>(arrayValue);
    uint32_t index = 0;
    for (const T& item : data) {
        array->SetElement(index++, CreateJsValue(engine, item));
    }
    return arrayValue;
}

NativeValue* CreateJsError(NativeEngine& engine, int32_t errCode, const std::string& message = std::string());
void BindNativeFunction(NativeEngine& engine, NativeObject& object, const char* name,
    const char* moduleName, NativeCallback func);
void BindNativeProperty(NativeObject& object, const char* name, NativeCallback getter);
void* GetNativePointerFromCallbackInfo(const NativeEngine* engine, NativeCallbackInfo* info, const char* name);

void SetNamedNativePointer(
    NativeEngine& engine, NativeObject& object, const char* name, void* ptr, NativeFinalize func);
void* GetNamedNativePointer(NativeEngine& engine, NativeObject& object, const char* name);

template<class T>
T* CheckParamsAndGetThis(NativeEngine* engine, NativeCallbackInfo* info, const char* name = nullptr)
{
    return static_cast<T*>(GetNativePointerFromCallbackInfo(engine, info, name));
}

class AsyncTask final {
public:
    using ExecuteCallback = std::function<void()>;
    using CompleteCallback = std::function<void(NativeEngine&, AsyncTask&, int32_t)>;

    static void Schedule(const std::string& name, NativeEngine& engine, std::unique_ptr<AsyncTask>&& task);
    static void ScheduleWithDefaultQos(const std::string &name, NativeEngine& engine,
        std::unique_ptr<AsyncTask>&& task);
    static void ScheduleHighQos(const std::string& name, NativeEngine& engine, std::unique_ptr<AsyncTask>&& task);
    static void ScheduleLowQos(const std::string& name, NativeEngine& engine, std::unique_ptr<AsyncTask>&& task);
    bool StartWithDefaultQos(const std::string &name, NativeEngine& engine);

    AsyncTask(NativeDeferred* deferred, std::unique_ptr<ExecuteCallback>&& execute,
        std::unique_ptr<CompleteCallback>&& complete);
    AsyncTask(NativeReference* callbackRef, std::unique_ptr<ExecuteCallback>&& execute,
        std::unique_ptr<CompleteCallback>&& complete);
    ~AsyncTask();

    void Resolve(NativeEngine& engine, NativeValue* value);
    void Reject(NativeEngine& engine, NativeValue* error);
    void ResolveWithNoError(NativeEngine& engine, NativeValue* value);
    void ResolveWithCustomize(NativeEngine& engine, NativeValue* error, NativeValue* value);
    void RejectWithCustomize(NativeEngine& engine, NativeValue* error, NativeValue* value);

private:
    static void Execute(NativeEngine* engine, void* data);
    static void Complete(NativeEngine* engine, int32_t status, void* data);

    bool Start(const std::string &name, NativeEngine& engine);
    bool StartHighQos(const std::string &name, NativeEngine& engine);
    bool StartLowQos(const std::string &name, NativeEngine& engine);

    std::unique_ptr<NativeDeferred> deferred_;
    std::unique_ptr<NativeReference> callbackRef_;
    std::unique_ptr<NativeAsyncWork> work_;
    std::unique_ptr<ExecuteCallback> execute_;
    std::unique_ptr<CompleteCallback> complete_;
};

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    AsyncTask::ExecuteCallback&& execute, AsyncTask::CompleteCallback&& complete, NativeValue** result);

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    AsyncTask::ExecuteCallback&& execute, nullptr_t, NativeValue** result);

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    nullptr_t, AsyncTask::CompleteCallback&& complete, NativeValue** result);

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    nullptr_t, nullptr_t, NativeValue** result);
// ----------above going to delete----------

#define NAPI_CALL_NO_THROW(theCall, retVal)      \
    do {                                         \
        if ((theCall) != napi_ok) {              \
            return retVal;                       \
        }                                        \
    } while (0)

#define GET_CB_INFO_AND_CALL(env, info, T, func)                                       \
    do {                                                                               \
        if (env == nullptr || info == nullptr) {                                       \
            return nullptr;                                                            \
        }                                                                              \
        size_t argc = ARGS_MAX_COUNT;                                                  \
        napi_value argv[ARGS_MAX_COUNT] = {nullptr};                                   \
        T* me = static_cast<T*>(GetCbInfoFromCallbackInfo(env, info, &argc, argv));    \
        return (me != nullptr) ? me->func(env, argc, argv) : nullptr;                  \
    } while (0)

template<typename T, size_t N>
inline constexpr size_t ArraySize(T (&)[N]) noexcept
{
    return N;
}

template<class T>
inline napi_value CreateJsValue(napi_env env, const T& value)
{
    using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
    napi_value result = nullptr;
    if constexpr (std::is_same_v<ValueType, bool>) {
        napi_get_boolean(env, value, &result);
    } else if constexpr (std::is_same_v<ValueType, int32_t>) {
        napi_create_int32(env, value, &result);
    } else if constexpr (std::is_same_v<ValueType, uint32_t>) {
        napi_create_uint32(env, value, &result);
    } else if constexpr (std::is_same_v<ValueType, int64_t>) {
        napi_create_int64(env, value, &result);
    } else if constexpr (std::is_same_v<ValueType, double>) {
        napi_create_double(env, value, &result);
    } else if constexpr (std::is_same_v<ValueType, std::string>) {
        napi_create_string_utf8(env, value.c_str(), value.length(), &result);
    } else if constexpr (std::is_enum_v<ValueType>) {
        napi_create_int64(env, static_cast<int64_t>(value), &result);
    } else if constexpr (std::is_same_v<ValueType, const char*>) {
        (value != nullptr) ? napi_create_string_utf8(env, value, strlen(value), &result) :
            napi_get_undefined(env, &result);
    } else {
        napi_get_undefined(env, &result);
    }
    return result;
}

inline napi_value CreateJsUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

inline napi_value CreateJsNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

template<class T>
inline bool ConvertFromJsValue(napi_env env, napi_value jsValue, T& value)
{
    using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (std::is_same_v<ValueType, bool>) {
        NAPI_CALL_NO_THROW(napi_get_value_bool(env, jsValue, &value), false);
    } else if constexpr (std::is_same_v<ValueType, int32_t>) {
        NAPI_CALL_NO_THROW(napi_get_value_int32(env, jsValue, &value), false);
    } else if constexpr (std::is_same_v<ValueType, uint32_t>) {
        NAPI_CALL_NO_THROW(napi_get_value_uint32(env, jsValue, &value), false);
    } else if constexpr (std::is_same_v<ValueType, int64_t>) {
        NAPI_CALL_NO_THROW(napi_get_value_int64(env, jsValue, &value), false);
    } else if constexpr (std::is_same_v<ValueType, double>) {
        NAPI_CALL_NO_THROW(napi_get_value_double(env, jsValue, &value), false);
    } else if constexpr (std::is_same_v<ValueType, std::string>) {
        size_t len = 0;
        NAPI_CALL_NO_THROW(napi_get_value_string_utf8(env, jsValue, nullptr, 0, &len), false);
        auto buffer = std::make_unique<char[]>(len + 1);
        NAPI_CALL_NO_THROW(napi_get_value_string_utf8(env, jsValue, buffer.get(), len + 1, &value), false);
    } else if constexpr (std::is_enum_v<ValueType>) {
        int64_t retValue = 0;
        NAPI_CALL_NO_THROW(napi_get_value_int64(env, jsValue, &retValue), false);
        value = static_cast<ValueType>(retValue);
    }
    return true;
}

template<class T>
napi_value CreateNativeArray(napi_env env, const std::vector<T>& data)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, data.size(), &arrayValue);
    uint32_t index = 0;
    for (const T& item : data) {
        napi_set_element(env, arrayValue, index++, CreateJsValue(env, item));
    }
    return arrayValue;
}

napi_value CreateJsError(napi_env env, int32_t errCode, const std::string& message = std::string());
void BindNativeFunction(napi_env env, napi_value object, const char* name,
    const char* moduleName, napi_callback func);
void BindNativeProperty(napi_env env, napi_value object, const char* name, napi_callback getter);
void* GetNativePointerFromCallbackInfo(napi_env env, napi_callback_info info, const char* name);
void* GetCbInfoFromCallbackInfo(napi_env env, napi_callback_info info, size_t* argc, napi_value* argv);

void SetNamedNativePointer(
    napi_env env, napi_value object, const char* name, void* ptr, napi_finalize func);
void* GetNamedNativePointer(napi_env env, napi_value object, const char* name);

template<class T>
T* CheckParamsAndGetThis(napi_env env, napi_callback_info info, const char* name = nullptr)
{
    return static_cast<T*>(GetNativePointerFromCallbackInfo(env, info, name));
}

class HandleScope final {
public:
    explicit HandleScope(JsRuntime& jsRuntime);
    explicit HandleScope(napi_env env);
    ~HandleScope();

    HandleScope(const HandleScope&) = delete;
    HandleScope(HandleScope&&) = delete;
    HandleScope& operator=(const HandleScope&) = delete;
    HandleScope& operator=(HandleScope&&) = delete;

    // ---About to be deleted
    explicit HandleScope(NativeEngine& engine);
    // ---
private:
    napi_handle_scope scope_ = nullptr;
    napi_env env_ = nullptr;
};

class HandleEscape final {
public:
    explicit HandleEscape(JsRuntime& jsRuntime);
    explicit HandleEscape(napi_env env);
    ~HandleEscape();

    napi_value Escape(napi_value value);

    HandleEscape(const HandleEscape&) = delete;
    HandleEscape(HandleEscape&&) = delete;
    HandleEscape& operator=(const HandleEscape&) = delete;
    HandleEscape& operator=(HandleEscape&&) = delete;

    // ---About to be deleted
    explicit HandleEscape(NativeEngine& engine);
    NativeValue* Escape(NativeValue* value);
    // ---
private:
    napi_escapable_handle_scope scope_ = nullptr;
    napi_env env_ = nullptr;
};

class NapiAsyncTask final {
public:
    using ExecuteCallback = std::function<void()>;
    using CompleteCallback = std::function<void(napi_env, NapiAsyncTask&, int32_t)>;

    static void Schedule(const std::string& name, napi_env env, std::unique_ptr<NapiAsyncTask>&& task);
    static void ScheduleWithDefaultQos(const std::string &name, napi_env env,
        std::unique_ptr<NapiAsyncTask>&& task);
    static void ScheduleHighQos(const std::string& name, napi_env env, std::unique_ptr<NapiAsyncTask>&& task);
    static void ScheduleLowQos(const std::string& name, napi_env env, std::unique_ptr<NapiAsyncTask>&& task);
    bool StartWithDefaultQos(const std::string &name, napi_env env);

    NapiAsyncTask(napi_deferred deferred, std::unique_ptr<ExecuteCallback>&& execute,
        std::unique_ptr<CompleteCallback>&& complete);
    NapiAsyncTask(napi_ref callbackRef, std::unique_ptr<ExecuteCallback>&& execute,
        std::unique_ptr<CompleteCallback>&& complete);
    ~NapiAsyncTask();

    void Resolve(napi_env env, napi_value value);
    void Reject(napi_env env, napi_value error);
    void ResolveWithNoError(napi_env env, napi_value value);
    void ResolveWithCustomize(napi_env env, napi_value error, napi_value value);
    void RejectWithCustomize(napi_env env, napi_value error, napi_value value);
private:
    static void Execute(napi_env env, void* data);
    static void Complete(napi_env env, napi_status status, void* data);

    bool Start(const std::string &name, napi_env env);
    bool StartHighQos(const std::string &name, napi_env env);
    bool StartLowQos(const std::string &name, napi_env env);

    napi_deferred deferred_ = nullptr;
    napi_ref callbackRef_ = nullptr;
    napi_async_work work_ = nullptr;
    std::unique_ptr<ExecuteCallback> execute_;
    std::unique_ptr<CompleteCallback> complete_;
};

std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    NapiAsyncTask::ExecuteCallback&& execute, NapiAsyncTask::CompleteCallback&& complete, napi_value* result);

std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    NapiAsyncTask::ExecuteCallback&& execute, nullptr_t, napi_value* result);

std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    nullptr_t, NapiAsyncTask::CompleteCallback&& complete, napi_value* result);

std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    nullptr_t, nullptr_t, napi_value* result);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_RUNTIME_UTILS_H
