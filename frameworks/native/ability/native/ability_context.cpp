/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "ability_context.h"

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "authorization_result.h"
#include "bundle_constants.h"
#include "hilog_tag_wrapper.h"
#include "iservice_registry.h"
#include "os_account_manager_wrapper.h"
#include "resource_manager.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#include "hitrace_meter.h"
#include "remote_object_wrapper.h"
#include "scene_board_judgement.h"
#include "session/host/include/zidl/session_interface.h"
#include "session_info.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
int AbilityContext::ABILITY_CONTEXT_DEFAULT_REQUEST_CODE(0);
namespace {
const std::string GRANT_ABILITY_BUNDLE_NAME = "com.ohos.permissionmanager";
const std::string GRANT_ABILITY_ABILITY_NAME = "com.ohos.permissionmanager.GrantAbility";
const std::string PERMISSION_KEY = "ohos.user.grant.permission";
const std::string STATE_KEY = "ohos.user.grant.permission.state";
const std::string TOKEN_KEY = "ohos.ability.params.token";
const std::string CALLBACK_KEY = "ohos.ability.params.callback";
}

ErrCode AbilityContext::StartAbility(const AAFwk::Want &want, int requestCode)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "AbilityContext::StartAbility called, requestCode = %{public}d", requestCode);
    AppExecFwk::AbilityType type = GetAbilityInfoType();
    if (type != AppExecFwk::AbilityType::PAGE && type != AppExecFwk::AbilityType::SERVICE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::StartAbility AbilityType = %{public}d", type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode);
    TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s. End calling ams->StartAbility. ret=%{public}d", __func__, err);
    return err;
}

ErrCode AbilityContext::StartAbility(const Want &want, int requestCode, const AbilityStartSetting &abilityStartSetting)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "AbilityContext::StartAbility with start setting called, requestCode = %{public}d",
        requestCode);
    AppExecFwk::AbilityType type = GetAbilityInfoType();
    if (type != AppExecFwk::AbilityType::PAGE && type != AppExecFwk::AbilityType::SERVICE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::StartAbility AbilityType = %{public}d", type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err =
        AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, abilityStartSetting, token_, requestCode);
    TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s. End calling ams->StartAbility. ret=%{public}d", __func__, err);
    return err;
}

ErrCode AbilityContext::TerminateAbility()
{
    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::TerminateAbility info == nullptr");
        return ERR_NULL_OBJECT;
    }

    ErrCode err = ERR_OK;
    switch (info->type) {
        case AppExecFwk::AbilityType::PAGE:
            TAG_LOGD(AAFwkTag::CONTEXT, "Terminate ability begin, type is page, ability is %{public}s.",
                info->name.c_str());
            if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
                auto sessionToken = GetSessionToken();
                if (sessionToken == nullptr) {
                    TAG_LOGE(AAFwkTag::CONTEXT, "sessionToken is nullptr.");
                    return ERR_INVALID_VALUE;
                }
                sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
                sessionInfo->want = resultWant_;
                sessionInfo->resultCode = resultCode_;
                TAG_LOGI(AAFwkTag::CONTEXT, "FA TerminateAbility resultCode is %{public}d", sessionInfo->resultCode);
                auto ifaceSessionToken = iface_cast<Rosen::ISession>(sessionToken);
                auto err = ifaceSessionToken->TerminateSession(sessionInfo);
                TAG_LOGI(AAFwkTag::CONTEXT, "FA TerminateAbility. ret=%{public}d", err);
                return static_cast<int32_t>(err);
            } else {
                err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, resultCode_, &resultWant_);
            }
            break;
        case AppExecFwk::AbilityType::SERVICE:
            TAG_LOGD(AAFwkTag::CONTEXT, "Terminate ability begin, type is service, ability is %{public}s.",
                info->name.c_str());
            err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
            break;
        default:
            TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::TerminateAbility info type error is %{public}d", info->type);
            break;
    }

    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::TerminateAbility is failed %{public}d", err);
    }
    return err;
}

std::string AbilityContext::GetCallingBundle()
{
    return callingBundleName_;
}

std::shared_ptr<ElementName> AbilityContext::GetElementName()
{
    TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s begin.", __func__);
    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::GetElementName info == nullptr");
        return nullptr;
    }

    std::shared_ptr<ElementName> elementName = std::make_shared<ElementName>();
    elementName->SetAbilityName(info->name);
    elementName->SetBundleName(info->bundleName);
    elementName->SetDeviceID(info->deviceId);
    elementName->SetModuleName(info->moduleName);
    TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s end.", __func__);
    return elementName;
}

std::shared_ptr<ElementName> AbilityContext::GetCallingAbility()
{
    TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s begin.", __func__);
    std::shared_ptr<ElementName> elementName = std::make_shared<ElementName>();
    elementName->SetAbilityName(callingAbilityName_);
    elementName->SetBundleName(callingBundleName_);
    elementName->SetDeviceID(callingDeviceId_);
    elementName->SetModuleName(callingModuleName_);
    TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s end.", __func__);
    return elementName;
}

