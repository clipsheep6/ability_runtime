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

#include "js_zidl_test_service.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

JSZidlTestService::JSZidlTestService() = default;

JSZidlTestService::~JSZidlTestService() = default;

ErrCode JSZidlTestService::voidVoid()
{
    HILOG_INFO("mxh JSZidlTestService::voidVoid called");
    return 0;
}

ErrCode JSZidlTestService::booleanVoid(bool& result)
{
    HILOG_INFO("mxh JSZidlTestService::booleanVoid called");
    result = true;
    return 0;
}

ErrCode JSZidlTestService::byteVoid(int8_t& result)
{
    HILOG_INFO("mxh JSZidlTestService::byteVoid called");
    result = 1;
    return 0;
}

ErrCode JSZidlTestService::shortVoid(short& result)
{
    HILOG_INFO("mxh JSZidlTestService::shortVoid called");
    result = 222;
    return 0;
}

ErrCode JSZidlTestService::intVoid(int& result)
{
    HILOG_INFO("mxh JSZidlTestService::intVoid called");
    result = 333;
    return 0;
}

ErrCode JSZidlTestService::longVoid(long& result)
{
    HILOG_INFO("mxh JSZidlTestService::longVoid called");
    result = 4444;
    return 0;
}

ErrCode JSZidlTestService::floatVoid(float& result)
{
    HILOG_INFO("mxh JSZidlTestService::floatVoid called");
    result = 10.1;
    return 0;
}

ErrCode JSZidlTestService::doubleVoid(double& result)
{
    HILOG_INFO("mxh JSZidlTestService::doubleVoid called");
    result = 22.1;
    return 0;
}

ErrCode JSZidlTestService::stringVoid(std::string& result)
{
    HILOG_INFO("mxh JSZidlTestService::stringVoid called");
    result = "stringVoid";
    return 0;
}

ErrCode JSZidlTestService::mapStringStringVoid(std::unordered_map<std::string, std::string>& result)
{
    HILOG_INFO("mxh JSZidlTestService::mapStringStringVoid called");
    result["string1"] = "string2";
    return 0;
}

ErrCode JSZidlTestService::doubleArrayVoid(std::vector<double>& result)
{
    HILOG_INFO("mxh JSZidlTestService::doubleArrayVoid called");
    result.push_back(1.1);
    result.push_back(2.1);
    result.push_back(3.1);
    return 0;
}

ErrCode JSZidlTestService::voidInMapStringInt(const std::unordered_map<std::string, int>& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidInMapStringInt called");

    auto itr = _param.find("string1");
    if (itr != _param.end())
    {
        if (itr->second == 1)
        {
            return 0;
        }
    }

    return 1;
}
  
ErrCode JSZidlTestService::voidInStringArray(const std::vector<std::string>& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidInStringArray called");
    if ((_param[0] == "aaa") && (_param[1] == "bbb") && (_param[2] == "ccc"))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

ErrCode JSZidlTestService::voidOutMapStringInt(std::unordered_map<std::string, int>& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidOutMapStringInt called");
    _param["voidOutMapStringInt"] = 1;
    return 0;
}
  
ErrCode JSZidlTestService::voidOutStringArray(std::vector<std::string>& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidOutStringArray called");
    _param.push_back("aa");
    _param.push_back("dd");
    return 0;
}

ErrCode JSZidlTestService::voidInInt(int _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidInInt called");
    if (_param == 8888)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

ErrCode JSZidlTestService::voidInString(const std::string& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidInString called");
    if (_param == "voidInString")
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

ErrCode JSZidlTestService::voidOutInt(int& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidOutInt called");

    _param = 123;
    return 0;
}

ErrCode JSZidlTestService::voidOutString(std::string& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidOutString called");
    _param = "voidOutString";
    return 0;
}

ErrCode JSZidlTestService::voidParameterTypeBoolean(bool _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidParameterTypeBoolean called");

    if (_param == true)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

ErrCode JSZidlTestService::voidParameterTypeByte(int8_t _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidParameterTypeByte called");

    if (_param == 11)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

ErrCode JSZidlTestService::voidParameterTypeShort(short _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidParameterTypeShort called");

    if (_param == 2222)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
  
ErrCode JSZidlTestService::voidParameterTypeInt(int _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidParameterTypeInt called");

    if (_param == 3333)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

ErrCode JSZidlTestService::voidParameterTypeLong(long _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidParameterTypeLong called");

    if (_param == 4444)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
 
ErrCode JSZidlTestService::voidParameterTypeFloat(float _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidParameterTypeFloat called %{public}f", _param);

    if (_param == 5555.1)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

ErrCode JSZidlTestService::voidParameterTypeDouble(double _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidParameterTypeDouble called");

    if (_param == 6666.1)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

ErrCode JSZidlTestService::voidParameterTypeString(const std::string& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidParameterTypeString called");

    if (_param == "voidParameterTypeString")
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
    
ErrCode JSZidlTestService::voidParameterTypeMapStringString(const std::unordered_map<std::string, std::string>& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidParameterTypeMapStringString called");

    auto itr = _param.find("voidParameterTypeMapStringString");
    if (itr != _param.end())
    {
        if (itr->second == "fdhatdh")
        {
            return 0;
        }
    }

    return 1;
}
    
ErrCode JSZidlTestService::voidParameterTypeDoubleArray(const std::vector<double>& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidParameterTypeDoubleArray called");

    if ((_param[0] == 1.1) && (_param[1] == 2.1))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

ErrCode JSZidlTestService::voidOrderOutIn(std::string& _paramS, int _paramI)
{
    HILOG_INFO("mxh JSZidlTestService::voidOrderOutIn called");

    if (_paramI == 61)
    {
        _paramS = "hello world";
        return 0;
    }
    else
    {
        return 1;
    }
}

}  // namespace AbilityRuntime
}  // namespace OHOS