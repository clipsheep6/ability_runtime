/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "cj_ability_stage.h"
#include "cj_ability_stage_context.h"
#include "cj_runtime.h"
#include "context_impl.h"
#include "hilog_tag_wrapper.h"
#include "securec.h"

using namespace OHOS::AbilityRuntime;

namespace {
char* CreateCStringFromString(const std::string& source)
{
    if (source.size() == 0) {
        return nullptr;
    }
    size_t length = source.size() + 1;
    auto res = static_cast<char*>(malloc(length));
    if (res == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "fail to mallc string.");
        return nullptr;
    }
    if (strcpy_s(res, length, source.c_str()) != 0) {
        free(res);
        TAG_LOGE(AAFwkTag::APPKIT, "fail to strcpy source.");
        return nullptr;
    }
    return res;
}
}

extern "C" {
CJ_EXPORT CurrentHapModuleInfo* FFICJCurrentHapModuleInfo(int64_t id)
{
    auto abilityStageContext = OHOS::FFI::FFIData::GetData<CJAbilityStageContext>(id);
    if (abilityStageContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Get abilityStageContext failed. ");
        return nullptr;
    }

    auto hapInfo = abilityStageContext->GetHapModuleInfo();
    if (hapInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "CurrentHapMoudleInfo is nullptr.");
        return nullptr;
    }

    CurrentHapModuleInfo* buffer = static_cast<CurrentHapModuleInfo*>(malloc(sizeof(CurrentHapModuleInfo)));
 
    if (buffer == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Create CurrentHapMoudleInfo failed, CurrentHapMoudleInfo is nullptr.");
        return nullptr;
    }

    buffer->name = CreateCStringFromString(hapInfo->name);
    buffer->icon = CreateCStringFromString(hapInfo->iconPath);
    buffer->iconId = hapInfo->iconId;
    buffer->label = CreateCStringFromString(hapInfo->label);
    buffer->labelId = hapInfo->labelId;
    buffer->description = CreateCStringFromString(hapInfo->description);
    buffer->descriptionId = hapInfo->descriptionId;
    buffer->mainElementName = CreateCStringFromString(hapInfo->mainElementName);
    buffer->installationFree = hapInfo->installationFree;
    buffer->hashValue = CreateCStringFromString(hapInfo->hashValue);

    return buffer;
}

CJ_EXPORT int64_t FFIAbilityGetAbilityStageContext(AbilityStageHandle abilityStageHandle)
{
    auto ability = static_cast<CJAbilityStage*>(abilityStageHandle);
    auto context = ability->GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetAbilityStageContext failed, abilityContext is nullptr.");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto cjStageContext = OHOS::FFI::FFIData::Create<CJAbilityStageContext>(context);
    if (cjStageContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetAbilityStageContext failed, abilityContext is nullptr.");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return cjStageContext->GetID();
}
}

std::shared_ptr<CJAbilityStage> CJAbilityStage::Create(
    const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    if (!runtime) {
        TAG_LOGE(AAFwkTag::APPKIT, "Runtime does not exist.");
        return nullptr;
    }
    auto& cjRuntime = static_cast<CJRuntime&>(*runtime);
    // Load cj app library.
    if (!cjRuntime.IsAppLibLoaded()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create CJAbilityStage, applib not loaded.");
        return nullptr;
    }

    auto cjAbilityStageObject = CJAbilityStageObject::LoadModule(hapModuleInfo.moduleName);
    if (cjAbilityStageObject == nullptr) {
        cjRuntime.UnLoadCJAppLibrary();
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create CJAbilityStage.");
        return nullptr;
    }

    return std::make_shared<CJAbilityStage>(cjAbilityStageObject);
}

void CJAbilityStage::Init(const std::shared_ptr<Context> &context,
    const std::weak_ptr<AppExecFwk::OHOSApplication> application)
{
    AbilityStage::Init(context, application);
    if (!cjAbilityStageObject_) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create CJAbilityStage.");
        return;
    }
    cjAbilityStageObject_->Init(this);
}

void CJAbilityStage::OnCreate(const AAFwk::Want& want) const
{
    AbilityStage::OnCreate(want);
    if (!cjAbilityStageObject_) {
        TAG_LOGE(AAFwkTag::APPKIT, "CJAbilityStage is not loaded.");
        return;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "CJAbilityStage::OnCreate");
    cjAbilityStageObject_->OnCreate();
}

std::string CJAbilityStage::OnAcceptWant(const AAFwk::Want& want)
{
    AbilityStage::OnAcceptWant(want);
    if (!cjAbilityStageObject_) {
        TAG_LOGE(AAFwkTag::APPKIT, "CJAbilityStage is not loaded.");
        return "";
    }
    return cjAbilityStageObject_->OnAcceptWant(want);
}

void CJAbilityStage::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    AbilityStage::OnConfigurationUpdated(configuration);
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::APPKIT, "configuration is nullptr.");
        return;
    }

    if (!cjAbilityStageObject_) {
        TAG_LOGE(AAFwkTag::APPKIT, "CJAbilityStage is not loaded.");
        return;
    }
    cjAbilityStageObject_->OnConfigurationUpdated(fullConfig);
}

void CJAbilityStage::OnMemoryLevel(int level)
{
    AbilityStage::OnMemoryLevel(level);
    if (!cjAbilityStageObject_) {
        TAG_LOGE(AAFwkTag::APPKIT, "CJAbilityStage is not loaded.");
        return;
    }
    cjAbilityStageObject_->OnMemoryLevel(level);
}
