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

#ifndef ECMASCRIPT_COMPILER_LOG_H
#define ECMASCRIPT_COMPILER_LOG_H

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include "ecmascript/log_wrapper.h"
#include "ecmascript/mem/clock_scope.h"

namespace panda::ecmascript::kungfu {
class CompilerLog {
public:
    explicit CompilerLog(const std::string &logOpt, bool enableBCTrace = false);
    ~CompilerLog() = default;

    bool AllMethod() const
    {
        return allMethod_;
    }

    bool CertainMethod() const
    {
        return cerMethod_;
    }

    bool NoneMethod() const
    {
        return noneMethod_;
    }

    bool OutputCIR() const
    {
        return outputCIR_;
    }

    bool OutputLLIR() const
    {
        return outputLLIR_;
    }

    bool OutputASM() const
    {
        return outputASM_;
    }

    bool OutputType() const
    {
        return outputType_;
    }

    bool IsEnableByteCodeTrace() const
    {
        return enableBCTrace_;
    }

    bool GetEnableCompilerLogTime() const
    {
        return compilerLogTime_;
    }

    void SetEnableCompilerLogTime(bool compilerLogTime)
    {
        compilerLogTime_ = compilerLogTime;
    }

    void PrintPassTime() const;
    void PrintMethodTime() const;
    void PrintTime() const;
    void AddMethodTime(const std::string& name, double time);
    void AddPassTime(const std::string& name, double time);

private:
    static constexpr int PASS_LENS = 25;
    static constexpr int METHOD_LENS = 18;
    static constexpr int PERCENT_LENS = 4;
    static constexpr int TIME_LENS = 12;
    static constexpr int MILLION_TIME = 1000;
    static constexpr int HUNDRED_TIME = 100;

    bool allMethod_ {false};
    bool cerMethod_ {false};
    bool noneMethod_ {false};
    bool outputCIR_ {false};
    bool outputLLIR_ {false};
    bool outputASM_ {false};
    bool outputType_ {false};
    bool enableBCTrace_ {false};
    bool compilerLogTime_ {true};
    std::map<std::string, double> timePassMap_ {};
    std::map<std::string, double> timeMethodMap_ {};
};

class MethodLogList {
public:
    explicit MethodLogList(const std::string &logMethods) : methods_(logMethods) {}
    ~MethodLogList() = default;
    bool IncludesMethod(const std::string &methodName) const;
private:
    std::string methods_ {};
};

class AotMethodLogList : public MethodLogList {
public:
    static const char fileSplitSign = ':';
    static const char methodSplitSign = ',';

    explicit AotMethodLogList(const std::string &logMethods) : MethodLogList(logMethods)
    {
        ParseFileMethodsName(logMethods);
    }
    ~AotMethodLogList() = default;

    bool IncludesMethod(const std::string &fileName, const std::string &methodName) const;

private:
    std::vector<std::string> spiltString(const std::string &str, const char ch);
    void ParseFileMethodsName(const std::string &logMethods);
    std::map<std::string, std::vector<std::string>> fileMethods_ {};
};

class TimeScope : public ClockScope {
public:
    TimeScope(std::string name, std::string methodName, CompilerLog* log);
    ~TimeScope();
   
private:
    static constexpr int PASS_LENS = 25;
    static constexpr int TIME_LENS = 12;
    static constexpr int MILLION_TIME = 1000;

    std::string name_ {""};
    double startTime_ {0};
    double timeUsed_ {0};
    std::string methodName_ {""};
    CompilerLog *log_ {nullptr};

    const std::string GetShortName(const std::string& methodName);
};
}  // namespace panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_LOG_H
