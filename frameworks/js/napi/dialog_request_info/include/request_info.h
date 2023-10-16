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

#ifndef OHOS_ABILITY_RUNTIME_REQUEST_INFO_H
#define OHOS_ABILITY_RUNTIME_REQUEST_INFO_H

#include "iremote_object.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
class RequestInfo {
public:
    RequestInfo(const sptr<IRemoteObject> &token, int32_t left, int32_t top, int32_t width, int32_t height);
    ~RequestInfo();

    /**
     * @brief Wrap request info to native value.
     *
     * @param env napi_env.
     * @param request Request information.
     * @return Native value wrapped from request.
     */
    static napi_value WrapRequestInfo(napi_env env, RequestInfo *request);

    /**
     * @brief Unwrap native value to request information.
     *
     * @param env napi_env.
     * @param jsParam napi_value.
     * @return Request information unwrapped from native value.
     */
    static std::shared_ptr<RequestInfo> UnwrapRequestInfo(napi_env env, napi_value jsParam);

    /**
     * @brief Get caller token.
     *
     * @return token.
     */
    sptr<IRemoteObject> GetToken();

    /**
     * @brief Create JsWindowRect.
     *
     * @param env napi_env.
     * @param the left position of WindowRect.
     * @param the top position of WindowRect.
     * @param the width position of WindowRect.
     * @param the height position of WindowRect.
     * @return Native value Created from left, top, width, height.
     */
    static napi_value CreateJsWindowRect(
        napi_env env, int32_t left, int32_t top, int32_t width, int32_t height);
private:
    sptr<IRemoteObject> callerToken_;
    int32_t left_ = 0;
    int32_t top_ = 0;
    int32_t width_ = 0;
    int32_t height_ = 0;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_REQUEST_INFO_H