bool AbilityContext::ConnectAbility(const Want &want, const sptr<AAFwk::IAbilityConnection> &conn)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppExecFwk::AbilityType type = GetAbilityInfoType();

    std::shared_ptr<AbilityInfo> abilityInfo = GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::ConnectAbility info == nullptr");
        return false;
    }

    TAG_LOGI(AAFwkTag::CONTEXT, "Connect ability begin, ability:%{public}s.", abilityInfo->name.c_str());

    if (type != AppExecFwk::AbilityType::PAGE && type != AppExecFwk::AbilityType::SERVICE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::ConnectAbility AbilityType = %{public}d", type);
        return false;
    }

    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(want, conn, token_);
    TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s end ConnectAbility, ret=%{public}d", __func__, ret);
    bool value = ((ret == ERR_OK) ? true : false);
    if (!value) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::ConnectAbility ErrorCode = %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s end.", __func__);
    return value;
}

ErrCode AbilityContext::DisconnectAbility(const sptr<AAFwk::IAbilityConnection> &conn)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    TAG_LOGI(AAFwkTag::CONTEXT, "Disconnect ability begin, caller:%{public}s.",
        info == nullptr ? "" : info->name.c_str());

    AppExecFwk::AbilityType type = GetAbilityInfoType();
    if (type != AppExecFwk::AbilityType::PAGE && type != AppExecFwk::AbilityType::SERVICE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::DisconnectAbility AbilityType = %{public}d", type);
        return ERR_INVALID_VALUE;
    }

    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(conn);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::DisconnectAbility error, ret=%{public}d.", ret);
    }
    return ret;
}

bool AbilityContext::StopAbility(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s begin.", __func__);
    AppExecFwk::AbilityType type = GetAbilityInfoType();
    if (type != AppExecFwk::AbilityType::PAGE && type != AppExecFwk::AbilityType::SERVICE) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::StopAbility AbilityType = %{public}d", type);
        return false;
    }

    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StopServiceAbility(want, token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::StopAbility is failed %{public}d", err);
        return false;
    }

    return true;
}

sptr<IRemoteObject> AbilityContext::GetToken()
{
    return token_;
}

AppExecFwk::AbilityType AbilityContext::GetAbilityInfoType()
{
    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::GetAbilityInfoType info == nullptr");
        return AppExecFwk::AbilityType::UNKNOWN;
    }

    return info->type;
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityContext::GetResourceManager() const
{
    std::shared_ptr<Context> appContext = GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::GetResourceManager appContext is nullptr");
        return nullptr;
    }

    TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s begin appContext->GetResourceManager.", __func__);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = appContext->GetResourceManager();
    TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s end appContext->GetResourceManager.", __func__);
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::GetResourceManager resourceManager is nullptr");
        return nullptr;
    }
    return resourceManager;
}

int AbilityContext::VerifyPermission(const std::string &permission, int pid, int uid)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "%{public}s begin. permission=%{public}s, pid=%{public}d, uid=%{public}d",
        __func__,
        permission.c_str(),
        pid,
        uid);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->VerifyPermission(permission, pid, uid);
    TAG_LOGD(AAFwkTag::CONTEXT, "End calling VerifyPermission. ret=%{public}d", err);
    if (err != ERR_OK) {
        return AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
    }
    return 0;
}

void AbilityContext::GetPermissionDes(const std::string &permissionName, std::string &des)
{
    Security::AccessToken::PermissionDef permissionDef;
    int32_t ret = Security::AccessToken::AccessTokenKit::GetDefPermission(permissionName, permissionDef);
    if (ret == Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        TAG_LOGD(AAFwkTag::CONTEXT, "GetPermissionDes %{public}s: RET_SUCCESS", permissionName.c_str());
        des = permissionDef.description;
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s end GetPermissionDef.", __func__);
}

void AbilityContext::RequestPermissionsFromUser(std::vector<std::string> &permissions,
    std::vector<int> &permissionsState, PermissionRequestTask &&task)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s begin.", __func__);
    if (permissions.size() == 0) {
        TAG_LOGE(AAFwkTag::CONTEXT, "AbilityContext::RequestPermissionsFromUser permissions is empty");
        return;
    }

    AAFwk::Want want;
    want.SetElementName(GRANT_ABILITY_BUNDLE_NAME, GRANT_ABILITY_ABILITY_NAME);
    want.SetParam(PERMISSION_KEY, permissions);
    want.SetParam(STATE_KEY, permissionsState);
    want.SetParam(TOKEN_KEY, token_);
    sptr<IRemoteObject> remoteObject = new AbilityRuntime::AuthorizationResult(std::move(task));
    want.SetParam(CALLBACK_KEY, remoteObject);
    StartAbility(want, -1);
    TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s end.", __func__);
}

void AbilityContext::SetCallingContext(const std::string &deviceId, const std::string &bundleName,
    const std::string &abilityName, const std::string &moduleName)
{
    callingDeviceId_ = deviceId;
    callingBundleName_ = bundleName;
    callingAbilityName_ = abilityName;
    callingModuleName_ = moduleName;
}

void AbilityContext::StartAbilities(const std::vector<AAFwk::Want> &wants)
{
    for (auto want : wants) {
        StartAbility(want, ABILITY_CONTEXT_DEFAULT_REQUEST_CODE);
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "%{public}s end.", __func__);
}

sptr<IRemoteObject> AbilityContext::GetSessionToken()
{
    std::lock_guard lock(sessionTokenMutex_);
    return sessionToken_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
