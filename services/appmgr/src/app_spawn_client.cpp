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

#include "hitrace_meter.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t CONNECT_RETRY_DELAY = 200 * 1000;  // 200ms
const int32_t CONNECT_RETRY_MAX_TIMES = 2;
const size_t SOCK_MAX_SEND_BUFFER = 5 * 1024; // 5KB
}  // namespace

AppSpawnClient::AppSpawnClient(bool isNWebSpawn)
{
    socket_ = std::make_shared<AppSpawnSocket>(isNWebSpawn);
    state_ = SpawnConnectionState::STATE_NOT_CONNECT;
}

ErrCode AppSpawnClient::OpenConnection()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (state_ == SpawnConnectionState::STATE_CONNECTED) {
        return ERR_OK;
    }

    if (!socket_) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to open connection without socket!");
        return ERR_APPEXECFWK_BAD_APPSPAWN_SOCKET;
    }

    int32_t retryCount = 1;
    ErrCode errCode = socket_->OpenAppSpawnConnection();
    while (FAILED(errCode) && retryCount <= CONNECT_RETRY_MAX_TIMES) {
        TAG_LOGW(AAFwkTag::APPMGR, "failed to OpenConnection, retry times %{public}d ...", retryCount);
        usleep(CONNECT_RETRY_DELAY);
        errCode = socket_->OpenAppSpawnConnection();
        retryCount++;
    }
    if (SUCCEEDED(errCode)) {
        state_ = SpawnConnectionState::STATE_CONNECTED;
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to openConnection, errorCode is %{public}08x", errCode);
        state_ = SpawnConnectionState::STATE_CONNECT_FAILED;
    }
    return errCode;
}

ErrCode AppSpawnClient::PreStartNWebSpawnProcess()
{
    TAG_LOGI(AAFwkTag::APPMGR, "PreStartNWebSpawnProcess");
    int32_t retryCount = 1;
    ErrCode errCode = PreStartNWebSpawnProcessImpl();
    while (FAILED(errCode) && retryCount <= CONNECT_RETRY_MAX_TIMES) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to Start NWebSpawn Process, retry times %{public}d ...", retryCount);
        usleep(CONNECT_RETRY_DELAY);
        errCode = PreStartNWebSpawnProcessImpl();
        retryCount++;
    }
    return errCode;
}

ErrCode AppSpawnClient::PreStartNWebSpawnProcessImpl()
{
    TAG_LOGI(AAFwkTag::APPMGR, "PreStartNWebSpawnProcessImpl");
    if (!socket_) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to Pre Start NWebSpawn Process without socket!");
        return ERR_APPEXECFWK_BAD_APPSPAWN_SOCKET;
    }

    // openconnection failed, return fail
    ErrCode result = OpenConnection();
    if (FAILED(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "connect to nwebspawn failed!");
        return result;
    }

    std::unique_ptr<AppSpawnClient, void (*)(AppSpawnClient *)> autoCloseConnection(
        this, [](AppSpawnClient *client) { client->CloseConnection(); });

    return result;
}

ErrCode AppSpawnClient::StartProcess(const AppSpawnStartMsg &startMsg, pid_t &pid)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!socket_) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to startProcess without socket!");
        return ERR_APPEXECFWK_BAD_APPSPAWN_SOCKET;
    }
    int32_t retryCount = 1;
    ErrCode errCode = StartProcessImpl(startMsg, pid);
    while (FAILED(errCode) && retryCount <= CONNECT_RETRY_MAX_TIMES) {
        TAG_LOGW(AAFwkTag::APPMGR, "failed to StartProcess, retry times %{public}d ...", retryCount);
        usleep(CONNECT_RETRY_DELAY);
        errCode = StartProcessImpl(startMsg, pid);
        retryCount++;
    }
    return errCode;
}

