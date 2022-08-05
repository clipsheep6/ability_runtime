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

#include "test_connection_observer.h"

#include <iostream>

#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
void TestConnectionObserver::OnExtensionConnected(const ConnectionData& data)
{
    std::string info = "TestObserver::OnExtensionConnected->" + GenerateString(data);
    HILOG_INFO("%{public}s", info.c_str());
    std::cout<<info<<std::endl;
}

void TestConnectionObserver::OnExtensionDisconnected(const ConnectionData& data)
{
    std::string info = "TestObserver::OnExtensionDisconnected->" + GenerateString(data);
    HILOG_INFO("%{public}s", info.c_str());
    std::cout<<info<<std::endl;
}

void TestConnectionObserver::OnDlpAbilityOpened(const DlpStateData& data)
{
    std::string info = "TestObserver::OnDlpAbilityOpened->" + GenerateString(data);
    HILOG_INFO("%{public}s", info.c_str());
    std::cout<<info<<std::endl;
}

void TestConnectionObserver::OnDlpAbilityClosed(const DlpStateData& data)
{
    std::string info = "TestObserver::OnDlpAbilityClosed->" + GenerateString(data);
    HILOG_INFO("%{public}s", info.c_str());
    std::cout<<info<<std::endl;
}

std::string TestConnectionObserver::GenerateString(const ConnectionData& data)
{
    std::string result = "extPid[" + std::to_string(data.extensionPid) + "]; ";
    result += "extUid[" + std::to_string(data.extensionUid) + "]; ";
    result += "bundleName[" +  data.extensionBundleName + "]; ";
    result += "moduleName[" +  data.extensionModuleName + "]; ";
    result += "name[" +  data.extensionName + "]; ";
    result += "type[" + std::to_string(static_cast<int>(data.extensionType)) + "]; ";
    result += "callerUid[" + std::to_string(data.callerUid) + "]; ";
    result += "callerPid[" + std::to_string(data.callerPid) + "]; ";
    result += "callerName[" +  data.callerName + "]; ";

    return result;
}

std::string TestConnectionObserver::GenerateString(const DlpStateData& data)
{
    std::string result = "targetPid[" + std::to_string(data.targetPid) + "]; ";
    result += "targetUid[" + std::to_string(data.targetUid) + "]; ";
    result += "bundleName[" +  data.targetBundleName + "]; ";
    result += "moduleName[" +  data.targetModuleName + "]; ";
    result += "name[" +  data.targetAbilityName + "]; ";
    result += "dlpUid[" + std::to_string(data.dlpUid) + "]; ";
    result += "dlpPid[" + std::to_string(data.dlpPid) + "]; ";
    result += "dlpBundleName[" +  data.dlpBundleName + "]; ";

    return result;
}
} // namespace AbilityRuntime
} // namespace OHOS
