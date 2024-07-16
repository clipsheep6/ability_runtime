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


#ifndef OHOS_ABILITY_RUNTIME_MEDIA_PERMISSION_MANAGER_H
#define OHOS_ABILITY_RUNTIME_MEDIA_PERMISSION_MANAGER_H

#include <sys/types.h>
#include <vector>

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "tokenid_permission.h"

namespace OHOS {
namespace AAFwk {
class MediaPermissionManager {
public:
    static std::vector<bool> CheckUriPermission(std::vector<Uri> &uriVec, uint32_t callerTokenId, uint32_t flag)
    {
        std::vector<bool> result(uriVec.size(), false);
        TokenIdPermission tokenIdPermission(callerTokenId);
        bool isWriteFlag = (flag & Want::FLAG_AUTH_WRITE_URI_PERMISSION) != 0;
        for (size_t i = 0; i < uriVec.size(); i++) {
            auto path = uriVec[i].GetPath();
            if (path.rfind("/Photo/", 0) == 0) {
                if (tokenIdPermission.VerifyWriteImageVideoPermission()) {
                    result[i] = true;
                    continue;
                }
                if (!isWriteFlag && tokenIdPermission.VerifyReadImageVideoPermission()) {
                    result[i] = true;
                    continue;
                }
                TAG_LOGI(AAFwkTag::URIPERMMGR, "Do not have IMAGEVIDEO Permission.");
            }
            if (path.rfind("/Audio/", 0) == 0) {
                if (tokenIdPermission.VerifyWriteAudioPermission()) {
                    result[i] = true;
                    continue;
                }
                if (!isWriteFlag && tokenIdPermission.VerifyReadAudioPermission()) {
                    result[i] = true;
                    continue;
                }
                TAG_LOGI(AAFwkTag::URIPERMMGR, "Do not have Audio Permission.");
            }
        }
        return result;
    }
};

} // OHOS
} // AAFwk
#endif // OHOS_ABILITY_RUNTIME_MEDIA_PERMISSION_MANAGER_H