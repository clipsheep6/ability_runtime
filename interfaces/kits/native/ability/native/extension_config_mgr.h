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

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_CONFIG_HANDLER_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_CONFIG_HANDLER_H

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "bundle_info.h"
#include "extension_ability_info.h"
#include "native_engine/native_engine.h"

namespace OHOS::AbilityRuntime {
namespace ExtensionConfigItem {
    constexpr char ITEM_NAME_BLOCKLIST[] = "blocklist";
}

/**
 * @brief Manage extension configuration.
 */
class ExtensionConfigMgr {
public:
    ExtensionConfigMgr() = default;

    virtual ~ExtensionConfigMgr() = default;

    /**
     * @brief ExtensionConfigMgr initialization
     *
     */
    void Init();

    /**
     * @brief Update bundle extension information
     *
     * @param engine JS NativeEngine
     */
    void UpdateBundleExtensionInfo(NativeEngine &engine, AppExecFwk::BundleInfo &bundleInfo);

    /**
     * @brief Add extension blocklist item
     *
     * @param name Extension name
     * @param type Extension type
     */
    void AddBlockListItem(const std::string &name, int32_t type);

    /**
     * @brief Update extension blocklist to native engine
     *
     * @param engine JS NativeEngine
     */
    void UpdateBlockListToEngine(NativeEngine &engine);

private:
    std::unordered_map<std::string, std::unordered_set<std::string>> blocklistConfig_;
    std::unordered_map<int32_t, std::unordered_set<std::string>> extensionBlocklist_;
};
} // namespace OHOS::AbilityRuntime

#endif // OHOS_ABILITY_RUNTIME_EXTENSION_CONFIG_HANDLER_H
