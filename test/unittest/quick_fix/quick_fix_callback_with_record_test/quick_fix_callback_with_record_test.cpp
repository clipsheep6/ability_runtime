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

#include <gtest/gtest.h>

#include "quick_fix_callback_with_record.h"
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class QuickFixCallbackImpl : public QuickFixCallbackStub {
public:
    QuickFixCallbackImpl() = default;
    virtual ~QuickFixCallbackImpl() = default;

    void OnLoadPatchDone(int32_t resultCode, [[maybe_unused]] int32_t recordId) override
    {
        HILOG_DEBUG("function called.");
        loadPatchDone_ = true;
        loadPatchResult_ = resultCode;
        loadPatchTimes_++;
    }

    void OnUnloadPatchDone(int32_t resultCode, [[maybe_unused]] int32_t recordId) override
    {
        HILOG_DEBUG("function called.");
        unloadPatchDone_ = true;
        unloadPatchResult_ = resultCode;
        unloadPatchTimes_++;
    }

    void OnReloadPageDone(int32_t resultCode, [[maybe_unused]] int32_t recordId) override
    {
        HILOG_DEBUG("function called.");
        reloadPageDone_ = true;
        reloadPageResult_ = resultCode;
        reloadPageTimes_++;
    }

    bool loadPatchDone_ = false;
    bool unloadPatchDone_ = false;
    bool reloadPageDone_ = false;
    int32_t loadPatchResult_ = 0;
    int32_t unloadPatchResult_ = 0;
    int32_t reloadPageResult_ = 0;
    int32_t loadPatchTimes_ = 0;
    int32_t unloadPatchTimes_ = 0;
    int32_t reloadPageTimes_ = 0;
};

class QuickFixCallbackWithRecordTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void QuickFixCallbackWithRecordTest::SetUpTestCase(void)
{}

void QuickFixCallbackWithRecordTest::TearDownTestCase(void)
{}

void QuickFixCallbackWithRecordTest::SetUp()
{}

void QuickFixCallbackWithRecordTest::TearDown()
{}

/**
 * @tc.name: OnLoadPatchDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI62KJK
 */
