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

#include "connection_manager.h"
#include "mock_context.h"
#include "resource_manager.h"
#include "sys_mgr_client.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t Context::CONTEXT_TYPE_ID(std::hash<const char*> {} ("Context"));
std::string MockContext::GetBundleName() const
{
    return "com.test.bundleName";
}

std::string MockContext::GetBundleCodeDir()
{
    return "/code";
}

std::string MockContext::GetCacheDir()
{
    return "/cache";
}

bool MockContext::IsUpdatingConfigurations()
{
    return true;
}

bool MockContext::PrintDrawnCompleted()
{
    return true;
}

std::string MockContext::GetDatabaseDir()
{
    return "/data/app/database";
}

std::string MockContext::GetPreferencesDir()
{
    return "/preferences";
}

std::string MockContext::GetTempDir()
{
    return "/temp";
}

std::string MockContext::GetFilesDir()
{
    return "/files";
}

std::string MockContext::GetDistributedFilesDir()
{
    return "/mnt/hmdfs/device_view/local/data/bundleName";
}

std::shared_ptr<Context> MockContext::CreateModuleContext(const std::string &moduleName)
{
    return nullptr;
}

std::shared_ptr<Context> MockContext::CreateModuleContext(const std::string &bundleName, const std::string &moduleName)
{
    return nullptr;
}

std::shared_ptr<Global::Resource::ResourceManager> MockContext::CreateModuleResourceManager(
    const std::string &bundleName, const std::string &moduleName)
{
    return nullptr;
}

int MockContext::GetArea()
{
    return mode_;
}

std::string MockContext::GetBaseDir() const
{
    return "/data/app/base";
}

std::shared_ptr<Context> MockContext::CreateBundleContext(const std::string &bundleName)
{
    return nullptr;
}

sptr<AppExecFwk::IBundleMgr> MockContext::GetBundleManager() const
{
    auto instance = OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (instance == nullptr) {
        return nullptr;
    }
    auto bundleObj = instance->GetSystemAbility(401);
    if (bundleObj == nullptr) {
        return nullptr;
    }
    sptr<AppExecFwk::IBundleMgr> bms = iface_cast<AppExecFwk::IBundleMgr>(bundleObj);
    return bms;
}

std::string MockContext::GetBundleCodePath() const
{
    return "codePath";
}

void MockContext::InitHapModuleInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo)
{}

Global::Resource::DeviceType MockContext::GetDeviceType() const
{
    return Global::Resource::DeviceType::DEVICE_NOT_SET;
}

std::shared_ptr<AppExecFwk::ApplicationInfo> MockContext::GetApplicationInfo() const
{
    std::shared_ptr<AppExecFwk::ApplicationInfo> info = std::make_shared<AppExecFwk::ApplicationInfo>();
    info->name = "ContextTest";
    return info;
}

std::shared_ptr<Global::Resource::ResourceManager> MockContext::GetResourceManager() const
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    return resourceManager;
}

std::shared_ptr<AppExecFwk::Configuration> MockContext::GetConfiguration() const
{
    std::shared_ptr<AppExecFwk::Configuration> config = std::make_shared<AppExecFwk::Configuration>();
    return config;
}

void MockContext::SetToken(const sptr<IRemoteObject> &token)
{}

sptr<IRemoteObject> MockContext::GetToken()
{
    return nullptr;
}

std::shared_ptr<AppExecFwk::HapModuleInfo> MockContext::GetHapModuleInfo() const
{
    return nullptr;  
}

void MockContext::SwitchArea(int mode)
{
    mode_ = mode;
}

std::string MockContext::GetGroupDir(std::string groupId)
{
    return "/group";
}

int32_t MockContext::GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir)
{
    return 0;
}

int32_t MockContext::GetSystemPreferencesDir(const std::string &groupId, bool checkExist, std::string &preferencesDir)
{
    return 0;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
