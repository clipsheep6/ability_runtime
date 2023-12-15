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

#ifndef OHOS_FORM_FWK_APP_MODULE_CHECKER_H
#define OHOS_FORM_FWK_APP_MODULE_CHECKER_H

#include <unordered_map>
#include <unordered_set>

#include "module_checker_delegate.h"

namespace {
    constexpr int32_t EXTENSION_TYPE_UNKNOWN = 255;
}

/**
 * @brief Form module load checker. check whether module can be loaded in form
 *
 */
class AppModuleChecker : public ModuleCheckerDelegate {
public:
    AppModuleChecker(int32_t extensionType, std::unordered_map<int32_t, std::unordered_set<std::string>> &&blocklist)
        : processExtensionType_(extensionType), moduleBlocklist_(std::move(blocklist)) {}
    ~AppModuleChecker() override = default;

    bool CheckModuleLoadable(const char* moduleName,
        std::unique_ptr<ApiAllowListChecker>& apiAllowListChecker) override;
    bool DiskCheckOnly() override;
protected:
    int32_t processExtensionType_{EXTENSION_TYPE_UNKNOWN};
    std::unordered_map<int32_t, std::unordered_set<std::string>> moduleBlocklist_;
};

#endif /* OHOS_FORM_FWK_APP_MODULE_CHECKER_H */