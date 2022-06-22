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

#include "core/components_ng/render/adapter/rosen_window.h"

#include "transaction/rs_interfaces.h"

#include "base/thread/task_executor.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/common/thread_checker.h"
#include "core/components_ng/render/adapter/rosen_render_context.h"

namespace OHOS::Ace::NG {

RosenWindow::RosenWindow(const OHOS::sptr<OHOS::Rosen::Window>& window, RefPtr<TaskExecutor> taskExecutor, int32_t id)
    : rsWindow_(window), taskExecutor_(taskExecutor), id_(id)
{
    auto& rsClient = OHOS::Rosen::RSInterfaces::GetInstance();
    vsyncReceiver_ = rsClient.CreateVSyncReceiver("ACE");
    vsyncReceiver_->Init();
    rsUIDirector_ = OHOS::Rosen::RSUIDirector::Create();
    rsUIDirector_->SetRSSurfaceNode(window->GetSurfaceNode());
    rsUIDirector_->Init();
    rsUIDirector_->SetUITaskRunner([taskExecutor, id](const std::function<void()>& task) {
        ContainerScope scope(id);
        if (taskExecutor) {
            taskExecutor->PostTask(task, TaskExecutor::TaskType::UI);
        }
    });
}

void RosenWindow::RequestFrame()
{
    if (!onShow_) {
        LOGD("window is not show, stop request frame");
        return;
    }
    CHECK_RUN_ON(UI);
    OHOS::Rosen::VSyncReceiver::FrameCallback callback = {
        .userData_ = nullptr,
        .callback_ =
            [weakTask = taskExecutor_, id = id_](int64_t nanoTimestamp, void* data) {
                ContainerScope scope(id);
                auto onVsync = [nanoTimestamp] {
                    // use container to get window can make sure the window is valid
                    auto container = Container::Current();
                    CHECK_NULL_VOID(container);
                    auto window = container->GetWindow();
                    CHECK_NULL_VOID(window);
                    window->OnVsync(static_cast<uint64_t>(nanoTimestamp), 0);
                };
                auto taskExecutor = weakTask.Upgrade();
                if (taskExecutor) {
                    taskExecutor->PostTask(onVsync, TaskExecutor::TaskType::UI);
                }
            },
    };
    if (!isRequestVsync_) {
        LOGD("request next vsync");
        vsyncReceiver_->RequestNextVSync(callback);
        isRequestVsync_ = true;
    }
}

void RosenWindow::Destroy()
{
    LOG_DESTROY();
    rsWindow_ = nullptr;
    rootNode_.Reset();
    vsyncReceiver_.reset();
    rsUIDirector_->Destroy();
    rsUIDirector_.reset();
    callbacks_.clear();
}

void RosenWindow::SetRootFrameNode(const RefPtr<NG::FrameNode>& root)
{
    LOGI("Rosenwindow set root frame node");
    CHECK_NULL_VOID(root);
    rootNode_ = root;
    auto rosenRenderContext = AceType::DynamicCast<RosenRenderContext>(rootNode_->GetRenderContext());
    CHECK_NULL_VOID(rosenRenderContext);
    if (rosenRenderContext->GetRSNode()) {
        rsUIDirector_->SetRoot(rosenRenderContext->GetRSNode()->GetId());
    }
}

void RosenWindow::RecordFrameTime(uint64_t timeStamp, const std::string name)
{
    LOGD("Rosenwindow RecordFrameTime");
    rsUIDirector_->SetTimeStamp(timeStamp, name);
}

void RosenWindow::FlushTasks()
{
    CHECK_RUN_ON(UI);
    LOGD("Rosenwindow flush tasks");
    rsUIDirector_->SendMessages();
}

} // namespace OHOS::Ace::NG
