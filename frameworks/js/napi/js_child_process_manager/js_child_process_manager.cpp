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

#include "js_child_process_manager.h"

#include <unistd.h>

#include "child_process_manager.h"
#include "child_process_manager_error_utils.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *PROCESS_MANAGER_NAME = "JsChildProcessManager";
constexpr size_t ARGC_TWO = 2;

enum {
    MODE_SELF_FORK = 0,
    MODE_APP_SPAWN_FORK = 1,
};
}

class JsChildProcessManager {
public:
    JsChildProcessManager() = default;
    ~JsChildProcessManager() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGI(AAFwkTag::PROCESSMGR, "%{public}s::Finalizer is called", PROCESS_MANAGER_NAME);
        std::unique_ptr<JsChildProcessManager>(static_cast<JsChildProcessManager*>(data));
    }

    static napi_value StartChildProcess(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsChildProcessManager, OnStartChildProcess);
    }

private:
    napi_value OnStartChildProcess(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::PROCESSMGR, "called.");
        if (ChildProcessManager::GetInstance().IsChildProcess()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
            return CreateJsUndefined(env);
        }
        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        std::string srcEntry;
        int32_t startMode;
        if (!ConvertFromJsValue(env, argv[0], srcEntry)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse param srcEntry failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        if (!ConvertFromJsValue(env, argv[1], startMode)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse param startMode failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        TAG_LOGD(AAFwkTag::PROCESSMGR, "StartMode: %{public}d", startMode);
        if (startMode != MODE_SELF_FORK && startMode != MODE_APP_SPAWN_FORK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Not supported StartMode");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        NapiAsyncTask::CompleteCallback complete = [srcEntry, startMode](napi_env env, NapiAsyncTask &task,
                                                                         int32_t status) {
            ForkProcess(env, task, srcEntry, startMode);
        };
        napi_value lastParam = (argc <= ARGC_TWO) ? nullptr : argv[ARGC_TWO];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsChildProcessManager::OnStartChildProcess",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    static void ForkProcess(napi_env env, NapiAsyncTask &task, const std::string &srcEntry, const int32_t startMode)
    {
        TAG_LOGD(AAFwkTag::PROCESSMGR, "called.");
        pid_t pid = 0;
        ChildProcessManagerErrorCode errorCode;
        switch (startMode) {
            case MODE_SELF_FORK: {
                errorCode = ChildProcessManager::GetInstance().StartChildProcessBySelfFork(srcEntry, pid);
                break;
            }
            case MODE_APP_SPAWN_FORK: {
                errorCode = ChildProcessManager::GetInstance().StartChildProcessByAppSpawnFork(srcEntry, pid);
                break;
            }
            default: {
                TAG_LOGE(AAFwkTag::PROCESSMGR, "Not supported StartMode");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
                return;
            }
        }
        TAG_LOGD(
            AAFwkTag::PROCESSMGR, "ChildProcessManager start resultCode: %{public}d, pid:%{public}d", errorCode, pid);
        if (errorCode == ChildProcessManagerErrorCode::ERR_OK) {
            task.ResolveWithNoError(env, CreateJsValue(env, pid));
        } else {
            task.Reject(env, CreateJsError(env, ChildProcessManagerErrorUtil::GetAbilityErrorCode(errorCode)));
        }
    }
};

napi_value JsChildProcessManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "called.");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Invalid input params");
        return nullptr;
    }

    std::unique_ptr<JsChildProcessManager> childProcessManager = std::make_unique<JsChildProcessManager>();
    napi_wrap(env, exportObj, childProcessManager.release(), JsChildProcessManager::Finalizer, nullptr, nullptr);

    const char *moduleName = PROCESS_MANAGER_NAME;
    BindNativeFunction(env, exportObj, "startChildProcess", moduleName, JsChildProcessManager::StartChildProcess);
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
