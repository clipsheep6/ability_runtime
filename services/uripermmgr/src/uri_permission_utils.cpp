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

#include "uri_permission_utils.h"

#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "os_account_manager_wrapper.h"
#include "permission_verification.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t DEFAULT_USER_ID = 0;
constexpr const char* FOUNDATION_PROCESS_NAME = "foundation";
}

std::shared_ptr<AppExecFwk::BundleMgrHelper> UPMSUtils::ConnectManagerHelper()
{
    if (bundleMgrHelper_ == nullptr) {
        bundleMgrHelper_ = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    }
    return bundleMgrHelper_;
}

bool UPMSUtils::SendShareUnPrivilegeUriEvent(uint32_t callerTokenId, uint32_t targetTokenId)
{
    std::string callerBundleName;
    if (!GetBundleNameByTokenId(callerTokenId, callerBundleName)) {
        return false;
    }
    std::string targetBundleName;
    if (!GetBundleNameByTokenId(targetTokenId, targetBundleName)) {
        return false;
    }
    AAFwk::EventInfo eventInfo;
    eventInfo.callerBundleName = callerBundleName;
    eventInfo.bundleName = targetBundleName;
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Send SHARE_UNPRIVILEGED_FILE_URI Event.");
    AAFwk::EventReport::SendGrantUriPermissionEvent(AAFwk::EventName::SHARE_UNPRIVILEGED_FILE_URI, eventInfo);
    return true;
}

bool UPMSUtils::SendSystemAppGrantUriPermissionEvent(uint32_t callerTokenId, uint32_t targetTokenId,
    const std::vector<std::string> &uriVec, const std::vector<int32_t> &resVec)
{
    EventInfo eventInfo;
    if (!CheckAndCreateEventInfo(callerTokenId, targetTokenId, eventInfo)) {
        return false;
    }
    for (size_t i = 0; i < resVec.size(); i++) {
        if (resVec[i] == 0 || resVec[i] == -EEXIST) {
            eventInfo.uri = uriVec[i];
            EventReport::SendGrantUriPermissionEvent(EventName::GRANT_URI_PERMISSION, eventInfo);
        }
    }
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Send GRANT_URI_PERMISSION Event.");
    return true;
}

bool UPMSUtils::CheckAndCreateEventInfo(uint32_t callerTokenId, uint32_t targetTokenId,
    EventInfo &eventInfo)
{
    std::string callerBundleName;
    if (!GetBundleNameByTokenId(callerTokenId, callerBundleName)) {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "get caller bundle name failed.");
        return false;
    }
    if (!CheckIsSystemAppByBundleName(callerBundleName)) {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "caller is not system.");
        return false;
    }
    std::string targetBundleName;
    if (!GetBundleNameByTokenId(targetTokenId, targetBundleName)) {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "get target bundle name failed.");
        return false;
    }
    if (CheckIsSystemAppByBundleName(targetBundleName)) {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "target is system app.");
        return false;
    }
    eventInfo.callerBundleName = callerBundleName;
    eventInfo.bundleName = targetBundleName;
    return true;
}

int32_t UPMSUtils::GetCurrentAccountId()
{
    std::vector<int32_t> osActiveAccountIds;
    auto ret = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        QueryActiveOsAccountIds(osActiveAccountIds);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "QueryActiveOsAccountIds error.");
        return DEFAULT_USER_ID;
    }
    if (osActiveAccountIds.empty()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "the QueryActiveOsAccountIds is empty, no accounts.");
        return DEFAULT_USER_ID;
    }
    return osActiveAccountIds.front();
}

bool UPMSUtils::IsFoundationCall()
{
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "callerTokenId is %{public}u", callerTokenId);
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerTokenId);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Is not native call");
        return false;
    }
    Security::AccessToken::NativeTokenInfo nativeInfo;
    auto result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerTokenId, nativeInfo);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "GetNativeTokenInfo failed, callerTokenId is %{public}u.", callerTokenId);
        return false;
    }
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Caller process name : %{public}s", nativeInfo.processName.c_str());
    return nativeInfo.processName == FOUNDATION_PROCESS_NAME;
}

bool UPMSUtils::IsSAOrSystemAppCall()
{
    return PermissionVerification::GetInstance()->IsSystemAppCall() ||
        PermissionVerification::GetInstance()->IsSACall();
}

