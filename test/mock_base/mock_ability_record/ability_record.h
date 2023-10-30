/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H

#include "ability_info.h"
#include "mock_base_ability_record.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
class AbilityRecord;
class Token : public MockBaseToken, public std::enable_shared_from_this<Token> {
DECLARE_DELAYED_SINGLETON(Token)
public:
    explicit Token(std::weak_ptr<AbilityRecord> abilityRecord);
    virtual ~Token();

    static std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject> &token)
    {
        return DelayedSingleton<Token>::GetInstance()->GetAbilityRecordByToken(token, placeHolder);
    }

    MOCK_METHOD(std::shared_ptr<AbilityRecord>, GetAbilityRecordByToken,
        (const sptr<IRemoteObject> &token, bool placeHolder), (override));
};

class AbilityResult {};
class SystemAbilityCallerRecord {};
class CallerRecord {};

enum AbilityCallType {
    INVALID_TYPE = 0,
    CALL_REQUEST_TYPE,
    START_OPTIONS_TYPE,
    START_SETTINGS_TYPE,
    START_EXTENSION_TYPE,
};

enum CollaboratorType {
    DEFAULT_TYPE = 0,
    RESERVE_TYPE,
    OTHERS_TYPE
};

struct ComponentRequest {
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 0;
    int componentStatus = 0;
    int requestResult = 0;
};

struct AbilityRequest {};

enum ResolveResultType {
    OK_NO_REMOTE_OBJ = 0,
    OK_HAS_REMOTE_OBJ,
    NG_INNER_ERROR,
};

class AbilityRecord : public std::enable_shared_from_this<AbilityRecord> {
public:
    MOCK_METHOD(AppExecFwk::AbilityInfo&, GetAbilityInfo, (), (override));
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H
