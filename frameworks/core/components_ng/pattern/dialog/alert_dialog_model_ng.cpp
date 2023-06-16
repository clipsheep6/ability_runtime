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

#include "core/components_ng/pattern/dialog/alert_dialog_model_ng.h"

#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/common/ace_engine.h"

namespace OHOS::Ace::NG {
void AlertDialogModelNG::SetParseButtonObj(
    std::function<void()>&& eventFunc, ButtonInfo& buttonInfo, DialogProperties& arg, const std::string& property)
{
    auto func = eventFunc;
    auto eventFuncNg = [func](GestureEvent& /* info */) { func(); };
    buttonInfo.action = AceType::MakeRefPtr<NG::ClickEvent>(std::move(eventFuncNg));
}

void AlertDialogModelNG::SetOnCancel(std::function<void()>&& eventFunc, DialogProperties& arg)
{
    arg.onCancel = eventFunc;
}

void AlertDialogModelNG::SetShowDialog(const DialogProperties& arg)
{
    auto container = Container::Current();
    auto currentId = Container::CurrentId();
    CHECK_NULL_VOID(container);
    if (container->IsSubContainer()) {
        currentId = SubwindowManager::GetInstance()->GetParentContainerId(currentId);
        container = AceEngine::Get().GetContainer(currentId);
    }
    ContainerScope scope(currentId);
    auto pipelineContext = container->GetPipelineContext();
    CHECK_NULL_VOID(pipelineContext);
    auto context = AceType::DynamicCast<NG::PipelineContext>(pipelineContext);
    CHECK_NULL_VOID(context);
    auto overlayManager = context->GetOverlayManager();
    CHECK_NULL_VOID(overlayManager);

    auto dialog = overlayManager->ShowDialog(arg, nullptr, false);
    CHECK_NULL_VOID(dialog);
    auto hub = dialog->GetEventHub<NG::DialogEventHub>();
    hub->SetOnCancel(std::move(arg.onCancel));
}
} // namespace OHOS::Ace::NG
