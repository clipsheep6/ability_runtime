/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "zidl_interoperate_test_stub.h"

namespace OHOS {
namespace AbilityRuntime {
int ZidlInteroperateTestStub::OnRemoteRequest(
    /* [in] */ uint32_t code,
    /* [in] */ MessageParcel& data,
    /* [out] */ MessageParcel& reply,
    /* [in] */ MessageOption& option)
{
    switch (code) {
        case COMMAND_VOID_VOID: {
            ErrCode ec = voidVoid();
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        case COMMAND_BOOLEAN_VOID: {
            bool result;
            ErrCode ec = booleanVoid(result);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt32(result ? 1 : 0);
            }
            return ERR_NONE;
        }
        case COMMAND_BYTE_VOID: {
            int8_t result;
            ErrCode ec = byteVoid(result);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt32(result);
            }
            return ERR_NONE;
        }
        case COMMAND_SHORT_VOID: {
            short result;
            ErrCode ec = shortVoid(result);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt32(result);
            }
            return ERR_NONE;
        }
        case COMMAND_INT_VOID: {
            int result;
            ErrCode ec = intVoid(result);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt32(result);
            }
            return ERR_NONE;
        }
        case COMMAND_LONG_VOID: {
            long result;
            ErrCode ec = longVoid(result);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt64(result);
            }
            return ERR_NONE;
        }
        case COMMAND_FLOAT_VOID: {
            float result;
            ErrCode ec = floatVoid(result);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteFloat(result);
            }
            return ERR_NONE;
        }
        case COMMAND_DOUBLE_VOID: {
            double result;
            ErrCode ec = doubleVoid(result);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteDouble(result);
            }
            return ERR_NONE;
        }
        case COMMAND_STRING_VOID: {
            std::string result;
            ErrCode ec = stringVoid(result);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteString16(Str8ToStr16(result));
            }
            return ERR_NONE;
        }
        case COMMAND_MAP_STRING_STRING_VOID: {
            std::unordered_map<std::string, std::string> result;
            ErrCode ec = mapStringStringVoid(result);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt32(result.size());
                for (auto it = result.begin(); it != result.end(); ++it) {
                    reply.WriteString16(Str8ToStr16((it->first)));
                    reply.WriteString16(Str8ToStr16((it->second)));
                }
            }
            return ERR_NONE;
        }
        case COMMAND_DOUBLE_ARRAY_VOID: {
            std::vector<double> result;
            ErrCode ec = doubleArrayVoid(result);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt32(result.size());
                for (auto it = result.begin(); it != result.end(); ++it) {
                    reply.WriteDouble((*it));
                }
            }
            return ERR_NONE;
        }
        case COMMAND_VOID_IN_MAP_STRING_INT: {
            std::unordered_map<std::string, int> _param;
            int _paramSize = data.ReadInt32();
            for (int i = 0; i < _paramSize; ++i) {
                std::string key = Str16ToStr8(data.ReadString16());
                int value = data.ReadInt32();
                _param[key] = value;
            }
            ErrCode ec = voidInMapStringInt(_param);
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        case COMMAND_VOID_IN_STRING_ARRAY: {
            std::vector<std::string> _param;
            int _paramSize = data.ReadInt32();
            for (int i = 0; i < _paramSize; ++i) {
                std::string value = Str16ToStr8(data.ReadString16());
                _param.push_back(value);
            }
            ErrCode ec = voidInStringArray(_param);
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        case COMMAND_VOID_OUT_MAP_STRING_INT: {
            std::unordered_map<std::string, int> _param;
            ErrCode ec = voidOutMapStringInt(_param);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt32(_param.size());
                for (auto it = _param.begin(); it != _param.end(); ++it) {
                    reply.WriteString16(Str8ToStr16((it->first)));
                    reply.WriteInt32((it->second));
                }
            }
            return ERR_NONE;
        }
        case COMMAND_VOID_OUT_STRING_ARRAY: {
            std::vector<std::string> _param;
            ErrCode ec = voidOutStringArray(_param);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt32(_param.size());
                for (auto it = _param.begin(); it != _param.end(); ++it) {
                    reply.WriteString16(Str8ToStr16((*it)));
                }
            }
            return ERR_NONE;
        }
        case COMMAND_VOID_IN_OUT_MAP_STRING_INT: {
            std::unordered_map<std::string, int> _param;
            ErrCode ec = voidInOutMapStringInt(_param);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt32(_param.size());
                for (auto it = _param.begin(); it != _param.end(); ++it) {
                    reply.WriteString16(Str8ToStr16((it->first)));
                    reply.WriteInt32((it->second));
                }
            }
            return ERR_NONE;
        }
        case COMMAND_VOID_IN_OUT_STRING_ARRAY: {
            std::vector<std::string> _param;
            ErrCode ec = voidInOutStringArray(_param);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt32(_param.size());
                for (auto it = _param.begin(); it != _param.end(); ++it) {
                    reply.WriteString16(Str8ToStr16((*it)));
                }
            }
            return ERR_NONE;
        }
        case COMMAND_VOID_IN_INT: {
            int _param = data.ReadInt32();
            ErrCode ec = voidInInt(_param);
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        case COMMAND_VOID_IN_STRING: {
            std::string _param = Str16ToStr8(data.ReadString16());
            ErrCode ec = voidInString(_param);
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        case COMMAND_VOID_OUT_INT: {
            int _param;
            ErrCode ec = voidOutInt(_param);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt32(_param);
            }
            return ERR_NONE;
        }
        case COMMAND_VOID_OUT_STRING: {
            std::string _param;
            ErrCode ec = voidOutString(_param);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteString16(Str8ToStr16(_param));
            }
            return ERR_NONE;
        }
        case COMMAND_VOID_IN_OUT_INT: {
            int _param = data.ReadInt32();
            ErrCode ec = voidInOutInt(_param);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt32(_param);
            }
            return ERR_NONE;
        }
        case COMMAND_VOID_IN_OUT_STRING: {
            std::string _param = Str16ToStr8(data.ReadString16());
            ErrCode ec = voidInOutString(_param);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteString16(Str8ToStr16(_param));
            }
            return ERR_NONE;
        }
        case COMMAND_VOID_PARAMETER_TYPE_BOOLEAN: {
            bool _param = data.ReadInt32() == 1 ? true : false;
            ErrCode ec = voidParameterTypeBoolean(_param);
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        case COMMAND_VOID_PARAMETER_TYPE_BYTE: {
            int8_t _param = (int8_t)data.ReadInt32();
            ErrCode ec = voidParameterTypeByte(_param);
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        case COMMAND_VOID_PARAMETER_TYPE_SHORT: {
            short _param = (short)data.ReadInt32();
            ErrCode ec = voidParameterTypeShort(_param);
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        case COMMAND_VOID_PARAMETER_TYPE_INT: {
            int _param = data.ReadInt32();
            ErrCode ec = voidParameterTypeInt(_param);
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        case COMMAND_VOID_PARAMETER_TYPE_LONG: {
            long _param = data.ReadInt64();
            ErrCode ec = voidParameterTypeLong(_param);
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        case COMMAND_VOID_PARAMETER_TYPE_FLOAT: {
            float _param = data.ReadFloat();
            ErrCode ec = voidParameterTypeFloat(_param);
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        case COMMAND_VOID_PARAMETER_TYPE_DOUBLE: {
            double _param = data.ReadDouble();
            ErrCode ec = voidParameterTypeDouble(_param);
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        case COMMAND_VOID_PARAMETER_TYPE_STRING: {
            std::string _param = Str16ToStr8(data.ReadString16());
            ErrCode ec = voidParameterTypeString(_param);
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        case COMMAND_VOID_PARAMETER_TYPE_MAP_STRING_STRING: {
            std::unordered_map<std::string, std::string> _param;
            int _paramSize = data.ReadInt32();
            for (int i = 0; i < _paramSize; ++i) {
                std::string key = Str16ToStr8(data.ReadString16());
                std::string value = Str16ToStr8(data.ReadString16());
                _param[key] = value;
            }
            ErrCode ec = voidParameterTypeMapStringString(_param);
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        case COMMAND_VOID_PARAMETER_TYPE_DOUBLE_ARRAY: {
            std::vector<double> _param;
            int _paramSize = data.ReadInt32();
            for (int i = 0; i < _paramSize; ++i) {
                double value = data.ReadDouble();
                _param.push_back(value);
            }
            ErrCode ec = voidParameterTypeDoubleArray(_param);
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        case COMMAND_VOID_ORDER_OUT_IN: {
            std::string _paramS;
            int _paramI = data.ReadInt32();
            ErrCode ec = voidOrderOutIn(_paramS, _paramI);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteString16(Str8ToStr16(_paramS));
            }
            return ERR_NONE;
        }
        case COMMAND_VOID_ORDER_IN_OUT_IN: {
            int _paramI = data.ReadInt32();
            std::string _paramS = Str16ToStr8(data.ReadString16());
            ErrCode ec = voidOrderInOutIn(_paramI, _paramS);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt32(_paramI);
            }
            return ERR_NONE;
        }
        case COMMAND_VOID_ORDER_OUT_IN_OUT: {
            int _paramI;
            std::string _paramS = Str16ToStr8(data.ReadString16());
            ErrCode ec = voidOrderOutInOut(_paramI, _paramS);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt32(_paramI);
                reply.WriteString16(Str8ToStr16(_paramS));
            }
            return ERR_NONE;
        }
        case COMMAND_VOID_ORDER_IN_IN_OUT_OUT: {
            std::string _paramS = Str16ToStr8(data.ReadString16());
            int _paramI = data.ReadInt32();
            std::string _paramS2;
            ErrCode ec = voidOrderInInOutOut(_paramS, _paramI, _paramS2);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt32(_paramI);
                reply.WriteString16(Str8ToStr16(_paramS));
            }
            return ERR_NONE;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return ERR_TRANSACTION_FAILED;
}
} // namespace AbilityRuntime
} // namespace OHOS
