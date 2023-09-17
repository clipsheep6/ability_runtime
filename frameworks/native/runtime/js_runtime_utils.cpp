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

#include "js_runtime_utils.h"

#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    std::unique_ptr<AsyncTask::ExecuteCallback>&& execute, std::unique_ptr<AsyncTask::CompleteCallback>&& complete,
    NativeValue** result)
{
    if (lastParam == nullptr || lastParam->TypeOf() != NATIVE_FUNCTION) {
        NativeDeferred* nativeDeferred = nullptr;
        *result = engine.CreatePromise(&nativeDeferred);
        return std::make_unique<AsyncTask>(nativeDeferred, std::move(execute), std::move(complete));
    } else {
        *result = engine.CreateUndefined();
        NativeReference* callbackRef = engine.CreateReference(lastParam, 1);
        return std::make_unique<AsyncTask>(callbackRef, std::move(execute), std::move(complete));
    }
}
} // namespace

// Help Functions
NativeValue* CreateJsError(NativeEngine& engine, int32_t errCode, const std::string& message)
{
    return engine.CreateError(CreateJsValue(engine, errCode), CreateJsValue(engine, message));
}

void BindNativeFunction(NativeEngine& engine, NativeObject& object, const char* name,
    const char* moduleName, NativeCallback func)
{
    std::string fullName(moduleName);
    fullName += ".";
    fullName += name;
    object.SetProperty(name, engine.CreateFunction(fullName.c_str(), fullName.length(), func, nullptr));
}

void BindNativeProperty(NativeObject& object, const char* name, NativeCallback getter)
{
    NativePropertyDescriptor property;
    property.utf8name = name;
    property.name = nullptr;
    property.method = nullptr;
    property.getter = getter;
    property.setter = nullptr;
    property.value = nullptr;
    property.attributes = napi_default;
    property.data = nullptr;
    object.DefineProperty(property);
}

