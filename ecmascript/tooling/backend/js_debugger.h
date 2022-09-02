/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef ECMASCRIPT_TOOLING_BACKEND_JS_DEBUGGER_H
#define ECMASCRIPT_TOOLING_BACKEND_JS_DEBUGGER_H

#include "ecmascript/ecma_vm.h"
#include "ecmascript/jspandafile/method_literal.h"
#include "ecmascript/tooling/backend/debugger_api.h"
#include "ecmascript/tooling/base/pt_method.h"
#include "ecmascript/tooling/interface/notification_manager.h"
#include "ecmascript/tooling/interface/js_debugger_manager.h"

namespace panda::ecmascript::tooling {
class JSBreakpoint {
public:
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    JSBreakpoint(PtMethod *ptMethod, uint32_t bcOffset, const Global<FunctionRef> &condFuncRef)
        : ptMethod_(ptMethod), bcOffset_(bcOffset), condFuncRef_(condFuncRef) {}
    ~JSBreakpoint() = default;

    PtMethod *GetPtMethod() const
    {
        return ptMethod_;
    }

    uint32_t GetBytecodeOffset() const
    {
        return bcOffset_;
    }

    bool operator==(const JSBreakpoint &bpoint) const
    {
        return ptMethod_->GetMethodId() == bpoint.GetPtMethod()->GetMethodId() &&
            ptMethod_->GetJSPandaFile() == bpoint.GetPtMethod()->GetJSPandaFile() &&
            GetBytecodeOffset() == bpoint.GetBytecodeOffset();
    }

    const Global<FunctionRef> &GetConditionFunction()
    {
        return condFuncRef_;
    }

    DEFAULT_COPY_SEMANTIC(JSBreakpoint);
    DEFAULT_MOVE_SEMANTIC(JSBreakpoint);

private:
    PtMethod *ptMethod_ {nullptr};
    uint32_t bcOffset_;
    Global<FunctionRef> condFuncRef_;
};

class HashJSBreakpoint {
public:
    size_t operator()(const JSBreakpoint &bpoint) const
    {
        return (std::hash<PtMethod *>()(bpoint.GetPtMethod())) ^ (std::hash<uint32_t>()(bpoint.GetBytecodeOffset()));
    }
};

class JSDebugger : public JSDebugInterface, RuntimeListener {
public:
    explicit JSDebugger(const EcmaVM *vm) : ecmaVm_(vm)
    {
        notificationMgr_ = ecmaVm_->GetJsDebuggerManager()->GetNotificationManager();
        notificationMgr_->AddListener(this);
    }
    ~JSDebugger() override
    {
        notificationMgr_->RemoveListener();
    }

    void RegisterHooks(PtHooks *hooks) override
    {
        hooks_ = hooks;
        // send vm start event after add hooks
        notificationMgr_->VmStartEvent();
    }
    void UnregisterHooks() override
    {
        // send vm death event before delete hooks
        notificationMgr_->VmDeathEvent();
        hooks_ = nullptr;
    }

    bool SetBreakpoint(const JSPtLocation &location, Local<FunctionRef> condFuncRef) override;
    bool RemoveBreakpoint(const JSPtLocation &location) override;
    void BytecodePcChanged(JSThread *thread, JSHandle<Method> method, uint32_t bcOffset) override;
    void LoadModule(std::string_view filename, std::string_view entryPoint) override
    {
        if (hooks_ == nullptr) {
            return;
        }
        hooks_->LoadModule(filename, entryPoint);
    }
    void VmStart() override
    {
        if (hooks_ == nullptr) {
            return;
        }
        hooks_->VmStart();
    }
    void VmDeath() override
    {
        if (hooks_ == nullptr) {
            return;
        }
        hooks_->VmDeath();
    }
    void PendingJobEntry() override
    {
        if (hooks_ == nullptr) {
            return;
        }
        hooks_->PendingJobEntry();
    }

private:
    std::unique_ptr<PtMethod> FindMethod(const JSPtLocation &location) const;
    std::optional<JSBreakpoint> FindBreakpoint(JSHandle<Method> method, uint32_t bcOffset) const;
    bool RemoveBreakpoint(const std::unique_ptr<PtMethod> &ptMethod, uint32_t bcOffset);
    void HandleExceptionThrowEvent(const JSThread *thread, JSHandle<Method> method, uint32_t bcOffset);
    bool HandleStep(JSHandle<Method> method, uint32_t bcOffset);
    bool HandleBreakpoint(JSHandle<Method> method, uint32_t bcOffset);

    const EcmaVM *ecmaVm_;
    PtHooks *hooks_ {nullptr};
    NotificationManager *notificationMgr_ {nullptr};

    CUnorderedSet<JSBreakpoint, HashJSBreakpoint> breakpoints_ {};
};
}  // namespace panda::ecmascript::tooling

#endif  // ECMASCRIPT_TOOLING_BACKEND_JS_DEBUGGER_H
