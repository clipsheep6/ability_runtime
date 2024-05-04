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

#include "ability_runtime/cj_ability_context.h"

#include "cj_common_ffi.h"
#include "hilog_wrapper.h"

#include "pixel_map.h"
#include "cj_ability_connect_callback_proxy.h"
#include "cj_ability_context_proxy.h"
#include "cj_error_code.h"
#include "napi_common_start_options.h"
#include "napi_common_util.h"

using namespace OHOS::FFI;

extern CJAbilityCallbacks* g_cjAbilityCallbacks;

namespace OHOS {
namespace AbilityRuntime {

void UnWrapStartOption(CJStartOptions* source, AAFwk::StartOptions& target)
{
    if (source == nullptr) {
        HILOG_ERROR("UnWrapStartOption failed, source is nullptr");
        return;
    }
    target.SetWindowMode(source->windowMode);
    target.SetDisplayID(source->displayId);
}

std::function<void(int32_t, CJAbilityResult*)> WrapCJAbilityResultTask(int64_t lambdaId)
{
    auto cjTask = [lambdaId](int32_t error, CJAbilityResult* abilityResult) {
        if (g_cjAbilityCallbacks == nullptr) {
            HILOG_ERROR("StartAbilityForResult failed, cangjie callbacks are not registered");
            return;
        }
        g_cjAbilityCallbacks->invokeAbilityResultCallback(lambdaId, error, abilityResult);
        HILOG_INFO("WrapCJAbilityResultTask: error is %{public}d", error);
    };
    return cjTask;
}

RuntimeTask WrapRuntimeTask(std::function<void(int32_t, CJAbilityResult*)> cjTask, int32_t error)
{
    RuntimeTask task = [cjTask, error](int32_t resultCode, const AAFwk::Want& want, bool isInner) {
        WantHandle wantHandle = const_cast<AAFwk::Want*>(&want);
        CJAbilityResult abilityResult = { resultCode, wantHandle };
        cjTask(error, &abilityResult);
        HILOG_INFO("WrapRuntimeTask: error is %{public}d", error);
    };
    return task;
}

AbilityContextBroker* CJAbilityContext::GetBrokerPointer()
{
    return &broker_;
}

std::shared_ptr<AbilityRuntime::AbilityContext> CJAbilityContext::GetAbilityContext()
{
    return context_;
}

sptr<IRemoteObject> CJAbilityContext::GetToken()
{
    return context_->GetToken();
}

std::string CJAbilityContext::GetPreferencesDir()
{
    return context_->GetPreferencesDir();
}

std::string CJAbilityContext::GetDatabaseDir()
{
    return context_->GetDatabaseDir();
}

std::string CJAbilityContext::GetBundleName()
{
    return context_->GetBundleName();
}

int32_t CJAbilityContext::GetArea()
{
    return context_->GetArea();
}

std::shared_ptr<AppExecFwk::AbilityInfo> CJAbilityContext::GetAbilityInfo()
{
    return context_->GetAbilityInfo();
}

std::shared_ptr<AppExecFwk::HapModuleInfo> CJAbilityContext::GetHapModuleInfo()
{
    return context_->GetHapModuleInfo();
}

std::shared_ptr<AppExecFwk::Configuration> CJAbilityContext::GetConfiguration()
{
    return context_->GetConfiguration();
}

int32_t CJAbilityContext::StartAbility(const AAFwk::Want& want)
{
    // -1 is default accountId which is the same as js.
    return context_->StartAbility(want, -1);
}

int32_t CJAbilityContext::StartAbility(const AAFwk::Want& want, const AAFwk::StartOptions& startOptions)
{
    return context_->StartAbility(want, startOptions, -1);
}

int32_t CJAbilityContext::StartAbilityWithAccount(const AAFwk::Want& want, int32_t accountId)
{
    return context_->StartAbilityWithAccount(want, accountId, -1);
}

int32_t CJAbilityContext::StartAbilityWithAccount(
    const AAFwk::Want& want, int32_t accountId, const AAFwk::StartOptions& startOptions)
{
    return context_->StartAbilityWithAccount(want, accountId, startOptions, -1);
}

int32_t CJAbilityContext::StartServiceExtensionAbility(const Want& want)
{
    return context_->StartServiceExtensionAbility(want);
}

int32_t CJAbilityContext::StartServiceExtensionAbilityWithAccount(const Want& want, int32_t accountId)
{
    return context_->StartServiceExtensionAbility(want, accountId);
}

int32_t CJAbilityContext::StopServiceExtensionAbility(const Want& want)
{
    return context_->StopServiceExtensionAbility(want);
}

int32_t CJAbilityContext::StopServiceExtensionAbilityWithAccount(const Want& want, int32_t accountId)
{
    return context_->StopServiceExtensionAbility(want, accountId);
}

int32_t CJAbilityContext::TerminateSelf()
{
    return context_->TerminateSelf();
}

int32_t CJAbilityContext::TerminateSelfWithResult(const AAFwk::Want& want, int32_t resultCode)
{
    return context_->TerminateAbilityWithResult(want, resultCode);
}

std::tuple<int32_t, bool> CJAbilityContext::IsTerminating()
{
    return std::make_tuple(SUCCESS_CODE, context_->IsTerminating());
}

bool CJAbilityContext::ConnectAbility(const AAFwk::Want& want, int64_t connectionId)
{
    auto connection = new CJAbilityConnectCallback(connectionId);
    return context_->ConnectAbility(want, connection);
}

int32_t CJAbilityContext::ConnectAbilityWithAccount(
    const AAFwk::Want& want, int32_t accountId, int64_t connectionId)
{
    auto connection = new CJAbilityConnectCallback(connectionId);
    return context_->ConnectAbilityWithAccount(want, accountId, connection);
}

void CJAbilityContext::DisconnectAbility(const AAFwk::Want& want, int64_t connectionId)
{
    auto connection = new CJAbilityConnectCallback(connectionId);
    context_->ConnectAbility(want, connection);
}

int32_t CJAbilityContext::StartAbilityForResult(const AAFwk::Want& want, int32_t requestCode, RuntimeTask&& task)
{
    return context_->StartAbilityForResult(want, requestCode, std::move(task));
}

int32_t CJAbilityContext::StartAbilityForResult(
    const AAFwk::Want& want, const AAFwk::StartOptions& startOptions, int32_t requestCode, RuntimeTask&& task)
{
    return context_->StartAbilityForResult(want, startOptions, requestCode, std::move(task));
}

int32_t CJAbilityContext::StartAbilityForResultWithAccount(
    const AAFwk::Want& want, int32_t accountId, int32_t requestCode, RuntimeTask&& task)
{
    return context_->StartAbilityForResultWithAccount(want, accountId, requestCode, std::move(task));
}

int32_t CJAbilityContext::StartAbilityForResultWithAccount(const AAFwk::Want& want, int32_t accountId,
    const AAFwk::StartOptions& startOptions, int32_t requestCode, RuntimeTask&& task)
{
    return context_->StartAbilityForResultWithAccount(want, accountId, startOptions, requestCode, std::move(task));
}

int32_t CJAbilityContext::RequestPermissionsFromUser(
    AppExecFwk::Ability* ability, std::vector<std::string>& permissions, PermissionRequestTask&& task)
{
    return SUCCESS_CODE;
}

#ifdef SUPPORT_GRAPHICS
int32_t CJAbilityContext::SetMissionLabel(const std::string& label)
{
    return context_->SetMissionLabel(label);
}

int32_t CJAbilityContext::SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap>& icon)
{
    return context_->SetMissionIcon(icon);
}
#endif

void CJAbilityContext::InheritWindowMode(AAFwk::Want& want)
{
    HILOG_INFO("CJAbilityContext::InheritWindowMode called.");
#ifdef SUPPORT_GRAPHICS
    // Only split mode need inherit.
    auto windowMode = context_->GetCurrentWindowMode();
    if (windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }
    HILOG_INFO("CJAbilityContext::InheritWindowMode called end. window mode is %{public}d", windowMode);
#endif
}

int32_t CJAbilityContext::RequestDialogService(AAFwk::Want& want, RequestDialogResultTask&& task)
{
    return context_->RequestDialogService(want, std::move(task));
}

extern "C" {
#define EXPORT __attribute__((visibility("default")))
EXPORT AbilityContextBroker* FFIAbilityContextGetBroker()
{
    return CJAbilityContext::GetBrokerPointer();
}

EXPORT void *FFIGetContext(int64_t id)
{
    if (auto cjContext = FFIData::GetData<CJAbilityContext>(id)) {
        return cjContext->GetAbilityContext().get();
    }
    return nullptr;
}

typedef struct napi_env__ *napi_env;
typedef struct napi_value__* napi_value;

EXPORT napi_value FFICreateNapiValue(void *env, void *context)
{
    napi_value result = nullptr;
    napi_create_object((napi_env)env, &result);
    if (result == nullptr) {
        HILOG_ERROR("FFICreateNapiValue create object failed.");
        return nullptr;
    }
    auto nativeFinalize = [](napi_env env, void* data, void* hint) {
        auto tmp = reinterpret_cast<std::weak_ptr<Context> *>(data);
        delete tmp;
    };
    auto tmpContext = reinterpret_cast<AbilityContext*>(context);
    auto weakContext = new std::weak_ptr<Context>(tmpContext->weak_from_this());
    napi_wrap((napi_env)env, result, weakContext, nativeFinalize, nullptr, nullptr);
    napi_value value = nullptr;
    napi_get_boolean((napi_env)env, true, &value);
    napi_set_named_property((napi_env)env, result, "stageMode", value);

    return result;
}

#undef EXPORT

bool FFIAbilityContextIsAbilityContextExisted(int64_t id)
{
    return FFIData::GetData<CJAbilityContext>(id) != nullptr;
}

int64_t FFIAbilityContextGetSizeOfStartOptions()
{
    return sizeof(CJStartOptions);
}

int64_t FFIAbilityContextGetAbilityInfo(int64_t id)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("GetAbilityInfo failed, invalid id of CJAbilityContext");
        return INVALID_DATA_ID;
    }
    return (int64_t)NOT_SUPPORT;
}


