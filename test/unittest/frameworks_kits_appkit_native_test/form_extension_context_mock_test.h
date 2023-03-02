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
#ifndef MOCK_OHOS_ABILITY_FORM_MOCK_EXTENSION_H
#define MOCK_OHOS_ABILITY_FORM_MOCK_EXTENSION_H

#include "form_mgr.h"
#include "form_mgr_interface.h"
#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_stub.h>

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

constexpr int32_t STARTABILITYVALUE = 0;

class MockIFormMgr : public IRemoteStub<OHOS::AppExecFwk::IFormMgr> {
public:
    MockIFormMgr() = default;
    ~MockIFormMgr() = default;
    int32_t StartAbility(const Want &want, const sptr<IRemoteObject> &callerToken) override
    {
        return STARTABILITYVALUE;
    }

    int AddForm(
        const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken, FormJsInfo &formInfo) override
    {
        return 0;
    }

    int DeleteForm(const int64_t formId, const sptr<IRemoteObject> &callerToken) override
    {
        return 0;
    }

    int ReleaseForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const bool delCache) override
    {
        return 0;
    }

    int UpdateForm(const int64_t formId, const FormProviderData &formProviderData) override
    {
        return 0;
    }

    int SetNextRefreshTime(const int64_t formId, const int64_t nextTime) override
    {
        return 0;
    }

    ErrCode RequestPublishForm(Want &want, bool withFormBindingData, std::unique_ptr<FormProviderData> &formBindingData,
        int64_t &formId) override
    {
        return 0;
    }

    int LifecycleUpdate(
        const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken, bool updateType) override
    {
        return 0;
    }

    int RequestForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const Want &want) override
    {
        return 0;
    }

    int NotifyWhetherVisibleForms(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken,
        const int32_t formVisibleType) override
    {
        return 0;
    }

    int CastTempForm(const int64_t formId, const sptr<IRemoteObject> &callerToken) override
    {
        return 0;
    }

    int DumpStorageFormInfos(std::string &formInfos) override
    {
        return 0;
    }

    int DumpFormInfoByBundleName(const std::string &bundleName, std::string &formInfos) override
    {
        return 0;
    }

    int DumpFormInfoByFormId(const std::int64_t formId, std::string &formInfo) override
    {
        return 0;
    }

    int DumpFormTimerByFormId(const std::int64_t formId, std::string &isTimingService) override
    {
        return 0;
    }

    int MessageEvent(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken) override
    {
        return 0;
    }

    int RouterEvent(const int64_t formId, Want &want, const sptr<IRemoteObject> &callerToken) override
    {
        return 0;
    }

    int BackgroundEvent(const int64_t formId, Want &want, const sptr<IRemoteObject> &callerToken) override
    {
        return 0;
    }

    int DeleteInvalidForms(
        const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken, int32_t &numFormsDeleted) override
    {
        return 0;
    }

    int AcquireFormState(const Want &want, const sptr<IRemoteObject> &callerToken, FormStateInfo &stateInfo) override
    {
        return 0;
    }

    int NotifyFormsVisible(
        const std::vector<int64_t> &formIds, bool isVisible, const sptr<IRemoteObject> &callerToken) override
    {
        return 0;
    }

    int NotifyFormsPrivacyProtected(
        const std::vector<int64_t> &formIds, bool isProtected, const sptr<IRemoteObject> &callerToken) override
    {
        return 0;
    }

    int NotifyFormsEnableUpdate(
        const std::vector<int64_t> &formIds, bool isEnableUpdate, const sptr<IRemoteObject> &callerToken) override
    {
        return 0;
    }

    int GetAllFormsInfo(std::vector<FormInfo> &formInfos) override
    {
        return 0;
    }

    int GetFormsInfoByApp(std::string &bundleName, std::vector<FormInfo> &formInfos) override
    {
        return 0;
    }

    int GetFormsInfoByModule(
        std::string &bundleName, std::string &moduleName, std::vector<FormInfo> &formInfos) override
    {
        return 0;
    }

    int32_t GetFormsInfo(const FormInfoFilter &filter, std::vector<FormInfo> &formInfos) override
    {
        return 0;
    }

    bool IsRequestPublishFormSupported() override
    {
        return 0;
    }

    int32_t ShareForm(int64_t formId, const std::string &deviceId, const sptr<IRemoteObject> &callerToken,
        int64_t requestCode) override
    {
        return 0;
    }

    int32_t RecvFormShareInfoFromRemote(const FormShareInfo &info) override
    {
        return 0;
    }
    
    bool CheckFMSReady() override
    {
        return false;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_FORM_MOCK_EXTENSION_H