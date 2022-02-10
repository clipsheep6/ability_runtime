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

#ifndef OHOS_APPEXECFWK_DATASHARE_PROXY_H
#define OHOS_APPEXECFWK_DATASHARE_PROXY_H

#include <iremote_proxy.h>
#include "idatashare.h"

namespace OHOS {
namespace AppExecFwk {
class DataShareProxy : public IRemoteProxy<IDataShare> {
public:
    explicit DataShareProxy(const sptr<IRemoteObject>& remote) : IRemoteProxy<IDataShare>(remote) {}

    virtual ~DataShareProxy() {}

    virtual std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter) override;

    virtual int OpenFile(const Uri &uri, const std::string &mode) override;

    virtual int OpenRawFile(const Uri &uri, const std::string &mode) override;

    virtual int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value) override;

    virtual int Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
        const NativeRdb::DataAbilityPredicates &predicates) override;

    virtual int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates) override;

    virtual std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(const Uri &uri,
        std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates) override;

    virtual std::string GetType(const Uri &uri) override;

    virtual int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values) override;

    virtual bool RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver) override;

    virtual bool UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver) override;

    virtual bool NotifyChange(const Uri &uri) override;

    virtual Uri NormalizeUri(const Uri &uri) override;

    virtual Uri DenormalizeUri(const Uri &uri) override;

    virtual std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations) override;
private:
    static inline BrokerDelegator<DataShareProxy> delegator_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_APPEXECFWK_DATASHARE_PROXY_H