void* GetNativePointerFromCallbackInfo(const NativeEngine* engine, NativeCallbackInfo* info, const char* name)
{
    if (engine == nullptr || info == nullptr) {
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(info->thisVar);
    if (object != nullptr && name != nullptr) {
        object = ConvertNativeValueTo<NativeObject>(object->GetProperty(name));
    }
    return (object != nullptr) ? object->GetNativePointer() : nullptr;
}

void SetNamedNativePointer(NativeEngine& engine, NativeObject& object, const char* name, void* ptr, NativeFinalize func)
{
    NativeValue* value = engine.CreateObject();
    NativeObject* newObject = ConvertNativeValueTo<NativeObject>(value);
    if (newObject == nullptr) {
        return;
    }
    newObject->SetNativePointer(ptr, func, nullptr);
    object.SetProperty(name, value);
}

void* GetNamedNativePointer(NativeEngine& engine, NativeObject& object, const char* name)
{
    NativeObject* namedObj = ConvertNativeValueTo<NativeObject>(object.GetProperty(name));
    return (namedObj != nullptr) ? namedObj->GetNativePointer() : nullptr;
}

// Async Task
AsyncTask::AsyncTask(NativeDeferred* deferred, std::unique_ptr<AsyncTask::ExecuteCallback>&& execute,
    std::unique_ptr<AsyncTask::CompleteCallback>&& complete)
    : deferred_(deferred), execute_(std::move(execute)), complete_(std::move(complete))
{}

AsyncTask::AsyncTask(NativeReference* callbackRef, std::unique_ptr<AsyncTask::ExecuteCallback>&& execute,
    std::unique_ptr<AsyncTask::CompleteCallback>&& complete)
    : callbackRef_(callbackRef), execute_(std::move(execute)), complete_(std::move(complete))
{}

AsyncTask::~AsyncTask() = default;

void AsyncTask::Schedule(const std::string &name, NativeEngine& engine, std::unique_ptr<AsyncTask>&& task)
{
    if (task && task->Start(name, engine)) {
        task.release();
    }
}

void AsyncTask::ScheduleHighQos(const std::string &name, NativeEngine& engine, std::unique_ptr<AsyncTask>&& task)
{
    if (task && task->StartHighQos(name, engine)) {
        task.release();
    }
}

void AsyncTask::ScheduleLowQos(const std::string &name, NativeEngine& engine, std::unique_ptr<AsyncTask>&& task)
{
    if (task && task->StartLowQos(name, engine)) {
        task.release();
    }
}

void AsyncTask::ScheduleWithDefaultQos(const std::string &name,
    NativeEngine& engine, std::unique_ptr<AsyncTask>&& task)
{
    if (task && task->StartWithDefaultQos(name, engine)) {
        task.release();
    }
}

bool AsyncTask::StartWithDefaultQos(const std::string &name, NativeEngine& engine)
{
    work_.reset(engine.CreateAsyncWork(name, Execute, Complete, this));
    return work_->QueueWithQos(napi_qos_default);
}

void AsyncTask::Resolve(NativeEngine& engine, NativeValue* value)
{
    HILOG_DEBUG("AsyncTask::Resolve is called");
    if (deferred_) {
        deferred_->Resolve(value);
        deferred_.reset();
    }
    if (callbackRef_) {
        NativeValue* argv[] = {
            CreateJsError(engine, 0),
            value,
        };
        engine.CallFunction(engine.CreateUndefined(), callbackRef_->Get(), argv, ArraySize(argv));
        callbackRef_.reset();
    }
    HILOG_DEBUG("AsyncTask::Resolve is called end.");
}

void AsyncTask::ResolveWithNoError(NativeEngine& engine, NativeValue* value)
{
    HILOG_DEBUG("AsyncTask::Resolve is called");
    if (deferred_) {
        deferred_->Resolve(value);
        deferred_.reset();
    }
    if (callbackRef_) {
        NativeValue* argv[] = {
            engine.CreateNull(),
            value,
        };
        engine.CallFunction(engine.CreateUndefined(), callbackRef_->Get(), argv, ArraySize(argv));
        callbackRef_.reset();
    }
    HILOG_DEBUG("AsyncTask::Resolve is called end.");
}

void AsyncTask::Reject(NativeEngine& engine, NativeValue* error)
{
    if (deferred_) {
        deferred_->Reject(error);
        deferred_.reset();
    }
    if (callbackRef_) {
        NativeValue* argv[] = {
            error,
            engine.CreateUndefined(),
        };
        engine.CallFunction(engine.CreateUndefined(), callbackRef_->Get(), argv, ArraySize(argv));
        callbackRef_.reset();
    }
}

void AsyncTask::ResolveWithCustomize(NativeEngine& engine, NativeValue* error, NativeValue* value)
{
    HILOG_DEBUG("AsyncTask::ResolveWithCustomize is called");
    if (deferred_) {
        deferred_->Resolve(value);
        deferred_.reset();
    }
    if (callbackRef_) {
        NativeValue* argv[] = {
            error,
            value,
        };
        engine.CallFunction(engine.CreateUndefined(), callbackRef_->Get(), argv, ArraySize(argv));
        callbackRef_.reset();
    }
    HILOG_DEBUG("AsyncTask::ResolveWithCustomize is called end.");
}

void AsyncTask::RejectWithCustomize(NativeEngine& engine, NativeValue* error, NativeValue* value)
{
    HILOG_DEBUG("AsyncTask::RejectWithCustomize is called");
    if (deferred_) {
        deferred_->Reject(error);
        deferred_.reset();
    }
    if (callbackRef_) {
        NativeValue* argv[] = {
            error,
            value,
        };
        engine.CallFunction(engine.CreateUndefined(), callbackRef_->Get(), argv, ArraySize(argv));
        callbackRef_.reset();
    }
    HILOG_DEBUG("AsyncTask::RejectWithCustomize is called end.");
}

void AsyncTask::Execute(NativeEngine* engine, void* data)
{
    if (engine == nullptr || data == nullptr) {
        return;
    }
    auto me = static_cast<AsyncTask*>(data);
    if (me->execute_ && *(me->execute_)) {
        (*me->execute_)();
    }
}

void AsyncTask::Complete(NativeEngine* engine, int32_t status, void* data)
{
    if (engine == nullptr || data == nullptr) {
        return;
    }
    std::unique_ptr<AsyncTask> me(static_cast<AsyncTask*>(data));
    if (me->complete_ && *(me->complete_)) {
        HandleScope handleScope(*engine);
        (*me->complete_)(*engine, *me, status);
    }
}

bool AsyncTask::Start(const std::string &name, NativeEngine& engine)
{
    work_.reset(engine.CreateAsyncWork(name, Execute, Complete, this));
    return work_->Queue();
}

bool AsyncTask::StartHighQos(const std::string &name, NativeEngine& engine)
{
    work_.reset(engine.CreateAsyncWork(name, Execute, Complete, this));
    return work_->QueueWithQos(napi_qos_user_initiated);
}

bool AsyncTask::StartLowQos(const std::string &name, NativeEngine& engine)
{
    work_.reset(engine.CreateAsyncWork(name, Execute, Complete, this));
    return work_->QueueWithQos(napi_qos_utility);
}

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    AsyncTask::ExecuteCallback&& execute, AsyncTask::CompleteCallback&& complete, NativeValue** result)
{
    return CreateAsyncTaskWithLastParam(engine, lastParam,
        std::make_unique<AsyncTask::ExecuteCallback>(std::move(execute)),
        std::make_unique<AsyncTask::CompleteCallback>(std::move(complete)), result);
}

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    AsyncTask::ExecuteCallback&& execute, nullptr_t, NativeValue** result)
{
    return CreateAsyncTaskWithLastParam(
        engine, lastParam, std::make_unique<AsyncTask::ExecuteCallback>(std::move(execute)), nullptr, result);
}

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    nullptr_t, AsyncTask::CompleteCallback&& complete, NativeValue** result)
{
    return CreateAsyncTaskWithLastParam(
        engine, lastParam, nullptr, std::make_unique<AsyncTask::CompleteCallback>(std::move(complete)), result);
}

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    nullptr_t, nullptr_t, NativeValue** result)
{
    return CreateAsyncTaskWithLastParam(engine, lastParam, std::unique_ptr<AsyncTask::ExecuteCallback>(),
        std::unique_ptr<AsyncTask::CompleteCallback>(), result);
}
// ----------above going to delete----------

