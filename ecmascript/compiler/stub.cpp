/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "ecmascript/compiler/stub.h"
#include "ecmascript/compiler/llvm_ir_builder.h"
#include "ecmascript/compiler/stub-inl.h"
#include "ecmascript/js_api_arraylist.h"
#include "ecmascript/js_object.h"
#include "ecmascript/message_string.h"
#include "ecmascript/tagged_dictionary.h"
#include "ecmascript/tagged_hash_table-inl.h"
#include "ecmascript/compiler/rt_call_signature.h"
#include "libpandabase/macros.h"

namespace panda::ecmascript::kungfu {
Stub::Label::Label(Environment *env)
{
    impl_ = env->NewLabel(env);
}

GateRef Stub::Variable::AddPhiOperand(GateRef val)
{
    ASSERT(IsSelector(val));
    Label label = env_->GetLabelFromSelector(val);
    size_t idx = 0;
    for (auto pred : label.GetPredecessors()) {
        auto preVal = pred.ReadVariable(this);
        ASSERT(!env_->GetCircuit()->GetOpCode(preVal).IsNop());
        idx++;
        val = AddOperandToSelector(val, idx, preVal);
    }
    return TryRemoveTrivialPhi(val);
}

GateRef Stub::Variable::AddOperandToSelector(GateRef val, size_t idx, GateRef in)
{
    env_->GetCircuit()->NewIn(val, idx, in);
    return val;
}

GateRef Stub::Variable::TryRemoveTrivialPhi(GateRef phiVal)
{
    Gate *phi = env_->GetCircuit()->LoadGatePtr(phiVal);
    Gate *same = nullptr;
    for (size_t i = 1; i < phi->GetNumIns(); ++i) {
        In *phiIn = phi->GetIn(i);
        Gate *op = (!phiIn->IsGateNull()) ? phiIn->GetGate() : nullptr;
        if (op == same || op == phi) {
            continue;  // unique value or self-reference
        }
        if (same != nullptr) {
            return phiVal;  // the phi merges at least two valusses: not trivial
        }
        same = op;
    }
    if (same == nullptr) {
        // the phi is unreachable or in the start block
        GateType type = env_->GetCircuit()->GetGateType(phiVal);
        same = env_->GetCircuit()->LoadGatePtr(env_->GetCircuitBuilder().UndefineConstant(type));
    }
    auto same_addr_shift = env_->GetCircuit()->SaveGatePtr(same);

    // remove the trivial phi
    // get all users of phi except self
    std::vector<Out *> outs;
    if (!phi->IsFirstOutNull()) {
        Out *phiOut = phi->GetFirstOut();
        while (!phiOut->IsNextOutNull()) {
            if (phiOut->GetGate() != phi) {
                // remove phi
                outs.push_back(phiOut);
            }
            phiOut = phiOut->GetNextOut();
        }
        // save last phi out
        if (phiOut->GetGate() != phi) {
            outs.push_back(phiOut);
        }
    }
    // reroute all outs of phi to same and remove phi
    RerouteOuts(outs, same);
    phi->DeleteGate();

    // try to recursiveby remove all phi users, which might have vecome trivial
    for (auto out : outs) {
        if (IsSelector(out->GetGate())) {
            auto out_addr_shift = env_->GetCircuit()->SaveGatePtr(out->GetGate());
            auto result = TryRemoveTrivialPhi(out_addr_shift);
            if (same_addr_shift == out_addr_shift) {
                same_addr_shift = result;
            }
        }
    }
    return same_addr_shift;
}

void Stub::Variable::RerouteOuts(const std::vector<Out *> &outs, Gate *newGate)
{
    // reroute all outs to new node
    for (auto out : outs) {
        size_t idx = out->GetIndex();
        out->GetGate()->ModifyIn(idx, newGate);
    }
}

void LabelImpl::Seal()
{
    for (auto &[variable, gate] : incompletePhis_) {
        variable->AddPhiOperand(gate);
    }
    isSealed_ = true;
}

void LabelImpl::WriteVariable(Variable *var, GateRef value)
{
    valueMap_[var] = value;
}

GateRef LabelImpl::ReadVariable(Variable *var)
{
    if (valueMap_.find(var) != valueMap_.end()) {
        auto result = valueMap_.at(var);
        if (!env_->GetCircuit()->GetOpCode(result).IsNop()) {
            return result;
        }
    }
    return ReadVariableRecursive(var);
}

GateRef LabelImpl::ReadVariableRecursive(Variable *var)
{
    GateRef val;
    MachineType machineType = CircuitBuilder::GetMachineTypeFromVariableType(var->Type());
    if (!IsSealed()) {
        // only loopheader gate will be not sealed
        int valueCounts = static_cast<int>(predecessors_.size()) + 1;
        if (machineType == MachineType::NOVALUE) {
            val = env_->GetCircuitBuilder().NewSelectorGate(
                OpCode(OpCode::DEPEND_SELECTOR), predeControl_, valueCounts, var->Type());
        } else {
            val = env_->GetCircuitBuilder().NewSelectorGate(
                OpCode(OpCode::VALUE_SELECTOR), machineType, predeControl_, valueCounts, var->Type());
        }
        env_->AddSelectorToLabel(val, Label(this));
        incompletePhis_[var] = val;
    } else if (predecessors_.size() == 1) {
        val = predecessors_[0]->ReadVariable(var);
    } else {
        if (machineType == MachineType::NOVALUE) {
            val = env_->GetCircuitBuilder().NewSelectorGate(
                OpCode(OpCode::DEPEND_SELECTOR), predeControl_, predecessors_.size(), var->Type());
        } else {
            val = env_->GetCircuitBuilder().NewSelectorGate(
                OpCode(OpCode::VALUE_SELECTOR), machineType, predeControl_, predecessors_.size(), var->Type());
        }
        env_->AddSelectorToLabel(val, Label(this));
        WriteVariable(var, val);
        val = var->AddPhiOperand(val);
    }
    WriteVariable(var, val);
    return val;
}

void LabelImpl::Bind()
{
    ASSERT(!predecessors_.empty());
    if (IsLoopHead()) {
        // 2 means input number of depend selector gate
        loopDepend_ = env_->GetCircuitBuilder().NewSelectorGate(OpCode(OpCode::DEPEND_SELECTOR), predeControl_, 2);
        env_->GetCircuit()->NewIn(loopDepend_, 1, predecessors_[0]->GetDepend());
        depend_ = loopDepend_;
    }
    if (IsNeedSeal()) {
        Seal();
        MergeAllControl();
        MergeAllDepend();
    }
}

void LabelImpl::MergeAllControl()
{
    if (predecessors_.size() < 2) {  // 2 : Loop Head only support two predecessors_
        return;
    }

    if (IsLoopHead()) {
        ASSERT(predecessors_.size() == 2);  // 2 : Loop Head only support two predecessors_
        ASSERT(otherPredeControls_.size() == 1);
        env_->GetCircuit()->NewIn(predeControl_, 1, otherPredeControls_[0]);
        return;
    }

    // merge all control of predecessors_
    std::vector<GateRef> inGates(predecessors_.size());
    size_t i = 0;
    ASSERT(predeControl_ != -1);
    ASSERT((otherPredeControls_.size() + 1) == predecessors_.size());
    inGates[i++] = predeControl_;
    for (auto in : otherPredeControls_) {
        inGates[i++] = in;
    }

    GateRef merge = env_->GetCircuitBuilder().NewMerge(inGates.data(), inGates.size());
    predeControl_ = merge;
    control_ = merge;
}

void LabelImpl::MergeAllDepend()
{
    if (IsControlCase()) {
        // Add depend_relay to current label
        auto denpendEntry = Circuit::GetCircuitRoot(OpCode(OpCode::DEPEND_ENTRY));
        dependRelay_ = env_->GetCircuitBuilder().NewDependRelay(predeControl_, denpendEntry);
    }

    if (predecessors_.size() < 2) {  // 2 : Loop Head only support two predecessors_
        depend_ = predecessors_[0]->GetDepend();
        if (dependRelay_ != -1) {
            depend_ = env_->GetCircuitBuilder().NewDependAnd({ depend_, dependRelay_ });
        }
        return;
    }
    if (IsLoopHead()) {
        ASSERT(predecessors_.size() == 2);  // 2 : Loop Head only support two predecessors_
        // Add loop depend to in of depend_seclector
        ASSERT(loopDepend_ != -1);
        // 2 mean 3rd input gate for loopDepend_(depend_selector)
        env_->GetCircuit()->NewIn(loopDepend_, 2, predecessors_[1]->GetDepend());
        return;
    }

    //  Merge all depends to depend_seclector
    std::vector<GateRef> dependsList;
    for (auto prede : GetPredecessors()) {
        dependsList.push_back(prede->GetDepend());
    }
    depend_ = env_->GetCircuitBuilder().NewSelectorGate(OpCode(OpCode::DEPEND_SELECTOR), predeControl_, dependsList,
                                                        dependsList.size());
}

void LabelImpl::AppendPredecessor(LabelImpl *predecessor)
{
    if (predecessor != nullptr) {
        predecessors_.push_back(predecessor);
    }
}

bool LabelImpl::IsNeedSeal() const
{
    auto control = env_->GetCircuit()->LoadGatePtr(predeControl_);
    auto stateCount = control->GetOpCode().GetStateCount(control->GetBitField());
    return predecessors_.size() >= stateCount;
}

bool LabelImpl::IsLoopHead() const
{
    return env_->GetCircuit()->IsLoopHead(predeControl_);
}

bool LabelImpl::IsControlCase() const
{
    return env_->GetCircuit()->IsControlCase(predeControl_);
}

Stub::Environment::Environment(size_t arguments, Circuit *circuit)
    : circuit_(circuit), builder_(circuit), arguments_(arguments)
{
    for (size_t i = 0; i < arguments; i++) {
        arguments_[i] = builder_.NewArguments(i);
    }
    entry_ = Label(NewLabel(this, Circuit::GetCircuitRoot(OpCode(OpCode::STATE_ENTRY))));
    currentLabel_ = &entry_;
    currentLabel_->Seal();
    auto depend_entry = Circuit::GetCircuitRoot(OpCode(OpCode::DEPEND_ENTRY));
    currentLabel_->SetDepend(depend_entry);
}

Stub::Environment::~Environment()
{
    for (auto label : rawLabels_) {
        delete label;
    }
}

void Stub::Jump(Label *label)
{
    ASSERT(label);
    auto currentLabel = env_.GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto jump = env_.GetCircuitBuilder().Goto(currentControl);
    currentLabel->SetControl(jump);
    label->AppendPredecessor(currentLabel);
    label->MergeControl(currentLabel->GetControl());
    env_.SetCurrentLabel(nullptr);
}

void Stub::Branch(GateRef condition, Label *trueLabel, Label *falseLabel)
{
    auto currentLabel = env_.GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    GateRef ifBranch = env_.GetCircuitBuilder().Branch(currentControl, condition);
    currentLabel->SetControl(ifBranch);
    GateRef ifTrue = env_.GetCircuitBuilder().NewIfTrue(ifBranch);
    trueLabel->AppendPredecessor(env_.GetCurrentLabel());
    trueLabel->MergeControl(ifTrue);
    GateRef ifFalse = env_.GetCircuitBuilder().NewIfFalse(ifBranch);
    falseLabel->AppendPredecessor(env_.GetCurrentLabel());
    falseLabel->MergeControl(ifFalse);
    env_.SetCurrentLabel(nullptr);
}

void Stub::Switch(GateRef index, Label *defaultLabel, int64_t *keysValue, Label *keysLabel, int numberOfKeys)
{
    auto currentLabel = env_.GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    GateRef switchBranch = env_.GetCircuitBuilder().SwitchBranch(currentControl, index, numberOfKeys);
    currentLabel->SetControl(switchBranch);
    for (int i = 0; i < numberOfKeys; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        GateRef switchCase = env_.GetCircuitBuilder().NewSwitchCase(switchBranch, keysValue[i]);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        keysLabel[i].AppendPredecessor(currentLabel);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        keysLabel[i].MergeControl(switchCase);
    }

    GateRef defaultCase = env_.GetCircuitBuilder().NewDefaultCase(switchBranch);
    defaultLabel->AppendPredecessor(currentLabel);
    defaultLabel->MergeControl(defaultCase);
    env_.SetCurrentLabel(nullptr);
}

void Stub::LoopBegin(Label *loopHead)
{
    ASSERT(loopHead);
    auto loopControl = env_.GetCircuitBuilder().LoopBegin(loopHead->GetControl());
    loopHead->SetControl(loopControl);
    loopHead->SetPreControl(loopControl);
    loopHead->Bind();
    env_.SetCurrentLabel(loopHead);
}

void Stub::LoopEnd(Label *loopHead)
{
    ASSERT(loopHead);
    auto currentLabel = env_.GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto loopend = env_.GetCircuitBuilder().LoopEnd(currentControl);
    currentLabel->SetControl(loopend);
    loopHead->AppendPredecessor(currentLabel);
    loopHead->MergeControl(loopend);
    loopHead->Seal();
    loopHead->MergeAllControl();
    loopHead->MergeAllDepend();
    env_.SetCurrentLabel(nullptr);
}

// FindElementWithCache in ecmascript/layout_info-inl.h
GateRef Stub::FindElementWithCache(GateRef glue, GateRef layoutInfo, GateRef hClass,
    GateRef key, GateRef propsNum)
{
    auto env = GetEnvironment();
    Label subEntry(env);
    env->PushCurrentLabel(&subEntry);
    DEFVARIABLE(result, VariableType::INT32(), GetInt32Constant(-1));
    DEFVARIABLE(i, VariableType::INT32(), GetInt32Constant(0));
    Label exit(env);
    Label notExceedUpper(env);
    Label exceedUpper(env);
    Label afterExceedCon(env);
    // 9 : Builtins Object properties number is nine
    Branch(Int32LessThanOrEqual(propsNum, GetInt32Constant(9)), &notExceedUpper, &exceedUpper);
    {
        Bind(&notExceedUpper);
            Label loopHead(env);
            Label loopEnd(env);
            Label afterLoop(env);
            Jump(&loopHead);
            LoopBegin(&loopHead);
            {
                Label propsNumIsZero(env);
                Label propsNumNotZero(env);
                Branch(Int32Equal(propsNum, GetInt32Constant(0)), &propsNumIsZero, &propsNumNotZero);
                Bind(&propsNumIsZero);
                Jump(&afterLoop);
                Bind(&propsNumNotZero);
                GateRef elementAddr = GetPropertiesAddrFromLayoutInfo(layoutInfo);
                GateRef keyInProperty = Load(VariableType::INT64(), elementAddr,
                    IntPtrMul(ChangeInt32ToIntPtr(*i),
                        GetIntPtrConstant(sizeof(panda::ecmascript::Properties))));
                Label equal(env);
                Label notEqual(env);
                Label afterEqualCon(env);
                Branch(Int64Equal(keyInProperty, key), &equal, &notEqual);
                Bind(&equal);
                result = *i;
                Jump(&exit);
                Bind(&notEqual);
                Jump(&afterEqualCon);
                Bind(&afterEqualCon);
                i = Int32Add(*i, GetInt32Constant(1));
                Branch(UInt32LessThan(*i, propsNum), &loopEnd, &afterLoop);
                Bind(&loopEnd);
                LoopEnd(&loopHead);
            }
            Bind(&afterLoop);
            result = GetInt32Constant(-1);
            Jump(&exit);
        Bind(&exceedUpper);
        Jump(&afterExceedCon);
    }
    Bind(&afterExceedCon);
    result = TaggedCastToInt32(CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(FindElementWithCache)), {
            hClass, key, IntBuildTaggedTypeWithNoGC(propsNum)
        }));
    Jump(&exit);
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::FindElementFromNumberDictionary(GateRef glue, GateRef elements, GateRef index)
{
    auto env = GetEnvironment();
    Label subentry(env);
    env->PushCurrentLabel(&subentry);
    DEFVARIABLE(result, VariableType::INT32(), GetInt32Constant(-1));
    Label exit(env);
    GateRef capcityoffset =
        IntPtrMul(GetIntPtrConstant(JSTaggedValue::TaggedTypeSize()),
            GetIntPtrConstant(TaggedHashTable<NumberDictionary>::SIZE_INDEX));
    GateRef dataoffset = GetIntPtrConstant(TaggedArray::DATA_OFFSET);
    GateRef capacity = TaggedCastToInt32(Load(VariableType::INT64(), elements,
                                              IntPtrAdd(dataoffset, capcityoffset)));
    DEFVARIABLE(count, VariableType::INT32(), GetInt32Constant(1));
    GateRef len = GetInt32Constant(sizeof(int) / sizeof(uint8_t));
    GateRef hash = CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(GetHash32)),
        { IntBuildTaggedTypeWithNoGC(index), IntBuildTaggedTypeWithNoGC(len) });
    DEFVARIABLE(entry, VariableType::INT32(),
        Int32And(TruncInt64ToInt32(ChangeTaggedPointerToInt64(hash)), Int32Sub(capacity, GetInt32Constant(1))));
    Label loopHead(env);
    Label loopEnd(env);
    Label afterLoop(env);
    Jump(&loopHead);
    LoopBegin(&loopHead);
    GateRef element = GetKeyFromDictionary<NumberDictionary>(VariableType::JS_ANY(), elements, *entry);
    Label isHole(env);
    Label notHole(env);
    Branch(TaggedIsHole(element), &isHole, &notHole);
    Bind(&isHole);
    Jump(&loopEnd);
    Bind(&notHole);
    Label isUndefined(env);
    Label notUndefined(env);
    Branch(TaggedIsUndefined(element), &isUndefined, &notUndefined);
    Bind(&isUndefined);
    result = GetInt32Constant(-1);
    Jump(&exit);
    Bind(&notUndefined);
    Label isMatch(env);
    Label notMatch(env);
    Branch(Int32Equal(index, TaggedCastToInt32(element)), &isMatch, &notMatch);
    Bind(&isMatch);
    result = *entry;
    Jump(&exit);
    Bind(&notMatch);
    Jump(&loopEnd);
    Bind(&loopEnd);
    entry = GetNextPositionForHash(*entry, *count, capacity);
    count = Int32Add(*count, GetInt32Constant(1));
    LoopEnd(&loopHead);
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

