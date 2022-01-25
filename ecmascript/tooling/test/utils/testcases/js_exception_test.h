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

#ifndef ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_EXCEPTION_TEST_H
#define ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_EXCEPTION_TEST_H

#include "ecmascript/mem/c_string.h"
#include "ecmascript/tooling/test/utils/test_util.h"

namespace panda::tooling::ecmascript::test {
class JsExceptionTest : public TestEvents {
public:
    JsExceptionTest()
    {
        vmStart = [this] {
            location_ = TestUtil::GetLocation("exception.js", 22, 0, pandaFile_.c_str());
            ASSERT_TRUE(location_.GetMethodId().IsValid());
            return true;
        };

        breakpoint = [this](PtThread thread, const PtLocation &location) {
            ASSERT_TRUE(location.GetMethodId().IsValid());
            ASSERT_LOCATION_EQ(location, location_);
            ++breakpointCounter_;
            CVector<std::unique_ptr<CallFrame>> callFrames;
            ASSERT_TRUE(backend_->GenerateCallFrames(&callFrames));
            ASSERT_TRUE(callFrames.size() > 0);
            auto jsLocation = callFrames[0]->GetLocation();
            ASSERT_TRUE(jsLocation != nullptr);
            ASSERT_EQ(jsLocation->GetLine(), 22);
            ASSERT_EQ(jsLocation->GetColumn(), 0);
            TestUtil::SuspendUntilContinue(DebugEvent::BREAKPOINT, thread, location);
            return true;
        };

        exception = [this](PtThread thread, const PtLocation &location) {
            std::cout << "exception: 0" << std::endl;
            auto sourceLocation = TestUtil::GetSourceLocation(location, pandaFile_.c_str());
            ASSERT_EQ(sourceLocation.line, 17);
            ASSERT_EQ(sourceLocation.column, 27);
            ++exceptionCounter_;
            CVector<std::unique_ptr<CallFrame>> callFrames;
            ASSERT_TRUE(backend_->GenerateCallFrames(&callFrames));
            ASSERT_TRUE(callFrames.size() > 0);
            auto jsLocation = callFrames[0]->GetLocation();
            ASSERT_TRUE(jsLocation != nullptr);
            ASSERT_EQ(jsLocation->GetLine(), 17);
            ASSERT_EQ(jsLocation->GetColumn(), 27);
            TestUtil::SuspendUntilContinue(DebugEvent::EXCEPTION, thread, location);
            return true;
        };

        loadModule = [this](std::string_view moduleName) {
            if (flag_) {
                if (moduleName != pandaFile_) {
                    return true;
                }
                ASSERT_TRUE(backend_->NotifyScriptParsed(0, pandaFile_));
                flag_ = false;
                auto error = debugInterface_->SetBreakpoint(location_);
                ASSERT_FALSE(error.has_value());
            }
            return true;
        };

        scenario = [this]() {
            ASSERT_BREAKPOINT_SUCCESS(location_);
            TestUtil::Continue();
            TestUtil::WaitForException();
            TestUtil::Continue();
            ASSERT_SUCCESS(debugInterface_->RemoveBreakpoint(location_));
            ASSERT_EXITED();
            return true;
        };

        vmDeath = [this]() {
            ASSERT_EQ(breakpointCounter_, 1U);
            ASSERT_EQ(exceptionCounter_, 1U);
            return true;
        };
    }

    std::pair<CString, CString> GetEntryPoint() override
    {
        return {pandaFile_, entryPoint_};
    }
    ~JsExceptionTest() = default;
private:
    CString pandaFile_ = "/data/test/exception.abc";
    CString entryPoint_ = "_GLOBAL::func_main_0";
    PtLocation location_ {nullptr, PtLocation::EntityId(0), 0};
    size_t breakpointCounter_ = 0;
    size_t exceptionCounter_ = 0;
    bool flag_ = true;
};

std::unique_ptr<TestEvents> GetJsExceptionTest()
{
    return std::make_unique<JsExceptionTest>();
}
}  // namespace panda::tooling::ecmascript::test

#endif  // ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_EXCEPTION_TEST_H
