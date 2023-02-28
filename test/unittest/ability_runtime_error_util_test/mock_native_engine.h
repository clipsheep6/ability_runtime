/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ABILITY_RUNTIME_MOCK_NATIVE_ENGINE_H
#define FOUNDATION_ABILITY_RUNTIME_MOCK_NATIVE_ENGINE_H

#include "gmock/gmock.h"
#include "native_engine/native_engine.h"

class MockNativeEngine : public NativeEngine {
public:
    MockNativeEngine() : NativeEngine(nullptr) {}

    virtual ~MockNativeEngine() {}

    NativeValue* GetGlobal() override
    {
        return nullptr;
    }

    NativeValue* CreateNull() override
    {
        return nullptr;
    }

    NativeValue* CreateUndefined() override
    {
        return nullptr;
    }

    NativeValue* CreateBoolean(bool value) override
    {
        return nullptr;
    }

    NativeValue* CreateNumber(int32_t value) override
    {
        return nullptr;
    }

    NativeValue* CreateNumber(uint32_t value) override
    {
        return nullptr;
    }

    NativeValue* CreateNumber(int64_t value) override
    {
        return nullptr;
    }

    NativeValue* CreateNumber(double value) override
    {
        return nullptr;
    }

    NativeValue* CreateBigInt(int64_t value) override
    {
        return nullptr;
    }

    NativeValue* CreateBigInt(uint64_t value) override
    {
        return nullptr;
    }

    NativeValue* CreateString(const char* value, size_t length) override
    {
        return nullptr;
    }

    NativeValue* CreateString16(const char16_t* value, size_t length) override
    {
        return nullptr;
    }

    NativeValue* CreateSymbol(NativeValue* value) override
    {
        return nullptr;
    }

    NativeValue* CreateExternal(void* value, NativeFinalize callback, void* hint,
        size_t nativeBindingSize = 0) override
    {
        return nullptr;
    }

    NativeValue* CreateObject() override
    {
        return nullptr;
    }

    NativeValue* CreateFunction(const char* name, size_t length, NativeCallback cb, void* value) override
    {
        return nullptr;
    }

    NativeValue* CreateArray(size_t length) override
    {
        return nullptr;
    }

    NativeValue* CreateBuffer(void** value, size_t length) override
    {
        return nullptr;
    }

    NativeValue* CreateBufferCopy(void** value, size_t length, const void* data) override
    {
        return nullptr;
    }

