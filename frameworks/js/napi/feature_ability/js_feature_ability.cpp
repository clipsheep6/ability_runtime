/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "js_feature_ability.h"

#include "distribute_constants.h"
#include "distribute_req_param.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
const std::string RESULT_DATA_TAG = "resultData";

void JsFeatureAbility::Finalizer(napi_env env, void* data, void* hint)
{
    TAG_LOGI(AAFwkTag::FA, "JsFeatureAbility::Finalizer is called");
    std::unique_ptr<JsFeatureAbility>(static_cast<JsFeatureAbility*>(data));
}

napi_value JsFeatureAbility::StartAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFeatureAbility, OnStartAbility);
}

napi_value JsFeatureAbility::StartAbilityForResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFeatureAbility, OnStartAbilityForResult);
}

napi_value JsFeatureAbility::FinishWithResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFeatureAbility, OnFinishWithResult);
}

napi_value JsFeatureAbility::GetDeviceList(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFeatureAbility, OnGetDeviceList);
}

napi_value JsFeatureAbility::CallAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFeatureAbility, OnCallAbility);
}

napi_value JsFeatureAbility::ContinueAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFeatureAbility, OnContinueAbility);
}

napi_value JsFeatureAbility::SubscribeAbilityEvent(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFeatureAbility, OnSubscribeAbilityEvent);
}

napi_value JsFeatureAbility::UnsubscribeAbilityEvent(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFeatureAbility, OnUnsubscribeAbilityEvent);
}

napi_value JsFeatureAbility::SendMsg(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFeatureAbility, OnSendMsg);
}

napi_value JsFeatureAbility::SubscribeMsg(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFeatureAbility, OnSubscribeMsg);
}

napi_value JsFeatureAbility::UnsubscribeMsg(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFeatureAbility, OnUnsubscribeMsg);
}

napi_value JsFeatureAbility::OnStartAbility(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s is called", __FUNCTION__);
    if (info.argc != 1) {
        TAG_LOGE(AAFwkTag::FA, "arguments not match");
        return CreateJsUndefined(env);
    }

    Ability* ability = GetAbility(env);
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "ability is null");
        return CreateJsUndefined(env);
    }

    DistributeReqParam requestParam;
    if (!UnWrapRequestParams(env, info.argv[0], requestParam)) {
        TAG_LOGE(AAFwkTag::FA, "unwrap request arguments failed");
        return CreateJsUndefined(env);
    }

    Want want = GetWant(requestParam);
    NapiAsyncTask::CompleteCallback complete =
        [want, ability](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto errcode = ability->StartAbility(want);
            task.Resolve(env, JsFeatureAbility::CreateJsResult(env, errcode, "Start Ability failed."));
        };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSFeatureAbility::OnStartAbility",
        env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsFeatureAbility::OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s is called", __FUNCTION__);
    if (info.argc != 1) {
        TAG_LOGE(AAFwkTag::FA, "Params not match");
        return CreateJsUndefined(env);
    }
    Ability* ability = GetAbility(env);
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "ability is nullptr");
        return CreateJsUndefined(env);
    }

    DistributeReqParam requestParam;
    if (!UnWrapRequestParams(env, info.argv[0], requestParam)) {
        TAG_LOGE(AAFwkTag::FA, "unwrap request params failed");
        return CreateJsUndefined(env);
    }

    Want want = GetWant(requestParam);
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> uasyncTask = CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);

    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);
    FeatureAbilityTask task = [env, asyncTask](int resultCode, const AAFwk::Want& want) {
        TAG_LOGI(AAFwkTag::FA, "OnStartAbilityForResult async callback is called");
        std::string data = want.GetStringParam(RESULT_DATA_TAG);
        napi_value abilityResult = JsFeatureAbility::CreateJsResult(env, resultCode, data);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::FA, "wrap abilityResult failed");
            asyncTask->Reject(env, CreateJsError(env, 1, "failed to get result data!"));
        } else {
            asyncTask->Resolve(env, abilityResult);
        }
        TAG_LOGI(AAFwkTag::FA, "OnStartAbilityForResult async callback is called end");
    };

    want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
    requestCode_ = (requestCode_ == INT_MAX) ? 0 : (requestCode_ + 1);
    ability->StartFeatureAbilityForResult(want, requestCode_, std::move(task));

    TAG_LOGI(AAFwkTag::FA, "OnStartAbilityForResult is called end");
    return result;
}

