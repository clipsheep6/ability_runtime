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

#include "ability_manager_radar.h"

#include "ability_manager_errors.h"
#include "hisysevent.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFWK {

ContinueRadar &ContinueRadar::GetInstance()
{
    static ContinueRadar instance;
    return instance;
}

bool ContinueRadar::ClickIconContinue(const std::string& func)
{
    int32_t res = HiSysEventWrite(
        APP_CONTINUE_DOMAIN,
        APPLICATION_CONTINUE_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        ORG_PKG, ORG_PKG_NAME,
        FUNC, func,
        BIZ_SCENE, static_cast<int32_t>(BizScene::CLICK_ICON),
        BIZ_STAGE, static_cast<int32_t>(ClickIcon::CLICKICON_CONTINUE),
        STAGE_RES, static_cast<int32_t>(StageRes::STAGE_SUCC));
    if (res != ERR_OK) {
        HILOG_ERROR("ClickIconContinue error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool ContinueRadar::ClickIconStartAbility(const std::string& func, int32_t errCode)
{
    int32_t res = ERR_OK;
    StageRes stageRes = (errCode == ERR_OK) ? StageRes::STAGE_SUCC : StageRes::STAGE_FAIL;
    if (stageRes == StageRes::STAGE_SUCC) {
        res = HiSysEventWrite(
            APP_CONTINUE_DOMAIN,
            APPLICATION_CONTINUE_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::CLICK_ICON),
            BIZ_STAGE, static_cast<int32_t>(ClickIcon::CLICKICON_STARTABILITY),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_SUCC));
    } else {
        res = HiSysEventWrite(
            APP_CONTINUE_DOMAIN,
            APPLICATION_CONTINUE_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::CLICK_ICON),
            BIZ_STAGE, static_cast<int32_t>(ClickIcon::CLICKICON_STARTABILITY),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_FAIL),
            ERROR_CODE, errCode);
    }
    if (res != ERR_OK) {
        HILOG_ERROR("ClickIconStartAbility error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool ContinueRadar::ClickIconRecvOver(const std::string& func)
{
    int32_t res = ERR_OK;
    res = HiSysEventWrite(
        APP_CONTINUE_DOMAIN,
        APPLICATION_CONTINUE_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        ORG_PKG, ORG_PKG_NAME,
        FUNC, func,
        BIZ_SCENE, static_cast<int32_t>(BizScene::CLICK_ICON),
        BIZ_STAGE, static_cast<int32_t>(ClickIcon::CLICKICON_RECV_OVER),
        STAGE_RES, static_cast<int32_t>(StageRes::STAGE_SUCC));
    if (res != ERR_OK) {
        HILOG_ERROR("ClickIconRecvOver error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool ContinueRadar::SaveDataContinue(const std::string& func, int32_t errCode)
{
    int32_t res = ERR_OK;
    StageRes stageRes = (errCode == ERR_OK) ? StageRes::STAGE_SUCC : StageRes::STAGE_FAIL;
    if (stageRes == StageRes::STAGE_SUCC) {
        res = HiSysEventWrite(
            APP_CONTINUE_DOMAIN,
            APPLICATION_CONTINUE_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::SAVE_DATA),
            BIZ_STAGE, static_cast<int32_t>(SaveData::SAVEDATA_CONTINUE),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_SUCC));
    } else {
        res = HiSysEventWrite(
            APP_CONTINUE_DOMAIN,
            APPLICATION_CONTINUE_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::SAVE_DATA),
            BIZ_STAGE, static_cast<int32_t>(SaveData::SAVEDATA_CONTINUE),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_FAIL),
            ERROR_CODE, errCode);
    }
    if (res != ERR_OK) {
        HILOG_ERROR("SaveDataContinue error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool ContinueRadar::SaveDataRes(const std::string& func)
{
    int32_t res = HiSysEventWrite(
        APP_CONTINUE_DOMAIN,
        APPLICATION_CONTINUE_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        ORG_PKG, ORG_PKG_NAME,
        FUNC, func,
        BIZ_SCENE, static_cast<int32_t>(BizScene::SAVE_DATA),
        BIZ_STAGE, static_cast<int32_t>(SaveData::SAVEDATA_RES),
        STAGE_RES, static_cast<int32_t>(StageRes::STAGE_SUCC));
    if (res != ERR_OK) {
        HILOG_ERROR("SaveDataRes error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool ContinueRadar::SaveDataRemoteWant(const std::string& func, int32_t errCode)
{
    int32_t res = ERR_OK;
    StageRes stageRes = (errCode == ERR_OK) ? StageRes::STAGE_SUCC : StageRes::STAGE_FAIL;
    if (stageRes == StageRes::STAGE_SUCC) {
        res = HiSysEventWrite(
            APP_CONTINUE_DOMAIN,
            APPLICATION_CONTINUE_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::SAVE_DATA),
            BIZ_STAGE, static_cast<int32_t>(SaveData::SAVEDATA_REMOTE_WANT),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_SUCC),
            TO_CALL_PKG, DMS_PKG_NAME);
    } else {
        res = HiSysEventWrite(
            APP_CONTINUE_DOMAIN,
            APPLICATION_CONTINUE_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::SAVE_DATA),
            BIZ_STAGE, static_cast<int32_t>(SaveData::SAVEDATA_REMOTE_WANT),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_FAIL),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            TO_CALL_PKG, DMS_PKG_NAME,
            ERROR_CODE, errCode);
    }
    if (res != ERR_OK) {
        HILOG_ERROR("SaveDataRemoteWant error, res:%{public}d", res);
        return false;
    }
    return true;
}

} // namespace AAFWK
} // namespace OHOS