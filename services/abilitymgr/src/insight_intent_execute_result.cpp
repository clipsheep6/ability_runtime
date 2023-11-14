/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "insight_intent_execute_result.h"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;
bool InsightIntentExecuteResult::ReadFromParcel(Parcel &parcel)
{
    innerErr = parcel.ReadInt32();
    code = parcel.ReadInt32();
    result = std::shared_ptr<WantParams>(parcel.ReadParcelable<WantParams>());
    return true;
}

bool InsightIntentExecuteResult::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(innerErr)) {
        return false;
    }
    if (!parcel.WriteInt32(code)) {
        return false;
    }
    if (!parcel.WriteParcelable(result.get())) {
        return false;
    }
    return true;
}

InsightIntentExecuteResult *InsightIntentExecuteResult::Unmarshalling(Parcel &parcel)
{
    auto res = new (std::nothrow) InsightIntentExecuteResult();
    if (res == nullptr) {
        return nullptr;
    }

    if (!res->ReadFromParcel(parcel)) {
        delete res;
        res = nullptr;
    }
    return res;
}

bool InsightIntentExecuteResult::CheckResult(std::shared_ptr<const WantParams> result)
{
    return true;
}
} // namespace AppExecFwk
} // namespace OHOS
