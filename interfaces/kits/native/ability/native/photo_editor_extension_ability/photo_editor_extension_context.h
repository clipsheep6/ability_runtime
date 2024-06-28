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

#ifndef OHOS_ABILITY_RUNTIME_PHOTO_EDITOR_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_PHOTO_EDITOR_EXTENSION_CONTEXT_H

#include "ui_extension_context.h"
#include "pixel_map_napi.h"
#include "image_packer.h"

namespace OHOS {
namespace AbilityRuntime {

enum class PhotoEditorErrorCode {
    // success
    ERROR_OK = 0,

    // param error
    ERROR_CODE_PARAM_ERROR = 401,

    // internal error
    ERROR_CODE_INTERNAL_ERROR = 29600001,

    // image input error
    ERROR_CODE_IMAGE_INPUT_ERROR = 29600002,

    // image too big
    ERROR_CODE_IMAGE_TOO_BIG_ERROR = 29600003
};

class PhotoEditorExtensionContext : public UIExtensionContext {
public:
    PhotoEditorExtensionContext() = default;

    virtual ~PhotoEditorExtensionContext() = default;

    /**
     * @brief Create Extension

     * @param newWant newWant
     * @return errCode
     */
    PhotoEditorErrorCode SaveEditedContent(const std::string &uri, AAFwk::Want &newWant);

    /**
     * @brief Save content editing

     * @param image image
     * @param option option
     * @param newWant newWant
     * @return errCode
     */
    PhotoEditorErrorCode SaveEditedContent(const std::shared_ptr<OHOS::Media::PixelMap> &image,
                                           const Media::PackOption &packOption, AAFwk::Want &newWant);

    void SetWant(const std::shared_ptr<AAFwk::Want> &want);

    static const size_t CONTEXT_TYPE_ID;

private:
    PhotoEditorErrorCode CopyImageToPanel(const std::string &imageUri, const std::string &panelUri);
    std::string GetRealPath(std::string &uri);

private:
    std::shared_ptr<AAFwk::Want> want_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_PHOTO_EDITOR_EXTENSION_CONTEXT_H