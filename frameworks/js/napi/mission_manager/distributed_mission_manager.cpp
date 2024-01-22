/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "distributed_mission_manager.h"

#include "ability_manager_client.h"
#include "dms_sa_client.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "napi_common_data.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
using AbilityManagerClient = AAFwk::AbilityManagerClient;
const std::string TAG = "NAPIMissionRegistration";
constexpr size_t VALUE_BUFFER_SIZE = 128;
const std::string CODE_KEY_NAME = "code";

napi_value GenerateBusinessError(const napi_env &env, int32_t errCode, const std::string &errMsg)
{
    napi_value code = nullptr;
    napi_create_int32(env, errCode, &code);
    napi_value msg = nullptr;
    napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &msg);
    napi_value businessError = nullptr;
    napi_create_error(env, nullptr, msg, &businessError);
    napi_set_named_property(env, businessError, CODE_KEY_NAME.c_str(), code);
    return businessError;
}

static int32_t ErrorCodeReturn(int32_t code)
{
    switch (code) {
        case NO_ERROR:
            return NO_ERROR;
        case CHECK_PERMISSION_FAILED:
            return PERMISSION_DENIED;
        case DMS_PERMISSION_DENIED:
            return PERMISSION_DENIED;
        case ERR_INVALID_VALUE:
            return PARAMETER_CHECK_FAILED;
        case INVALID_PARAMETERS_ERR:
            return PARAMETER_CHECK_FAILED;
        case REGISTER_REMOTE_MISSION_LISTENER_FAIL:
            return PARAMETER_CHECK_FAILED;
        case NO_MISSION_INFO_FOR_MISSION_ID:
            return NO_MISSION_INFO_FOR_MISSION_ID;
        case CONTINUE_REMOTE_UNINSTALLED_UNSUPPORT_FREEINSTALL:
            return REMOTE_UNINSTALLED_AND_UNSUPPORT_FREEINSTALL_FOR_CONTINUE;
        case CONTINUE_REMOTE_UNINSTALLED_SUPPORT_FREEINSTALL:
            return CONTINUE_WITHOUT_FREEINSTALL_FLAG;
        case OPERATION_DEVICE_NOT_INITIATOR_OR_TARGET:
            return OPERATION_DEVICE_NOT_INITIATOR_OR_TARGET;
        case CONTINUE_ALREADY_IN_PROGRESS:
            return CONTINUE_ALREADY_IN_PROGRESS;
        case MISSION_FOR_CONTINUING_IS_NOT_ALIVE:
            return MISSION_FOR_CONTINUING_IS_NOT_ALIVE;
        default:
            return SYSTEM_WORK_ABNORMALLY;
    };
}

static std::string ErrorMessageReturn(int32_t code)
{
    switch (code) {
        case NO_ERROR:
            return std::string();
        case PERMISSION_DENIED:
            return std::string("permission denied");
        case PARAMETER_CHECK_FAILED:
            return std::string("parameter check failed.");
        case SYSTEM_WORK_ABNORMALLY:
            return std::string("the system ability work abnormally.");
        case NO_MISSION_INFO_FOR_MISSION_ID:
            return std::string("failed to get the missionInfo of the specified missionId.");
        case REMOTE_UNINSTALLED_AND_UNSUPPORT_FREEINSTALL_FOR_CONTINUE:
            return std::string("the application is not installed on the "
                "remote end and installation-free is not supported.");
        case CONTINUE_WITHOUT_FREEINSTALL_FLAG:
            return std::string("The application is not installed on the remote end and "
                "installation-free is supported. Try again with the freeInstall flag.");
        case OPERATION_DEVICE_NOT_INITIATOR_OR_TARGET:
            return std::string("The operation device must be the device where the "
                "application to be continued is currently located or the target device.");
        case CONTINUE_ALREADY_IN_PROGRESS:
            return std::string("the local continuation task is already in progress.");
        case MISSION_FOR_CONTINUING_IS_NOT_ALIVE:
            return std::string("the mission for continuing is not alive, "
                "try again after restart this mission.");
        default:
            return std::string("the system ability work abnormally.");
    };
}

napi_value GetUndefined(const napi_env &env)
{
    napi_value nullResult = nullptr;
    napi_get_undefined(env, &nullResult);
    return nullResult;
}

bool SetStartSyncMissionsContext(const napi_env &env, const napi_value &value,
    SyncRemoteMissionsContext* context, std::string &errInfo)
{
    HILOG_INFO("%{public}s call.", __func__);
    bool isFixConflict = false;
    napi_has_named_property(env, value, "fixConflict", &isFixConflict);
    if (!isFixConflict) {
        HILOG_ERROR("%{public}s, Wrong argument name for fixConflict.", __func__);
        errInfo = "Parameter error. The key of \"MissionParameter\" must be fixConflict";
        return false;
    }
    napi_value fixConflictValue = nullptr;
    napi_get_named_property(env, value, "fixConflict", &fixConflictValue);
    if (fixConflictValue == nullptr) {
        HILOG_ERROR("%{public}s, not find fixConflict.", __func__);
        errInfo = "Parameter error. The value of \"fixConflict\" must not be undefined";
        return false;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, fixConflictValue, &valueType);
    if (valueType != napi_boolean) {
        HILOG_ERROR("%{public}s, fixConflict error type.", __func__);
        errInfo = "Parameter error. The type of \"fixConflict\" must be boolean";
        return false;
    }
    napi_get_value_bool(env, fixConflictValue, &context->fixConflict);
    bool isTag = false;
    napi_has_named_property(env, value, "tag", &isTag);
    if (!isTag) {
        HILOG_ERROR("%{public}s, Wrong argument name for tag.", __func__);
        errInfo = "Parameter error. The key of \"MissionParameter\" must be tag";
        return false;
    }
    napi_value tagValue = nullptr;
    napi_get_named_property(env, value, "tag", &tagValue);
    if (tagValue == nullptr) {
        HILOG_ERROR("%{public}s, not find tag.", __func__);
        errInfo = "Parameter error. The value of \"tag\" must not be undefined";
        return false;
    }
    napi_typeof(env, tagValue, &valueType);
    if (valueType != napi_number) {
        HILOG_ERROR("%{public}s, tag error type.", __func__);
        errInfo = "Parameter error. The type of \"tag\" must be number";
        return false;
    }
    napi_get_value_int64(env, tagValue, &context->tag);
    HILOG_INFO("%{public}s end.", __func__);
    return true;
}

bool SetSyncRemoteMissionsContext(const napi_env &env, const napi_value &value,
    bool isStart, SyncRemoteMissionsContext* context, std::string &errInfo)
{
    HILOG_INFO("%{public}s call.", __func__);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_object) {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        errInfo = "Parameter error. The type of \"parameter\" must be MissionParameter";
        return false;
    }
    napi_value deviceIdValue = nullptr;
    bool isDeviceId = false;
    napi_has_named_property(env, value, "deviceId", &isDeviceId);
    if (!isDeviceId) {
        HILOG_ERROR("%{public}s, Wrong argument name for deviceId.", __func__);
        errInfo = "Parameter error. The key of \"parameter\" must be deviceId";
        return false;
    }
    napi_get_named_property(env, value, "deviceId", &deviceIdValue);
    if (deviceIdValue == nullptr) {
        HILOG_ERROR("%{public}s, not find deviceId.", __func__);
        errInfo = "Parameter error. The value of \"deviceId\" must not be undefined";
        return false;
    }
    napi_typeof(env, deviceIdValue, &valueType);
    if (valueType != napi_string) {
        HILOG_ERROR("%{public}s, deviceId error type.", __func__);
        errInfo = "Parameter error. The type of \"deviceId\" must be string";
        return false;
    }

    char deviceId[VALUE_BUFFER_SIZE + 1] = {0};
    napi_get_value_string_utf8(env, deviceIdValue, deviceId, VALUE_BUFFER_SIZE + 1, &context->valueLen);
    if (context->valueLen > VALUE_BUFFER_SIZE) {
        HILOG_ERROR("%{public}s, deviceId length not correct", __func__);
        errInfo = "Parameter error. The length of \"deviceId\" must be less than " +
            std::to_string(VALUE_BUFFER_SIZE);
        return false;
    }
    context->deviceId = deviceId;

    if (isStart) {
        if (!SetStartSyncMissionsContext (env, value, context, errInfo)) {
            HILOG_ERROR("%{public}s, Wrong argument for start sync.", __func__);
            return false;
        }
    }
    HILOG_INFO("%{public}s end.", __func__);
    return true;
}

bool ProcessSyncInput(napi_env &env, napi_callback_info info, bool isStart,
    SyncRemoteMissionsContext* syncContext, std::string &errInfo)
{
    HILOG_INFO("%{public}s,called.", __func__);
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGS_ONE && argc != ARGS_TWO) {
        HILOG_ERROR("%{public}s, argument size error.", __func__);
        errInfo = "Parameter error. The type of \"number of parameters\" must be 1 or 2";
        return false;
    }
    syncContext->env = env;
    if (!SetSyncRemoteMissionsContext(env, argv[0], isStart, syncContext, errInfo)) {
        HILOG_ERROR("%{public}s, Wrong argument.", __func__);
        return false;
    }
    if (argc == ARGS_TWO) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[1], &valueType);
        if (valueType != napi_function) {
            HILOG_ERROR("%{public}s, callback error type.", __func__);
            errInfo = "Parameter error. The type of \"callback\" must be AsynCallback<void>: void";
            return false;
        }
        napi_create_reference(env, argv[1], 1, &syncContext->callbackRef);
    }
    HILOG_INFO("%{public}s, end.", __func__);
    return true;
}

