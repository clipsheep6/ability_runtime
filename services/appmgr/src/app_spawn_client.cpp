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
#include "app_spawn_client.h"

#include <unordered_set>

#include "hitrace_meter.h"
#include "hilog_tag_wrapper.h"
#include "nlohmann/json.hpp"
#include "securec.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char* HSPLIST_BUNDLES = "bundles";
constexpr const char* HSPLIST_MODULES = "modules";
constexpr const char* HSPLIST_VERSIONS = "versions";
constexpr const char* DATAGROUPINFOLIST_DATAGROUPID = "dataGroupId";
constexpr const char* DATAGROUPINFOLIST_GID = "gid";
constexpr const char* DATAGROUPINFOLIST_DIR = "dir";
constexpr const char* JSON_DATA_APP = "/data/app/el2/";
constexpr const char* JSON_GROUP = "/group/";
constexpr const char* VERSION_PREFIX = "v";
constexpr const char* APPSPAWN_CLIENT_USER_NAME = "APP_MANAGER_SERVICE";
constexpr int32_t RIGHT_SHIFT_STEP = 1;
constexpr int32_t START_FLAG_TEST_NUM = 1;
constexpr const char* MAX_CHILD_PROCESS = "MaxChildProcess";
}
AppSpawnClient::AppSpawnClient(bool isNWebSpawn)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppspawnCreateClient");
    if (isNWebSpawn) {
        serviceName_ = NWEBSPAWN_SERVER_NAME;
    }
    state_ = SpawnConnectionState::STATE_NOT_CONNECT;
}

AppSpawnClient::AppSpawnClient(const char* serviceName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppspawnCreateClient");
    std::string serviceName__ = serviceName;
    if (serviceName__ == APPSPAWN_SERVER_NAME) {
        serviceName_ = APPSPAWN_SERVER_NAME;
    } else if (serviceName__ == CJAPPSPAWN_SERVER_NAME) {
        serviceName_ = CJAPPSPAWN_SERVER_NAME;
    } else if (serviceName__ == NWEBSPAWN_SERVER_NAME) {
        serviceName_ = NWEBSPAWN_SERVER_NAME;
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "unknown service name");
        serviceName_ = NWEBSPAWN_SERVER_NAME;
    }
    state_ = SpawnConnectionState::STATE_NOT_CONNECT;
}

AppSpawnClient::~AppSpawnClient()
{
    CloseConnection();
}

ErrCode AppSpawnClient::OpenConnection()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (state_ == SpawnConnectionState::STATE_CONNECTED) {
        return 0;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "OpenConnection");
    
    AppSpawnClientHandle handle = nullptr;
    ErrCode ret = 0;
    ret = AppSpawnClientInit(serviceName_.c_str(), &handle);
    if (FAILED(ret)) {
        TAG_LOGE(AAFwkTag::APPMGR, "create appspawn client faild.");
        state_ = SpawnConnectionState::STATE_CONNECT_FAILED;
        return ret;
    }
    handle_ = handle;
    state_ = SpawnConnectionState::STATE_CONNECTED;

    return ret;
}

void AppSpawnClient::CloseConnection()
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppspawnDestroyClient");
    if (state_ == SpawnConnectionState::STATE_CONNECTED) {
        AppSpawnClientDestroy(handle_);
    }
    state_ = SpawnConnectionState::STATE_NOT_CONNECT;
}

SpawnConnectionState AppSpawnClient::QueryConnectionState() const
{
    return state_;
}

AppSpawnClientHandle AppSpawnClient::GetAppSpawnClientHandle() const
{
    if (state_ == SpawnConnectionState::STATE_CONNECTED) {
        return handle_;
    }
    return nullptr;
}

static std::string DumpDataGroupInfoListToJson(const DataGroupInfoList &dataGroupInfoList)
{
    nlohmann::json dataGroupInfoListJson;
    for (auto& dataGroupInfo : dataGroupInfoList) {
        dataGroupInfoListJson[DATAGROUPINFOLIST_DATAGROUPID].emplace_back(dataGroupInfo.dataGroupId);
        dataGroupInfoListJson[DATAGROUPINFOLIST_GID].emplace_back(std::to_string(dataGroupInfo.gid));
        std::string dir = JSON_DATA_APP + std::to_string(dataGroupInfo.userId)
            + JSON_GROUP + dataGroupInfo.uuid;
        dataGroupInfoListJson[DATAGROUPINFOLIST_DIR].emplace_back(dir);
    }
    return dataGroupInfoListJson.dump();
}

