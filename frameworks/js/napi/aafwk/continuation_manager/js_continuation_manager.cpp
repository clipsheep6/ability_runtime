/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "js_continuation_manager.h"

#include <memory>

#include "device_connect_status.h"
#include "distributed_client.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
namespace {
    constexpr int32_t ERR_NOT_OK = -1;
    constexpr int32_t ARG_COUNT_ONE = 1;
    constexpr int32_t ARG_COUNT_TWO = 2;
    constexpr int32_t ARG_COUNT_THREE = 3;
}

void JsContinuationManager::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_INFO("JsContinuationManager::Finalizer is called");
    std::unique_ptr<JsContinuationManager>(static_cast<JsContinuationManager*>(data));
}

NativeValue* JsContinuationManager::Register(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsContinuationManager* me = CheckParamsAndGetThis<JsContinuationManager>(engine, info);
    return (me != nullptr) ? me->OnRegister(*engine, *info) : nullptr;
}

NativeValue* JsContinuationManager::Unregister(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsContinuationManager* me = CheckParamsAndGetThis<JsContinuationManager>(engine, info);
    return (me != nullptr) ? me->OnUnregister(*engine, *info) : nullptr;
}

NativeValue* JsContinuationManager::RegisterDeviceSelectionCallback(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsContinuationManager* me = CheckParamsAndGetThis<JsContinuationManager>(engine, info);
    return (me != nullptr) ? me->OnRegisterDeviceSelectionCallback(*engine, *info) : nullptr;
}

NativeValue* JsContinuationManager::UnregisterDeviceSelectionCallback(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsContinuationManager* me = CheckParamsAndGetThis<JsContinuationManager>(engine, info);
    return (me != nullptr) ? me->OnUnregisterDeviceSelectionCallback(*engine, *info) : nullptr;
}

NativeValue *JsContinuationManager::UpdateConnectStatus(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsContinuationManager *me = CheckParamsAndGetThis<JsContinuationManager>(engine, info);
    return (me != nullptr) ? me->OnUpdateConnectStatus(*engine, *info) : nullptr;
}

NativeValue *JsContinuationManager::StartDeviceManager(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsContinuationManager *me = CheckParamsAndGetThis<JsContinuationManager>(engine, info);
    return (me != nullptr) ? me->OnStartDeviceManager(*engine, *info) : nullptr;
}

NativeValue *JsContinuationManager::InitDeviceConnectStateObject(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsContinuationManager *me = CheckParamsAndGetThis<JsContinuationManager>(engine, info);
    return (me != nullptr) ? me->OnInitDeviceConnectStateObject(*engine, *info) : nullptr;
}

NativeValue *JsContinuationManager::InitContinuationModeObject(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsContinuationManager *me = CheckParamsAndGetThis<JsContinuationManager>(engine, info);
    return (me != nullptr) ? me->OnInitContinuationModeObject(*engine, *info) : nullptr;
}

