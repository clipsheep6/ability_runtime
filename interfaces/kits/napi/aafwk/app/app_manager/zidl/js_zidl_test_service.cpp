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
    HILOG_INFO("ACTS_Zidl_Tool_0500 voidVoid called");
    return 0;
}

ErrCode JSZidlTestService::booleanVoid(bool& result)
{
    HILOG_INFO("mxh JSZidlTestService::booleanVoid called");
    HILOG_INFO("ACTS_Zidl_Tool_0600 voidVoid called");
    result = true;
    return 0;
}

ErrCode JSZidlTestService::byteVoid(int8_t& result)
{
    HILOG_INFO("mxh JSZidlTestService::voidVoid called");
    HILOG_INFO("ACTS_Zidl_Tool_0700 voidVoid called");
    result = 1;
    return 0;
}

ErrCode JSZidlTestService::shortVoid(short& result)
{
    HILOG_INFO("mxh JSZidlTestService::voidVoid called");
    HILOG_INFO("ACTS_Zidl_Tool_0800 voidVoid called");
    result = 222;
    return 0;
}

ErrCode JSZidlTestService::intVoid(int& result)
{
    HILOG_INFO("mxh JSZidlTestService::intVoid called");
    HILOG_INFO("ACTS_Zidl_Tool_0900 voidVoid called");
    result = 333;
    return 0;
}

ErrCode JSZidlTestService::longVoid(long& result)
{
    HILOG_INFO("mxh JSZidlTestService::longVoid called");
    HILOG_INFO("ACTS_Zidl_Tool_1000 voidVoid called");
    result = 4444;
    return 0;
}

ErrCode JSZidlTestService::floatVoid(float& result)
{
    HILOG_INFO("mxh JSZidlTestService::floatVoid called");
    HILOG_INFO("ACTS_Zidl_Tool_1100 voidVoid called");
    result = 10.1;
    return 0;
}

ErrCode JSZidlTestService::doubleVoid(double& result)
{
    HILOG_INFO("mxh JSZidlTestService::doubleVoid called");
    HILOG_INFO("ACTS_Zidl_Tool_1200 voidVoid called");
    result = 22.1;
    return 0;
}

ErrCode JSZidlTestService::stringVoid(std::string& result)
{
    HILOG_INFO("mxh JSZidlTestService::stringVoid called");
    HILOG_INFO("ACTS_Zidl_Tool_1300 voidVoid called");
    result = "stringVoid";
    return 0;
}

ErrCode JSZidlTestService::mapStringStringVoid(std::unordered_map<std::string, std::string>& result)
{
    HILOG_INFO("mxh JSZidlTestService::mapStringStringVoid called");
    HILOG_INFO("ACTS_Zidl_Tool_1600 voidVoid called");
    result["string1"] = "string2";
    return 0;
}

ErrCode JSZidlTestService::doubleArrayVoid(std::vector<double>& result)
{
    HILOG_INFO("mxh JSZidlTestService::doubleArrayVoid called");
    HILOG_INFO("ACTS_Zidl_Tool_1700 voidVoid called");
    result.push_back(1.1);
    result.push_back(2.1);
    result.push_back(3.1);
    return 0;
}

ErrCode JSZidlTestService::voidInMapStringInt(const std::unordered_map<std::string, int>& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidInMapStringInt called");
    HILOG_INFO("ACTS_Zidl_Tool_1800_1 voidVoid called");

    // std::unordered_map<std::string, int>::iterator itr;
    auto itr = _param.find("string1");
    if (itr != _param.end())
    {
        if (itr->second == 1)
        {
            return 0;
        }
    }

    return 1;
    // int temp = _param["string1"];
    // if (_param.find("string1") != _param.end())
    // {
    //     return 0;
    // }
}
  
ErrCode JSZidlTestService::voidInStringArray(const std::vector<std::string>& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidInStringArray called");
    HILOG_INFO("ACTS_Zidl_Tool_1800_2 voidVoid called");
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
    HILOG_INFO("ACTS_Zidl_Tool_1900_1 voidVoid called");
    _param["voidOutMapStringInt"] = 1;
    return 0;
}
  
ErrCode JSZidlTestService::voidOutStringArray(std::vector<std::string>& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidOutStringArray called");
    HILOG_INFO("ACTS_Zidl_Tool_1900_2 voidVoid called");
    _param.push_back("aa");
    _param.push_back("dd");
    return 0;
}

ErrCode JSZidlTestService::voidInOutMapStringInt(std::unordered_map<std::string, int>& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidInOutMapStringInt called");
    HILOG_INFO("ACTS_Zidl_Tool_2000_1 voidVoid called");
    _param["voidInOutMapStringInt"] = 1;
    return 0;
}
  