static std::string DumpHspListToJson(const HspList &hspList)
{
    nlohmann::json hspListJson;
    for (auto& hsp : hspList) {
        hspListJson[HSPLIST_BUNDLES].emplace_back(hsp.bundleName);
        hspListJson[HSPLIST_MODULES].emplace_back(hsp.moduleName);
        hspListJson[HSPLIST_VERSIONS].emplace_back(VERSION_PREFIX + std::to_string(hsp.versionCode));
    }
    return hspListJson.dump();
}

static std::string DumpAppEnvToJson(const std::map<std::string, std::string> &appEnv)
{
    nlohmann::json appEnvJson;
    for (const auto &[envName, envValue] : appEnv) {
        appEnvJson[envName] = envValue;
    }
    return appEnvJson.dump();
}

static std::string DumpExtensionSandboxDirsToJson(const std::map<std::string, std::string> &extensionSandboxDirs)
{
    nlohmann::json extensionSandboxDirsJson;
    for (auto &[userId, sandboxDir] : extensionSandboxDirs) {
        extensionSandboxDirsJson[userId] = sandboxDir;
    }
    return extensionSandboxDirsJson.dump();
}

int32_t AppSpawnClient::SetDacInfo(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle)
{
    int32_t ret = 0;
    AppDacInfo appDacInfo = {0};
    appDacInfo.uid = startMsg.uid;
    appDacInfo.gid = startMsg.gid;
    appDacInfo.gidCount = startMsg.gids.size() + startMsg.dataGroupInfoList.size();
    for (uint32_t i = 0; i < startMsg.gids.size(); i++) {
        appDacInfo.gidTable[i] = startMsg.gids[i];
    }
    for (uint32_t i = startMsg.gids.size(); i < appDacInfo.gidCount; i++) {
        appDacInfo.gidTable[i] = startMsg.dataGroupInfoList[i - startMsg.gids.size()].gid;
    }
    ret = strcpy_s(appDacInfo.userName, sizeof(appDacInfo.userName), APPSPAWN_CLIENT_USER_NAME);
    if (ret) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to set dac userName!");
        return ret;
    }
    return AppSpawnReqMsgSetAppDacInfo(reqHandle, &appDacInfo);
}

int32_t AppSpawnClient::SetMountPermission(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle)
{
    int32_t ret = 0;
    std::set<std::string> mountPermissionList = startMsg.permissions;
    for (std::string permission : mountPermissionList) {
        ret = AppSpawnClientAddPermission(handle_, reqHandle, permission.c_str());
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnReqMsgAddPermission %{public}s failed", permission.c_str());
            return ret;
        }
    }
    return ret;
}

int32_t AppSpawnClient::SetStartFlags(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle)
{
    int32_t ret = 0;
    uint32_t startFlagTmp = startMsg.flags;
    int flagIndex = 0;
    while (startFlagTmp > 0) {
        if (startFlagTmp & START_FLAG_TEST_NUM) {
            ret = AppSpawnReqMsgSetAppFlag(reqHandle, static_cast<AppFlagsIndex>(flagIndex));
            if (ret != 0) {
                TAG_LOGE(AAFwkTag::APPMGR, "SetFlagIdx %{public}d failed, ret: %{public}d", flagIndex, ret);
                return ret;
            }
        }
        startFlagTmp = startFlagTmp >> RIGHT_SHIFT_STEP;
        flagIndex++;
    }
    if (startMsg.atomicServiceFlag) {
        ret = AppSpawnReqMsgSetAppFlag(reqHandle, APP_FLAGS_ATOMIC_SERVICE);
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "SetAtomicServiceFlag failed, ret: %{public}d", ret);
            return ret;
        }
    }
    if (startMsg.strictMode) {
        ret = AppSpawnReqMsgSetAppFlag(reqHandle, APP_FLAGS_ISOLATED_SANDBOX);
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "SetStrictMode failed, ret: %{public}d", ret);
            return ret;
        }
    }
    if (startMsg.isolatedExtension) {
        ret = AppSpawnReqMsgSetAppFlag(reqHandle, APP_FLAGS_EXTENSION_SANDBOX);
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "SetAppExtension failed, ret: %{public}d", ret);
            return ret;
        }
    }
    if (startMsg.flags & APP_FLAGS_CLONE_ENABLE) {
        ret = AppSpawnReqMsgSetAppFlag(reqHandle, APP_FLAGS_CLONE_ENABLE);
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "SetCloneFlag failed, ret: %{public}d", ret);
            return ret;
        }
    }
    ret = SetChildProcessTypeStartFlag(reqHandle, startMsg.childProcessType);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Set childProcessType flag failed, ret: %{public}d", ret);
        return ret;
    }
    return ret;
}

