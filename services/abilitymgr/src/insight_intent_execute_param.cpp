/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "insight_intent_execute_param.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;
bool InsightIntentExecuteParam::ReadFromParcel(Parcel &parcel)
{
    bundleName_ = Str16ToStr8(parcel.ReadString16());
    moduleName_ = Str16ToStr8(parcel.ReadString16());
    abilityName_ = Str16ToStr8(parcel.ReadString16());
    insightIntentName_ = Str16ToStr8(parcel.ReadString16());
    std::shared_ptr<WantParams> wantParams(parcel.ReadParcelable<WantParams>());
    if (wantParams == nullptr) {
        return false;
    }
    insightIntentParam_ = wantParams;
    executeMode_ = parcel.ReadInt32();
    insightIntentId_ = parcel.ReadUint64();
    displayId_ = parcel.ReadInt32();
    return true;
}

InsightIntentExecuteParam *InsightIntentExecuteParam::Unmarshalling(Parcel &parcel)
{
    InsightIntentExecuteParam *param = new (std::nothrow) InsightIntentExecuteParam();
    if (param == nullptr) {
        return nullptr;
    }

    if (!param->ReadFromParcel(parcel)) {
        delete param;
        param = nullptr;
    }
    return param;
}

bool InsightIntentExecuteParam::Marshalling(Parcel &parcel) const
{
    parcel.WriteString16(Str8ToStr16(bundleName_));
    parcel.WriteString16(Str8ToStr16(moduleName_));
    parcel.WriteString16(Str8ToStr16(abilityName_));
    parcel.WriteString16(Str8ToStr16(insightIntentName_));
    parcel.WriteParcelable(insightIntentParam_.get());
    parcel.WriteInt32(executeMode_);
    parcel.WriteUint64(insightIntentId_);
    parcel.WriteInt32(displayId_);
    return true;
}

bool InsightIntentExecuteParam::IsInsightIntentExecute(const AAFwk::Want &want)
{
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_NAME)) {
        return true;
    }
    return false;
}

bool InsightIntentExecuteParam::GenerateFromWant(const AAFwk::Want &want,
    InsightIntentExecuteParam &executeParam)
{
    const WantParams &wantParams = want.GetParams();
    if (!wantParams.HasParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME)) {
        HILOG_ERROR("The want does not contain insight intent name.");
        return false;
    }

    AppExecFwk::ElementName elementName = want.GetElement();
    executeParam.bundleName_ = elementName.GetBundleName();
    executeParam.moduleName_ = elementName.GetModuleName();
    executeParam.abilityName_ = elementName.GetAbilityName();
    executeParam.insightIntentName_ = wantParams.GetStringParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME);
    executeParam.insightIntentId_ = std::stoull(wantParams.GetStringParam(INSIGHT_INTENT_EXECUTE_PARAM_ID));
    executeParam.executeMode_ = wantParams.GetIntParam(INSIGHT_INTENT_EXECUTE_PARAM_MODE, 0);
    executeParam.insightIntentParam_ =
        std::make_shared<WantParams>(wantParams.GetWantParams(INSIGHT_INTENT_EXECUTE_PARAM_PARAM));
    return true;
}

bool InsightIntentExecuteParam::RemoveInsightIntent(AAFwk::Want &want)
{
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_NAME)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME);
    }
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_ID)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_PARAM_ID);
    }
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_MODE)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_PARAM_MODE);
    }
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_PARAM)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_PARAM_PARAM);
    }
    return true;
}
} // namespace AppExecFwk
} // namespace OHOS