// int TaggedHashTable<Derived>::FindEntry(const JSTaggedValue &key) in tagged_hash_table-inl.h
GateRef Stub::FindEntryFromNameDictionary(GateRef glue, GateRef elements, GateRef key)
{
    auto env = GetEnvironment();
    Label funcEntry(env);
    env->PushCurrentLabel(&funcEntry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::INT32(), GetInt32Constant(-1));
    GateRef capcityoffset =
        IntPtrMul(GetIntPtrConstant(JSTaggedValue::TaggedTypeSize()),
            GetIntPtrConstant(TaggedHashTable<NumberDictionary>::SIZE_INDEX));
    GateRef dataoffset = GetIntPtrConstant(TaggedArray::DATA_OFFSET);
    GateRef capacity = TaggedCastToInt32(Load(VariableType::INT64(), elements,
                                              IntPtrAdd(dataoffset, capcityoffset)));
    DEFVARIABLE(count, VariableType::INT32(), GetInt32Constant(1));
    DEFVARIABLE(hash, VariableType::INT32(), GetInt32Constant(0));
    // NameDictionary::hash
    Label isSymbol(env);
    Label notSymbol(env);
    Label loopHead(env);
    Label loopEnd(env);
    Label afterLoop(env);
    Label beforeDefineHash(env);
    Branch(IsSymbol(key), &isSymbol, &notSymbol);
    Bind(&isSymbol);
    {
        hash = TaggedCastToInt32(Load(VariableType::INT64(), key,
            GetIntPtrConstant(JSSymbol::HASHFIELD_OFFSET)));
        Jump(&beforeDefineHash);
    }
    Bind(&notSymbol);
    {
        Label isString(env);
        Label notString(env);
        Branch(IsString(key), &isString, &notString);
        Bind(&isString);
        {
            hash = TruncInt64ToInt32(ChangeTaggedPointerToInt64(CallRuntimeTrampoline(glue,
                GetInt64Constant(RTSTUB_ID(StringGetHashCode)), { key })));
            Jump(&beforeDefineHash);
        }
        Bind(&notString);
        {
            Jump(&beforeDefineHash);
        }
    }
    Bind(&beforeDefineHash);
    // GetFirstPosition(hash, size)
    DEFVARIABLE(entry, VariableType::INT32(), Int32And(*hash, Int32Sub(capacity, GetInt32Constant(1))));
    Jump(&loopHead);
    LoopBegin(&loopHead);
    {
        GateRef element = GetKeyFromDictionary<NameDictionary>(VariableType::JS_ANY(), elements, *entry);
        Label isHole(env);
        Label notHole(env);
        Branch(TaggedIsHole(element), &isHole, &notHole);
        {
            Bind(&isHole);
            {
                Jump(&loopEnd);
            }
            Bind(&notHole);
            {
                Label isUndefined(env);
                Label notUndefined(env);
                Branch(TaggedIsUndefined(element), &isUndefined, &notUndefined);
                {
                    Bind(&isUndefined);
                    {
                        result = GetInt32Constant(-1);
                        Jump(&exit);
                    }
                    Bind(&notUndefined);
                    {
                        Label isMatch(env);
                        Label notMatch(env);
                        Branch(Int64Equal(key, element), &isMatch, &notMatch);
                        {
                            Bind(&isMatch);
                            {
                                result = *entry;
                                Jump(&exit);
                            }
                            Bind(&notMatch);
                            {
                                Jump(&loopEnd);
                            }
                        }
                    }
                }
            }
        }
        Bind(&loopEnd);
        {
            entry = GetNextPositionForHash(*entry, *count, capacity);
            count = Int32Add(*count, GetInt32Constant(1));
            LoopEnd(&loopHead);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::IsMatchInTransitionDictionary(GateRef element, GateRef key, GateRef metaData, GateRef attr)
{
    return TruncInt32ToInt1(Int32And(ZExtInt1ToInt32(Int64Equal(element, key)),
        ZExtInt1ToInt32(Int32Equal(metaData, attr))));
}

// metaData is int32 type
GateRef Stub::FindEntryFromTransitionDictionary(GateRef glue, GateRef elements, GateRef key, GateRef metaData)
{
    auto env = GetEnvironment();
    Label funcEntry(env);
    env->PushCurrentLabel(&funcEntry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::INT32(), GetInt32Constant(-1));
    GateRef capcityoffset =
        IntPtrMul(GetIntPtrConstant(JSTaggedValue::TaggedTypeSize()),
            GetIntPtrConstant(TaggedHashTable<NumberDictionary>::SIZE_INDEX));
    GateRef dataoffset = GetIntPtrConstant(TaggedArray::DATA_OFFSET);
    GateRef capacity = TaggedCastToInt32(Load(VariableType::INT64(), elements,
                                              IntPtrAdd(dataoffset, capcityoffset)));
    DEFVARIABLE(count, VariableType::INT32(), GetInt32Constant(1));
    DEFVARIABLE(hash, VariableType::INT32(), GetInt32Constant(0));
    // TransitionDictionary::hash
    Label isSymbol(env);
    Label notSymbol(env);
    Label loopHead(env);
    Label loopEnd(env);
    Label afterLoop(env);
    Label beforeDefineHash(env);
    Branch(IsSymbol(key), &isSymbol, &notSymbol);
    Bind(&isSymbol);
    {
        hash = TaggedCastToInt32(Load(VariableType::INT64(), key,
            GetIntPtrConstant(panda::ecmascript::JSSymbol::HASHFIELD_OFFSET)));
        Jump(&beforeDefineHash);
    }
    Bind(&notSymbol);
    {
        Label isString(env);
        Label notString(env);
        Branch(IsString(key), &isString, &notString);
        Bind(&isString);
        {
            hash = TruncInt64ToInt32(ChangeTaggedPointerToInt64(CallRuntimeTrampoline(glue,
                GetInt64Constant(RTSTUB_ID(StringGetHashCode)), { key })));
            Jump(&beforeDefineHash);
        }
        Bind(&notString);
        {
            Jump(&beforeDefineHash);
        }
    }
    Bind(&beforeDefineHash);
    hash = Int32Add(*hash, metaData);
    // GetFirstPosition(hash, size)
    DEFVARIABLE(entry, VariableType::INT32(), Int32And(*hash, Int32Sub(capacity, GetInt32Constant(1))));
    Jump(&loopHead);
    LoopBegin(&loopHead);
    {
        GateRef element = GetKeyFromDictionary<TransitionsDictionary>(VariableType::JS_ANY(), elements, *entry);
        Label isHole(env);
        Label notHole(env);
        Branch(TaggedIsHole(element), &isHole, &notHole);
        {
            Bind(&isHole);
            {
                Jump(&loopEnd);
            }
            Bind(&notHole);
            {
                Label isUndefined(env);
                Label notUndefined(env);
                Branch(TaggedIsUndefined(element), &isUndefined, &notUndefined);
                {
                    Bind(&isUndefined);
                    {
                        result = GetInt32Constant(-1);
                        Jump(&exit);
                    }
                    Bind(&notUndefined);
                    {
                        Label isMatch(env);
                        Label notMatch(env);
                        Branch(
                            IsMatchInTransitionDictionary(element, key, metaData,
                                GetAttributesFromDictionary<TransitionsDictionary>(elements, *entry)),
                            &isMatch, &notMatch);
                        {
                            Bind(&isMatch);
                            {
                                result = *entry;
                                Jump(&exit);
                            }
                            Bind(&notMatch);
                            {
                                Jump(&loopEnd);
                            }
                        }
                    }
                }
            }
        }
        Bind(&loopEnd);
        {
            entry = GetNextPositionForHash(*entry, *count, capacity);
            count = Int32Add(*count, GetInt32Constant(1));
            LoopEnd(&loopHead);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::JSObjectGetProperty(VariableType returnType, GateRef obj, GateRef hClass, GateRef attr)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    DEFVARIABLE(result, returnType, GetUndefinedConstant(returnType));
    Label inlinedProp(env);
    Label notInlinedProp(env);
    GateRef attrOffset = GetOffsetFieldInPropAttr(attr);
    Branch(IsInlinedProperty(attr), &inlinedProp, &notInlinedProp);
    {
        Bind(&inlinedProp);
        {
            // GetPropertyInlinedProps
            GateRef inlinedPropsStart = GetInlinedPropsStartFromHClass(hClass);
            GateRef propOffset = Int32Mul(
                Int32Add(inlinedPropsStart, attrOffset),
                GetInt32Constant(JSTaggedValue::TaggedTypeSize()));
            result = Load(returnType, obj, ZExtInt32ToInt64(propOffset));
            Jump(&exit);
        }
        Bind(&notInlinedProp);
        {
            // compute outOfLineProp offset, get it and return
            GateRef array =
                Load(VariableType::INT64(), obj, GetIntPtrConstant(JSObject::PROPERTIES_OFFSET));
            result = GetValueFromTaggedArray(returnType, array, Int32Sub(attrOffset,
                GetInlinedPropertiesFromHClass(hClass)));
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

void Stub::JSObjectSetProperty(GateRef glue, GateRef obj, GateRef hClass, GateRef attr, GateRef value)
{
    auto env = GetEnvironment();
    Label subEntry(env);
    env->PushCurrentLabel(&subEntry);
    Label exit(env);
    Label inlinedProp(env);
    Label notInlinedProp(env);
    GateRef attrOffset = GetOffsetFieldInPropAttr(attr);
    Branch(IsInlinedProperty(attr), &inlinedProp, &notInlinedProp);
    {
        Bind(&inlinedProp);
        {
            SetPropertyInlinedProps(glue, obj, hClass, value, attrOffset);
            Jump(&exit);
        }
        Bind(&notInlinedProp);
        {
            // compute outOfLineProp offset, get it and return
            GateRef array = Load(VariableType::JS_POINTER(), obj,
                                 GetIntPtrConstant(JSObject::PROPERTIES_OFFSET));
            SetValueToTaggedArray(VariableType::JS_ANY(), glue, array, Int32Sub(attrOffset,
                GetInlinedPropertiesFromHClass(hClass)), value);
            Jump(&exit);
        }
    }
    Bind(&exit);
    env->PopCurrentLabel();
    return;
}

GateRef Stub::ComputePropertyCapacityInJSObj(GateRef oldLength)
{
    auto env = GetEnvironment();
    Label subEntry(env);
    env->PushCurrentLabel(&subEntry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::INT32(), GetInt32Constant(0));
    GateRef newL = Int32Add(oldLength, GetInt32Constant(JSObject::PROPERTIES_GROW_SIZE));
    Label reachMax(env);
    Label notReachMax(env);
    Branch(Int32GreaterThan(newL, GetInt32Constant(PropertyAttributes::MAX_CAPACITY_OF_PROPERTIES)),
        &reachMax, &notReachMax);
    {
        Bind(&reachMax);
        result = GetInt32Constant(PropertyAttributes::MAX_CAPACITY_OF_PROPERTIES);
        Jump(&exit);
        Bind(&notReachMax);
        result = newL;
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::CallSetterUtil(GateRef glue, GateRef holder, GateRef accessor, GateRef value)
{
    auto env = GetEnvironment();
    Label subEntry(env);
    env->PushCurrentLabel(&subEntry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::INT64(), GetUndefinedConstant(VariableType::INT64()));
    GateRef callRes = CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(CallSetter)), {
            accessor, holder, value, TaggedTrue()
        });
    Label callSuccess(env);
    Label callFail(env);
    Branch(TaggedIsTrue(callRes), &callSuccess, &callFail);
    {
        Bind(&callSuccess);
        result = GetUndefinedConstant(VariableType::INT64());
        Jump(&exit);
        Bind(&callFail);
        result = GetExceptionConstant(VariableType::INT64());
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::ShouldCallSetter(GateRef receiver, GateRef holder, GateRef accessor, GateRef attr)
{
    auto env = GetEnvironment();
    Label subEntry(env);
    env->PushCurrentLabel(&subEntry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::BOOL(), TrueConstant());
    Label isInternal(env);
    Label notInternal(env);
    Branch(IsAccessorInternal(accessor), &isInternal, &notInternal);
    Bind(&isInternal);
    {
        Label receiverEqualsHolder(env);
        Label receiverNotEqualsHolder(env);
        Branch(Int64Equal(receiver, holder), &receiverEqualsHolder, &receiverNotEqualsHolder);
        Bind(&receiverEqualsHolder);
        {
            result = IsWritable(attr);
            Jump(&exit);
        }
        Bind(&receiverNotEqualsHolder);
        {
            result = FalseConstant();
            Jump(&exit);
        }
    }
    Bind(&notInternal);
    {
        result = TrueConstant();
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

void Stub::JSHClassAddProperty(GateRef glue, GateRef receiver, GateRef key, GateRef attr)
{
    auto env = GetEnvironment();
    Label subEntry(env);
    env->PushCurrentLabel(&subEntry);
    Label exit(env);
    GateRef hclass = LoadHClass(receiver);
    GateRef metaData = GetPropertyMetaDataFromAttr(attr);
    GateRef newDyn = FindTransitions(glue, receiver, hclass, key, metaData);
    Label findHClass(env);
    Label notFindHClass(env);
    Branch(Int64Equal(newDyn, GetInt64Constant(JSTaggedValue::VALUE_UNDEFINED)), &notFindHClass, &findHClass);
    Bind(&findHClass);
    {
        Jump(&exit);
    }
    Bind(&notFindHClass);
    {
        GateRef type = GetObjectType(hclass);
        GateRef size = Int32Mul(GetInlinedPropsStartFromHClass(hclass),
                                GetInt32Constant(JSTaggedValue::TaggedTypeSize()));
        GateRef inlineProps = GetInlinedPropertiesFromHClass(hclass);
        GateRef newJshclass = CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(NewEcmaDynClass)), {
                IntBuildTaggedTypeWithNoGC(size), IntBuildTaggedTypeWithNoGC(type),
                IntBuildTaggedTypeWithNoGC(inlineProps)
            });
        CopyAllHClass(glue, newJshclass, hclass);
        CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(UpdateLayOutAndAddTransition)), {
                              hclass, newJshclass, key, IntBuildTaggedTypeWithNoGC(attr) });
#if ECMASCRIPT_ENABLE_IC
        NotifyHClassChanged(glue, hclass, newJshclass);
#endif
        StoreHClass(glue, receiver, newJshclass);
        Jump(&exit);
    }
    Bind(&exit);
    env->PopCurrentLabel();
    return;
}

// if condition:objHandle->IsJSArray() &&
//      keyHandle.GetTaggedValue() == thread->GlobalConstants()->GetConstructorString()
GateRef Stub::SetHasConstructorCondition(GateRef glue, GateRef receiver, GateRef key)
{
    GateRef gConstOffset = IntPtrAdd(glue,
                                     GetIntPtrConstant(JSThread::GlueData::GetGlobalConstOffset(env_.Is32Bit())));
    GateRef gCtorStr = Load(VariableType::JS_ANY(),
        gConstOffset,
        Int64Mul(GetInt64Constant(sizeof(JSTaggedValue)),
            GetInt64Constant(static_cast<uint64_t>(ConstantIndex::CONSTRUCTOR_STRING_INDEX))));
    GateRef isCtorStr = Int64Equal(key, gCtorStr);
    return Int32NotEqual(
        Int32And(SExtInt1ToInt32(IsJsArray(receiver)), SExtInt1ToInt32(isCtorStr)), GetInt32Constant(0));
}

// Note: set return exit node
GateRef Stub::AddPropertyByName(GateRef glue, GateRef receiver, GateRef key, GateRef value,
                                GateRef propertyAttributes)
{
    auto env = GetEnvironment();
    Label subentry(env);
    env->PushCurrentLabel(&subentry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::INT64(), GetUndefinedConstant(VariableType::INT64()));
    Label setHasCtor(env);
    Label notSetHasCtor(env);
    Label afterCtorCon(env);
    GateRef hClass = LoadHClass(receiver);
    Branch(SetHasConstructorCondition(glue, receiver, key), &setHasCtor, &notSetHasCtor);
    {
        Bind(&setHasCtor);
        SetHasConstructorToHClass(glue, hClass, GetInt32Constant(1));
        Jump(&afterCtorCon);
        Bind(&notSetHasCtor);
        Jump(&afterCtorCon);
    }
    Bind(&afterCtorCon);
    // 0x111 : default attribute for property: writable, enumerable, configurable
    DEFVARIABLE(attr, VariableType::INT32(), propertyAttributes);
    GateRef numberOfProps = GetNumberOfPropsFromHClass(hClass);
    GateRef inlinedProperties = GetInlinedPropertiesFromHClass(hClass);
    Label hasUnusedInProps(env);
    Label noUnusedInProps(env);
    Label afterInPropsCon(env);
    Branch(UInt32LessThan(numberOfProps, inlinedProperties), &hasUnusedInProps, &noUnusedInProps);
    {
        Bind(&noUnusedInProps);
        Jump(&afterInPropsCon);
        Bind(&hasUnusedInProps);
        {
            SetPropertyInlinedProps(glue, receiver, hClass, value, numberOfProps);
            attr = SetOffsetFieldInPropAttr(*attr, numberOfProps);
            attr = SetIsInlinePropsFieldInPropAttr(*attr, GetInt32Constant(1)); // 1: set inInlineProps true
            JSHClassAddProperty(glue, receiver, key, *attr);
            result = GetUndefinedConstant(VariableType::INT64());
            Jump(&exit);
        }
    }
    Bind(&afterInPropsCon);
    DEFVARIABLE(array, VariableType::JS_POINTER(), GetPropertiesArray(receiver));
    DEFVARIABLE(length, VariableType::INT32(), GetLengthOfTaggedArray(*array));
    Label lenIsZero(env);
    Label lenNotZero(env);
    Label afterLenCon(env);
    Branch(Int32Equal(*length, GetInt32Constant(0)), &lenIsZero, &lenNotZero);
    {
        Bind(&lenIsZero);
        {
            length = GetInt32Constant(JSObject::MIN_PROPERTIES_LENGTH);
            array = CallRuntimeTrampoline(glue,
                GetInt64Constant(RTSTUB_ID(NewTaggedArray)), { IntBuildTaggedTypeWithNoGC(*length) });
            SetPropertiesArray(glue, receiver, *array);
            Jump(&afterLenCon);
        }
        Bind(&lenNotZero);
        Jump(&afterLenCon);
    }
    Bind(&afterLenCon);
    Label isDictMode(env);
    Label notDictMode(env);
    Branch(IsDictionaryMode(*array), &isDictMode, &notDictMode);
    {
        Bind(&isDictMode);
        {
            GateRef res = CallRuntimeTrampoline(glue,
                GetInt64Constant(RTSTUB_ID(NameDictPutIfAbsent)), {
                    receiver, *array, key, value, IntBuildTaggedTypeWithNoGC(*attr), TaggedFalse()
                });
            SetPropertiesArray(glue, receiver, res);
            Jump(&exit);
        }
        Bind(&notDictMode);
        {
            attr = SetIsInlinePropsFieldInPropAttr(*attr, GetInt32Constant(0));
            GateRef outProps = Int32Sub(numberOfProps, inlinedProperties);
            Label isArrayFull(env);
            Label arrayNotFull(env);
            Label afterArrLenCon(env);
            Branch(Int32Equal(*length, outProps), &isArrayFull, &arrayNotFull);
            {
                Bind(&isArrayFull);
                {
                    Label ChangeToDict(env);
                    Label notChangeToDict(env);
                    Label afterDictChangeCon(env);
                    Branch(Int32Equal(*length, GetInt32Constant(JSHClass::MAX_CAPACITY_OF_OUT_OBJECTS)),
                        &ChangeToDict, &notChangeToDict);
                    {
                        Bind(&ChangeToDict);
                        {
                            attr = SetDictionaryOrderFieldInPropAttr(*attr,
                                GetInt32Constant(PropertyAttributes::MAX_CAPACITY_OF_PROPERTIES));
                            GateRef res = CallRuntimeTrampoline(glue,
                                GetInt64Constant(RTSTUB_ID(NameDictPutIfAbsent)), {
                                    receiver, *array, key, value, IntBuildTaggedTypeWithNoGC(*attr), TaggedTrue()
                                });
                            SetPropertiesArray(glue, receiver, res);
                            result = GetUndefinedConstant(VariableType::INT64());
                            Jump(&exit);
                        }
                        Bind(&notChangeToDict);
                        Jump(&afterDictChangeCon);
                    }
                    Bind(&afterDictChangeCon);
                    GateRef capacity = ComputePropertyCapacityInJSObj(*length);
                    array = CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(CopyArray)),
                        { *array, IntBuildTaggedTypeWithNoGC(*length), IntBuildTaggedTypeWithNoGC(capacity) });
                    SetPropertiesArray(glue, receiver, *array);
                    Jump(&afterArrLenCon);
                }
                Bind(&arrayNotFull);
                Jump(&afterArrLenCon);
            }
            Bind(&afterArrLenCon);
            {
                attr = SetOffsetFieldInPropAttr(*attr, numberOfProps);
                JSHClassAddProperty(glue, receiver, key, *attr);
                SetValueToTaggedArray(VariableType::JS_ANY(), glue, *array, outProps, value);
                Jump(&exit);
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

void Stub::ThrowTypeAndReturn(GateRef glue, int messageId, GateRef val)
{
    GateRef msgIntId = GetInt32Constant(messageId);
    CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(ThrowTypeError)),
        { IntBuildTaggedTypeWithNoGC(msgIntId) });
    Return(val);
}

GateRef Stub::TaggedToRepresentation(GateRef value)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    DEFVARIABLE(resultRep, VariableType::INT64(),
                GetInt64Constant(static_cast<int32_t>(Representation::OBJECT)));
    Label isInt(env);
    Label notInt(env);

    Branch(TaggedIsInt(value), &isInt, &notInt);
    Bind(&isInt);
    {
        resultRep = GetInt64Constant(static_cast<int32_t>(Representation::INT));
        Jump(&exit);
    }
    Bind(&notInt);
    {
        Label isDouble(env);
        Label notDouble(env);
        Branch(TaggedIsDouble(value), &isDouble, &notDouble);
        Bind(&isDouble);
        {
            resultRep = GetInt64Constant(static_cast<int32_t>(Representation::DOUBLE));
            Jump(&exit);
        }
        Bind(&notDouble);
        {
            resultRep = GetInt64Constant(static_cast<int32_t>(Representation::OBJECT));
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *resultRep;
    env->PopCurrentLabel();
    return ret;
}

void Stub::Store(VariableType type, GateRef glue, GateRef base, GateRef offset, GateRef value)
{
    auto depend = env_.GetCurrentLabel()->GetDepend();
    GateRef result;
    if (env_.IsArch64Bit()) {
        GateRef ptr = Int64Add(base, offset);
        if (type == VariableType::POINTER()) {
            type = VariableType::INT64();
        }
        result = env_.GetCircuitBuilder().NewStoreGate(type, ptr, value, depend);
        env_.GetCurrentLabel()->SetDepend(result);
    } else if (env_.IsArch32Bit()) {
        if (type == VariableType::POINTER()) {
            type = VariableType::INT32();
        }
        GateRef ptr = Int32Add(base, offset);
        result = env_.GetCircuitBuilder().NewStoreGate(type, ptr, value, depend);
        env_.GetCurrentLabel()->SetDepend(result);
    } else {
        UNREACHABLE();
    }
    if (type == VariableType::JS_POINTER() || type == VariableType::JS_ANY()) {
        SetValueWithBarrier(glue, base, offset, value);
    }
    return;
}

void Stub::SetValueWithBarrier(GateRef glue, GateRef obj, GateRef offset, GateRef value)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    Label isHeapObject(env);
    Label isVailedIndex(env);
    Label notValidIndex(env);
    Label marking(env);

    Branch(TaggedIsHeapObject(value), &isHeapObject, &exit);
    Bind(&isHeapObject);
    {
        GateRef objectRegion = ObjectAddressToRange(obj);
        GateRef valueRegion = ObjectAddressToRange(value);
        GateRef slotAddr = IntPtrAdd(TaggedCastToIntPtr(obj), offset);
        GateRef objectNotInYoung = BoolNot(InYoungGeneration(glue, objectRegion));
        GateRef valueRegionInYoung = InYoungGeneration(glue, valueRegion);
        Branch(BoolAnd(objectNotInYoung, valueRegionInYoung), &isVailedIndex, &notValidIndex);
        Bind(&isVailedIndex);
        {
            GateRef offset = GetIntPtrConstant(Region::GetOldToNewSetOffset(env_.Is32Bit()));
            auto oldToNewSet = Load(VariableType::POINTER(), objectRegion, offset);
            Label isNullPtr(env);
            Label notNullPtr(env);
            Branch(IntptrEuqal(oldToNewSet, GetIntPtrConstant(0)), &isNullPtr, &notNullPtr);
            Bind(&notNullPtr);
            {
                // 1. bit_offset set AddrToBitOffset(address)
                GateRef bitOffset = IntPtrLSR(IntPtrAnd(slotAddr,
                                              GetIntPtrConstant(panda::ecmascript::DEFAULT_REGION_MASK)),
                                              GetIntPtrConstant(RememberedSet::BYTESPERCHUNK_LOG2));
                // (bit_offset >> LOG_BITSPERWORD) << LOG_INTPTR_SIZE
                GateRef logBitsPerWord = GetIntPtrConstant(BitmapHelper::LogBitsPerWord(env_.Is32Bit()));
                GateRef logIntPtrSize = GetIntPtrConstant(BitmapHelper::LogIntptrSize(env_.Is32Bit()));
                GateRef wordOffset = IntPtrLSL(IntPtrLSR(bitOffset, logBitsPerWord), logIntPtrSize);
                // 2. bitmap_[GetWordIdx(bit_offset)] |= GetBitMask(bit_offset)
                // 2.0: wordIdx GetWordIdx(bit_offset)
                // 2.1 bitmap_[wordIdx]
                GateRef bitmapoffset = GetIntPtrConstant(0);
                GateRef bitmapData = Load(VariableType::POINTER(), oldToNewSet, bitmapoffset);
                // 2.2 bitmap_[wordIdx] |= GetBitMask(bit_offset)
                GateRef oldmapValue = Load(VariableType::POINTER(), bitmapData, wordOffset);
                Store(VariableType::POINTER(), glue, bitmapData, wordOffset,
                      IntPtrOr(oldmapValue, GetBitMask(bitOffset)));
                Jump(&notValidIndex);
            }
            Bind(&isNullPtr);
            {
                const CallSignature *insertOldToNewRememberedSet =
                    RuntimeStubCSigns::Get(RTSTUB_ID(InsertOldToNewRememberedSet));
                CallRuntime(insertOldToNewRememberedSet, glue,
                            GetIntPtrConstant(RTSTUB_ID(InsertOldToNewRememberedSet) + NOGC_RTSTUB_CSIGNS_BEGIN),
                            { glue, objectRegion, slotAddr });
                Jump(&notValidIndex);
            }
        }
        Bind(&notValidIndex);
        {
            Label marking(env);
            bool isArch32 = GetEnvironment()->Is32Bit();
            GateRef stateBitFieldAddr = Int64Add(glue,
                GetInt64Constant(JSThread::GlueData::GetStateBitFieldOffset(isArch32)));
            GateRef stateBitField = Load(VariableType::INT64(), stateBitFieldAddr, GetInt64Constant(0));
            Branch(Int64Equal(stateBitField, GetInt64Constant(0)), &exit, &marking);

            Bind(&marking);
            const CallSignature *markingBarrier = RuntimeStubCSigns::Get(RTSTUB_ID(MarkingBarrier));
            CallRuntime(markingBarrier, glue,
                GetIntPtrConstant(RTSTUB_ID(MarkingBarrier) + NOGC_RTSTUB_CSIGNS_BEGIN), {
                glue, slotAddr, objectRegion, TaggedCastToIntPtr(value), valueRegion });
            Jump(&exit);
        }
    }
    Bind(&exit);
    env->PopCurrentLabel();
    return;
}

GateRef Stub::TaggedIsString(GateRef obj)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::BOOL(), FalseConstant());
    Label isHeapObject(env);
    Branch(TaggedIsHeapObject(obj), &isHeapObject, &exit);
    Bind(&isHeapObject);
    {
        result = Int32Equal(GetObjectType(LoadHClass(obj)),
                            GetInt32Constant(static_cast<int32_t>(JSType::STRING)));
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::TaggedIsStringOrSymbol(GateRef obj)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::BOOL(), FalseConstant());
    Label isHeapObject(env);
    Branch(TaggedIsHeapObject(obj), &isHeapObject, &exit);
    Bind(&isHeapObject);
    {
        GateRef objType = GetObjectType(LoadHClass(obj));
        result = Int32Equal(objType, GetInt32Constant(static_cast<int32_t>(JSType::STRING)));
        Label isString(env);
        Label notString(env);
        Branch(*result, &exit, &notString);
        Bind(&notString);
        {
            result = Int32Equal(objType, GetInt32Constant(static_cast<int32_t>(JSType::SYMBOL)));
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::IsUtf16String(GateRef string)
{
    // compressedStringsEnabled fixed to true constant
    GateRef len = Load(VariableType::INT32(), string, GetIntPtrConstant(EcmaString::MIX_LENGTH_OFFSET));
    return Int32Equal(
        Int32And(len, GetInt32Constant(EcmaString::STRING_COMPRESSED_BIT)),
        GetInt32Constant(EcmaString::STRING_UNCOMPRESSED));
}

GateRef Stub::IsUtf8String(GateRef string)
{
    // compressedStringsEnabled fixed to true constant
    GateRef len = Load(VariableType::INT32(), string, GetIntPtrConstant(EcmaString::MIX_LENGTH_OFFSET));
    return Int32Equal(
        Int32And(len, GetInt32Constant(EcmaString::STRING_COMPRESSED_BIT)),
        GetInt32Constant(EcmaString::STRING_COMPRESSED));
}

GateRef Stub::IsInternalString(GateRef string)
{
    // compressedStringsEnabled fixed to true constant
    GateRef len = Load(VariableType::INT32(), string, GetIntPtrConstant(EcmaString::MIX_LENGTH_OFFSET));
    return Int32NotEqual(
        Int32And(len, GetInt32Constant(EcmaString::STRING_INTERN_BIT)),
        GetInt32Constant(0));
}

GateRef Stub::IsDigit(GateRef ch)
{
    return TruncInt32ToInt1(
        Int32And(SExtInt1ToInt32(Int32LessThanOrEqual(ch, GetInt32Constant('9'))),
                 SExtInt1ToInt32(Int32GreaterThanOrEqual(ch, GetInt32Constant('0')))));
}

GateRef Stub::StringToElementIndex(GateRef string)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::INT32(), GetInt32Constant(-1));
    Label greatThanZero(env);
    Label inRange(env);
    GateRef len = Load(VariableType::INT32(), string, GetIntPtrConstant(EcmaString::MIX_LENGTH_OFFSET));
    len = UInt32LSR(len, GetInt32Constant(2));  // 2 : 2 means len must be right shift 2 bits
    Branch(Int32Equal(len, GetInt32Constant(0)), &exit, &greatThanZero);
    Bind(&greatThanZero);
    Branch(Int32GreaterThan(len, GetInt32Constant(MAX_INDEX_LEN)), &exit, &inRange);
    Bind(&inRange);
    {
        GateRef dataUtf16 = IntPtrAdd(string, GetIntPtrConstant(EcmaString::DATA_OFFSET));
        DEFVARIABLE(c, VariableType::INT32(), GetInt32Constant(0));
        Label isUtf16(env);
        Label isUtf8(env);
        Label getChar1(env);
        GateRef isUtf16String = IsUtf16String(string);
        Branch(isUtf16String, &isUtf16, &isUtf8);
        Bind(&isUtf16);
        {
            c = ZExtInt16ToInt32(Load(VariableType::INT16(), dataUtf16));
            Jump(&getChar1);
        }
        Bind(&isUtf8);
        {
            c = ZExtInt8ToInt32(Load(VariableType::INT8(), dataUtf16));
            Jump(&getChar1);
        }
        Bind(&getChar1);
        {
            Label isDigitZero(env);
            Label notDigitZero(env);
            Branch(Int32Equal(*c, GetInt32Constant('0')), &isDigitZero, &notDigitZero);
            Bind(&isDigitZero);
            {
                Label lengthIsOne(env);
                Branch(Int32Equal(len, GetInt32Constant(1)), &lengthIsOne, &exit);
                Bind(&lengthIsOne);
                {
                    result = GetInt32Constant(0);
                    Jump(&exit);
                }
            }
            Bind(&notDigitZero);
            {
                Label isDigit(env);
                DEFVARIABLE(i, VariableType::INT32(), GetInt32Constant(1));
                DEFVARIABLE(n, VariableType::INT32(), Int32Sub(*c, GetInt32Constant('0')));
                Branch(IsDigit(*c), &isDigit, &exit);
                Label loopHead(env);
                Label loopEnd(env);
                Label afterLoop(env);
                Bind(&isDigit);
                Branch(UInt32LessThan(*i, len), &loopHead, &afterLoop);
                LoopBegin(&loopHead);
                {
                    Label isUtf16(env);
                    Label notUtf16(env);
                    Label getChar2(env);
                    Branch(isUtf16String, &isUtf16, &notUtf16);
                    Bind(&isUtf16);
                    {
                        // 2 : 2 means utf16 char width is two bytes
                        auto charOffset = IntPtrMul(ChangeInt32ToIntPtr(*i),  GetIntPtrConstant(2));
                        c = ZExtInt16ToInt32(Load(VariableType::INT16(), dataUtf16, charOffset));
                        Jump(&getChar2);
                    }
                    Bind(&notUtf16);
                    {
                        c = ZExtInt8ToInt32(Load(VariableType::INT8(), dataUtf16, ChangeInt32ToIntPtr(*i)));
                        Jump(&getChar2);
                    }
                    Bind(&getChar2);
                    {
                        Label isDigit2(env);
                        Label notDigit2(env);
                        Branch(IsDigit(*c), &isDigit2, &notDigit2);
                        Bind(&isDigit2);
                        {
                            // 10 means the base of digit is 10.
                            n = Int32Add(Int32Mul(*n, GetInt32Constant(10)),
                                         Int32Sub(*c, GetInt32Constant('0')));
                            i = Int32Add(*i, GetInt32Constant(1));
                            Branch(UInt32LessThan(*i, len), &loopEnd, &afterLoop);
                        }
                        Bind(&notDigit2);
                        Jump(&exit);
                    }
                }
                Bind(&loopEnd);
                LoopEnd(&loopHead);
                Bind(&afterLoop);
                {
                    Label lessThanMaxIndex(env);
                    Branch(UInt32LessThan(*n, GetInt32Constant(JSObject::MAX_ELEMENT_INDEX)),
                           &lessThanMaxIndex, &exit);
                    Bind(&lessThanMaxIndex);
                    {
                        result = *n;
                        Jump(&exit);
                    }
                }
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::TryToElementsIndex(GateRef key)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    Label isKeyInt(env);
    Label notKeyInt(env);

    DEFVARIABLE(resultKey, VariableType::INT32(), GetInt32Constant(-1));
    Branch(TaggedIsInt(key), &isKeyInt, &notKeyInt);
    Bind(&isKeyInt);
    {
        resultKey = TaggedCastToInt32(key);
        Jump(&exit);
    }
    Bind(&notKeyInt);
    {
        Label isString(env);
        Label notString(env);
        Branch(TaggedIsString(key), &isString, &notString);
        Bind(&isString);
        {
            resultKey = StringToElementIndex(key);
            Jump(&exit);
        }
        Bind(&notString);
        {
            Label isDouble(env);
            Branch(TaggedIsDouble(key), &isDouble, &exit);
            Bind(&isDouble);
            {
                GateRef number = TaggedCastToDouble(key);
                GateRef integer = ChangeFloat64ToInt32(number);
                Label isEqual(env);
                Branch(DoubleEqual(number, ChangeInt32ToFloat64(integer)), &isEqual, &exit);
                Bind(&isEqual);
                {
                    resultKey = integer;
                    Jump(&exit);
                }
            }
        }
    }
    Bind(&exit);
    auto ret = *resultKey;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::LoadFromField(GateRef receiver, GateRef handlerInfo)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    Label handlerInfoIsInlinedProps(env);
    Label handlerInfoNotInlinedProps(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), GetUndefinedConstant());
    GateRef index = HandlerBaseGetOffset(handlerInfo);
    Branch(HandlerBaseIsInlinedProperty(handlerInfo), &handlerInfoIsInlinedProps, &handlerInfoNotInlinedProps);
    Bind(&handlerInfoIsInlinedProps);
    {
        result = Load(VariableType::JS_ANY(), receiver, IntPtrMul(ChangeInt32ToIntPtr(index),
            GetIntPtrConstant(JSTaggedValue::TaggedTypeSize())));
        Jump(&exit);
    }
    Bind(&handlerInfoNotInlinedProps);
    {
        result = GetValueFromTaggedArray(VariableType::JS_ANY(), GetPropertiesArray(receiver), index);
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::LoadGlobal(GateRef cell)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    Label cellIsInvalid(env);
    Label cellNotInvalid(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), GetHoleConstant());
    Branch(IsInvalidPropertyBox(cell), &cellIsInvalid, &cellNotInvalid);
    Bind(&cellIsInvalid);
    {
        Jump(&exit);
    }
    Bind(&cellNotInvalid);
    {
        result = GetValueFromPropertyBox(cell);
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::CheckPolyHClass(GateRef cachedValue, GateRef hclass)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    Label loopHead(env);
    Label loopEnd(env);
    Label iLessLength(env);
    Label hasHclass(env);
    Label cachedValueNotWeak(env);
    DEFVARIABLE(i, VariableType::INT32(), GetInt32Constant(0));
    DEFVARIABLE(result, VariableType::JS_ANY(), GetHoleConstant());
    Branch(TaggedIsWeak(cachedValue), &exit, &cachedValueNotWeak);
    Bind(&cachedValueNotWeak);
    {
        GateRef length = GetLengthOfTaggedArray(cachedValue);
        Jump(&loopHead);
        LoopBegin(&loopHead);
        {
            Branch(UInt32LessThan(*i, length), &iLessLength, &exit);
            Bind(&iLessLength);
            {
                GateRef element = GetValueFromTaggedArray(VariableType::JS_ANY(), cachedValue, *i);
                Branch(Int64Equal(TaggedCastToWeakReferentUnChecked(element), hclass), &hasHclass, &loopEnd);
                Bind(&hasHclass);
                result = GetValueFromTaggedArray(VariableType::JS_ANY(), cachedValue,
                                                 Int32Add(*i, GetInt32Constant(1)));
                Jump(&exit);
            }
            Bind(&loopEnd);
            i = Int32Add(*i, GetInt32Constant(2));  // 2 means one ic, two slot
            LoopEnd(&loopHead);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::LoadICWithHandler(GateRef glue, GateRef receiver, GateRef argHolder, GateRef argHandler)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    Label handlerIsInt(env);
    Label handlerNotInt(env);
    Label handlerInfoIsField(env);
    Label handlerInfoNotField(env);
    Label handlerInfoIsNonExist(env);
    Label handlerInfoNotNonExist(env);
    Label handlerIsPrototypeHandler(env);
    Label handlerNotPrototypeHandler(env);
    Label cellHasChanged(env);
    Label loopHead(env);
    Label loopEnd(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), GetUndefinedConstant());
    DEFVARIABLE(holder, VariableType::JS_ANY(), argHolder);
    DEFVARIABLE(handler, VariableType::JS_ANY(), argHandler);

    Jump(&loopHead);
    LoopBegin(&loopHead);
    {
        Branch(TaggedIsInt(*handler), &handlerIsInt, &handlerNotInt);
        Bind(&handlerIsInt);
        {
            GateRef handlerInfo = TaggedCastToInt32(*handler);
            Branch(IsField(handlerInfo), &handlerInfoIsField, &handlerInfoNotField);
            Bind(&handlerInfoIsField);
            {
                result = LoadFromField(*holder, handlerInfo);
                Jump(&exit);
            }
            Bind(&handlerInfoNotField);
            {
                Branch(IsNonExist(handlerInfo), &handlerInfoIsNonExist, &handlerInfoNotNonExist);
                Bind(&handlerInfoIsNonExist);
                Jump(&exit);
                Bind(&handlerInfoNotNonExist);
                GateRef accessor = LoadFromField(*holder, handlerInfo);
                result = CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(CallGetter2)),
                    { receiver, *holder, accessor });
                Jump(&exit);
            }
        }
        Bind(&handlerNotInt);
        Branch(TaggedIsPrototypeHandler(*handler), &handlerIsPrototypeHandler, &handlerNotPrototypeHandler);
        Bind(&handlerIsPrototypeHandler);
        {
            GateRef cellValue = GetProtoCell(*handler);
            Branch(GetHasChanged(cellValue), &cellHasChanged, &loopEnd);
            Bind(&cellHasChanged);
            {
                result = GetHoleConstant();
                Jump(&exit);
            }
            Bind(&loopEnd);
            holder = GetPrototypeHandlerHolder(*handler);
            handler = GetPrototypeHandlerHandlerInfo(*handler);
            LoopEnd(&loopHead);
        }
    }
    Bind(&handlerNotPrototypeHandler);
    result = LoadGlobal(*handler);
    Jump(&exit);

    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::LoadElement(GateRef receiver, GateRef key)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    Label indexLessZero(env);
    Label indexNotLessZero(env);
    Label lengthLessIndex(env);
    Label lengthNotLessIndex(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), GetHoleConstant());
    GateRef index = TryToElementsIndex(key);
    Branch(UInt32LessThan(index, GetInt32Constant(0)), &indexLessZero, &indexNotLessZero);
    Bind(&indexLessZero);
    {
        Jump(&exit);
    }
    Bind(&indexNotLessZero);
    {
        GateRef elements = GetElementsArray(receiver);
        Branch(Int32LessThanOrEqual(GetLengthOfTaggedArray(elements), index), &lengthLessIndex, &lengthNotLessIndex);
        Bind(&lengthLessIndex);
        Jump(&exit);
        Bind(&lengthNotLessIndex);
        result = GetValueFromTaggedArray(VariableType::JS_ANY(), elements, index);
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::ICStoreElement(GateRef glue, GateRef receiver, GateRef key, GateRef value, GateRef handler)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    Label indexLessZero(env);
    Label indexNotLessZero(env);
    Label handerInfoIsJSArray(env);
    Label handerInfoNotJSArray(env);
    Label indexGreaterLength(env);
    Label indexGreaterCapacity(env);
    Label callRuntime(env);
    Label storeElement(env);
    Label handlerIsInt(env);
    Label handlerNotInt(env);
    Label cellHasChanged(env);
    Label cellHasNotChanged(env);
    Label loopHead(env);
    Label loopEnd(env);
    DEFVARIABLE(result, VariableType::INT64(), GetHoleConstant(VariableType::INT64()));
    DEFVARIABLE(varHandler, VariableType::JS_ANY(), handler);
    GateRef index = TryToElementsIndex(key);
    Branch(UInt32LessThan(index, GetInt32Constant(0)), &indexLessZero, &indexNotLessZero);
    Bind(&indexLessZero);
    {
        Jump(&exit);
    }
    Bind(&indexNotLessZero);
    {
        Jump(&loopHead);
        LoopBegin(&loopHead);
        Branch(TaggedIsInt(*varHandler), &handlerIsInt, &handlerNotInt);
        Bind(&handlerIsInt);
        {
            GateRef handlerInfo = TaggedCastToInt32(handler);
            Branch(HandlerBaseIsJSArray(handlerInfo), &handerInfoIsJSArray, &handerInfoNotJSArray);
            Bind(&handerInfoIsJSArray);
            {
                GateRef oldLength = GetArrayLength(receiver);
                Branch(Int32GreaterThanOrEqual(index, oldLength), &indexGreaterLength, &handerInfoNotJSArray);
                Bind(&indexGreaterLength);
                Store(VariableType::INT64(), glue, receiver,
                    GetIntPtrConstant(panda::ecmascript::JSArray::LENGTH_OFFSET),
                    IntBuildTaggedWithNoGC(Int32Add(index, GetInt32Constant(1))));
                Jump(&handerInfoNotJSArray);
            }
            Bind(&handerInfoNotJSArray);
            {
                GateRef elements = GetElementsArray(receiver);
                GateRef capacity = GetLengthOfTaggedArray(elements);
                Branch(Int32GreaterThanOrEqual(index, capacity), &callRuntime, &storeElement);
                Bind(&callRuntime);
                {
                    result = ChangeTaggedPointerToInt64(CallRuntimeTrampoline(glue,
                        GetInt64Constant(RTSTUB_ID(TaggedArraySetValue)), {
                            receiver, value, elements, IntBuildTaggedTypeWithNoGC(index),
                            IntBuildTaggedTypeWithNoGC(capacity)
                        }));
                    Jump(&exit);
                }
                Bind(&storeElement);
                {
                    SetValueToTaggedArray(VariableType::JS_ANY(), glue, elements, index, value);
                    result = GetUndefinedConstant(VariableType::INT64());
                    Jump(&exit);
                }
            }
        }
        Bind(&handlerNotInt);
        {
            GateRef cellValue = GetProtoCell(*varHandler);
            Branch(GetHasChanged(cellValue), &cellHasChanged, &loopEnd);
            Bind(&cellHasChanged);
            {
                Jump(&exit);
            }
            Bind(&loopEnd);
            {
                varHandler = GetPrototypeHandlerHandlerInfo(*varHandler);
                LoopEnd(&loopHead);
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::GetArrayLength(GateRef object)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    Label lengthIsInt(env);
    Label lengthNotInt(env);
    DEFVARIABLE(result, VariableType::INT32(), GetInt32Constant(0));
    GateRef lengthOffset = GetIntPtrConstant(panda::ecmascript::JSArray::LENGTH_OFFSET);
    GateRef length = Load(VariableType::INT64(), object, lengthOffset);
    Branch(TaggedIsInt(length), &lengthIsInt, &lengthNotInt);
    Bind(&lengthIsInt);
    {
        result = TaggedCastToInt32(length);
        Jump(&exit);
    }
    Bind(&lengthNotInt);
    {
        result = ChangeFloat64ToInt32(TaggedCastToDouble(length));
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::StoreICWithHandler(GateRef glue, GateRef receiver, GateRef argHolder, GateRef value, GateRef argHandler)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    Label handlerIsInt(env);
    Label handlerNotInt(env);
    Label handlerInfoIsField(env);
    Label handlerInfoNotField(env);
    Label handlerIsTransitionHandler(env);
    Label handlerNotTransitionHandler(env);
    Label handlerIsPrototypeHandler(env);
    Label handlerNotPrototypeHandler(env);
    Label handlerIsPropertyBox(env);
    Label handlerNotPropertyBox(env);
    Label cellHasChanged(env);
    Label loopHead(env);
    Label loopEnd(env);
    DEFVARIABLE(result, VariableType::INT64(), GetUndefinedConstant(VariableType::INT64()));
    DEFVARIABLE(holder, VariableType::JS_ANY(), argHolder);
    DEFVARIABLE(handler, VariableType::JS_ANY(), argHandler);
    Jump(&loopHead);
    LoopBegin(&loopHead);
    {
        Branch(TaggedIsInt(*handler), &handlerIsInt, &handlerNotInt);
        Bind(&handlerIsInt);
        {
            GateRef handlerInfo = TaggedCastToInt32(*handler);
            Branch(IsField(handlerInfo), &handlerInfoIsField, &handlerInfoNotField);
            Bind(&handlerInfoIsField);
            {
                StoreField(glue, receiver, value, handlerInfo);
                Jump(&exit);
            }
            Bind(&handlerInfoNotField);
            {
                GateRef accessor = LoadFromField(*holder, handlerInfo);
                result = ChangeTaggedPointerToInt64(CallRuntimeTrampoline(glue,
                    GetInt64Constant(RTSTUB_ID(CallSetter2)), {
                        receiver, value, accessor
                    }));
                Jump(&exit);
            }
        }
        Bind(&handlerNotInt);
        {
            Branch(TaggedIsTransitionHandler(*handler), &handlerIsTransitionHandler, &handlerNotTransitionHandler);
            Bind(&handlerIsTransitionHandler);
            {
                StoreWithTransition(glue, receiver, value, *handler);
                Jump(&exit);
            }
            Bind(&handlerNotTransitionHandler);
            {
                Branch(TaggedIsPrototypeHandler(*handler), &handlerIsPrototypeHandler, &handlerNotPrototypeHandler);
                Bind(&handlerNotPrototypeHandler);
                {
                    Branch(TaggedIsPropertyBox(*handler), &handlerIsPropertyBox, &handlerNotPropertyBox);
                    Bind(&handlerIsPropertyBox);
                    StoreGlobal(glue, value, *handler);
                    Jump(&exit);
                    Bind(&handlerNotPropertyBox);
                    Jump(&exit);
                }
            }
        }
        Bind(&handlerIsPrototypeHandler);
        {
            GateRef cellValue = GetProtoCell(*handler);
            Branch(GetHasChanged(cellValue), &cellHasChanged, &loopEnd);
            Bind(&cellHasChanged);
            {
                result = GetHoleConstant(VariableType::INT64());
                Jump(&exit);
            }
            Bind(&loopEnd);
            {
                holder = GetPrototypeHandlerHolder(*handler);
                handler = GetPrototypeHandlerHandlerInfo(*handler);
                LoopEnd(&loopHead);
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

void Stub::StoreField(GateRef glue, GateRef receiver, GateRef value, GateRef handler)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    Label handlerIsInlinedProperty(env);
    Label handlerNotInlinedProperty(env);
    GateRef index = HandlerBaseGetOffset(handler);
    Branch(HandlerBaseIsInlinedProperty(handler), &handlerIsInlinedProperty, &handlerNotInlinedProperty);
    Bind(&handlerIsInlinedProperty);
    {
        Store(VariableType::JS_ANY(), glue, receiver,
            IntPtrMul(ChangeInt32ToIntPtr(index), GetIntPtrConstant(JSTaggedValue::TaggedTypeSize())),
            value);
        Jump(&exit);
    }
    Bind(&handlerNotInlinedProperty);
    {
        GateRef array = GetPropertiesArray(receiver);
        SetValueToTaggedArray(VariableType::JS_ANY(), glue, array, index, value);
        Jump(&exit);
    }
    Bind(&exit);
    env->PopCurrentLabel();
}

void Stub::StoreWithTransition(GateRef glue, GateRef receiver, GateRef value, GateRef handler)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);

    Label handlerInfoIsInlinedProps(env);
    Label handlerInfoNotInlinedProps(env);
    Label indexMoreCapacity(env);
    Label indexLessCapacity(env);
    GateRef newHClass = GetTransitionFromHClass(handler);
    StoreHClass(glue, receiver, newHClass);
    GateRef handlerInfo = TaggedCastToInt32(GetTransitionHandlerInfo(handler));
    Branch(HandlerBaseIsInlinedProperty(handlerInfo), &handlerInfoIsInlinedProps, &handlerInfoNotInlinedProps);
    Bind(&handlerInfoNotInlinedProps);
    {
        GateRef array = GetPropertiesArray(receiver);
        GateRef capacity = GetLengthOfTaggedArray(array);
        GateRef index = HandlerBaseGetOffset(handlerInfo);
        Branch(Int32GreaterThanOrEqual(index, capacity), &indexMoreCapacity, &indexLessCapacity);
        Bind(&indexMoreCapacity);
        {
            CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(PropertiesSetValue)), {
                receiver, value, array, IntBuildTaggedTypeWithNoGC(capacity), IntBuildTaggedTypeWithNoGC(index)
            });
            Jump(&exit);
        }
        Bind(&indexLessCapacity);
        {
            Store(VariableType::INT64(), glue, IntPtrAdd(array, GetIntPtrConstant(TaggedArray::DATA_OFFSET)),
                IntPtrMul(ChangeInt32ToIntPtr(index), GetIntPtrConstant(JSTaggedValue::TaggedTypeSize())),
                value);
            Jump(&exit);
        }
    }
    Bind(&handlerInfoIsInlinedProps);
    {
        StoreField(glue, receiver, value, handlerInfo);
        Jump(&exit);
    }
    Bind(&exit);
    env->PopCurrentLabel();
}

GateRef Stub::StoreGlobal(GateRef glue, GateRef value, GateRef cell)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    Label cellIsInvalid(env);
    Label cellNotInvalid(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), GetHoleConstant());
    Branch(IsInvalidPropertyBox(cell), &cellIsInvalid, &cellNotInvalid);
    Bind(&cellIsInvalid);
    {
        Jump(&exit);
    }
    Bind(&cellNotInvalid);
    {
        Store(VariableType::JS_ANY(), glue, cell, GetIntPtrConstant(PropertyBox::VALUE_OFFSET), value);
        result = GetUndefinedConstant();
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

template<typename DictionaryT>
GateRef Stub::GetAttributesFromDictionary(GateRef elements, GateRef entry)
{
    GateRef arrayIndex =
    Int32Add(GetInt32Constant(DictionaryT::TABLE_HEADER_SIZE),
             Int32Mul(entry, GetInt32Constant(DictionaryT::ENTRY_SIZE)));
    GateRef attributesIndex =
        Int32Add(arrayIndex, GetInt32Constant(DictionaryT::ENTRY_DETAILS_INDEX));
    return TaggedCastToInt32(GetValueFromTaggedArray(VariableType::INT64(), elements, attributesIndex));
}

template<typename DictionaryT>
GateRef Stub::GetValueFromDictionary(VariableType returnType, GateRef elements, GateRef entry)
{
    GateRef arrayIndex =
        Int32Add(GetInt32Constant(DictionaryT::TABLE_HEADER_SIZE),
                 Int32Mul(entry, GetInt32Constant(DictionaryT::ENTRY_SIZE)));
    GateRef valueIndex =
        Int32Add(arrayIndex, GetInt32Constant(DictionaryT::ENTRY_VALUE_INDEX));
    return GetValueFromTaggedArray(returnType, elements, valueIndex);
}

template<typename DictionaryT>
GateRef Stub::GetKeyFromDictionary(VariableType returnType, GateRef elements, GateRef entry)
{
    auto env = GetEnvironment();
    Label subentry(env);
    env->PushCurrentLabel(&subentry);
    Label exit(env);
    DEFVARIABLE(result, returnType, GetUndefinedConstant());
    Label ltZero(env);
    Label notLtZero(env);
    Label gtLength(env);
    Label notGtLength(env);
    GateRef dictionaryLength =
        Load(VariableType::INT32(), elements, GetIntPtrConstant(TaggedArray::LENGTH_OFFSET));
    GateRef arrayIndex =
        Int32Add(GetInt32Constant(DictionaryT::TABLE_HEADER_SIZE),
                 Int32Mul(entry, GetInt32Constant(DictionaryT::ENTRY_SIZE)));
    Branch(Int32LessThan(arrayIndex, GetInt32Constant(0)), &ltZero, &notLtZero);
    Bind(&ltZero);
    Jump(&exit);
    Bind(&notLtZero);
    Branch(Int32GreaterThan(arrayIndex, dictionaryLength), &gtLength, &notGtLength);
    Bind(&gtLength);
    Jump(&exit);
    Bind(&notGtLength);
    result = GetValueFromTaggedArray(returnType, elements, arrayIndex);
    Jump(&exit);
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

inline void Stub::UpdateValueAndAttributes(GateRef glue, GateRef elements, GateRef index, GateRef value, GateRef attr)
{
    GateRef arrayIndex =
        Int32Add(GetInt32Constant(NameDictionary::TABLE_HEADER_SIZE),
                 Int32Mul(index, GetInt32Constant(NameDictionary::ENTRY_SIZE)));
    GateRef valueIndex =
        Int32Add(arrayIndex, GetInt32Constant(NameDictionary::ENTRY_VALUE_INDEX));
    GateRef attributesIndex =
        Int32Add(arrayIndex, GetInt32Constant(NameDictionary::ENTRY_DETAILS_INDEX));
    SetValueToTaggedArray(VariableType::JS_ANY(), glue, elements, valueIndex, value);
    GateRef attroffset =
        IntPtrMul(ChangeInt32ToIntPtr(attributesIndex), GetIntPtrConstant(JSTaggedValue::TaggedTypeSize()));
    GateRef dataOffset = IntPtrAdd(attroffset, GetIntPtrConstant(TaggedArray::DATA_OFFSET));
    Store(VariableType::INT64(), glue, elements, dataOffset, IntBuildTaggedWithNoGC(attr));
}

inline void Stub::UpdateValueInDict(GateRef glue, GateRef elements, GateRef index, GateRef value)
{
    GateRef arrayIndex = Int32Add(GetInt32Constant(NameDictionary::TABLE_HEADER_SIZE),
        Int32Mul(index, GetInt32Constant(NameDictionary::ENTRY_SIZE)));
    GateRef valueIndex = Int32Add(arrayIndex, GetInt32Constant(NameDictionary::ENTRY_VALUE_INDEX));
    SetValueToTaggedArray(VariableType::JS_ANY(), glue, elements, valueIndex, value);
}

GateRef Stub::GetPropertyByIndex(GateRef glue, GateRef receiver, GateRef index)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), GetHoleConstant());
    DEFVARIABLE(holder, VariableType::JS_ANY(), receiver);
    Label exit(env);
    Label loopHead(env);
    Label loopEnd(env);
    Label loopExit(env);
    Label afterLoop(env);
    Jump(&loopHead);
    LoopBegin(&loopHead);
    {
        GateRef hclass = LoadHClass(*holder);
        GateRef jsType = GetObjectType(hclass);
        Label isSpecialIndexed(env);
        Label notSpecialIndexed(env);
        Branch(IsSpecialIndexedObj(jsType), &isSpecialIndexed, &notSpecialIndexed);
        Bind(&isSpecialIndexed);
        {
            Label isSpecialContainer(env);
            Label notSpecialContainer(env);
            // Add SpecialContainer
            Branch(IsSpecialContainer(jsType), &isSpecialContainer, &notSpecialContainer);
            Bind(&isSpecialContainer);
            {
                result = GetContainerProperty(glue, *holder, index, jsType);
                Jump(&exit);
            }
            Bind(&notSpecialContainer);
            {
                result = GetHoleConstant();
                Jump(&exit);
            }
        }
        Bind(&notSpecialIndexed);
        {
            GateRef elements = GetElementsArray(*holder);
            Label isDictionaryElement(env);
            Label notDictionaryElement(env);
            Branch(IsDictionaryElement(hclass), &isDictionaryElement, &notDictionaryElement);
            Bind(&notDictionaryElement);
            {
                Label lessThanLength(env);
                Label notLessThanLength(env);
                Branch(UInt32LessThan(index, GetLengthOfTaggedArray(elements)), &lessThanLength, &notLessThanLength);
                Bind(&lessThanLength);
                {
                    Label notHole(env);
                    Label isHole(env);
                    GateRef value = GetValueFromTaggedArray(VariableType::JS_ANY(), elements, index);
                    Branch(TaggedIsNotHole(value), &notHole, &isHole);
                    Bind(&notHole);
                    {
                        result = value;
                        Jump(&exit);
                    }
                    Bind(&isHole);
                    {
                        Jump(&loopExit);
                    }
                }
                Bind(&notLessThanLength);
                {
                    result = GetHoleConstant();
                    Jump(&exit);
                }
            }
            Bind(&isDictionaryElement);
            {
                GateRef entry = FindElementFromNumberDictionary(glue, elements, index);
                Label notNegtiveOne(env);
                Label negtiveOne(env);
                Branch(Int32NotEqual(entry, GetInt32Constant(-1)), &notNegtiveOne, &negtiveOne);
                Bind(&notNegtiveOne);
                {
                    GateRef attr = GetAttributesFromDictionary<NumberDictionary>(elements, entry);
                    GateRef value = GetValueFromDictionary<NumberDictionary>(VariableType::JS_ANY(), elements, entry);
                    Label isAccessor(env);
                    Label notAccessor(env);
                    Branch(IsAccessor(attr), &isAccessor, &notAccessor);
                    Bind(&isAccessor);
                    {
                        Label isInternal(env);
                        Label notInternal(env);
                        Branch(IsAccessorInternal(value), &isInternal, &notInternal);
                        Bind(&isInternal);
                        {
                            result = CallRuntimeTrampoline(glue,
                                GetInt64Constant(RTSTUB_ID(CallInternalGetter)), { value, *holder });
                            Jump(&exit);
                        }
                        Bind(&notInternal);
                        {
                            result = CallRuntimeTrampoline(glue,
                                GetInt64Constant(RTSTUB_ID(CallGetter)), { value, receiver });
                            Jump(&exit);
                        }
                    }
                    Bind(&notAccessor);
                    {
                        result = value;
                        Jump(&exit);
                    }
                }
                Bind(&negtiveOne);
                Jump(&loopExit);
            }
            Bind(&loopExit);
            {
                holder = GetPrototypeFromHClass(LoadHClass(*holder));
                Branch(TaggedIsHeapObject(*holder), &loopEnd, &afterLoop);
            }
        }
        Bind(&loopEnd);
        LoopEnd(&loopHead);
        Bind(&afterLoop);
        {
            result = GetUndefinedConstant();
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::GetPropertyByName(GateRef glue, GateRef receiver, GateRef key)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), GetHoleConstant());
    DEFVARIABLE(holder, VariableType::JS_ANY(), receiver);
    Label exit(env);
    Label loopHead(env);
    Label loopEnd(env);
    Label loopExit(env);
    Label afterLoop(env);
    Jump(&loopHead);
    LoopBegin(&loopHead);
    {
        GateRef hClass = LoadHClass(*holder);
        GateRef jsType = GetObjectType(hClass);
        Label isSIndexObj(env);
        Label notSIndexObj(env);
        Branch(IsSpecialIndexedObj(jsType), &isSIndexObj, &notSIndexObj);
        Bind(&isSIndexObj);
        {
            result = GetHoleConstant();
            Jump(&exit);
        }
        Bind(&notSIndexObj);
        {
            Label isDicMode(env);
            Label notDicMode(env);
            Branch(IsDictionaryModeByHClass(hClass), &isDicMode, &notDicMode);
            Bind(&notDicMode);
            {
                GateRef layOutInfo = GetLayoutFromHClass(hClass);
                GateRef propsNum = GetNumberOfPropsFromHClass(hClass);
                GateRef entry = FindElementWithCache(glue, layOutInfo, hClass, key, propsNum);
                Label hasEntry(env);
                Label noEntry(env);
                Branch(Int32NotEqual(entry, GetInt32Constant(-1)), &hasEntry, &noEntry);
                Bind(&hasEntry);
                {
                    GateRef propAttr = GetPropAttrFromLayoutInfo(layOutInfo, entry);
                    GateRef attr = TaggedCastToInt32(propAttr);
                    GateRef value = JSObjectGetProperty(VariableType::JS_ANY(), *holder, hClass, attr);
                    Label isAccessor(env);
                    Label notAccessor(env);
                    Branch(IsAccessor(attr), &isAccessor, &notAccessor);
                    Bind(&isAccessor);
                    {
                        Label isInternal(env);
                        Label notInternal(env);
                        Branch(IsAccessorInternal(value), &isInternal, &notInternal);
                        Bind(&isInternal);
                        {
                            result = CallRuntimeTrampoline(glue,
                                GetInt64Constant(RTSTUB_ID(CallInternalGetter)), { value, *holder });
                            Jump(&exit);
                        }
                        Bind(&notInternal);
                        {
                            result = CallRuntimeTrampoline(glue,
                                GetInt64Constant(RTSTUB_ID(CallGetter)), { value, receiver });
                            Jump(&exit);
                        }
                    }
                    Bind(&notAccessor);
                    {
                        result = value;
                        Jump(&exit);
                    }
                }
                Bind(&noEntry);
                {
                    Jump(&loopExit);
                }
            }
            Bind(&isDicMode);
            {
                GateRef array = GetPropertiesArray(*holder);
                GateRef entry = FindEntryFromNameDictionary(glue, array, key);
                Label notNegtiveOne(env);
                Label negtiveOne(env);
                Branch(Int32NotEqual(entry, GetInt32Constant(-1)), &notNegtiveOne, &negtiveOne);
                Bind(&notNegtiveOne);
                {
                    GateRef attr = GetAttributesFromDictionary<NameDictionary>(array, entry);
                    GateRef value = GetValueFromDictionary<NameDictionary>(VariableType::JS_ANY(), array, entry);
                    Label isAccessor1(env);
                    Label notAccessor1(env);
                    Branch(IsAccessor(attr), &isAccessor1, &notAccessor1);
                    Bind(&isAccessor1);
                    {
                        Label isInternal1(env);
                        Label notInternal1(env);
                        Branch(IsAccessorInternal(value), &isInternal1, &notInternal1);
                        Bind(&isInternal1);
                        {
                            result = CallRuntimeTrampoline(glue,
                                GetInt64Constant(RTSTUB_ID(CallInternalGetter)), { value, *holder });
                            Jump(&exit);
                        }
                        Bind(&notInternal1);
                        {
                            result = CallRuntimeTrampoline(glue,
                                GetInt64Constant(RTSTUB_ID(CallGetter)), { value, receiver });
                            Jump(&exit);
                        }
                    }
                    Bind(&notAccessor1);
                    {
                        result = value;
                        Jump(&exit);
                    }
                }
                Bind(&negtiveOne);
                Jump(&loopExit);
            }
            Bind(&loopExit);
            {
                holder = GetPrototypeFromHClass(LoadHClass(*holder));
                Branch(TaggedIsHeapObject(*holder), &loopEnd, &afterLoop);
            }
        }
        Bind(&loopEnd);
        LoopEnd(&loopHead);
        Bind(&afterLoop);
        {
            result = GetUndefinedConstant();
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

void Stub::CopyAllHClass(GateRef glue, GateRef dstHClass, GateRef srcHClass)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    auto proto = GetPrototypeFromHClass(srcHClass);
    SetPrototypeToHClass(VariableType::JS_POINTER(), glue, dstHClass, proto);
    SetBitFieldToHClass(glue, dstHClass, GetBitFieldFromHClass(srcHClass));
    SetNumberOfPropsToHClass(glue, dstHClass, GetNumberOfPropsFromHClass(srcHClass));
    SetTransitionsToHClass(VariableType::INT64(), glue, dstHClass, GetInt64Constant(JSTaggedValue::VALUE_UNDEFINED));
    SetProtoChangeDetailsToHClass(VariableType::INT64(), glue, dstHClass,
                                  GetInt64Constant(JSTaggedValue::VALUE_NULL));
    SetEnumCacheToHClass(VariableType::INT64(), glue, dstHClass, GetInt64Constant(JSTaggedValue::VALUE_NULL));
    SetLayoutToHClass(VariableType::JS_POINTER(), glue, dstHClass, GetLayoutFromHClass(srcHClass));
    env->PopCurrentLabel();
    return;
}

GateRef Stub::FindTransitions(GateRef glue, GateRef receiver, GateRef hclass, GateRef key, GateRef metaData)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    GateRef transitionOffset = GetIntPtrConstant(JSHClass::TRANSTIONS_OFFSET);
    GateRef transition = Load(VariableType::JS_POINTER(), hclass, transitionOffset);
    DEFVARIABLE(result, VariableType::JS_ANY(), transition);

    Label notUndefined(env);
    Branch(Int64Equal(transition, GetUndefinedConstant()), &exit, &notUndefined);
    Bind(&notUndefined);
    {
        Label isWeak(env);
        Label notWeak(env);
        Branch(TaggedIsWeak(transition), &isWeak, &notWeak);
        Bind(&isWeak);
        {
            GateRef transitionHClass = TaggedCastToWeakReferentUnChecked(transition);
            GateRef propNums = GetNumberOfPropsFromHClass(transitionHClass);
            GateRef last = Int32Sub(propNums, GetInt32Constant(1));
            GateRef layoutInfo = GetLayoutFromHClass(transitionHClass);
            GateRef cachedKey = GetKeyFromLayoutInfo(layoutInfo, last);
            GateRef cachedAttr = TaggedCastToInt32(GetPropAttrFromLayoutInfo(layoutInfo, last));
            GateRef cachedMetaData = GetPropertyMetaDataFromAttr(cachedAttr);
            Label keyMatch(env);
            Label isMatch(env);
            Label notMatch(env);
            Branch(Int64Equal(cachedKey, key), &keyMatch, &notMatch);
            Bind(&keyMatch);
            {
                Branch(Int32Equal(metaData, cachedMetaData), &isMatch, &notMatch);
                Bind(&isMatch);
                {
#if ECMASCRIPT_ENABLE_IC
                    NotifyHClassChanged(glue, hclass, transitionHClass);
#endif
                    StoreHClass(glue, receiver, transitionHClass);
                    Jump(&exit);
                }
            }
            Bind(&notMatch);
            {
                result = GetUndefinedConstant();
                Jump(&exit);
            }
        }
        Bind(&notWeak);
        {
            // need to find from dictionary
            GateRef entry = FindEntryFromTransitionDictionary(glue, transition, key, metaData);
            Label isFound(env);
            Label notFound(env);
            Branch(Int32NotEqual(entry, GetInt32Constant(-1)), &isFound, &notFound);
            Bind(&isFound);
            auto value = GetValueFromDictionary<TransitionsDictionary>(
                VariableType::JS_POINTER(), transition, entry);
            Label valueUndefined(env);
            Label valueNotUndefined(env);
            Branch(Int64NotEqual(value, GetUndefinedConstant()), &valueNotUndefined,
                &valueUndefined);
            Bind(&valueNotUndefined);
            {
                GateRef newHClass = TaggedCastToWeakReferentUnChecked(value);
                result = ChangeInt64ToTagged(newHClass);
#if ECMASCRIPT_ENABLE_IC
                NotifyHClassChanged(glue, hclass, newHClass);
#endif
                StoreHClass(glue, receiver, newHClass);
                Jump(&exit);
                Bind(&notFound);
                result = GetUndefinedConstant();
                Jump(&exit);
            }
            Bind(&valueUndefined);
            {
                result = GetUndefinedConstant();
                Jump(&exit);
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::SetPropertyByIndex(GateRef glue, GateRef receiver, GateRef index, GateRef value)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    DEFVARIABLE(returnValue, VariableType::INT64(), GetHoleConstant(VariableType::INT64()));
    DEFVARIABLE(holder, VariableType::JS_ANY(), receiver);
    Label exit(env);
    Label loopHead(env);
    Label loopEnd(env);
    Label loopExit(env);
    Label afterLoop(env);
    Jump(&loopHead);
    LoopBegin(&loopHead);
    {
        GateRef hclass = LoadHClass(*holder);
        GateRef jsType = GetObjectType(hclass);
        Label isSpecialIndex(env);
        Label notSpecialIndex(env);
        Branch(IsSpecialIndexedObj(jsType), &isSpecialIndex, &notSpecialIndex);
        Bind(&isSpecialIndex);
        {
            returnValue = GetHoleConstant(VariableType::INT64());
            Jump(&exit);
        }
        Bind(&notSpecialIndex);
        {
            GateRef elements = GetElementsArray(*holder);
            Label isDictionaryElement(env);
            Label notDictionaryElement(env);
            Branch(IsDictionaryElement(hclass), &isDictionaryElement, &notDictionaryElement);
            Bind(&notDictionaryElement);
            {
                Label isReceiver(env);
                Label notReceiver(env);
                Branch(Int64Equal(*holder, receiver), &isReceiver, &notReceiver);
                Bind(&isReceiver);
                {
                    GateRef length = GetLengthOfTaggedArray(elements);
                    Label inRange(env);
                    Branch(Int64LessThan(index, length), &inRange, &loopExit);
                    Bind(&inRange);
                    {
                        GateRef value1 = GetValueFromTaggedArray(VariableType::JS_ANY(), elements, index);
                        Label notHole(env);
                        Branch(Int64NotEqual(value1, GetHoleConstant()), &notHole, &loopExit);
                        Bind(&notHole);
                        {
                            SetValueToTaggedArray(VariableType::JS_ANY(), glue, elements, index, value);
                            returnValue = GetUndefinedConstant(VariableType::INT64());
                            Jump(&exit);
                        }
                    }
                }
                Bind(&notReceiver);
                Jump(&afterLoop);
            }
            Bind(&isDictionaryElement);
            returnValue = GetHoleConstant(VariableType::INT64());
            Jump(&exit);
        }
        Bind(&loopExit);
        {
            holder = GetPrototypeFromHClass(LoadHClass(*holder));
            Branch(TaggedIsHeapObject(*holder), &loopEnd, &afterLoop);
        }
    }
    Bind(&loopEnd);
    LoopEnd(&loopHead);
    Bind(&afterLoop);
    {
        Label isExtensible(env);
        Label notExtensible(env);
        Branch(IsExtensible(receiver), &isExtensible, &notExtensible);
        Bind(&isExtensible);
        {
            GateRef result =
                CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(AddElementInternal)), {
                        receiver, IntBuildTaggedTypeWithNoGC(index), value,
                        IntBuildTaggedTypeWithNoGC(GetInt32Constant(PropertyAttributes::GetDefaultAttributes()))
                    });
            Label success(env);
            Label failed(env);
            Branch(TaggedIsTrue(result), &success, &failed);
            Bind(&success);
            returnValue = GetUndefinedConstant(VariableType::INT64());
            Jump(&exit);
            Bind(&failed);
            returnValue = GetExceptionConstant(VariableType::INT64());
            Jump(&exit);
        }
        Bind(&notExtensible);
        {
            GateRef taggedId = GetInt32Constant(GET_MESSAGE_STRING_ID(SetPropertyWhenNotExtensible));
            CallRuntimeTrampoline(glue,
                GetInt64Constant(RTSTUB_ID(ThrowTypeError)), { IntBuildTaggedTypeWithNoGC(taggedId) });
            returnValue = GetExceptionConstant(VariableType::INT64());
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *returnValue;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::SetPropertyByName(GateRef glue, GateRef receiver, GateRef key, GateRef value)
{
    auto env = GetEnvironment();
    Label entryPass(env);
    env->PushCurrentLabel(&entryPass);
    DEFVARIABLE(result, VariableType::INT64(), GetHoleConstant(VariableType::INT64()));
    DEFVARIABLE(holder, VariableType::JS_POINTER(), receiver);
    Label exit(env);
    Label loopHead(env);
    Label loopEnd(env);
    Label loopExit(env);
    Label afterLoop(env);
    // a do-while loop
    Jump(&loopHead);
    LoopBegin(&loopHead);
    {
        // auto *hclass = holder.GetTaggedObject()->GetClass()
        // JSType jsType = hclass->GetObjectType()
        GateRef hClass = LoadHClass(*holder);
        GateRef jsType = GetObjectType(hClass);
        Label isSIndexObj(env);
        Label notSIndexObj(env);
        // if branch condition : IsSpecialIndexedObj(jsType)
        Branch(IsSpecialIndexedObj(jsType), &isSIndexObj, &notSIndexObj);
        Bind(&isSIndexObj);
        {
            Label isSpecialContainer(env);
            Label notSpecialContainer(env);
            // Add SpecialContainer
            Branch(IsSpecialContainer(jsType), &isSpecialContainer, &notSpecialContainer);
            Bind(&isSpecialContainer);
            {
                GateRef taggedId = GetInt32Constant(GET_MESSAGE_STRING_ID(CanNotSetPropertyOnContainer));
                CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(ThrowTypeError)),
                                      { IntBuildTaggedTypeWithNoGC(taggedId) });
                result = GetExceptionConstant(VariableType::INT64());
                Jump(&exit);
            }
            Bind(&notSpecialContainer);
            {
                result = GetHoleConstant(VariableType::INT64());
                Jump(&exit);
            }
        }
        Bind(&notSIndexObj);
        {
            Label isDicMode(env);
            Label notDicMode(env);
            // if branch condition : LIKELY(!hclass->IsDictionaryMode())
            Branch(IsDictionaryModeByHClass(hClass), &isDicMode, &notDicMode);
            Bind(&notDicMode);
            {
                // LayoutInfo *layoutInfo = LayoutInfo::Cast(hclass->GetAttributes().GetTaggedObject())
                GateRef layOutInfo = GetLayoutFromHClass(hClass);
                // int propsNumber = hclass->NumberOfPropsFromHClass()
                GateRef propsNum = GetNumberOfPropsFromHClass(hClass);
                // int entry = layoutInfo->FindElementWithCache(thread, hclass, key, propsNumber)
                GateRef entry = FindElementWithCache(glue, layOutInfo, hClass, key, propsNum);
                Label hasEntry(env);
                Label noEntry(env);
                // if branch condition : entry != -1
                Branch(Int32NotEqual(entry, GetInt32Constant(-1)), &hasEntry, &noEntry);
                Bind(&hasEntry);
                {
                    // PropertyAttributes attr(layoutInfo->GetAttr(entry))
                    GateRef propAttr = GetPropAttrFromLayoutInfo(layOutInfo, entry);
                    GateRef attr = TaggedCastToInt32(propAttr);
                    Label isAccessor(env);
                    Label notAccessor(env);
                    Label afterIsAccessor(env);
                    Branch(IsAccessor(attr), &isAccessor, &notAccessor);
                    Bind(&isAccessor);
                    {
                        // auto accessor = JSObject::Cast(holder)->GetProperty(hclass, attr)
                        GateRef accessor = JSObjectGetProperty(VariableType::JS_ANY(), *holder, hClass, attr);
                        Label shouldCall(env);
                        Label shouldNotCall(env);
                        // ShouldCallSetter(receiver, *holder, accessor, attr)
                        Branch(ShouldCallSetter(receiver, *holder, accessor, attr), &shouldCall, &shouldNotCall);
                        {
                            Bind(&shouldCall);
                            result = CallSetterUtil(glue, receiver, accessor, value);
                            Jump(&exit);
                            Bind(&shouldNotCall);
                            Jump(&afterIsAccessor);
                        }
                    }
                    Bind(&notAccessor);
                    Jump(&afterIsAccessor);
                    Bind(&afterIsAccessor);
                    {
                        Label writable(env);
                        Label notWritable(env);
                        Label afterIsWritable(env);
                        Branch(IsWritable(attr), &writable, &notWritable);
                        {
                            Bind(&writable);
                            Jump(&afterIsWritable);
                            Bind(&notWritable);
                            GateRef taggedId = GetInt32Constant(GET_MESSAGE_STRING_ID(SetPropertyWhenNotExtensible));
                            CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(ThrowTypeError)),
                                { IntBuildTaggedTypeWithNoGC(taggedId) });
                            result = GetExceptionConstant(VariableType::INT64());
                            Jump(&exit);
                        }
                        Bind(&afterIsWritable);
                        {
                            Label holdEqualsRecv(env);
                            Label holdNotEqualsRecv(env);
                            Label afterComp(env);
                            Branch(Int64Equal(*holder, receiver), &holdEqualsRecv, &holdNotEqualsRecv);
                            {
                                Bind(&holdEqualsRecv);
                                Jump(&afterComp);
                                Bind(&holdNotEqualsRecv);
                                Jump(&afterLoop);
                            }
                            Bind(&afterComp);
                            // JSObject::Cast(holder)->SetProperty(thread, hclass, attr, value)
                            // return JSTaggedValue::Undefined()
                            JSObjectSetProperty(glue, *holder, hClass, attr, value);
                            result = GetUndefinedConstant(VariableType::INT64());
                            Jump(&exit);
                        }
                    }
                }
                Bind(&noEntry);
                {
                    Jump(&loopExit);
                }
            }
            Bind(&isDicMode);
            {
                GateRef array = GetPropertiesArray(*holder);
                // int entry = dict->FindEntry(key)
                GateRef entry1 = FindEntryFromNameDictionary(glue, array, key);
                Label notNegtiveOne(env);
                Label negtiveOne(env);
                // if branch condition : entry != -1
                Branch(Int32NotEqual(entry1, GetInt32Constant(-1)), &notNegtiveOne, &negtiveOne);
                Bind(&notNegtiveOne);
                {
                    // auto attr = dict->GetAttributes(entry)
                    GateRef attr1 = GetAttributesFromDictionary<NameDictionary>(array, entry1);
                    Label isAccessor1(env);
                    Label notAccessor1(env);
                    Label afterIsAccessor1(env);
                    // if branch condition : UNLIKELY(attr.IsAccessor())
                    Branch(IsAccessor(attr1), &isAccessor1, &notAccessor1);
                    Bind(&isAccessor1);
                    {
                        Label isInternal1(env);
                        Label notInternal1(env);
                        // auto accessor = dict->GetValue(entry)
                        GateRef accessor1 = GetValueFromDictionary<NameDictionary>(VariableType::JS_ANY(),
                            array, entry1);
                        Label shouldCall1(env);
                        Label shouldNotCall1(env);
                        Branch(ShouldCallSetter(receiver, *holder, accessor1, attr1), &shouldCall1, &shouldNotCall1);
                        Bind(&shouldCall1);
                        result = CallSetterUtil(glue, receiver, accessor1, value);
                        Jump(&exit);
                        Bind(&shouldNotCall1);
                        Jump(&afterIsAccessor1);
                    }
                    Bind(&notAccessor1);
                    Jump(&afterIsAccessor1);
                    Bind(&afterIsAccessor1);
                    {
                        Label writable1(env);
                        Label notWritable1(env);
                        Label afterIsWritable1(env);
                        Branch(IsWritable(attr1), &writable1, &notWritable1);
                        {
                            Bind(&writable1);
                            Jump(&afterIsWritable1);
                            Bind(&notWritable1);
                            GateRef taggedId = GetInt32Constant(GET_MESSAGE_STRING_ID(SetPropertyWhenNotExtensible));
                            CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(ThrowTypeError)),
                                { IntBuildTaggedTypeWithNoGC(taggedId) });
                            result = GetExceptionConstant(VariableType::INT64());
                            Jump(&exit);
                        }
                        Bind(&afterIsWritable1);
                        {
                            Label holdEqualsRecv1(env);
                            Label holdNotEqualsRecv1(env);
                            Label afterComp1(env);
                            Branch(Int64Equal(*holder, receiver), &holdEqualsRecv1, &holdNotEqualsRecv1);
                            {
                                Bind(&holdEqualsRecv1);
                                Jump(&afterComp1);
                                Bind(&holdNotEqualsRecv1);
                                Jump(&afterLoop);
                            }
                            Bind(&afterComp1);
                            // dict->UpdateValue(thread, entry, value)
                            // return JSTaggedValue::Undefined()
                            UpdateValueInDict(glue, array, entry1, value);
                            result = GetUndefinedConstant(VariableType::INT64());
                            Jump(&exit);
                        }
                    }
                }
                Bind(&negtiveOne);
                Jump(&loopExit);
            }
            Bind(&loopExit);
            {
                // holder = hclass->GetPrototype()
                holder = GetPrototypeFromHClass(LoadHClass(*holder));
                // loop condition for a do-while loop
                Branch(TaggedIsHeapObject(*holder), &loopEnd, &afterLoop);
            }
        }
    }
    Bind(&loopEnd);
    LoopEnd(&loopHead);
    Bind(&afterLoop);
    {
        Label extensible(env);
        Label inextensible(env);
        Label afterExtenCon(env);
        Branch(IsExtensible(receiver), &extensible, &inextensible);
        {
            Bind(&extensible);
            Jump(&afterExtenCon);
            Bind(&inextensible);
            GateRef taggedId = GetInt32Constant(GET_MESSAGE_STRING_ID(SetPropertyWhenNotExtensible));
            CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(ThrowTypeError)),
                { IntBuildTaggedTypeWithNoGC(taggedId) });
            result = GetExceptionConstant(VariableType::INT64());
            Jump(&exit);
        }
        Bind(&afterExtenCon);
        result = AddPropertyByName(glue, receiver, key, value,
                                   GetInt32Constant(PropertyAttributes::GetDefaultAttributes()));
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::SetPropertyByNameWithOwn(GateRef glue, GateRef receiver, GateRef key, GateRef value)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    DEFVARIABLE(result, VariableType::INT64(), GetHoleConstant(VariableType::INT64()));
    DEFVARIABLE(holder, VariableType::JS_POINTER(), receiver);
    Label exit(env);
    Label ifEnd(env);
    // auto *hclass = holder.GetTaggedObject()->GetClass()
    // JSType jsType = hclass->GetObjectType()
    GateRef hClass = LoadHClass(*holder);
    GateRef jsType = GetObjectType(hClass);
    Label isSIndexObj(env);
    Label notSIndexObj(env);
    // if branch condition : IsSpecialIndexedObj(jsType)
    Branch(IsSpecialIndexedObj(jsType), &isSIndexObj, &notSIndexObj);
    Bind(&isSIndexObj);
    {
        Label isSpecialContainer(env);
        Label notSpecialContainer(env);
        // Add SpecialContainer
        Branch(IsSpecialContainer(jsType), &isSpecialContainer, &notSpecialContainer);
        Bind(&isSpecialContainer);
        {
            GateRef taggedId = GetInt32Constant(GET_MESSAGE_STRING_ID(CanNotSetPropertyOnContainer));
            CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(ThrowTypeError)),
                { IntBuildTaggedTypeWithNoGC(taggedId) });
            result = GetExceptionConstant(VariableType::INT64());
            Jump(&exit);
        }
        Bind(&notSpecialContainer);
        {
            result = GetHoleConstant(VariableType::INT64());
            Jump(&exit);
        }
    }
    Bind(&notSIndexObj);
    {
        Label isDicMode(env);
        Label notDicMode(env);
        // if branch condition : LIKELY(!hclass->IsDictionaryMode())
        Branch(IsDictionaryModeByHClass(hClass), &isDicMode, &notDicMode);
        Bind(&notDicMode);
        {
            // LayoutInfo *layoutInfo = LayoutInfo::Cast(hclass->GetAttributes().GetTaggedObject())
            GateRef layOutInfo = GetLayoutFromHClass(hClass);
            // int propsNumber = hclass->NumberOfPropsFromHClass()
            GateRef propsNum = GetNumberOfPropsFromHClass(hClass);
            // int entry = layoutInfo->FindElementWithCache(thread, hclass, key, propsNumber)
            GateRef entry = FindElementWithCache(glue, layOutInfo, hClass, key, propsNum);
            Label hasEntry(env);
            Label noEntry(env);
            // if branch condition : entry != -1
            Branch(Int32NotEqual(entry, GetInt32Constant(-1)), &hasEntry, &noEntry);
            Bind(&hasEntry);
            {
                // PropertyAttributes attr(layoutInfo->GetAttr(entry))
                GateRef propAttr = GetPropAttrFromLayoutInfo(layOutInfo, entry);
                GateRef attr = TaggedCastToInt32(propAttr);
                Label isAccessor(env);
                Label notAccessor(env);
                Label afterIsAccessor(env);
                Branch(IsAccessor(attr), &isAccessor, &notAccessor);
                Bind(&isAccessor);
                {
                    // auto accessor = JSObject::Cast(holder)->GetProperty(hclass, attr)
                    GateRef accessor = JSObjectGetProperty(VariableType::JS_ANY(), *holder, hClass, attr);
                    Label shouldCall(env);
                    Label shouldNotCall(env);
                    // ShouldCallSetter(receiver, *holder, accessor, attr)
                    Branch(ShouldCallSetter(receiver, *holder, accessor, attr), &shouldCall, &shouldNotCall);
                    {
                        Bind(&shouldCall);
                        result = CallSetterUtil(glue, receiver, accessor, value);
                        Jump(&exit);
                        Bind(&shouldNotCall);
                        Jump(&afterIsAccessor);
                    }
                }
                Bind(&notAccessor);
                Jump(&afterIsAccessor);
                Bind(&afterIsAccessor);
                {
                    Label writable(env);
                    Label notWritable(env);
                    Label afterIsWritable(env);
                    Branch(IsWritable(attr), &writable, &notWritable);
                    {
                        Bind(&writable);
                        Jump(&afterIsWritable);
                        Bind(&notWritable);
                        GateRef taggedId = GetInt32Constant(GET_MESSAGE_STRING_ID(SetPropertyWhenNotExtensible));
                        CallRuntimeTrampoline(glue,
                            GetInt64Constant(RTSTUB_ID(ThrowTypeError)), { IntBuildTaggedTypeWithNoGC(taggedId) });
                        result = GetExceptionConstant(VariableType::INT64());
                        Jump(&exit);
                    }
                    Bind(&afterIsWritable);
                    {
                        Label holdEqualsRecv(env);
                        Label holdNotEqualsRecv(env);
                        Label afterComp(env);
                        Branch(Int64Equal(*holder, receiver), &holdEqualsRecv, &holdNotEqualsRecv);
                        {
                            Bind(&holdEqualsRecv);
                            Jump(&afterComp);
                            Bind(&holdNotEqualsRecv);
                            Jump(&ifEnd);
                        }
                        Bind(&afterComp);
                        // JSObject::Cast(holder)->SetProperty(thread, hclass, attr, value)
                        // return JSTaggedValue::Undefined()
                        JSObjectSetProperty(glue, *holder, hClass, attr, value);
                        result = GetUndefinedConstant(VariableType::INT64());
                        Jump(&exit);
                    }
                }
            }
            Bind(&noEntry);
            {
                Jump(&ifEnd);
            }
        }
        Bind(&isDicMode);
        {
            // TaggedArray *array = TaggedArray::Cast(JSObject::Cast(holder)->GetPropertiesArray().GetTaggedObject())
            GateRef array = GetPropertiesArray(*holder);
            // int entry = dict->FindEntry(key)
            GateRef entry1 = FindEntryFromNameDictionary(glue, array, key);
            Label notNegtiveOne(env);
            Label negtiveOne(env);
            // if branch condition : entry != -1
            Branch(Int32NotEqual(entry1, GetInt32Constant(-1)), &notNegtiveOne, &negtiveOne);
            Bind(&notNegtiveOne);
            {
                // auto attr = dict->GetAttributes(entry)
                GateRef attr1 = GetAttributesFromDictionary<NameDictionary>(array, entry1);
                Label isAccessor1(env);
                Label notAccessor1(env);
                Label afterIsAccessor1(env);
                // if branch condition : UNLIKELY(attr.IsAccessor())
                Branch(IsAccessor(attr1), &isAccessor1, &notAccessor1);
                Bind(&isAccessor1);
                {
                    Label isInternal1(env);
                    Label notInternal1(env);
                    // auto accessor = dict->GetValue(entry)
                    GateRef accessor1 = GetValueFromDictionary<NameDictionary>(VariableType::JS_ANY(), array, entry1);
                    Label shouldCall1(env);
                    Label shouldNotCall1(env);
                    Branch(ShouldCallSetter(receiver, *holder, accessor1, attr1), &shouldCall1, &shouldNotCall1);
                    Bind(&shouldCall1);
                    result = CallSetterUtil(glue, receiver, accessor1, value);
                    Jump(&exit);
                    Bind(&shouldNotCall1);
                    Jump(&afterIsAccessor1);
                }
                Bind(&notAccessor1);
                Jump(&afterIsAccessor1);
                Bind(&afterIsAccessor1);
                {
                    Label writable1(env);
                    Label notWritable1(env);
                    Label afterIsWritable1(env);
                    Branch(IsWritable(attr1), &writable1, &notWritable1);
                    {
                        Bind(&writable1);
                        Jump(&afterIsWritable1);
                        Bind(&notWritable1);
                        GateRef taggedId = GetInt32Constant(GET_MESSAGE_STRING_ID(SetPropertyWhenNotExtensible));
                        CallRuntimeTrampoline(glue,
                            GetInt64Constant(RTSTUB_ID(ThrowTypeError)), { IntBuildTaggedTypeWithNoGC(taggedId) });
                        result = GetExceptionConstant(VariableType::INT64());
                        Jump(&exit);
                    }
                    Bind(&afterIsWritable1);
                    {
                        Label holdEqualsRecv1(env);
                        Label holdNotEqualsRecv1(env);
                        Label afterComp1(env);
                        Branch(Int64Equal(*holder, receiver), &holdEqualsRecv1, &holdNotEqualsRecv1);
                        {
                            Bind(&holdEqualsRecv1);
                            Jump(&afterComp1);
                            Bind(&holdNotEqualsRecv1);
                            Jump(&ifEnd);
                        }
                        Bind(&afterComp1);
                        // dict->UpdateValue(thread, entry, value)
                        // return JSTaggedValue::Undefined()
                        UpdateValueInDict(glue, array, entry1, value);
                        result = GetUndefinedConstant(VariableType::INT64());
                        Jump(&exit);
                    }
                }
            }
            Bind(&negtiveOne);
            Jump(&ifEnd);
        }
    }
    Bind(&ifEnd);
    {
        Label extensible(env);
        Label inextensible(env);
        Label afterExtenCon(env);
        Branch(IsExtensible(receiver), &extensible, &inextensible);
        {
            Bind(&extensible);
            Jump(&afterExtenCon);
            Bind(&inextensible);
            GateRef taggedId = GetInt32Constant(GET_MESSAGE_STRING_ID(SetPropertyWhenNotExtensible));
            CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(ThrowTypeError)),
                { IntBuildTaggedTypeWithNoGC(taggedId) });
            result = GetExceptionConstant(VariableType::INT64());
            Jump(&exit);
        }
        Bind(&afterExtenCon);
        result = AddPropertyByName(glue, receiver, key, value,
                                   GetInt32Constant(PropertyAttributes::GetDefaultAttributes()));
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

void Stub::NotifyHClassChanged(GateRef glue, GateRef oldHClass, GateRef newHClass)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    Label isProtoType(env);
    Branch(IsProtoTypeHClass(oldHClass), &isProtoType, &exit);
    Bind(&isProtoType);
    {
        Label notEqualHClass(env);
        Branch(Int64Equal(oldHClass, newHClass), &exit, &notEqualHClass);
        Bind(&notEqualHClass);
        {
            SetIsProtoTypeToHClass(glue, newHClass, TrueConstant());
            CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(NoticeThroughChainAndRefreshUser)), {
                    oldHClass, newHClass
                });
            Jump(&exit);
        }
    }
    Bind(&exit);
    env->PopCurrentLabel();
    return;
}

GateRef Stub::GetContainerProperty(GateRef glue, GateRef receiver, GateRef index, GateRef jsType)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), GetUndefinedConstant());

    Label arrayListLabel(env);
    Label queueLabel(env);
    Label defaultLabel(env);
    std::array<Label, 2> repCaseLabels = { // 2 : 2 means that there are 2 args in total.
        arrayListLabel,
        queueLabel,
    };
    std::array<int64_t, 2> keyValues = { // 2 : 2 means that there are 2 args in total.
        static_cast<int64_t>(JSType::JS_API_ARRAY_LIST),
        static_cast<int64_t>(JSType::JS_API_QUEUE),
    };
    // 2 : 2 means that there are 2 cases.
    Switch(ZExtInt32ToInt64(jsType), &defaultLabel, keyValues.data(), repCaseLabels.data(), 2);
    Bind(&arrayListLabel);
    {
        result = JSArrayListGet(glue, receiver, index);
        Jump(&exit);
    }
    Bind(&queueLabel);
    {
        Jump(&exit);
    }
    Bind(&defaultLabel);
    {
        Jump(&exit);
    }

    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::FastTypeOf(GateRef glue, GateRef obj)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);

    GateRef gConstOffset = IntPtrAdd(glue,
                                     GetIntPtrConstant(JSThread::GlueData::GetGlobalConstOffset(env->Is32Bit())));
    GateRef booleanIndex = GetGlobalConstantString(ConstantIndex::UNDEFINED_STRING_INDEX);
    GateRef gConstUndefindStr = Load(VariableType::JS_POINTER(), gConstOffset, booleanIndex);
    DEFVARIABLE(resultRep, VariableType::JS_POINTER(), gConstUndefindStr);
    Label objIsTrue(env);
    Label objNotTrue(env);
    Label defaultLabel(env);
    GateRef gConstBooleanStr = Load(
        VariableType::JS_POINTER(), gConstOffset, GetGlobalConstantString(ConstantIndex::BOOLEAN_STRING_INDEX));
    Branch(Int64Equal(obj, GetInt64Constant(JSTaggedValue::VALUE_TRUE)), &objIsTrue, &objNotTrue);
    Bind(&objIsTrue);
    {
        resultRep = gConstBooleanStr;
        Jump(&exit);
    }
    Bind(&objNotTrue);
    {
        Label objIsFalse(env);
        Label objNotFalse(env);
        Branch(Int64Equal(obj, GetInt64Constant(JSTaggedValue::VALUE_FALSE)), &objIsFalse, &objNotFalse);
        Bind(&objIsFalse);
        {
            resultRep = gConstBooleanStr;
            Jump(&exit);
        }
        Bind(&objNotFalse);
        {
            Label objIsNull(env);
            Label objNotNull(env);
            Branch(Int64Equal(obj, GetInt64Constant(JSTaggedValue::VALUE_NULL)), &objIsNull, &objNotNull);
            Bind(&objIsNull);
            {
                resultRep = Load(
                    VariableType::JS_POINTER(), gConstOffset,
                    GetGlobalConstantString(ConstantIndex::OBJECT_STRING_INDEX));
                Jump(&exit);
            }
            Bind(&objNotNull);
            {
                Label objIsUndefined(env);
                Label objNotUndefined(env);
                Branch(Int64Equal(obj, GetInt64Constant(JSTaggedValue::VALUE_UNDEFINED)), &objIsUndefined,
                    &objNotUndefined);
                Bind(&objIsUndefined);
                {
                    resultRep = Load(VariableType::JS_POINTER(), gConstOffset,
                        GetGlobalConstantString(ConstantIndex::UNDEFINED_STRING_INDEX));
                    Jump(&exit);
                }
                Bind(&objNotUndefined);
                Jump(&defaultLabel);
            }
        }
    }
    Bind(&defaultLabel);
    {
        Label objIsHeapObject(env);
        Label objNotHeapObject(env);
        Branch(TaggedIsHeapObject(obj), &objIsHeapObject, &objNotHeapObject);
        Bind(&objIsHeapObject);
        {
            Label objIsString(env);
            Label objNotString(env);
            Branch(IsString(obj), &objIsString, &objNotString);
            Bind(&objIsString);
            {
                resultRep = Load(
                    VariableType::JS_POINTER(), gConstOffset,
                    GetGlobalConstantString(ConstantIndex::STRING_STRING_INDEX));
                Jump(&exit);
            }
            Bind(&objNotString);
            {
                Label objIsSymbol(env);
                Label objNotSymbol(env);
                Branch(IsSymbol(obj), &objIsSymbol, &objNotSymbol);
                Bind(&objIsSymbol);
                {
                    resultRep = Load(VariableType::JS_POINTER(), gConstOffset,
                        GetGlobalConstantString(ConstantIndex::SYMBOL_STRING_INDEX));
                    Jump(&exit);
                }
                Bind(&objNotSymbol);
                {
                    Label objIsCallable(env);
                    Label objNotCallable(env);
                    Branch(IsCallable(obj), &objIsCallable, &objNotCallable);
                    Bind(&objIsCallable);
                    {
                        resultRep = Load(
                            VariableType::JS_POINTER(), gConstOffset,
                            GetGlobalConstantString(ConstantIndex::FUNCTION_STRING_INDEX));
                        Jump(&exit);
                    }
                    Bind(&objNotCallable);
                    {
                        resultRep = Load(
                            VariableType::JS_POINTER(), gConstOffset,
                            GetGlobalConstantString(ConstantIndex::OBJECT_STRING_INDEX));
                        Jump(&exit);
                    }
                }
            }
        }
        Bind(&objNotHeapObject);
        {
            Label objIsNum(env);
            Label objNotNum(env);
            Branch(TaggedIsNumber(obj), &objIsNum, &objNotNum);
            Bind(&objIsNum);
            {
                resultRep = Load(
                    VariableType::JS_POINTER(), gConstOffset,
                    GetGlobalConstantString(ConstantIndex::NUMBER_STRING_INDEX));
                Jump(&exit);
            }
            Bind(&objNotNum);
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *resultRep;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::FastEqual(GateRef left, GateRef right)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), GetHoleConstant());
    Label leftEqualRight(env);
    Label leftNotEqualRight(env);
    Label exit(env);
    Branch(Int64Equal(left, right), &leftEqualRight, &leftNotEqualRight);
    Bind(&leftEqualRight);
    {
        Label leftIsDouble(env);
        Label leftNotDoubleOrLeftNotNan(env);
        Branch(TaggedIsDouble(left), &leftIsDouble, &leftNotDoubleOrLeftNotNan);
        Bind(&leftIsDouble);
        {
            GateRef doubleLeft = TaggedCastToDouble(left);
            Label leftIsNan(env);
            Branch(DoubleIsNAN(doubleLeft), &leftIsNan, &leftNotDoubleOrLeftNotNan);
            Bind(&leftIsNan);
            {
                result = ChangeInt64ToTagged(TaggedFalse());
                Jump(&exit);
            }
        }
        Bind(&leftNotDoubleOrLeftNotNan);
        {
            result = ChangeInt64ToTagged(TaggedTrue());
            Jump(&exit);
        }
    }
    Bind(&leftNotEqualRight);
    {
        Label leftIsNumber(env);
        Label leftNotNumberOrLeftNotIntOrRightNotInt(env);
        Branch(TaggedIsNumber(left), &leftIsNumber, &leftNotNumberOrLeftNotIntOrRightNotInt);
        Bind(&leftIsNumber);
        {
            Label leftIsInt(env);
            Branch(TaggedIsInt(left), &leftIsInt, &leftNotNumberOrLeftNotIntOrRightNotInt);
            Bind(&leftIsInt);
            {
                Label rightIsInt(env);
                Branch(TaggedIsInt(right), &rightIsInt, &leftNotNumberOrLeftNotIntOrRightNotInt);
                Bind(&rightIsInt);
                {
                    result = ChangeInt64ToTagged(TaggedFalse());
                    Jump(&exit);
                }
            }
        }
        Bind(&leftNotNumberOrLeftNotIntOrRightNotInt);
        {
            Label rightIsUndefinedOrNull(env);
            Label leftOrRightNotUndefinedOrNull(env);
            Branch(TaggedIsUndefinedOrNull(right), &rightIsUndefinedOrNull, &leftOrRightNotUndefinedOrNull);
            Bind(&rightIsUndefinedOrNull);
            {
                Label leftIsHeapObject(env);
                Label leftNotHeapObject(env);
                Branch(TaggedIsHeapObject(left), &leftIsHeapObject, &leftNotHeapObject);
                Bind(&leftIsHeapObject);
                {
                    result = ChangeInt64ToTagged(TaggedFalse());
                    Jump(&exit);
                }
                Bind(&leftNotHeapObject);
                {
                    Label leftIsUndefinedOrNull(env);
                    Branch(TaggedIsUndefinedOrNull(left), &leftIsUndefinedOrNull, &leftOrRightNotUndefinedOrNull);
                    Bind(&leftIsUndefinedOrNull);
                    {
                        result = ChangeInt64ToTagged(TaggedTrue());
                        Jump(&exit);
                    }
                }
            }
            Bind(&leftOrRightNotUndefinedOrNull);
            {
                Label leftIsBool(env);
                Label leftNotBoolOrRightNotSpecial(env);
                Branch(TaggedIsBoolean(left), &leftIsBool, &leftNotBoolOrRightNotSpecial);
                Bind(&leftIsBool);
                {
                    Label rightIsSpecial(env);
                    Branch(TaggedIsSpecial(right), &rightIsSpecial, &leftNotBoolOrRightNotSpecial);
                    Bind(&rightIsSpecial);
                    {
                        result = ChangeInt64ToTagged(TaggedFalse());
                        Jump(&exit);
                    }
                }
                Bind(&leftNotBoolOrRightNotSpecial);
                {
                    Jump(&exit);
                }
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::FastDiv(GateRef left, GateRef right)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), GetHoleConstant());
    DEFVARIABLE(doubleLeft, VariableType::FLOAT64(), GetDoubleConstant(0));
    DEFVARIABLE(doubleRight, VariableType::FLOAT64(), GetDoubleConstant(0));
    Label leftIsNumber(env);
    Label leftNotNumberOrRightNotNumber(env);
    Label leftIsNumberAndRightIsNumber(env);
    Label leftIsDoubleAndRightIsDouble(env);
    Label exit(env);
    Branch(TaggedIsNumber(left), &leftIsNumber, &leftNotNumberOrRightNotNumber);
    Bind(&leftIsNumber);
    {
        Label rightIsNumber(env);
        Branch(TaggedIsNumber(right), &rightIsNumber, &leftNotNumberOrRightNotNumber);
        Bind(&rightIsNumber);
        {
            Label leftIsInt(env);
            Label leftNotInt(env);
            Branch(TaggedIsInt(left), &leftIsInt, &leftNotInt);
            Bind(&leftIsInt);
            {
                doubleLeft = ChangeInt32ToFloat64(TaggedCastToInt32(left));
                Jump(&leftIsNumberAndRightIsNumber);
            }
            Bind(&leftNotInt);
            {
                doubleLeft = TaggedCastToDouble(left);
                Jump(&leftIsNumberAndRightIsNumber);
            }
        }
    }
    Bind(&leftNotNumberOrRightNotNumber);
    {
        Jump(&exit);
    }
    Bind(&leftIsNumberAndRightIsNumber);
    {
        Label rightIsInt(env);
        Label rightNotInt(env);
        Branch(TaggedIsInt(right), &rightIsInt, &rightNotInt);
        Bind(&rightIsInt);
        {
            doubleRight = ChangeInt32ToFloat64(TaggedCastToInt32(right));
            Jump(&leftIsDoubleAndRightIsDouble);
        }
        Bind(&rightNotInt);
        {
            doubleRight = TaggedCastToDouble(right);
            Jump(&leftIsDoubleAndRightIsDouble);
        }
    }
    Bind(&leftIsDoubleAndRightIsDouble);
    {
        Label rightIsZero(env);
        Label rightNotZero(env);
        Branch(DoubleEqual(*doubleRight, GetDoubleConstant(0.0)), &rightIsZero, &rightNotZero);
        Bind(&rightIsZero);
        {
            Label leftIsZero(env);
            Label leftNotZero(env);
            Label leftIsZeroOrNan(env);
            Label leftNotZeroAndNotNan(env);
            Branch(DoubleEqual(*doubleLeft, GetDoubleConstant(0.0)), &leftIsZero, &leftNotZero);
            Bind(&leftIsZero);
            {
                Jump(&leftIsZeroOrNan);
            }
            Bind(&leftNotZero);
            {
                Label leftIsNan(env);
                Branch(DoubleIsNAN(*doubleLeft), &leftIsNan, &leftNotZeroAndNotNan);
                Bind(&leftIsNan);
                {
                    Jump(&leftIsZeroOrNan);
                }
            }
            Bind(&leftIsZeroOrNan);
            {
                result = DoubleBuildTaggedWithNoGC(GetDoubleConstant(base::NAN_VALUE));
                Jump(&exit);
            }
            Bind(&leftNotZeroAndNotNan);
            {
                GateRef intLeftTmp = CastDoubleToInt64(*doubleLeft);
                GateRef intRightTmp = CastDoubleToInt64(*doubleRight);
                GateRef flagBit = Int64And(Int64Xor(intLeftTmp, intRightTmp), GetInt64Constant(base::DOUBLE_SIGN_MASK));
                GateRef tmpResult = Int64Xor(flagBit, CastDoubleToInt64(GetDoubleConstant(base::POSITIVE_INFINITY)));
                result = DoubleBuildTaggedWithNoGC(CastInt64ToFloat64(tmpResult));
                Jump(&exit);
            }
        }
        Bind(&rightNotZero);
        {
            result = DoubleBuildTaggedWithNoGC(DoubleDiv(*doubleLeft, *doubleRight));
            Jump(&exit);
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

template<OpCode::Op Op>
GateRef Stub::FastBinaryOp(GateRef left, GateRef right)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), GetHoleConstant());
    DEFVARIABLE(doubleLeft, VariableType::FLOAT64(), GetDoubleConstant(0));
    DEFVARIABLE(doubleRight, VariableType::FLOAT64(), GetDoubleConstant(0));

    Label exit(env);
    Label doFloatOp(env);
    Label leftIsNumber(env);
    Label rightIsNumber(env);
    Label leftIsInt(env);
    Label leftIsDouble(env);
    Label leftIsIntRightIsInt(env);
    Label leftIsIntRightIsDouble(env);
    Label rightIsInt(env);
    Label rightIsDouble(env);
    Label overflow(env);
    Label notOverflow(env);

    Branch(TaggedIsNumber(left), &leftIsNumber, &exit);
    Bind(&leftIsNumber);
    {
        Branch(TaggedIsNumber(right), &rightIsNumber, &exit);
        Bind(&rightIsNumber);
        {
            Label leftIsInt(env);
            Label leftIsDouble(env);
            Branch(TaggedIsInt(left), &leftIsInt, &leftIsDouble);
            Bind(&leftIsInt);
            {
                Branch(TaggedIsInt(right), &leftIsIntRightIsInt, &leftIsIntRightIsDouble);
                Bind(&leftIsIntRightIsInt);
                {
                    auto res = BinaryOp<Op, MachineType::I64>(TaggedCastToInt64(left), TaggedCastToInt64(right));
                    auto condition1 = Int64GreaterThan(res, GetInt64Constant(INT32_MAX));
                    auto condition2 = Int64LessThan(res, GetInt64Constant(INT32_MIN));
                    Branch(BoolOr(condition1, condition2), &overflow, &notOverflow);
                    Bind(&overflow);
                    {
                        doubleLeft = ChangeInt32ToFloat64(TaggedCastToInt32(left));
                        doubleRight = ChangeInt32ToFloat64(TaggedCastToInt32(right));
                        Jump(&doFloatOp);
                    }
                    Bind(&notOverflow);
                    {
                        result = IntBuildTaggedWithNoGC(ChangeInt64ToInt32(res));
                        Jump(&exit);
                    }
                }
                Bind(&leftIsIntRightIsDouble);
                {
                    doubleLeft = ChangeInt32ToFloat64(TaggedCastToInt32(left));
                    doubleRight = TaggedCastToDouble(right);
                    Jump(&doFloatOp);
                }
            }
            Bind(&leftIsDouble);
            {
                Branch(TaggedIsInt(right), &rightIsInt, &rightIsDouble);
                Bind(&rightIsInt);
                {
                    doubleLeft = TaggedCastToDouble(left);
                    doubleRight = ChangeInt32ToFloat64(TaggedCastToInt32(right));
                    Jump(&doFloatOp);
                }
                Bind(&rightIsDouble);
                {
                    doubleLeft = TaggedCastToDouble(left);
                    doubleRight = TaggedCastToDouble(right);
                    Jump(&doFloatOp);
                }
            }
        }
    }
    Bind(&doFloatOp);
    {
        auto res = BinaryOp<Op, MachineType::F64>(*doubleLeft, *doubleRight);
        result = DoubleBuildTaggedWithNoGC(res);
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::FastAdd(GateRef left, GateRef right)
{
    return FastBinaryOp<OpCode::ADD>(left, right);
}

GateRef Stub::FastSub(GateRef left, GateRef right)
{
    return FastBinaryOp<OpCode::SUB>(left, right);
}

GateRef Stub::FastMul(GateRef left, GateRef right)
{
    return FastBinaryOp<OpCode::MUL>(left, right);
}

GateRef Stub::FastMod(GateRef glue, GateRef left, GateRef right)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    DEFVARIABLE(result, VariableType::JS_ANY(), GetHoleConstant());
    DEFVARIABLE(intLeft, VariableType::INT32(), GetInt32Constant(0));
    DEFVARIABLE(intRight, VariableType::INT32(), GetInt32Constant(0));
    DEFVARIABLE(doubleLeft, VariableType::FLOAT64(), GetDoubleConstant(0));
    DEFVARIABLE(doubleRight, VariableType::FLOAT64(), GetDoubleConstant(0));
    Label leftIsInt(env);
    Label leftNotIntOrRightNotInt(env);
    Label exit(env);
    Branch(TaggedIsInt(left), &leftIsInt, &leftNotIntOrRightNotInt);
    Bind(&leftIsInt);
    {
        Label rightIsInt(env);
        Branch(TaggedIsInt(right), &rightIsInt, &leftNotIntOrRightNotInt);
        Bind(&rightIsInt);
        {
            intLeft = TaggedCastToInt32(left);
            intRight = TaggedCastToInt32(right);
            Label leftGreaterZero(env);
            Branch(Int32GreaterThan(*intLeft, GetInt32Constant(0)), &leftGreaterZero, &leftNotIntOrRightNotInt);
            Bind(&leftGreaterZero);
            {
                Label rightGreaterZero(env);
                Branch(Int32GreaterThan(*intRight, GetInt32Constant(0)), &rightGreaterZero, &leftNotIntOrRightNotInt);
                Bind(&rightGreaterZero);
                {
                    result = IntBuildTaggedWithNoGC(Int32Mod(*intLeft, *intRight));
                    Jump(&exit);
                }
            }
        }
    }
    Bind(&leftNotIntOrRightNotInt);
    {
        Label leftIsNumber(env);
        Label leftNotNumberOrRightNotNumber(env);
        Label leftIsNumberAndRightIsNumber(env);
        Label leftIsDoubleAndRightIsDouble(env);
        Branch(TaggedIsNumber(left), &leftIsNumber, &leftNotNumberOrRightNotNumber);
        Bind(&leftIsNumber);
        {
            Label rightIsNumber(env);
            Branch(TaggedIsNumber(right), &rightIsNumber, &leftNotNumberOrRightNotNumber);
            Bind(&rightIsNumber);
            {
                Label leftIsInt1(env);
                Label leftNotInt1(env);
                Branch(TaggedIsInt(left), &leftIsInt1, &leftNotInt1);
                Bind(&leftIsInt1);
                {
                    doubleLeft = ChangeInt32ToFloat64(TaggedCastToInt32(left));
                    Jump(&leftIsNumberAndRightIsNumber);
                }
                Bind(&leftNotInt1);
                {
                    doubleLeft = TaggedCastToDouble(left);
                    Jump(&leftIsNumberAndRightIsNumber);
                }
            }
        }
        Bind(&leftNotNumberOrRightNotNumber);
        {
            Jump(&exit);
        }
        Bind(&leftIsNumberAndRightIsNumber);
        {
            Label rightIsInt1(env);
            Label rightNotInt1(env);
            Branch(TaggedIsInt(right), &rightIsInt1, &rightNotInt1);
            Bind(&rightIsInt1);
            {
                doubleRight = ChangeInt32ToFloat64(TaggedCastToInt32(right));
                Jump(&leftIsDoubleAndRightIsDouble);
            }
            Bind(&rightNotInt1);
            {
                doubleRight = TaggedCastToDouble(right);
                Jump(&leftIsDoubleAndRightIsDouble);
            }
        }
        Bind(&leftIsDoubleAndRightIsDouble);
        {
            Label rightNotZero(env);
            Label rightIsZeroOrNanOrLeftIsNanOrInf(env);
            Label rightNotZeroAndNanAndLeftNotNanAndInf(env);
            Branch(DoubleEqual(*doubleRight, GetDoubleConstant(0.0)), &rightIsZeroOrNanOrLeftIsNanOrInf, &rightNotZero);
            Bind(&rightNotZero);
            {
                Label rightNotNan(env);
                Branch(DoubleIsNAN(*doubleRight), &rightIsZeroOrNanOrLeftIsNanOrInf, &rightNotNan);
                Bind(&rightNotNan);
                {
                    Label leftNotNan(env);
                    Branch(DoubleIsNAN(*doubleLeft), &rightIsZeroOrNanOrLeftIsNanOrInf, &leftNotNan);
                    Bind(&leftNotNan);
                    {
                        Branch(DoubleIsINF(*doubleLeft), &rightIsZeroOrNanOrLeftIsNanOrInf,
                            &rightNotZeroAndNanAndLeftNotNanAndInf);
                    }
                }
            }
            Bind(&rightIsZeroOrNanOrLeftIsNanOrInf);
            {
                result = DoubleBuildTaggedWithNoGC(GetDoubleConstant(base::NAN_VALUE));
                Jump(&exit);
            }
            Bind(&rightNotZeroAndNanAndLeftNotNanAndInf);
            {
                Label leftNotZero(env);
                Label leftIsZeroOrRightIsInf(env);
                Branch(DoubleEqual(*doubleLeft, GetDoubleConstant(0.0)), &leftIsZeroOrRightIsInf, &leftNotZero);
                Bind(&leftNotZero);
                {
                    Label rightNotInf(env);
                    Branch(DoubleIsINF(*doubleRight), &leftIsZeroOrRightIsInf, &rightNotInf);
                    Bind(&rightNotInf);
                    {
                        doubleLeft = TaggedCastToDouble(CallRuntimeTrampoline(glue,
                            GetInt64Constant(RTSTUB_ID(FloatMod)), { DoubleBuildTaggedTypeWithNoGC(*doubleLeft),
                                DoubleBuildTaggedTypeWithNoGC(*doubleRight)
                        }));
                        result = DoubleBuildTaggedWithNoGC(*doubleLeft);
                        Jump(&exit);
                    }
                }
                Bind(&leftIsZeroOrRightIsInf);
                {
                    result = DoubleBuildTaggedWithNoGC(*doubleLeft);
                    Jump(&exit);
                }
            }
        }
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::GetGlobalOwnProperty(GateRef glue, GateRef receiver, GateRef key)
{
    auto env = GetEnvironment();
    Label entryLabel(env);
    env->PushCurrentLabel(&entryLabel);
    DEFVARIABLE(result, VariableType::JS_ANY(), GetHoleConstant());
    GateRef properties = GetPropertiesFromJSObject(receiver);
    GateRef entry = FindEntryFromNameDictionary(glue, properties, key);
    Label notNegtiveOne(env);
    Label exit(env);
    Branch(Int32NotEqual(entry, GetInt32Constant(-1)), &notNegtiveOne, &exit);
    Bind(&notNegtiveOne);
    {
        result = GetValueFromGlobalDictionary(properties, entry);
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::JSArrayListGet(GateRef glue, GateRef receiver, GateRef index)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    DEFVARIABLE(result, VariableType::JS_ANY(), GetUndefinedConstant());

    GateRef lengthOffset = GetIntPtrConstant(panda::ecmascript::JSAPIArrayList::LENGTH_OFFSET);
    GateRef length = TaggedCastToInt32(Load(VariableType::INT64(), receiver, lengthOffset));
    Label isVailedIndex(env);
    Label notValidIndex(env);
    Branch(TruncInt32ToInt1(Int32And(ZExtInt1ToInt32(Int32GreaterThanOrEqual(index, GetInt32Constant(0))),
        ZExtInt1ToInt32(UInt32LessThan(index, length)))), &isVailedIndex, &notValidIndex);
    Bind(&isVailedIndex);
    {
        GateRef elements = GetElementsArray(receiver);
        result = GetValueFromTaggedArray(VariableType::JS_ANY(), elements, index);
        Jump(&exit);
    }
    Bind(&notValidIndex);
    {
        GateRef taggedId = GetInt32Constant(GET_MESSAGE_STRING_ID(GetPropertyOutOfBounds));
        CallRuntimeTrampoline(glue, GetInt64Constant(RTSTUB_ID(ThrowTypeError)),
            { IntBuildTaggedTypeWithNoGC(taggedId) });
        result = GetExceptionConstant();
        Jump(&exit);
    }

    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}

GateRef Stub::DoubleToInt(GateRef glue, GateRef x)
{
    auto env = GetEnvironment();
    Label entry(env);
    env->PushCurrentLabel(&entry);
    Label exit(env);
    Label overflow(env);

    GateRef xInt = ChangeFloat64ToInt32(x);
    DEFVARIABLE(result, VariableType::INT32(), xInt);

    if (env->IsAmd64()) {
        // 0x80000000: amd64 overflow return value
        Branch(Int32Equal(xInt, GetInt32Constant(0x80000000)), &overflow, &exit);
    } else {
        GateRef xInt64 = CastDoubleToInt64(x);
        // exp = (u64 & DOUBLE_EXPONENT_MASK) >> DOUBLE_SIGNIFICAND_SIZE - DOUBLE_EXPONENT_BIAS
        GateRef exp = Int64And(xInt64, GetInt64Constant(base::DOUBLE_EXPONENT_MASK));
        exp = ChangeInt64ToInt32(UInt64LSR(exp, GetInt64Constant(base::DOUBLE_SIGNIFICAND_SIZE)));
        exp = Int32Sub(exp, GetInt32Constant(base::DOUBLE_EXPONENT_BIAS));
        GateRef bits = GetInt32Constant(base::INT32_BITS - 1);
        // exp < 32 - 1
        Branch(Int32LessThan(exp, bits), &exit, &overflow);
    }
    Bind(&overflow);
    {
        const CallSignature *doubleToInt = RuntimeStubCSigns::Get(RTSTUB_ID(DoubleToInt));
        result = CallRuntime(doubleToInt, glue,
            GetIntPtrConstant(RTSTUB_ID(DoubleToInt) + NOGC_RTSTUB_CSIGNS_BEGIN), { x });
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *result;
    env->PopCurrentLabel();
    return ret;
}
}  // namespace panda::ecmascript::kungfu
