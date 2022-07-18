/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_MODULE_SEARCHER_H
#define OHOS_ABILITY_RUNTIME_JS_MODULE_SEARCHER_H

#include <string>

namespace OHOS {
namespace AbilityRuntime {
class JsModuleSearcher final {
public:
    explicit JsModuleSearcher(const std::string& bundleName) : bundleName_(bundleName) {}
    ~JsModuleSearcher() = default;

    JsModuleSearcher(const JsModuleSearcher&) = default;
    JsModuleSearcher(JsModuleSearcher&&) = default;
    JsModuleSearcher& operator=(const JsModuleSearcher&) = default;
    JsModuleSearcher& operator=(JsModuleSearcher&&) = default;

    std::string operator()(const std::string& curJsModulePath, const std::string& newJsModuleUri) const;

private:
    static void FixExtName(std::string& path);
    static std::string GetInstallPath(const std::string& curJsModulePath, bool module = true);
    static std::string MakeNewJsModulePath(const std::string& curJsModulePath, const std::string& newJsModuleUri);
    static std::string FindNpmPackageInPath(const std::string& npmPath);
    static std::string FindNpmPackageInTopLevel(
        const std::string& moduleInstallPath, const std::string& npmPackage, size_t start = 0);
    static std::string FindNpmPackage(const std::string& curJsModulePath, const std::string& npmPackage);

    std::string ParseOhmUri(const std::string& curJsModulePath, const std::string& newJsModuleUri) const;

    std::string bundleName_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_MODULE_SEARCHER_H