int64_t FFIAbilityContextGetHapModuleInfo(int64_t id)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("GetHapModuleInfo failed, invalid id of CJAbilityContext");
        return INVALID_DATA_ID;
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    return (int64_t)NOT_SUPPORT;
}


int64_t FFIAbilityContextGetConfiguration(int64_t id)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("GetConfiguration failed, invalid id of CJAbilityContext");
        return INVALID_DATA_ID;
    }
    auto configuration = context->GetConfiguration();
    return (int64_t)NOT_SUPPORT;
}

int32_t FFIAbilityContextStartAbility(int64_t id, WantHandle want)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("StartAbility failed, invalid id of CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }

    auto actualWant = reinterpret_cast<Want*>(want);
    context->InheritWindowMode(*actualWant);
    return context->StartAbility(*actualWant);
}

int32_t FFIAbilityContextStartAbilityWithOption(int64_t id, WantHandle want, CJStartOptions* startOption)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("StartAbility failed, invalid id of CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto actualWant = reinterpret_cast<Want*>(want);
    context->InheritWindowMode(*actualWant);
    AAFwk::StartOptions option;
    UnWrapStartOption(startOption, option);
    return context->StartAbility(*actualWant, option);
}

int32_t FFIAbilityContextStartAbilityWithAccount(int64_t id, WantHandle want, int32_t accountId)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("StartAbilityWithAccount failed, invalid id of CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto actualWant = reinterpret_cast<Want*>(want);
    context->InheritWindowMode(*actualWant);
    return context->StartAbilityWithAccount(*actualWant, accountId);
}

