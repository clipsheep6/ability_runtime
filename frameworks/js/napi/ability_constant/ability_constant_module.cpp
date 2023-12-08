/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "ability_window_configuration.h"
#include "hilog_wrapper.h"
#include "launch_param.h"
#include "mission_info.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "recovery_param.h"

namespace OHOS {
namespace AAFwk {
enum class MemoryLevel {
    MEMORY_LEVEL_MODERATE = 0,
    MEMORY_LEVEL_LOW = 1,
    MEMORY_LEVEL_CRITICAL = 2,
};

static napi_status SetEnumItem(napi_env env, napi_value object, const char* name, int32_t value)
{
    HILOG_DEBUG("start");
    napi_status status;
    napi_value itemName;
    napi_value itemValue;

    NAPI_CALL_BASE(env, status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &itemName), status);
    NAPI_CALL_BASE(env, status = napi_create_int32(env, value, &itemValue), status);

    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemName, itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemValue, itemName), status);

    HILOG_DEBUG("end");
    return napi_ok;
}

static napi_value InitLaunchReasonObject(napi_env env)
{
    HILOG_DEBUG("start");
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "UNKNOWN", LAUNCHREASON_UNKNOWN));
    NAPI_CALL(env, SetEnumItem(env, object, "START_ABILITY", LAUNCHREASON_START_ABILITY));
    NAPI_CALL(env, SetEnumItem(env, object, "CALL", LAUNCHREASON_CALL));
    NAPI_CALL(env, SetEnumItem(env, object, "CONTINUATION", LAUNCHREASON_CONTINUATION));
    NAPI_CALL(env, SetEnumItem(env, object, "APP_RECOVERY", LAUNCHREASON_APP_RECOVERY));
    NAPI_CALL(env, SetEnumItem(env, object, "SHARE", LAUNCHREASON_SHARE));
    NAPI_CALL(env, SetEnumItem(env, object, "AUTO_STARTUP", LAUNCHREASON_AUTO_STARTUP));
    NAPI_CALL(env, SetEnumItem(env, object, "INSIGHT_INTENT", LAUNCHREASON_INSIGHT_INTENT));

    HILOG_DEBUG("end");
    return object;
}

static napi_value InitLastExitReasonObject(napi_env env)
{
    HILOG_DEBUG("start");
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "UNKNOWN", LASTEXITREASON_UNKNOWN));
    NAPI_CALL(env, SetEnumItem(env, object, "ABILITY_NOT_RESPONDING", LASTEXITREASON_ABILITY_NOT_RESPONDING));
    NAPI_CALL(env, SetEnumItem(env, object, "NORMAL", LASTEXITREASON_NORMAL));
    NAPI_CALL(env, SetEnumItem(env, object, "CPP_CRASH", LASTEXITREASON_CPP_CRASH));
    NAPI_CALL(env, SetEnumItem(env, object, "JS_ERROR", LASTEXITREASON_JS_ERROR));
    NAPI_CALL(env, SetEnumItem(env, object, "APP_FREEZE", LASTEXITREASON_APP_FREEZE));
    NAPI_CALL(env, SetEnumItem(env, object, "PERFORMANCE_CONTROL", LASTEXITREASON_PERFORMANCE_CONTROL));
    NAPI_CALL(env, SetEnumItem(env, object, "RESOURCE_CONTROL", LASTEXITREASON_RESOURCE_CONTROL));
    NAPI_CALL(env, SetEnumItem(env, object, "UPGRADE", LASTEXITREASON_UPGRADE));

    HILOG_DEBUG("end");
    return object;
}

static napi_value InitOnContinueResultObject(napi_env env)
{
    HILOG_DEBUG("start");
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "AGREE", ONCONTINUE_AGREE));
    NAPI_CALL(env, SetEnumItem(env, object, "REJECT", ONCONTINUE_REJECT));
    NAPI_CALL(env, SetEnumItem(env, object, "MISMATCH", ONCONTINUE_MISMATCH));

    HILOG_DEBUG("end");
    return object;
}

static napi_value InitContinueStateObject(napi_env env)
{
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "ACTIVE", AAFwk::ContinueState::CONTINUESTATE_ACTIVE));
    NAPI_CALL(env, SetEnumItem(env, object, "INACTIVE", AAFwk::ContinueState::CONTINUESTATE_INACTIVE));

    return object;
}

static napi_value InitWindowModeObject(napi_env env)
{
    HILOG_DEBUG("start");
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "WINDOW_MODE_UNDEFINED", MULTI_WINDOW_DISPLAY_UNDEFINED));
    NAPI_CALL(env, SetEnumItem(env, object, "WINDOW_MODE_FULLSCREEN", MULTI_WINDOW_DISPLAY_FULLSCREEN));
    NAPI_CALL(env, SetEnumItem(env, object, "WINDOW_MODE_SPLIT_PRIMARY", MULTI_WINDOW_DISPLAY_PRIMARY));
    NAPI_CALL(env, SetEnumItem(env, object, "WINDOW_MODE_SPLIT_SECONDARY", MULTI_WINDOW_DISPLAY_SECONDARY));
    NAPI_CALL(env, SetEnumItem(env, object, "WINDOW_MODE_FLOATING", MULTI_WINDOW_DISPLAY_FLOATING));

    HILOG_DEBUG("end");
    return object;
}