ErrCode JSZidlTestService::voidInOutStringArray(std::vector<std::string>& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidInOutStringArray called");
    HILOG_INFO("ACTS_Zidl_Tool_2000_2 voidVoid called");
    _param.push_back("aaa");
    _param.push_back("bbb");

    return 0;
}

ErrCode JSZidlTestService::voidInInt(int _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidInInt called");
    HILOG_INFO("ACTS_Zidl_Tool_2100_1 voidVoid called");
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
    HILOG_INFO("ACTS_Zidl_Tool_2100_2 voidVoid called");
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
    HILOG_INFO("ACTS_Zidl_Tool_2200_1 voidVoid called");

    _param = 123;
    return 0;
}

ErrCode JSZidlTestService::voidOutString(std::string& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidOutString called");
    HILOG_INFO("ACTS_Zidl_Tool_2200_2 voidVoid called");
    _param = "voidOutString";
    return 0;
}

ErrCode JSZidlTestService::voidInOutInt(int _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidInOutInt called");
    HILOG_INFO("ACTS_Zidl_Tool_2300_1 voidVoid called");

    if (_param == 123456)
    {
        _param = 456789;
    }
    return 0;
}

ErrCode JSZidlTestService::voidInOutString(const std::string& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidInOutString called");
    HILOG_INFO("ACTS_Zidl_Tool_2300_2 voidVoid called");

    if (_param == "123456")
    {
       // _param = "456789";
    }
    return 0;
}

ErrCode JSZidlTestService::voidParameterTypeBoolean(bool _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidParameterTypeBoolean called");
    HILOG_INFO("ACTS_Zidl_Tool_2400 voidVoid called");

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
    HILOG_INFO("ACTS_Zidl_Tool_2500 voidVoid called");

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
    HILOG_INFO("ACTS_Zidl_Tool_2600 voidVoid called");

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
    HILOG_INFO("ACTS_Zidl_Tool_2700 voidVoid called");

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
    HILOG_INFO("ACTS_Zidl_Tool_2800 voidVoid called");

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
    HILOG_INFO("ACTS_Zidl_Tool_2900 voidVoid called");

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
    HILOG_INFO("ACTS_Zidl_Tool_3000 voidVoid called");

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
    HILOG_INFO("ACTS_Zidl_Tool_3100 voidVoid called");

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
    HILOG_INFO("ACTS_Zidl_Tool_3400 voidVoid called");

    auto itr = _param.find("voidParameterTypeMapStringString");
    if (itr != _param.end())
    {
        if (itr->second == "fdhatdh")
        {
            return 0;
        }
    }

    return 1;

    // if (_param["voidParameterTypeMapStringString"] == "fdhatdh")
    // {
    //     return 0;
    // }
    // else
    // {
    //     return 1;
    // }


    // std::string temp = _param["voidParameterTypeMapStringString"];
    // if (temp == "fdhatdh")
//     if (_param.find("voidParameterTypeMapStringString") != _param.end())
//     {
//         return 0;
//     }

//     return 1;
}
    
ErrCode JSZidlTestService::voidParameterTypeDoubleArray(const std::vector<double>& _param)
{
    HILOG_INFO("mxh JSZidlTestService::voidParameterTypeDoubleArray called");
    HILOG_INFO("ACTS_Zidl_Tool_3500 voidVoid called");

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
    HILOG_INFO("ACTS_Zidl_Tool_6100 voidVoid called");

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

ErrCode JSZidlTestService::voidOrderInOutIn(int _paramI, const std::string& _paramS)
{
    HILOG_INFO("mxh JSZidlTestService::voidOrderInOutIn called");
    HILOG_INFO("ACTS_Zidl_Tool_6200 voidVoid called");

    if (_paramI == 62)
    {
        // _paramS = "hello world";
        return 0;
    }
    else
    {
        return 1;
    }
}
    
ErrCode JSZidlTestService::voidOrderOutInOut(int& _paramI, const std::string& _paramS)
{
    HILOG_INFO("mxh JSZidlTestService::voidOrderOutInOut called");
    HILOG_INFO("ACTS_Zidl_Tool_6300 voidVoid called");

    _paramI = 3;
    // _paramS = "paramcallback"
    return 0;
}

ErrCode JSZidlTestService::voidOrderInInOutOut(const std::string& _paramS, int _paramI, std::string& _paramS2)
{
    HILOG_INFO("mxh JSZidlTestService::voidOrderInInOutOut called");
    HILOG_INFO("ACTS_Zidl_Tool_6400 voidVoid called");

    if ((_paramI == 64) && (_paramS == "paramInput"))
    {
        _paramS2 = "callback";
        return 0;
    }
    else
    {
        return 1;
    }
}

}  // namespace AbilityRuntime
}  // namespace OHOS