    NativeValue* CreateBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint) override
    {
        return nullptr;
    }

    NativeValue* CreateArrayBuffer(void** value, size_t length) override
    {
        return nullptr;
    }

    NativeValue* CreateArrayBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint) override
    {
        return nullptr;
    }

    NativeValue* CreateTypedArray(NativeTypedArrayType type,
        NativeValue* value,
        size_t length,
        size_t offset) override
    {
        return nullptr;
    }

    NativeValue* CreateDataView(NativeValue* value, size_t length, size_t offset) override
    {
        return nullptr;
    }

    NativeValue* CreatePromise(NativeDeferred** deferred) override
    {
        return nullptr;
    }

    void SetPromiseRejectCallback(NativeReference* rejectCallbackRef, NativeReference* checkCallbackRef) override
    {}

    MOCK_METHOD2(CreateError, NativeValue* (NativeValue*, NativeValue*));

    bool InitTaskPoolThread(NativeEngine* engine, NapiConcurrentCallback callback) override
    {
        return false;
    }
    bool InitTaskPoolFunc(NativeEngine* engine, NativeValue* func) override
    {
        return false;
    }

    NativeValue* CallFunction(NativeValue* thisVar,
        NativeValue* function,
        NativeValue* const* argv,
        size_t argc) override
    {
        return nullptr;
    }

    NativeValue* RunScript(NativeValue* script) override
    {
        return nullptr;
    }

    NativeValue* RunScriptPath(const char* path) override
    {
        return nullptr;
    }

    NativeValue* RunScriptBuffer(const char* path, std::vector<uint8_t>& buffer, bool isBundle) override
    {
        return nullptr;
    }

    NativeValue* RunBufferScript(std::vector<uint8_t>& buffer) override
    {
        return nullptr;
    }

    NativeValue* RunActor(std::vector<uint8_t>& buffer, const char* descriptor) override
    {
        return nullptr;
    }

    NativeValue* DefineClass(const char* name,
        NativeCallback callback,
        void* data,
        const NativePropertyDescriptor* properties,
        size_t length) override
    {
        return nullptr;
    }

    NativeValue* CreateInstance(NativeValue* constructor, NativeValue* const* argv, size_t argc) override
    {
        return nullptr;
    }

    NativeReference* CreateReference(NativeValue* value, uint32_t initialRefcount,
        NativeFinalize callback = nullptr, void* data = nullptr, void* hint = nullptr) override
    {
        return nullptr;
    }

    MOCK_METHOD1(Throw, bool(NativeValue* error));

    bool Throw(NativeErrorType type, const char* code, const char* message) override
    {
        return true;
    }

    void* CreateRuntime() override
    {
        return nullptr;
    }

    NativeValue* Serialize(NativeEngine* context, NativeValue* value, NativeValue* transfer) override
    {
        return nullptr;
    }

    NativeValue* Deserialize(NativeEngine* context, NativeValue* recorder) override
    {
        return nullptr;
    }

    void DeleteSerializationData(NativeValue* value) const override
    {}

    NativeValue* LoadModule(NativeValue* str, const std::string& fileName) override
    {
        return nullptr;
    }

    void StartCpuProfiler(const std::string& fileName = "") override
    {}

    void StopCpuProfiler() override
    {}

    void ResumeVM() override
    {}

    NativeValue* ValueToNativeValue(JSValueWrapper& value) override
    {
        return nullptr;
    }

    NativeValue* CreateDate(double value) override
    {
        return nullptr;
    }

    NativeValue* CreateBigWords(int sign_bit, size_t word_count, const uint64_t* words) override
    {
        return nullptr;
    }

    bool SuspendVM() override
    {
        return true;
    }

    bool IsSuspended() override
    {
        return true;
    }

    bool CheckSafepoint() override
    {
        return true;
    }

    bool BuildNativeAndJsStackTrace(std::string& stackTraceStr) override
    {
        return true;
    }

    bool BuildJsStackTrace(std::string& stackTraceStr) override
    {
        return true;
    }

    bool BuildJsStackInfoList(uint32_t tid, std::vector<JsFrameInfo>& jsFrames) override
    {
        return true;
    }

    bool DeleteWorker(NativeEngine* hostEngine, NativeEngine* workerEngine) override
    {
        return true;
    }

    bool StartHeapTracking(double timeInterval, bool isVmMode = true) override
    {
        return true;
    }

    bool StopHeapTracking(const std::string& filePath) override
    {
        return true;
    }

    bool IsExceptionPending() const override
    {
        return false;
    }

    NativeValue* GetAndClearLastException() override
    {
        return nullptr;
    }

    bool TriggerFatalException(NativeValue* error) override
    {
        return true;
    }

    bool AdjustExternalMemory(int64_t ChangeInBytes, int64_t* AdjustedValue) override
    {
        return true;
    }

    void PrintStatisticResult() override
    {}

    void StartRuntimeStat() override
    {}

    void StopRuntimeStat() override
    {}

    void NotifyApplicationState(bool inBackground) override
    {}

    void NotifyIdleTime(int idleMicroSec) override
    {}

    void NotifyMemoryPressure(bool inHighMemoryPressure = false) override
    {}

    void RegisterUncaughtExceptionHandler(UncaughtExceptionCallback callback) override
    {}

    void HandleUncaughtException() override
    {}

    void RegisterPermissionCheck(PermissionCheckCallback callback) override
    {}

    bool ExecutePermissionCheck() override
    {
        return true;
    }
    
    void DumpHeapSnapshot(bool isVmMode = true, DumpFormat dumpFormat = DumpFormat::JSON,
        bool isPrivate = false) override
    {}

    void DumpHeapSnapshot(const std::string& path, bool isVmMode = true,
        DumpFormat dumpFormat = DumpFormat::JSON) override
    {}

    size_t GetArrayBufferSize() override
    {
        return 0;
    }

    size_t GetHeapTotalSize() override
    {
        return 0;
    }

    size_t GetHeapUsedSize() override
    {
        return 0;
    }

    void AllowCrossThreadExecution() const override
    {}
};

#endif /* FOUNDATION_ABILITY_RUNTIME_MOCK_NATIVE_ENGINE_H */
