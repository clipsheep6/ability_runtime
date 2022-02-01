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

#ifndef _FORM_ABILITY_B_H_
#define _FORM_ABILITY_B_H_
#include "ability_loader.h"
#include "common_event.h"
#include "common_event_manager.h"

namespace OHOS {
namespace AppExecFwk {
class FormEventSubscriber;
class FormAbilityB : public Ability {
public:
    ~FormAbilityB();
    void SubscribeEvent();
    void handleEvent(std::string action, std::string data);

    void FMS_deleteFormCommon(std::string strFormId);

    void FMS_acquireForm_1400(std::string data);
    void FMS_acquireForm_1500(std::string data);
    void FMS_acquireForm_1800_2(std::string data);
    void FMS_acquireForm_1800_3(std::string data);
    void FMS_acquireForm_2400_1(std::string data);
    void FMS_acquireForm_3100(std::string data);
    void FMS_acquireForm_tempForm(std::string data);
    void FMS_acquireFormBatch(std::string data);
    void FMS_acquireFormTempBatch(std::string data);

    std::shared_ptr<FormEventSubscriber> subscriber_;

    class AcquireFormCallback : public FormCallback {
    public:
        AcquireFormCallback(std::string name, int code):caseName_(name), code_(code)
        {
        }
        virtual ~AcquireFormCallback() = default;
        void OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const override;
        void OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const override;
        void OnFormUninstall(const int64_t formId) const override;

        std::string caseName_ = "";
        int code_ = 0;
        FormAbilityB *ability_ = nullptr;
    };
protected:
    virtual void Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
        const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;
    virtual void OnStart(const Want &want) override;
    virtual void OnStop() override;
    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void OnBackground() override;
private:
    void Clear();

    using FormFunc = void (FormAbilityB::*)(std::string data);
    std::map<std::string, FormFunc> memberFuncMap_;
    std::map<std::string, int> calledFuncMap_;
    std::shared_ptr<AcquireFormCallback> callback_;
};

class FormEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    FormEventSubscriber(const EventFwk::CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {
        ability_ = nullptr;
    }
    ~FormEventSubscriber()
    {
        ability_ = nullptr;
    }
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
    void KitTerminateAbility();

    FormAbilityB *ability_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // _FORM_ABILITY_B_H_