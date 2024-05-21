#include "mock_native_token.h"

namespace OHOS {
namespace AppExecFwk {
void MockNativeToken::SetNativeToken()
{
    int permission = 0;
    int permission1 = 1;
    int permission2 = 2;
    int permission3 = 3;
    int permission4 = 4;
    int permission5 = 5;
    uint64_t tokenId;
    const char** perms = new const char* [6];
    perms[permission] = "ohos.permission.DISTRIBUTED_DATASYNC";
    perms[permission1] = "ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS";
    perms[permission2] = "ohos.permission.UPDATE_CONFIGURATION";
    perms[permission3] = "ohos.permission.GET_RUNNING_INFO";
    perms[permission4] = "ohos.permission.MANAGE_MISSIONS";
    perms[permission5] = "ohos.permission.CLEAN_BACKGROUND_PROCESSES";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 6,
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