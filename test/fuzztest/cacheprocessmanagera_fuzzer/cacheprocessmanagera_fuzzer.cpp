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

#include "cacheprocessmanagera_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "cache_process_manager.h"
#include "ability_record.h"
#undef protected
#undef private

#include "app_mgr_service_inner.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_THREE = 3;
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
}

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[ENABLE] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

void CacheProcessManagerFuzztestFunc1(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    std::shared_ptr<CacheProcessManager> mgr = std::make_shared<CacheProcessManager>();
    std::shared_ptr<AppMgrServiceInner> serviceInner1;
    mgr->SetAppMgr(serviceInner1); // null mgr
    mgr->RefreshCacheNum(); // called.
    mgr->QueryEnableProcessCache(); // called.
    mgr->maxProcCacheNum_ = 0;
    mgr->PenddingCacheProcess(nullptr); // called.
    mgr->maxProcCacheNum_ = int32Param;

    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<AppRunningRecord> appRecord1 = std::make_shared<AppRunningRecord>(appInfo, int32Param, stringParam);
    mgr->PenddingCacheProcess(nullptr); // nullptr
    appRecord1->isKeepAliveApp_ = true;
    appRecord1->isSingleton_ = true;
    appRecord1->isMainProcess_ = true;
    mgr->PenddingCacheProcess(appRecord1); // keepalive
    std::shared_ptr<AppRunningRecord> appRecord2 = std::make_shared<AppRunningRecord>(appInfo, int32Param, stringParam);
    mgr->PenddingCacheProcess(appRecord2); // not alive

    mgr->maxProcCacheNum_ = 0;
    mgr->CheckAndCacheProcess(nullptr); // nullptr
    mgr->maxProcCacheNum_ = int32Param;
    mgr->CheckAndCacheProcess(appRecord2); // not cached
    mgr->cachedAppRecordQueue_.emplace_back(appRecord2);
    mgr->CheckAndCacheProcess(appRecord2); // cached

    mgr->CheckAndNotifyCachedState(nullptr);
    std::shared_ptr<AppMgrServiceInner> serviceInner = std::make_shared<AppMgrServiceInner>();
    mgr->SetAppMgr(serviceInner);
    mgr->CheckAndNotifyCachedState(appRecord2); // appMgr not null
}

void CacheProcessManagerFuzztestFunc2(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    std::shared_ptr<CacheProcessManager> mgr = std::make_shared<CacheProcessManager>();
    std::shared_ptr<AppMgrServiceInner> serviceInner1;
    mgr->SetAppMgr(serviceInner1);
    mgr->IsCachedProcess(nullptr); // called.
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<AppRunningRecord> appRecord2 = std::make_shared<AppRunningRecord>(appInfo, int32Param, stringParam);
    mgr->IsCachedProcess(appRecord2); // not cached called.
    mgr->cachedAppRecordQueue_.emplace_back(appRecord2);
    mgr->IsCachedProcess(appRecord2); // cached called.
    mgr->cachedAppRecordQueue_.clear(); // clear

    mgr->maxProcCacheNum_ = int32Param;
    mgr->OnProcessKilled(nullptr); // nullptr called.
    mgr->OnProcessKilled(appRecord2); // not cached called.
    mgr->cachedAppRecordQueue_.emplace_back(appRecord2);
    mgr->OnProcessKilled(appRecord2); // cached called.
    mgr->cachedAppRecordQueue_.clear(); // clear

    mgr->maxProcCacheNum_ = int32Param;
    mgr->ReuseCachedProcess(nullptr);
    mgr->ReuseCachedProcess(appRecord2); // not cached
    mgr->cachedAppRecordQueue_.emplace_back(appRecord2);
    mgr->ReuseCachedProcess(appRecord2); // cached
    mgr->cachedAppRecordQueue_.clear(); // clear

    std::shared_ptr<AppMgrServiceInner> serviceInner = std::make_shared<AppMgrServiceInner>();
    mgr->SetAppMgr(serviceInner); // appInner not null
    mgr->cachedAppRecordQueue_.emplace_back(appRecord2);
    mgr->ReuseCachedProcess(appRecord2); // cached

    mgr->IsAppSupportProcessCache(nullptr); // null ptr check
    std::shared_ptr<AppRunningRecord> appRecord3 = std::make_shared<AppRunningRecord>(nullptr, int32Param, stringParam);
    mgr->IsAppSupportProcessCache(appRecord3); // null appInfo
    mgr->srvExtRecords.emplace(appRecord2);
    mgr->IsAppSupportProcessCache(appRecord2); // appInfo not null
    mgr->srvExtRecords.clear();

    appRecord2->SetAttachedToStatusBar(true);
    mgr->IsAppSupportProcessCache(appRecord2); // appInfo not null &attached true
    appRecord2->SetAttachedToStatusBar(false);
    mgr->IsAppSupportProcessCache(appRecord2); // appInfo not null &attached false
    appRecord2->isKeepAliveApp_ = true;
    appRecord2->isSingleton_ = true;
    appRecord2->isMainProcess_ = true;
    mgr->IsAppSupportProcessCache(appRecord2); // appInfo not null &attached false & keepalive called
}