void StartSyncRemoteMissionsAsyncWork(napi_env &env, const napi_value resourceName,
    SyncRemoteMissionsContext* syncContext)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_create_async_work(env, nullptr, resourceName,
        [](napi_env env, void* data) {
            SyncRemoteMissionsContext* syncContext = (SyncRemoteMissionsContext*)data;
            syncContext->result = AbilityManagerClient::GetInstance()->
                StartSyncRemoteMissions(syncContext->deviceId,
                syncContext->fixConflict, syncContext->tag);
        },
        [](napi_env env, napi_status status, void* data) {
            SyncRemoteMissionsContext* syncContext = (SyncRemoteMissionsContext*)data;
            // set result
            napi_value result[2] = { nullptr };
            napi_get_undefined(env, &result[1]);
            if (syncContext->result == 0) {
                napi_get_undefined(env, &result[0]);
            } else {
                int32_t errCode = ErrorCodeReturn(syncContext->result);
                result[0] = GenerateBusinessError(env, errCode, ErrorMessageReturn(errCode));
            }

            if (syncContext->callbackRef == nullptr) { // promise
                if (syncContext->result == 0) {
                    napi_resolve_deferred(env, syncContext->deferred, result[1]);
                } else {
                    napi_reject_deferred(env, syncContext->deferred, result[0]);
                }
            } else { // AsyncCallback
                napi_value callback = nullptr;
                napi_get_reference_value(env, syncContext->callbackRef, &callback);
                napi_value callResult;
                napi_call_function(env, nullptr, callback, ARGS_TWO, &result[0], &callResult);
                napi_delete_reference(env, syncContext->callbackRef);
            }
            napi_delete_async_work(env, syncContext->work);
            delete syncContext;
            syncContext = nullptr;
        },
        static_cast<void *>(syncContext),
        &syncContext->work);
        napi_queue_async_work_with_qos(env, syncContext->work, napi_qos_user_initiated);
    HILOG_INFO("%{public}s, end.", __func__);
}

napi_value NAPI_StartSyncRemoteMissions(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s, called.", __func__);
    std::string errInfo = "Parameter error";
    auto syncContext = new SyncRemoteMissionsContext();
    if (!ProcessSyncInput(env, info, true, syncContext, errInfo)) {
        delete syncContext;
        syncContext = nullptr;
        HILOG_ERROR("%{public}s, Wrong argument.", __func__);
        napi_throw(env, GenerateBusinessError(env, PARAMETER_CHECK_FAILED, errInfo));
        return GetUndefined(env);
    }
    napi_value result = nullptr;
    if (syncContext->callbackRef == nullptr) {
        napi_create_promise(env, &syncContext->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    StartSyncRemoteMissionsAsyncWork(env, resourceName, syncContext);
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

void StopSyncRemoteMissionsAsyncWork(napi_env &env, napi_value resourceName,
    SyncRemoteMissionsContext* syncContext)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_create_async_work(env, nullptr, resourceName,
        [](napi_env env, void* data) {
            SyncRemoteMissionsContext* syncContext = (SyncRemoteMissionsContext*)data;
            syncContext->result = AbilityManagerClient::GetInstance()->
                StopSyncRemoteMissions(syncContext->deviceId);
        },
        [](napi_env env, napi_status status, void* data) {
            SyncRemoteMissionsContext* syncContext = (SyncRemoteMissionsContext*)data;
            // set result
            napi_value result[2] = { nullptr };
            napi_get_undefined(env, &result[1]);
            if (syncContext->result == 0) {
                napi_get_undefined(env, &result[0]);
            } else {
                int32_t errCode = ErrorCodeReturn(syncContext->result);
                result[0] = GenerateBusinessError(env, errCode, ErrorMessageReturn(errCode));
            }

            if (syncContext->callbackRef == nullptr) { // promise
                if (syncContext->result == 0) {
                    napi_resolve_deferred(env, syncContext->deferred, result[1]);
                } else {
                    napi_reject_deferred(env, syncContext->deferred, result[0]);
                }
            } else { // AsyncCallback
                napi_value callback = nullptr;
                napi_get_reference_value(env, syncContext->callbackRef, &callback);
                napi_value callResult;
                napi_call_function(env, nullptr, callback, ARGS_TWO, &result[0], &callResult);
                napi_delete_reference(env, syncContext->callbackRef);
            }
            napi_delete_async_work(env, syncContext->work);
            delete syncContext;
            syncContext = nullptr;
        },
        static_cast<void *>(syncContext),
        &syncContext->work);
        napi_queue_async_work_with_qos(env, syncContext->work, napi_qos_user_initiated);
    HILOG_INFO("%{public}s, end.", __func__);
}

napi_value NAPI_StopSyncRemoteMissions(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s, called.", __func__);
    std::string errInfo = "Parameter error";
    auto syncContext = new SyncRemoteMissionsContext();
    if (!ProcessSyncInput(env, info, false, syncContext, errInfo)) {
        delete syncContext;
        syncContext = nullptr;
        HILOG_ERROR("%{public}s, Wrong argument.", __func__);
        napi_throw(env, GenerateBusinessError(env, PARAMETER_CHECK_FAILED, errInfo));
        return GetUndefined(env);
    }
    napi_value result = nullptr;
    if (syncContext->callbackRef == nullptr) {
        napi_create_promise(env, &syncContext->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    StopSyncRemoteMissionsAsyncWork(env, resourceName, syncContext);
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

RegisterMissionCB *CreateRegisterMissionCBCBInfo(napi_env &env)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto registerMissionCB = new (std::nothrow) RegisterMissionCB;
    if (registerMissionCB == nullptr) {
        HILOG_ERROR("%{public}s registerMissionCB == nullptr", __func__);
        return nullptr;
    }
    registerMissionCB->cbBase.cbInfo.env = env;
    registerMissionCB->cbBase.asyncWork = nullptr;
    registerMissionCB->cbBase.deferred = nullptr;
    registerMissionCB->callbackRef = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
    return registerMissionCB;
}

OnCB *CreateOnCBCBInfo(napi_env &env)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto onCB = new (std::nothrow) OnCB;
    if (onCB == nullptr) {
        HILOG_ERROR("%{public}s onCB == nullptr", __func__);
        return nullptr;
    }
    onCB->cbBase.cbInfo.env = env;
    onCB->cbBase.asyncWork = nullptr;
    onCB->cbBase.deferred = nullptr;
    onCB->callbackRef = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
    return onCB;
}

void RegisterMissionExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto registerMissionCB = (RegisterMissionCB*)data;

    std::lock_guard<std::mutex> autoLock(registrationLock_);
    sptr<NAPIRemoteMissionListener> registration;
    auto item = registration_.find(registerMissionCB->deviceId);
    if (item != registration_.end()) {
        HILOG_INFO("registration exits.");
        registration = registration_[registerMissionCB->deviceId];
    } else {
        HILOG_INFO("registration not exits.");
        registration = new (std::nothrow) NAPIRemoteMissionListener();
    }
    registerMissionCB->missionRegistration = registration;
    if (registerMissionCB->missionRegistration == nullptr) {
        HILOG_ERROR("%{public}s missionRegistration == nullptr.", __func__);
        registerMissionCB->result = -1;
        return;
    }
    registerMissionCB->missionRegistration->SetEnv(env);
    registerMissionCB->missionRegistration->
        SetNotifyMissionsChangedCBRef(registerMissionCB->missionRegistrationCB.callback[0]);
    registerMissionCB->missionRegistration->
        SetNotifySnapshotCBRef(registerMissionCB->missionRegistrationCB.callback[1]);
    registerMissionCB->missionRegistration->
        SetNotifyNetDisconnectCBRef(registerMissionCB->
            missionRegistrationCB.callback[2]); // 2 refers the second argument
    HILOG_INFO("set callback success.");

    registerMissionCB->result =
        AbilityManagerClient::GetInstance()->
        RegisterMissionListener(registerMissionCB->deviceId,
        registerMissionCB->missionRegistration);
    if (registerMissionCB->result == NO_ERROR) {
        HILOG_INFO("add registration.");
        registration_[registerMissionCB->deviceId] = registration;
    }
    HILOG_DEBUG("%{public}s end.deviceId:%{public}d ", __func__, registerMissionCB->result);
}

void RegisterMissionCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto registerMissionCB = static_cast<RegisterMissionCB *>(data);
    // set result
    napi_value result[2] = { nullptr };
    napi_get_undefined(env, &result[1]);
    if (registerMissionCB->result == 0) {
        napi_get_undefined(env, &result[0]);
    } else {
        int32_t errCode = ErrorCodeReturn(registerMissionCB->result);
        result[0] = GenerateBusinessError(env, errCode, ErrorMessageReturn(errCode));
    }

    ReturnValueToApplication(env, &result[0], registerMissionCB);
    delete registerMissionCB;
    registerMissionCB = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
}

void ReturnValueToApplication(napi_env &env, napi_value *result, RegisterMissionCB *registerMissionCB)
{
    if (registerMissionCB->callbackRef == nullptr) { // promise
        if (registerMissionCB->result == 0) {
            napi_resolve_deferred(env, registerMissionCB->cbBase.deferred, result[1]);
        } else {
            napi_reject_deferred(env, registerMissionCB->cbBase.deferred, result[0]);
        }
    } else { // AsyncCallback
        napi_value callback = nullptr;
        napi_get_reference_value(env, registerMissionCB->callbackRef, &callback);
        napi_value callResult;
        napi_call_function(env, nullptr, callback, ARGS_TWO, &result[0], &callResult);
        napi_delete_reference(env, registerMissionCB->callbackRef);
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, registerMissionCB->cbBase.asyncWork));
}