bool UPMSUtils::IsSystemAppCall(uint32_t tokenId)
{
    if (UPMSUtils::IsFoundationCall()) {
        return UPMSUtils::CheckIsSystemAppByTokenId(tokenId);
    }
    return PermissionVerification::GetInstance()->IsSystemAppCall();
}

bool UPMSUtils::CheckIsSystemAppByBundleName(std::string &bundleName)
{
    auto bundleMgrHelper = ConnectManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "The bundleMgrHelper is nullptr.");
        return false;
    }
    AppExecFwk::ApplicationInfo appInfo;
    if (!IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(bundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, GetCurrentAccountId(), appInfo))) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "Get application info failed.");
        return false;
    }
    auto isSystemApp = Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(appInfo.accessTokenIdEx);
    TAG_LOGD(AAFwkTag::URIPERMMGR, "BundleName is %{public}s, isSystemApp = %{public}d", bundleName.c_str(),
        static_cast<int32_t>(isSystemApp));
    return isSystemApp;
}

bool UPMSUtils::CheckIsSystemAppByTokenId(uint32_t tokenId)
{
    std::string bundleName;
    if (GetBundleNameByTokenId(tokenId, bundleName)) {
        return CheckIsSystemAppByBundleName(bundleName);
    }
    return false;
}

bool UPMSUtils::GetBundleNameByTokenId(uint32_t tokenId, std::string &bundleName)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        Security::AccessToken::HapTokenInfo hapInfo;
        auto ret = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo);
        if (ret != Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "GetHapTokenInfo failed, ret is %{public}d.", ret);
            return false;
        }
        bundleName = hapInfo.bundleName;
        return true;
    }
    return false;
}

std::string UPMSUtils::GetCallerNameByTokenId(uint32_t tokenId)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        // for SA, return process name
        Security::AccessToken::NativeTokenInfo nativeInfo;
        auto result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(tokenId, nativeInfo);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "GetNativeTokenInfo failed, tokenId is %{public}u.", tokenId);
            return "";
        }
        return nativeInfo.processName;
    }
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        // for application, return bundle name
        Security::AccessToken::HapTokenInfo hapInfo;
        auto ret = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo);
        if (ret != Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "GetHapTokenInfo failed, ret is %{public}d.", ret);
            return "";
        }
        return hapInfo.bundleName;
    }
    return "";
}

int32_t UPMSUtils::GetTokenIdByBundleName(const std::string &bundleName, int32_t appIndex, uint32_t &tokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "BundleName is %{public}s, appIndex is %{public}d.", bundleName.c_str(), appIndex);
    auto bms = ConnectManagerHelper();
    if (bms == nullptr) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "The bundleMgrHelper is nullptr.");
        return GET_BUNDLE_MANAGER_SERVICE_FAILED;
    }
    AppExecFwk::BundleInfo bundleInfo;
    auto userId = GetCurrentAccountId();
    if (appIndex == 0) {
        auto bundleFlag = AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO;
        if (!IN_PROCESS_CALL(bms->GetBundleInfo(bundleName, bundleFlag, bundleInfo, userId))) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "Failed to get bundle info.");
            return GET_BUNDLE_INFO_FAILED;
        }
        tokenId = bundleInfo.applicationInfo.accessTokenId;
        return ERR_OK;
    }
    if (appIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        auto bundleFlag = static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION);
        if (IN_PROCESS_CALL(bms->GetCloneBundleInfo(bundleName, bundleFlag, appIndex, bundleInfo, userId)) != ERR_OK) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "Failed to get clone bundle info.");
            return GET_BUNDLE_INFO_FAILED;
        }
        tokenId = bundleInfo.applicationInfo.accessTokenId;
        return ERR_OK;
    }
    if (IN_PROCESS_CALL(bms->GetSandboxBundleInfo(bundleName, appIndex, userId, bundleInfo) != ERR_OK)) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "Failed to get sandbox bundle info.");
        return GET_BUNDLE_INFO_FAILED;
    }
    tokenId = bundleInfo.applicationInfo.accessTokenId;
    return ERR_OK;
}

std::shared_ptr<AppExecFwk::BundleMgrHelper> UPMSUtils::bundleMgrHelper_ = nullptr;
}  // namespace AAFwk
}  // namespace OHOS
