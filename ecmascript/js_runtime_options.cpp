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

#include "ecmascript/js_runtime_options.h"

#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <getopt.h>

#include "ecmascript/mem/mem_common.h"

namespace panda::ecmascript {
const std::string PUBLIC_API COMMON_HELP_HEAD_MSG =
    "Usage: jsvm  <option>  <filename.abc>\n"
    "\n"
    "Options:\n";

const std::string PUBLIC_API COMPILER_HELP_HEAD_MSG =
    "Usage: ark_aot_compiler  <option>  --aot-file=<filename>  <filename.abc>\n"
    "\n"
    "Options:\n";

const std::string PUBLIC_API STUB_HELP_HEAD_MSG =
    "Usage: ark_stub_compiler <option>\n"
    "\n"
    "Options:\n";

const std::string PUBLIC_API HELP_OPTION_MSG =
    "--aot-file:                           Path (file suffix not needed) to AOT output file. Default: 'aot_file'\n"
    "--ark-properties:                     Set ark properties\n"
    "--ark-bundle-name:                    Set ark bundle name\n"
    "--asm-interpreter:                    Enable asm interpreter. Default: 'true'\n"
    "--asm-opcode-disable-range:           Opcode range when asm interpreter is enabled.\n"
    "--assert-types:                       Enable type assertion for type inference tests. Default: 'false'\n"
    "--builtins-dts:                       Builtins.d.abc file path for AOT.\n"
    "--compiler-log:                       Log Option For aot compiler and stub compiler,\n"
    "                                      'none': no log,\n"
    "                                      'allllircirasm' or 'all012': print all log for all methods,\n"
    "                                      'allcir' or 'all0': print IR info for all methods,\n"
    "                                      'allllir' or 'all1': print llir info for all methods,\n"
    "                                      'allasm' or 'all2': print asm log for all methods,\n"
    "                                      'alltype' or 'all3': print type infer log for all methods,\n"
    "                                      'cerllircirasm' or 'cer0112': print all log for certain method defined "
                                           "in 'mlist-for-log',\n"
    "                                      'cercir' or 'cer0': print IR for methods in 'mlist-for-log',\n"
    "                                      'cerasm' or 'cer2': print log for methods in 'mlist-for-log',\n"
    "                                      Default: 'none'\n"
    "--compiler-log-methods:               Specific method list for compiler log, only used when compiler-log."
                                           "Default: 'none'\n"
    "--compiler-type-threshold:            enable to skip methods whose type is no more than threshold.. Default: -1\n"
    "--compiler-log-snapshot:              Enable to print snapshot information. Default: 'false'\n"
    "--compiler-log-time:                  Enable to print pass compiler time. Default: 'false'\n"
    "--enable-ark-tools:                   Enable ark tools to debug. Default: 'false'\n"
    "--trace-bc:                           Enable tracing bytecode for aot runtime. Default: 'false'\n"
    "--trace-deopt:                        Enable tracing deopt for aot runtime. Default: 'false'\n"
    "--deopt-threshold:                    Set max count which aot function can occur deoptimization. Default: '10'\n"
    "--opt-code-profiler:                  Enable opt code Bytecode Statistics for aot runtime. Default: 'false'\n"
    "--enable-force-gc:                    Enable force gc when allocating object. Default: 'true'\n"
    "--enable-ic:                          Switch of inline cache. Default: 'true'\n"
    "--enable-runtime-stat:                Enable statistics of runtime state. Default: 'false'\n"
    "--enable-type-lowering:               Enable TSTypeLowering and TypeLowering for aot runtime. Default: 'true'\n"
    "--enable-opt-inlining:                Enable inlining function for aot compiler: Default: 'false'\n"
    "--entry-point:                        Full name of entrypoint function. Default: '_GLOBAL::func_main_0'\n"
    "--force-full-gc:                      If true trigger full gc, else trigger semi and old gc. Default: 'true'\n"
    "--framework-abc-file:                 Snapshot file. Default: 'strip.native.min.abc'\n"
    "--gcThreadNum:                        Set gcThreadNum. Default: '7'\n"
    "--heap-size-limit:                    Max heap size (MB). Default: '512'\n"
    "--help:                               Print this message and exit\n"
    "--icu-data-path:                      Path to generated icu data file. Default: 'default'\n"
    "--IsWorker:                           Whether is worker vm. Default: 'false'\n"
    "--log-level:                          Log level: ['debug', 'info', 'warning', 'error', 'fatal'].\n"
    "--log-components:                     Enable logs from specified components: ['all', 'gc', 'ecma',\n"
    "                                      'interpreter', 'debugger', 'compiler', 'builtins', 'all']. \n"
    "                                      Default: 'all'\n"
    "--log-debug:                          Enable debug or above logs for components: ['all', 'gc', 'ecma',\n"
    "                                      'interpreter', 'debugger', 'compiler', 'builtins', 'all'].\n"
    "                                      Default: 'all'\n"
    "--log-error:                          Enable error log for components: ['all', 'gc', 'ecma',\n"
    "                                      'interpreter', 'debugger', 'compiler', 'builtins', 'all']. \n"
    "                                      Default: 'all'\n"
    "--log-fatal:                          Enable fatal log for components: ['all', 'gc', 'ecma',\n"
    "                                      'interpreter', 'debugger', 'compiler', 'builtins', 'all']. \n"
    "                                      Default: 'all'\n"
    "--log-info:                           Enable info log for components: ['all', 'gc', 'ecma',\n"
    "                                      'interpreter', 'debugger', 'compiler', 'builtins', 'all']. \n"
    "                                      Default: 'all'\n"
    "--log-warning:                        Enable warning log for components: ['all', 'gc', 'ecma',\n"
    "                                      'interpreter', 'debugger', 'compiler', 'builtins', \n"
    "                                      'all']. Default: 'all'\n"
    "--longPauseTime:                      Set longPauseTime in millisecond. Default: '40'\n"
    "--maxAotMethodSize:                   Enable aot compiler to skip method larger than limit (KB). Default: '32'\n"
    "--maxNonmovableSpaceCapacity:         Set max non-movable space capacity\n"
    "--merge-abc:                          ABC file is merge abc. Default: 'false'\n"
    "--opt-level:                          Optimization level configuration of aot compiler. Default: '3'\n"
    "--options:                            Print compiler and runtime options\n"
    "--print-any-types:                    Enable print any types after type inference. Default: 'false'\n"
    "--serializer-buffer-size-limit:       Max serializer buffer size used by the VM in Byte. Default size is 2GB\n"
    "--snapshot-file:                      Snapshot file. Default: '/system/etc/snapshot'\n"
    "--startup-time:                       Print the start time of command execution. Default: 'false'\n"
    "--stub-file:                          Path of file includes common stubs module compiled by stub compiler. "
                                           "Default: 'stub.an'\n"
    "--enable-pgo-profiler:                Enable pgo profiler to sample jsfunction call and output to file. "
                                           "Default: 'false'\n"
    "--pgo-hotness-threshold:              Set hotness threshold for pgo in aot compiler. Default: '2'\n"
    "--pgo-profiler-path:                  The pgo file output dir or the pgo file dir of AOT compiler. Default: ''\n"
    "--target-triple:                      CPU triple for aot compiler or stub compiler. \n"
    "                                      values: ['x86_64-unknown-linux-gnu', 'arm-unknown-linux-gnu', \n"
    "                                      'aarch64-unknown-linux-gnu'], Default: 'x86_64-unknown-linux-gnu'\n"
    "--enable-print-execute-time:          Enable print execute panda file spent time\n\n";

bool JSRuntimeOptions::ParseCommand(const int argc, const char **argv)
{
    const struct option longOptions[] = {
        {"aot-file", required_argument, nullptr, OPTION_AOT_FILE},
        {"ark-properties", required_argument, nullptr, OPTION_ARK_PROPERTIES},
        {"ark-bundleName", required_argument, nullptr, OPTION_ARK_BUNDLENAME},
        {"asm-interpreter", required_argument, nullptr, OPTION_ENABLE_ASM_INTERPRETER},
        {"asm-opcode-disable-range", required_argument, nullptr, OPTION_ASM_OPCODE_DISABLE_RANGE},
        {"assert-types", required_argument, nullptr, OPTION_ASSERT_TYPES},
        {"builtins-dts", required_argument, nullptr, OPTION_BUILTINS_DTS},
        {"compiler-log", required_argument, nullptr, OPTION_COMPILER_LOG_OPT},
        {"compiler-log-methods", required_argument, nullptr, OPTION_COMPILER_LOG_METHODS},
        {"compiler-log-snapshot", required_argument, nullptr, OPTION_COMPILER_LOG_SNAPSHOT},
        {"compiler-log-time", required_argument, nullptr, OPTION_COMPILER_LOG_TIME},
        {"compiler-type-threshold", required_argument, nullptr, OPTION_COMPILER_TYPE_THRESHOLD},
        {"enable-ark-tools", required_argument, nullptr, OPTION_ENABLE_ARK_TOOLS},
        {"trace-bc", required_argument, nullptr, OPTION_TRACE_BC},
        {"trace-deopt", required_argument, nullptr, OPTION_TRACE_DEOPT},
        {"deopt-threshold", required_argument, nullptr, OPTION_DEOPT_THRESHOLD},
        {"opt-code-profiler", required_argument, nullptr, OPTION_OPT_CODE_PROFILER},
        {"enable-force-gc", required_argument, nullptr, OPTION_ENABLE_FORCE_GC},
        {"enable-ic", required_argument, nullptr, OPTION_ENABLE_IC},
        {"enable-runtime-stat", required_argument, nullptr, OPTION_ENABLE_RUNTIME_STAT},
        {"enable-type-lowering", required_argument, nullptr, OPTION_ENABLE_TYPE_LOWERING},
        {"enable-opt-inlining", required_argument, nullptr, OPTION_ENABLE_OPT_INLINING},
        {"enable-opt-pgotype", required_argument, nullptr, OPTION_ENABLE_OPT_PGOTYPE},
        {"entry-point", required_argument, nullptr, OPTION_ENTRY_POINT},
        {"force-full-gc", required_argument, nullptr, OPTION_FORCE_FULL_GC},
        {"gcThreadNum", required_argument, nullptr, OPTION_GC_THREADNUM},
        {"heap-size-limit", required_argument, nullptr, OPTION_HEAP_SIZE_LIMIT},
        {"help", no_argument, nullptr, OPTION_HELP},
        {"icu-data-path", required_argument, nullptr, OPTION_ICU_DATA_PATH},
        {"IsWorker", required_argument, nullptr, OPTION_IS_WORKER},
        {"log-components", required_argument, nullptr, OPTION_LOG_COMPONENTS},
        {"log-debug", required_argument, nullptr, OPTION_LOG_DEBUG},
        {"log-error", required_argument, nullptr, OPTION_LOG_ERROR},
        {"log-fatal", required_argument, nullptr, OPTION_LOG_FATAL},
        {"log-info", required_argument, nullptr, OPTION_LOG_INFO},
        {"log-level", required_argument, nullptr, OPTION_LOG_LEVEL},
        {"log-warning", required_argument, nullptr, OPTION_LOG_WARNING},
        {"longPauseTime", required_argument, nullptr, OPTION_LONG_PAUSE_TIME},
        {"maxAotMethodSize", required_argument, nullptr, OPTION_MAX_AOTMETHODSIZE},
        {"maxNonmovableSpaceCapacity", required_argument, nullptr, OPTION_MAX_NONMOVABLE_SPACE_CAPACITY},
        {"merge-abc", required_argument, nullptr, OPTION_MERGE_ABC},
        {"opt-level", required_argument, nullptr, OPTION_ASM_OPT_LEVEL},
        {"options", no_argument, nullptr, OPTION_OPTIONS},
        {"print-any-types", required_argument, nullptr, OPTION_PRINT_ANY_TYPES},
        {"reloc-mode", required_argument, nullptr, OPTION_RELOCATION_MODE},
        {"serializer-buffer-size-limit", required_argument, nullptr, OPTION_SERIALIZER_BUFFER_SIZE_LIMIT},
        {"startup-time", required_argument, nullptr, OPTION_STARTUP_TIME},
        {"stub-file", required_argument, nullptr, OPTION_STUB_FILE},
        {"target-triple", required_argument, nullptr, OPTION_TARGET_TRIPLE},
        {"enable-print-execute-time", required_argument, nullptr, OPTION_PRINT_EXECUTE_TIME},
        {"enable-pgo-profiler", required_argument, nullptr, OPTION_ENABLE_PGO_PROFILER},
        {"pgo-profiler-path", required_argument, nullptr, OPTION_PGO_PROFILER_PATH},
        {"pgo-hotness-threshold", required_argument, nullptr, OPTION_PGO_HOTNESS_THRESHOLD},
        {nullptr, 0, nullptr, 0},
    };

    int index = 0;
    opterr = 0;
    bool ret = false;
    int option = 0;
    arg_list_t argListStr;
    uint32_t argUint32 = 0;
    int argInt = 0;
    bool argBool = false;
    double argDouble = 0.0;
    static std::string COLON = ":";

    if (argc <= 1) {
        return true;
    }

    while (true) {
        option = getopt_long_only(argc, const_cast<char **>(argv), "", longOptions, &index);
        LOG_ECMA(INFO) << "option: " << option << ", optopt: " << optopt << ", optind: " << optind;

        if (optind <= 0 || optind > argc) {
            return false;
        }

        if (option == -1) {
            return true;
        }

        // unknown option or required_argument option has no argument
        if (option == '?') {
            ret = SetDefaultValue(const_cast<char *>(argv[optind - 1]));
            if (ret) {
                continue;
            } else {
                return ret;
            }
        }

        WasSet(option);
        switch (option) {
            case OPTION_AOT_FILE:
                SetAOTOutputFile(optarg);
                break;
            case OPTION_ARK_PROPERTIES:
                ret = ParseIntParam("ark-properties", &argInt);
                if (ret) {
                    SetArkProperties(argInt);
                } else {
                    return false;
                }
                break;
            case OPTION_ARK_BUNDLENAME:
                SetArkBundleName(optarg);
                break;
            case OPTION_ENABLE_ASM_INTERPRETER:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetEnableAsmInterpreter(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_ASM_OPCODE_DISABLE_RANGE:
                SetAsmOpcodeDisableRange(optarg);
                break;
            case OPTION_ASSERT_TYPES:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetAssertTypes(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_BUILTINS_DTS:
                SetBuiltinsDTS(optarg);
                break;
            case OPTION_COMPILER_LOG_OPT:
                SetCompilerLogOption(optarg);
                break;
            case OPTION_COMPILER_LOG_SNAPSHOT:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetCompilerLogSnapshot(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_COMPILER_LOG_TIME:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetCompilerLogTime(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_ENABLE_ARK_TOOLS:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetEnableArkTools(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_TRACE_BC:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetTraceBc(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_TRACE_DEOPT:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetTraceDeopt(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_OPT_CODE_PROFILER:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetOptCodeProfiler(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_ENABLE_FORCE_GC:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetEnableForceGC(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_FORCE_FULL_GC:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetForceFullGC(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_ENABLE_IC:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetEnableIC(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_ENABLE_RUNTIME_STAT:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetEnableRuntimeStat(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_GC_THREADNUM:
                ret = ParseUint32Param("gcThreadNum", &argUint32);
                if (ret) {
                    SetGcThreadNum(argUint32);
                } else {
                    return false;
                }
                break;
            case OPTION_HEAP_SIZE_LIMIT:
                ret = ParseUint32Param("heap-size-limit", &argUint32);
                if (ret) {
                    SetHeapSizeLimit(argUint32);
                } else {
                    return false;
                }
                break;
            case OPTION_HELP:
                return false;
            case OPTION_ICU_DATA_PATH:
                SetIcuDataPath(optarg);
                break;
            case OPTION_IS_WORKER:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetIsWorker(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_LOG_COMPONENTS:
                ParseListArgParam(optarg, &argListStr, COLON);
                SetLogComponents(argListStr);
                break;
            case OPTION_LOG_DEBUG:
                ParseListArgParam(optarg, &argListStr, COLON);
                SetLogDebug(argListStr);
                break;
            case OPTION_LOG_ERROR:
                ParseListArgParam(optarg, &argListStr, COLON);
                SetLogError(argListStr);
                break;
            case OPTION_LOG_FATAL:
                ParseListArgParam(optarg, &argListStr, COLON);
                SetLogFatal(argListStr);
                break;
            case OPTION_LOG_INFO:
                ParseListArgParam(optarg, &argListStr, COLON);
                SetLogInfo(argListStr);
                break;
            case OPTION_LOG_LEVEL:
                SetLogLevel(optarg);
                break;
            case OPTION_LOG_WARNING:
                ParseListArgParam(optarg, &argListStr, COLON);
                SetLogWarning(argListStr);
                break;
            case OPTION_LONG_PAUSE_TIME:
                ret = ParseUint32Param("longPauseTime", &argUint32);
                if (ret) {
                    SetLongPauseTime(argUint32);
                } else {
                    return false;
                }
                break;
            case OPTION_MAX_AOTMETHODSIZE:
                ret = ParseUint32Param("maxAotMethodSize", &argUint32);
                if (ret) {
                    SetMaxAotMethodSize(argUint32);
                } else {
                    return false;
                }
                break;
            case OPTION_COMPILER_TYPE_THRESHOLD:
                ret = ParseDoubleParam("compiler-type-threshold", &argDouble);
                if (ret) {
                    SetTypeThreshold(argDouble);
                } else {
                    return false;
                }
                break;
            case OPTION_MAX_NONMOVABLE_SPACE_CAPACITY:
                ret = ParseUint32Param("maxNonmovableSpaceCapacity", &argUint32);
                if (ret) {
                    SetMaxNonmovableSpaceCapacity(argUint32);
                } else {
                    return false;
                }
                break;
            case OPTION_COMPILER_LOG_METHODS:
                SetMethodsListForLog(optarg);
                break;
            case OPTION_ASM_OPT_LEVEL:
                ret = ParseUint32Param("opt-level", &argUint32);
                if (ret) {
                    SetOptLevel(argUint32);
                } else {
                    return false;
                }
                break;
            case OPTION_PRINT_ANY_TYPES:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetPrintAnyTypes(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_PRINT_EXECUTE_TIME:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetEnablePrintExecuteTime(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_ENABLE_PGO_PROFILER:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetEnablePGOProfiler(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_PGO_PROFILER_PATH:
                if (*optarg == '\0') {
                    return false;
                }
                SetPGOProfilerPath(optarg);
                break;
            case OPTION_PGO_HOTNESS_THRESHOLD:
                ret = ParseUint32Param("pgo-hotness-threshold", &argUint32);
                if (ret) {
                    SetPGOHotnessThreshold(argUint32);
                } else {
                    return false;
                }
                break;
            case OPTION_RELOCATION_MODE:
                ret = ParseUint32Param("reloc-mode", &argUint32);
                if (ret) {
                    SetRelocMode(argUint32);
                } else {
                    return false;
                }
                break;
            case OPTION_SERIALIZER_BUFFER_SIZE_LIMIT:
                uint64_t argUInt64;
                ret = ParseUint64Param("serializer-buffer-size-limit", &argUInt64);
                if (ret) {
                    SetSerializerBufferSizeLimit(argUInt64);
                } else {
                    return false;
                }
                break;
            case OPTION_STARTUP_TIME:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetStartupTime(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_STUB_FILE:
                SetStubFile(optarg);
                break;
            case OPTION_TARGET_TRIPLE:
                SetTargetTriple(optarg);
                break;
            case OPTION_ENTRY_POINT:
                SetEntryPoint(optarg);
                break;
            case OPTION_MERGE_ABC:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetMergeAbc(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_ENABLE_TYPE_LOWERING:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetEnableTypeLowering(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_ENABLE_OPT_INLINING:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetEnableOptInlining(argBool);
                } else {
                    return false;
                }
                break;
            case OPTION_ENABLE_OPT_PGOTYPE:
                ret = ParseBoolParam(&argBool);
                if (ret) {
                    SetEnableOptPGOType(argBool);
                } else {
                    return false;
                }
                break;
            default:
                std::cerr << "Invalid option\n"<< std::endl;
                return false;
        }
    }
}

bool JSRuntimeOptions::SetDefaultValue(char* argv)
{
    WasSet(optopt);

    if (optopt == OPTION_DEFAULT) { // unknown option
        std::cerr << " Invalid option \"" << argv << "\"" << std::endl;
        return false;
    }

    if (optopt > OPTION_OPTIONS) { // unknown argument
        std::cerr << "getopt: \"" << argv <<"\" argument has invalid parameter value \n" << std::endl;
        return false;
    }
    return true;
}

bool JSRuntimeOptions::ParseBoolParam(bool* argBool)
{
    if ((strcmp(optarg, "false") == 0) || (strcmp(optarg, "0") == 0)) {
        *argBool = false;
    } else {
        *argBool = true;
    }
    return true;
}

bool JSRuntimeOptions::ParseDoubleParam(const std::string &option, double *argDouble)
{
    *argDouble = std::stod(optarg, nullptr);
    if (errno == ERANGE) {
        std::cerr << "getopt: \"" << option << "\" argument has invalid parameter value \""
            << optarg <<"\"\n" << std::endl;
        return false;
    }
    return true;
}

bool JSRuntimeOptions::ParseIntParam(const std::string &option, int *argInt)
{
    if (StartsWith(optarg, "0x")) {
        const int HEX = 16;
        *argInt = std::stoi(optarg, nullptr, HEX);
    } else {
        *argInt = std::stoi(optarg);
    }

    if (errno == ERANGE) {
        std::cerr << "getopt: \"" << option << "\" argument has invalid parameter value \""
            << optarg <<"\"\n" << std::endl;
        return false;
    }
    return true;
}

bool JSRuntimeOptions::ParseUint32Param(const std::string &option, uint32_t *argUInt32)
{
    if (StartsWith(optarg, "0x")) {
        const int HEX = 16;
        *argUInt32 = std::strtoull(optarg, nullptr, HEX);
    } else {
        const int DEC = 10;
        *argUInt32 = std::strtoull(optarg, nullptr, DEC);
    }

    if (errno == ERANGE) {
        std::cerr << "getopt: \"" << option << "\" argument has invalid parameter value \""
            << optarg <<"\"\n" << std::endl;
        return false;
    }
    return true;
}

bool JSRuntimeOptions::ParseUint64Param(const std::string &option, uint64_t *argUInt64)
{
    if (StartsWith(optarg, "0x")) {
        const int HEX = 16;
        *argUInt64 = std::strtoull(optarg, nullptr, HEX);
    } else {
        const int DEC = 10;
        *argUInt64 = std::strtoull(optarg, nullptr, DEC);
    }

    if (errno == ERANGE) {
        std::cerr << "getopt: \"" << option << "\" argument has invalid parameter value \""
            << optarg <<"\"\n" << std::endl;
        return false;
    }
    return true;
}

void JSRuntimeOptions::ParseListArgParam(const std::string &option, arg_list_t *argListStr, std::string delimiter)
{
    argListStr->clear();
    std::size_t strIndex = 0;
    std::size_t pos = option.find_first_of(delimiter, strIndex);
    while ((pos < option.size()) && (pos > strIndex)) {
        argListStr->push_back(option.substr(strIndex, pos - strIndex));
        strIndex = pos;
        strIndex = option.find_first_not_of(delimiter, strIndex);
        pos = option.find_first_of(delimiter, strIndex);
    }
    if (pos > strIndex) {
        argListStr->push_back(option.substr(strIndex, pos - strIndex));
    }
    return;
}
}
