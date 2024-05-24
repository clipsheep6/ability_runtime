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

#include "uripermissionmanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "uri_permission_manager_stub.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.aafwk.UriPermissionManager";
}

class UriPermissionManagerStubFuzzTest : public UriPermissionManagerStub {
public:
    UriPermissionManagerStubFuzzTest() = default;
    virtual ~UriPermissionManagerStubFuzzTest()
    {}
    int GrantUriPermission(const Uri &uri, unsigned int flag,
        const std::string targetBundleName, int32_t appIndex = 0, uint32_t initiatorTokenId = 0) override
    {
        return 0;
    }
    int GrantUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
        const std::string targetBundleName, int32_t appIndex = 0, uint32_t initiatorTokenId = 0) override
    {
        return 0;
    }
    int32_t GrantUriPermissionPrivileged(const std::vector<Uri> &uriVec, uint32_t flag,
        const std::string &targetBundleName, int32_t appIndex = 0) override
    {
        return 0;
    }
    int GrantUriPermissionFor2In1(const std::vector<Uri> &uriVec, unsigned int flag,
        const std::string &targetBundleName, int32_t appIndex = 0, bool isSystemAppCall = false) override
    {
        return 0;
    }
    void RevokeUriPermission(const Security::AccessToken::AccessTokenID tokenId) override
    {
        return;
    }
    int RevokeUriPermissionManually(const Uri &uri, const std::string bundleName) override
    {
        return 0;
    }
    bool VerifyUriPermission(const Uri& uri, uint32_t flag, uint32_t tokenId) override
    {
        return false;
    }
    std::vector<bool> CheckUriAuthorization(const std::vector<std::string> &uriVec,
        uint32_t flag, uint32_t tokenId) override
    {
        return std::vector<bool>();
    }
    bool IsAuthorizationUriAllowed(uint32_t fromTokenId) override
    {
        return false;
    }
    int RevokeAllUriPermissions(const Security::AccessToken::AccessTokenID tokenId) override
    {
        return 0;
    }
};

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    uint32_t code = GetU32Data(data);

    MessageParcel parcel;
    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    parcel.WriteBuffer(data, size);
    parcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    sptr<UriPermissionManagerStub> uripermissionMgr = new UriPermissionManagerStubFuzzTest();
    uripermissionMgr->OnRemoteRequest(code, parcel, reply, option);

    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    /* Validate the length of size */
    if (size > OHOS::FOO_MAX_LEN || size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = (char*)malloc(size + 1);
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}