void CacheProcessManagerFuzztestFunc3(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    std::shared_ptr<CacheProcessManager> mgr = std::make_shared<CacheProcessManager>();
    std::shared_ptr<AppMgrServiceInner> serviceInner1;
    mgr->SetAppMgr(serviceInner1);
    mgr->IsAppSupportProcessCacheInnerFirst(nullptr); // nullptr
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<AppRunningRecord> appRecord1 = std::make_shared<AppRunningRecord>(appInfo, int32Param, stringParam);
    mgr->shouldCheckSupport = true;
    appRecord1->procCacheSupportState_ = SupportProcessCacheState::UNSPECIFIED;
    mgr->IsAppSupportProcessCacheInnerFirst(appRecord1); // case CacheState UNSPECIFIED
    appRecord1->procCacheSupportState_ = SupportProcessCacheState::SUPPORT;
    mgr->IsAppSupportProcessCacheInnerFirst(appRecord1); // case CacheState SUPPORT
    appRecord1->procCacheSupportState_ = SupportProcessCacheState::NOT_SUPPORT;
    mgr->IsAppSupportProcessCacheInnerFirst(appRecord1); // case CacheState SUPPORT
    int32_t num = static_cast<int32_t>(SupportProcessCacheState::NOT_SUPPORT) + 1;
    appRecord1->procCacheSupportState_ = static_cast<SupportProcessCacheState>(num);
    mgr->IsAppSupportProcessCacheInnerFirst(appRecord1); // case CacheState default branch

    mgr->shouldCheckSupport = false;
    appRecord1->procCacheSupportState_ = SupportProcessCacheState::UNSPECIFIED;
    mgr->IsAppSupportProcessCacheInnerFirst(appRecord1); // case CacheState UNSPECIFIED

    mgr->IsAppShouldCache(nullptr); // called.
    mgr->maxProcCacheNum_ = int32Param;
    mgr->IsAppShouldCache(appRecord1); // not ccached called.
    mgr->cachedAppRecordQueue_.emplace_back(appRecord1);
    mgr->IsAppShouldCache(appRecord1); //ccached called.

    mgr->IsAppAbilitiesEmpty(nullptr); // called
    mgr->IsAppAbilitiesEmpty(appRecord1); // called
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    bool boolParam = *data % ENABLE;
    std::string stringParam(data, size);
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    CacheProcessManagerFuzztestFunc1(boolParam, stringParam, int32Param);
    CacheProcessManagerFuzztestFunc2(boolParam, stringParam, int32Param);
    CacheProcessManagerFuzztestFunc3(boolParam, stringParam, int32Param);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE || size > OHOS::FOO_MAX_LEN) {
        return 0;
    }

    char* ch = (char*)malloc(size + 1);
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}

