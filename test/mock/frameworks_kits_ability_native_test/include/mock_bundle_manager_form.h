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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_FORM_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_FORM_H

#include <vector>

#include "ability_info.h"
#include "application_info.h"
#include "bundle_mgr_interface.h"
#include "gmock/gmock.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
const int32_t ERROR_USER_ID_U256 = 256;
const int32_t USER_ID_U600 = 600;
class BundleMgrProxy : public IRemoteProxy<IBundleMgr> {
public:
    explicit BundleMgrProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IBundleMgr>(impl)
    {}
    virtual ~BundleMgrProxy()
    {}

    std::string GetAppType(const std::string& bundleName) override
    {
        return "system";
    }

    int GetUidByBundleName(const std::string& bundleName, const int userId) override
    {
        if (bundleName.compare("com.form.host.app600") == 0) {
            return USER_ID_U600;
        }
        return 0;
    }

    bool GetBundleNameForUid(const int uid, std::string& bundleName) override
    {
        bundleName = "com.form.provider.service";
        return true;
    }

    bool CheckIsSystemAppByUid(const int uid) override
    {
        if (uid == USER_ID_U600) {
            return true;
        }
        return false;
    }

    bool GetBundleInfo(
        const std::string& bundleName, const BundleFlag flag, BundleInfo& bundleInfo, int32_t userId) override;
    ErrCode GetBaseSharedPackageInfos(const std::string &bundleName,
        int32_t userId, std::vector<BaseSharedPackageInfo> &baseSharedPackageInfos) override
    {
        return ERR_OK;
    }
    bool GetAllFormsInfo(std::vector<FormInfo>& formInfo) override;
    bool GetFormsInfoByApp(const std::string& bundleName, std::vector<FormInfo>& formInfo) override;
    bool GetFormsInfoByModule(
        const std::string& bundleName,
        const std::string& moduleName,
        std::vector<FormInfo>& formInfo) override;
};

class BundleMgrStub : public IRemoteStub<IBundleMgr> {
public:
    ~BundleMgrStub() = default;
    int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
};

class BundleMgrService : public BundleMgrStub {
public:
    ~BundleMgrService() = default;

    std::string GetAppType(const std::string& bundleName) override;
    int GetUidByBundleName(const std::string& bundleName, const int userId) override;
    bool GetBundleInfo(
        const std::string& bundleName, const BundleFlag flag, BundleInfo& bundleInfo, int32_t userId) override;
    bool GetBundleNameForUid(const int uid, std::string& bundleName) override
    {
        bundleName = "com.form.provider.service";
        return true;
    }
    bool CheckIsSystemAppByUid(const int uid) override
    {
        if (uid == USER_ID_U600) {
            return false;
        }

        return true;
    }
    bool GetAllFormsInfo(std::vector<FormInfo>& formInfo) override;
    bool GetFormsInfoByApp(const std::string& bundleName, std::vector<FormInfo>& formInfo) override;
    bool GetFormsInfoByModule(
        const std::string& bundleName,
        const std::string& moduleName,
        std::vector<FormInfo>& formInfo) override;
    ErrCode GetBaseSharedPackageInfos(const std::string &bundleName,
        int32_t userId, std::vector<BaseSharedPackageInfo> &baseSharedPackageInfos) override
    {
        return ERR_OK;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_FORM_H
