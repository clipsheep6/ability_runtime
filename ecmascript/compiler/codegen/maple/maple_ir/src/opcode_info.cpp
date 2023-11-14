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

#include "opcode_info.h"

namespace maple {
OpcodeTable::OpcodeTable()
{
#define STR(s) #s
#define OPCODE(O, P, F, S)       \
    table[OP_##O].flag = F;      \
    table[OP_##O].name = STR(O); \
    table[OP_##O].instrucSize = S;
#include "opcodes.def"
#undef OPCODE
    table[OP_maydassign].name = "dassign";  // maydassign is printed dassign
}
const OpcodeTable kOpcodeInfo;
}  // namespace maple
