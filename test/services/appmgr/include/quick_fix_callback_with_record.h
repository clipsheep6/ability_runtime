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

#ifndef OHOS_ABILITY_RUNTIME_QUICK_FIX_CALLBACK_WITH_RECORD_H
#define OHOS_ABILITY_RUNTIME_QUICK_FIX_CALLBACK_WITH_RECORD_H

#include <atomic>
#include <list>
#include <mutex>

#include "quick_fix_callback_stub.h"

namespace OHOS {
namespace AppExecFwk {
class QuickFixCallbackWithRecord : public QuickFixCallbackStub {
public:
    explicit QuickFixCallbackWithRecord(sptr<IQuickFixCallback> callback)
        : callback_(callback)
    {}

    ~QuickFixCallbackWithRecord() override;

    void OnLoadPatchDone(int32_t resultCode, int32_t recordId) override;
    void OnUnloadPatchDone(int32_t resultCode, int32_t recordId) override;
    void OnReloadPageDone(int32_t resultCode, int32_t recordId) override;

    void AddRecordId(int32_t recordId);
    void RemoveRecordId(int32_t recordId);

private:
    void ProcessCallback(int32_t resultCode, int32_t recordId);
    bool IsRecordExist(const int32_t recordId);
    bool IsRecordListEmpty();

    sptr<IQuickFixCallback> callback_ = nullptr;
    std::mutex mutex_;
    std::list<int32_t> recordIds_;
    std::atomic<int32_t> finalResult = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_QUICK_FIX_CALLBACK_WITH_RECORD_H