napi_value RegisterMissionAsync(napi_env env, RegisterMissionCB *registerMissionCB)
{
    HILOG_INFO("%{public}s asyncCallback.", __func__);
    if (registerMissionCB == nullptr) {
        HILOG_ERROR("%{public}s, registerMissionCB == nullptr.", __func__);
        napi_throw(env, GenerateBusinessError(env, SYSTEM_WORK_ABNORMALLY, ErrorMessageReturn(SYSTEM_WORK_ABNORMALLY)));
        return nullptr;
    }
    napi_value result = nullptr;
    if (registerMissionCB->callbackRef == nullptr) {
        napi_create_promise(env, &registerMissionCB->cbBase.deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env,
        nullptr,
        resourceName,
        RegisterMissionExecuteCB,
        RegisterMissionCallbackCompletedCB,
        static_cast<void *>(registerMissionCB),
        &registerMissionCB->cbBase.asyncWork);
    napi_queue_async_work(env, registerMissionCB->cbBase.asyncWork);
    HILOG_INFO("%{public}s asyncCallback end.", __func__);
    return result;
}

bool CheckMissionCallbackProperty(napi_env &env, const napi_value &value, std::string &errInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    bool isFirstCallback = false;
    napi_has_named_property(env, value, "notifyMissionsChanged", &isFirstCallback);
    bool isSecondCallback = false;
    napi_has_named_property(env, value, "notifySnapshot", &isSecondCallback);
    bool isThirdCallback = false;
    napi_has_named_property(env, value, "notifyNetDisconnect", &isThirdCallback);
    if (!isFirstCallback || !isSecondCallback || !isThirdCallback) {
        HILOG_ERROR("%{public}s, Wrong argument name for callback.", __func__);
        errInfo = "Parameter error. The type of \"options\" must be MissionCallback";
        return false;
    }
    HILOG_INFO("%{public}s called end.", __func__);
    return true;
}

bool SetCallbackReference(napi_env &env, const napi_value &value,
    RegisterMissionCB *registerMissionCB, std::string &errInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (!CheckMissionCallbackProperty(env, value, errInfo)) {
        return false;
    }
    napi_value jsMethod = nullptr;
    napi_get_named_property(env, value, "notifyMissionsChanged", &jsMethod);
    if (jsMethod == nullptr) {
        HILOG_ERROR("%{public}s, not find callback notifyMissionsChanged.", __func__);
        errInfo = "Parameter error. The value of \"notifyMissionsChanged\" must not be undefined";
        return false;
    }
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, jsMethod, &valuetype);
    if (valuetype != napi_function) {
        HILOG_ERROR("%{public}s, notifyMissionsChanged callback error type.", __func__);
        errInfo = "Parameter error. The type of \"notifyMissionsChanged\" must be function";
        return false;
    }
    napi_create_reference(env, jsMethod, 1, &registerMissionCB->missionRegistrationCB.callback[0]);
    napi_get_named_property(env, value, "notifySnapshot", &jsMethod);
    if (jsMethod == nullptr) {
        HILOG_ERROR("%{public}s, not find callback notifySnapshot.", __func__);
        errInfo = "Parameter error. The value of \"notifySnapshot\" must not be undefined";
        return false;
    }
    napi_typeof(env, jsMethod, &valuetype);
    if (valuetype != napi_function) {
        HILOG_ERROR("%{public}s, notifySnapshot callback error type.", __func__);
        errInfo = "Parameter error. The type of \"notifySnapshot\" must be function";
        return false;
    }
    napi_create_reference(env, jsMethod, 1, &registerMissionCB->missionRegistrationCB.callback[1]);
    napi_get_named_property(env, value, "notifyNetDisconnect", &jsMethod);
    if (jsMethod == nullptr) {
        HILOG_ERROR("%{public}s, not find callback notifyNetDisconnect.", __func__);
        errInfo = "Parameter error. The value of \"notifyNetDisconnect\" must not be undefined";
        return false;
    }
    napi_typeof(env, jsMethod, &valuetype);
    if (valuetype != napi_function) {
        HILOG_ERROR("%{public}s, notifyNetDisconnect callback error type.", __func__);
        errInfo = "Parameter error. The type of \"notifyNetDisconnect\" must be function";
        return false;
    }
    // 2 refers the second argument
    napi_create_reference(env, jsMethod, 1, &registerMissionCB->missionRegistrationCB.callback[2]);
    HILOG_INFO("%{public}s called end.", __func__);
    return true;
}

bool CreateCallbackReference(napi_env &env, const napi_value &value,
    RegisterMissionCB *registerMissionCB, std::string &errInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, value, &valuetype);
    if (valuetype == napi_object) {
        if (!SetCallbackReference(env, value, registerMissionCB, errInfo)) {
            HILOG_ERROR("%{public}s, Wrong callback.", __func__);
            return false;
        }
    } else {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        errInfo = "Parameter error. The type of \"options\" must be MissionCallback";
        return false;
    }
    HILOG_INFO("%{public}s called end.", __func__);
    return true;
}

bool CreateOnCallbackReference(napi_env &env, const napi_value &jsMethod,
    OnCB *onCB, std::string &errInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, jsMethod, &valuetype);
    if (valuetype != napi_function) {
        HILOG_ERROR("%{public}s, onCallback error type.", __func__);
        errInfo = "Parameter error. The type of \"onCallback\" must be function";
        return false;
    }
    napi_create_reference(env, jsMethod, 1, &onCB->onCallbackCB.callback);
    napi_create_reference(env, jsMethod, 1, &onCB->callbackRef);
    onCB->onCallbackCB.napiCallback =
        std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference *>(onCB->onCallbackCB.callback));
    HILOG_INFO("%{public}s called end.", __func__);
    return true;
}

bool RegisterMissionWrapDeviceId(napi_env &env, napi_value &argc,
    RegisterMissionCB *registerMissionCB, std::string &errInfo)
{
    napi_valuetype valueType = napi_undefined;
    bool isDeviceId = false;
    napi_has_named_property(env, argc, "deviceId", &isDeviceId);
    napi_typeof(env, argc, &valueType);
    if (!isDeviceId || valueType != napi_object) {
        HILOG_ERROR("%{public}s, Wrong argument name for deviceId.", __func__);
        errInfo = "Parameter error. The key of \"MissionDeviceInfo\" must be deviceId";
        return false;
    }

    napi_value napiDeviceId = nullptr;
    napi_get_named_property(env, argc, "deviceId", &napiDeviceId);
    if (napiDeviceId == nullptr) {
        HILOG_ERROR("%{public}s, not find deviceId.", __func__);
        errInfo = "Parameter error. The value of \"deviceId\" must not be undefined";
        return false;
    }
    napi_typeof(env, napiDeviceId, &valueType);
    if (valueType != napi_string) {
        HILOG_ERROR("%{public}s, deviceId error type.", __func__);
        errInfo = "Parameter error. The type of \"deviceId\" must be string";
        return false;
    }
    char deviceId[VALUE_BUFFER_SIZE + 1] = {0};
    size_t valueLen = 0;
    napi_get_value_string_utf8(env, napiDeviceId, deviceId, VALUE_BUFFER_SIZE + 1, &valueLen);
    if (valueLen > VALUE_BUFFER_SIZE) {
        HILOG_ERROR("%{public}s, deviceId length not correct", __func__);
        errInfo = "Parameter error. The length of \"deviceId\" must be less than " +
            std::to_string(VALUE_BUFFER_SIZE);
        return false;
    }
    registerMissionCB->deviceId = std::string(deviceId);
    return true;
}

bool OnWrapType(napi_env &env, napi_value &argc,
    OnCB *onCB, std::string &errInfo)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argc, &valueType);
    if (valueType != napi_string) {
        HILOG_ERROR("%{public}s, Wrong argument name for type.", __func__);
        errInfo = "Parameter error. The type of \"type\" must be string";
        return false;
    }
    std::string type = AppExecFwk::UnwrapStringFromJS(env, argc, "");
    if (type != "continueStateChange") {
        HILOG_ERROR("%{public}s, not find type.", __func__);
        errInfo = "Parameter error. The value of \"type\" must not be continueStateChange";
        return false;
    }
    onCB->type = type;
    return true;
}

napi_value RegisterMissionWrap(napi_env &env, napi_callback_info info,
    RegisterMissionCB *registerMissionCB, std::string &errInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argcAsync = 3;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr);
    if (argcAsync != ARGS_TWO && argcAsync != ARGS_THREE) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        errInfo = "Parameter error. The type of \"number of parameters\" must be 2 or 3";
        return nullptr;
    }

    if (!RegisterMissionWrapDeviceId(env, args[0], registerMissionCB, errInfo)) {
        HILOG_INFO("%{public}s, RegisterMissionWrapDeviceId failed.", __func__);
        return nullptr;
    }
    if (argcAsync > 1 && !CreateCallbackReference(env, args[1], registerMissionCB, errInfo)) {
        return nullptr;
    }
    napi_valuetype valueType = napi_undefined;
    if (argcAsync == ARGS_THREE) {
        napi_typeof(env, args[ARGS_TWO], &valueType);
        if (valueType != napi_function) {
            HILOG_ERROR("%{public}s, callback error type.", __func__);
            errInfo = "Parameter error. The type of \"options\" must be MissionCallback";
            return nullptr;
        }
        napi_create_reference(env, args[ARGS_TWO], 1, &registerMissionCB->callbackRef);
    }

    napi_value ret = RegisterMissionAsync(env, registerMissionCB);
    HILOG_INFO("%{public}s called end.", __func__);
    return ret;
}

void OnExecuteCB(napi_env &env, OnCB *onCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::lock_guard<std::mutex> autoLock(onLock_);
    sptr<NAPIRemoteOnListener> registrationOfOn;
    auto item = registrationOfOn_.find(onCB->type);
    if (item != registrationOfOn_.end()) {
        HILOG_INFO("registrationOfOn exits.");
        registrationOfOn = registrationOfOn_[onCB->type];
    } else {
        HILOG_INFO("registrationOfOn not exits.");
        registrationOfOn = new (std::nothrow) NAPIRemoteOnListener();
    }
    onCB->onRegistration = registrationOfOn;
    if (onCB->onRegistration == nullptr) {
        HILOG_ERROR("%{public}s onRegistration == nullptr.", __func__);
        onCB->result = -1;
        int32_t errCode = ErrorCodeReturn(onCB->result);
        napi_throw(env, GenerateBusinessError(env, errCode, ErrorMessageReturn(errCode)));
        return;
    }
    onCB->onRegistration->SetEnv(env);
    std::vector<std::shared_ptr<NativeReference>> vecCallback = onCB->onRegistration->GetOnCallbackCBRef();
    bool result = false;
    for (auto ele = vecCallback.begin(); ele != vecCallback.end(); ++ele) {
        napi_strict_equals(env, (*ele)->GetNapiValue(), onCB->onCallbackCB.napiCallback->GetNapiValue(), &result);
        if (result) {
            HILOG_ERROR("Object does match value.");
            return;
        }
    }
    onCB->onRegistration->
        SetOnCallbackCBRef(onCB->onCallbackCB.napiCallback);
    HILOG_INFO("set callback success.");
    onCB->result = DmsSaClient::GetInstance().AddListener(onCB->type, onCB->onRegistration);
    if (onCB->result == NO_ERROR) {
        HILOG_INFO("add registrationOfOn success.");
        registrationOfOn_[onCB->type] = registrationOfOn;
    } else {
        HILOG_INFO("add registrationOfOn failed");
    }
    HILOG_INFO("%{public}s called end.", __func__);
}

