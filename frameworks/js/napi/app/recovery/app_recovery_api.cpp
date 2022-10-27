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

#include "app_recovery_api.h"

#include "app_recovery.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

#include "recovery_param.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
enum RestartErrno {
    NO_ERROR,
    SAVED_STATE_NOT_EXIST,
    LAST_RESTART_ENCOUNTER_ERROR,
};
class AppRecoveryApiRegistry {
public:
    AppRecoveryApiRegistry() = default;
    ~AppRecoveryApiRegistry() = default;

    static void Finalizer(NativeEngine *engine, void *data, void *hint)
    {
        std::unique_ptr<AppRecoveryApiRegistry>(static_cast<AppRecoveryApiRegistry*>(data));
    }

    static NativeValue *EnableAppRecovery(NativeEngine *engine, NativeCallbackInfo *info)
    {
        AppRecoveryApiRegistry *me = CheckParamsAndGetThis<AppRecoveryApiRegistry>(engine, info);
        return (me != nullptr) ? me->OnEnableAppRecovery(*engine, *info) : nullptr;
    }

    static NativeValue *RestartApp(NativeEngine *engine, NativeCallbackInfo *info)
    {
        AppRecoveryApiRegistry *me = CheckParamsAndGetThis<AppRecoveryApiRegistry>(engine, info);
        return (me != nullptr) ? me->OnRestartApp(*engine, *info) : nullptr;
    }

    static NativeValue *SaveAppState(NativeEngine *engine, NativeCallbackInfo *info)
    {
        AppRecoveryApiRegistry *me = CheckParamsAndGetThis<AppRecoveryApiRegistry>(engine, info);
        return (me != nullptr) ? me->OnSaveAppState(*engine, *info) : nullptr;
    }

private:
    NativeValue *OnEnableAppRecovery(NativeEngine &engine, NativeCallbackInfo &info)
    {
        size_t parameterCount = info.argc;
        NativeValue* result = engine.CreateUndefined();
        constexpr int maxCount = 3;
        if (parameterCount > maxCount) {
            return result;
        }

        uint16_t flags[] = {
            RestartFlag::ALWAYS_RESTART,
            SaveOccasionFlag::SAVE_WHEN_ERROR,
            SaveModeFlag::SAVE_WITH_FILE
        };

        for (size_t i = 0; i < parameterCount; ++i) {
            napi_valuetype paramType;
            napi_typeof(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[i]), &paramType);
            if (paramType != napi_number) {
                HILOG_ERROR("AppRecoveryApi info.argv[%{public}s] type isn't number", std::to_string(i).c_str());
                return result;
            }
            int32_t tmp = 0;
            napi_get_value_int32(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[i]), &tmp);
            flags[i] = static_cast<uint16_t>(tmp);
        }
        AppRecovery::GetInstance().EnableAppRecovery(flags[0],  // 0:RestartFlag
                                                     flags[1],  // 1:SaveOccasionFlag
                                                     flags[2]); // 2:SaveModeFlag
        return result;
    }

    NativeValue *OnSaveAppState(NativeEngine &engine, const NativeCallbackInfo &info)
    {
        if (info.argc != 0) {
            HILOG_ERROR("AppRecoveryApi SaveAppState Incorrect number of parameters");
            return engine.CreateUndefined();
        }

        if (AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::DEVELOPER_REQUEST)) {
            return engine.CreateBoolean(true);
        }
        return engine.CreateBoolean(false);
    }

    NativeValue *OnRestartApp(NativeEngine &engine, const NativeCallbackInfo &info)
    {
        if (info.argc != 0) {
            HILOG_ERROR("AppRecoveryApi OnRestartApp Incorrect number of parameters");
            return engine.CreateUndefined();
        }

        AppRecovery::GetInstance().ScheduleRecoverApp(StateReason::DEVELOPER_REQUEST);
        return engine.CreateUndefined();
    }
};
} // namespace

