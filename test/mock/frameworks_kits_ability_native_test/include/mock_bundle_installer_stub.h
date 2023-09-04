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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_INSTALLER_STUB_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_INSTALLER_STUB_H

#include "bundle_installer_interface.h"
#include "gmock/gmock.h"

namespace OHOS {
namespace AppExecFwk {
class MockBundleInstallerStub : public IRemoteStub<IBundleInstaller> {
public:
    MockBundleInstallerStub() {};
    virtual ~MockBundleInstallerStub() {};
    MOCK_METHOD3(Install, bool(const std::string &, const InstallParam &, const sptr<IStatusReceiver> &));
    MOCK_METHOD4(OnRemoteRequest, int(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));
    MOCK_METHOD3(Recover, bool(const std::string &, const InstallParam &, const sptr<IStatusReceiver> &));
    MOCK_METHOD3(Install, bool(const std::vector<std::string> &, const InstallParam &, const sptr<IStatusReceiver> &));
    MOCK_METHOD3(Uninstall, bool(const std::string &, const InstallParam &, const sptr<IStatusReceiver> &));
    MOCK_METHOD4(
        Uninstall, bool(const std::string &, const std::string &, const InstallParam &, const sptr<IStatusReceiver> &));
    MOCK_METHOD2(Uninstall, bool(const UninstallParam &, const sptr<IStatusReceiver> &));
    MOCK_METHOD4(InstallSandboxApp, ErrCode(const std::string &, int32_t, int32_t, int32_t &));
    MOCK_METHOD3(UninstallSandboxApp, ErrCode(const std::string &, int32_t, int32_t));
    MOCK_METHOD2(
        CreateStreamInstaller, sptr<IBundleStreamInstaller>(const InstallParam &, const sptr<IStatusReceiver> &));
    MOCK_METHOD1(DestoryBundleStreamInstaller, bool(uint32_t));
    MOCK_METHOD3(
        StreamInstall, ErrCode(const std::vector<std::string> &, const InstallParam &, const sptr<IStatusReceiver> &));
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_INSTALLER_STUB_H