int32_t FFIAbilityContextStartAbilityWithAccountAndOption(
    int64_t id, WantHandle want, int32_t accountId, CJStartOptions* startOption)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("StartAbilityWithAccount failed, invalid id of CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto actualWant = reinterpret_cast<Want*>(want);
    context->InheritWindowMode(*actualWant);
    AAFwk::StartOptions option;
    UnWrapStartOption(startOption, option);
    return context->StartAbilityWithAccount(*actualWant, accountId, option);
}

int32_t FFIAbilityContextStartServiceExtensionAbility(int64_t id, WantHandle want)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("StartServiceExtensionAbility failed, invalid id of CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto actualWant = reinterpret_cast<Want*>(want);
    return context->StartServiceExtensionAbility(*actualWant);
}

int32_t FFIAbilityContextStartServiceExtensionAbilityWithAccount(int64_t id, WantHandle want, int32_t accountId)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("StartServiceExtensionAbilityWithAccount failed, invalid id of CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto actualWant = reinterpret_cast<Want*>(want);
    return context->StartServiceExtensionAbilityWithAccount(*actualWant, accountId);
}

int32_t FFIAbilityContextStopServiceExtensionAbility(int64_t id, WantHandle want)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("StopServiceExtensionAbility failed, invalid id of CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto actualWant = reinterpret_cast<Want*>(want);
    return context->StopServiceExtensionAbility(*actualWant);
}

