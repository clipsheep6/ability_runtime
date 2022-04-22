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

#ifndef ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_BREAKPOINT_TEST_H
#define ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_BREAKPOINT_TEST_H

#include "ecmascript/tooling/test/utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsBreakpointTest : public TestEvents {
public:
    JsBreakpointTest()
    {
        vmStart = [this] {
            location_ = TestUtil::GetLocation("Sample.js", 22, 0, pandaFile_.c_str());
            ASSERT_TRUE(location_.GetMethodId().IsValid());
            return true;
        };

        breakpoint = [this](const JSPtLocation &location) {
            ASSERT_TRUE(location.GetMethodId().IsValid());
            ASSERT_LOCATION_EQ(location, location_);
            ++breakpointCounter_;
            TestUtil::SuspendUntilContinue(DebugEvent::BREAKPOINT, location);
            return true;
        };

        loadModule = [this](std::string_view moduleName) {
            if (flag_) {
                if (moduleName != pandaFile_) {
                    return true;
                }
                ASSERT_TRUE(backend_->NotifyScriptParsed(0, pandaFile_));
                flag_ = false;
                auto ret = debugInterface_->SetBreakpoint(location_);
                ASSERT_TRUE(ret);
            }
            return true;
        };

        scenario = [this]() {
            ASSERT_BREAKPOINT_SUCCESS(location_);
            TestUtil::Continue();
            ASSERT_BREAKPOINT_SUCCESS(location_);
            TestUtil::Continue();
            ASSERT_TRUE(debugInterface_->RemoveBreakpoint(location_));
            ASSERT_EXITED();
            return true;
        };

        vmDeath = [this]() {
            ASSERT_EQ(breakpointCounter_, 2U);
            return true;
        };
    }

    std::pair<CString, CString> GetEntryPoint() override
    {
        return {pandaFile_, entryPoint_};
    }
    ~JsBreakpointTest() = default;

private:
    CString pandaFile_ = DEBUGGER_ABC_DIR "Sample.abc";
    CString entryPoint_ = "_GLOBAL::func_main_0";
    JSPtLocation location_ {nullptr, JSPtLocation::EntityId(0), 0};
    size_t breakpointCounter_ = 0;
    bool flag_ = true;
};

std::unique_ptr<TestEvents> GetJsBreakpointTest()
{
    return std::make_unique<JsBreakpointTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_BREAKPOINT_TEST_H
