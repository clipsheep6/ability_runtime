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

#include "js_my_sa_mgr.h"

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "uri.h"
#include "my_sa_manager_client.h"

namespace OHOS {
namespace AbilityRuntime {
class JsMySaMgr {
public:
    JsMySaMgr() = default;
    ~JsMySaMgr() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsMySaMgr::Finalizer is called");
        std::unique_ptr<JsMySaMgr>(static_cast<JsMySaMgr*>(data));
    }

    static NativeValue* VerifyUriPermission(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsMySaMgr* me = CheckParamsAndGetThis<JsMySaMgr>(engine, info);
        return (me != nullptr) ? me->OnVerifyUriPermission(*engine, *info) : nullptr;
    }

private:
    NativeValue* OnVerifyUriPermission(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_DEBUG("OnVerifyUriPermission is called");
        std::vector<std::shared_ptr<NativeReference>> args;
        for (size_t i = 0; i < info.argc; ++i) {
            args.emplace_back(engine.CreateReference(info.argv[i], 1));
        }

        constexpr int32_t argCountThree = 3;
        AsyncTask::CompleteCallback complete =
        [args, argCountThree](NativeEngine& engine, AsyncTask& task, int32_t status) {
            constexpr int32_t argCountFour = 4;
            if (args.size() != argCountThree && args.size() != argCountFour) {
                HILOG_ERROR("Wrong number of parameters.");
                task.Reject(engine, CreateJsError(engine, -1, "Wrong number of parameters."));
                return;
            }

            std::string uriStr;
            if (!ConvertFromJsValue(engine, args[0]->Get(), uriStr)) {
                HILOG_ERROR("%{public}s called, the first parameter is invalid.", __func__);
                task.Reject(engine, CreateJsError(engine, -1, "uri conversion failed."));
                return;
            }

            int flag = 0;
            if (!ConvertFromJsValue(engine, args[1]->Get(), flag)) {
                HILOG_ERROR("%{public}s called, the second parameter is invalid.", __func__);
                task.Reject(engine, CreateJsError(engine, -1, "flag conversion failed."));
                return;
            }

            int accessTokenId = 0;
            constexpr int32_t index = 2;
            if (!ConvertFromJsValue(engine, args[index]->Get(), accessTokenId)) {
                HILOG_ERROR("%{public}s called, the third parameter is invalid.", __func__);
                task.Reject(engine, CreateJsError(engine, -1, "accessTokenId conversion failed."));
                return;
            }

            Uri uri(uriStr);
            if (AAFwk::MySaManagerClient::GetInstance()->VerifyUriPermission(uri, flag, accessTokenId)) {
                task.Resolve(engine, CreateJsValue(engine, 0));
            } else {
                task.Reject(engine, CreateJsError(engine, -1, "The app doesn't have the uri permission!"));
            }
        };

        NativeValue* lastParam = (info.argc == argCountThree) ? nullptr : info.argv[argCountThree];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        HILOG_DEBUG("OnVerifyUriPermission is called end");
        return result;
    }
};

NativeValue* CreateJsMySaMgr(NativeEngine* engine, NativeValue* exportObj)
{
    HILOG_INFO("CreateJsMySaMgr is called");
    if (engine == nullptr || exportObj == nullptr) {
        HILOG_INFO("Invalid input parameters");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_INFO("object is nullptr");
        return nullptr;
    }

    std::unique_ptr<JsMySaMgr> jsMySaMgr = std::make_unique<JsMySaMgr>();
    object->SetNativePointer(jsMySaMgr.release(), JsMySaMgr::Finalizer, nullptr);

    BindNativeFunction(*engine, *object, "verifyUriPermission", JsMySaMgr::VerifyUriPermission);
    return engine->CreateUndefined();
}
}  // namespace AbilityRuntime
}  // namespace OHOS