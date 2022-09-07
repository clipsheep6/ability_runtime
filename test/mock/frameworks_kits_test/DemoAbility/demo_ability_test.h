/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_DEMO_ABILITY_TEST_H
#define MOCK_OHOS_ABILITY_RUNTIME_DEMO_ABILITY_TEST_H
#include "ability.h"
#include "ability_loader.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
class DemoAbility : public Ability {
protected:
    void OnStart(const Want &want) override;
    void OnStop() override;
    void OnActive() override;
    void OnInactive() override;
    void OnBackground() override;
    void OnForeground(const Want &want) override;
    void OnNewWant(const Want &want) override;
    sptr<IRemoteObject> OnConnect(const Want &want) override;
    void OnDisconnect(const Want &want) override;
    void OnCommand(const AAFwk::Want &want, bool restart, int startId) override;

    void OnRestoreAbilityState(const PacMap &inState);
    void OnSaveAbilityState(PacMap &outState);
    void OnAbilityResult(int requestCode, int resultCode, const Want &resultData);

    std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter);
    int OpenFile(const Uri &uri, const std::string &mode);
    int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates);
    int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value);
    int Update(
        const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates);
    int OpenRawFile(const Uri &uri, const std::string &mode);
    bool Reload(const Uri &uri, const PacMap &extras);
    int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values);
    std::string GetType(const Uri &uri);
    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_DEMO_ABILITY_TEST_H
