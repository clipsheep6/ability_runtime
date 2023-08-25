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

#include "resource_manager_addon.h"

namespace OHOS {
namespace Global {
namespace Resource {
static thread_local napi_ref g_constructor = nullptr;
static std::shared_ptr<ResourceManager> sysResMgr = nullptr;
static std::mutex sysMgrMutex;

napi_value ResourceManagerAddon::Create(
    napi_env env, const std::string &bundleName, const std::shared_ptr<ResourceManager> &resMgr,
    std::shared_ptr<AbilityRuntime::Context> context)
{
    std::shared_ptr<ResourceManagerAddon> addon = std::make_shared<ResourceManagerAddon>(bundleName, resMgr, context);
    return WrapResourceManager(env, addon);
}

napi_value ResourceManagerAddon::WrapResourceManager(napi_env env, std::shared_ptr<ResourceManagerAddon> &addon)
{
    if (!Init(env)) {
        return nullptr;
    }

    napi_value constructor = nullptr;
    napi_status status = napi_get_reference_value(env, g_constructor, &constructor);
    if (status != napi_ok || constructor == nullptr) {
        return nullptr;
    }
    napi_value result = nullptr;
    status = napi_new_instance(env, constructor, 0, nullptr, &result);
    if (status != napi_ok) {
        return nullptr;
    }

    auto addonPtr = std::make_unique<std::shared_ptr<ResourceManagerAddon>>(addon);
    status = napi_wrap(env, result, reinterpret_cast<void *>(addonPtr.get()), ResourceManagerAddon::Destructor,
        nullptr, nullptr);
    if (status != napi_ok) {
        return nullptr;
    }
    addonPtr.release();
    return result;
}

ResourceManagerAddon::ResourceManagerAddon(
    const std::string &bundleName, const std::shared_ptr<ResourceManager> &resMgr,
    const std::shared_ptr<AbilityRuntime::Context> &context, bool isSystem)
    : bundleName_(bundleName), resMgr_(resMgr), context_(context), isSystem_(isSystem)
{
}

ResourceManagerAddon::ResourceManagerAddon(const std::shared_ptr<ResourceManager> &resMgr, bool isSystem)
    : resMgr_(resMgr), isSystem_(isSystem)
{
}

ResourceManagerAddon::~ResourceManagerAddon()
{
}

void ResourceManagerAddon::Destructor(napi_env env, void *nativeObject, void *hint)
{
    std::unique_ptr<std::shared_ptr<ResourceManagerAddon>> addonPtr;
    addonPtr.reset(static_cast<std::shared_ptr<ResourceManagerAddon>*>(nativeObject));
}

bool ResourceManagerAddon::Init(napi_env env)
{
    if (g_constructor != nullptr) {
        return true;
    }

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getString", GetString),
        DECLARE_NAPI_FUNCTION("getStringByName", GetStringByName),
        DECLARE_NAPI_FUNCTION("getStringArray", GetStringArray),
        DECLARE_NAPI_FUNCTION("getStringArrayByName", GetStringArrayByName),
        DECLARE_NAPI_FUNCTION("getMedia", GetMedia),
        DECLARE_NAPI_FUNCTION("getMediaByName", GetMediaByName),
        DECLARE_NAPI_FUNCTION("getMediaBase64", GetMediaBase64),
        DECLARE_NAPI_FUNCTION("getMediaBase64ByName", GetMediaBase64ByName),
        DECLARE_NAPI_FUNCTION("getConfiguration", GetConfiguration),
        DECLARE_NAPI_FUNCTION("getDeviceCapability", GetDeviceCapability),
        DECLARE_NAPI_FUNCTION("getPluralString", GetPluralString),
        DECLARE_NAPI_FUNCTION("getPluralStringByName", GetPluralStringByName),
        DECLARE_NAPI_FUNCTION("getRawFile", GetRawFile),
        DECLARE_NAPI_FUNCTION("getRawFileDescriptor", GetRawFileDescriptor),
        DECLARE_NAPI_FUNCTION("closeRawFileDescriptor", CloseRawFileDescriptor),
        DECLARE_NAPI_FUNCTION("getStringSync", GetStringSync),
        DECLARE_NAPI_FUNCTION("getStringByNameSync", GetStringByNameSync),
        DECLARE_NAPI_FUNCTION("getBoolean", GetBoolean),
        DECLARE_NAPI_FUNCTION("getNumber", GetNumber),
        DECLARE_NAPI_FUNCTION("getBooleanByName", GetBooleanByName),
        DECLARE_NAPI_FUNCTION("getNumberByName", GetNumberByName),
        DECLARE_NAPI_FUNCTION("release", Release),
        DECLARE_NAPI_FUNCTION("getStringValue", GetStringValue),
        DECLARE_NAPI_FUNCTION("getStringArrayValue", GetStringArrayValue),
        DECLARE_NAPI_FUNCTION("getPluralStringValue", GetPluralStringValue),
        DECLARE_NAPI_FUNCTION("getMediaContent", GetMediaContent),
        DECLARE_NAPI_FUNCTION("getMediaContentBase64", GetMediaContentBase64),
        DECLARE_NAPI_FUNCTION("getRawFileContent", GetRawFileContent),
        DECLARE_NAPI_FUNCTION("getRawFd", GetRawFd),
        DECLARE_NAPI_FUNCTION("closeRawFd", CloseRawFd),
        DECLARE_NAPI_FUNCTION("getDrawableDescriptor", GetDrawableDescriptor),
        DECLARE_NAPI_FUNCTION("getDrawableDescriptorByName", GetDrawableDescriptorByName),
        DECLARE_NAPI_FUNCTION("getRawFileList", GetRawFileList),
        DECLARE_NAPI_FUNCTION("getColor", GetColor),
        DECLARE_NAPI_FUNCTION("getColorByName", GetColorByName),
        DECLARE_NAPI_FUNCTION("getColorSync", GetColorSync),
        DECLARE_NAPI_FUNCTION("getColorByNameSync", GetColorByNameSync),
        DECLARE_NAPI_FUNCTION("addResource", AddResource),
        DECLARE_NAPI_FUNCTION("removeResource", RemoveResource)
    };

