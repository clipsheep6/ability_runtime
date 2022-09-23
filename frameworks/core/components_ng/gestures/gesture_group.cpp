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

#include "core/components_ng/gestures/gesture_group.h"

#include "core/components_ng/gestures/recognizers/exclusive_recognizer.h"
#include "core/components_ng/gestures/recognizers/parallel_recognizer.h"
#include "core/components_ng/gestures/recognizers/sequenced_recognizer.h"

namespace OHOS::Ace::NG {

RefPtr<GestureRecognizer> GestureGroup::CreateRecognizer()
{
    LOGD("create gesture group, sub size %{public}zu, mode %{public}d", gestures_.size(), mode_);
    std::vector<RefPtr<GestureRecognizer>> recognizers;
    for (auto& subGesture : gestures_) {
        auto recognizer = subGesture->CreateRecognizer();
        if (recognizer) {
            recognizers.emplace_back(recognizer);
        }
    }

    RefPtr<GestureRecognizer> groupRecognizer;
    switch (mode_) {
        case GestureMode::Sequence:
            groupRecognizer = AceType::MakeRefPtr<SequencedRecognizer>(recognizers);
            if (onActionCancelId_) {
                groupRecognizer->SetOnActionCancel(*onActionCancelId_);
            }
            break;
        case GestureMode::Parallel:
            groupRecognizer = AceType::MakeRefPtr<ParallelRecognizer>(recognizers);
            break;
        case GestureMode::Exclusive:
            groupRecognizer = AceType::MakeRefPtr<ExclusiveRecognizer>(recognizers);
            break;
        default:
            LOGE("unknown gesture mode %{public}d when create gesture group recognizer", mode_);
            return nullptr;
    }

    groupRecognizer->SetPriority(priority_);
    groupRecognizer->SetPriorityMask(gestureMask_);
    return groupRecognizer;
}

} // namespace OHOS::Ace::NG
