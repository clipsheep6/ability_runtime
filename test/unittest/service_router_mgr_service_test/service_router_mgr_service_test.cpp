/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "service_router_mgr_service.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;
using OHOS::DelayedSingleton;

namespace OHOS {
namespace AbilityRuntime {
class ServiceRouterMgrServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
public:
    static std::shared_ptr<ServiceRouterMgrService> serviceRouterMgrService_;
};
void ServiceRouterMgrServiceTest::SetUpTestCase(void)
{}

void ServiceRouterMgrServiceTest::TearDownTestCase(void)
{
    serviceRouterMgrService_->OnStop();
}

void ServiceRouterMgrServiceTest::SetUp()
{
    serviceRouterMgrService_->OnStart();
}

void ServiceRouterMgrServiceTest::TearDown()
{}

std::shared_ptr<ServiceRouterMgrService> ServiceRouterMgrServiceTest::serviceRouterMgrService_ =
    DelayedSingleton<ServiceRouterMgrService>::GetInstance();

/**
 * @tc.name: test StartUIExtensionAbility_001
 * @tc.desc: StartUIExtensionAbility
 */
HWTEST_F(ServiceRouterMgrServiceTest, StartUIExtensionAbility_001, Function | SmallTest | Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "StartUIExtensionAbility_001 start");
    sptr<SessionInfo> sessionInfo = nullptr;
    int32_t userId = 1;
    auto ret = serviceRouterMgrService_->StartUIExtensionAbility(sessionInfo, userId);
    TAG_LOGI(AAFwkTag::TEST, "StartUIExtensionAbility_001 result %{public}d", ret);
    TAG_LOGI(AAFwkTag::TEST, "StartUIExtensionAbility_001 end");
}

/**
 * @tc.name: test ConnectUIExtensionAbility_001
 * @tc.desc: ConnectUIExtensionAbility
 */
HWTEST_F(ServiceRouterMgrServiceTest, ConnectUIExtensionAbility_001, Function | SmallTest | Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectUIExtensionAbility_001 start");
    Want want;
    sptr<IAbilityConnection> connect = nullptr;
    sptr<SessionInfo> sessionInfo = nullptr;
    int32_t userId = 1;
    auto ret = serviceRouterMgrService_->ConnectUIExtensionAbility(want, connect, sessionInfo, userId);
    TAG_LOGI(AAFwkTag::TEST, "ConnectUIExtensionAbility_001 result %{public}d", ret);
    TAG_LOGI(AAFwkTag::TEST, "ConnectUIExtensionAbility_001 end");
}

/**
 * @tc.name: test QueryPurposeInfos_001
 * @tc.desc: QueryPurposeInfos
 */
HWTEST_F(ServiceRouterMgrServiceTest, QueryPurposeInfos_001, Function | SmallTest | Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryPurposeInfos_001 start");
    Want want;
    std::vector<PurposeInfo> purposeInfos;
    auto ret = serviceRouterMgrService_->QueryPurposeInfos(want, "", purposeInfos);
    TAG_LOGI(AAFwkTag::TEST, "QueryPurposeInfos_001 result %{public}d", ret);
    TAG_LOGI(AAFwkTag::TEST, "QueryPurposeInfos_001 end");
}

/**
 * @tc.name: test QueryBusinessAbilityInfos_001
 * @tc.desc: QueryBusinessAbilityInfos
 */
HWTEST_F(ServiceRouterMgrServiceTest, QueryBusinessAbilityInfos_001, Function | SmallTest | Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryBusinessAbilityInfos_001 start");
    BusinessAbilityFilter filter;
    filter.businessType = BusinessType::UNSPECIFIED;
    std::vector<BusinessAbilityInfo> abilityInfos;
    auto ret = serviceRouterMgrService_->QueryBusinessAbilityInfos(filter, abilityInfos);
    TAG_LOGI(AAFwkTag::TEST, "QueryBusinessAbilityInfos_001 result %{public}d", ret);
    TAG_LOGI(AAFwkTag::TEST, "QueryBusinessAbilityInfos_001 end");
}
}
}

