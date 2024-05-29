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

#include "abilitycontext_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "ability_record.h"
#define private public
#include "ability_context.h"
#undef private
#include "want.h"
#include "parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_TWO = 2;
constexpr int INPUT_THREE = 3;
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
constexpr size_t U32_AT_SIZE = 4;
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}
sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;

    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }

    return token;
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    AbilityContext abilityContext;
    // fuzz for want
    Parcel wantParcel;
    Want* want = nullptr;
    if (wantParcel.WriteBuffer(data, size)) {
        want = Want::Unmarshalling(wantParcel);
        if (!want) {
            return false;
        }
    }
    int requestCode = static_cast<int>(GetU32Data(data));
    abilityContext.StartAbility(*want, requestCode);
    sptr<AAFwk::IAbilityConnection> conn = nullptr;
    abilityContext.ConnectAbility(*want, conn);
    abilityContext.StopAbility(*want);
    std::string name(data, size);
    int mode = static_cast<int>(GetU32Data(data));
    abilityContext.GetDir(name, mode);
    std::string bundleName(data, size);
    int flag = static_cast<int>(GetU32Data(data));
    int accountId = static_cast<int>(GetU32Data(data));
    abilityContext.CreateBundleContext(bundleName, flag, accountId);
    std::string permission(data, size);
    int pid = static_cast<int>(GetU32Data(data));
    int uid = static_cast<int>(GetU32Data(data));
    abilityContext.VerifyPermission(permission, pid, uid);
    std::string permissionName(data, size);
    std::string des(data, size);
    abilityContext.GetPermissionDes(permissionName, des);
    std::vector<std::string> permissions;
    std::string fileName(data, size);
    std::string deviceId(data, size);
    std::string abilityName(data, size);
    std::string moduleName(data, size);
    abilityContext.SetCallingContext(deviceId, bundleName, abilityName, moduleName);
    std::shared_ptr<ContextDeal> base = nullptr;
    abilityContext.AttachBaseContext(base);
    std::string type(data, size);
    abilityContext.GetExternalFilesDir(type);
    std::string url(data, size);
    Uri uri = Uri(url);
    abilityContext.UnauthUriPermission(permission, uri, uid);
    int patternId = static_cast<int>(GetU32Data(data));
    abilityContext.SetPattern(patternId);
    BundleInfo bundleInfo;
    std::shared_ptr<ContextDeal> deal = nullptr;
    abilityContext.InitResourceManager(bundleInfo, deal);
    int resId = static_cast<int>(GetU32Data(data));
    abilityContext.GetString(resId);
    abilityContext.GetStringArray(resId);
    abilityContext.GetIntArray(resId);
    int themeId = static_cast<int>(GetU32Data(data));
    abilityContext.SetTheme(themeId);
    abilityContext.GetColor(resId);
    abilityContext.SetColorMode(mode);
    std::vector<AAFwk::Want> wants;
    abilityContext.StartAbilities(wants);
    if (want) {
        delete want;
        want = nullptr;
    }
    return (abilityContext.DisconnectAbility(conn) == 0);
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