namespace {
std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    std::unique_ptr<NapiAsyncTask::ExecuteCallback>&& execute,
    std::unique_ptr<NapiAsyncTask::CompleteCallback>&& complete,
    napi_value* result)
{
    napi_valuetype type = napi_undefined;
    napi_typeof(env, lastParam, &type);
    if (lastParam == nullptr || type != napi_function) {
        napi_deferred nativeDeferred = nullptr;
        napi_create_promise(env, &nativeDeferred, result);
        return std::make_unique<NapiAsyncTask>(nativeDeferred, std::move(execute), std::move(complete));
    } else {
        napi_get_undefined(env, result);
        napi_ref callbackRef = nullptr;
        napi_create_reference(env, lastParam, 1, &callbackRef);
        return std::make_unique<NapiAsyncTask>(callbackRef, std::move(execute), std::move(complete));
    }
}
} // namespace

// Help Functions
napi_value CreateJsError(napi_env env, int32_t errCode, const std::string& message)
{
    napi_value result = nullptr;
    napi_create_error(env, CreateJsValue(env, errCode), CreateJsValue(env, message), &result);
    return result;
}

void BindNativeFunction(napi_env env, napi_value object, const char* name,
    const char* moduleName, napi_callback func)
{
    std::string fullName(moduleName);
    fullName += ".";
    fullName += name;
    napi_value result = nullptr;
    napi_create_function(env, fullName.c_str(), fullName.length(), func, nullptr, &result);
    napi_set_named_property(env, object, name, result);
}

void BindNativeProperty(napi_env env, napi_value object, const char* name, napi_callback getter)
{
    napi_property_descriptor properties[1];
    properties[0].utf8name = name;
    properties[0].name = nullptr;
    properties[0].method = nullptr;
    properties[0].getter = getter;
    properties[0].setter = nullptr;
    properties[0].value = nullptr;
    properties[0].attributes = napi_default;
    properties[0].data = nullptr;
    napi_define_properties(env, object, 1, properties);
}

