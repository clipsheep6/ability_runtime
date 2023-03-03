/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_DATA_ABILITY_OBSERVER_STUB_DATAOBS_MGR_INNER_EXT_TEST_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_DATA_ABILITY_OBSERVER_STUB_DATAOBS_MGR_INNER_EXT_TEST_H
#include <atomic>
#include "gmock/gmock.h"
#include "semaphore_ex.h"
#include "data_ability_observer_stub.h"
#include "event_handler.h"
#include "dataobs_mgr_inner_ext.h"

namespace OHOS {
namespace DataObsMgrInnerExtTest {
using namespace AAFwk;
class MockDataAbilityObserverStub : public AAFwk::DataAbilityObserverStub {
public:
    void OnChange() override
    {
    }

    void OnChangeExt(const ChangeInfo &changeInfo) override
    {
        onChangeCall_ += changeInfo.uris_.size();
        changeInfo_ = changeInfo;
        if (func) {
            func();
        }
    }

    void ReSet()
    {
        onChangeCall_.store(0);
        changeInfo_ = {};
    }

    void Wait()
    {
        std::unique_lock<std::mutex> lck(mtx_);
        while (!flag_) {
            cv_.wait(lck);
        }
    }

    void Notify()
    {
        std::unique_lock<std::mutex> lck(mtx_);
        flag_ = true;
        cv_.notify_one();
    }

private:
    std::atomic_int onChangeCall_ = 0;
    ChangeInfo changeInfo_;
    std::mutex mtx_;
    std::condition_variable cv_;
    bool flag_ = false;
    std::function<void(void)> func;
};

}  // namespace AAFwk
}  // namespace OHOS

#endif  // UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_DATA_ABILITY_OBSERVER_STUB_CALL_H
