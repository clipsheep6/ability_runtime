#include "mock_native_token.h"

namespace OHOS {
namespace AppExecFwk {
void MockNativeToken::SetNativeToken()
{
    int permission = 0;
    uint64_t tokenId;
    const char** perms = new const char* [1];
    perms[permission] = "ohos.permission.SET_ABILITY_CONTROLLER";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .aplStr = "system_core",
    };

    infoInstance.processName = "SetUpTestCase";
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    delete[] perms;
}
}  // namespace AppExecFwk
}  // namespace OHOS