int32_t FFIAbilityContextStopServiceExtensionAbilityWithAccount(int64_t id, WantHandle want, int32_t accountId)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("StopServiceExtensionAbilityWithAccount failed, invalid id of CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto actualWant = reinterpret_cast<Want*>(want);
    return context->StopServiceExtensionAbilityWithAccount(*actualWant, accountId);
}

int32_t FFIAbilityContextTerminateSelf(int64_t id)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("TerminateSelf failed, invalid id of CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context->TerminateSelf();
}

int32_t FFIAbilityContextTerminateSelfWithResult(int64_t id, WantHandle want, int32_t resultCode)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("TerminateSelfWithResult failed, invalid id of CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto actualWant = reinterpret_cast<Want*>(want);
    return context->TerminateSelfWithResult(*actualWant, resultCode);
}

RetDataBool FFIAbilityContextIsTerminating(int64_t id)
{
    RetDataBool res = { ERR_INVALID_INSTANCE_CODE, false };
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("IsTerminating failed, invalid id of CJAbilityContext");
        return res;
    }

    auto [code, data] = context->IsTerminating();
    res.code = code;
    res.data = data;
    return res;
}

int32_t FFIAbilityContextConnectAbility(int64_t id, WantHandle want, int64_t connection)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("ConnectAbility failed, invalid id of CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto actualWant = reinterpret_cast<AAFwk::Want*>(want);
    auto res = context->ConnectAbility(*actualWant, connection);
    return res ? SUCCESS_CODE : ERR_INVALID_INSTANCE_CODE;
}

int32_t FFIAbilityContextConnectAbilityWithAccount(int64_t id, WantHandle want, int32_t accountId, int64_t connection)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("ConnectAbility failed, invalid id of CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto actualWant = reinterpret_cast<AAFwk::Want*>(want);
    auto res = context->ConnectAbilityWithAccount(*actualWant, accountId, connection);
    return res ? SUCCESS_CODE : ERR_INVALID_INSTANCE_CODE;
}

int32_t FFIAbilityContextDisconnectAbility(int64_t id, WantHandle want, int64_t connection)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("ConnectAbility failed, invalid id of CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto actualWant = reinterpret_cast<AAFwk::Want*>(want);
    context->ConnectAbility(*actualWant, connection);
    return SUCCESS_CODE;
}

int32_t FFIAbilityContextStartAbilityForResult(int64_t id, WantHandle want, int32_t requestCode, int64_t lambdaId)
{
    auto cjTask = WrapCJAbilityResultTask(lambdaId);
    RuntimeTask task = WrapRuntimeTask(cjTask, SUCCESS_CODE);

    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("StartAbilityForResult failed, invalid id of CJAbilityContext");
        cjTask(ERR_INVALID_INSTANCE_CODE, nullptr);
        return ERR_INVALID_INSTANCE_CODE;
    }

    auto actualWant = reinterpret_cast<AAFwk::Want*>(want);
    context->InheritWindowMode(*actualWant);
    return context->StartAbilityForResult(*actualWant, requestCode, std::move(task));
}