void* GetNativePointerFromCallbackInfo(napi_env env, napi_callback_info info, const char* name)
{
    size_t argcAsync = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr), nullptr);
    if (name != nullptr) {
        napi_get_named_property(env, thisVar, name, &thisVar);
    }
    void* result = nullptr;
    NAPI_CALL_NO_THROW(napi_unwrap(env, thisVar, &result), nullptr);
    return result;
}

void* GetCbInfoFromCallbackInfo(napi_env env, napi_callback_info info, size_t* argc, napi_value* argv)
{
    napi_value thisVar = nullptr;
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, argc, argv, &thisVar, nullptr), nullptr);
    void* result = nullptr;
    NAPI_CALL_NO_THROW(napi_unwrap(env, thisVar, &result), nullptr);
    return result;
}

void* GetNapiCallbackInfoAndThis(napi_env env, napi_callback_info info, NapiCallbackInfo& napiInfo, const char* name)
{
    NAPI_CALL_NO_THROW(napi_get_cb_info(
        env, info, &napiInfo.argc, napiInfo.argv, &napiInfo.thisVar, nullptr), nullptr);
    napi_value value = napiInfo.thisVar;
    if (name != nullptr) {
        napi_get_named_property(env, value, name, &value);
    }
    void* result = nullptr;
    NAPI_CALL_NO_THROW(napi_unwrap(env, value, &result), nullptr);
    return result;
}

void SetNamedNativePointer(napi_env env, napi_value object, const char* name, void* ptr, napi_finalize func)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_wrap(env, objValue, ptr, func, nullptr, nullptr);
    napi_set_named_property(env, object, name, objValue);
}

void* GetNamedNativePointer(napi_env env, napi_value object, const char* name)
{
    napi_value proValue = nullptr;
    napi_get_named_property(env, object, name, &proValue);
    void* result = nullptr;
    napi_unwrap(env, proValue, &result);
    return result;
}

bool CheckTypeForNapiValue(napi_env env, napi_value param, napi_valuetype expectType)
{
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, param, &valueType) != napi_ok) {
        return false;
    }
    return valueType == expectType;
}

// Handle Scope
HandleScope::HandleScope(JsRuntime& jsRuntime)
{
    env_ = (napi_env)jsRuntime.GetNativeEnginePointer();
    napi_open_handle_scope(env_, &scope_);
}

HandleScope::HandleScope(napi_env env)
{
    env_ = env;
    napi_open_handle_scope(env_, &scope_);
}

HandleScope::~HandleScope()
{
    napi_close_handle_scope(env_, scope_);
}

// ---About to be deleted
HandleScope::HandleScope(NativeEngine& engine)
{
    env_ = (napi_env)(&engine);
    napi_open_handle_scope(env_, &scope_);
}
// ---

// Handle Escape
HandleEscape::HandleEscape(JsRuntime& jsRuntime)
{
    env_ = (napi_env)jsRuntime.GetNativeEnginePointer();
    napi_open_escapable_handle_scope(env_, &scope_);
}

HandleEscape::HandleEscape(napi_env env)
{
    env_ = env;
    napi_open_escapable_handle_scope(env_, &scope_);
}

HandleEscape::~HandleEscape()
{
    napi_close_escapable_handle_scope(env_, scope_);
}

napi_value HandleEscape::Escape(napi_value value)
{
    napi_value result = nullptr;
    napi_escape_handle(env_, scope_, value, &result);
    return value;
}

// ---About to be deleted
HandleEscape::HandleEscape(NativeEngine& engine)
{
    env_ = (napi_env)(&engine);
    napi_open_escapable_handle_scope(env_, &scope_);
}

NativeValue* HandleEscape::Escape(NativeValue* value)
{
    return (NativeValue*)Escape((napi_value)value);
}
// ---

// Async Task
NapiAsyncTask::NapiAsyncTask(napi_deferred deferred, std::unique_ptr<NapiAsyncTask::ExecuteCallback>&& execute,
    std::unique_ptr<NapiAsyncTask::CompleteCallback>&& complete)
    : deferred_(deferred), execute_(std::move(execute)), complete_(std::move(complete))
{}

NapiAsyncTask::NapiAsyncTask(napi_ref callbackRef, std::unique_ptr<NapiAsyncTask::ExecuteCallback>&& execute,
    std::unique_ptr<NapiAsyncTask::CompleteCallback>&& complete)
    : callbackRef_(callbackRef), execute_(std::move(execute)), complete_(std::move(complete))
{}

