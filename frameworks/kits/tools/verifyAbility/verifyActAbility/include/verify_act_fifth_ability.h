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

#ifndef _VERIFY_ACT_FIFTH_ABILITY_
#define _VERIFY_ACT_FIFTH_ABILITY_
#include "ability_loader.h"
#include "common_event.h"
#include "ability_lifecycle_observer_interface.h"
#include "common_event_manager.h"
namespace OHOS {
namespace AppExecFwk {
namespace {
const int INSERT = 0;        //"Test Insert()"
const int DELETE = 1;        //"Test Delete()"
const int UPDATE = 2;        //"Test Update()"
const int QUERY = 3;         //"Test Query()"
const int GETFILETYPES = 4;  //"Test GetFileType()"
const int OPENFILE = 5;      //"Test openfile()"
}  // namespace 
class CommentDataAbilityTest : public EventFwk::CommonEventSubscriber {
public:
    CommentDataAbilityTest(const EventFwk::CommonEventSubscribeInfo &sp) : EventFwk::CommonEventSubscriber(sp){};
    virtual void OnReceiveEvent(const  EventFwk::CommonEventData &data) override;

    std::weak_ptr<Context> DataAbility_;
};
class VerifyActFifthAbility : public Ability {
public:
    virtual void OnStart(const Want &want) override;
    virtual void OnStop() override;
    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void OnBackground() override;
    virtual void OnForeground(const Want &want) override;
    std::shared_ptr<CommentDataAbilityTest> subscriberDataAbility = nullptr;
};
class VerifyFifthLifecycleObserver : public ILifecycleObserver {
public:
    VerifyFifthLifecycleObserver() = default;
    virtual ~VerifyFifthLifecycleObserver() = default;
    void OnActive() override;
    void OnBackground() override;
    void OnForeground(const Want &want) override;
    void OnInactive() override;
    void OnStart(const Want &want) override;
    void OnStop() override;
    void OnStateChanged(LifeCycle::Event event, const Want &want) override;
    void OnStateChanged(LifeCycle::Event event) override;
};
class SubscriberFifth : public EventFwk::CommonEventSubscriber {
public:
    SubscriberFifth(const EventFwk::CommonEventSubscribeInfo &sp) : EventFwk::CommonEventSubscriber(sp){};
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data);
    std::shared_ptr<LifeCycle> lifeCycle = nullptr;
    std::shared_ptr<VerifyFifthLifecycleObserver> observer = nullptr;
    Want want;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  //_VERIFY_ACT_FIFTH_ABILITY_