napi_value OnWrap(napi_env &env, napi_callback_info info,
    OnCB *onCB, std::string &errInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argcAsync = 2;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr);
    if (argcAsync != ARGS_TWO) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        errInfo = "Parameter error. The type of \"number of parameters\" must be 2";
        return nullptr;
    }
    if (!OnWrapType(env, args[0], onCB, errInfo)) {
        HILOG_INFO("%{public}s, OnWrapType failed.", __func__);
        return nullptr;
    }
    if (!CreateOnCallbackReference(env, args[1], onCB, errInfo)) {
        return nullptr;
    }
    OnExecuteCB(env, onCB);
    if (onCB->result != 0) {
        int32_t errCode = ErrorCodeReturn(onCB->result);
        napi_throw(env, GenerateBusinessError(env, errCode, ErrorMessageReturn(errCode)));
    }
    onCB->onCallbackCB.napiCallback = nullptr;
    if (onCB->callbackRef != nullptr) {
        napi_delete_reference(env, onCB->callbackRef);
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    HILOG_INFO("%{public}s called end.", __func__);
    return result;
}

void OffExecuteCB(napi_env env, OnCB *onCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::lock_guard<std::mutex> autoLock(onLock_);
    sptr<NAPIRemoteOnListener> registrationOfOn;
    auto item = registrationOfOn_.find(onCB->type);
    if (item != registrationOfOn_.end()) {
        HILOG_INFO("registrationOfOff exits.");
        registrationOfOn = registrationOfOn_[onCB->type];
    } else {
        HILOG_INFO("registrationOfOff not exits.");
        onCB->result = -1;
        return;
    }
    onCB->onRegistration = registrationOfOn;
    onCB->onRegistration->DelOnCallbackCBRef(env, onCB->onCallbackCB.napiCallback);
    if (!onCB->onRegistration->GetOnCallbackCBRef().empty()) {
        HILOG_INFO("There are still other remaining callback");
        return;
    }
    DmsSaClient::GetInstance().DelListener(onCB->type, onCB->onRegistration);
    if (onCB->result == NO_ERROR) {
        HILOG_INFO("remove registration.");
        registrationOfOn_.erase(onCB->type);
    }
    HILOG_DEBUG("%{public}s end.type:%{public}d ", __func__, onCB->result);
}

napi_value OffWrap(napi_env &env, napi_callback_info info,
    OnCB *onCB, std::string &errInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argcAsync = 2;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr);
    if (argcAsync != ARGS_ONE && argcAsync != ARGS_TWO) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        errInfo = "Parameter error. The type of \"number of parameters\" must be 1 or 2";
        return nullptr;
    }
    if (!OnWrapType(env, args[0], onCB, errInfo)) {
        HILOG_INFO("%{public}s, OffWrapType failed.", __func__);
        return nullptr;
    }
    if (argcAsync == ARGS_TWO && !CreateOnCallbackReference(env, args[1], onCB, errInfo)) {
        return nullptr;
    }
    OffExecuteCB(env, onCB);
    if (onCB->result != 0) {
        int32_t errCode = ErrorCodeReturn(onCB->result);
        napi_throw(env, GenerateBusinessError(env, errCode, ErrorMessageReturn(errCode)));
    }
    if (onCB->callbackRef != nullptr) {
        napi_delete_reference(env, onCB->callbackRef);
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    HILOG_INFO("%{public}s called end.", __func__);
    return result;
}

