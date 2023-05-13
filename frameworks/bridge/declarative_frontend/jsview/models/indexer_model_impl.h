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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_FORM_MODEL_IMPL_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_FORM_MODEL_IMPL_H

#include "core/components_ng/pattern/indexer/indexer_model.h"

namespace OHOS::Ace::Framework {
class IndexerModelImpl : public OHOS::Ace::IndexerModel {
public:
    void Create(std::vector<std::string>& indexerArray, int32_t selectedVal) override;
    void SetSelectedColor(const Color& color) override;
    void SetColor(const Color& color) override;
    void SetPopupColor(const Color& color) override;
    void SetSelectedBackgroundColor(const Color& color) override;
    void SetPopupBackground(const Color& color) override;
    void SetUsingPopup(bool state) override;
    void SetSelectedFont(const TextStyle& textStyle) override;
    void SetPopupFont(const TextStyle& textStyle) override;
    void SetFont(const TextStyle& textStyle) override;
    void SetItemSize(const Dimension& value) override;
    void SetAlignStyle(int32_t value) override;
    void SetFocusable(bool focusable) override;
    void SetFocusNode(bool isFocusNode) override;
    void SetOnSelected(std::function<void(const int32_t selected)>&& onSelect) override;
    void SetOnRequestPopupData(
        std::function<std::vector<std::string>(const int32_t selected)>&& RequestPopupData) override;
    void SetOnPopupSelected(std::function<void(const int32_t selected)>&& onPopupSelected) override;
};
} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_FORM_MODEL_IMPL_H