int32_t AppSpawnClient::AppspawnSetExtMsg(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle)
{
    int32_t ret = 0;
    ret = AppSpawnReqMsgAddStringInfo(reqHandle, MSG_EXT_NAME_RENDER_CMD, startMsg.renderParam.c_str());
    if (ret) {
        TAG_LOGE(AAFwkTag::APPMGR, "SetRenderCmd failed, ret: %{public}d", ret);
        return ret;
    }

    if (!startMsg.hspList.empty()) {
        ret = AppSpawnReqMsgAddStringInfo(reqHandle, MSG_EXT_NAME_HSP_LIST,
            DumpHspListToJson(startMsg.hspList).c_str());
        if (ret) {
            TAG_LOGE(AAFwkTag::APPMGR, "SetExtraHspList failed, ret: %{public}d", ret);
            return ret;
        }
    }

    if (!startMsg.dataGroupInfoList.empty()) {
        ret = AppSpawnReqMsgAddStringInfo(reqHandle, MSG_EXT_NAME_DATA_GROUP,
            DumpDataGroupInfoListToJson(startMsg.dataGroupInfoList).c_str());
        if (ret) {
            TAG_LOGE(AAFwkTag::APPMGR, "SetExtraDataGroupInfo failed, ret: %{public}d", ret);
            return ret;
        }
    }

    if (!startMsg.overlayInfo.empty()) {
        ret = AppSpawnReqMsgAddStringInfo(reqHandle, MSG_EXT_NAME_OVERLAY, startMsg.overlayInfo.c_str());
        if (ret) {
            TAG_LOGE(AAFwkTag::APPMGR, "SetExtraOverlayInfo failed, ret: %{public}d", ret);
            return ret;
        }
    }

    if (!startMsg.appEnv.empty()) {
        ret = AppSpawnReqMsgAddStringInfo(reqHandle, MSG_EXT_NAME_APP_ENV, DumpAppEnvToJson(startMsg.appEnv).c_str());
        if (ret) {
            TAG_LOGE(AAFwkTag::APPMGR, "SetExtraEnv failed, ret: %{public}d", ret);
            return ret;
        }
    }

    if (!startMsg.atomicAccount.empty()) {
        ret = AppSpawnReqMsgAddExtInfo(reqHandle, MSG_EXT_NAME_ACCOUNT_ID,
            reinterpret_cast<const uint8_t*>(startMsg.atomicAccount.c_str()), startMsg.atomicAccount.size());
        if (ret) {
            TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnReqMsgAddExtInfo failed, ret: %{public}d", ret);
            return ret;
        }
    }

    return AppspawnSetExtMsgMore(startMsg, reqHandle);
}

int32_t AppSpawnClient::AppspawnSetExtMsgMore(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle)
{
    int32_t ret = 0;

    if (!startMsg.provisionType.empty()) {
        ret = AppSpawnReqMsgAddStringInfo(reqHandle, MSG_EXT_NAME_PROVISION_TYPE, startMsg.provisionType.c_str());
        if (ret) {
            TAG_LOGE(AAFwkTag::APPMGR, "SetExtraProvisionType failed, ret: %{public}d", ret);
            return ret;
        }
    }

    if (!startMsg.extensionSandboxPath.empty()) {
        ret = AppSpawnReqMsgAddStringInfo(reqHandle, MSG_EXT_NAME_APP_EXTENSION,
            startMsg.extensionSandboxPath.c_str());
        if (ret) {
            TAG_LOGE(AAFwkTag::APPMGR, "SetExtraExtensionSandboxDirs failed, ret: %{public}d", ret);
            return ret;
        }
    }

    if (!startMsg.processType.empty()) {
        ret = AppSpawnReqMsgAddExtInfo(reqHandle, MSG_EXT_NAME_PROCESS_TYPE,
            reinterpret_cast<const uint8_t*>(startMsg.processType.c_str()), startMsg.processType.size());
        if (ret) {
            TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnReqMsgAddExtInfo failed, ret: %{public}d", ret);
            return ret;
        }
    }

    std::string maxChildProcessStr = std::to_string(startMsg.maxChildProcess);
    ret = AppSpawnReqMsgAddExtInfo(reqHandle, MAX_CHILD_PROCESS,
        reinterpret_cast<const uint8_t*>(maxChildProcessStr.c_str()), maxChildProcessStr.size());
    if (ret) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send maxChildProcess failed, ret: %{public}d", ret);
        return ret;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "Send maxChildProcess %{public}s success.", maxChildProcessStr.c_str());

    if (!startMsg.fds.empty()) {
        ret = SetExtMsgFds(reqHandle, startMsg.fds);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "SetExtMsgFds failed, ret: %{public}d", ret);
            return ret;
        }
    }

    return ret;
}