napi_value NAPI_RegisterMissionListener(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::string errInfo = "Parameter error";
    RegisterMissionCB *registerMissionCB = CreateRegisterMissionCBCBInfo(env);
    if (registerMissionCB == nullptr) {
        HILOG_ERROR("%{public}s registerMissionCB == nullptr", __func__);
        napi_throw(env, GenerateBusinessError(env, SYSTEM_WORK_ABNORMALLY, ErrorMessageReturn(SYSTEM_WORK_ABNORMALLY)));
        return GetUndefined(env);
    }

    napi_value ret = RegisterMissionWrap(env, info, registerMissionCB, errInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        delete registerMissionCB;
        registerMissionCB = nullptr;
        napi_throw(env, GenerateBusinessError(env, PARAMETER_CHECK_FAILED, errInfo));
        return GetUndefined(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

napi_value NAPI_On(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::string errInfo = "Parameter error";
    OnCB *onCB = CreateOnCBCBInfo(env);
    if (onCB == nullptr) {
        HILOG_ERROR("%{public}s onCB == nullptr", __func__);
        napi_throw(env, GenerateBusinessError(env, SYSTEM_WORK_ABNORMALLY, ErrorMessageReturn(SYSTEM_WORK_ABNORMALLY)));
        return GetUndefined(env);
    }

    napi_value ret = OnWrap(env, info, onCB, errInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        delete onCB;
        onCB = nullptr;
        napi_throw(env, GenerateBusinessError(env, PARAMETER_CHECK_FAILED, errInfo));
        return GetUndefined(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

napi_value NAPI_Off(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::string errInfo = "Parameter error";
    OnCB *onCB = CreateOnCBCBInfo(env);
    if (onCB == nullptr) {
        HILOG_ERROR("%{public}s onCB == nullptr", __func__);
        napi_throw(env, GenerateBusinessError(env, SYSTEM_WORK_ABNORMALLY, ErrorMessageReturn(SYSTEM_WORK_ABNORMALLY)));
        return GetUndefined(env);
    }

    napi_value ret = OffWrap(env, info, onCB, errInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        delete onCB;
        onCB = nullptr;
        napi_throw(env, GenerateBusinessError(env, PARAMETER_CHECK_FAILED, errInfo));
        return GetUndefined(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

napi_value NAPI_ContinueState(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value continueState = nullptr;
    napi_create_object(env, &continueState);
    napi_value active = nullptr;
    napi_value inActive = nullptr;
    napi_create_int32(env, 0, &active);
    napi_create_int32(env, 1, &inActive);
    napi_set_named_property(env, continueState, "ACTIVE", active);
    napi_set_named_property(env, continueState, "INACTIVE", inActive);
    HILOG_INFO("%{public}s end.", __func__);
    return continueState;
}

void NAPIMissionContinue::SetEnv(const napi_env &env)
{
    env_ = env;
}

NAPIRemoteMissionListener::~NAPIRemoteMissionListener()
{
    if (env_ == nullptr) {
        return;
    }
    if (notifyMissionsChangedRef_ != nullptr) {
        napi_delete_reference(env_, notifyMissionsChangedRef_);
        notifyMissionsChangedRef_ = nullptr;
    }
    if (notifySnapshotRef_ != nullptr) {
        napi_delete_reference(env_, notifySnapshotRef_);
        notifySnapshotRef_ = nullptr;
    }
    if (notifyNetDisconnectRef_ != nullptr) {
        napi_delete_reference(env_, notifyNetDisconnectRef_);
        notifyNetDisconnectRef_ = nullptr;
    }
}

NAPIRemoteOnListener::~NAPIRemoteOnListener() {}

void NAPIRemoteMissionListener::SetEnv(const napi_env &env)
{
    env_ = env;
}

void NAPIRemoteOnListener::SetEnv(const napi_env &env)
{
    env_ = env;
}

void NAPIRemoteMissionListener::SetNotifyMissionsChangedCBRef(const napi_ref &ref)
{
    notifyMissionsChangedRef_ = ref;
}

void NAPIRemoteOnListener::SetOnCallbackCBRef(std::shared_ptr<NativeReference> &ref)
{
    callbacks_.push_back(ref);
}

std::vector<std::shared_ptr<NativeReference>> NAPIRemoteOnListener::GetOnCallbackCBRef()
{
    return callbacks_;
}

bool NAPIRemoteOnListener::DelOnCallbackCBRef(napi_env env, std::shared_ptr<NativeReference> &ref)
{
    bool result = false;
    for (auto ele = callbacks_.begin(); ele != callbacks_.end(); ++ele) {
        napi_strict_equals(env, (*ele)->GetNapiValue(), ref->GetNapiValue(), &result);
        if (result) {
            HILOG_ERROR("Object does match value, need delete this callback.");
            callbacks_.erase(ele);
            return result;
        }
    }

    return result;
}

void NAPIRemoteMissionListener::SetNotifySnapshotCBRef(const napi_ref &ref)
{
    notifySnapshotRef_ = ref;
}

void NAPIRemoteMissionListener::SetNotifyNetDisconnectCBRef(const napi_ref &ref)
{
    notifyNetDisconnectRef_ = ref;
}

void UvWorkNotifyMissionChanged(uv_work_t *work, int status)
{
    HILOG_INFO("UvWorkNotifyMissionChanged start, uv_queue_work");
    if (work == nullptr) {
        HILOG_ERROR("UvWorkNotifyMissionChanged, work is nullptr");
        return;
    }
    RegisterMissionCB *registerMissionCB = static_cast<RegisterMissionCB *>(work->data);
    if (registerMissionCB == nullptr) {
        HILOG_ERROR("UvWorkNotifyMissionChanged, registerMissionCB is null");
        delete work;
        return;
    }
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(registerMissionCB->cbBase.cbInfo.env, &scope);
    if (scope == nullptr) {
        delete registerMissionCB;
        registerMissionCB = nullptr;
        delete work;
        return;
    }

    napi_value result = nullptr;
    result =
        WrapString(registerMissionCB->cbBase.cbInfo.env, registerMissionCB->deviceId.c_str(), "deviceId");

    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(registerMissionCB->cbBase.cbInfo.env, &undefined);
    napi_value callResult = nullptr;
    napi_get_reference_value(
        registerMissionCB->cbBase.cbInfo.env, registerMissionCB->cbBase.cbInfo.callback, &callback);

    napi_call_function(registerMissionCB->cbBase.cbInfo.env, undefined, callback, 1, &result, &callResult);

    napi_close_handle_scope(registerMissionCB->cbBase.cbInfo.env, scope);
    delete registerMissionCB;
    registerMissionCB = nullptr;
    delete work;
    HILOG_INFO("UvWorkNotifyMissionChanged, uv_queue_work end");
}

void UvWorkOnCallback(uv_work_t *work, int status)
{
    HILOG_INFO("UvWorkOnCallback, uv_queue_work");
    if (work == nullptr) {
        HILOG_ERROR("UvWorkOnCallback, work is null");
        return;
    }
    OnCB *onCB = static_cast<OnCB *>(work->data);
    if (onCB == nullptr) {
        HILOG_ERROR("UvWorkOnCallback, onCB is null");
        delete work;
        return;
    }

    napi_value result[3] = {nullptr};
    napi_create_int32(onCB->cbBase.cbInfo.env, onCB->continueState, &result[0]);
    napi_create_object(onCB->cbBase.cbInfo.env, &result[1]);
    napi_create_object(onCB->cbBase.cbInfo.env, &result[ARGS_TWO]);
    std::string napiValue1 = onCB->srcDeviceId;
    std::string napiValue2 = onCB->bundleName;
    napi_value jsValue1 = nullptr;
    napi_value jsValue2 = nullptr;
    napi_create_string_utf8(onCB->cbBase.cbInfo.env, napiValue1.c_str(), NAPI_AUTO_LENGTH, &jsValue1);
    napi_create_string_utf8(onCB->cbBase.cbInfo.env, napiValue2.c_str(), NAPI_AUTO_LENGTH, &jsValue2);
    std::string napiState = "state";
    std::string paramName1 = "srcDeviceId";
    std::string paramName2 = "bundleName";
    std::string napiInfo = "info";
    napi_set_named_property(onCB->cbBase.cbInfo.env, result[1], paramName1.c_str(), jsValue1);
    napi_set_named_property(onCB->cbBase.cbInfo.env, result[1], paramName2.c_str(), jsValue2);
    napi_set_named_property(onCB->cbBase.cbInfo.env, result[ARGS_TWO], napiState.c_str(), result[0]);
    napi_set_named_property(onCB->cbBase.cbInfo.env, result[ARGS_TWO], napiInfo.c_str(), result[1]);
    for (auto ele = onCB->cbBase.cbInfo.vecCallbacks.begin(); ele != onCB->cbBase.cbInfo.vecCallbacks.end(); ++ele) {
        napi_value undefined = nullptr;
        napi_get_undefined(onCB->cbBase.cbInfo.env, &undefined);
        napi_value callResult = nullptr;
        napi_call_function(onCB->cbBase.cbInfo.env, undefined,
            (*ele)->GetNapiValue(), ARGS_ONE, &result[ARGS_TWO], &callResult);
    }
    delete onCB;
    onCB = nullptr;
    delete work;
    HILOG_INFO("UvWorkOnCallback, uv_queue_work end");
}

void NAPIRemoteMissionListener::NotifyMissionsChanged(const std::string &deviceId)
{
    HILOG_INFO("%{public}s, called.", __func__);
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("%{public}s, loop == nullptr.", __func__);
        return;
    }

    uv_work_t *work = new uv_work_t;

    auto registerMissionCB = new (std::nothrow) RegisterMissionCB;
    if (registerMissionCB == nullptr) {
        HILOG_ERROR("%{public}s, registerMissionCB == nullptr.", __func__);
        delete work;
        return;
    }
    registerMissionCB->cbBase.cbInfo.env = env_;
    registerMissionCB->cbBase.cbInfo.callback = notifyMissionsChangedRef_;
    registerMissionCB->deviceId = deviceId;
    work->data = static_cast<void *>(registerMissionCB);

    int rev = uv_queue_work_with_qos(
        loop, work, [](uv_work_t *work) {}, UvWorkNotifyMissionChanged, uv_qos_user_initiated);
    if (rev != 0) {
        delete registerMissionCB;
        registerMissionCB = nullptr;
        delete work;
    }
    HILOG_INFO("%{public}s, end.", __func__);
}

void NAPIRemoteOnListener::OnCallback(const uint32_t continueState, const std::string &srcDeviceId,
    const std::string &bundleName)
{
    HILOG_INFO("%{public}s, called.", __func__);
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("%{public}s, loop == nullptr.", __func__);
        return;
    }

    uv_work_t *work = new uv_work_t;

    auto onCB = new (std::nothrow) OnCB;
    if (onCB == nullptr) {
        HILOG_ERROR("%{public}s, onCB == nullptr.", __func__);
        delete work;
        return;
    }
    for (auto ele = callbacks_.begin(); ele != callbacks_.end(); ++ele) {
        onCB->cbBase.cbInfo.vecCallbacks.push_back(*ele);
    }
    onCB->cbBase.cbInfo.env = env_;
    onCB->continueState = continueState;
    onCB->srcDeviceId = srcDeviceId;
    onCB->bundleName = bundleName;
    work->data = static_cast<void *>(onCB);

    int rev = uv_queue_work_with_qos(
        loop, work, [](uv_work_t *work) {}, UvWorkOnCallback, uv_qos_user_initiated);
    if (rev != 0) {
        delete onCB;
        onCB = nullptr;
        delete work;
    }
    HILOG_INFO("%{public}s, OnCallback end.", __func__);
}

void UvWorkNotifySnapshot(uv_work_t *work, int status)
{
    HILOG_INFO("UvWorkNotifySnapshot enter, uv_queue_work");
    if (work == nullptr) {
        HILOG_ERROR("UvWorkNotifySnapshot, work is null");
        return;
    }
    RegisterMissionCB *registerMissionCB = static_cast<RegisterMissionCB *>(work->data);
    if (registerMissionCB == nullptr) {
        HILOG_ERROR("UvWorkNotifySnapshot, registerMissionCB is null");
        delete work;
        return;
    }
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(registerMissionCB->cbBase.cbInfo.env, &scope);
    if (scope == nullptr) {
        delete registerMissionCB;
        registerMissionCB = nullptr;
        delete work;
        return;
    }

    napi_value result[2] = {nullptr};
    result[0] =
        WrapString(registerMissionCB->cbBase.cbInfo.env, registerMissionCB->deviceId.c_str(), "deviceId");
    result[1] =
        CreateInt32(registerMissionCB->cbBase.cbInfo.env, registerMissionCB->missionId, "missionId");
    CallbackReturn(&result[0], registerMissionCB);

    napi_close_handle_scope(registerMissionCB->cbBase.cbInfo.env, scope);
    delete registerMissionCB;
    registerMissionCB = nullptr;
    delete work;
    HILOG_INFO("UvWorkNotifySnapshot, uv_queue_work end");
}

void CallbackReturn(napi_value *result, RegisterMissionCB *registerMissionCB)
{
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(registerMissionCB->cbBase.cbInfo.env, &undefined);
    napi_value callResult = nullptr;
    napi_get_reference_value(
        registerMissionCB->cbBase.cbInfo.env, registerMissionCB->cbBase.cbInfo.callback, &callback);

    napi_call_function(registerMissionCB->cbBase.cbInfo.env, undefined, callback, ARGS_TWO, &result[0], &callResult);
}

void NAPIRemoteMissionListener::NotifySnapshot(const std::string &deviceId, int32_t missionId)
{
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("%{public}s, loop == nullptr.", __func__);
        return;
    }

    uv_work_t *work = new uv_work_t;

    auto registerMissionCB = new (std::nothrow) RegisterMissionCB;
    if (registerMissionCB == nullptr) {
        HILOG_ERROR("%{public}s, registerMissionCB == nullptr.", __func__);
        delete work;
        return;
    }
    registerMissionCB->cbBase.cbInfo.env = env_;
    registerMissionCB->cbBase.cbInfo.callback = notifySnapshotRef_;
    registerMissionCB->deviceId = deviceId;
    registerMissionCB->missionId = missionId;
    work->data = static_cast<void *>(registerMissionCB);

    int rev = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, UvWorkNotifySnapshot);
    if (rev != 0) {
        delete registerMissionCB;
        registerMissionCB = nullptr;
        delete work;
    }
    HILOG_INFO("%{public}s, NotifySnapshot end.", __func__);
}

void UvWorkNotifyNetDisconnect(uv_work_t *work, int status)
{
    HILOG_INFO("UvWorkNotifyNetDisconnect begin, uv_queue_work");
    if (work == nullptr) {
        HILOG_ERROR("UvWorkNotifyNetDisconnect, work is null.");
        return;
    }
    RegisterMissionCB *registerMissionCB = static_cast<RegisterMissionCB *>(work->data);
    if (registerMissionCB == nullptr) {
        HILOG_ERROR("UvWorkNotifyNetDisconnect, registerMissionCB is null");
        delete work;
        return;
    }
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(registerMissionCB->cbBase.cbInfo.env, &scope);
    if (scope == nullptr) {
        delete registerMissionCB;
        registerMissionCB = nullptr;
        delete work;
        return;
    }

    napi_value result[2] = {nullptr};
    result[0] =
        WrapString(registerMissionCB->cbBase.cbInfo.env, registerMissionCB->deviceId.c_str(), "deviceId");
    HILOG_INFO("UvWorkNotifyNetDisconnect, state = %{public}d", registerMissionCB->state);
    result[1] =
        CreateInt32(registerMissionCB->cbBase.cbInfo.env, registerMissionCB->state, "state");

    CallbackReturn(&result[0], registerMissionCB);

    napi_close_handle_scope(registerMissionCB->cbBase.cbInfo.env, scope);
    delete registerMissionCB;
    registerMissionCB = nullptr;
    delete work;
    HILOG_INFO("UvWorkNotifyNetDisconnect, uv_queue_work end");
}

void NAPIRemoteMissionListener::NotifyNetDisconnect(const std::string &deviceId, int32_t state)
{
    HILOG_INFO("%{public}s called. state = %{public}d", __func__, state);
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("%{public}s, loop == nullptr.", __func__);
        return;
    }

    uv_work_t *work = new uv_work_t;

    auto registerMissionCB = new (std::nothrow) RegisterMissionCB;
    if (registerMissionCB == nullptr) {
        HILOG_ERROR("%{public}s, registerMissionCB == nullptr.", __func__);
        delete work;
        return;
    }
    registerMissionCB->cbBase.cbInfo.env = env_;
    registerMissionCB->cbBase.cbInfo.callback = notifyNetDisconnectRef_;
    registerMissionCB->deviceId = deviceId;
    registerMissionCB->state = state;
    work->data = static_cast<void *>(registerMissionCB);

    int rev = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, UvWorkNotifyNetDisconnect);
    if (rev != 0) {
        delete registerMissionCB;
        registerMissionCB = nullptr;
        delete work;
    }
    HILOG_INFO("%{public}s, end.", __func__);
}

void UnRegisterMissionExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto registerMissionCB = (RegisterMissionCB*)data;

    std::lock_guard<std::mutex> autoLock(registrationLock_);
    sptr<NAPIRemoteMissionListener> registration;
    auto item = registration_.find(registerMissionCB->deviceId);
    if (item != registration_.end()) {
        HILOG_INFO("registration exits.");
        registration = registration_[registerMissionCB->deviceId];
    } else {
        HILOG_INFO("registration not exits.");
        registerMissionCB->result = INVALID_PARAMETERS_ERR;
        return;
    }
    registerMissionCB->missionRegistration = registration;

    registerMissionCB->result =
        AbilityManagerClient::GetInstance()->
        UnRegisterMissionListener(registerMissionCB->deviceId,
        registerMissionCB->missionRegistration);
    if (registerMissionCB->result == NO_ERROR) {
        HILOG_INFO("remove registration.");
        registration_.erase(registerMissionCB->deviceId);
    }
    HILOG_DEBUG("%{public}s end.deviceId:%{public}d ", __func__, registerMissionCB->result);
}

void UnRegisterMissionPromiseCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto registerMissionCB = (RegisterMissionCB*)data;
    // set result
    napi_value result[2] = { nullptr };
    napi_get_undefined(env, &result[1]);
    if (registerMissionCB->result == 0) {
        napi_get_undefined(env, &result[0]);
    } else {
        int32_t errCode = ErrorCodeReturn(registerMissionCB->result);
        result[0] = GenerateBusinessError(env, errCode, ErrorMessageReturn(errCode));
    }

    ReturnValueToApplication(env, &result[0], registerMissionCB);
    delete registerMissionCB;
    registerMissionCB = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
}

napi_value UnRegisterMissionPromise(napi_env env, RegisterMissionCB *registerMissionCB)
{
    HILOG_INFO("%{public}s asyncCallback.", __func__);
    if (registerMissionCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value promise = nullptr;
    if (registerMissionCB->callbackRef == nullptr) {
        napi_create_promise(env, &registerMissionCB->cbBase.deferred, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env,
        nullptr,
        resourceName,
        UnRegisterMissionExecuteCB,
        UnRegisterMissionPromiseCompletedCB,
        static_cast<void *>(registerMissionCB),
        &registerMissionCB->cbBase.asyncWork);
    napi_queue_async_work(env, registerMissionCB->cbBase.asyncWork);
    HILOG_INFO("%{public}s asyncCallback end.", __func__);
    return promise;
}

bool GetUnRegisterMissionDeviceId(napi_env &env, const napi_value &value,
    RegisterMissionCB *registerMissionCB, std::string &errInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value napiDeviceId = nullptr;
    napi_valuetype valueType = napi_undefined;
    bool isDeviceId = false;
    napi_has_named_property(env, value, "deviceId", &isDeviceId);
    napi_typeof(env, value, &valueType);
    if (isDeviceId && valueType == napi_object) {
        napi_get_named_property(env, value, "deviceId", &napiDeviceId);
    } else {
        HILOG_ERROR("%{public}s, Wrong argument name for deviceId.", __func__);
        errInfo = "Parameter error. The key of \"MissionDeviceInfo\" must be deviceId";
        return false;
    }
    if (napiDeviceId == nullptr) {
        HILOG_ERROR("%{public}s, not find deviceId.", __func__);
        errInfo = "Parameter error. The value of \"deviceId\" must not be undefined";
        return false;
    }

    size_t valueLen = 0;
    napi_typeof(env, napiDeviceId, &valueType);
    if (valueType != napi_string) {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        errInfo = "Parameter error. The type of \"deviceId\" must be string";
        return false;
    }
    char deviceId[VALUE_BUFFER_SIZE + 1] = {0};
    napi_get_value_string_utf8(env, napiDeviceId, deviceId, VALUE_BUFFER_SIZE + 1, &valueLen);
    if (valueLen > VALUE_BUFFER_SIZE) {
        HILOG_ERROR("%{public}s, deviceId length not correct", __func__);
        errInfo = "Parameter error. The length of \"deviceId\" must be less than " +
            std::to_string(VALUE_BUFFER_SIZE);
        return false;
    }
    registerMissionCB->deviceId = std::string(deviceId);
    HILOG_INFO("%{public}s called end.", __func__);
    return true;
}

napi_value UnRegisterMissionWrap(napi_env &env, napi_callback_info info,
    RegisterMissionCB *registerMissionCB, std::string &errInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argc = 2;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    HILOG_INFO("argc is %{public}zu", argc);
    if (argc != ARGS_ONE && argc != ARGS_TWO) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        errInfo = "Parameter error. The type of \"number of parameters\" must be 1 or 2";
        return nullptr;
    }

    if (!GetUnRegisterMissionDeviceId(env, args[0], registerMissionCB, errInfo)) {
        HILOG_ERROR("%{public}s, Wrong argument.", __func__);
        return nullptr;
    }

    if (argc == ARGS_TWO) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, args[1], &valueType);
        if (valueType != napi_function) {
            HILOG_ERROR("%{public}s, callback error type.", __func__);
            errInfo = "Parameter error. The type of \"callback\" must be AsynCallback<void>: void";
            return nullptr;
        }
        napi_create_reference(env, args[1], 1, &registerMissionCB->callbackRef);
    }
    ret = UnRegisterMissionPromise(env, registerMissionCB);
    HILOG_INFO("%{public}s called end.", __func__);
    return ret;
}