NapiAsyncTask::~NapiAsyncTask() = default;

void NapiAsyncTask::Schedule(const std::string &name, napi_env env, std::unique_ptr<NapiAsyncTask>&& task)
{
    if (task && task->Start(name, env)) {
        task.release();
    }
}

void NapiAsyncTask::ScheduleHighQos(const std::string &name, napi_env env, std::unique_ptr<NapiAsyncTask>&& task)
{
    if (task && task->StartHighQos(name, env)) {
        task.release();
    }
}

void NapiAsyncTask::ScheduleLowQos(const std::string &name, napi_env env, std::unique_ptr<NapiAsyncTask>&& task)
{
    if (task && task->StartLowQos(name, env)) {
        task.release();
    }
}

void NapiAsyncTask::ScheduleWithDefaultQos(const std::string &name, napi_env env, std::unique_ptr<NapiAsyncTask>&& task)
{
    if (task && task->StartWithDefaultQos(name, env)) {
        task.release();
    }
}

bool NapiAsyncTask::StartWithDefaultQos(const std::string &name, napi_env env)
{
    if (work_) {
        napi_delete_async_work(env, work_);
        work_ = nullptr;
    }
    napi_create_async_work(env, nullptr, CreateJsValue(env, name), Execute, Complete, this, &work_);
    napi_queue_async_work_with_qos(env, work_, napi_qos_default);
    return true;
}

void NapiAsyncTask::Resolve(napi_env env, napi_value value)
{
    HILOG_DEBUG("NapiAsyncTask::Resolve is called");
    if (deferred_) {
        napi_resolve_deferred(env, deferred_, value);
        deferred_ = nullptr;
    }
    if (callbackRef_) {
        napi_value argv[] = {
            CreateJsError(env, 0),
            value,
        };
        napi_value func = nullptr;
        napi_get_reference_value(env, callbackRef_, &func);
        napi_call_function(env, CreateJsUndefined(env), func, ArraySize(argv), argv, nullptr);
        napi_delete_reference(env, callbackRef_);
        callbackRef_ = nullptr;
    }
    HILOG_DEBUG("NapiAsyncTask::Resolve is called end.");
}

void NapiAsyncTask::ResolveWithNoError(napi_env env, napi_value value)
{
    HILOG_DEBUG("NapiAsyncTask::Resolve is called");
    if (deferred_) {
        napi_resolve_deferred(env, deferred_, value);
        deferred_ = nullptr;
    }
    if (callbackRef_) {
        napi_value argv[] = {
            CreateJsNull(env),
            value,
        };
        napi_value func = nullptr;
        napi_get_reference_value(env, callbackRef_, &func);
        napi_call_function(env, CreateJsUndefined(env), func, ArraySize(argv), argv, nullptr);
        napi_delete_reference(env, callbackRef_);
        callbackRef_ = nullptr;
    }
    HILOG_DEBUG("NapiAsyncTask::Resolve is called end.");
}

void NapiAsyncTask::Reject(napi_env env, napi_value error)
{
    if (deferred_) {
        napi_reject_deferred(env, deferred_, error);
        deferred_ = nullptr;
    }
    if (callbackRef_) {
        napi_value argv[] = {
            error,
            CreateJsUndefined(env),
        };
        napi_value func = nullptr;
        napi_get_reference_value(env, callbackRef_, &func);
        napi_call_function(env, CreateJsUndefined(env), func, ArraySize(argv), argv, nullptr);
        napi_delete_reference(env, callbackRef_);
        callbackRef_ = nullptr;
    }
}

void NapiAsyncTask::ResolveWithCustomize(napi_env env, napi_value error, napi_value value)
{
    HILOG_DEBUG("NapiAsyncTask::ResolveWithCustomize is called");
    if (deferred_) {
        napi_resolve_deferred(env, deferred_, value);
        deferred_ = nullptr;
    }
    if (callbackRef_) {
        napi_value argv[] = {
            error,
            value,
        };
        napi_value func = nullptr;
        napi_get_reference_value(env, callbackRef_, &func);
        napi_call_function(env, CreateJsUndefined(env), func, ArraySize(argv), argv, nullptr);
        napi_delete_reference(env, callbackRef_);
        callbackRef_ = nullptr;
    }
    HILOG_DEBUG("NapiAsyncTask::ResolveWithCustomize is called end.");
}