    napi_value constructor;
    napi_status status = napi_define_class(env, "ResourceManager", NAPI_AUTO_LENGTH, New, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        return false;
    }

    status = napi_create_reference(env, constructor, 1, &g_constructor);
    if (status != napi_ok) {
        return false;
    }
    return true;
}

napi_value ResourceManagerAddon::New(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::Release(napi_env env, napi_callback_info info)
{
    napi_value undefined;
    if (napi_get_undefined(env, &undefined) != napi_ok) {
        return nullptr;
    }
    return undefined;
}

napi_value ResourceManagerAddon::GetString(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetStringArray(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetMedia(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetMediaBase64(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetConfiguration(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetDeviceCapability(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetPluralString(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetRawFile(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetRawFileDescriptor(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::CloseRawFileDescriptor(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetPluralStringByName(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetMediaBase64ByName(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetMediaByName(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetStringArrayByName(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetStringByName(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetStringValue(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetStringArrayValue(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetMediaContent(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetMediaContentBase64(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetPluralStringValue(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetRawFileContent(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetRawFd(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::CloseRawFd(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetRawFileList(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetColor(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetColorByName(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetStringSync(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetStringByNameSync(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetBoolean(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetNumber(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetNumberByName(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetBooleanByName(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetDrawableDescriptor(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetDrawableDescriptorByName(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetColorSync(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::GetColorByNameSync(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::AddResource(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ResourceManagerAddon::RemoveResource(napi_env env, napi_callback_info info)
{
    return nullptr;
}
} // namespace Resource
} // namespace Global
} // namespace OHOS