napi_value NAPI_UnRegisterMissionListener(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::string errInfo = "Parameter error";
    RegisterMissionCB *registerMissionCB = CreateRegisterMissionCBCBInfo(env);
    if (registerMissionCB == nullptr) {
        HILOG_ERROR("%{public}s registerMissionCB == nullptr", __func__);
        napi_throw(env, GenerateBusinessError(env, SYSTEM_WORK_ABNORMALLY, ErrorMessageReturn(SYSTEM_WORK_ABNORMALLY)));
        return GetUndefined(env);
    }

    napi_value ret = UnRegisterMissionWrap(env, info, registerMissionCB, errInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        delete registerMissionCB;
        registerMissionCB = nullptr;
        napi_throw(env, GenerateBusinessError(env, PARAMETER_CHECK_FAILED, errInfo));
        return GetUndefined(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

napi_value WrapString(napi_env &env, const std::string &param, const std::string &paramName)
{
    HILOG_INFO("%{public}s called.", __func__);

    napi_value jsValue = nullptr;
    HILOG_DEBUG("%{public}s called. %{public}s = %{public}s", __func__, paramName.c_str(), param.c_str());
    napi_create_string_utf8(env, param.c_str(), NAPI_AUTO_LENGTH, &jsValue);

    return jsValue;
}

napi_value WrapInt32(napi_env &env, int32_t num, const std::string &paramName)
{
    HILOG_INFO("%{public}s called.", __func__);

    napi_value jsObject = nullptr;
    napi_create_object(env, &jsObject);

    napi_value jsValue = nullptr;
    HILOG_DEBUG("%{public}s called. %{public}s = %{public}d", __func__, paramName.c_str(), num);
    napi_create_int32(env, num, &jsValue);
    napi_set_named_property(env, jsObject, paramName.c_str(), jsValue);

    return jsObject;
}

napi_value CreateInt32(napi_env &env, int32_t num, const std::string &paramName)
{
    HILOG_DEBUG("%{public}s called. %{public}s = %{public}d", __func__, paramName.c_str(), num);

    napi_value jsValue = nullptr;
    napi_create_int32(env, num, &jsValue);

    return jsValue;
}

ContinueAbilityCB *CreateContinueAbilityCBCBInfo(napi_env &env)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto continueAbilityCB = new (std::nothrow) ContinueAbilityCB;
    if (continueAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s continueAbilityCB == nullptr", __func__);
        return nullptr;
    }
    continueAbilityCB->cbBase.cbInfo.env = env;
    continueAbilityCB->cbBase.asyncWork = nullptr;
    continueAbilityCB->cbBase.deferred = nullptr;
    continueAbilityCB->callbackRef = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
    return continueAbilityCB;
}

void ContinueAbilityExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto continueAbilityCB = static_cast<ContinueAbilityCB *>(data);
    HILOG_INFO("create continueAbilityCB success.");
    sptr<NAPIMissionContinue> continuation(new (std::nothrow) NAPIMissionContinue());
    continueAbilityCB->abilityContinuation = continuation;
    if (continueAbilityCB->abilityContinuation == nullptr) {
        HILOG_ERROR("%{public}s abilityContinuation == nullptr.", __func__);
        return;
    }
    continueAbilityCB->abilityContinuation->SetContinueAbilityEnv(env);
    HILOG_INFO("set env success.");
    if (continueAbilityCB->abilityContinuationCB.callback[0] != nullptr) {
        continueAbilityCB->abilityContinuation->
            SetContinueAbilityCBRef(continueAbilityCB->abilityContinuationCB.callback[0]);
        HILOG_INFO("set callback success.");
    } else {
        continueAbilityCB->abilityContinuation->
            SetContinueAbilityPromiseRef(continueAbilityCB->cbBase.deferred);
        HILOG_INFO("set promise success.");
    }

    continueAbilityCB->result = -1;
    continueAbilityCB->abilityContinuation->SetContinueAbilityHasBundleName(continueAbilityCB->hasArgsWithBundleName);
    if (continueAbilityCB->hasArgsWithBundleName) {
        continueAbilityCB->result = AAFwk::AbilityManagerClient::GetInstance()->
        ContinueMission(continueAbilityCB->srcDeviceId, continueAbilityCB->dstDeviceId,
        continueAbilityCB->bundleName, continueAbilityCB->abilityContinuation,
        continueAbilityCB->wantParams);
    } else {
        continueAbilityCB->result = AAFwk::AbilityManagerClient::GetInstance()->
        ContinueMission(continueAbilityCB->srcDeviceId, continueAbilityCB->dstDeviceId,
        continueAbilityCB->missionId, continueAbilityCB->abilityContinuation,
        continueAbilityCB->wantParams);
    }
    HILOG_INFO("%{public}s end. error:%{public}d ", __func__, continueAbilityCB->result);
}

void ContinueAbilityCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto continueAbilityCB = static_cast<ContinueAbilityCB *>(data);
    // set result
    napi_value result[2] = { nullptr };
    napi_get_undefined(env, &result[1]);
    if (continueAbilityCB->result == 0) {
        napi_get_undefined(env, &result[0]);
    } else {
        int32_t errCode = ErrorCodeReturn(continueAbilityCB->result);
        result[0] = GenerateBusinessError(env, errCode, ErrorMessageReturn(errCode));
    }
    if (!continueAbilityCB->hasArgsWithBundleName) {
        if (continueAbilityCB->callbackRef == nullptr) { // promise
            if (continueAbilityCB->result == 0) {
                napi_resolve_deferred(env, continueAbilityCB->cbBase.deferred, result[1]);
            } else {
                napi_reject_deferred(env, continueAbilityCB->cbBase.deferred, result[0]);
            }
        } else { // AsyncCallback
            napi_value callback = nullptr;
            napi_get_reference_value(env, continueAbilityCB->callbackRef, &callback);
            napi_value callResult;
            napi_call_function(env, nullptr, callback, ARGS_TWO, &result[0], &callResult);
            napi_delete_reference(env, continueAbilityCB->callbackRef);
        }
    } else {
        if (continueAbilityCB->callbackRef == nullptr && continueAbilityCB->result != 0) { // promise
            napi_reject_deferred(env, continueAbilityCB->cbBase.deferred, result[0]);
        } else if (continueAbilityCB->callbackRef != nullptr && continueAbilityCB->result != 0) { // AsyncCallback
            napi_value callback = nullptr;
            napi_get_reference_value(env, continueAbilityCB->callbackRef, &callback);
            napi_value callResult;
            napi_call_function(env, nullptr, callback, ARGS_TWO, &result[0], &callResult);
            napi_delete_reference(env, continueAbilityCB->callbackRef);
        }
    }
    napi_delete_async_work(env, continueAbilityCB->cbBase.asyncWork);
    delete continueAbilityCB;
    continueAbilityCB = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
}