NativeValue *AppRecoveryRestartFlagInit(NativeEngine *engine)
{
    if (engine == nullptr) {
        HILOG_ERROR("AppRecoveryRestartFlagInit Invalid input parameters");
        return nullptr;
    }

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);

    if (object == nullptr) {
        HILOG_ERROR("AppRecoveryRestartFlagInit Failed to get object");
        return nullptr;
    }

    object->SetProperty("ALWAYS_RESTART", CreateJsValue(*engine, RestartFlag::ALWAYS_RESTART));
    object->SetProperty("CPP_CRASH_NO_RESTART", CreateJsValue(*engine, RestartFlag::CPP_CRASH_NO_RESTART));
    object->SetProperty("JS_CRASH_NO_RESTART", CreateJsValue(*engine, RestartFlag::JS_CRASH_NO_RESTART));
    object->SetProperty("APP_FREEZE_NO_RESTART", CreateJsValue(*engine, RestartFlag::APP_FREEZE_NO_RESTART));
    return objValue;
}

NativeValue *AppRecoveryStateSaveFlagInit(NativeEngine *engine)
{
    if (engine == nullptr) {
        HILOG_ERROR("AppRecoveryStateSaveFlagInit Invalid input parameters");
        return nullptr;
    }

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);

    if (object == nullptr) {
        HILOG_ERROR("AppRecoveryStateSaveFlagInit Failed to get object");
        return nullptr;
    }

    object->SetProperty("NONE", CreateJsValue(*engine, SaveOccasionFlag::NO_SAVE));
    object->SetProperty("SAVE_WHEN_ERROR", CreateJsValue(*engine, SaveOccasionFlag::SAVE_WHEN_ERROR));
    object->SetProperty("SAVE_WHEN_BACKGROUND", CreateJsValue(*engine, SaveOccasionFlag::SAVE_WHEN_BACKGROUND));
    return objValue;
}

NativeValue *AppRecoverySaveModeFlagInit(NativeEngine *engine)
{
    if (engine == nullptr) {
        HILOG_ERROR("AppRecoverySaveModeFlagInit Invalid input parameters");
        return nullptr;
    }

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);

    if (object == nullptr) {
        HILOG_ERROR("AppRecoverySaveModeFlagInit Failed to get object");
        return nullptr;
    }

    object->SetProperty("SAVE_WITH_FILE", CreateJsValue(*engine, SaveModeFlag::SAVE_WITH_FILE));
    object->SetProperty("SAVE_WITH_SHARED_MEMORY", CreateJsValue(*engine, SaveModeFlag::SAVE_WITH_SHARED_MEMORY));
    return objValue;
}

NativeValue *InitAppRecoveryApiModule(NativeEngine *engine, NativeValue *exportObj)
{
    if (engine == nullptr || exportObj == nullptr) {
        HILOG_ERROR("AppRecovery API Invalid input parameters");
        return nullptr;
    }

    NativeObject *object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_ERROR("AppRecovery API Failed to get object");
        return nullptr;
    }

    std::unique_ptr<AppRecoveryApiRegistry> appRecoveryApi = std::make_unique<AppRecoveryApiRegistry>();
    object->SetNativePointer(appRecoveryApi.release(), AppRecoveryApiRegistry::Finalizer, nullptr);

    const char *moduleName = "AppRecovery";
    BindNativeFunction(*engine, *object, "enableAppRecovery", moduleName, AppRecoveryApiRegistry::EnableAppRecovery);
    BindNativeFunction(*engine, *object, "restartApp", moduleName, AppRecoveryApiRegistry::RestartApp);
    BindNativeFunction(*engine, *object, "saveAppState", moduleName, AppRecoveryApiRegistry::SaveAppState);

    object->SetProperty("RestartFlag", AppRecoveryRestartFlagInit(engine));
    object->SetProperty("SaveOccasionFlag", AppRecoveryStateSaveFlagInit(engine));
    object->SetProperty("SaveModeFlag", AppRecoverySaveModeFlagInit(engine));

    return engine->CreateUndefined();
}
}  // namespace AbilityRuntime
}  // namespace OHOS