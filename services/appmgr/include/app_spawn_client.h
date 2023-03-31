/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_SPAWN_CLIENT_H
#define OHOS_ABILITY_RUNTIME_APP_SPAWN_CLIENT_H

#include "nocopyable.h"
#include "app_spawn_msg_wrapper.h"
#include "app_spawn_socket.h"

namespace OHOS {
namespace AppExecFwk {
enum class SpawnConnectionState { STATE_NOT_CONNECT, STATE_CONNECTED, STATE_CONNECT_FAILED };

class AppSpawnClient {
public:
    /**
     * Constructor.
     */
    explicit AppSpawnClient(bool isNWebSpawn = false);

    /**
     * Destructor
     */
    virtual ~AppSpawnClient() = default;

    /**
     * Disable copy.
     */
    DISALLOW_COPY_AND_MOVE(AppSpawnClient);

    /**
     * Try connect to appSpawn.
     */
    ErrCode OpenConnection();

    /**
     * Close the connect of appSpawn.
     */
    void CloseConnection();

    /**
     * Start request to nwebspawn process.
     *
     */
    virtual ErrCode PreStartNWebSpawnProcess();

    /**
     * AppSpawnClient core function, Start request to appSpawn.
     *
     * @param startMsg, request message.
     * @param pid, pid of app process, get from appSpawn.
     */
    virtual ErrCode StartProcess(const AppSpawnStartMsg &startMsg, pid_t &pid);

    /**
     * Get render process termination status.
     *
     * @param startMsg, request message.
     * @param status, termination status of render process, get from appSpawn.
     */
    virtual ErrCode GetRenderProcessTerminationStatus(const AppSpawnStartMsg &startMsg, int &status);

    /**
     * Return the connect state.
     */
    SpawnConnectionState QueryConnectionState() const;

    /**
     * Set function, unit test also use it.
     */
    void SetSocket(const std::shared_ptr<AppSpawnSocket> socket);

private:
    /**
     * AppSpawnClient core function,
     *
     * @param startMsg, request message.
     * @param pid, pid of app process, get it from appSpawn.
     */
    ErrCode StartProcessImpl(const AppSpawnStartMsg &startMsg, pid_t &pid);

    /**
     * Start request to nwebspawn process.
     *
     */
    ErrCode PreStartNWebSpawnProcessImpl();

    /**
     * write hsp list to appspawn
     *
     * @param msgWrapper, request message wrapper.
     */
    ErrCode WriteHspList(AppSpawnMsgWrapper &msgWrapper);

private:
    std::shared_ptr<AppSpawnSocket> socket_;
    SpawnConnectionState state_ = SpawnConnectionState::STATE_NOT_CONNECT;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_SPAWN_CLIENT_H
