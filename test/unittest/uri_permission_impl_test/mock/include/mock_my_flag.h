/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef MOCK_MY_FLAG_H
#define MOCK_MY_FLAG_H

#include <string>
#include <sys/types.h>
#include <unordered_map>

#include "access_token.h"

namespace OHOS {
namespace AAFwk {
struct TokenInfo;
using MyATokenTypeEnum = Security::AccessToken::ATokenTypeEnum;
using TokenInfoMap = std::unordered_map<uint32_t, TokenInfo>;

struct TokenInfo {
    uint32_t tokenId = 0;
    std::string processName = "";
    std::string bundleName = "";
    MyATokenTypeEnum tokenType = MyATokenTypeEnum::TOKEN_INVALID;
    
    TokenInfo() {}

    TokenInfo(uint32_t tokenId, MyATokenTypeEnum tokenType, std::string processName = "", std::string bundleName = "")
    {
        this->tokenId = tokenId;
        this->tokenType = tokenType;
        this->processName = processName;
        this->bundleName = bundleName;
    }
};


class MyFlag {
public:
    enum FLAG {
        IS_SA_CALL = 1,
        IS_SHELL_CALL,
        IS_SA_AND_SHELL_CALL
    };
    static int flag_;
    static bool isSACall_;
    static bool permissionFileAccessManager_;
    static bool permissionWriteImageVideo_;
    static bool permissionReadImageVideo_;
    static bool permissionWriteAudio_;
    static bool permissionReadAudio_;
    static bool permissionAllMedia_;
    static bool permissionProxyAuthorization_;
    static bool permissionAll_;
    static bool permissionPrivileged_;

    static TokenInfoMap tokenInfos;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // MOCK_MY_FLAG_H