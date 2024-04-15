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

#ifndef OHOS_ABILITY_BASE_SKILLUTILS_H
#define OHOS_ABILITY_BASE_SKILLUTILS_H

#include <vector>
#include <string>
#include "want.h"
#include "ability_info.h"
#include "extension_ability_info.h"

#include "nocopyable.h"
#include "access_token.h"
#include "bundle_info.h"
#include "common_event_info.h"
// #include "common_profile.h"
#include "data_group_info.h"
#include "distributed_bundle_info.h"
#include "form_info.h"
#include "hap_module_info.h"
#include "json_util.h"
#include "shortcut_info.h"

namespace OHOS {
namespace AAFwk {

using  SkillUri = OHOS::AppExecFwk::SkillUri;

// class Skills final : public Parcelable {
class SkillUtils {
public:
    /**
     * @brief Default constructor used to create a SkillUtils instance.
     *
     */
    SkillUtils();
    ~SkillUtils();

    /**
     * @brief Match this skill against a Want's data.
     *
     * @param want The desired want data to match for.
     */
    bool Match(const OHOS::AAFwk::Want &want) const;

    /**
     * @brief skills set actions and entities.
     *
     * @param actions Indicates the Skills actions.
     * @param entities Indicates the Skills entities.
     */
    void SetActionsAndEntitiesAndUri(const std::vector<std::string> &tempActions,
        const std::vector<std::string> &tempEntities, const std::vector<OHOS::AppExecFwk::SkillUriForAbility> &tempUris);

private:
    std::vector<std::string> entities;
    std::vector<std::string> actions;
    std::vector<SkillUri> uris;

    static std::multimap<std::string, std::string> mimeTypeMap_;

private:
    bool MatchType(const std::string &type, const std::string &skillUriType) const;
    bool MatchAction(const std::string &action) const;
    bool MatchEntities(const std::vector<std::string> &paramEntities) const;
    bool MatchActionAndEntities(const OHOS::AAFwk::Want &want) const;
    bool MatchUriAndType(const std::string &uriString, const std::string &type) const;
    bool MatchUri(const std::string &uriString, const SkillUri &skillUri) const;
    bool StartsWith(const std::string &sourceString, const std::string &targetPrefix) const;
    bool MatchMimeType(const std::string &uriString) const;
    bool MatchMimeType(const std::string &uriString, size_t &matchUriIndex) const;
    std::string GetOptParamUri(const std::string &uriString) const;

    bool GetMimeTypeByUri(const std::string &uri, std::vector<std::string> &mimeTypes) const;
    bool GetUriSuffix(const std::string &uri, std::string &suffix) const;

};

}
}

#endif