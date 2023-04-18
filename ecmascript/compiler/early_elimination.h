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

#ifndef ECMASCRIPT_COMPILER_EARLY_ELIMINATION_H
#define ECMASCRIPT_COMPILER_EARLY_ELIMINATION_H

#include "ecmascript/compiler/circuit_builder.h"
#include "ecmascript/compiler/gate_accessor.h"
#include "ecmascript/compiler/graph_visitor.h"
#include "ecmascript/mem/chunk_containers.h"

namespace panda::ecmascript::kungfu {
class EarlyElimination;

class DependInfoNode : public ChunkObject {
public:
    DependInfoNode(Chunk* chunk) : chunk_(chunk) {}
    ~DependInfoNode() = default;

    GateRef LookupFrameState() const;
    GateRef LookupNode(EarlyElimination* elimination, GateRef gate);
    GateRef LookupCheckedNode(EarlyElimination* elimination, GateRef gate);
    DependInfoNode* UpdateNode(GateRef gate);
    DependInfoNode* UpdateFrameState(GateRef framestate);
    DependInfoNode* UpdateStoreProperty(EarlyElimination* elimination, GateRef gate);
    bool Equals(DependInfoNode* that);
    void Merge(EarlyElimination* elimination, DependInfoNode* that);
    void CopyFrom(DependInfoNode *other)
    {
        head_ = other->head_;
        size_ = other->size_;
    }
private:
    struct Node {
        Node(GateRef gate, Node* next) : gate(gate), next(next) {}
        GateRef gate;
        Node *next;
    };

    GateRef frameState_ {Circuit::NullGate()};
    Node *head_{nullptr};
    size_t size_ {0};
    Chunk* chunk_;
};

class EarlyElimination : public GraphVisitor {
public:
    EarlyElimination(Circuit *circuit, bool enableLog, const std::string& name, Chunk* chunk)
        : GraphVisitor(circuit, chunk), enableLog_(enableLog),
        methodName_(name), dependChains_(chunk), renames_(chunk) {}

    ~EarlyElimination() = default;

    void Run();

    GateRef VisitGate(GateRef gate) override;
    bool CheckReplacement(GateRef lhs, GateRef rhs);
    bool CheckRenameReplacement(GateRef lhs, GateRef rhs);
    bool MayAccessOneMemory(GateRef lhs, GateRef rhs);
    bool CompareOrder(GateRef lhs, GateRef rhs);
private:
    bool IsLogEnabled() const
    {
        return enableLog_;
    }

    const std::string& GetMethodName() const
    {
        return methodName_;
    }

    DependInfoNode* GetDependChain(GateRef dependIn)
    {
        size_t idx = acc_.GetId(dependIn);
        ASSERT(idx <= circuit_->GetMaxGateId());
        return dependChains_[idx];
    }

    GateRef VisitDependEntry(GateRef gate);
    GateRef UpdateDependChain(GateRef gate, DependInfoNode* dependInfo);
    DependInfoNode* UpdateWrite(GateRef gate, DependInfoNode* dependInfo);
    GateRef TryEliminateGate(GateRef gate);
    GateRef TryEliminateFrameState(GateRef gate);
    GateRef TryEliminateOther(GateRef gate);
    GateRef TryEliminateDependSelector(GateRef gate);
    DependInfoNode* GetLoopDependInfo(GateRef depend);
    GateRef Rename(GateRef gate);

    bool enableLog_ {false};
    std::string methodName_;
    ChunkVector<DependInfoNode*> dependChains_;
    ChunkVector<GateRef> renames_;
};
}  // panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_EARLY_ELIMINATION_H
