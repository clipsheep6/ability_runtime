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

#include "napi_common_want.h"
#include "recovery_param.h"
#include "want.h"

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

    static NativeValue *SetRestartWant(NativeEngine *engine, NativeCallbackInfo *info)
    {
        AppRecoveryApiRegistry *me = CheckParamsAndGetThis<AppRecoveryApiRegistry>(engine, info);
        return (me != nullptr) ? me->OnSetRestartWant(*engine, *info) : nullptr;
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

        if (!CheckParamsValid(flags)) {
            return result;
        }

        AppRecovery::GetInstance().EnableAppRecovery(flags[0],  // 0:RestartFlag
                                                     flags[1],  // 1:SaveOccasionFlag
                                                     flags[2]); // 2:SaveModeFlag
        return result;
    }

    bool CheckParamsValid(const uint16_t params[])
    {
        uint16_t restartFlag = params[0];
        constexpr uint16_t restartMaxVal = 0x0003;
        if ((restartFlag < 0 || restartFlag > restartMaxVal) && (restartFlag != RestartFlag::NO_RESTART)) {
            HILOG_ERROR("AppRecoveryApi CheckParamsValid restartFlag: %{public}d is Invalid", restartFlag);
            return false;
        }
        uint16_t saveFlag = params[1];
        constexpr uint16_t saveMaxVal = 0x0003;
        if (saveFlag < SaveOccasionFlag::SAVE_WHEN_ERROR || saveFlag > saveMaxVal) {
            HILOG_ERROR("AppRecoveryApi CheckParamsValid SaveOccasionFlag: %{public}d is Invalid", saveFlag);
            return false;
        }
        uint16_t saveModeFlag = params[2];
        if (saveModeFlag < SaveModeFlag::SAVE_WITH_FILE || saveModeFlag > SaveModeFlag::SAVE_WITH_SHARED_MEMORY) {
            HILOG_ERROR("AppRecoveryApi CheckParamsValid saveModeFlag: %{public}d is Invalid", saveModeFlag);
            return false;
        }
        return true;
    }

    NativeValue *OnSaveAppState(NativeEngine &engine, const NativeCallbackInfo &info)
    {
        if (info.argc > 1) {
            HILOG_ERROR("AppRecoveryApi SaveAppState Incorrect number of parameters");
            return engine.CreateBoolean(false);
        }
        uintptr_t ability = 0;
        if (info.argc == 1) {
            NativeValue* value = reinterpret_cast<NativeValue*>(info.argv[0]);
            NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
            if (obj == nullptr) {
                HILOG_ERROR("AppRecoveryApi Invalid abilityContext.");
                return engine.CreateBoolean(false);
            }
            ability = reinterpret_cast<uintptr_t>(obj->GetNativePointer());
        }
        if (AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::DEVELOPER_REQUEST, ability)) {
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

    NativeValue *OnSetRestartWant(NativeEngine &engine, const NativeCallbackInfo &info)
    {
        if (info.argc != 1) {
            HILOG_ERROR("AppRecoveryApi OnSetRestartWant Incorrect number of parameters");
            return engine.CreateUndefined();
        }
        std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
        OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[0]), *(want.get()));
        AppRecovery::GetInstance().setRestartWant(want);
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
    object->SetProperty("RESTART_WHEN_JS_CRASH", CreateJsValue(*engine, RestartFlag::RESTART_WHEN_JS_CRASH));
    object->SetProperty("RESTART_WHEN_APP_FREEZE", CreateJsValue(*engine, RestartFlag::RESTART_WHEN_APP_FREEZE));
    object->SetProperty("NO_RESTART", CreateJsValue(*engine, RestartFlag::NO_RESTART));
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
    BindNativeFunction(*engine, *object, "setRestartWant", moduleName, AppRecoveryApiRegistry::SetRestartWant);

    object->SetProperty("RestartFlag", AppRecoveryRestartFlagInit(engine));
    object->SetProperty("SaveOccasionFlag", AppRecoveryStateSaveFlagInit(engine));
    object->SetProperty("SaveModeFlag", AppRecoverySaveModeFlagInit(engine));

    return engine->CreateUndefined();
}
}  // namespace AbilityRuntime
}  // namespace OHOS