napi_value JsFeatureAbility::OnFinishWithResult(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s is called", __FUNCTION__);
    if (info.argc != 1) {
        TAG_LOGE(AAFwkTag::FA, "Params not match");
        return CreateJsUndefined(env);
    }

    Ability *ability = GetAbility(env);
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "ability is nullptr");
        return CreateJsUndefined(env);
    }

    if (!IsTypeForNapiValue(env, info.argv[0], napi_object)) {
        TAG_LOGE(AAFwkTag::FA, "Params is invalid.");
        return CreateJsUndefined(env);
    }

    int32_t code = ERR_OK;
    if (!UnwrapInt32ByPropertyName(env, info.argv[0], "code", code)) {
        TAG_LOGE(AAFwkTag::FA, "Failed to get code.");
        return CreateJsUndefined(env);
    }

    napi_value jsResultObj = GetPropertyValueByPropertyName(env, info.argv[0], "result", napi_object);
    if (jsResultObj == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "Failed to get result.");
        return CreateJsUndefined(env);
    }

    napi_value globalValue = nullptr;
    napi_get_global(env, &globalValue);
    napi_value jsonValue;
    napi_get_named_property(env, globalValue, "JSON", &jsonValue);
    napi_value stringifyValue = nullptr;
    napi_get_named_property(env, jsonValue, "stringify", &stringifyValue);
    napi_value transValue = nullptr;
    napi_call_function(env, jsonValue, stringifyValue, 1, &jsResultObj, &transValue);
    std::string resultStr {};
    resultStr = UnwrapStringFromJS(env, transValue, "");

    TAG_LOGD(AAFwkTag::FA, "code: %{public}d, result:%{public}s", code, resultStr.c_str());
    Want want;
    want.SetParam(RESULT_DATA_TAG, resultStr);
    ability->SetResult(code, want);

    NapiAsyncTask::CompleteCallback complete =
        [ability](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto errCode = ability->TerminateAbility();
            task.Resolve(env, JsFeatureAbility::CreateJsResult(env, errCode, "FinishWithResult failed."));
        };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSFeatureAbility::OnFinishWithResult",
        env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsFeatureAbility::OnGetDeviceList(napi_env env, const NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s is called", __FUNCTION__);
    return CreateJsUndefined(env);
}

napi_value JsFeatureAbility::OnCallAbility(napi_env env, const NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s is called", __FUNCTION__);
    return CreateJsUndefined(env);
}

napi_value JsFeatureAbility::OnContinueAbility(napi_env env, const NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s is called", __FUNCTION__);
    return CreateJsUndefined(env);
}

napi_value JsFeatureAbility::OnSubscribeAbilityEvent(napi_env env, const NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s is called", __FUNCTION__);
    return CreateJsUndefined(env);
}

napi_value JsFeatureAbility::OnUnsubscribeAbilityEvent(napi_env env, const NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s is called", __FUNCTION__);
    return CreateJsUndefined(env);
}

napi_value JsFeatureAbility::OnSendMsg(napi_env env, const NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s is called", __FUNCTION__);
    return CreateJsUndefined(env);
}

napi_value JsFeatureAbility::OnSubscribeMsg(napi_env env, const NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s is called", __FUNCTION__);
    return CreateJsUndefined(env);
}

napi_value JsFeatureAbility::OnUnsubscribeMsg(napi_env env, const NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s is called", __FUNCTION__);
    return CreateJsUndefined(env);
}

