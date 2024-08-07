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
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi_common_child_process_param.h"
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
        TAG_LOGI(AAFwkTag::PROCESSMGR, "Called");
        std::unique_ptr<JsChildProcessManager>(static_cast<JsChildProcessManager*>(data));
    }

    static napi_value StartChildProcess(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsChildProcessManager, OnStartChildProcess);
    }

    static napi_value StartArkChildProcess(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsChildProcessManager, OnStartArkChildProcess);
    }

private:
    napi_value OnStartChildProcess(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::PROCESSMGR, "called");
        if (ChildProcessManager::GetInstance().IsChildProcess()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
            return CreateJsUndefined(env);
        }
        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        std::string srcEntry;
        int32_t startMode;
        if (!ConvertFromJsValue(env, argv[PARAM0], srcEntry)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse srcEntry failed");
            ThrowInvalidParamError(env, "Parse param srcEntry failed, must be a valid string.");
            return CreateJsUndefined(env);
        }
        if (!ConvertFromJsValue(env, argv[PARAM1], startMode)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse startMode failed");
            ThrowInvalidParamError(env,
                "Unsupported startMode, must be StartMode.SELF_FORK or StartMode.APP_SPAWN_FORK.");
            return CreateJsUndefined(env);
        }
        TAG_LOGD(AAFwkTag::PROCESSMGR, "StartMode: %{public}d", startMode);
        if (startMode != MODE_SELF_FORK && startMode != MODE_APP_SPAWN_FORK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Invalid StartMode");
            ThrowInvalidParamError(env,
                "Unsupported startMode, must be StartMode.SELF_FORK or StartMode.APP_SPAWN_FORK.");
            return CreateJsUndefined(env);
        }
        napi_value result = nullptr;
        napi_value lastParam = (argc <= ARGC_TWO) ? nullptr : argv[ARGC_TWO];
        if (startMode == MODE_SELF_FORK) {
            StartChildProcessSelfForkTask(env, lastParam, result, srcEntry);
        } else {
            StartChildProcessAppSpawnForkTask(env, lastParam, result, srcEntry);
        }
        return result;
    }

    void StartChildProcessSelfForkTask(const napi_env &env, const napi_value &lastParam, napi_value &result,
        const std::string &srcEntry)
    {
        NapiAsyncTask::CompleteCallback complete = [srcEntry](napi_env env, NapiAsyncTask &task, int32_t status) {
            pid_t pid = 0;
            ChildProcessManagerErrorCode errorCode =
                ChildProcessManager::GetInstance().StartChildProcessBySelfFork(srcEntry, pid);
            if (errorCode == ChildProcessManagerErrorCode::ERR_OK) {
                task.ResolveWithNoError(env, CreateJsValue(env, pid));
            } else {
                task.Reject(env, CreateJsError(env,
                    ChildProcessManagerErrorUtil::GetAbilityErrorCode(errorCode)));
            }
        };
        NapiAsyncTask::Schedule("JsChildProcessManager::OnStartChildProcess",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    }

    void StartChildProcessAppSpawnForkTask(const napi_env &env, const napi_value &lastParam, napi_value &result,
        const std::string &srcEntry)
    {
        auto innerErrorCode = std::make_shared<ChildProcessManagerErrorCode>(ChildProcessManagerErrorCode::ERR_OK);
        auto pid = std::make_shared<pid_t>(ERR_INVALID_VALUE);
        NapiAsyncTask::ExecuteCallback execute = [srcEntry, pid, innerErrorCode]() {
            if (!pid || !innerErrorCode) {
                TAG_LOGE(AAFwkTag::PROCESSMGR, "null innerErrorCode or pid");
                return;
            }
            *innerErrorCode = ChildProcessManager::GetInstance().StartChildProcessByAppSpawnFork(srcEntry, *pid);
        };
        NapiAsyncTask::CompleteCallback complete =
            [pid, innerErrorCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (!pid || !innerErrorCode) {
                TAG_LOGE(AAFwkTag::PROCESSMGR, "null innerErrorCode or pid");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            if (*innerErrorCode == ChildProcessManagerErrorCode::ERR_OK) {
                task.ResolveWithNoError(env, CreateJsValue(env, *pid));
            } else {
                task.Reject(env, CreateJsError(env,
                    ChildProcessManagerErrorUtil::GetAbilityErrorCode(*innerErrorCode)));
            }
        };
        NapiAsyncTask::Schedule("JsChildProcessManager::OnStartChildProcess",
            env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    }

    napi_value OnStartArkChildProcess(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::PROCESSMGR, "called");
        if (ChildProcessManager::GetInstance().IsChildProcess()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
            return CreateJsUndefined(env);
        }
        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Not enough params.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        std::string srcEntry;
        AppExecFwk::ChildProcessArgs args;
        AppExecFwk::ChildProcessOptions options;
        if (!ConvertFromJsValue(env, argv[PARAM0], srcEntry)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse param srcEntry failed, must be a valid string.");
            ThrowInvalidParamError(env, "Parse param srcEntry failed, must be a valid string.");
            return CreateJsUndefined(env);
        }
        if (srcEntry.empty()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Param srcEntry cannot be empty.");
            ThrowInvalidParamError(env, "Param srcEntry cannot be empty.");
            return CreateJsUndefined(env);
        }
        std::string errorMsg;
        if (!UnwrapChildProcessArgs(env, argv[PARAM1], args, errorMsg)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse param args failed.");
            ThrowInvalidParamError(env, errorMsg);
            return CreateJsUndefined(env);
        }
        if (argc > ARGS_TWO && !UnwrapChildProcessOptions(env, argv[PARAM2], options, errorMsg)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse param options failed.");
            ThrowInvalidParamError(env, errorMsg);
            return CreateJsUndefined(env);
        }
        napi_value result = nullptr;
        StartArkChildProcessTask(env, result, srcEntry, args, options);
        return result;
    }

    void StartArkChildProcessTask(const napi_env &env, napi_value &result, const std::string &srcEntry,
        const AppExecFwk::ChildProcessArgs &args, const AppExecFwk::ChildProcessOptions &options)
    {
        TAG_LOGD(AAFwkTag::PROCESSMGR, "OnStartArkChildProcess, srcEntry:%{private}s, args.entryParams:%{private}s,"
            " args.fds size:%{public}zu, options.isolationMode:%{public}d", srcEntry.c_str(),
            args.entryParams.c_str(), args.fds.size(), options.isolationMode);
        auto innerErrorCode = std::make_shared<ChildProcessManagerErrorCode>(ChildProcessManagerErrorCode::ERR_OK);
        auto pid = std::make_shared<pid_t>(0);
        NapiAsyncTask::ExecuteCallback execute = [srcEntry, args, options, pid, innerErrorCode]() {
            if (!pid || !innerErrorCode) {
                TAG_LOGE(AAFwkTag::PROCESSMGR, "innerErrorCode or pid is nullptr");
                return;
            }
            *innerErrorCode = ChildProcessManager::GetInstance().StartArkChildProcess(srcEntry, *pid,
                AppExecFwk::CHILD_PROCESS_TYPE_ARK, args, options);
        };
        NapiAsyncTask::CompleteCallback complete =
            [pid, innerErrorCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (!pid || !innerErrorCode) {
                TAG_LOGE(AAFwkTag::PROCESSMGR, "innerErrorCode or pid is nullptr");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            if (*innerErrorCode == ChildProcessManagerErrorCode::ERR_OK) {
                task.ResolveWithNoError(env, CreateJsValue(env, *pid));
            } else {
                task.Reject(env, CreateJsError(env,
                    ChildProcessManagerErrorUtil::GetAbilityErrorCode(*innerErrorCode)));
            }
        };
        NapiAsyncTask::ScheduleHighQos("JsChildProcessManager::OnStartArkChildProcess",
            env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    }
};

napi_value JsChildProcessManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Invalid input params");
        return nullptr;
    }

    std::unique_ptr<JsChildProcessManager> childProcessManager = std::make_unique<JsChildProcessManager>();
    napi_wrap(env, exportObj, childProcessManager.release(), JsChildProcessManager::Finalizer, nullptr, nullptr);

    const char *moduleName = PROCESS_MANAGER_NAME;
    BindNativeFunction(env, exportObj, "startChildProcess", moduleName, JsChildProcessManager::StartChildProcess);
    BindNativeFunction(env, exportObj, "startArkChildProcess", moduleName, JsChildProcessManager::StartArkChildProcess);
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
