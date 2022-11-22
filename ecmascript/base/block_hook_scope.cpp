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

#include "ecmascript/base/block_hook_scope.h"

#ifdef BLOCK_HOOK
#include "musl_preinit_common.h"
#endif

namespace panda::ecmascript::base {

BlockHookScope::BlockHookScope()
{
#ifdef BLOCK_HOOK
    __set_hook_flag(false);
#endif
}

BlockHookScope::~BlockHookScope()
{
#ifdef BLOCK_HOOK
    __set_hook_flag(true);
#endif
}
}  // namespace panda::ecmascript::base