Ability* JsFeatureAbility::GetAbility(napi_env env)
{
    napi_status ret;
    napi_value global = nullptr;
    const napi_extended_error_info *errorInfo = nullptr;
    ret = napi_get_global(env, &global);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        TAG_LOGE(AAFwkTag::FA, "JsFeatureAbility::GetAbility, get_global=%{public}d err:%{public}s",
            ret, errorInfo->error_message);
        return nullptr;
    }

    napi_value abilityObj = nullptr;
    ret = napi_get_named_property(env, global, "ability", &abilityObj);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        TAG_LOGE(AAFwkTag::FA, "JsFeatureAbility::GetAbility, get_named_property=%{public}d err:%{public}s",
            ret, errorInfo->error_message);
        return nullptr;
    }

    Ability* ability = nullptr;
    ret = napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability));
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        TAG_LOGE(AAFwkTag::FA, "JsFeatureAbility::GetAbility, get_value_external=%{public}d err:%{public}s",
            ret, errorInfo->error_message);
    return nullptr;
    }

    return ability;
}

Want JsFeatureAbility::GetWant(DistributeReqParam &requestParam)
{
    Want want;
    Uri parseUri("");
    if (CheckThenGetDeepLinkUri(requestParam, parseUri)) {
        want.SetUri(parseUri);
        want.SetAction(requestParam.GetAction());
        for (auto entity : requestParam.GetEntities()) {
            want.AddEntity(entity);
        }
        if (!requestParam.GetType().empty()) {
            want.SetType(requestParam.GetType());
        }
        return want;
    }

    if (requestParam.GetDeviceType() == DistributeConstants::DEVICE_TYPE_DEFAULT) {
        want.AddFlags(want.FLAG_ABILITYSLICE_MULTI_DEVICE);
        want.AddFlags(want.FLAG_NOT_OHOS_COMPONENT);
        want.SetDeviceId(requestParam.GetNetworkId());
    }

    if (requestParam.GetDeviceType() == DistributeConstants::DEVICE_TYPE_LOCAL) {
        want.AddFlags(want.FLAG_NOT_OHOS_COMPONENT);
    }
    want.AddFlags(requestParam.GetFlag());

    if (!requestParam.GetData().empty()) {
        want.SetParam(DistributeConstants::START_ABILITY_PARAMS_KEY, requestParam.GetData());
    }

    if (!requestParam.GetUrl().empty()) {
        want.SetParam(DistributeConstants::START_ABILITY_URL_KEY, requestParam.GetUrl());
        want.SetUri(Uri(requestParam.GetUrl()));
    }

    if (!requestParam.GetType().empty()) {
        want.SetType(requestParam.GetType());
    }

    GetExtraParams(requestParam, want);

    if (!requestParam.GetBundleName().empty() && !requestParam.GetAbilityName().empty()) {
        want.SetElementName(requestParam.GetNetworkId(), requestParam.GetBundleName(),
            requestParam.GetAbilityName(), requestParam.GetModuleName());
    } else {
        want.SetAction(requestParam.GetAction());
        for (auto entity : requestParam.GetEntities()) {
            want.AddEntity(entity);
        }
    }

    return want;
}

void JsFeatureAbility::GetExtraParams(const DistributeReqParam &requestParam, const Want &want)
{
    return;
}

bool JsFeatureAbility::CheckThenGetDeepLinkUri(const DistributeReqParam &requestParam, Uri &uri)
{
    std::string url = requestParam.GetUrl();
    std::string action = requestParam.GetAction();
    if (url.empty() || action.empty()) {
        return false;
    }

    if (action != DistributeConstants::DEEP_LINK_ACTION_NAME) {
        return false;
    }

    uri = Uri(url);
    if (uri.GetScheme().empty() || uri.GetHost().empty()) {
        return false;
    }

    return true;
}