ErrCode AppSpawnClient::StartProcessImpl(const AppSpawnStartMsg &startMsg, pid_t &pid)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    ErrCode result = OpenConnection();
    // open connection failed, return fail
    if (FAILED(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "connect to appSpawn failed!");
        return result;
    }
    std::unique_ptr<AppSpawnClient, void (*)(AppSpawnClient *)> autoCloseConnection(
        this, [](AppSpawnClient *client) { client->CloseConnection(); });

    AppSpawnMsgWrapper msgWrapper;
    if (!msgWrapper.AssembleMsg(startMsg)) {
        TAG_LOGE(AAFwkTag::APPMGR, "AssembleMsg failed!");
        return ERR_APPEXECFWK_ASSEMBLE_START_MSG_FAILED;
    }
    AppSpawnPidMsg pidMsg;
    if (msgWrapper.IsValid()) {
        result = socket_->WriteMessage(msgWrapper.GetMsgBuf(), msgWrapper.GetMsgLength());
        if (FAILED(result)) {
            TAG_LOGE(AAFwkTag::APPMGR, "WriteMessage failed!");
            return result;
        }
        result = StartProcessForWriteMsg(msgWrapper);
        if (FAILED(result)) {
            TAG_LOGE(AAFwkTag::APPMGR, "StartProcessForWriteMsg failed!");
            return result;
        }
        result = socket_->ReadMessage(reinterpret_cast<void *>(pidMsg.pidBuf), LEN_PID);
        if (FAILED(result)) {
            TAG_LOGE(AAFwkTag::APPMGR, "ReadMessage failed!");
            return result;
        }
    }
    if (pidMsg.pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid pid!");
        result = ERR_APPEXECFWK_INVALID_PID;
    } else {
        pid = pidMsg.pid;
    }
    return result;
}

ErrCode AppSpawnClient::StartProcessForWriteMsg(const AppSpawnMsgWrapper &msgWrapper)
{
    ErrCode result = ERR_OK;
    result = WriteStrInfoMessage(msgWrapper.GetExtraInfoStr());
    if (FAILED(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write extra info failed!");
        return result;
    }
    return result;
}

ErrCode AppSpawnClient::WriteStrInfoMessage(const std::string &strInfo)
{
    ErrCode result = ERR_OK;
    if (strInfo.empty()) {
        return result;
    }

    // split msg
    const char *buff = strInfo.c_str();
    size_t leftLen = strInfo.size() + 1;
    TAG_LOGD(AAFwkTag::APPMGR, "strInfo length is %zu", leftLen);
    while (leftLen >= SOCK_MAX_SEND_BUFFER) {
        result = socket_->WriteMessage(buff, SOCK_MAX_SEND_BUFFER);
        if (FAILED(result)) {
            return result;
        }
        buff += SOCK_MAX_SEND_BUFFER;
        leftLen -= SOCK_MAX_SEND_BUFFER;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "strInfo: leftLen = %zu", leftLen);
    if (leftLen > 0) {
        result = socket_->WriteMessage(buff, leftLen);
    }
    return result;
}

ErrCode AppSpawnClient::GetRenderProcessTerminationStatus(const AppSpawnStartMsg &startMsg, int &status)
{
    if (!socket_) {
        TAG_LOGE(AAFwkTag::APPMGR, "socket_ is null!");
        return ERR_APPEXECFWK_BAD_APPSPAWN_SOCKET;
    }

    ErrCode result = OpenConnection();
    // open connection failed, return fail
    if (FAILED(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "connect to appSpawn failed!");
        return result;
    }
    std::unique_ptr<AppSpawnClient, void (*)(AppSpawnClient *)> autoCloseConnection(
        this, [](AppSpawnClient *client) { client->CloseConnection(); });

    AppSpawnMsgWrapper msgWrapper;
    if (!msgWrapper.AssembleMsg(startMsg)) {
        TAG_LOGE(AAFwkTag::APPMGR, "AssembleMsg failed!");
        return ERR_APPEXECFWK_ASSEMBLE_START_MSG_FAILED;
    }
    if (msgWrapper.IsValid()) {
        result = socket_->WriteMessage(msgWrapper.GetMsgBuf(), msgWrapper.GetMsgLength());
        if (FAILED(result)) {
            TAG_LOGE(AAFwkTag::APPMGR, "WriteMessage failed!");
            return result;
        }
        result = socket_->ReadMessage(reinterpret_cast<void *>(&status), sizeof(int));
        if (FAILED(result)) {
            TAG_LOGE(AAFwkTag::APPMGR, "ReadMessage failed!");
            return result;
        }
    }
    return result;
}

SpawnConnectionState AppSpawnClient::QueryConnectionState() const
{
    return state_;
}

void AppSpawnClient::CloseConnection()
{
    if (socket_ && state_ == SpawnConnectionState::STATE_CONNECTED) {
        socket_->CloseAppSpawnConnection();
    }
    state_ = SpawnConnectionState::STATE_NOT_CONNECT;
}

void AppSpawnClient::SetSocket(const std::shared_ptr<AppSpawnSocket> socket)
{
    socket_ = socket;
}
}  // namespace AppExecFwk
}  // namespace OHOS
