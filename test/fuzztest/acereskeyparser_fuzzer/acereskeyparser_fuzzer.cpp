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

#include "acereskeyparser_fuzzer.h"
#include "base/resource/ace_res_key_parser.h"

namespace OHOS::Ace {
constexpr size_t enumNum = 10;
bool GetString(const uint8_t* data, size_t size)
{
    std::string randomString(reinterpret_cast<const char*>(data), size);
    auto str = AceResKeyParser::GetInstance().GetMccByValue(reinterpret_cast<uint32_t>(randomString.c_str()));
    str += AceResKeyParser::GetInstance().GetMncByValue(reinterpret_cast<uint32_t>(randomString.c_str()));
    str += AceResKeyParser::GetInstance().GetMncShortLenByValue(reinterpret_cast<uint32_t>(randomString.c_str()));
    str += AceResKeyParser::GetInstance().GetOrientationByType(DeviceOrientation(size % enumNum));
    str += AceResKeyParser::GetInstance().GetColorModeByType(ColorMode(size % enumNum));
    str += AceResKeyParser::GetInstance().GetDeviceByType(DeviceType(size % enumNum));
    str += AceResKeyParser::GetInstance().GetResolutionByType(ResolutionType(size % enumNum));
    str += AceResKeyParser::GetInstance().GetScreenShapeByType(ScreenShape(size % enumNum));
    str += AceResKeyParser::GetInstance().GetScreenLongByType(LongScreenType(size % enumNum));
    str += AceResKeyParser::GetInstance().GetDeclarativeColorModeByType(ColorMode(size % enumNum));
    return str.empty();
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::Ace::GetString(data, size);
    return 0;
}

