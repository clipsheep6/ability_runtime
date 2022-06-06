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
#ifndef FOUNDATION_APPEXECFWK_OHOS_MOCK_FORM_MGR_SERVICE_H
#define FOUNDATION_APPEXECFWK_OHOS_MOCK_FORM_MGR_SERVICE_H

#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "form_mgr_stub.h"

namespace OHOS {
namespace AppExecFwk {
class MockFormMgrService : public FormMgrStub {
public:
    MOCK_METHOD4(AddForm, int(const int64_t formId, const Want &want,
        const sptr<IRemoteObject> &callerToken, FormJsInfo &formInfo));
    MOCK_METHOD2(DeleteForm, int(const int64_t formId, const sptr<IRemoteObject> &callerToken));
    MOCK_METHOD3(ReleaseForm, int(const int64_t formId, const sptr<IRemoteObject> &callerToken, const bool delCache));
    MOCK_METHOD2(UpdateForm, int(const int64_t formId, const FormProviderData &formProviderData));
    MOCK_METHOD2(SetNextRefreshTime, int(const int64_t formId, const int64_t nextTime));
    MOCK_METHOD1(AddFormInfo, ErrCode(FormInfo &formInfo));
    MOCK_METHOD2(RemoveFormInfo, ErrCode(const std::string &moduleName, const std::string &formName));
    MOCK_METHOD4(RequestPublishForm, ErrCode(Want &want, bool withFormBindingData,
        std::unique_ptr<FormProviderData> &formBindingData, int64_t &formId));
    MOCK_METHOD3(LifecycleUpdate, int(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken,
        const int32_t updateType));
    MOCK_METHOD3(RequestForm, int(const int64_t formId, const sptr<IRemoteObject> &callerToken, const Want &want));
    MOCK_METHOD3(NotifyWhetherVisibleForms, int(const std::vector<int64_t> &formIds,
        const sptr<IRemoteObject> &callerToken, const int32_t formVisibleType));
    MOCK_METHOD2(CastTempForm, int(const int64_t formId, const sptr<IRemoteObject> &callerToken));
    MOCK_METHOD1(DumpStorageFormInfos, int(std::string &formInfos));
    MOCK_METHOD2(DumpFormInfoByBundleName, int(const std::string &bundleName, std::string &formInfos));
    MOCK_METHOD2(DumpFormInfoByFormId, int(const std::int64_t formId, std::string &formInfo));
    MOCK_METHOD2(DumpFormTimerByFormId, int(const std::int64_t formId, std::string &isTimingService));
    MOCK_METHOD3(MessageEvent, int(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken));
    MOCK_METHOD2(RouterEvent, int(const int64_t formId, Want &want));
    MOCK_METHOD1(BatchAddFormRecords, int(const Want &want));
    MOCK_METHOD0(ClearFormRecords, int());
    MOCK_METHOD1(DistributedDataAddForm, int(const Want &want));
    MOCK_METHOD1(DistributedDataDeleteForm, int(const std::string &formId));
    MOCK_METHOD3(DeleteInvalidForms, int(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken,
        int32_t &numFormsDeleted));
    MOCK_METHOD3(AcquireFormState, int(const Want &want, const sptr<IRemoteObject> &callerToken,
        FormStateInfo &stateInfo));

    MOCK_METHOD3(NotifyFormsVisible, int(const std::vector<int64_t> &formIds, bool isVisible,
        const sptr<IRemoteObject> &callerToken));
    MOCK_METHOD3(NotifyFormsEnableUpdate, int(const std::vector<int64_t> &formIds, bool isEnableUpdate,
        const sptr<IRemoteObject> &callerToken));
    MOCK_METHOD1(GetAllFormsInfo, int(std::vector<FormInfo> &formInfos));
    MOCK_METHOD2(GetFormsInfoByApp, int(std::string &bundleName, std::vector<FormInfo> &formInfos));
    MOCK_METHOD3(GetFormsInfoByModule, int(std::string &bundleName, std::string &moduleName,
        std::vector<FormInfo> &formInfos));
    MOCK_METHOD2(GetFormsInfo, int(const std::string &moduleName, std::vector<FormInfo> &formInfos));
    MOCK_METHOD2(UpdateRouterAction, int(const int64_t formId, std::string &action));
};
}
}
#endif // FOUNDATION_APPEXECFWK_OHOS_MOCK_FORM_MGR_SERVICE_H