int32_t AppSpawnClient::AppspawnCreateDefaultMsg(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle)
{
    TAG_LOGI(AAFwkTag::APPMGR, "AppspawnCreateDefaultMsg");
    int32_t ret = 0;
    do {
        ret = SetDacInfo(startMsg, reqHandle);
        if (ret) {
            TAG_LOGE(AAFwkTag::APPMGR, "SetDacInfo failed, ret: %{public}d", ret);
            break;
        }
        ret = AppSpawnReqMsgSetBundleInfo(reqHandle, startMsg.bundleIndex, startMsg.bundleName.c_str());
        if (ret) {
            TAG_LOGE(AAFwkTag::APPMGR, "SetBundleInfo failed, ret: %{public}d", ret);
            break;
        }
        ret = AppSpawnReqMsgSetAppInternetPermissionInfo(reqHandle, startMsg.allowInternet,
            startMsg.setAllowInternet);
        if (ret) {
            TAG_LOGE(AAFwkTag::APPMGR, "SetInternetPermissionInfo failed, ret: %{public}d", ret);
            break;
        }
        if (startMsg.ownerId.size()) {
            ret = AppSpawnReqMsgSetAppOwnerId(reqHandle, startMsg.ownerId.c_str());
            if (ret) {
                TAG_LOGE(AAFwkTag::APPMGR, "SetOwnerId %{public}s failed, ret: %{public}d",
                    startMsg.ownerId.c_str(), ret);
                break;
            }
        }
        ret = AppSpawnReqMsgSetAppAccessToken(reqHandle, startMsg.accessTokenIdEx);
        if (ret) {
            TAG_LOGE(AAFwkTag::APPMGR, "ret: %{public}d", ret);
            break;
        }
        ret = AppSpawnReqMsgSetAppDomainInfo(reqHandle, startMsg.hapFlags, startMsg.apl.c_str());
        if (ret) {
            TAG_LOGE(AAFwkTag::APPMGR,
                "SetDomainInfo failed, hapFlags is %{public}d, apl is %{public}s, ret: %{public}d",
                startMsg.hapFlags, startMsg.apl.c_str(), ret);
            break;
        }
        ret = SetStartFlags(startMsg, reqHandle);
        if (ret) {
            TAG_LOGE(AAFwkTag::APPMGR, "SetStartFlags failed, ret: %{public}d", ret);
            break;
        }
        ret = SetMountPermission(startMsg, reqHandle);
        if (ret) {
            TAG_LOGE(AAFwkTag::APPMGR, "SetMountPermission failed, ret: %{public}d", ret);
            break;
        }
        if (AppspawnSetExtMsg(startMsg, reqHandle)) {
            break;
        }
        return ret;
    } while (0);

    TAG_LOGI(AAFwkTag::APPMGR, "AppSpawnReqMsgFree");
    AppSpawnReqMsgFree(reqHandle);

    return ret;
}

bool AppSpawnClient::VerifyMsg(const AppSpawnStartMsg &startMsg)
{
    TAG_LOGI(AAFwkTag::APPMGR, "VerifyMsg");
    if (startMsg.code == MSG_APP_SPAWN ||
        startMsg.code == MSG_SPAWN_NATIVE_PROCESS) {
        if (startMsg.uid < 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid uid! [%{public}d]", startMsg.uid);
            return false;
        }

        if (startMsg.gid < 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid gid! [%{public}d]", startMsg.gid);
            return false;
        }

        if (startMsg.gids.size() > APP_MAX_GIDS) {
            TAG_LOGE(AAFwkTag::APPMGR, "too many app gids!");
            return false;
        }

        for (uint32_t i = 0; i < startMsg.gids.size(); ++i) {
            if (startMsg.gids[i] < 0) {
                TAG_LOGE(AAFwkTag::APPMGR, "invalid gids array! [%{public}d]", startMsg.gids[i]);
                return false;
            }
        }
        if (startMsg.procName.empty() || startMsg.procName.size() >= MAX_PROC_NAME_LEN) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid procName!");
            return false;
        }
    } else if (startMsg.code == MSG_GET_RENDER_TERMINATION_STATUS) {
        if (startMsg.pid < 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid pid!");
            return false;
        }
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid code!");
        return false;
    }

    return true;
}

