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

#include "ecmascript/tooling/base/pt_base64.h"

namespace panda::ecmascript::tooling {
static const uint8_t DECODE_STR_LEN = 3;
static const uint8_t ENCODE_STR_LEN = 4;
static const uint8_t INVAILD_STR = 255;
static uint8_t decodeMap[] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62, 255, 255, 255, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255, 255, 255, 255, 255,
    255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255,
    255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 255, 255, 255, 255, 255
};
uint32_t PtBase64::Decode(const std::string &input, std::string &output)
{
    size_t srcLen = input.size();
    if (srcLen < ENCODE_STR_LEN || srcLen % ENCODE_STR_LEN != 0) {
        return 0;
    }

    std::string strDecode;
    uint32_t decodeLen = srcLen / ENCODE_STR_LEN * DECODE_STR_LEN;
    strDecode.resize(decodeLen);

    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t equalsCnt = 0;
    const char* src = input.data();
    char base64Char[ENCODE_STR_LEN];
    while (srcLen--) {
        if (*src != '=') {
            auto const v = decodeMap[static_cast<uint8_t>(*src)];
            if (v == INVAILD_STR) {
                return 0;
            }
            base64Char[j++] = v;
        } else {
            base64Char[j++] = '\0';
            equalsCnt++;
        }
        if (j == ENCODE_STR_LEN) {
            // 2: shift 2bits, 4: shift 4bits
            strDecode[i] = (base64Char[0] << 2) | ((base64Char[1] & 0x30) >> 4);
            // 2: shift 2bits, 4: shift 4bits
            strDecode[i + 1] = (base64Char[1] << 4) | ((base64Char[2] & 0x3c) >> 2);
            // 2: shift 2bits, 3: the last encode str, 6: shift 6bits
            strDecode[i + 2] = (base64Char[2] << 6) | base64Char[3];
            j = 0;
            i += DECODE_STR_LEN;
        }
        src++;
    }
    decodeLen -= equalsCnt;
    strDecode.resize(decodeLen);
    output = std::move(strDecode);
    return decodeLen;
}
}  // namespace panda::ecmascript::tooling