/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_FORM_RESOURCE_FORM_MANAGER_DELEGATE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_FORM_RESOURCE_FORM_MANAGER_DELEGATE_H

#include <list>

#include "interfaces/inner_api/form_render/include/form_renderer_delegate_impl.h"
#include "interfaces/inner_api/form_render/include/form_renderer_dispatcher_interface.h"
#include "core/components/common/layout/constants.h"
#include "core/components/form/resource/form_manager_resource.h"
#include "core/components/form/resource/form_request_data.h"
#include "core/pipeline/pipeline_base.h"

#ifdef OHOS_STANDARD_SYSTEM
#include "form_js_info.h"
#include "ui/rs_surface_node.h"
#include "want.h"
#include "want_params_wrapper.h"

#include "core/components/form/resource/form_utils.h"
#endif

namespace OHOS::Ace {
class FormCallbackClient;
class FormSurfaceCallbackClient;
class FormManagerDelegate : public FormManagerResource {
    DECLARE_ACE_TYPE(FormManagerDelegate, FormManagerResource);

public:
    using onFormAcquiredCallbackForJava =
        std::function<void(int64_t, const std::string&, const std::string&, const std::string&)>;
    using OnFormUpdateCallbackForJava = std::function<void(int64_t, const std::string&)>;
    using OnFormAcquiredCallback = std::function<void(int64_t, const std::string&, const std::string&,
        const std::string&, const std::map<std::string, sptr<AppExecFwk::FormAshmem>>&,
        const AppExecFwk::FormJsInfo&, const FrontendType& frontendType, const FrontendType& uiSyntax)>;
    using OnFormUpdateCallback =
        std::function<void(int64_t, const std::string&, const std::map<std::string, sptr<AppExecFwk::FormAshmem>>&)>;
    using OnFormErrorCallback = std::function<void(const std::string&, const std::string&)>;
    using OnFormUninstallCallback = std::function<void(int64_t)>;
    using OnFormSurfaceNodeCallback = std::function<void(const std::shared_ptr<Rosen::RSSurfaceNode>&)>;

    enum class State : char {
        WAITINGFORSIZE,
        CREATING,
        CREATED,
        CREATEFAILED,
        RELEASED,
    };

    FormManagerDelegate() = delete;
    ~FormManagerDelegate() override;
    explicit FormManagerDelegate(const WeakPtr<PipelineBase>& context)
        : FormManagerResource("formAdaptor", context), state_(State::WAITINGFORSIZE)
    {
        renderDelegate_ = std::make_shared<FormRendererDelegateImpl>();
    }

    void AddForm(const WeakPtr<PipelineBase>& context, const RequestFormInfo& info);
    void ReleasePlatformResource();

    void AddFormAcquireCallback(const OnFormAcquiredCallback& callback);
    void AddFormUpdateCallback(const OnFormUpdateCallback& callback);
    void AddFormErrorCallback(const OnFormErrorCallback& callback);
    void AddFormUninstallCallback(const OnFormUninstallCallback& callback);
    void AddFormSurfaceNodeCallback(const OnFormSurfaceNodeCallback& callback);

    void OnActionEvent(const std::string& action);
    void DispatchPointerEvent(
        const std::shared_ptr<MMI::PointerEvent>& pointerEvent);
    void RegisterRenderDelegateEvent();
    void OnFormError(const std::string& code, const std::string& msg);
#ifdef OHOS_STANDARD_SYSTEM
    void ProcessFormUpdate(const AppExecFwk::FormJsInfo& formJsInfo);
    void ProcessFormUninstall(const int64_t formId);
    void OnDeathReceived();
    void SetFormUtils(const std::shared_ptr<FormUtils>& formUtils);
    void ProcessAddFormSurface(
        const AppExecFwk::FormJsInfo &formInfo,
        const std::shared_ptr<Rosen::RSSurfaceNode> &rsSurfaceNode,
        const AAFwk::Want& want);
#endif

private:
    void CreatePlatformResource(const WeakPtr<PipelineBase>& context, const RequestFormInfo& info);
    void Stop();
    void RegisterEvent();
    void UnregisterEvent();
    std::string ConvertRequestInfo(const RequestFormInfo& info) const;

    void OnFormAcquired(const std::string& param);
    void OnFormUpdate(const std::string& param);
    void OnFormError(const std::string& param);
    bool ParseAction(const std::string& action, AAFwk::Want& want);

    onFormAcquiredCallbackForJava onFormAcquiredCallbackForJava_;
    OnFormUpdateCallbackForJava onFormUpdateCallbackForJava_;
    OnFormAcquiredCallback onFormAcquiredCallback_;
    OnFormUpdateCallback onFormUpdateCallback_;
    OnFormErrorCallback onFormErrorCallback_;
    OnFormUninstallCallback onFormUninstallCallback_;
    OnFormSurfaceNodeCallback onFormSurfaceNodeCallback_;

    State state_ { State::WAITINGFORSIZE };
#ifdef OHOS_STANDARD_SYSTEM
    int64_t runningCardId_ = -1;
    AAFwk::Want wantCache_;
    bool hasCreated_ = false;
    std::shared_ptr<FormCallbackClient> formCallbackClient_;
    std::shared_ptr<FormUtils> formUtils_;
    std::shared_ptr<FormSurfaceCallbackClient> formSurfaceCallbackClient_;
    std::shared_ptr<FormRendererDelegateImpl> renderDelegate_;
    sptr<IFormRendererDispatcher> formRendererDispatcher_;
#endif
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_FORM_RESOURCE_FORM_MANAGER_DELEGATE_H
