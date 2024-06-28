/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_PHOTO_EDITOR_EXTENSION_IMPL_H
#define OHOS_ABILITY_RUNTIME_JS_PHOTO_EDITOR_EXTENSION_IMPL_H

#include "js_ui_extension_base.h"
#include "photo_editor_extension_context.h"
#include <set>

namespace OHOS {
namespace AbilityRuntime {

class PhotoEditorExtensionContext;

class JsPhotoEditorExtensionImpl : public JsUIExtensionBase {
public:
    explicit JsPhotoEditorExtensionImpl(const std::unique_ptr<Runtime> &runtime);
    virtual ~JsPhotoEditorExtensionImpl() override = default;

    void SetContext(const std::shared_ptr<PhotoEditorExtensionContext> &context)
    {
        context_ = context;
        JsUIExtensionBase::SetContext(context);
    }

protected:
    void BindContext() override;
    void OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo) override;

private:
    void OnStartContentEditing(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo);

protected:
    std::shared_ptr<PhotoEditorExtensionContext> context_ = nullptr;
    std::set<uint64_t> uiExtensionComponentIdSet_;

private:
    using JsUIExtensionBase::SetContext;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_PHOTO_EDITOR_EXTENSION_IMPL_H