// AbilityConstant.OnSaveResult
static napi_value InitOnSaveResultEnum(napi_env env)
{
    HILOG_DEBUG("start");
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));
    NAPI_CALL(env, SetEnumItem(env, object, "ALL_AGREE", AppExecFwk::ALL_AGREE));
    NAPI_CALL(env, SetEnumItem(env, object, "CONTINUATION_REJECT", AppExecFwk::CONTINUATION_REJECT));
    NAPI_CALL(env, SetEnumItem(env, object, "CONTINUATION_MISMATCH", AppExecFwk::CONTINUATION_MISMATCH));
    NAPI_CALL(env, SetEnumItem(env, object, "RECOVERY_AGREE", AppExecFwk::RECOVERY_AGREE));
    NAPI_CALL(env, SetEnumItem(env, object, "RECOVERY_REJECT", AppExecFwk::RECOVERY_REJECT));
    NAPI_CALL(env, SetEnumItem(env, object, "ALL_REJECT", AppExecFwk::ALL_REJECT));
    HILOG_DEBUG("end");
    return object;
}

// AbilityConstant.StateType
static napi_value InitStateTypeEnum(napi_env env)
{
    HILOG_DEBUG("start");
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));
    NAPI_CALL(env, SetEnumItem(env, object, "CONTINUATION", AppExecFwk::CONTINUATION));
    NAPI_CALL(env, SetEnumItem(env, object, "APP_RECOVERY", AppExecFwk::APP_RECOVERY));
    HILOG_DEBUG("end");
    return object;
}

static napi_value InitMemoryLevelObject(napi_env env)
{
    HILOG_DEBUG("start");
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "MEMORY_LEVEL_MODERATE",
        static_cast<int>(MemoryLevel::MEMORY_LEVEL_MODERATE)));
    NAPI_CALL(env, SetEnumItem(env, object, "MEMORY_LEVEL_LOW",
        static_cast<int>(MemoryLevel::MEMORY_LEVEL_LOW)));
    NAPI_CALL(env, SetEnumItem(env, object, "MEMORY_LEVEL_CRITICAL",
        static_cast<int>(MemoryLevel::MEMORY_LEVEL_CRITICAL)));

    HILOG_DEBUG("end");
    return object;
}

/*
 * The module initialization.
 */
static napi_value AbilityConstantInit(napi_env env, napi_value exports)
{
    napi_value launchReason = InitLaunchReasonObject(env);
    if (launchReason == nullptr) {
        HILOG_ERROR("error to create launch reason object");
        return nullptr;
    }

    napi_value lastExitReason = InitLastExitReasonObject(env);
    if (lastExitReason == nullptr) {
        HILOG_ERROR("error to create last exit reason object");
        return nullptr;
    }

    napi_value onContinueResult = InitOnContinueResultObject(env);
    if (onContinueResult == nullptr) {
        HILOG_ERROR("error to create onContinue result object");
        return nullptr;
    }

    napi_value continueState = InitContinueStateObject(env);
    if (continueState == nullptr) {
        HILOG_ERROR("error to create continue state object");
        return nullptr;
    }

    napi_value windowMode = InitWindowModeObject(env);
    if (windowMode == nullptr) {
        HILOG_ERROR("error to create window mode object");
        return nullptr;
    }

    napi_value memoryLevel = InitMemoryLevelObject(env);
    if (memoryLevel == nullptr) {
        HILOG_ERROR("error to create memory level object");
        return nullptr;
    }

    napi_value stateType = InitStateTypeEnum(env);
    if (stateType == nullptr) {
        HILOG_ERROR("error to create state type object");
        return nullptr;
    }

    napi_value saveResult = InitOnSaveResultEnum(env);
    if (saveResult == nullptr) {
        HILOG_ERROR("error to create save result object");
        return nullptr;
    }

    napi_property_descriptor exportObjs[] = {
        DECLARE_NAPI_PROPERTY("LaunchReason", launchReason),
        DECLARE_NAPI_PROPERTY("LastExitReason", lastExitReason),
        DECLARE_NAPI_PROPERTY("OnContinueResult", onContinueResult),
        DECLARE_NAPI_PROPERTY("ContinueState", continueState),
        DECLARE_NAPI_PROPERTY("WindowMode", windowMode),
        DECLARE_NAPI_PROPERTY("MemoryLevel", memoryLevel),
        DECLARE_NAPI_PROPERTY("OnSaveResult", saveResult),
        DECLARE_NAPI_PROPERTY("StateType", stateType),
    };
    napi_status status = napi_define_properties(env, exports, sizeof(exportObjs) / sizeof(exportObjs[0]), exportObjs);
    if (status != napi_ok) {
        HILOG_ERROR("error to define properties for exports");
        return nullptr;
    }

    return exports;
}

/*
 * The module definition.
 */
static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = AbilityConstantInit,
#ifdef ENABLE_ERRCODE
    .nm_modname = "app.ability.AbilityConstant",
#else
    .nm_modname = "application.AbilityConstant",
#endif
    .nm_priv = (static_cast<void *>(0)),
    .reserved = {0}
};

/*
 * The module registration.
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
}  // namespace AAFwk
}  // namespace OHOS
