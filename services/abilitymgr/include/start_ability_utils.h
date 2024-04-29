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

#ifndef OHOS_ABILITY_RUNTIME_START_ABILITY_UTILS_H
#define OHOS_ABILITY_RUNTIME_START_ABILITY_UTILS_H

#include <memory>
#include <string>

#include "ability_info.h"
#include "extension_ability_info.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
struct StartAbilityInfo {
    static void InitAbilityInfoFromExtension(AppExecFwk::ExtensionAbilityInfo &extensionInfo,
        AppExecFwk::AbilityInfo &abilityInfo);
    static std::shared_ptr<StartAbilityInfo> CreateStartAbilityInfo(const Want &want, int32_t userId,
        int32_t appIndex);

    static std::shared_ptr<StartAbilityInfo> CreateStartExtensionInfo(const Want &want, int32_t userId,
        int32_t appIndex);

    std::string GetAppBundleName() const
    {
        return abilityInfo.applicationInfo.bundleName;
    }

    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ExtensionProcessMode extensionProcessMode = AppExecFwk::ExtensionProcessMode::UNDEFINED;
    int32_t status = ERR_OK;
};

struct StartAbilityUtils {
    static int32_t GetAppIndex(const Want &want, sptr<IRemoteObject> callerToken);
    static bool GetApplicationInfo(const std::string &bundleName, int32_t userId,
        AppExecFwk::ApplicationInfo &appInfo);
    static thread_local std::shared_ptr<StartAbilityInfo> startAbilityInfo;

    static thread_local bool skipCrowTest;
    static thread_local bool skipStartOther;
    static thread_local bool skipErms;
};

struct StartAbilityInfoWrap {
    StartAbilityInfoWrap(const Want &want, int32_t validUserId, int32_t appIndex, bool isExtension = false);
    ~StartAbilityInfoWrap();
};
}
}
#endif // OHOS_ABILITY_RUNTIME_START_ABILITY_UTILS_H