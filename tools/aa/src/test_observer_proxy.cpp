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

#include "test_observer_proxy.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
TestObserverProxy::TestObserverProxy(const sptr<IRemoteObject>& object) : IRemoteProxy<ITestObserver>(object)
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "test observer proxy instance is created");
}

TestObserverProxy::~TestObserverProxy()
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "test observer proxy is destroyed");
}

void TestObserverProxy::TestStatus(const std::string& msg, const int64_t& resultCode)
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "TestStatus start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }

    if (!data.WriteString(msg)) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Write string msg failed");
        return;
    }

    if (!data.WriteInt64(resultCode)) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Write resultCode failed");
        return;
    }

    int32_t result = SendTransactCmd(
        static_cast<uint32_t>(ITestObserver::Message::AA_TEST_STATUS), data, reply, option);
    if (result != OHOS::NO_ERROR) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "SendRequest failed, error code: %{public}d", result);
        return;
    }
}

void TestObserverProxy::TestFinished(const std::string& msg, const int64_t& resultCode)
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "TestFinished start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }

    if (!data.WriteString(msg)) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Failed to write string msg");
        return;
    }

    if (!data.WriteInt64(resultCode)) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Failed to write resultCode");
        return;
    }

    int32_t result = SendTransactCmd(
        static_cast<uint32_t>(ITestObserver::Message::AA_TEST_FINISHED), data, reply, option);
    if (result != OHOS::NO_ERROR) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Failed to SendRequest, error code: %{public}d", result);
        return;
    }
}

ShellCommandResult TestObserverProxy::ExecuteShellCommand(
    const std::string& cmd, const int64_t timeoutSec)
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "start");

    ShellCommandResult result;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return result;
    }

    if (!data.WriteString(cmd)) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Failed to write string cmd");
        return result;
    }

    if (!data.WriteInt64(timeoutSec)) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Failed to write timeoutSec");
        return result;
    }

    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(ITestObserver::Message::AA_EXECUTE_SHELL_COMMAND), data, reply, option);
    if (ret != OHOS::NO_ERROR) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Failed to SendRequest, error code: %{public}d", ret);
        return result;
    }
    ShellCommandResult* resultPtr = reply.ReadParcelable<ShellCommandResult>();
    if (!resultPtr) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Failed to read result");
        return result;
    }
    result = *resultPtr;
    if (resultPtr != nullptr) {
        delete resultPtr;
    }
    return result;
}

int32_t TestObserverProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    return remote->SendRequest(code, data, reply, option);
}

}  // namespace AAFwk
}  // namespace OHOS
