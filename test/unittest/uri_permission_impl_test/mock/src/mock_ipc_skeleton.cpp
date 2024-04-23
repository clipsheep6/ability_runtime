/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "mock_ipc_skeleton.h"

namespace OHOS {

uint32_t IPCSkeleton::GetCallingTokenID()
{
    return IPCSkeleton::callerTokenId;
}

uint32_t IPCSkeleton::GetCallingPid()
{
    return IPCSkeleton::callerPId;
}

void IPCSkeleton::SetCallingTokenId(uint32_t tokenId)
{
    IPCSkeleton::callerTokenId = tokenId;
}

void IPCSkeleton::SetCallingPid(uint32_t pid)
{
    IPCSkeleton::callerPId = pid;
}

void IPCSkeleton::ResetTokenId()
{
    IPCSkeleton::callerTokenId = 0;
}
void IPCSkeleton::ResetPId()
{
    IPCSkeleton::callerPId = 0;
}
void IPCSkeleton::Reset()
{
    IPCSkeleton::callerTokenId = 0;
    IPCSkeleton::callerPId = 0;
}

uint32_t IPCSkeleton::callerTokenId = 0;
uint32_t IPCSkeleton::callerPId = 0;
}  // namespace OHOS