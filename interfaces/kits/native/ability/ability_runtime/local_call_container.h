/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_LOCAL_CALL_CONTAINER_H
#define OHOS_ABILITY_RUNTIME_LOCAL_CALL_CONTAINER_H

#include "ability_connect_callback_stub.h"
#include "ability_connect_callback_proxy.h"
#include "local_call_record.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
using Want = OHOS::AAFwk::Want;
using AbilityConnectionStub = OHOS::AAFwk::AbilityConnectionStub;
class CallerConnection;
class LocalCallContainer : public AbilityConnectionStub {
public:
    LocalCallContainer() = default;
    virtual ~LocalCallContainer() = default;

    int StartAbilityByCallInner(
        const Want &want, const std::shared_ptr<CallerCallBack> &callback, const sptr<IRemoteObject> &callerToken);

    int ReleaseCall(const std::shared_ptr<CallerCallBack> &callback);

    void ClearFailedCallConnection(const std::shared_ptr<CallerCallBack> &callback);

    void DumpCalls(std::vector<std::string> &info) const;

    virtual void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int code) override;

    virtual void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int code) override;

    void SetCallLocalRecord(
        const AppExecFwk::ElementName& element, const std::shared_ptr<LocalCallRecord> &localCallRecord);
    void SetMultipleCallLocalRecord(
        const AppExecFwk::ElementName& element, const std::shared_ptr<LocalCallRecord> &localCallRecord);

    void OnCallStubDied(const wptr<IRemoteObject> &remote);
    void OnRemoteStateChanged(const AppExecFwk::ElementName &element, int32_t abilityState) override;
private:
    bool GetCallLocalRecord(
        const AppExecFwk::ElementName &elementName, std::shared_ptr<LocalCallRecord> &localCallRecord);
    void OnSingletonCallStubDied(const wptr<IRemoteObject> &remote);
    int32_t RemoveSingletonCallLocalRecord(const std::string &uri);
    int32_t RemoveMultipleCallLocalRecord(std::shared_ptr<LocalCallRecord> &record);

private:
    // used to store single instance call records
    std::map<std::string, std::shared_ptr<LocalCallRecord>> callProxyRecords_;
    // used to store multi instance call records
    std::map<std::string, std::set<std::shared_ptr<LocalCallRecord>>> multipleCallProxyRecords_;
    std::set<sptr<CallerConnection>> connections_;
};

class CallerConnection : public AbilityConnectionStub {
public:
    CallerConnection() = default;
    virtual ~CallerConnection() = default;

    void SetRecordAndContainer(const std::shared_ptr<LocalCallRecord> &localCallRecord,
        const sptr<IRemoteObject> &container);

    virtual void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int code) override;

    virtual void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int code) override;

private:
    std::shared_ptr<LocalCallRecord> localCallRecord_;
    wptr<LocalCallContainer> container_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_LOCAL_CALL_CONTAINER_H
