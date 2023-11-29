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

#include "quick_fix_callback_with_record.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
QuickFixCallbackWithRecord::~QuickFixCallbackWithRecord()
{
    HILOG_DEBUG("destroyed.");
}

void QuickFixCallbackWithRecord::OnLoadPatchDone(int32_t resultCode, int32_t recordId)
{
    HILOG_DEBUG("function called.");
    ProcessCallback(resultCode, recordId);
    if (IsRecordListEmpty() && callback_ != nullptr) {
        callback_->OnLoadPatchDone(finalResult.load(), recordId);
    }
}

void QuickFixCallbackWithRecord::OnUnloadPatchDone(int32_t resultCode, int32_t recordId)
{
    HILOG_DEBUG("function called.");
    ProcessCallback(resultCode, recordId);
    if (IsRecordListEmpty() && callback_ != nullptr) {
        callback_->OnUnloadPatchDone(finalResult.load(), recordId);
    }
}

void QuickFixCallbackWithRecord::OnReloadPageDone(int32_t resultCode, int32_t recordId)
{
    HILOG_DEBUG("function called.");
    ProcessCallback(resultCode, recordId);
    if (IsRecordListEmpty() && callback_ != nullptr) {
        callback_->OnReloadPageDone(finalResult.load(), recordId);
    }
}

void QuickFixCallbackWithRecord::ProcessCallback(int32_t resultCode, int32_t recordId)
{
    if (!IsRecordExist(recordId)) {
        HILOG_DEBUG("Record id %{public}d didn't exist.", recordId);
        return;
    }
    RemoveRecordId(recordId);

    if (resultCode != 0) {
        finalResult.store(resultCode);
    }
}

void QuickFixCallbackWithRecord::AddRecordId(const int32_t recordId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    recordIds_.emplace_back(recordId);
}

void QuickFixCallbackWithRecord::RemoveRecordId(const int32_t recordId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = recordIds_.begin(); it != recordIds_.end();) {
        if (*it == recordId) {
            it = recordIds_.erase(it);
            return;
        }
        it++;
    }
}

bool QuickFixCallbackWithRecord::IsRecordExist(const int32_t recordId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find(recordIds_.begin(), recordIds_.end(), recordId);
    return (it != recordIds_.end()) ? true : false;
}

bool QuickFixCallbackWithRecord::IsRecordListEmpty()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return recordIds_.empty();
}
} // namespace AAFwk
} // namespace OHOS
