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

#ifndef MOCK_JSNAPI_H
#define MOCK_JSNAPI_H

#include <memory>

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "jsnapi.h"

using RequestAotCallback =
    std::function<int32_t(const std::string &bundleName, const std::string &moduleName, int32_t triggerMode)>;

namespace panda {
class MockJSNApi final {
public:
    MockJSNApi() = default;
    virtual ~MockJSNApi() = default;

    static std::shared_ptr<MockJSNApi> GetInstance()
    {
        if (instance_ == nullptr) {
            instance_ = std::make_shared<MockJSNApi>();
        }
        return instance_;
    }

    void SetRequestAotCallback(const RequestAotCallback& cb)
    {
        requestAotCallback_ = cb;
    }

    int32_t RequestAot(const std::string &bundleName, const std::string &moduleName, int32_t triggerMode)
    {
        if (requestAotCallback_ == nullptr) {
            TAG_LOGE(AAFwkTag::TEST, "callback is invalid.");
            return -1;
        }

        return requestAotCallback_(bundleName, moduleName, triggerMode);
    }

private:
    friend JSNApi;
    static std::shared_ptr<MockJSNApi> instance_;
    RequestAotCallback requestAotCallback_ = nullptr;
};
} // namespace panda
#endif // MOCK_JSNAPI_H