int32_t FFIAbilityContextStartAbilityForResultWithOption(
    int64_t id, WantHandle want, CJStartOptions* startOption, int32_t requestCode, int64_t lambdaId)
{
    auto cjTask = WrapCJAbilityResultTask(lambdaId);
    RuntimeTask task = WrapRuntimeTask(cjTask, SUCCESS_CODE);

    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("StartAbilityForResult failed, invalid id of CJAbilityContext");
        cjTask(ERR_INVALID_INSTANCE_CODE, nullptr);
        return ERR_INVALID_INSTANCE_CODE;
    }

    auto actualWant = reinterpret_cast<AAFwk::Want*>(want);
    context->InheritWindowMode(*actualWant);
    AAFwk::StartOptions option;
    UnWrapStartOption(startOption, option);
    return context->StartAbilityForResult(*actualWant, option, requestCode, std::move(task));
}

int32_t FFIAbilityContextStartAbilityForResultWithAccount(
    int64_t id, WantHandle want, int32_t accountId, int32_t requestCode, int64_t lambdaId)
{
    auto cjTask = WrapCJAbilityResultTask(lambdaId);
    RuntimeTask task = WrapRuntimeTask(cjTask, SUCCESS_CODE);

    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("StartAbilityForResult failed, invalid id of CJAbilityContext");
        cjTask(ERR_INVALID_INSTANCE_CODE, nullptr);
        return ERR_INVALID_INSTANCE_CODE;
    }

    auto actualWant = reinterpret_cast<AAFwk::Want*>(want);
    context->InheritWindowMode(*actualWant);
    return context->StartAbilityForResultWithAccount(*actualWant, accountId, requestCode, std::move(task));
}

int32_t FFIAbilityContextStartAbilityForResultWithAccountAndOption(
    int64_t id, WantHandle want, int32_t accountId, CJStartOptions* startOption, int32_t requestCode, int64_t lambdaId)
{
    auto cjTask = WrapCJAbilityResultTask(lambdaId);
    RuntimeTask task = WrapRuntimeTask(cjTask, SUCCESS_CODE);

    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("StartAbilityForResult failed, invalid id of CJAbilityContext");
        cjTask(ERR_INVALID_INSTANCE_CODE, nullptr);
        return ERR_INVALID_INSTANCE_CODE;
    }

    auto actualWant = reinterpret_cast<AAFwk::Want*>(want);
    context->InheritWindowMode(*actualWant);
    AAFwk::StartOptions option;
    UnWrapStartOption(startOption, option);
    return context->StartAbilityForResultWithAccount(*actualWant, accountId, option, requestCode, std::move(task));
}

int32_t FFIAbilityContextRequestPermissionsFromUser(
    int64_t id, VectorStringHandle permissions, int32_t requestCode, int64_t lambdaId)
{
    auto cjTask = [lambdaId](int32_t error, CJPermissionRequestResult* cjPermissionRequestResult) {
        if (g_cjAbilityCallbacks == nullptr) {
            HILOG_ERROR("RequestPermissionsFromUser failed, cangjie callbacks are not registered");
            return;
        }
        g_cjAbilityCallbacks->invokePermissionRequestResultCallback(lambdaId, error, cjPermissionRequestResult);
        HILOG_INFO("RequestPermissionsFromUser callback is called, error is %{public}d", error);
    };
    PermissionRequestTask task = [cjTask](const std::vector<std::string>& permissions,
                                     const std::vector<int>& grantResults) {
        VectorStringHandle permissionList = const_cast<std::vector<std::string>*>(&permissions);
        VectorInt32Handle result = const_cast<std::vector<int>*>(&grantResults);
        CJPermissionRequestResult cjPermissionRequestResult = { permissionList, result };
        cjTask(SUCCESS_CODE, &cjPermissionRequestResult);
    };

    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("RequestPermissionsFromUser failed, invalid id of CJAbilityContext");
        cjTask(ERR_INVALID_INSTANCE_CODE, nullptr);
        return ERR_INVALID_INSTANCE_CODE;
    }

    auto actualPermissions = reinterpret_cast<std::vector<std::string>*>(permissions);
    if (actualPermissions->empty()) {
        HILOG_ERROR("Params do not meet specification, permissions is empty.");
        cjTask(ERR_INVALID_INSTANCE_CODE, nullptr);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return true;
}

