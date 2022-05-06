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

#include "zidl_interoperate_test_stub.h"
#include "native_engine/native_engine.h"

#ifndef OHOS_APPEXECFWK_RUNTIME_JS_ZIDL_TEST_SERVICE_H
#define OHOS_APPEXECFWK_RUNTIME_JS_ZIDL_TEST_SERVICE_H

namespace OHOS {
namespace AbilityRuntime {
class JSZidlTestService : public ZidlInteroperateTestStub {
public:
    JSZidlTestService();
    ~JSZidlTestService();

    ErrCode voidVoid()  override;
    ErrCode booleanVoid(bool& result)  override;
    ErrCode byteVoid(int8_t& result)  override;
    ErrCode shortVoid(short& result)  override;
    ErrCode intVoid(int& result)  override;
    ErrCode longVoid(long& result)  override;
    ErrCode floatVoid(float& result)  override;
    ErrCode doubleVoid(double& result)  override;
    ErrCode stringVoid(std::string& result)  override;
    ErrCode mapStringStringVoid(std::unordered_map<std::string, std::string>& result)  override;
    ErrCode doubleArrayVoid(std::vector<double>& result)  override;
    ErrCode voidInMapStringInt(const std::unordered_map<std::string, int>& _param)  override;
    ErrCode voidInStringArray(const std::vector<std::string>& _param)  override;
    ErrCode voidOutMapStringInt(std::unordered_map<std::string, int>& _param)  override;
    ErrCode voidOutStringArray(std::vector<std::string>& _param)  override;
    ErrCode voidInInt(int _param)  override;
    ErrCode voidInString(const std::string& _param)  override;
    ErrCode voidOutInt(int& _param)  override;
    ErrCode voidOutString(std::string& _param)  override;
    ErrCode voidParameterTypeBoolean(bool _param)  override;
    ErrCode voidParameterTypeByte(int8_t _param)  override;
    ErrCode voidParameterTypeShort(short _param)  override;
    ErrCode voidParameterTypeInt(int _param)  override;
    ErrCode voidParameterTypeLong(long _param)  override;
    ErrCode voidParameterTypeFloat(float _param)  override;
    ErrCode voidParameterTypeDouble(double _param)  override;
    ErrCode voidParameterTypeString(const std::string& _param)  override;
    ErrCode voidParameterTypeMapStringString(const std::unordered_map<std::string, std::string>& _param)  override;
    ErrCode voidParameterTypeDoubleArray(const std::vector<double>& _param)  override;
    ErrCode voidOrderOutIn(std::string& _paramS, int _paramI)  override; 
};

}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_APPEXECFWK_RUNTIME_JS_ZIDL_TEST_SERVICE_H