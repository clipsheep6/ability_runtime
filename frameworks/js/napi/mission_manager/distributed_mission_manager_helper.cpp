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

#include <string>

#include "distributed_mission_manager_helper.h"

#include "ability_manager_client.h"
#include "dms_sa_client.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "napi_common_data.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
    
bool CheckContinueKeyExist(napi_env &env, const napi_value &value)
{
    bool isSrcDeviceId = false;
    napi_has_named_property(env, value, "srcDeviceId", &isSrcDeviceId);
    bool isDstDeviceId = false;
    napi_has_named_property(env, value, "dstDeviceId", &isDstDeviceId);
    bool isMissionId = false;
    napi_has_named_property(env, value, "missionId", &isMissionId);
    bool isWantParam = false;
    napi_has_named_property(env, value, "wantParam", &isWantParam);
    if (!isSrcDeviceId && !isDstDeviceId && !isMissionId && !isWantParam) {
        TAG_LOGE(AAFwkTag::MISSION, "%{public}s, Wrong argument key.", __func__);
        return false;
    }
    return true;
}

bool CheckBundleNameExist(napi_env &env, const napi_value &value)
{
    bool isSrcDeviceId = false;
    napi_has_named_property(env, value, "srcDeviceId", &isSrcDeviceId);
    bool isDstDeviceId = false;
    napi_has_named_property(env, value, "dstDeviceId", &isDstDeviceId);
    bool isBundleName = false;
    napi_has_named_property(env, value, "bundleName", &isBundleName);
    bool isWantParam = false;
    napi_has_named_property(env, value, "wantParam", &isWantParam);
    if (!isSrcDeviceId && !isDstDeviceId && !isBundleName && !isWantParam) {
        return false;
    }
    return true;
}
}
}