#ifdef SUPPORT_GRAPHICS
int32_t FFIAbilityContextSetMissionLabel(int64_t id, const char* label)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("SetMissionLabel failed, invalid id of CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context->SetMissionLabel(label);
}

int32_t FFIAbilityContextSetMissionIcon(int64_t id, int64_t pixelMapId)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("SetMissionIcon failed, invalid id of CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return 0;
}
#endif

int32_t FFIAbilityContextRequestDialogService(int64_t id, WantHandle want, int64_t lambdaId)
{
    auto context = FFIData::GetData<CJAbilityContext>(id);
    if (context == nullptr) {
        HILOG_ERROR("RequestDialogService failed, invalid id of CJAbilityContext");
        return ERR_INVALID_INSTANCE_CODE;
    }
    RequestDialogResultTask task = [lambdaId](int32_t resultCode, const AAFwk::Want &resultWant) {
        if (g_cjAbilityCallbacks == nullptr) {
            HILOG_ERROR("RequestDialogService failed, cangjie callbacks are not registered");
            return;
        }
        CJDialogRequestResult dialogRequestResult = {
            .resultCode = resultCode,
            .wantHandle = (WantHandle)&resultWant
        };
        g_cjAbilityCallbacks->invokeDialogRequestResultCallback(lambdaId, resultCode, &dialogRequestResult);
        HILOG_INFO("RequestDialogService callback is called, resultCode is %{public}d", resultCode);
    };
    auto actualWant = reinterpret_cast<AAFwk::Want*>(want);
    return context->RequestDialogService(*actualWant, std::move(task));
}


AbilityContextBroker CJAbilityContext::broker_ = { .isAbilityContextExisted = FFIAbilityContextIsAbilityContextExisted,
    .getSizeOfStartOptions = FFIAbilityContextGetSizeOfStartOptions,

    .getAbilityInfo = FFIAbilityContextGetAbilityInfo,
    .getHapModuleInfo = FFIAbilityContextGetHapModuleInfo,
    .getConfiguration = FFIAbilityContextGetConfiguration,

    .startAbility = FFIAbilityContextStartAbility,
    .startAbilityWithOption = FFIAbilityContextStartAbilityWithOption,
    .startAbilityWithAccount = FFIAbilityContextStartAbilityWithAccount,
    .startAbilityWithAccountAndOption = FFIAbilityContextStartAbilityWithAccountAndOption,
    .startServiceExtensionAbility = FFIAbilityContextStartServiceExtensionAbility,
    .startServiceExtensionAbilityWithAccount = FFIAbilityContextStartServiceExtensionAbilityWithAccount,
    .stopServiceExtensionAbility = FFIAbilityContextStopServiceExtensionAbility,
    .stopServiceExtensionAbilityWithAccount = FFIAbilityContextStopServiceExtensionAbilityWithAccount,

    .terminateSelf = FFIAbilityContextTerminateSelf,
    .terminateSelfWithResult = FFIAbilityContextTerminateSelfWithResult,
    .isTerminating = FFIAbilityContextIsTerminating,

    .connectAbility = FFIAbilityContextConnectAbility,
    .connectAbilityWithAccount = FFIAbilityContextConnectAbilityWithAccount,
    .disconnectAbility = FFIAbilityContextDisconnectAbility,
    .startAbilityForResult = FFIAbilityContextStartAbilityForResult,
    .startAbilityForResultWithOption = FFIAbilityContextStartAbilityForResultWithOption,
    .startAbilityForResultWithAccount = FFIAbilityContextStartAbilityForResultWithAccount,
    .startAbilityForResultWithAccountAndOption = FFIAbilityContextStartAbilityForResultWithAccountAndOption,
    .requestPermissionsFromUser = FFIAbilityContextRequestPermissionsFromUser,

    .setMissionLabel = FFIAbilityContextSetMissionLabel,
    .setMissionIcon = FFIAbilityContextSetMissionIcon };
}

} // namespace AbilityRuntime
} // namespace OHOS
