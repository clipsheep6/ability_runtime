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

#ifndef FOUNDATION_APPEXECFWK_OHOS_ABILITY_DELEGATOR_ARGS_H
#define FOUNDATION_APPEXECFWK_OHOS_ABILITY_DELEGATOR_ARGS_H

#include <map>
#include <string>

namespace OHOS {
namespace AppExecFwk {
class AbilityDelegatorArgs
{
public:
    static const std::string KEY_TEST_RUNNER_CLASS;
    static const std::string KEY_TEST_CASE;

public:
    void SetTestBundleName(const std::string &bundleName);
    std::string GetTestBundleName() const;

    std::string GetTestRunnerClassName() const;
    std::string GetTestCaseName() const;

    void SetTestParam(const std::map<std::string, std::string> &params);
    std::map<std::string, std::string> GetTestParam() const;

private:
    std::string bundleName_;
    std::map<std::string, std::string> params_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_OHOS_ABILITY_DELEGATOR_ARGS_H
