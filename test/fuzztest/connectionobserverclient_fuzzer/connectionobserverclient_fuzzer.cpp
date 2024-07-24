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

#include "connectionobserverclient_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "connection_observer_client.h"
#include "service_proxy_adapter.h"
#undef protected
#undef private

#include "ability_record.h"
#include "continuous_task_callback_info.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;

class MyConnectionObserver : public ConnectionObserver {
public:
    MyConnectionObserver() = default;
    virtual ~MyConnectionObserver() = default;
    void OnExtensionConnected(const ConnectionData& data) override
    {}
    void OnExtensionDisconnected(const ConnectionData& data) override
    {}
    void OnDlpAbilityOpened(const DlpStateData& data) override
    {}
    void OnDlpAbilityClosed(const DlpStateData& data) override
    {}
    void OnServiceDied() override
    {}
};

class MyAbilityConnectionObserver : public IConnectionObserver {
public:
    MyAbilityConnectionObserver() = default;
    virtual ~MyAbilityConnectionObserver() = default;
    void OnExtensionConnected(const ConnectionData& data) override
    {}
    void OnExtensionDisconnected(const ConnectionData& data) override
    {}
#ifdef WITH_DLP
    void OnDlpAbilityOpened(const DlpStateData& data) override
    {}
    void OnDlpAbilityClosed(const DlpStateData& data) override
    {}
#endif // WITH_DLP
    sptr<IRemoteObject> AsObject() override
    {
        return {};
    }
};
}

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    std::shared_ptr<ConnectionObserver> observer = std::make_shared<MyConnectionObserver>();
    // fuzz for connectionObserverClient
    auto connectionObserverClient = std::make_shared<ConnectionObserverClient>();
    connectionObserverClient->UnregisterObserver(observer);
    sptr<IRemoteObject> remoteObj;
    auto serviceProxyAdapter = std::make_shared<ServiceProxyAdapter>(remoteObj);
    sptr<AbilityRuntime::IConnectionObserver> cobserver = new MyAbilityConnectionObserver();
    serviceProxyAdapter->UnregisterObserver(cobserver);
    serviceProxyAdapter->GetProxyObject();
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE || size > OHOS::FOO_MAX_LEN) {
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

