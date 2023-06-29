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

#ifndef OHOS_ABILITY_RUNTIME_SIMULAOTR_ABILITY_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_SIMULAOTR_ABILITY_CONTEXT_H

#include "foundation/ability/ability_runtime/frameworks/simulator/common/include/context.h"
#include "uv.h"

namespace OHOS {
namespace AbilityRuntime {
class AbilityContext : public Context {
public:
    AbilityContext() = default;
    virtual ~AbilityContext() = default;

    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() override;

    Options GetOptions() override;
    void SetOptions(const Options& options) override;
    std::string GetBundleName() override;
    std::string GetBundleCodePath() override;
    std::string GetBundleCodeDir() override;
    std::string GetCacheDir() override;
    std::string GetTempDir() override;
    std::string GetFilesDir() override;
    std::string GetDatabaseDir() override;
    std::string GetPreferencesDir() override;
    std::string GetDistributedFilesDir() override;
    void SwitchArea(int mode) override;
    int GetArea() override;
    std::string GetBaseDir() override;
    std::string GetPreviewPath();
    bool Access(const std::string& path);
    void Mkdir(const std::string& path);
    static void fs_req_cleanup(uv_fs_t* req);
    bool CreateMutiDir(const std::string &path);

private:
    static const int EL_DEFAULT = 1;
    Options options_;
    std::string currArea_ = "el2";
    std::string fileSeparator_ = "/";
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SIMULAOTR_ABILITY_CONTEXT_H