napi_value ContinueAbilityAsync(napi_env env, ContinueAbilityCB *continueAbilityCB)
{
    HILOG_INFO("%{public}s asyncCallback.", __func__);
    if (continueAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }

    napi_value result = nullptr;
    if (continueAbilityCB->callbackRef == nullptr) {
        napi_create_promise(env, &continueAbilityCB->cbBase.deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "ContinueAbilityAsyncForLauncher", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env,
        nullptr,
        resourceName,
        ContinueAbilityExecuteCB,
        ContinueAbilityCallbackCompletedCB,
        static_cast<void *>(continueAbilityCB),
        &continueAbilityCB->cbBase.asyncWork);
    napi_queue_async_work_with_qos(env, continueAbilityCB->cbBase.asyncWork, napi_qos_user_initiated);
    HILOG_INFO("%{public}s asyncCallback end.", __func__);
    return result;
}

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
        HILOG_ERROR("%{public}s, Wrong argument key.", __func__);
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

bool CheckContinueDeviceInfoSrcDeviceId(napi_env &env, napi_value &napiSrcDeviceId,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiSrcDeviceId, &valueType);
    if (valueType != napi_string) {
        HILOG_ERROR("%{public}s, Wrong argument type srcDeviceId.", __func__);
        errInfo = "Parameter error. The type of \"srcDeviceId\" must be string";
        return false;
    }
    continueAbilityCB->srcDeviceId = AppExecFwk::UnwrapStringFromJS(env, napiSrcDeviceId, "");
    return true;
}

bool CheckContinueDeviceInfoDstDeviceId(napi_env &env, napi_value &napiDstDeviceId,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiDstDeviceId, &valueType);
    if (valueType != napi_string) {
        HILOG_ERROR("%{public}s, Wrong argument type dstDeviceId.", __func__);
        errInfo = "Parameter error. The type of \"dstDeviceId\" must be string";
        return false;
    }
    continueAbilityCB->dstDeviceId = AppExecFwk::UnwrapStringFromJS(env, napiDstDeviceId, "");
    return true;
}

bool CheckContinueDeviceInfoMissionId(napi_env &env, napi_value &napiMissionId,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiMissionId, &valueType);
    if (valueType != napi_number) {
        HILOG_ERROR("%{public}s, Wrong argument type missionId.", __func__);
        errInfo = "Parameter error. The type of \"missionId\" must be number";
        return false;
    }
    continueAbilityCB->missionId = AppExecFwk::UnwrapInt32FromJS(env, napiMissionId, -1);
    return true;
}

bool CheckContinueDeviceInfoBundleName(napi_env &env, napi_value &napiBundleName,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiBundleName, &valueType);
    if (valueType != napi_string) {
        HILOG_ERROR("%{public}s, Wrong argument type missionId.", __func__);
        errInfo = "Parameter error. The type of \"bundleName\" must be string";
        return false;
    }
    continueAbilityCB->bundleName = AppExecFwk::UnwrapStringFromJS(env, napiBundleName, "");
    return true;
}

bool CheckContinueDeviceInfoWantParam(napi_env &env, napi_value &napiWantParam,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiWantParam, &valueType);
    if (valueType != napi_object) {
        HILOG_ERROR("%{public}s, Wrong argument type wantParam.", __func__);
        errInfo = "Parameter error. The type of \"wantParams\" must be object";
        return false;
    }
    if (!AppExecFwk::UnwrapWantParams(env, napiWantParam, continueAbilityCB->wantParams)) {
        HILOG_ERROR("%{public}s, Wrong argument type wantParam.", __func__);
        errInfo = "Parameter error. The type of \"wantParams\" must be array";
        return false;
    }
    return true;
}

bool CheckContinueFirstArgs(napi_env &env, const napi_value &value,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (!CheckContinueKeyExist(env, value)) {
        HILOG_ERROR("%{public}s, Wrong argument key.", __func__);
        errInfo = "Parameter error. The type of \"parameter\" must be ContinueMission";
        return false;
    }
    napi_value napiSrcDeviceId = nullptr;
    napi_value napiDstDeviceId = nullptr;
    napi_value napiMissionId = nullptr;
    napi_value napiWantParam = nullptr;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_object) {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        errInfo = "Parameter error. The type of \"parameter\" must be ContinueMission";
        return false;
    }
    napi_get_named_property(env, value, "srcDeviceId", &napiSrcDeviceId);
    napi_get_named_property(env, value, "dstDeviceId", &napiDstDeviceId);
    napi_get_named_property(env, value, "missionId", &napiMissionId);
    napi_get_named_property(env, value, "wantParam", &napiWantParam);
    if (napiSrcDeviceId == nullptr || napiDstDeviceId == nullptr ||
        napiMissionId == nullptr || napiWantParam == nullptr) {
        HILOG_ERROR("%{public}s, miss required parameters.", __func__);
        errInfo = "Parameter error. The number of \"ContinueMission\" must be 4";
        return false;
    }

    if (!CheckContinueDeviceInfoSrcDeviceId(env, napiSrcDeviceId, continueAbilityCB, errInfo) ||
        !CheckContinueDeviceInfoDstDeviceId(env, napiDstDeviceId, continueAbilityCB, errInfo) ||
        !CheckContinueDeviceInfoMissionId(env, napiMissionId, continueAbilityCB, errInfo) ||
        !CheckContinueDeviceInfoWantParam(env, napiWantParam, continueAbilityCB, errInfo)) {
        HILOG_ERROR("%{public}s, continueMission check ContinueDeviceInfo value failed.", __func__);
        return false;
    }

    HILOG_INFO("%{public}s called end.", __func__);
    return true;
}

bool CheckArgsWithBundleName(napi_env &env, const napi_value &value,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (!CheckBundleNameExist(env, value)) {
        HILOG_ERROR("%{public}s, Args without bundleName.", __func__);
        return false;
    }
    napi_value napiSrcDeviceId = nullptr;
    napi_value napiDstDeviceId = nullptr;
    napi_value napiBundleName = nullptr;
    napi_value napiWantParam = nullptr;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_object) {
        HILOG_ERROR("%{public}s, Args without bundleName.", __func__);
        return false;
    }
    napi_get_named_property(env, value, "srcDeviceId", &napiSrcDeviceId);
    napi_get_named_property(env, value, "dstDeviceId", &napiDstDeviceId);
    napi_get_named_property(env, value, "bundleName", &napiBundleName);
    napi_get_named_property(env, value, "wantParam", &napiWantParam);
    if (napiSrcDeviceId == nullptr || napiDstDeviceId == nullptr ||
        napiBundleName == nullptr || napiWantParam == nullptr) {
        HILOG_ERROR("%{public}s, miss required parameters.", __func__);
        return false;
    }

    if (!CheckContinueDeviceInfoSrcDeviceId(env, napiSrcDeviceId, continueAbilityCB, errInfo) ||
        !CheckContinueDeviceInfoDstDeviceId(env, napiDstDeviceId, continueAbilityCB, errInfo) ||
        !CheckContinueDeviceInfoBundleName(env, napiBundleName, continueAbilityCB, errInfo) ||
        !CheckContinueDeviceInfoWantParam(env, napiWantParam, continueAbilityCB, errInfo)) {
        HILOG_ERROR("%{public}s, continueMission check ContinueDeviceInfo value failed.", __func__);
        return false;
    }
    HILOG_INFO("%{public}s called end.", __func__);
    return true;
}

bool CheckContinueCallback(napi_env &env, const napi_value &value,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value jsMethod = nullptr;
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, value, &valuetype);
    if (valuetype != napi_object) {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        errInfo = "Parameter error. The type of \"options\" must be ContinueCallback";
        return false;
    }
    bool isFirstCallback = false;
    napi_has_named_property(env, value, "onContinueDone", &isFirstCallback);
    if (!isFirstCallback) {
        HILOG_ERROR("%{public}s, Wrong argument name for onContinueDone.", __func__);
        errInfo = "Parameter error. The key of \"ContinueCallback\" must be onContinueDone";
        return false;
    }
    napi_get_named_property(env, value, "onContinueDone", &jsMethod);
    if (jsMethod == nullptr) {
        HILOG_ERROR("%{public}s, not find callback onContinueDone.", __func__);
        errInfo = "Parameter error. The value of \"onContinueDone\" must not be undefined";
        return false;
    }
    napi_typeof(env, jsMethod, &valuetype);
    if (valuetype != napi_function) {
        HILOG_ERROR("%{public}s, onContinueDone callback error type.", __func__);
        errInfo = "Parameter error. The type of \"onContinueDone\" must be function";
        return false;
    }
    napi_create_reference(env, jsMethod, 1, &continueAbilityCB->abilityContinuationCB.callback[0]);
    HILOG_INFO("%{public}s called end.", __func__);
    return true;
}

