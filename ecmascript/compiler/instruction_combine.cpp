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
#include "ecmascript/compiler/instruction_combine.h"
#include "ecmascript/compiler/circuit_builder-inl.h"
#include "ecmascript/compiler/gate_matchers.h"

namespace panda::ecmascript::kungfu {

template <typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline constexpr int WhichPowerOfTwo(T value)
{
    // Ensure the size of the integer is no more than 8 bytes (64 bits).
    static_assert(sizeof(T) <= 8);
    // Use __builtin_ctzll for 8 bytes (64 bits) and __builtin_ctz for 32-bit integers.
    return sizeof(T) == 8 ? __builtin_ctzll(static_cast<uint64_t>(value)) : __builtin_ctz(static_cast<uint32_t>(value));
}


int32_t SignedDiv32(int32_t lhs, int32_t rhs)
{
    if (rhs == 0) {
        return 0;
    }
    if (rhs == -1) {
        return lhs == std::numeric_limits<int32_t>::min() ? lhs : -lhs;
    }
    return lhs / rhs;
}

int64_t SignedDiv64(int64_t lhs, int64_t rhs)
{
    if (rhs == 0) {
        return 0;
    }
    if (rhs == -1) {
        return lhs == std::numeric_limits<int64_t>::min() ? lhs : -lhs;
    }
    return lhs / rhs;
}

int32_t SignedMod32(int32_t lhs, int32_t rhs)
{
    if (rhs == 0 || rhs == -1) {
        return 0;
    }
    return lhs % rhs;
}


inline bool SignedAddOverflow32(int32_t lhs, int32_t rhs, int32_t *val)
{
    uint32_t res = static_cast<uint32_t>(lhs) + static_cast<uint32_t>(rhs);
    *val = base::bit_cast<int32_t>(res);
    // Check for overflow by examining the sign bit.(bit 31 in a 32-bit integer)
    return ((res ^ lhs) & (res ^ rhs) & (1U << 31)) != 0;
}


inline bool SignedSubOverflow32(int32_t lhs, int32_t rhs, int32_t *val)
{
    uint32_t res = static_cast<uint32_t>(lhs) - static_cast<uint32_t>(rhs);
    *val = base::bit_cast<int32_t>(res);
    // Check for overflow by examining the sign bit.(bit 31 in a 32-bit integer)
    return ((res ^ lhs) & (res ^ ~rhs) & (1U << 31)) != 0;
}

inline bool SignedMulOverflow32(int32_t lhs, int32_t rhs, int32_t *val)
{
    int64_t result = int64_t{lhs} * int64_t{rhs};
    *val = static_cast<int32_t>(result);
    using limits = std::numeric_limits<int32_t>;
    return result < limits::min() || result > limits::max();
}


// Returns the quotient x/y, avoiding C++ undefined behavior if y == 0.
template <typename T> inline T Divide(T x, T y)
{
    if (y != 0) {
        return x / y;
    }
    if (x == 0 || x != x) {
        return std::numeric_limits<T>::quiet_NaN();
    }
    if ((x >= 0) == (std::signbit(y) == 0)) {
        return std::numeric_limits<T>::infinity();
    }
    return -std::numeric_limits<T>::infinity();
}

// Helpers for performing overflowing arithmetic operations without relying
// on C++ undefined behavior.
#define ASSERT_SIGNED_INTEGER_TYPE(Type)                                                                               \
    static_assert(std::is_integral<Type>::value && std::is_signed<Type>::value, "use this for signed integer types")
#define OP_WITH_WRAPAROUND(Name, OP)                                                                                   \
    template <typename signed_type> inline signed_type Name##WithWraparound(signed_type a, signed_type b)              \
    {                                                                                                                  \
        ASSERT_SIGNED_INTEGER_TYPE(signed_type);                                                                       \
        using unsigned_type = typename std::make_unsigned<signed_type>::type;                                          \
        unsigned_type a_unsigned = static_cast<unsigned_type>(a);                                                      \
        unsigned_type b_unsigned = static_cast<unsigned_type>(b);                                                      \
        unsigned_type result = a_unsigned OP b_unsigned;                                                               \
        return static_cast<signed_type>(result);                                                                       \
    }

OP_WITH_WRAPAROUND(Add, +)
OP_WITH_WRAPAROUND(Sub, -)
OP_WITH_WRAPAROUND(Mul, *)

template <typename signed_type> inline signed_type ShlWithWraparound(signed_type a, signed_type b)
{
    using unsigned_type = typename std::make_unsigned<signed_type>::type;
    const unsigned_type kMask = (sizeof(a) * 8) - 1;
    return static_cast<signed_type>(static_cast<unsigned_type>(a) << (b & kMask));
}

template <typename signed_type> inline signed_type NegateWithWraparound(signed_type a)
{
    ASSERT_SIGNED_INTEGER_TYPE(signed_type);
    if (a == std::numeric_limits<signed_type>::min()) {
        return a;
    }
    return -a;
}

GateRef InstructionCombine::ReplaceOld(GateRef gate, GateRef newGate)
{
    acc_.UpdateAllUses(gate, newGate);
    return newGate;
}


GateRef InstructionCombine::VisitGate(GateRef gate)
{
    OpCode op = acc_.GetOpCode(gate);
    GateRef result = Circuit::NullGate();
    ;
    switch (op) {
        case OpCode::ADD:
            result = VisitADD(gate);
            break;
        case OpCode::SUB:
            result = VisitSUB(gate);
            break;
        case OpCode::MUL:
            result = VisitMUL(gate);
            break;
        case OpCode::SDIV:
            result = VisitSDIV(gate);
            break;
        case OpCode::FDIV:
            result = VisitFDIV(gate);
            break;
        case OpCode::SMOD:
            result = VisitSMOD(gate);
            break;
        case OpCode::AND:
            result = VisitAND(gate);
            break;
        case OpCode::OR:
            result = VisitOR(gate);
            break;
        case OpCode::XOR:
            result = VisitXOR(gate);
            break;
        case OpCode::ASR:
            result = VisitASR(gate);
            break;
        case OpCode::LSL:
            result = VisitLSL(gate);
            break;
        case OpCode::LSR:
            result = VisitLSR(gate);
            break;
        case OpCode::ICMP:
            result = VisitICMP(gate);
            break;
        case OpCode::REV:
            result = VisitREV(gate);
            break;
        case OpCode::IF_BRANCH:
            result = VisitBranch(gate);
            break;
        case OpCode::EXTRACT_VALUE:
            return VisitExtractValue(gate);
        case OpCode::TAGGED_TO_INT64:
        case OpCode::INT64_TO_TAGGED:
        case OpCode::SIGNED_INT_TO_FLOAT:
        case OpCode::FLOAT_TO_SIGNED_INT:
        case OpCode::UNSIGNED_FLOAT_TO_INT:
        case OpCode::BITCAST:
        case OpCode::ZEXT:
        case OpCode::SEXT:
        case OpCode::TRUNC:
        case OpCode::FEXT:
        case OpCode::FTRUNC:
            return VisitConvert(gate);
            break;
        default:
            break;
    }
    if (enableLog_ && result != Circuit::NullGate()) {
        LOG_COMPILER(INFO) << "InstructionCombine opt befor -> afert";
        acc_.Print(gate);
        acc_.Print(result);
    }
    return result;
}

// Wait for the implementation of constant folding and partial redundancy elimination.
// The Convert-related IR operations need refactoring for more effective optimization.
// 1. Merge or differentiate the functionalities of SignIntToFloat and Bitcast
//    as both handle Int-to-Float conversions. Int32ToFloat32 also exhibits similar behavior.
// 2. Clarify the roles of TruncFloatToInt64 and FTrunc, ensuring they are distinct or unified.
// 3. Standardize naming conventions for operations that are inverse or similar to each other
//    to avoid confusion. ex: ChangeTaggedPointerToInt64 and Int64ToTaggedPtr perform similar functions,
//    yet their names are significantly different
// 4. .................
GateRef InstructionCombine::VisitConvert(GateRef gate)
{
    // For the meanings of the following Opcodes, please refer to
    // UNARY_ARITHMETIC_METHOD_LIST_WITH_BITWIDTH
    switch (acc_.GetOpCode(gate)) {
        case OpCode::TAGGED_TO_INT64:
            {
                GateMatcher in(acc_.GetValueIn(gate, 0), circuit_);
                if (in.IsInt64ToTagged()) {
                    return in.ValueIn(0);
                }
                break;
            }
        case OpCode::INT64_TO_TAGGED:
            {
                GateMatcher in(acc_.GetValueIn(gate, 0), circuit_);
                if (in.IsTaggedToInt64()) {
                    return in.ValueIn(0);
                }
                break;
            }
        case OpCode::SIGNED_INT_TO_FLOAT:
            {
                GateMatcher in(acc_.GetValueIn(gate, 0), circuit_);
                if (in.IsFloatToSignedInt()) {
                    return in.ValueIn(0);
                }
                break;
            }
        case OpCode::FLOAT_TO_SIGNED_INT:
            {
                GateMatcher in(acc_.GetValueIn(gate, 0), circuit_);
                if (in.IsSignedIntToFloat()) {
                    return in.ValueIn(0);
                }
                break;
            }
        case OpCode::UNSIGNED_FLOAT_TO_INT:
            break;
        case OpCode::BITCAST:
        case OpCode::ZEXT:
        case OpCode::SEXT:
        case OpCode::TRUNC:
        case OpCode::FEXT:
        case OpCode::FTRUNC:
            break;
        default:
            break;
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::VisitBranch(GateRef gate)
{
    (void)gate;
    return Circuit::NullGate();
}

GateRef InstructionCombine::VisitREV(GateRef gate)
{
    // REV Constant => Constant
    auto revValue = acc_.GetValueIn(gate, 0);
    if (acc_.GetOpCode(revValue) == OpCode::CONSTANT && acc_.GetMachineType(revValue) == I1) {
        if (acc_.GetConstantValue(revValue) == 0) {
            return builder_.True();
        } else {
            return builder_.False();
        }
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::VisitICMP(GateRef gate)
{
    Int64BinopMatcher m(gate, circuit_);
    // Match {EQ ((x or constant1) , constant2)} {((constant1 | constant2) != constant2)} => false
    GateRef result = Circuit::NullGate();
    if (m.ConditionValue() == static_cast<uint64_t>(ICmpCondition::EQ) && m.Right().HasResolvedValue()) {
        // In essence, it's all about comparing I64, so we can optimize further
        // Subsequently, considering whether it can be automated within the gateMatcher
        if (m.Left().Opcode() == OpCode::INT64_TO_TAGGED) {
            m.SetLeft(m.Left().InputAt(0), circuit_);
        }

        if (m.Left().IsOr()) {
            Int64BinopMatcher cmpLeft(m.Left().Gate(), circuit_);
            if (cmpLeft.Right().HasResolvedValue()) {
                int constant1 = cmpLeft.Right().ResolvedValue();
                int constant2 = m.Right().ResolvedValue();
                bool flag = ((constant1 | constant2) != constant2);
                result = flag ? builder_.False() : Circuit::NullGate();
            }
        }

        if (result != Circuit::NullGate()) {
            return result;
        }

        // Match {EQ((X or constant1) & constant2, 0)} { (constan2 !=0 && constant1 & constant2 !=0) }=> false
        if (m.Left().IsAnd() && m.Right().ResolvedValue() == 0) {
            Int64BinopMatcher andOp(m.Left().Gate(), circuit_);
            if (andOp.Left().IsOr() && andOp.Right().HasResolvedValue() && andOp.Right().ResolvedValue() != 0) {
                // The awkwardness in writing it this way is simply to reduce cyclomatic complexity.
                Int64BinopMatcher orOp(andOp.Left().Gate(), circuit_);
                auto constant2 = andOp.Right().ResolvedValue();
                auto constant1 = orOp.Right().HasResolvedValue() ? orOp.Right().ResolvedValue() : 0;
                bool flag = ((constant1 & constant2) != 0);
                result = flag ? builder_.False() : Circuit::NullGate();
            }
        }
        if (result != Circuit::NullGate()) {
            return result;
        }
    }

    return Circuit::NullGate();
}

GateRef InstructionCombine::VisitADD(GateRef gate)
{
    auto machineType = acc_.GetMachineType(gate);
    switch (machineType) {
        case MachineType::I32:
            return ReduceInt32Add(gate);
        case MachineType::I64:
            return ReduceInt64Add(gate);
        case MachineType::F64:
            return ReduceDoubleAdd(gate);
        default:
            break;
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::VisitSUB(GateRef gate)
{
    auto machineType = acc_.GetMachineType(gate);
    switch (machineType) {
        case MachineType::I32:
            return ReduceInt32Sub(gate);
        case MachineType::I64:
            return ReduceInt64Sub(gate);
        case MachineType::F64:
            return ReduceDoubleSub(gate);
        default:
            break;
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::VisitMUL(GateRef gate)
{
    auto machineType = acc_.GetMachineType(gate);
    switch (machineType) {
        case MachineType::I32:
            return ReduceInt32Mul(gate);
        case MachineType::I64:
            return ReduceInt64Mul(gate);
        case MachineType::F64:
            return ReduceDoubleMul(gate);
        default:
            break;
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::VisitSDIV(GateRef gate)
{
    auto machineType = acc_.GetMachineType(gate);
    switch (machineType) {
        case MachineType::I32:
            return ReduceInt32Div(gate);
        case MachineType::I64:
            return ReduceInt64Div(gate);
        default:
            break;
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::VisitFDIV(GateRef gate)
{
    auto machineType = acc_.GetMachineType(gate);
    switch (machineType) {
        case MachineType::F64:
            return ReduceDoubleDiv(gate);
        default:
            break;
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::VisitSMOD(GateRef gate)
{
    auto machineType = acc_.GetMachineType(gate);
    switch (machineType) {
        case MachineType::I32:
            return ReduceInt32Mod(gate);
        case MachineType::F64:
            return ReduceDoubleMod(gate);
        default:
            break;
    }
    return Circuit::NullGate();
}


GateRef InstructionCombine::VisitAND(GateRef gate)
{
    auto machineType = acc_.GetMachineType(gate);
    switch (machineType) {
        case MachineType::I32:
            return ReduceWord32And(gate);
        case MachineType::I64:
            return ReduceWord64And(gate);
        default:
            break;
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::VisitOR(GateRef gate)
{
    auto machineType = acc_.GetMachineType(gate);
    switch (machineType) {
        case MachineType::I32:
            return ReduceWord32Or(gate);
        case MachineType::I64:
            return ReduceWord64Or(gate);
        default:
            break;
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::VisitXOR(GateRef gate)
{
    auto machineType = acc_.GetMachineType(gate);
    switch (machineType) {
        case MachineType::I32:
            return ReduceWord32Xor(gate);
        case MachineType::I64:
            return ReduceWord64Xor(gate);
        default:
            break;
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::VisitLSR(GateRef gate)
{
    auto machineType = acc_.GetMachineType(gate);
    switch (machineType) {
        case MachineType::I32:
            return ReduceWord32Lsr(gate);
        case MachineType::I64:
            return ReduceWord64Lsr(gate);
        default:
            break;
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::VisitASR(GateRef gate)
{
    auto machineType = acc_.GetMachineType(gate);
    switch (machineType) {
        case MachineType::I32:
            return ReduceWord32Asr(gate);
        case MachineType::I64:
            return ReduceWord64Asr(gate);
        default:
            break;
    }
    return Circuit::NullGate();
}


GateRef InstructionCombine::VisitLSL(GateRef gate)
{
    auto machineType = acc_.GetMachineType(gate);
    switch (machineType) {
        case MachineType::I32:
            return ReduceWord32Lsl(gate);
        case MachineType::I64:
            return ReduceWord64Lsl(gate);
        default:
            break;
    }
    return Circuit::NullGate();
}


GateRef InstructionCombine::VisitExtractValue(GateRef gate)
{
    Int32BinopMatcher n(gate, circuit_);
    int32_t index = n.Right().ResolvedValue();
    int32_t val;
    assert(index == 0 || index == 1);
    switch (n.Left().Opcode()) {
        case OpCode::ADD_WITH_OVERFLOW:
            {
                Int32BinopMatcher m(n.Left().Gate(), circuit_);
                if (m.IsFoldable()) {
                    bool ovf = SignedAddOverflow32(m.Left().ResolvedValue(), m.Right().ResolvedValue(), &val);
                    return index == 0 ? builder_.Int32(val) : builder_.Boolean(ovf);
                }
                if (m.Right().Is(0)) {
                    return (index == 0 ? m.Left().Gate() : builder_.Boolean(false));
                }
                break;
            }
        case OpCode::SUB_WITH_OVERFLOW:
            {
                Int32BinopMatcher m(n.Left().Gate(), circuit_);
                if (m.IsFoldable()) {
                    bool ovf = SignedSubOverflow32(m.Left().ResolvedValue(), m.Right().ResolvedValue(), &val);
                    return index == 0 ? builder_.Int32(val) : builder_.Boolean(ovf);
                }
                if (m.Right().Is(0)) {
                    return (index == 0 ? m.Left().Gate() : builder_.Boolean(false));
                }
                break;
            }
        case OpCode::MUL_WITH_OVERFLOW:
            {
                Int32BinopMatcher m(n.Left().Gate(), circuit_);
                if (m.IsFoldable()) {
                    bool ovf = SignedMulOverflow32(m.Left().ResolvedValue(), m.Right().ResolvedValue(), &val);
                    return index == 0 ? builder_.Int32(val) : builder_.Boolean(ovf);
                }
                if (m.Right().Is(0)) {
                    return (index == 0 ? builder_.Int32(0) : builder_.Boolean(false));
                }
                if (m.Right().Is(1)) {
                    return (index == 0 ? m.Left().Gate() : builder_.Boolean(false));
                }
                break;
            }
        default:
            break;
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceInt64Add(GateRef gate)
{
    Int64BinopMatcher m(gate, circuit_);

    if (m.Right().Is(0)) {
        return m.Left().Gate();
    }

    if (m.IsFoldable()) {
        return builder_.Int64(AddWithWraparound(m.Right().ResolvedValue(), m.Left().ResolvedValue()));
    }

    // (x + Int64Constant(a)) + Int64Constant(b) => x + Int64Constant(a + b)
    if (m.Right().HasResolvedValue() && m.Left().IsmInt64Add()) {
        Int64BinopMatcher mLeft(m.Left().Gate(), circuit_);
        if (mLeft.Right().HasResolvedValue() && m.OwnsInput(m.Left().Gate())) {
            acc_.ReplaceValueIn(gate, mLeft.Left().Gate(), 0);
            acc_.ReplaceValueIn(
                gate, builder_.Int64(AddWithWraparound(m.Right().ResolvedValue(), mLeft.Right().ResolvedValue())), 1);
            return gate;
        }
    }

    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceInt32Add(GateRef gate)
{
    Int32BinopMatcher m(gate, circuit_);
    // x + 0 => x
    if (m.Right().Is(0)) {
        return m.Left().Gate();
    }

    if (m.IsFoldable()) {
        return builder_.Int32(AddWithWraparound(m.Left().ResolvedValue(), m.Right().ResolvedValue()));
    }

    if (m.Left().IsmInt32Sub()) {
        Int32BinopMatcher mleft(m.Left().Gate(), circuit_);
        // (0 - x) + y => y - x
        if (mleft.Left().Is(0)) {
            auto newGate = builder_.Int32Sub(m.Right().Gate(), mleft.Right().Gate());
            return ReplaceOld(gate, newGate);
        }
    }

    if (m.Right().IsmInt32Sub()) {
        Int32BinopMatcher mright(m.Right().Gate(), circuit_);
        // y + (0 - x) => y - x
        if (mright.Left().Is(0)) {
            auto newGate = builder_.Int32Sub(m.Left().Gate(), mright.Right().Gate());
            return ReplaceOld(gate, newGate);
        }
    }
    // (x + Int32Constant(a)) + Int32Constant(b)) => x + Int32Constant(a + b)
    if (m.Right().HasResolvedValue() && m.Left().IsmInt32Add()) {
        Int32BinopMatcher mleft(m.Left().Gate(), circuit_);
        if (mleft.Right().HasResolvedValue() && m.OwnsInput(m.Left().Gate())) {
            acc_.ReplaceValueIn(gate, mleft.Left().Gate(), 0);
            acc_.ReplaceValueIn(
                gate, builder_.Int32(AddWithWraparound(mleft.Right().ResolvedValue(), m.Right().ResolvedValue())), 1);
            return gate;
        }
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceInt64Sub(GateRef gate)
{
    Int64BinopMatcher m(gate, circuit_);
    // x - 0 => x
    if (m.Right().Is(0)) {
        return (m.Left().Gate());
    }
    if (m.IsFoldable()) {
        return builder_.Int64(SubWithWraparound(m.Left().ResolvedValue(), m.Right().ResolvedValue()));
    }
    // x - x => 0
    if (m.LeftEqualsRight()) {
        return builder_.Int64(0);
    }
    // x - K => x + -K
    if (m.Right().HasResolvedValue()) {
        auto newGate =
            builder_.Int64Add(m.Left().Gate(), builder_.Int64(NegateWithWraparound(m.Right().ResolvedValue())));
        return ReplaceOld(gate, newGate);
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceInt32Sub(GateRef gate)
{
    Int32BinopMatcher m(gate, circuit_);
    // x - 0 => x
    if (m.Right().Is(0)) {
        return (m.Left().Gate());
    }
    if (m.IsFoldable()) {
        return builder_.Int32(SubWithWraparound(m.Left().ResolvedValue(), m.Right().ResolvedValue()));
    }
    // x - x => 0
    if (m.LeftEqualsRight()) {
        return builder_.Int32(0);
    }
    // x - K => x + -K
    if (m.Right().HasResolvedValue()) {
        auto newGate =
            builder_.Int32Add(m.Left().Gate(), builder_.Int32(NegateWithWraparound(m.Right().ResolvedValue())));
        return ReplaceOld(gate, newGate);
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceInt64Mul(GateRef gate)
{
    Int64BinopMatcher m(gate, circuit_);
    // x * 0 => 0
    if (m.Right().Is(0)) {
        return m.Right().Gate();
    }
    // x * 1 => x
    if (m.Right().Is(1)) {
        return m.Left().Gate();
    }
    // K * K => K  (K stands for arbitrary constants)
    if (m.IsFoldable()) {
        return builder_.Int64(MulWithWraparound(m.Left().ResolvedValue(), m.Right().ResolvedValue()));
    }
    // x * -1 => 0 - x
    if (m.Right().Is(-1)) {
        auto newGate = builder_.Int64Sub(builder_.Int64(0), m.Left().Gate());
        return ReplaceOld(gate, newGate);
    }
    // x * 2^n => x << n
    if (m.Right().IsPowerOf2()) {
        auto newGate = builder_.Int64LSL(m.Left().Gate(), builder_.Int64(WhichPowerOfTwo(m.Right().ResolvedValue())));
        return ReplaceOld(gate, newGate);
    }

    // (x * Int64Constant(a)) * Int64Constant(b)) => x * Int64Constant(a * b)
    if (m.Right().HasResolvedValue() && m.Left().IsmInt64Mul()) {
        Int64BinopMatcher n(m.Left().Gate(), circuit_);
        if (n.Right().HasResolvedValue() && m.OwnsInput(m.Left().Gate())) {
            acc_.ReplaceValueIn(gate, n.Left().Gate(), 0);
            acc_.ReplaceValueIn(
                gate, builder_.Int64(MulWithWraparound(n.Right().ResolvedValue(), m.Right().ResolvedValue())), 1);
            return gate;
        }
    }

    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceInt32Mul(GateRef gate)
{
    Int32BinopMatcher m(gate, circuit_);
    // x * 0 => 0
    if (m.Right().Is(0)) {
        return m.Right().Gate();
    }
    // x * 1 => x
    if (m.Right().Is(1)) {
        return m.Left().Gate();
    }
    // K * K => K  (K stands for arbitrary constants)
    if (m.IsFoldable()) {
        return builder_.Int32(MulWithWraparound(m.Left().ResolvedValue(), m.Right().ResolvedValue()));
    }
    // x * -1 => 0 - x
    if (m.Right().Is(-1)) {
        auto newGate = builder_.Int32Sub(builder_.Int32(0), m.Left().Gate());
        return ReplaceOld(gate, newGate);
    }
    // x * 2^n => x << n
    if (m.Right().IsPowerOf2()) {
        auto newGate = builder_.Int32LSL(m.Left().Gate(), builder_.Int32(WhichPowerOfTwo(m.Right().ResolvedValue())));
        return ReplaceOld(gate, newGate);
    }

    // (x * Int32Constant(a)) * Int32Constant(b)) => x * Int32Constant(a * b)
    if (m.Right().HasResolvedValue() && m.Left().IsmInt32Mul()) {
        Int32BinopMatcher n(m.Left().Gate(), circuit_);
        if (n.Right().HasResolvedValue() && m.OwnsInput(m.Left().Gate())) {
            acc_.ReplaceValueIn(gate, n.Left().Gate(), 0);
            acc_.ReplaceValueIn(
                gate, builder_.Int32(MulWithWraparound(n.Right().ResolvedValue(), m.Right().ResolvedValue())), 1);
            return gate;
        }
    }

    return Circuit::NullGate();
}


GateRef InstructionCombine::ReduceInt64Div(GateRef gate)
{
    Int64BinopMatcher m(gate, circuit_);
    // 0 / x => 0
    if (m.Left().Is(0)) {
        return m.Left().Gate();
    }
    // x / 0 => 0
    if (m.Right().Is(0)) {
        return m.Right().Gate();
    }
    // x / 1 => x
    if (m.Right().Is(1)) {
        return m.Left().Gate();
    }
    // K / K => K
    if (m.IsFoldable()) {
        return builder_.Int64(SignedDiv64(m.Left().ResolvedValue(), m.Right().ResolvedValue()));
    }
    // x / -1 => 0 - x
    if (m.Right().Is(-1)) {
        auto newGate = builder_.Int64Sub(builder_.Int64(0), m.Left().Gate());
        return ReplaceOld(gate, newGate);
    }

    //  x/-K => 0-(x/K)
    if (m.Right().HasResolvedValue()) {
        int64_t const divisor = m.Right().ResolvedValue();
        if (divisor < 0) {
            auto newDiv = builder_.Int64Div(m.Left().Gate(), builder_.Int64(abs(m.Right().ResolvedValue())));
            auto newGate = builder_.Int64Sub(builder_.Int64(0), newDiv);
            return ReplaceOld(gate, newGate);
        }
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceInt32Div(GateRef gate)
{
    Int32BinopMatcher m(gate, circuit_);
    // 0 / x => 0
    if (m.Left().Is(0)) {
        return m.Left().Gate();
    }
    // x / 0 => 0
    if (m.Right().Is(0)) {
        return m.Right().Gate();
    }
    // x / 1 => x
    if (m.Right().Is(1)) {
        return m.Left().Gate();
    }
    // K / K => K
    if (m.IsFoldable()) {
        return builder_.Int32(SignedDiv32(m.Left().ResolvedValue(), m.Right().ResolvedValue()));
    }
    // x / -1 => 0 - x
    if (m.Right().Is(-1)) {
        auto newGate = builder_.Int32Sub(builder_.Int32(0), m.Left().Gate());
        return ReplaceOld(gate, newGate);
    }

    //  x/-K => 0-(x/K)
    if (m.Right().HasResolvedValue()) {
        int32_t const divisor = m.Right().ResolvedValue();
        if (divisor < 0) {
            auto newDiv = builder_.Int32Div(m.Left().Gate(), builder_.Int32(abs(m.Right().ResolvedValue())));
            auto newGate = builder_.Int32Sub(builder_.Int32(0), newDiv);
            return ReplaceOld(gate, newGate);
        }
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceDoubleAdd(GateRef gate)
{
    Float64BinopMatcher m(gate, circuit_);
    // x + NaN => NaN
    if (m.Right().IsNaN()) {
        return builder_.NanValue();
    }
    // NaN + x => NaN
    if (m.Left().IsNaN()) {
        return builder_.NanValue();
    }
    // K + K => K  (K stands for arbitrary constants)
    if (m.IsFoldable()) {
        return builder_.Double(m.Left().ResolvedValue() + m.Right().ResolvedValue());
    }
    return Circuit::NullGate();
}
GateRef InstructionCombine::ReduceDoubleSub(GateRef gate)
{
    Float64BinopMatcher m(gate, circuit_);
    // x - NaN => NaN
    if (m.Right().IsNaN()) {
        return builder_.NanValue();
    }
    // NaN - x => NaN
    if (m.Left().IsNaN()) {
        return builder_.NanValue();
    }
    // L - R => (L - R)
    if (m.IsFoldable()) {
        return builder_.Double(m.Left().ResolvedValue() - m.Right().ResolvedValue());
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceDoubleMul(GateRef gate)
{
    Float64BinopMatcher m(gate, circuit_);
    if (m.Right().Is(-1)) { // x * -1.0 => -0.0 - x
        auto newGate = builder_.DoubleSub(builder_.Double(-0.0), m.Left().Gate());
        return ReplaceOld(gate, newGate);
    }
    if (m.Right().IsNaN()) { // x * NaN => NaN
        return builder_.NanValue();
    }
    if (m.IsFoldable()) { // K * K => K  (K stands for arbitrary constants)
        return builder_.Double(m.Left().ResolvedValue() * m.Right().ResolvedValue());
    }
    if (m.Right().Is(2)) { // x * 2.0 => x + x
        auto newGate = builder_.DoubleAdd(m.Left().Gate(), m.Left().Gate());
        return ReplaceOld(gate, newGate);
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceDoubleDiv(GateRef gate)
{
    Float64BinopMatcher m(gate, circuit_);

    if (m.Right().IsNaN()) { // x / NaN => NaN
        return builder_.NanValue();
    }
    if (m.Left().IsNaN()) { // NaN / x => NaN
        return builder_.NanValue();
    }
    if (m.IsFoldable()) { // K / K => K  (K stands for arbitrary constants)
        return builder_.Double(Divide(m.Left().ResolvedValue(), m.Right().ResolvedValue()));
    }

    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceInt32Mod(GateRef gate)
{
    Int32BinopMatcher m(gate, circuit_);
    // 0 % x  => 0
    if (m.Left().Is(0)) {
        return m.Left().Gate();
    }
    // x % 0  => 0
    if (m.Right().Is(0)) {
        return m.Right().Gate();
    }
    // x % 1  => 0
    if (m.Right().Is(1)) {
        return builder_.Int32(0);
    }
    // x % -1 => 0
    if (m.Right().Is(-1)) {
        return builder_.Int32(0);
    }
    // x % x  => 0
    if (m.LeftEqualsRight()) {
        return builder_.Int32(0);
    }
    // K % K => K  (K stands for arbitrary constants)
    if (m.IsFoldable()) {
        return builder_.Int32(SignedMod32(m.Left().ResolvedValue(), m.Right().ResolvedValue()));
    }

    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceDoubleMod(GateRef gate)
{
    Float64BinopMatcher m(gate, circuit_);
    if (m.Right().Is(0)) { // x % 0 => NaN
        return builder_.NanValue();
    }
    if (m.Right().IsNaN()) { // x % NaN => NaN
        return builder_.NanValue();
    }
    if (m.Left().IsNaN()) { // NaN % x => NaN
        return builder_.NanValue();
    }
    return Circuit::NullGate();
}


GateRef InstructionCombine::ReduceWord64And(GateRef gate)
{
    Int64BinopMatcher m(gate, circuit_);
    // x & 0  => 0
    if (m.Right().Is(0)) {
        return m.Right().Gate();
    }
    // x & -1 => x
    if (m.Right().Is(-1)) {
        return m.Left().Gate();
    }
    // CMP & 1 => CMP
    if (m.Left().IsIcmp() && m.Right().Is(1)) {
        return m.Left().Gate();
    }
    // K & K  => K  (K stands for arbitrary constants)
    if (m.IsFoldable()) {
        return builder_.Int64(m.Left().ResolvedValue() & m.Right().ResolvedValue());
    }
    // x & x => x
    if (m.LeftEqualsRight()) {
        return m.Left().Gate();
    }
    // (x & K) & K => x & K
    if (m.Left().IsmInt64And() && m.Right().HasResolvedValue()) {
        Int64BinopMatcher mleft(m.Left().Gate(), circuit_);
        if (mleft.Right().HasResolvedValue()) {
            auto newGate = builder_.Int64And(
                mleft.Left().Gate(), builder_.Int64(m.Right().ResolvedValue() & mleft.Right().ResolvedValue()));
            return ReplaceOld(gate, newGate);
        }
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceWord32And(GateRef gate)
{
    Int32BinopMatcher m(gate, circuit_);
    // x & 0  => 0
    if (m.Right().Is(0)) {
        return m.Right().Gate();
    }
    // x & -1 => x
    if (m.Right().Is(-1)) {
        return m.Left().Gate();
    }
    // CMP & 1 => CMP
    if (m.Left().IsIcmp() && m.Right().Is(1)) {
        return m.Left().Gate();
    }
    // K & K  => K  (K stands for arbitrary constants)
    if (m.IsFoldable()) {
        return builder_.Int32(m.Left().ResolvedValue() & m.Right().ResolvedValue());
    }
    // x & x => x
    if (m.LeftEqualsRight()) {
        return m.Left().Gate();
    }
    // (x & K) & K => x & K
    if (m.Left().IsmInt32And() && m.Right().HasResolvedValue()) {
        Int32BinopMatcher mleft(m.Left().Gate(), circuit_);
        if (mleft.Right().HasResolvedValue()) {
            auto newGate = builder_.Int32And(
                mleft.Left().Gate(), builder_.Int32(m.Right().ResolvedValue() & mleft.Right().ResolvedValue()));
            return ReplaceOld(gate, newGate);
        }
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceWord64Or(GateRef gate)
{
    Int64BinopMatcher m(gate, circuit_);
    // x | 0  => x
    if (m.Right().Is(0)) {
        return m.Left().Gate();
    }
    // x | -1 => -1
    if (m.Right().Is(-1)) {
        return m.Right().Gate();
    }
    // K | K  => K  (K stands for arbitrary constants)
    if (m.IsFoldable()) {
        return builder_.Int64(m.Left().ResolvedValue() | m.Right().ResolvedValue());
    }
    // x | x => x
    if (m.LeftEqualsRight()) {
        return m.Left().Gate();
    }

    // (x & K1) | K2 => x | K2 if K2 has ones for every zero bit in K1.
    if (m.Right().HasResolvedValue() && m.Left().IsmInt64And()) {
        Int64BinopMatcher mand(m.Left().Gate(), circuit_);
        if (mand.Right().HasResolvedValue()) {
            if ((m.Right().ResolvedValue() | mand.Right().ResolvedValue()) == -1) {
                acc_.ReplaceValueIn(gate, mand.Left().Gate(), 0);
                return gate;
            }
        }
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceWord32Or(GateRef gate)
{
    Int32BinopMatcher m(gate, circuit_);
    // x | 0  => x
    if (m.Right().Is(0)) {
        return m.Left().Gate();
    }
    // x | -1 => -1
    if (m.Right().Is(-1)) {
        return m.Right().Gate();
    }
    // K | K  => K  (K stands for arbitrary constants)
    if (m.IsFoldable()) {
        return builder_.Int32(m.Left().ResolvedValue() | m.Right().ResolvedValue());
    }
    // x | x => x
    if (m.LeftEqualsRight()) {
        return m.Left().Gate();
    }

    // (x & K1) | K2 => x | K2 if K2 has ones for every zero bit in K1.
    if (m.Right().HasResolvedValue() && m.Left().IsmInt32And()) {
        Int32BinopMatcher mand(m.Left().Gate(), circuit_);
        if (mand.Right().HasResolvedValue()) {
            if ((m.Right().ResolvedValue() | mand.Right().ResolvedValue()) == -1) {
                acc_.ReplaceValueIn(gate, mand.Left().Gate(), 0);
                return gate;
            }
        }
    }

    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceWord64Xor(GateRef gate)
{
    Int64BinopMatcher m(gate, circuit_);
    // x ^ 0 => x
    if (m.Right().Is(0)) {
        return m.Left().Gate();
    }
    // K ^ K => K  (K stands for arbitrary constants)
    if (m.IsFoldable()) {
        return builder_.Int64(m.Left().ResolvedValue() ^ m.Right().ResolvedValue());
    }
    if (m.LeftEqualsRight()) {
        return builder_.Int64(0); // x ^ x => 0
    }
    // (x ^ -1) ^ -1 => x
    if (m.Left().IsmInt64Xor() && m.Right().Is(-1)) {
        Int64BinopMatcher mleft(m.Left().Gate(), circuit_);
        if (mleft.Right().Is(-1)) {
            return mleft.Left().Gate();
        }
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceWord32Xor(GateRef gate)
{
    Int32BinopMatcher m(gate, circuit_);
    // x ^ 0 => x
    if (m.Right().Is(0)) {
        return m.Left().Gate();
    }
    // K ^ K => K  (K stands for arbitrary constants)
    if (m.IsFoldable()) {
        return builder_.Int32(m.Left().ResolvedValue() ^ m.Right().ResolvedValue());
    }
    if (m.LeftEqualsRight()) {
        return builder_.Int32(0); // x ^ x => 0
    }
    // (x ^ -1) ^ -1 => x
    if (m.Left().IsmInt32Xor() && m.Right().Is(-1)) {
        Int32BinopMatcher mleft(m.Left().Gate(), circuit_);
        if (mleft.Right().Is(-1)) {
            return mleft.Left().Gate();
        }
    }
    return Circuit::NullGate();
}
GateRef InstructionCombine::ReduceWord64Lsr(GateRef gate)
{
    Uint64BinopMatcher m(gate, circuit_);

    // x >>> 0 => x
    if (m.Right().Is(0)) {
        return m.Left().Gate();
    }
    if (m.IsFoldable()) {
        // The '63' here is used as a mask to limit the shift amount to 0-63 bits, preventing overflow.
        return builder_.Int64(m.Left().ResolvedValue() >> (m.Right().ResolvedValue() & 63));
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceWord32Lsr(GateRef gate)
{
    Uint32BinopMatcher m(gate, circuit_);
    // x >>> 0 => x
    if (m.Right().Is(0)) {
        return m.Left().Gate();
    }
    if (m.IsFoldable()) {
        // The '31' here is used as a mask to limit the shift amount to 0-31 bits, preventing overflow.
        return builder_.Int32(m.Left().ResolvedValue() >> (m.Right().ResolvedValue() & 31));
    }
    // (m >>> s) == 0 implies ((x & m) >>> s) == 0
    if (m.Left().IsmInt32And() && m.Right().HasResolvedValue()) {
        Uint32BinopMatcher mleft(m.Left().Gate(), circuit_);
        if (mleft.Right().HasResolvedValue()) {
            uint32_t shift = m.Right().ResolvedValue() & 31;
            uint32_t mask = mleft.Right().ResolvedValue();
            if ((mask >> shift) == 0) {
                return builder_.Int32(0);
            }
        }
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceWord64Asr(GateRef gate)
{
    Int64BinopMatcher m(gate, circuit_);
    // x >> 0 => x
    if (m.Right().Is(0)) {
        return m.Left().Gate();
    }
    if (m.IsFoldable()) {
        // The '63' here is used as a mask to limit the shift amount to 0-63 bits, preventing overflow.
        return builder_.Int64(m.Left().ResolvedValue() >> (m.Right().ResolvedValue() & 63));
    }
    return Circuit::NullGate();
}
GateRef InstructionCombine::ReduceWord32Asr(GateRef gate)
{
    Int32BinopMatcher m(gate, circuit_);
    // x >> 0 => x
    if (m.Right().Is(0)) {
        return m.Left().Gate();
    }
    if (m.IsFoldable()) {
        // The '31' here is used as a mask to limit the shift amount to 0-31 bits, preventing overflow.
        return builder_.Int32(m.Left().ResolvedValue() >> (m.Right().ResolvedValue() & 31));
    }
    if (m.Left().IsmInt32LSL()) {
        Int32BinopMatcher mleft(m.Left().Gate(), circuit_);
        if (mleft.Left().IsIcmp()) {
            // Check if the right shift amount is 31 (logical shift by 31 bits).
            if (m.Right().Is(31) && mleft.Right().Is(31)) {
                auto newGate = builder_.Int32Sub(builder_.Int32(0), mleft.Left().Gate());
                return ReplaceOld(gate, newGate);
            }
        } else if (mleft.Left().IsLoad()) {
            // Check if the right shift amount is 24 (logical shift by 24 bits).
            if (m.Right().Is(24) && mleft.Right().Is(24) &&
                acc_.GetMachineType(mleft.Left().Gate()) == MachineType::I8) {
                return mleft.Left().Gate();
            }
        }
    }
    return Circuit::NullGate();
}

GateRef InstructionCombine::ReduceWord64Lsl(GateRef gate)
{
    Int64BinopMatcher m(gate, circuit_);
    // x << 0 => x
    if (m.Right().Is(0)) {
        return m.Left().Gate();
    }
    if (m.IsFoldable()) {
        return builder_.Int64(ShlWithWraparound(m.Left().ResolvedValue(), m.Right().ResolvedValue()));
    }
    // Check if the right shift amount is in the range of 1 to 63 bits (inclusive).
    if (m.Right().IsInRange(1, 63) && (m.Left().IsmInt64ASR() || m.Left().IsmInt64LSR())) {
        Int64BinopMatcher mleft(m.Left().Gate(), circuit_);
        // (x >>> K) << K => x & ~(2^K - 1)
        // (x >> K) << K => x & ~(2^K - 1)
        if (mleft.Right().Is(m.Right().ResolvedValue())) {
            auto newGate = builder_.Int64And(
                mleft.Left().Gate(), builder_.Int64(std::numeric_limits<uint64_t>::max() << m.Right().ResolvedValue()));
            return ReplaceOld(gate, newGate);
        }
    }
    return Circuit::NullGate();
}
GateRef InstructionCombine::ReduceWord32Lsl(GateRef gate)
{
    Int32BinopMatcher m(gate, circuit_);
    // x << 0 => x
    if (m.Right().Is(0)) {
        return m.Left().Gate();
    }
    if (m.IsFoldable()) {
        return builder_.Int32(ShlWithWraparound(m.Left().ResolvedValue(), m.Right().ResolvedValue()));
    }

    // Check if the right shift amount is in the range of 1 to 31 bits (inclusive).
    if (m.Right().IsInRange(1, 31) && (m.Left().IsmInt32ASR() || m.Left().IsmInt32LSR())) {
        Int64BinopMatcher mleft(m.Left().Gate(), circuit_);
        // (x >>> K) << K => x & ~(2^K - 1)
        // (x >> K) << K => x & ~(2^K - 1)
        if (mleft.Right().Is(m.Right().ResolvedValue())) {
            auto newGate = builder_.Int32And(
                mleft.Left().Gate(), builder_.Int32(std::numeric_limits<uint64_t>::max() << m.Right().ResolvedValue()));
            return ReplaceOld(gate, newGate);
        }
    }
    return Circuit::NullGate();
}

} // namespace panda::ecmascript::kungfu