bool JsFeatureAbility::UnWrapRequestParams(napi_env env, napi_value param, DistributeReqParam &requestParam)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s called.", __func__);

    if (!IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGI(AAFwkTag::FA, "%{public}s called. Params is invalid.", __func__);
        return false;
    }

    std::string bundleName;
    if (UnwrapStringByPropertyName(env, param, "bundleName", bundleName)) {
        requestParam.SetBundleName(bundleName);
    }

    std::string abilityName;
    if (UnwrapStringByPropertyName(env, param, "abilityName", abilityName)) {
        requestParam.SetAbilityName(abilityName);
    }

    std::vector<std::string> entities;
    if (UnwrapStringArrayByPropertyName(env, param, "entities", entities)) {
        requestParam.SetEntities(entities);
    }

    std::string action;
    if (UnwrapStringByPropertyName(env, param, "action", action)) {
        requestParam.SetAction(action);
    }

    std::string networkId;
    if (UnwrapStringByPropertyName(env, param, "networkId", networkId)) {
        requestParam.SetNetWorkId(networkId);
    }

    int32_t deviceType = 0;
    if (UnwrapInt32ByPropertyName(env, param, "deviceType", deviceType)) {
        requestParam.SetDeviceType(deviceType);
    }

    std::string data;
    if (UnwrapStringByPropertyName(env, param, "data", data)) {
        requestParam.SetData(data);
    }

    int32_t flag = 0;
    if (UnwrapInt32ByPropertyName(env, param, "flag", flag)) {
        requestParam.SetFlag(flag);
    }

    std::string url;
    if (UnwrapStringByPropertyName(env, param, "url", url)) {
        requestParam.SetUrl(url);
    }

    return true;
}

napi_value JsFeatureAbility::CreateJsResult(napi_env env, int32_t errCode, const std::string &message)
{
    napi_value jsResult = nullptr;
    napi_create_object(env, &jsResult);
    napi_set_named_property(env, jsResult, "code", CreateJsNumber(env, errCode));
    if (errCode == 0) {
        napi_set_named_property(env, jsResult, "data", CreateJsUndefined(env));
    } else {
        napi_value dataVal = nullptr;
        napi_create_string_utf8(env, message.c_str(), message.length(), &dataVal);
        napi_set_named_property(env, jsResult, "data", dataVal);
    }

    return jsResult;
}

napi_value JsFeatureAbility::CreateJsFeatureAbility(napi_env env)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);

    std::unique_ptr<JsFeatureAbility> jsFeatureAbility = std::make_unique<JsFeatureAbility>();
    napi_wrap(env, object, jsFeatureAbility.release(), JsFeatureAbility::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsFeatureAbility";
    BindNativeFunction(env, object, "startAbility", moduleName, JsFeatureAbility::StartAbility);
    BindNativeFunction(env, object, "startAbilityForResult", moduleName, JsFeatureAbility::StartAbilityForResult);
    BindNativeFunction(env, object, "finishWithResult", moduleName, JsFeatureAbility::FinishWithResult);
    BindNativeFunction(env, object, "getDeviceList", moduleName, JsFeatureAbility::GetDeviceList);
    BindNativeFunction(env, object, "callAbility", moduleName, JsFeatureAbility::CallAbility);
    BindNativeFunction(env, object, "continueAbility", moduleName, JsFeatureAbility::ContinueAbility);
    BindNativeFunction(env, object, "subscribeAbilityEvent", moduleName, JsFeatureAbility::SubscribeAbilityEvent);
    BindNativeFunction(env, object, "unsubscribeAbilityEvent", moduleName,
        JsFeatureAbility::UnsubscribeAbilityEvent);
    BindNativeFunction(env, object, "sendMsg", moduleName, JsFeatureAbility::SendMsg);
    BindNativeFunction(env, object, "subscribeMsg", moduleName, JsFeatureAbility::SubscribeMsg);
    BindNativeFunction(env, object, "unsubscribeMsg", moduleName, JsFeatureAbility::UnsubscribeMsg);

    return object;
}

napi_value JsFeatureAbilityInit(napi_env env, napi_value exports)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "env nullptr.");
        return nullptr;
    }

    napi_value global = nullptr;
    napi_get_global(env, &global);
    if (!CheckTypeForNapiValue(env, global, napi_object)) {
        TAG_LOGE(AAFwkTag::FA, "global is not NativeObject");
        return nullptr;
    }

    napi_set_named_property(env, global, "FeatureAbility", JsFeatureAbility::CreateJsFeatureAbility(env));

    return global;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
