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

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_PLUGIN_INFO_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_PLUGIN_INFO_H

#include <string>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {

struct ExtensionPluginItem {
    int32_t extensionType;
    std::string extensionName;
    std::string extensionLibFile;
};

/**
 * @brief Manage extension plugin info.
 */
class ExtensionPluginInfo {
public:
    static ExtensionPluginInfo& GetInstance();

    /**
     * @brief Destructor.
     *
     */
    virtual ~ExtensionPluginInfo() = default;

    /**
     * @brief Preload extension plugin in app spawn.
     *
     */
    void Preload();

    /**
     * Get extension plugin item info.
     *
     * @return Return all preloaded extension plugin items.
     */
    std::vector<ExtensionPluginItem> GetExtensionPlugins();

private:
    ExtensionPluginInfo();
    ExtensionPluginInfo(const ExtensionPluginInfo&) = delete;
    ExtensionPluginInfo(ExtensionPluginInfo&&) = delete;
    ExtensionPluginInfo& operator=(const ExtensionPluginInfo&) = delete;
    ExtensionPluginInfo& operator=(ExtensionPluginInfo&&) = delete;

    bool ScanExtensions(std::vector<std::string>& files);
    bool CheckFileType(const std::string &fileName, const std::string &extensionName);
    void ParseExtensions(const std::vector<std::string>& extensionFiles);

    std::vector<ExtensionPluginItem> extensionPlugins_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_EXTENSION_PLUGIN_INFO_H
