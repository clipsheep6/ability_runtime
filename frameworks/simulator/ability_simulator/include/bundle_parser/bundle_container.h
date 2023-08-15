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

#ifndef OHOS_ABILITY_RUNTIME_SIMULATOR_BUNDLE_CONTAINER_H
#define OHOS_ABILITY_RUNTIME_SIMULATOR_BUNDLE_CONTAINER_H

#include <vector>

#include "application_info.h"
#include "bundle_constants.h"
#include "inner_bundle_info.h"

namespace OHOS {
namespace AppExecFwk {
class BundleContainer {
public:
    BundleContainer() = default;
    ~BundleContainer() = default;

    static BundleContainer &GetInstance();
    void LoadBundleInfos(const std::vector<uint8_t> &buffer);
    std::shared_ptr<ApplicationInfo> GetApplicationInfo() const;
    std::shared_ptr<HapModuleInfo> GetHapModuleInfo(const std::string &modulePackage) const;
    std::shared_ptr<AbilityInfo> GetAbilityInfo(const std::string &moduleName, const std::string &abilityName) const;
    void SetAppCodePath(const std::string &codePath);
private:
    std::shared_ptr<InnerBundleInfo> bundleInfo_ = nullptr;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SIMULATOR_BUNDLE_CONTAINER_H