void NapiAsyncTask::RejectWithCustomize(napi_env env, napi_value error, napi_value value)
{
    HILOG_DEBUG("NapiAsyncTask::RejectWithCustomize is called");
    if (deferred_) {
        napi_reject_deferred(env, deferred_, error);
        deferred_ = nullptr;
    }
    if (callbackRef_) {
        napi_value argv[] = {
            error,
            value,
        };
        napi_value func = nullptr;
        napi_get_reference_value(env, callbackRef_, &func);
        napi_call_function(env, CreateJsUndefined(env), func, ArraySize(argv), argv, nullptr);
        napi_delete_reference(env, callbackRef_);
        callbackRef_ = nullptr;
    }
    HILOG_DEBUG("NapiAsyncTask::RejectWithCustomize is called end.");
}

void NapiAsyncTask::Execute(napi_env env, void* data)
{
    if (data == nullptr) {
        return;
    }
    auto me = static_cast<NapiAsyncTask*>(data);
    if (me->execute_ && *(me->execute_)) {
        (*me->execute_)();
    }
}

void NapiAsyncTask::Complete(napi_env env, napi_status status, void* data)
{
    if (data == nullptr) {
        return;
    }
    std::unique_ptr<NapiAsyncTask> me(static_cast<NapiAsyncTask*>(data));
    if (me->complete_ && *(me->complete_)) {
        HandleScope handleScope(env);
        (*me->complete_)(env, *me, static_cast<int32_t>(status));
    }
}

bool NapiAsyncTask::Start(const std::string &name, napi_env env)
{
    if (work_) {
        napi_delete_async_work(env, work_);
        work_ = nullptr;
    }
    napi_create_async_work(env, nullptr, CreateJsValue(env, name), Execute, Complete, this, &work_);
    napi_queue_async_work(env, work_);
    return true;
}

bool NapiAsyncTask::StartHighQos(const std::string &name, napi_env env)
{
    if (work_) {
        napi_delete_async_work(env, work_);
        work_ = nullptr;
    }
    napi_create_async_work(env, nullptr, CreateJsValue(env, name), Execute, Complete, this, &work_);
    napi_queue_async_work_with_qos(env, work_, napi_qos_user_initiated);
    return true;
}

bool NapiAsyncTask::StartLowQos(const std::string &name, napi_env env)
{
    if (work_) {
        napi_delete_async_work(env, work_);
        work_ = nullptr;
    }
    napi_create_async_work(env, nullptr, CreateJsValue(env, name), Execute, Complete, this, &work_);
    napi_queue_async_work_with_qos(env, work_, napi_qos_utility);
    return true;
}

std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    NapiAsyncTask::ExecuteCallback&& execute, NapiAsyncTask::CompleteCallback&& complete, napi_value* result)
{
    return CreateAsyncTaskWithLastParam(env, lastParam,
        std::make_unique<NapiAsyncTask::ExecuteCallback>(std::move(execute)),
        std::make_unique<NapiAsyncTask::CompleteCallback>(std::move(complete)), result);
}

std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    NapiAsyncTask::ExecuteCallback&& execute, nullptr_t, napi_value* result)
{
    return CreateAsyncTaskWithLastParam(
        env, lastParam, std::make_unique<NapiAsyncTask::ExecuteCallback>(std::move(execute)), nullptr, result);
}

std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    nullptr_t, NapiAsyncTask::CompleteCallback&& complete, napi_value* result)
{
    return CreateAsyncTaskWithLastParam(
        env, lastParam, nullptr, std::make_unique<NapiAsyncTask::CompleteCallback>(std::move(complete)), result);
}

std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    nullptr_t, nullptr_t, napi_value* result)
{
    return CreateAsyncTaskWithLastParam(env, lastParam, std::unique_ptr<NapiAsyncTask::ExecuteCallback>(),
        std::unique_ptr<NapiAsyncTask::CompleteCallback>(), result);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
