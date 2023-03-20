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

#ifndef FOUNDATION_ABILITY_RUNTIME_MOCK_NATIVE_VALUE_H
#define FOUNDATION_ABILITY_RUNTIME_MOCK_NATIVE_VALUE_H

#include "native_engine/native_value.h"

class MockNativeValue : public NativeValue {
public:
    virtual ~MockNativeValue() {}

    void* GetInterface(int interfaceId) override
    {
        return nullptr;
    }

    bool InstanceOf(NativeValue* obj) override
    {
        return true;
    }

    bool IsArray() override
    {
        return true;
    }

    bool IsArrayBuffer() override
    {
        return true;
    }

    bool IsDate() override
    {
        return true;
    }

    bool IsError() override
    {
        return true;
    }

    bool IsTypedArray() override
    {
        return true;
    }

    bool IsDataView() override
    {
        return true;
    }

    bool IsPromise() override
    {
        return true;
    }

    bool IsCallable() override
    {
        return true;
    }

    bool IsArgumentsObject() override
    {
        return true;
    }

    bool IsAsyncFunction() override
    {
        return true;
    }

    bool IsBooleanObject() override
    {
        return true;
    }

    bool IsGeneratorFunction() override
    {
        return true;
    }

    bool IsMapIterator() override
    {
        return true;
    }

    bool IsSetIterator() override
    {
        return true;
    }

    bool IsGeneratorObject() override
    {
        return true;
    }

    bool IsModuleNamespaceObject() override
    {
        return true;
    }

    bool IsProxy() override
    {
        return true;
    }

    bool IsRegExp() override
    {
        return true;
    }

    bool IsNumberObject() override
    {
        return true;
    }

    bool IsMap() override
    {
        return true;
    }

    bool IsBuffer() override
    {
        return true;
    }

    bool IsStringObject() override
    {
        return true;
    }

    bool IsSymbolObject() override
    {
        return true;
    }

    bool IsWeakMap() override
    {
        return true;
    }

    bool IsWeakSet() override
    {
        return true;
    }

    bool IsSet() override
    {
        return true;
    }

    bool IsBigInt64Array() override
    {
        return true;
    }

    bool IsBigUint64Array() override
    {
        return true;
    }

    bool IsSharedArrayBuffer() override
    {
        return true;
    }

    bool StrictEquals(NativeValue* value) override
    {
        return true;
    }

    NativeValue* ToBoolean() override
    {
        return nullptr;
    }

    NativeValue* ToNumber() override
    {
        return nullptr;
    }

    NativeValue* ToString() override
    {
        return nullptr;
    }

    NativeValue* ToObject() override
    {
        return nullptr;
    }

    NativeValueType TypeOf() override
    {
        return NativeValueType::NATIVE_UNDEFINED;
    }
};
#endif // FOUNDATION_ABILITY_RUNTIME_MOCK_NATIVE_VALUE_H