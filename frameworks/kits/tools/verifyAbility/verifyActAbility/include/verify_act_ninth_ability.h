/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef _VERIFY_ACT_NINTH_ABILITY_
#define _VERIFY_ACT_NINTH_ABILITY_
#include "ability_loader.h"
#include "common_event.h"
#include "common_event_manager.h"

namespace OHOS {
namespace AppExecFwk {
//using namespace OHOS::EventFwk;
namespace {
const int ABORT_COMMON_EVENT = 0;
const int GET_CLEAR_ABORT_COMMON_EVENT = 1;
const int SET_CODE_DATA = 2;
const int SET_CODE_AND_DATA = 3;
const int ASYNC_ABORT_COMMON_EVENT = 4;
const int ASYNC_GET_CLEAR_ABORT_COMMON_EVENT = 5;
const int UN_SUBSCRIBE_COMMON_EVENT = 6;
const int ASYNC_SET_CODE_DATA = 7;
}  // namespace
class VerifyActNinthAbility : public Ability {
public:
    virtual void OnStart(const Want &want) override;
    virtual void OnStop() override;
    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void OnBackground() override;
};
class SubscriberNinth : public EventFwk::CommonEventSubscriber, std::enable_shared_from_this<SubscriberNinth> {
public:
    SubscriberNinth(const EventFwk::CommonEventSubscribeInfo &sp) : EventFwk::CommonEventSubscriber(sp){};
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data);

private:
    static void AsyncProcess(const std::shared_ptr<EventFwk::AsyncCommonEventResult> &result, int code);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  //_VERIFY_ACT_NINTH_ABILITY_