/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "test_observer_stub.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
TestObserverStub::TestObserverStub()
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "test observer stub instance is created");
}

TestObserverStub::~TestObserverStub()
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "test observer stub instance is destroyed");
}

int TestObserverStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "local descriptor is not equal to remote");
        return ERR_TRANSACTION_FAILED;
    }
    switch (code) {
        case static_cast<uint32_t>(ITestObserver::Message::AA_TEST_STATUS): {
            std::string msg = data.ReadString();
            int64_t resultCode = data.ReadInt64();
            TestStatus(msg, resultCode);
            break;
        }
        case static_cast<uint32_t>(ITestObserver::Message::AA_TEST_FINISHED): {
            std::string msg = data.ReadString();
            int64_t resultCode = data.ReadInt64();
            TestFinished(msg, resultCode);
            break;
        }
        case static_cast<uint32_t>(ITestObserver::Message::AA_EXECUTE_SHELL_COMMAND): {
            std::string cmd = data.ReadString();
            int64_t timeoutSecs = data.ReadInt64();
            ShellCommandResult result = ExecuteShellCommand(cmd, timeoutSecs);
            if (!reply.WriteParcelable(&result)) {
                TAG_LOGE(AAFwkTag::AA_TOOL, "Failed to write reply ShellCommandResult!");
                return ERR_INVALID_VALUE;
            }
            break;
        }
        default:
            TAG_LOGW(AAFwkTag::AA_TOOL, "event receive stub receives unknown code, code = %{public}u", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS
