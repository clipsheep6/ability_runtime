/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "form_runtime/js_form_extension_context.h"

#include <cinttypes>
#include <cstdint>

#include "form_runtime/js_form_extension_util.h"
#include "hilog_wrapper.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_ability.h"
#include "napi_common_start_options.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
const int UPDATE_FORM_PARAMS_SIZE = 2;
class JsFormExtensionContext final {
public:
    explicit JsFormExtensionContext(const std::shared_ptr<FormExtensionContext>& context) : context_(context) {}
    ~JsFormExtensionContext() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsAbilityContext::Finalizer is called");
        std::unique_ptr<JsFormExtensionContext>(static_cast<JsFormExtensionContext*>(data));
    }

    static NativeValue* UpdateForm(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsFormExtensionContext* me = CheckParamsAndGetThis<JsFormExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnUpdateForm(*engine, *info) : nullptr;
    }

    static NativeValue* StartAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsFormExtensionContext* me = CheckParamsAndGetThis<JsFormExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnStartAbility(*engine, *info) : nullptr;
    }

private:
    std::weak_ptr<FormExtensionContext> context_;

    NativeValue* OnUpdateForm(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("%{public}s called.", __func__);
        if (info.argc < UPDATE_FORM_PARAMS_SIZE) {
            HILOG_ERROR("Not enough params, not enough params");
            return engine.CreateUndefined();
        }

        std::string strFormId;
        ConvertFromJsValue(engine, info.argv[0], strFormId);
        int64_t formId = strFormId.empty() ? -1 : std::stoll(strFormId);

        AppExecFwk::FormProviderData formProviderData;
        std::string formDataStr = "{}";
        std::map<std::string, int> rawImageDataMap;
        NativeObject* nativeObject = ConvertNativeValueTo<NativeObject>(info.argv[1]);
        if (nativeObject != nullptr) {
            NativeValue* nativeDataValue = nativeObject->GetProperty("data");
            if (nativeDataValue == nullptr || !ConvertFromJsValue(engine, nativeDataValue, formDataStr)) {
                HILOG_ERROR("%{public}s called. nativeDataValue is nullptr or ConvertFromJsValue failed", __func__);
            }
            nativeDataValue = nativeObject->GetProperty("image");
            if (nativeDataValue != nullptr) {
                UnwrapRawImageDataMap(engine, nativeDataValue, rawImageDataMap);
            }
        } else {
            HILOG_ERROR("%{public}s called. nativeObject is nullptr", __func__);
        }

        formProviderData = AppExecFwk::FormProviderData(formDataStr);
        HILOG_INFO("Image number is %{public}zu", rawImageDataMap.size());
        for (auto entry : rawImageDataMap) {
            formProviderData.AddImageData(entry.first, entry.second);
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_, formId, formProviderData](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, 1, "Context is released"));
                    return;
                }
                auto errcode = context->UpdateForm(formId, formProviderData);
                if (errcode == ERR_OK) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "update form failed."));
                }
            };

        NativeValue* lastParam =
            (info.argc == UPDATE_FORM_PARAMS_SIZE) ? nullptr : info.argv[info.argc - 1];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JsFormExtensionContext::OnUpdateForm",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnStartAbility(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnStartAbility is called");
        // only support one or two params
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }

        decltype(info.argc) unwrapArgc = 0;
        AAFwk::Want want;
        OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), want);
        HILOG_INFO("%{public}s bundlename:%{public}s abilityname:%{public}s",
            __func__,
            want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        unwrapArgc++;

        AsyncTask::CompleteCallback complete =
            [weak = context_, want, unwrapArgc](NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("startAbility begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "Context is released"));
                    return;
                }

                // entry to the core functionality.
                ErrCode innerErrorCode = context->StartAbility(want);
                ErrCode errcode = AppExecFwk::GetStartAbilityErrorCode(innerErrorCode);
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Start Ability failed."));
                }
            };

        NativeValue* lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JsFormExtensionContext::OnStartAbility",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};
} // namespace

NativeValue* CreateJsFormExtensionContext(NativeEngine& engine, std::shared_ptr<FormExtensionContext> context,
                                          DetachCallback detach, AttachCallback attach)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }
    NativeValue* objValue = CreateJsExtensionContext(engine, context, abilityInfo, detach, attach);
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsFormExtensionContext> jsContext = std::make_unique<JsFormExtensionContext>(context);
    object->SetNativePointer(jsContext.release(), JsFormExtensionContext::Finalizer, nullptr);

    const char *moduleName = "JsFormExtensionContext";
    BindNativeFunction(engine, *object, "updateForm", moduleName, JsFormExtensionContext::UpdateForm);
    BindNativeFunction(engine, *object, "startAbility", moduleName, JsFormExtensionContext::StartAbility);

    HILOG_INFO("%{public}s called end.", __func__);
    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
