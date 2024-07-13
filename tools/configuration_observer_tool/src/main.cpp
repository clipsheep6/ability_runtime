#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>

#include "app_mgr_client.h"
#include "configuration_observer_stub.h"
#include "mock_native_token.h"
#include "singleton.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

volatile int32_t g_count = 0;

// 定义一个mock的ConfigurationObserver及其行为
class MockConfigurationObserver : public ConfigurationObserverStub {
public:
    MockConfigurationObserver() = default;
    virtual ~MockConfigurationObserver() = default;
    void OnConfigurationUpdated(const Configuration& configuration) override
    {
        // 打印回调入参
        g_count++;
        cout << "OnConfigurationUpdated" << endl;
        cout << "ohos.application.direction:" << configuration.GetItem("ohos.application.direction") << endl;
        cout << "ohos.application.densitydpi:" << configuration.GetItem("ohos.application.densitydpi") << endl;
        cout << "ohos.application.displayid:" << configuration.GetItem("ohos.application.displayid") << endl;
        cout << "ohos.application.font:" << configuration.GetItem("ohos.application.font") << endl;
        cout << "ohos.system.language:" << configuration.GetItem("ohos.system.language") << endl;
        cout << "ohos.system.hour:" << configuration.GetItem("ohos.system.hour") << endl;
        cout << "ohos.system.colorMode:" << configuration.GetItem("ohos.system.colorMode") << endl;
        cout << "input.pointer.device:" << configuration.GetItem("input.pointer.device") << endl;
        cout << "const.build.characteristics:" << configuration.GetItem("const.build.characteristics") << endl;
    }
};

int main()
{
    // mock权限，在mock_native_token.cpp中新增
    MockNativeToken::SetNativeToken();

    sptr<IConfigurationObserver> observer_ = new MockConfigurationObserver();
    int32_t ret = DelayedSingleton<AppMgrClient>::GetInstance()->RegisterConfigurationObserver(observer_);
    if (ret != ERR_OK) {
        cout << "RegisterConfigurationObserver failed, ret:" << ret << endl;
        return 0;
    }
    // 循环，避免可执行程序退出
    while (g_count < 4) {}
    ret = DelayedSingleton<AppMgrClient>::GetInstance()->UnregisterConfigurationObserver(observer_);
    if (ret != ERR_OK) {
        cout << "UnregisterConfigurationObserver failed, ret:" << ret << endl;
        return 0;
    }

    return 0;
}