NativeValue* JsContinuationManager::OnRegister(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("%{public}s is called", __FUNCTION__);
    int32_t errCode = 0;
    decltype(info.argc) unwrapArgc = 0;
    ContinuationExtraParams continuationExtraParams;
    if (info.argc > 0 && info.argv[0]->TypeOf() == NATIVE_OBJECT) {
        HILOG_INFO("Register options is used.");
        if (!UnWrapContinuationExtraParams(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[0]), continuationExtraParams)) {
            HILOG_ERROR("Parse continuationExtraParams failed");
            errCode = ERR_NOT_OK;
        }
        unwrapArgc++;
    }
    AsyncTask::CompleteCallback complete =
        [continuationExtraParams, unwrapArgc, errCode](NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (errCode != 0) {
            task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
            return;
        }
        int32_t token = -1;
        DistributedClient dmsClient;
        int32_t ret = (unwrapArgc == 0) ? dmsClient.Register(token) :
            dmsClient.Register(continuationExtraParams, token);
        if (ret == 0) {
            task.Resolve(engine, engine.CreateNumber(token));
        } else {
            task.Reject(engine, CreateJsError(engine, ret, "Register failed."));
        }
    };

    NativeValue* lastParam = (info.argc <= unwrapArgc) ? nullptr : info.argv[unwrapArgc];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsContinuationManager::OnUnregister(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("%{public}s is called", __FUNCTION__);
    int32_t errCode = 0;
    if (info.argc == 0) {
        HILOG_ERROR("Params not match");
        errCode = ERR_NOT_OK;
    }
    int32_t token = -1;
    if (!errCode && !ConvertFromJsValue(engine, info.argv[0], token)) {
        HILOG_ERROR("Parse token failed");
        errCode = ERR_NOT_OK;
    }
    AsyncTask::CompleteCallback complete =
        [token, errCode](NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (errCode != 0) {
            task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
            return;
        }
        DistributedClient dmsClient;
        int32_t ret = dmsClient.Unregister(token);
        if (ret == 0) {
            task.Resolve(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine, CreateJsError(engine, ret, "Unregister failed."));
        }
    };

    NativeValue* lastParam = (info.argc <= ARG_COUNT_ONE) ? nullptr : info.argv[ARG_COUNT_ONE];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsContinuationManager::OnRegisterDeviceSelectionCallback(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("%{public}s is called", __FUNCTION__);
    if (info.argc != ARG_COUNT_THREE) {
        HILOG_ERROR("Params not match");
        return engine.CreateUndefined();
    }
    std::string cbType;
    if (!ConvertFromJsValue(engine, info.argv[0], cbType)) {
        HILOG_ERROR("Parse cbType failed");
        return engine.CreateUndefined();
    }
    if (cbType != EVENT_CONNECT && cbType != EVENT_DISCONNECT) {
        HILOG_ERROR("type: %{public}s not support!", cbType.c_str());
        return engine.CreateUndefined();
    }
    int32_t token = -1;
    if (!ConvertFromJsValue(engine, info.argv[ARG_COUNT_ONE], token)) {
        HILOG_ERROR("Parse token failed");
        return engine.CreateUndefined();
    }
    NativeValue* jsListenerObj = info.argv[ARG_COUNT_TWO];
    if (jsListenerObj == nullptr) {
        HILOG_ERROR("jsListenerObj is nullptr");
        return engine.CreateUndefined();
    }
    if (!jsListenerObj->IsCallable()) {
        HILOG_ERROR("jsListenerObj is not callable");
        return engine.CreateUndefined();
    }
    if (IfCallbackRegistered(token, cbType)) {
        HILOG_ERROR("callback already registered, token: %{public}d, cbType %{public}s", token, cbType.c_str());
        return engine.CreateUndefined();
    }
    std::unique_ptr<NativeReference> callbackRef;
    callbackRef.reset(engine.CreateReference(jsListenerObj, 1));
    sptr<JsDeviceSelectionListener> deviceSelectionListener = new JsDeviceSelectionListener(&engine);
    if (deviceSelectionListener == nullptr) {
        HILOG_ERROR("deviceSelectionListener is nullptr");
        return engine.CreateUndefined();
    }
    DistributedClient dmsClient;
    int32_t ret = dmsClient.RegisterDeviceSelectionCallback(token, cbType, deviceSelectionListener);
    if (ret == ERR_OK) {
        deviceSelectionListener->AddCallback(cbType, jsListenerObj);
        CallbackPair callbackPair = std::make_pair(std::move(callbackRef), deviceSelectionListener);
        std::lock_guard<std::mutex> jsCbMapLock(jsCbMapMutex_);
        {
            jsCbMap_[token][cbType] = std::move(callbackPair); // move assignment
        }
        HILOG_INFO("RegisterDeviceSelectionListener success");
    } else {
        deviceSelectionListener = nullptr;
        HILOG_ERROR("RegisterDeviceSelectionListener failed");
    }
    return engine.CreateUndefined();
}

NativeValue* JsContinuationManager::OnUnregisterDeviceSelectionCallback(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("%{public}s is called", __FUNCTION__);
    if (info.argc != ARG_COUNT_TWO) {
        HILOG_ERROR("Params not match");
        return engine.CreateUndefined();
    }
    std::string cbType;
    if (!ConvertFromJsValue(engine, info.argv[0], cbType)) {
        HILOG_ERROR("Parse cbType failed");
        return engine.CreateUndefined();
    }
    if (cbType != EVENT_CONNECT && cbType != EVENT_DISCONNECT) {
        HILOG_ERROR("type: %{public}s not support!", cbType.c_str());
        return engine.CreateUndefined();
    }
    int32_t token = -1;
    if (!ConvertFromJsValue(engine, info.argv[ARG_COUNT_ONE], token)) {
        HILOG_ERROR("Parse token failed");
        return engine.CreateUndefined();
    }
    std::lock_guard<std::mutex> jsCbMapLock(jsCbMapMutex_);
    {
        if (jsCbMap_.empty() || jsCbMap_.find(token) == jsCbMap_.end()) {
            HILOG_ERROR("token %{public}d not registered!", token);
            return engine.CreateUndefined();
        }
        if (jsCbMap_[token].empty() || jsCbMap_[token].find(cbType) == jsCbMap_[token].end()) {
            HILOG_ERROR("cbType %{public}s not registered!", cbType.c_str());
            return engine.CreateUndefined();
        }
        CallbackPair& callbackPair = jsCbMap_[token][cbType];
        auto& listener = callbackPair.second;
        DistributedClient dmsClient;
        int32_t ret = dmsClient.UnregisterDeviceSelectionCallback(token, cbType);
        if (ret == ERR_OK) {
            listener->RemoveCallback(cbType);
            jsCbMap_[token].erase(cbType);
            if (jsCbMap_[token].empty()) {
                jsCbMap_.erase(token);
            }
            HILOG_INFO("UnregisterDeviceSelectionCallback success");
        } else {
            HILOG_ERROR("UnregisterDeviceSelectionCallback failed");
        }
    }
    return engine.CreateUndefined();
}

NativeValue *JsContinuationManager::OnUpdateConnectStatus(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("%{public}s is called", __FUNCTION__);
    int32_t errCode = 0;
    if (info.argc < ARG_COUNT_THREE) {
        HILOG_ERROR("Params not match");
        errCode = ERR_NOT_OK;
    }
    int32_t token = -1;
    if (!errCode && !ConvertFromJsValue(engine, info.argv[0], token)) {
        HILOG_ERROR("Parse token failed");
        errCode = ERR_NOT_OK;
    }
    std::string deviceId;
    if (!errCode && !ConvertFromJsValue(engine, info.argv[ARG_COUNT_ONE], deviceId)) {
        HILOG_ERROR("Parse deviceId failed");
        errCode = ERR_NOT_OK;
    }
    if (deviceId.empty()) {
        HILOG_ERROR("deviceId is empty");
        errCode = ERR_NOT_OK;
    }
    AAFwk::DeviceConnectStatus deviceConnectStatus = AAFwk::DeviceConnectStatus::IDLE;
    if (!errCode && !ConvertFromJsValue(engine, info.argv[ARG_COUNT_TWO], deviceConnectStatus)) {
        HILOG_ERROR("Parse device connect status failed");
        errCode = ERR_NOT_OK;
    }
    AsyncTask::CompleteCallback complete =
        [token, deviceId, deviceConnectStatus, errCode](NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (errCode != 0) {
            task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
            return;
        }
        DistributedClient dmsClient;
        int32_t ret = dmsClient.UpdateConnectStatus(token, deviceId, deviceConnectStatus);
        if (ret == 0) {
            task.Resolve(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine, CreateJsError(engine, ret, "UpdateConnectStatus failed."));
        }
    };

    NativeValue* lastParam = (info.argc <= ARG_COUNT_THREE) ? nullptr : info.argv[ARG_COUNT_THREE];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsContinuationManager::OnStartDeviceManager(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("%{public}s is called", __FUNCTION__);
    int32_t errCode = 0;
    if (info.argc < ARG_COUNT_ONE) {
        HILOG_ERROR("Params not match");
        errCode = ERR_NOT_OK;
    }
    int32_t token = -1;
    if (!errCode && !ConvertFromJsValue(engine, info.argv[0], token)) {
        HILOG_ERROR("Parse token failed");
        errCode = ERR_NOT_OK;
    }
    decltype(info.argc) unwrapArgc = ARG_COUNT_ONE;
    ContinuationExtraParams continuationExtraParams;
    if (info.argc > ARG_COUNT_ONE && info.argv[ARG_COUNT_ONE]->TypeOf() == NATIVE_OBJECT) {
        HILOG_INFO("Register options is used.");
        if (!UnWrapContinuationExtraParams(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[ARG_COUNT_ONE]), continuationExtraParams)) {
            HILOG_ERROR("Parse continuationExtraParams failed");
            errCode = ERR_NOT_OK;
        }
        unwrapArgc++;
    }
    AsyncTask::CompleteCallback complete =
        [token, continuationExtraParams, unwrapArgc, errCode](NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (errCode != 0) {
            task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
            return;
        }
        DistributedClient dmsClient;
        int32_t ret = (unwrapArgc == ARG_COUNT_ONE) ? dmsClient.StartDeviceManager(token) :
            dmsClient.StartDeviceManager(token, continuationExtraParams);
        if (ret == 0) {
            task.Resolve(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine, CreateJsError(engine, ret, "StartDeviceManager failed."));
        }
    };

    NativeValue* lastParam = (info.argc <= unwrapArgc) ? nullptr : info.argv[unwrapArgc];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsContinuationManager::OnInitDeviceConnectStateObject(NativeEngine &engine, NativeCallbackInfo &info)
{
    napi_value object;
    napi_env env = reinterpret_cast<napi_env>(&engine);
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "IDLE",
        static_cast<int32_t>(AAFwk::DeviceConnectStatus::IDLE)));
    NAPI_CALL(env, SetEnumItem(env, object, "CONNECTING",
        static_cast<int32_t>(AAFwk::DeviceConnectStatus::CONNECTING)));
    NAPI_CALL(env, SetEnumItem(env, object, "CONNECTED",
        static_cast<int32_t>(AAFwk::DeviceConnectStatus::CONNECTED)));
    NAPI_CALL(env, SetEnumItem(env, object, "DISCONNECTING",
        static_cast<int32_t>(AAFwk::DeviceConnectStatus::DISCONNECTING)));

    return reinterpret_cast<NativeValue*>(object);
}

