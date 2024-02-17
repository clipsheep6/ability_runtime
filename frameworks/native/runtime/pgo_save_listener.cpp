/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "pgo_save_listener.h"
#include "hilog_wrapper.h"
#include "napi/native_api.h"
#include "ecmascript/napi/include/jsnapi.h"

#include <thread>

namespace OHOS {
namespace AppExecFwk {
PgoSaveListener::PgoSaveListener(const std::string &bundleName, const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo), bundleName_(bundleName)
{}

void PgoSaveListener::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    HILOG_INFO("receive arkruntime.action.SAVE_PGO event");
    auto want = data.GetWant();
    auto element = want.GetElement();
    std::string bundleName = element.GetBundleName();
    if (bundleName != bundleName_) {
        return;
    }
    auto task = []() {
        panda::JSNApi::ForceSave();
    };
    std::thread(task).detach();
}
}  // namespace AppExecFwk
}  // namespace OHOS
