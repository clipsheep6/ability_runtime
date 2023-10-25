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

#ifndef OHOS_ABILITY_RUNTIME_INTENT_EXECUTE_PARAM_H
#define OHOS_ABILITY_RUNTIME_INTENT_EXECUTE_PARAM_H

#include <string>
#include <vector>

#include "parcel.h"
#include "want_params.h"
namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;
/**
 * @enum ExecuteMode
 * ExecuteMode defines the supported execute mode.
 */
enum ExecuteMode {
    UIABLITY_FOREGROUND = 0,
    UIABLITY_BACKGROUND,
    UIEXTENSIONABLITY
};

class InsightIntentExecuteParam : public Parcelable {
public:
    InsightIntentExecuteParam() = default;
    ~InsightIntentExecuteParam() = default;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static InsightIntentExecuteParam *Unmarshalling(Parcel &parcel);

    std::string bundleName_;
    std::string moduleName_;
    std::string abilityName_;
    std::string insightIntentName_;
    std::shared_ptr<WantParams> insightIntentParam_;
    int32_t executeMode_ = -1;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_INTENT_EXECUTE_PARAM_H
