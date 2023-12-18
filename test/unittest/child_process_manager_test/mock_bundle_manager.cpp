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

#include <gtest/gtest.h>

#include "mock_bundle_manager.h"
#include "bundle_info.h"
#include "extension_ability_info.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
    constexpr const int32_t UID = 2001;
}
ErrCode BundleMgrService::GetBundleInfoForSelf(int32_t flags, BundleInfo &bundleInfo)
{
    std::vector<HapModuleInfo> hapModuleInfos;
    HapModuleInfo moduleInfo;
    moduleInfo.name = "entry";
    moduleInfo.moduleName = "entry";
    moduleInfo.moduleType = AppExecFwk::ModuleType::ENTRY;
    moduleInfo.hapPath = "/data/app/el1/bundle/public/com.ohos.demoprocess/entry";
    moduleInfo.compileMode = AppExecFwk::CompileMode::ES_MODULE;
    moduleInfo.isStageBasedModel = true;
    hapModuleInfos.push_back(moduleInfo);
    bundleInfo.hapModuleInfos = hapModuleInfos;

    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.uid = UID;
    bundleInfo.applicationInfo = applicationInfo;

    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
