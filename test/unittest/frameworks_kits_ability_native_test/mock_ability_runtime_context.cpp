/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"

#define private public
#define protected public
#include "context.h"
#include "mock_ability_runtime_context.h"
#undef private
#undef protected

namespace OHOS {
namespace AbilityRuntime {
MockAbilityRuntimeContext::MockAbilityRuntimeContext()
{};

std::string MockAbilityRuntimeContext::GetBundleName() const
{
    return {};
};

std::shared_ptr<Context> MockAbilityRuntimeContext::CreateBundleContext(const std::string &bundleName)
{
    return {};
};

std::shared_ptr<AppExecFwk::ApplicationInfo> MockAbilityRuntimeContext::GetApplicationInfo() const
{
    return {};
};

std::shared_ptr<Global::Resource::ResourceManager> MockAbilityRuntimeContext::GetResourceManager() const
{
    return {};
};

std::string MockAbilityRuntimeContext::GetBundleCodePath() const
{
    return {};
};

std::shared_ptr<AppExecFwk::HapModuleInfo> MockAbilityRuntimeContext::GetHapModuleInfo() const
{
    return {};
};

std::string MockAbilityRuntimeContext::GetBundleCodeDir()
{
    return {};
};

std::string MockAbilityRuntimeContext::GetCacheDir()
{
    return {};
};

std::string MockAbilityRuntimeContext::GetTempDir()
{
    return {};
};

std::string MockAbilityRuntimeContext::GetResourceDir()
{
    return {};
};

std::string MockAbilityRuntimeContext::GetFilesDir()
{
    return {};
};

bool MockAbilityRuntimeContext::IsUpdatingConfigurations()
{
    return {};
};

bool MockAbilityRuntimeContext::PrintDrawnCompleted()
{
    return {};
};

std::string MockAbilityRuntimeContext::GetDatabaseDir()
{
    return {};
};

std::string MockAbilityRuntimeContext::GetPreferencesDir()
{
    return {};
};

std::string MockAbilityRuntimeContext::GetDistributedFilesDir()
{
    return {};
};

sptr<IRemoteObject> MockAbilityRuntimeContext::GetToken()
{
    return {};
};

void MockAbilityRuntimeContext::SetToken(const sptr<IRemoteObject> &token)
{};

void MockAbilityRuntimeContext::SwitchArea(int mode)
{};

std::shared_ptr<Context> MockAbilityRuntimeContext::CreateModuleContext(const std::string &moduleName)
{
    return {};
};

std::shared_ptr<Context> MockAbilityRuntimeContext::CreateModuleContext(
    const std::string &bundleName, const std::string &moduleName)
{
    return {};
};

std::shared_ptr<Global::Resource::ResourceManager> MockAbilityRuntimeContext::CreateModuleResourceManager(
    const std::string &bundleName, const std::string &moduleName)
{
    return nullptr;
}

int32_t MockAbilityRuntimeContext::CreateSystemHspModuleResourceManager(const std::string &bundleName,
    const std::string &moduleName, std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    return 0;
}

int MockAbilityRuntimeContext::GetArea()
{
    return {};
};

std::shared_ptr<AppExecFwk::Configuration> MockAbilityRuntimeContext::GetConfiguration() const
{
    return {};
};

std::string MockAbilityRuntimeContext::GetBaseDir() const
{
    return {};
};

Global::Resource::DeviceType MockAbilityRuntimeContext::GetDeviceType() const
{
    return {};
};

std::string MockAbilityRuntimeContext::GetGroupDir(std::string groupId)
{
    return {};
}

int32_t MockAbilityRuntimeContext::GetSystemDatabaseDir(const std::string &groupId, bool checkExist,
    std::string &databaseDir)
{
    return 0;
}

int32_t MockAbilityRuntimeContext::GetSystemPreferencesDir(const std::string &groupId, bool checkExist,
    std::string &preferencesDir)
{
    return 0;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
