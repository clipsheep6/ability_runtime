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

#ifndef OHOS_ABILITYRUNTIME_IZIDLINTEROPERATETEST_H
#define OHOS_ABILITYRUNTIME_IZIDLINTEROPERATETEST_H

#include <vector>
#include <unordered_map>
#include <string_ex.h>
#include <cstdint>
#include <iremote_broker.h>

namespace OHOS {
namespace AbilityRuntime {
class IZidlInteroperateTest : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.AbilityRuntime.IZidlInteroperateTest");

    virtual ErrCode voidVoid() = 0;

    virtual ErrCode booleanVoid(
        /* [out] */ bool& result) = 0;

    virtual ErrCode byteVoid(
        /* [out] */ int8_t& result) = 0;

    virtual ErrCode shortVoid(
        /* [out] */ short& result) = 0;

    virtual ErrCode intVoid(
        /* [out] */ int& result) = 0;

    virtual ErrCode longVoid(
        /* [out] */ long& result) = 0;

    virtual ErrCode floatVoid(
        /* [out] */ float& result) = 0;

    virtual ErrCode doubleVoid(
        /* [out] */ double& result) = 0;

    virtual ErrCode stringVoid(
        /* [out] */ std::string& result) = 0;

    virtual ErrCode mapStringStringVoid(
        /* [out] */ std::unordered_map<std::string, std::string>& result) = 0;

    virtual ErrCode doubleArrayVoid(
        /* [out] */ std::vector<double>& result) = 0;

    virtual ErrCode voidInMapStringInt(
        /* [in] */ const std::unordered_map<std::string, int>& _param) = 0;

    virtual ErrCode voidInStringArray(
        /* [in] */ const std::vector<std::string>& _param) = 0;

    virtual ErrCode voidOutMapStringInt(
        /* [out] */ std::unordered_map<std::string, int>& _param) = 0;

    virtual ErrCode voidOutStringArray(
        /* [out] */ std::vector<std::string>& _param) = 0;

    virtual ErrCode voidInOutMapStringInt(
        /* [out] */ std::unordered_map<std::string, int>& _param) = 0;

    virtual ErrCode voidInOutStringArray(
        /* [out] */ std::vector<std::string>& _param) = 0;

    virtual ErrCode voidInInt(
        /* [in] */ int _param) = 0;

    virtual ErrCode voidInString(
        /* [in] */ const std::string& _param) = 0;

    virtual ErrCode voidOutInt(
        /* [out] */ int& _param) = 0;

    virtual ErrCode voidOutString(
        /* [out] */ std::string& _param) = 0;

    virtual ErrCode voidInOutInt(
        /* [in, out] */ int _param) = 0;

    virtual ErrCode voidInOutString(
        /* [in, out] */ const std::string& _param) = 0;

    virtual ErrCode voidParameterTypeBoolean(
        /* [in] */ bool _param) = 0;

    virtual ErrCode voidParameterTypeByte(
        /* [in] */ int8_t _param) = 0;

    virtual ErrCode voidParameterTypeShort(
        /* [in] */ short _param) = 0;

    virtual ErrCode voidParameterTypeInt(
        /* [in] */ int _param) = 0;

    virtual ErrCode voidParameterTypeLong(
        /* [in] */ long _param) = 0;

    virtual ErrCode voidParameterTypeFloat(
        /* [in] */ float _param) = 0;

    virtual ErrCode voidParameterTypeDouble(
        /* [in] */ double _param) = 0;

    virtual ErrCode voidParameterTypeString(
        /* [in] */ const std::string& _param) = 0;

    virtual ErrCode voidParameterTypeMapStringString(
        /* [in] */ const std::unordered_map<std::string, std::string>& _param) = 0;

    virtual ErrCode voidParameterTypeDoubleArray(
        /* [in] */ const std::vector<double>& _param) = 0;

    virtual ErrCode voidOrderOutIn(
        /* [out] */ std::string& _paramS,
        /* [in] */ int _paramI) = 0;

    virtual ErrCode voidOrderInOutIn(
        /* [in, out] */ int _paramI,
        /* [in] */ const std::string& _paramS) = 0;

    virtual ErrCode voidOrderOutInOut(
        /* [out] */ int& _paramI,
        /* [in, out] */ const std::string& _paramS) = 0;

    virtual ErrCode voidOrderInInOutOut(
        /* [in] */ const std::string& _paramS,
        /* [in, out] */ int _paramI,
        /* [out] */ std::string& _paramS2) = 0;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITYRUNTIME_IZIDLINTEROPERATETEST_H

