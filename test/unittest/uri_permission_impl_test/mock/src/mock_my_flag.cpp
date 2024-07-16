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

#include "mock_my_flag.h"

namespace OHOS {
namespace AAFwk {
int MyFlag::flag_ = 0;
bool MyFlag::permissionFileAccessManager_ = false;
bool MyFlag::permissionWriteImageVideo_ = false;
bool MyFlag::permissionReadImageVideo_ = false;
bool MyFlag::permissionAllMedia_ = false;
bool MyFlag::permissionWriteAudio_ = false;
bool MyFlag::permissionReadAudio_ = false;
bool MyFlag::permissionProxyAuthorization_ = false;
bool MyFlag::permissionAll_ = false;
bool MyFlag::permissionPrivileged_ = false;
bool MyFlag::permissionReadWriteDownload_ = false;
bool MyFlag::permissionReadWriteDesktop_ = false;
bool MyFlag::permissionReadWriteDocuments_ = false;
bool MyFlag::IsSystempAppCall_ = false;
TokenInfoMap MyFlag::tokenInfos = {};
} // namespace AAFwk
} // namespace OHOS