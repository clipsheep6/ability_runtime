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

#ifndef ECMASCRIPT_TOOLING_INTERFACE_DROPFRAME_MANAGER_H
#define ECMASCRIPT_TOOLING_INTERFACE_DROPFRAME_MANAGER_H

#include "ecmascript/js_handle.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/method.h"

namespace panda::ecmascript::tooling {
class DropframeManager {
public:
    DropframeManager() = default;
    ~DropframeManager() = default;
    NO_COPY_SEMANTIC(DropframeManager);
    NO_MOVE_SEMANTIC(DropframeManager);

    void MethodEntry(JSThread *thread, JSHandle<Method> method);
    void MethodExit(JSThread *thread, JSHandle<Method> method);
    void DropLastFrame(JSThread *thread);
private:
    bool IsNewlexenvOpcode(BytecodeInstruction::Opcode op);
    bool IsStlexvarOpcode(BytecodeInstruction::Opcode op);
    std::pair<uint16_t, uint16_t> ReadStlexvarParams(const uint8_t *pc, BytecodeInstruction::Opcode op);

    void NewLexModifyRecordLevel();
    void EmplaceLexModifyRecord(JSThread *thread, JSTaggedValue env, uint16_t slot, JSTaggedValue value);
    uint32_t GetLexModifyRecordLevel();
    std::vector<std::tuple<JSHandle<JSTaggedValue>, uint16_t, JSHandle<JSTaggedValue>>>
        GetLexModifyRecordOfTopFrame();
    void RemoveLexModifyRecordOfTopFrame(JSThread *thread);
    void MergeLexModifyRecordOfTopFrame(JSThread *thread);

    std::stack<std::vector<std::tuple<JSHandle<JSTaggedValue>, uint16_t, JSHandle<JSTaggedValue>>>> modifiedLexVar_;
};
}

#endif  // ECMASCRIPT_TOOLING_INTERFACE_DROPFRAME_MANAGER_H