NativeValue *JsContinuationManager::OnInitContinuationModeObject(NativeEngine &engine, NativeCallbackInfo &info)
{
    napi_value object;
    napi_env env = reinterpret_cast<napi_env>(&engine);
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "COLLABORATION_SINGLE",
        static_cast<int32_t>(AAFwk::ContinuationMode::COLLABORATION_SINGLE)));
    NAPI_CALL(env, SetEnumItem(env, object, "COLLABORATION_MULTIPLE",
        static_cast<int32_t>(AAFwk::ContinuationMode::COLLABORATION_MUTIPLE)));

    return reinterpret_cast<NativeValue*>(object);
}

napi_status JsContinuationManager::SetEnumItem(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_status status;
    napi_value itemName;
    napi_value itemValue;

    NAPI_CALL_BASE(env, status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &itemName), status);
    NAPI_CALL_BASE(env, status = napi_create_int32(env, value, &itemValue), status);

    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemName, itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemValue, itemName), status);

    return napi_ok;
}

bool JsContinuationManager::IfCallbackRegistered(int32_t token, const std::string& cbType)
{
    std::lock_guard<std::mutex> jsCbMapLock(jsCbMapMutex_);
    {
        if (jsCbMap_.empty() || jsCbMap_.find(token) == jsCbMap_.end()) {
            HILOG_ERROR("token %{public}d not registered!", token);
            return false;
        }
        if (jsCbMap_[token].empty() || jsCbMap_[token].find(cbType) == jsCbMap_[token].end()) {
            HILOG_ERROR("cbType %{public}s not registered!", cbType.c_str());
            return false;
        }
    }
    return true;
}

