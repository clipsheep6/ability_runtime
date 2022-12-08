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

#include "app_mgr.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "js_runtime_utils.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t ERROR_CODE_ONE = -2;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;

class JsAppManager final {
public:
    explicit JsAppManager(sptr<OHOS::AAFwk::IAbilityManager> abilityManager) : abilityManager_(abilityManager) {}
    ~JsAppManager() = default;

    static void Finalizer(NativeEngine *engine, void *data, void *hint)
    {
        HILOG_DEBUG("JsAppManager::Finalizer is called");
        std::unique_ptr<JsAppManager>(static_cast<JsAppManager*>(data));
    }

    static NativeValue* KillProcessesByBundleName(NativeEngine *engine, NativeCallbackInfo *info)
    {
        JsAppManager *me = CheckParamsAndGetThis<JsAppManager>(engine, info);
        return (me != nullptr) ? me->OnKillProcessByBundleName(*engine, *info) : nullptr;
    }

private:
    sptr<OHOS::AAFwk::IAbilityManager> abilityManager_ = nullptr;

    NativeValue* OnKillProcessByBundleName(NativeEngine &engine, const NativeCallbackInfo &info)
    {
        HILOG_DEBUG("%{public}s is called", __FUNCTION__);
        std::string bundleName;

        if (info.argc < ARGC_ONE || info.argc > ARGC_TWO) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }

        if (!ConvertFromJsValue(engine, info.argv[0], bundleName)) {
            HILOG_ERROR("get bundleName failed!");
            return engine.CreateUndefined();
        }

        HILOG_DEBUG("kill process [%{public}s]", bundleName.c_str());
        AsyncTask::CompleteCallback complete =
            [bundleName, abilityManager = abilityManager_](NativeEngine &engine, AsyncTask &task,
                int32_t status) {
            if (abilityManager == nullptr) {
                HILOG_ERROR("abilityManager nullptr");
                task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "abilityManager nullptr"));
                return;
            }
            auto ret = abilityManager->KillProcess(bundleName);
            if (ret == 0) {
                task.Resolve(engine, CreateJsValue(engine, ret));
            } else {
                task.Reject(engine, CreateJsError(engine, ret, "kill process failed."));
            }
        };

        NativeValue *lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
        NativeValue *result = nullptr;
        AsyncTask::Schedule("JSAppManager::OnKillProcessByBundleName",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};
} // namespace

OHOS::sptr<OHOS::AAFwk::IAbilityManager> GetAbilityManagerInstance()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> abilityObject =
        systemAbilityManager->GetSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AAFwk::IAbilityManager>(abilityObject);
}

NativeValue* JsAppMgrInit(NativeEngine *engine, NativeValue *exportObj)
{
    HILOG_DEBUG("JsAppMgrInit is called");

    if (engine == nullptr || exportObj == nullptr) {
        HILOG_ERROR("engine or exportObj null");
        return nullptr;
    }

    NativeObject *object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_ERROR("object null");
        return nullptr;
    }

    std::unique_ptr<JsAppManager> jsAppManager = std::make_unique<JsAppManager>(GetAbilityManagerInstance());
    object->SetNativePointer(jsAppManager.release(), JsAppManager::Finalizer, nullptr);

    const char *moduleName = "JsAppManager";
    BindNativeFunction(*engine, *object, "killProcessesByBundleName", moduleName,
        JsAppManager::KillProcessesByBundleName);
    HILOG_DEBUG("JsAppMgrInit end");
    return exportObj;
}
}  // namespace AbilityRuntime
}  // namespace OHOS