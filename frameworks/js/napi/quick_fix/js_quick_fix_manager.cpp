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

#include "js_quick_fix_manager.h"

#include "hilog_wrapper.h"
#include "js_application_quick_fix_info.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"
#include "quick_fix_error_utils.h"
#include "quick_fix_manager_client.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
const char *QUICK_FIX_MANAGER_NAME = "JsQuickFixMgr";
} // namespace

class JsQuickFixManager {
public:
    JsQuickFixManager() = default;
    ~JsQuickFixManager() = default;

    static void Finalizer(NativeEngine *engine, void *data, void *hint)
    {
        HILOG_DEBUG("function called.");
        std::unique_ptr<JsQuickFixManager>(static_cast<JsQuickFixManager*>(data));
    }

    static NativeValue *ApplyQuickFix(NativeEngine *engine, NativeCallbackInfo *info)
    {
        JsQuickFixManager *me = CheckParamsAndGetThis<JsQuickFixManager>(engine, info);
        return (me != nullptr) ? me->OnApplyQuickFix(*engine, *info) : nullptr;
    }

    static NativeValue *GetApplyedQuickFixInfo(NativeEngine *engine, NativeCallbackInfo *info)
    {
        JsQuickFixManager *me = CheckParamsAndGetThis<JsQuickFixManager>(engine, info);
        return (me != nullptr) ? me->OnGetApplyedQuickFixInfo(*engine, *info) : nullptr;
    }

    static bool Throw(NativeEngine &engine, int32_t errCode)
    {
        auto externalErrCode = AAFwk::QuickFixErrorUtil::GetErrorCode(errCode);
        auto errMsg = AAFwk::QuickFixErrorUtil::GetErrorMessage(errCode);
        NativeValue *error = engine.CreateError(CreateJsValue(engine, externalErrCode), CreateJsValue(engine, errMsg));
        return engine.Throw(error);
    }

    static NativeValue *CreateJsErrorByErrorCode(NativeEngine &engine, int32_t errCode)
    {
        auto externalErrCode = AAFwk::QuickFixErrorUtil::GetErrorCode(errCode);
        auto errMsg = AAFwk::QuickFixErrorUtil::GetErrorMessage(errCode);
        return CreateJsError(engine, externalErrCode, errMsg);
    }

private:
    NativeValue *OnGetApplyedQuickFixInfo(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_DEBUG("function called.");
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("The number of parameter is invalid.");
            Throw(engine, AAFwk::ERR_QUICKFIX_PARAM_INVALID);
            return engine.CreateUndefined();
        }

        std::string bundleName;
        if (!OHOS::AppExecFwk::UnwrapStringFromJS2(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[0]), bundleName)) {
            HILOG_ERROR("The bundleName is invalid.");
            Throw(engine, AAFwk::ERR_QUICKFIX_PARAM_INVALID);
            return engine.CreateUndefined();
        }

        auto complete = [bundleName](NativeEngine &engine, AsyncTask &task, int32_t status) {
            AppExecFwk::ApplicationQuickFixInfo quickFixInfo;
            auto errCode = DelayedSingleton<AAFwk::QuickFixManagerClient>::GetInstance()->GetApplyedQuickFixInfo(
                bundleName, quickFixInfo);
            if (errCode == 0) {
                task.ResolveWithNoError(engine, CreateJsApplicationQuickFixInfo(engine, quickFixInfo));
            } else {
                task.Reject(engine, CreateJsErrorByErrorCode(engine, errCode));
            }
        };

        NativeValue *lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[1];
        NativeValue *result = nullptr;
        AsyncTask::Schedule("JsQuickFixManager::OnGetApplyedQuickFixInfo", engine,
            CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        HILOG_DEBUG("function finished.");
        return result;
    }

    NativeValue *OnApplyQuickFix(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_DEBUG("function called.");
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("The number of parameter is invalid.");
            Throw(engine, AAFwk::ERR_QUICKFIX_PARAM_INVALID);
            return engine.CreateUndefined();
        }

        std::vector<std::string> hapQuickFixFiles;
        if (!OHOS::AppExecFwk::UnwrapArrayStringFromJS(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[0]), hapQuickFixFiles)) {
            HILOG_ERROR("Hap quick fix files is invalid.");
            Throw(engine, AAFwk::ERR_QUICKFIX_PARAM_INVALID);
            return engine.CreateUndefined();
        }

        auto complete = [hapQuickFixFiles](NativeEngine &engine, AsyncTask &task, int32_t status) {
            auto errcode = DelayedSingleton<AAFwk::QuickFixManagerClient>::GetInstance()->ApplyQuickFix(
                hapQuickFixFiles);
            if (errcode == 0) {
                task.ResolveWithNoError(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByErrorCode(engine, errcode));
            }
        };

        NativeValue *lastParam = (info.argc == 1) ? nullptr : info.argv[1];
        NativeValue *result = nullptr;
        AsyncTask::Schedule("JsQuickFixManager::OnApplyQuickFix", engine,
            CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        HILOG_DEBUG("function finished.");
        return result;
    }
};

NativeValue *CreateJsQuickFixManager(NativeEngine *engine, NativeValue *exportObj)
{
    HILOG_DEBUG("function called.");
    if (engine == nullptr || exportObj == nullptr) {
        HILOG_ERROR("Input parameter is invalid.");
        return nullptr;
    }

    NativeObject *object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr.");
        return nullptr;
    }

    std::unique_ptr<JsQuickFixManager> quickFixManager = std::make_unique<JsQuickFixManager>();
    object->SetNativePointer(quickFixManager.release(), JsQuickFixManager::Finalizer, nullptr);

    BindNativeFunction(*engine, *object, "applyQuickFix", QUICK_FIX_MANAGER_NAME, JsQuickFixManager::ApplyQuickFix);
    BindNativeFunction(*engine, *object, "getApplicationQuickFixInfo", QUICK_FIX_MANAGER_NAME,
        JsQuickFixManager::GetApplyedQuickFixInfo);
    return engine->CreateUndefined();
}
} // namespace AbilityRuntime
} // namespace OHOS
