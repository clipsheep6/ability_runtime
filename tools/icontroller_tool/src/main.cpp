#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>

#include "ability_controller_stub.h"
#include "ability_manager_client.h"
#include "mock_native_token.h"
#include "singleton.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

// 全局变量，用于限制回调执行多少次后退出
volatile int32_t g_count = 0;

// 定义一个mock的AbilityController及其行为
class MockAbilityController : public AbilityControllerStub
{
public:
    MockAbilityController() = default;
    virtual ~MockAbilityController() = default;
    bool AllowAbilityStart(const Want &want, const std::string &bundleName) override
    {
        cout << "AllowAbilityStart begin" << endl;
        g_count++;
        bool ret = true;
        if (bundleName == "com.acts.example.abilitytest" ||
            bundleName == "com.example.faabilitytest") {
            ret = false;
        }
        cout << "AllowAbilityStart ret:" << ret << endl;
        return ret;
    }
    bool AllowAbilityBackground(const std::string &bundleName) override
    {
        cout << "AllowAbilityBackground begin" << endl;
        g_count++;
        bool ret = true;
        if (bundleName == "com.acts.example.abilitytest" ||
            bundleName == "com.example.faabilitytest") {
            ret = false;
        }
        cout << "AllowAbilityBackground ret:" << ret << endl;
        return ret;
    }
};

int main()
{
    // mock权限，在mock_native_token.cpp中新增
    MockNativeToken::SetNativeToken();

    sptr<AppExecFwk::IAbilityController> controller_ = new MockAbilityController();
    int32_t ret = AbilityManagerClient::GetInstance()->SetAbilityController(controller_, true);
    if (ret != ERR_OK) {
        cout << "Set AbilityController failed, ret:" << ret << endl;
        return 0;
    }
    // 循环，避免可执行程序退出，设置10次回调触发后退出
    while (g_count < 10) {}

    return 0;
}