// 预启动
int32_t AppSpawnClient::PreStartNWebSpawnProcess()
{
    TAG_LOGI(AAFwkTag::APPMGR, "PreStartNWebSpawnProcess");
    return OpenConnection();
}

int32_t AppSpawnClient::StartProcess(const AppSpawnStartMsg &startMsg, pid_t &pid)
{
    TAG_LOGI(AAFwkTag::APPMGR, "StartProcess");
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!VerifyMsg(startMsg)) {
        return ERR_INVALID_VALUE;  // 入参非法
    }

    int32_t ret = 0;
    AppSpawnReqMsgHandle reqHandle = nullptr;

    ret = OpenConnection();
    if (ret != 0) {
        return ret;
    }

    ret = AppSpawnReqMsgCreate(static_cast<AppSpawnMsgType>(startMsg.code), startMsg.procName.c_str(), &reqHandle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnReqMsgCreate faild.");
        return ret;
    }

    ret = AppspawnCreateDefaultMsg(startMsg, reqHandle);
    if (ret != 0) {
        return ret; // create msg failed
    }

    TAG_LOGI(AAFwkTag::APPMGR, "AppspawnSendMsg");
    AppSpawnResult result = {0};
    ret = AppSpawnClientSendMsg(handle_, reqHandle, &result);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "appspawn send msg faild!");
        return ret;
    }
    if (result.pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid invalid!");
        return ERR_APPEXECFWK_INVALID_PID;
    } else {
        pid = result.pid;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "pid = [%{public}d]", pid);
    return ret;
}

int32_t AppSpawnClient::GetRenderProcessTerminationStatus(const AppSpawnStartMsg &startMsg, int &status)
{
    TAG_LOGI(AAFwkTag::APPMGR, "GetRenderProcessTerminationStatus");
    int32_t ret = 0;
    AppSpawnReqMsgHandle reqHandle = nullptr;

    // 入参校验
    if (!VerifyMsg(startMsg)) {
        return ERR_INVALID_VALUE;  // 入参非法
    }

    ret = OpenConnection();
    if (ret != 0) {
        return ret;
    }

    ret = AppSpawnTerminateMsgCreate(startMsg.pid, &reqHandle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnTerminateMsgCreate faild.");
        return ret;
    }

    TAG_LOGI(AAFwkTag::APPMGR, "AppspawnSendMsg");
    AppSpawnResult result = {0};
    ret = AppSpawnClientSendMsg(handle_, reqHandle, &result);
    status = result.result;
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "appspawn send msg faild!");
        return ret;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "status = [%{public}d]", status);

    return ret;
}

int32_t AppSpawnClient::SetChildProcessTypeStartFlag(const AppSpawnReqMsgHandle &reqHandle,
    int32_t childProcessType)
{
    TAG_LOGD(AAFwkTag::APPMGR, "SetChildProcessTypeStartFlag, type:%{public}d", childProcessType);
    if (childProcessType != CHILD_PROCESS_TYPE_NOT_CHILD) {
        return AppSpawnReqMsgSetAppFlag(reqHandle, APP_FLAGS_CHILDPROCESS);
    }
    return ERR_OK;
}

int32_t AppSpawnClient::SetExtMsgFds(const AppSpawnReqMsgHandle &reqHandle,
    const std::map<std::string, int32_t> &fds)
{
    TAG_LOGI(AAFwkTag::APPMGR, "SetExtMsgFds, fds size:%{public}zu", fds.size());
    int32_t ret = ERR_OK;
    for (const auto &item : fds) {
        ret = AppSpawnReqMsgAddFd(reqHandle, item.first.c_str(), item.second);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "AppSpawnReqMsgAddFd failed, key:%{public}s, fd:%{public}d, ret:%{public}d",
                item.first.c_str(), item.second, ret);
            return ret;
        }
    }
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
