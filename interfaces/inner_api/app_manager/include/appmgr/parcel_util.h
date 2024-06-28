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

#ifndef OHOS_ABILITY_RUNTIME_PARCEL_UTIL_H
#define OHOS_ABILITY_RUNTIME_PARCEL_UTIL_H

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
#define PARCEL_UTIL_WRITE_NORET(parcel, type, value) \
    do { \
        if (!(parcel).Write##type(value)) { \
            TAG_LOGE(AAFwkTag::APPMGR, "failed to write %{public}s", #value); \
            return; \
        } \
    } while (0)

#define PARCEL_UTIL_WRITE_RET_INT(parcel, type, value) \
    do { \
        if (!(parcel).Write##type(value)) { \
            TAG_LOGE(AAFwkTag::APPMGR, "failed to write %{public}s", #value); \
            return IPC_PROXY_ERR; \
        } \
    } while (0)

#define PARCEL_UTIL_SENDREQ_NORET(code, data, reply, option) \
    do { \
        int32_t ret = SendRequest(code, data, reply, option); \
        if (ret != NO_ERROR) { \
            TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret); \
        } \
    } while (0)

#define PARCEL_UTIL_SENDREQ_RET_INT(code, data, reply, option) \
    do { \
        int32_t ret = SendRequest(code, data, reply, option); \
        if (ret != NO_ERROR) { \
            TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret); \
            return ret; \
        } \
    } while (0)
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PARCEL_UTIL_H
