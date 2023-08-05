/*
 * Copyright (c) 2023 iSoftStone Information Technology (Group) Co.,Ltd.
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
#ifndef FRAMEWORKS_CORE_COMMON_TEST_UNITTEST_CLIPBOARD_MOCK_CLIP_BOARD_H
#define FRAMEWORKS_CORE_COMMON_TEST_UNITTEST_CLIPBOARD_MOCK_CLIP_BOARD_H

#include "gtest/gtest.h"

#include "base/utils/utils.h"
#include "core/common/clipboard/clipboard.h"
#include "core/common/clipboard/clipboard_interface.h"
#include "core/common/clipboard/clipboard_proxy.h"

namespace OHOS::Ace {
const std::string TEST = "test";

class MockClipboardImpl : public Clipboard {
public:
    explicit MockClipboardImpl(const RefPtr<TaskExecutor>& taskExecutor);
    ~MockClipboardImpl() override = default;

    void SetData(
        const std::string& data, CopyOptions copyOption = CopyOptions::InApp, bool isDragData = false) override {};
    void GetData(const std::function<void(const std::string&)>& callback, bool syncMode = false) override
    {
        callback(TEST);
    }
    void SetPixelMapData(const RefPtr<PixelMap>& pixmap, CopyOptions copyOption = CopyOptions::InApp) override {};
    void GetPixelMapData(
        const std::function<void(const RefPtr<PixelMap>&)>& callback, bool syncMode = false) override {};
    void HasData(const std::function<void(bool hasData)>& callback) override {};
    void AddPixelMapRecord(const RefPtr<PasteDataMix>& pasteData, const RefPtr<PixelMap>& pixmap) override {};
    void AddImageRecord(const RefPtr<PasteDataMix>& pasteData, const std::string& uri) override {};
    void AddTextRecord(const RefPtr<PasteDataMix>& pasteData, const std::string& selectedStr) override {};
    void SetData(const RefPtr<PasteDataMix>& pasteData, CopyOptions copyOption = CopyOptions::Distributed) override {};
    void GetData(const std::function<void(const std::string&, bool isLastRecord)>& textCallback,
        const std::function<void(const RefPtr<PixelMap>&, bool isLastRecord)>& pixelMapCallback,
        const std::function<void(const std::string&, bool isLastRecord)>& urlCallback,
        bool syncMode = false) override {};

    RefPtr<PasteDataMix> CreatePasteDataMix()
    {
        return AceType::MakeRefPtr<PasteDataMix>();
    }
    void Clear() override {};
};

MockClipboardImpl::MockClipboardImpl(const RefPtr<TaskExecutor>& taskExecutor) : Clipboard(taskExecutor) {}

class MockClipboardProxyImpl final : public ClipboardInterface {
public:
    MockClipboardProxyImpl() = default;
    ~MockClipboardProxyImpl() = default;

    RefPtr<Clipboard> GetClipboard(const RefPtr<TaskExecutor>& taskExecutor) const override
    {
        return AceType::MakeRefPtr<MockClipboardImpl>(taskExecutor);
    }

    ACE_DISALLOW_COPY_AND_MOVE(MockClipboardProxyImpl);
};
} // namespace OHOS::Ace
#endif