HWTEST_F(QuickFixCallbackWithRecordTest, OnLoadPatchDone_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    sptr<QuickFixCallbackImpl> callback = new QuickFixCallbackImpl();
    sptr<QuickFixCallbackWithRecord> callbackByRecord = new QuickFixCallbackWithRecord(callback);
    std::list<int32_t> recordIds{ 0, 1, 2 };
    for (auto it = recordIds.begin(); it != recordIds.end(); it++) {
        callbackByRecord->AddRecordId(*it);
    }

    // after called successful three times, trigger callback
    for (auto it = recordIds.begin(); it != recordIds.end(); it++) {
        callbackByRecord->OnLoadPatchDone(0, *it);
    }

    EXPECT_EQ(callback->loadPatchDone_, true);
    EXPECT_EQ(callback->loadPatchResult_, 0);
    EXPECT_EQ(callback->loadPatchTimes_, 1);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: OnUnloadPatchDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI62KJK
 */
HWTEST_F(QuickFixCallbackWithRecordTest, OnUnloadPatchDone_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    sptr<QuickFixCallbackImpl> callback = new QuickFixCallbackImpl();
    sptr<QuickFixCallbackWithRecord> callbackByRecord = new QuickFixCallbackWithRecord(callback);
    std::list<int32_t> recordIds{ 0 };
    for (auto it = recordIds.begin(); it != recordIds.end(); it++) {
        callbackByRecord->AddRecordId(*it);
    }

    // after called successful three times, trigger callback
    for (auto it = recordIds.begin(); it != recordIds.end(); it++) {
        callbackByRecord->OnUnloadPatchDone(-1, *it);
    }

    EXPECT_EQ(callback->unloadPatchDone_, true);
    EXPECT_EQ(callback->unloadPatchResult_, -1);
    EXPECT_EQ(callback->unloadPatchTimes_, 1);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: OnReloadPageDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI62KJK
 */
HWTEST_F(QuickFixCallbackWithRecordTest, OnReloadPageDone_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    sptr<QuickFixCallbackImpl> callback = new QuickFixCallbackImpl();
    sptr<QuickFixCallbackWithRecord> callbackByRecord = new QuickFixCallbackWithRecord(callback);
    std::list<int32_t> recordIds{ 0, 1 };
    for (auto it = recordIds.begin(); it != recordIds.end(); it++) {
        callbackByRecord->AddRecordId(*it);
    }

    // after called successful three times, trigger callback
    for (auto it = recordIds.begin(); it != recordIds.end(); it++) {
        if (*it == 1) {
            callbackByRecord->OnReloadPageDone(-1, *it);
        } else {
            callbackByRecord->OnReloadPageDone(0, *it);
        }
    }

    EXPECT_EQ(callback->reloadPageDone_, true);
    EXPECT_EQ(callback->reloadPageResult_, -1);
    EXPECT_EQ(callback->reloadPageTimes_, 1);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AddRecordId_0100
 * @tc.desc: basic function test, only one record.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixCallbackWithRecordTest, AddRecordId_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    sptr<QuickFixCallbackImpl> callback = new QuickFixCallbackImpl();
    sptr<QuickFixCallbackWithRecord> callbackByRecord = new QuickFixCallbackWithRecord(callback);
    int32_t recordId = 123456;
    callbackByRecord->AddRecordId(recordId);
    callbackByRecord->OnLoadPatchDone(0, recordId);
    EXPECT_EQ(callback->loadPatchDone_, true);
    EXPECT_EQ(callback->loadPatchResult_, 0);
    EXPECT_EQ(callback->loadPatchTimes_, 1);
    EXPECT_EQ(callback->unloadPatchDone_, false);
    EXPECT_EQ(callback->unloadPatchTimes_, 0);
    EXPECT_EQ(callback->reloadPageDone_, false);
    EXPECT_EQ(callback->reloadPageTimes_, 0);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AddRecordId_0200
 * @tc.desc: basic function test, value of record id is same.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixCallbackWithRecordTest, AddRecordId_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    sptr<QuickFixCallbackImpl> callback = new QuickFixCallbackImpl();
    sptr<QuickFixCallbackWithRecord> callbackByRecord = new QuickFixCallbackWithRecord(callback);
    std::list<int32_t> recordIds{ 1, 1, 1 };
    for (auto it = recordIds.begin(); it != recordIds.end(); it++) {
        callbackByRecord->AddRecordId(*it);
    }

    for (auto it = recordIds.begin(); it != recordIds.end(); it++) {
        callbackByRecord->OnLoadPatchDone(0, *it);
    }
    EXPECT_EQ(callback->loadPatchDone_, true);
    EXPECT_EQ(callback->loadPatchResult_, 0);
    EXPECT_EQ(callback->loadPatchTimes_, 1);
    EXPECT_EQ(callback->unloadPatchDone_, false);
    EXPECT_EQ(callback->unloadPatchTimes_, 0);
    EXPECT_EQ(callback->reloadPageDone_, false);
    EXPECT_EQ(callback->reloadPageTimes_, 0);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: RemoveRecordId_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixCallbackWithRecordTest, RemoveRecordId_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    sptr<QuickFixCallbackImpl> callback = new QuickFixCallbackImpl();
    sptr<QuickFixCallbackWithRecord> callbackByRecord = new QuickFixCallbackWithRecord(callback);
    std::list<int32_t> recordIds{ 0, 1, 2 };
    for (auto it = recordIds.begin(); it != recordIds.end(); it++) {
        callbackByRecord->AddRecordId(*it);
        if (*it == 0) {
            callbackByRecord->RemoveRecordId(*it);
        }
    }

    // recordIds[0] has been removed, recordIds[0] will be ignored, after called once, trigger callback
    for (auto it = recordIds.begin(); it != recordIds.end(); it++) {
        if (*it == 0) {
            callbackByRecord->OnLoadPatchDone(-1, *it);
        } else {
            callbackByRecord->OnLoadPatchDone(0, *it);
        }
    }

    EXPECT_EQ(callback->loadPatchDone_, true);
    EXPECT_EQ(callback->loadPatchResult_, 0);
    EXPECT_EQ(callback->loadPatchTimes_, 1);
    EXPECT_EQ(callback->unloadPatchDone_, false);
    EXPECT_EQ(callback->unloadPatchTimes_, 0);
    EXPECT_EQ(callback->reloadPageDone_, false);
    EXPECT_EQ(callback->reloadPageTimes_, 0);
    HILOG_INFO("%{public}s end.", __func__);
}
} // namespace AppExecFwk
} // namespace OHOS