bool CheckContinueCallbackWithBundleName(napi_env &env, const napi_value &value,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_function) {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return false;
    }
    napi_create_reference(env, value, 1, &continueAbilityCB->abilityContinuationCB.callback[0]);
    napi_create_reference(env, value, 1, &continueAbilityCB->callbackRef);
    HILOG_INFO("%{public}s called end.", __func__);
    return true;
}

napi_value ContinueAbilityWrap(napi_env &env, napi_callback_info info,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argcAsync = 3;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr);
    HILOG_INFO("argcAsync is %{public}zu", argcAsync);

    if (argcAsync != ARGS_ONE && argcAsync != ARGS_TWO && argcAsync != ARGS_THREE) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        errInfo = "Parameter error. The type of \"number of parameters\" must be 1 or 2 or 3";
        return nullptr;
    }

    if (CheckArgsWithBundleName(env, args[0], continueAbilityCB, errInfo)) {
        continueAbilityCB->hasArgsWithBundleName = true;
        if (argcAsync == ARGS_TWO && CheckContinueCallbackWithBundleName(env, args[1], continueAbilityCB, errInfo)) {
            ret = ContinueAbilityAsync(env, continueAbilityCB);
            HILOG_INFO("%{public}s called end.", __func__);
            return ret;
        }
    }

    if (!continueAbilityCB->hasArgsWithBundleName) {
        if (!CheckContinueFirstArgs(env, args[0], continueAbilityCB, errInfo)) {
            HILOG_ERROR("%{public}s, check the first argument failed.", __func__);
            return nullptr;
        }

        if (argcAsync > 1) {
            if (!CheckContinueCallback(env, args[1], continueAbilityCB, errInfo)) {
                HILOG_ERROR("%{public}s, check callback failed.", __func__);
                return nullptr;
            }
        }

        if (argcAsync == ARGS_THREE) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, args[ARGS_TWO], &valueType);
            if (valueType != napi_function) {
                HILOG_ERROR("%{public}s, callback error type.", __func__);
                errInfo = "Parameter error. The type of \"callback\" must be AsynCallback<void>: void";
                return nullptr;
            }
            napi_create_reference(env, args[ARGS_TWO], 1, &continueAbilityCB->callbackRef);
        }
    }

    ret = ContinueAbilityAsync(env, continueAbilityCB);
    HILOG_INFO("%{public}s called end.", __func__);
    return ret;
}

napi_value NAPI_ContinueAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::string errInfo = "Parameter error";
    ContinueAbilityCB *continueAbilityCB = CreateContinueAbilityCBCBInfo(env);
    if (continueAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s continueAbilityCB == nullptr", __func__);
        napi_throw(env, GenerateBusinessError(env, SYSTEM_WORK_ABNORMALLY, ErrorMessageReturn(SYSTEM_WORK_ABNORMALLY)));
        return GetUndefined(env);
    }

    napi_value ret = ContinueAbilityWrap(env, info, continueAbilityCB, errInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        delete continueAbilityCB;
        continueAbilityCB = nullptr;
        napi_throw(env, GenerateBusinessError(env, PARAMETER_CHECK_FAILED, errInfo));
        return GetUndefined(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

ContinueAbilityCB *CheckAndGetParameters(uv_work_t *work, napi_handle_scope *scope)
{
    HILOG_INFO("GetParam, start");
    if (work == nullptr) {
        HILOG_ERROR("GetParam, work is null");
        return nullptr;
    }
    ContinueAbilityCB *continueAbilityCB = static_cast<ContinueAbilityCB *>(work->data);
    if (continueAbilityCB == nullptr) {
        HILOG_ERROR("GetParam, continueAbilityCB is null");
        delete work;
        return nullptr;
    }
    napi_open_handle_scope(continueAbilityCB->cbBase.cbInfo.env, scope);
    if (scope == nullptr) {
        delete continueAbilityCB;
        continueAbilityCB = nullptr;
        delete work;
        return nullptr;
    }
    return continueAbilityCB;
}

void UvWorkOnContinueDone(uv_work_t *work, int status)
{
    HILOG_INFO("UvWorkOnContinueDone, uv_queue_work");
    napi_handle_scope scope = nullptr;
    ContinueAbilityCB *continueAbilityCB = CheckAndGetParameters(work, &scope);
    if (continueAbilityCB == nullptr) {
        return;
    }
    HILOG_INFO("UvWorkOnContinueDone, resultCode = %{public}d", continueAbilityCB->resultCode);
    napi_value result = WrapInt32(continueAbilityCB->cbBase.cbInfo.env, continueAbilityCB->resultCode, "resultCode");
    if (continueAbilityCB->hasArgsWithBundleName) {
        result = WrapInt32(continueAbilityCB->cbBase.cbInfo.env, continueAbilityCB->resultCode, "code");
    }
    if (continueAbilityCB->cbBase.deferred == nullptr) {
        napi_value callback = nullptr;
        napi_value undefined = nullptr;
        napi_get_undefined(continueAbilityCB->cbBase.cbInfo.env, &undefined);
        napi_value callResult = nullptr;
        napi_get_reference_value(continueAbilityCB->cbBase.cbInfo.env,
            continueAbilityCB->cbBase.cbInfo.callback, &callback);
        napi_call_function(continueAbilityCB->cbBase.cbInfo.env, undefined, callback, 1, &result, &callResult);
        if (continueAbilityCB->cbBase.cbInfo.callback != nullptr) {
            napi_delete_reference(continueAbilityCB->cbBase.cbInfo.env, continueAbilityCB->cbBase.cbInfo.callback);
        }
    } else {
        napi_value result[2] = { nullptr };
        napi_get_undefined(continueAbilityCB->cbBase.cbInfo.env, &result[1]);
        if (continueAbilityCB->resultCode == 0) {
            napi_resolve_deferred(continueAbilityCB->cbBase.cbInfo.env, continueAbilityCB->cbBase.deferred, result[1]);
        } else {
            result[0] = GenerateBusinessError(continueAbilityCB->cbBase.cbInfo.env,
                continueAbilityCB->resultCode, ErrorMessageReturn(continueAbilityCB->resultCode));
            napi_reject_deferred(continueAbilityCB->cbBase.cbInfo.env, continueAbilityCB->cbBase.deferred, result[0]);
        }
    }
    napi_close_handle_scope(continueAbilityCB->cbBase.cbInfo.env, scope);
    delete continueAbilityCB;
    continueAbilityCB = nullptr;
    delete work;
    HILOG_INFO("UvWorkOnContinueDone, uv_queue_work end");
}

void NAPIMissionContinue::OnContinueDone(int32_t result)
{
    HILOG_INFO("%{public}s, called. result = %{public}d", __func__, result);
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("%{public}s, loop == nullptr.", __func__);
        return;
    }

    uv_work_t *work = new uv_work_t;

    auto continueAbilityCB = new (std::nothrow) ContinueAbilityCB;
    if (continueAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s, continueAbilityCB == nullptr.", __func__);
        delete work;
        return;
    }
    continueAbilityCB->cbBase.cbInfo.env = env_;
    continueAbilityCB->hasArgsWithBundleName = onContinueDoneHasBundleName_;
    if (onContinueDoneRef_ != nullptr) {
        continueAbilityCB->cbBase.cbInfo.callback = onContinueDoneRef_;
    } else {
        continueAbilityCB->cbBase.deferred = promiseDeferred_;
    }
    continueAbilityCB->resultCode = result;
    work->data = static_cast<void *>(continueAbilityCB);

    int rev = uv_queue_work_with_qos(
        loop, work, [](uv_work_t *work) {}, UvWorkOnContinueDone, uv_qos_user_initiated);
    if (rev != 0) {
        delete continueAbilityCB;
        continueAbilityCB = nullptr;
        delete work;
    }
    HILOG_INFO("%{public}s, end.", __func__);
}

void NAPIMissionContinue::SetContinueAbilityEnv(const napi_env &env)
{
    env_ = env;
}

void NAPIMissionContinue::SetContinueAbilityCBRef(const napi_ref &ref)
{
    onContinueDoneRef_ = ref;
}

void NAPIMissionContinue::SetContinueAbilityHasBundleName(bool hasBundleName)
{
    onContinueDoneHasBundleName_ = hasBundleName;
}

void NAPIMissionContinue::SetContinueAbilityPromiseRef(const napi_deferred &promiseDeferred)
{
    promiseDeferred_ = promiseDeferred;
}

napi_value DistributedMissionManagerExport(napi_env env, napi_value exports)
{
    HILOG_INFO("%{public}s,called", __func__);
    napi_value continueState = NAPI_ContinueState(env);
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("startSyncRemoteMissions", NAPI_StartSyncRemoteMissions),
        DECLARE_NAPI_FUNCTION("stopSyncRemoteMissions", NAPI_StopSyncRemoteMissions),
        DECLARE_NAPI_FUNCTION("registerMissionListener", NAPI_RegisterMissionListener),
        DECLARE_NAPI_FUNCTION("unRegisterMissionListener", NAPI_UnRegisterMissionListener),
        DECLARE_NAPI_FUNCTION("continueMission", NAPI_ContinueAbility),
        DECLARE_NAPI_FUNCTION("on", NAPI_On),
        DECLARE_NAPI_FUNCTION("off", NAPI_Off),
        DECLARE_NAPI_PROPERTY("ContinueState", continueState),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties));
    return exports;
}

static napi_module missionModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = DistributedMissionManagerExport,
    .nm_modname = "distributedMissionManager",
    .nm_priv = (static_cast<void*>(nullptr)),
    .reserved = {nullptr}
};

extern "C" __attribute__((constructor)) void AbilityRegister()
{
    napi_module_register(&missionModule);
}
}
}