bool JsContinuationManager::UnWrapContinuationExtraParams(napi_env env, napi_value options,
    ContinuationExtraParams &continuationExtraParams)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (!IsTypeForNapiValue(env, options, napi_object)) {
        HILOG_ERROR("options is invalid.");
        return false;
    }
    std::vector<std::string> deviceTypeStringList;
    if (UnwrapStringArrayByPropertyName(env, options, "deviceType", deviceTypeStringList)) {
        continuationExtraParams.SetDeviceType(deviceTypeStringList);
    }
    std::string targetBundleString("");
    if (UnwrapStringByPropertyName(env, options, "targetBundle", targetBundleString)) {
        continuationExtraParams.SetTargetBundle(targetBundleString);
    }
    std::string descriptionString("");
    if (UnwrapStringByPropertyName(env, options, "description", descriptionString)) {
        continuationExtraParams.SetDescription(descriptionString);
    }
    nlohmann::json filterJson;
    if (UnwrapJsonByPropertyName(env, options, "filter", filterJson)) {
        std::string filterString = filterJson.dump();
        continuationExtraParams.SetFilter(filterString);
    }
    int32_t continuationMode = 0;
    if (UnwrapInt32ByPropertyName(env, options, "continuationMode", continuationMode)) {
        continuationExtraParams.SetContinuationMode(static_cast<ContinuationMode>(continuationMode));
    }
    nlohmann::json authInfoJson;
    if (UnwrapJsonByPropertyName(env, options, "authInfo", authInfoJson)) {
        std::string authInfoString = authInfoJson.dump();
        continuationExtraParams.SetAuthInfo(authInfoString);
    }
    return true;
}

