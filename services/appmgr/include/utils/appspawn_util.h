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

#ifndef OHOS_ABILITY_RUNTIME_APPSPAWN_UTIL_H
#define OHOS_ABILITY_RUNTIME_APPSPAWN_UTIL_H

#include "ability_info.h"
#include "app_spawn_client.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
namespace AppspawnUtil {
constexpr const char* DLP_PARAMS_INDEX = "ohos.dlp.params.index";

static uint32_t BuildStartFlags(const AAFwk::Want &want, const ApplicationInfo &applicationInfo)
{
    uint32_t startFlags = 0x0;
    if (want.GetBoolParam("coldStart", false)) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::COLD_START);
    }

    if (want.GetIntParam(DLP_PARAMS_INDEX, 0) != 0) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::DLP_MANAGER);
    }

    if (applicationInfo.debug) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::DEBUGGABLE);
    }
    if (applicationInfo.asanEnabled) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::ASANENABLED);
    }
    if (want.GetBoolParam("nativeDebug", false)) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::NATIVEDEBUG);
    }
    if (applicationInfo.gwpAsanEnabled) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::GWP_ENABLED_FORCE);
    }
    if (applicationInfo.isSystemApp) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::GWP_ENABLED_NORMAL);
    }
    if (applicationInfo.tsanEnabled) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::TSANENABLED);
    }

    return startFlags;
}

static uint32_t BuildStartFlags(const AAFwk::Want &want, const AbilityInfo &abilityInfo)
{
    uint32_t startFlags = BuildStartFlags(want, abilityInfo.applicationInfo);

    if (abilityInfo.extensionAbilityType == ExtensionAbilityType::BACKUP) {
        startFlags = startFlags | (START_FLAG_BASE << StartFlags::BACKUP_EXTENSION);
    }

    return startFlags;
}
}  // namespace AppspawnUtil
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_APPSPAWN_UTIL_H
