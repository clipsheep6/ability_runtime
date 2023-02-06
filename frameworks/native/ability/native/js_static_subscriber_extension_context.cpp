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

#include "js_static_subscriber_extension_context.h"

#include "hilog_wrapper.h"

#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_want.h"

#include "ability_runtime/js_caller_complex.h"
#include "js_data_struct_converter.h"
#include "napi_common_ability.h"
#include "napi_common_util.h"
#include "napi_remote_object.h"
#include "napi_common_start_options.h"
#include "start_options.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;

class JsStaticSubscriberExtensionContext final {
public:
    explicit JsStaticSubscriberExtensionContext(const std::shared_ptr<StaticSubscriberExtensionContext>& context)
        : context_(context) {}
    ~JsStaticSubscriberExtensionContext() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsStaticSubscriberExtensionContext::Finalizer is called");
        std::unique_ptr<JsStaticSubscriberExtensionContext>(
            static_cast<JsStaticSubscriberExtensionContext*>(data));
    }

    static NativeValue* StartAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsStaticSubscriberExtensionContext* me =
            CheckParamsAndGetThis<JsStaticSubscriberExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnStartAbility(*engine, *info) : nullptr;
    }

private:
    std::weak_ptr<StaticSubscriberExtensionContext> context_;

    NativeValue* OnStartAbility(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnStartAbility is called");
        // only support one or two or three params
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO && info.argc != ARGC_THREE) {
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

        AAFwk::StartOptions startOptions;
        if (info.argc > ARGC_ONE && info.argv[INDEX_ONE]->TypeOf() == NATIVE_OBJECT) {
            HILOG_INFO("OnStartAbility start options is used.");
            AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[INDEX_ONE]), startOptions);
            unwrapArgc++;
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_, want, startOptions, unwrapArgc](NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("startAbility begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "Context is released"));
                    return;
                }

                ErrCode innerErrorCode = ERR_OK;
                (unwrapArgc == 1) ? innerErrorCode = context->StartAbility(want) :
                    innerErrorCode = context->StartAbility(want, startOptions);
                ErrCode errcode = AppExecFwk::GetStartAbilityErrorCode(innerErrorCode);
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Start Ability failed."));
                }
            };

        NativeValue* lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JsStaticSubscriberExtensionContext::OnStartAbility",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};
} // namespace

NativeValue* CreateJsStaticSubscriberExtensionContext(NativeEngine& engine,
    std::shared_ptr<StaticSubscriberExtensionContext> context)
{
    HILOG_INFO("CreateJsStaticSubscriberExtensionContext begin");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }
    NativeValue* objValue = CreateJsExtensionContext(engine, context, abilityInfo);
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsStaticSubscriberExtensionContext> jsContext =
        std::make_unique<JsStaticSubscriberExtensionContext>(context);
    object->SetNativePointer(jsContext.release(), JsStaticSubscriberExtensionContext::Finalizer, nullptr);
    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