bool JsContinuationManager::UnwrapJsonByPropertyName(const napi_env& env, const napi_value& param,
    const std::string& fieldStr, nlohmann::json& jsonObj)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        HILOG_ERROR("param is invalid.");
        return false;
    }
    napi_value jsonField = nullptr;
    napi_get_named_property(env, param, fieldStr.c_str(), &jsonField);
    napi_valuetype jsValueType = napi_undefined;
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;
    napi_get_property_names(env, jsonField, &jsProNameList);
    napi_get_array_length(env, jsProNameList, &jsProCount);

    napi_value jsProName = nullptr;
    napi_value jsProValue = nullptr;
    for (uint32_t index = 0; index < jsProCount; index++) {
        napi_get_element(env, jsProNameList, index, &jsProName);
        std::string strProName = UnwrapStringFromJS(env, jsProName);
        napi_get_named_property(env, jsonField, strProName.c_str(), &jsProValue);
        napi_typeof(env, jsProValue, &jsValueType);
        switch (jsValueType) {
            case napi_string: {
                std::string natValue = UnwrapStringFromJS(env, jsProValue);
                HILOG_INFO("Property name=%s, string, value=%s", strProName.c_str(), natValue.c_str());
                jsonObj[strProName] = natValue;
                break;
            }
            case napi_boolean: {
                bool elementValue = false;
                napi_get_value_bool(env, jsProValue, &elementValue);
                HILOG_INFO("Property name=%s, boolean, value=%d.", strProName.c_str(), elementValue);
                jsonObj[strProName] = elementValue;
                break;
            }
            case napi_number: {
                int32_t elementValue = 0;
                if (napi_get_value_int32(env, jsProValue, &elementValue) != napi_ok) {
                    HILOG_ERROR("Property name=%s, Property int32_t parse error", strProName.c_str());
                } else {
                    jsonObj[strProName] = elementValue;
                    HILOG_INFO("Property name=%s, number, value=%d.", strProName.c_str(), elementValue);
                }
                break;
            }
            default: {
                HILOG_ERROR("Property name=%s, value type not support.", strProName.c_str());
                break;
            }
        }
    }
    return true;
}

NativeValue* JsContinuationManagerInit(NativeEngine* engine, NativeValue* exportObj)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (engine == nullptr || exportObj == nullptr) {
        HILOG_ERROR("%{public}s Invalid input parameters", __func__);
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_ERROR("%{public}s convertNativeValueTo result is nullptr.", __func__);
        return nullptr;
    }

    std::unique_ptr<JsContinuationManager> jsContinuationManager = std::make_unique<JsContinuationManager>();
    object->SetNativePointer(jsContinuationManager.release(), JsContinuationManager::Finalizer, nullptr);

    BindNativeFunction(*engine, *object, "register", JsContinuationManager::Register);
    BindNativeFunction(*engine, *object, "unregister", JsContinuationManager::Unregister);
    BindNativeFunction(*engine, *object, "on", JsContinuationManager::RegisterDeviceSelectionCallback);
    BindNativeFunction(*engine, *object, "off", JsContinuationManager::UnregisterDeviceSelectionCallback);
    BindNativeFunction(*engine, *object, "updateConnectStatus", JsContinuationManager::UpdateConnectStatus);
    BindNativeFunction(*engine, *object, "startDeviceManager", JsContinuationManager::StartDeviceManager);
    BindNativeProperty(*object, "DeviceConnectState", JsContinuationManager::InitDeviceConnectStateObject);
    BindNativeProperty(*object, "ContinuationMode", JsContinuationManager::InitContinuationModeObject);

    HILOG_INFO("%{public}s called end.", __func__);
    return engine->CreateUndefined();
}
}  // namespace AbilityRuntime
}  // namespace OHOS