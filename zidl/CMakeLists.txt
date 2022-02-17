# Copyright (c) 2021 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.8)

project(zidl CXX)

set(ZIDL_DIR ${CMAKE_SOURCE_DIR})

include_directories(
    ${ZIDL_DIR})

add_subdirectory(ast)
add_subdirectory(codegen)
add_subdirectory(metadata)
add_subdirectory(parser)
add_subdirectory(util)

set(SOURCES
    main.cpp)

set(LINK_LIBS
    -Wl,--whole-archive
    ast
    codegen
    metadata
    parser
    util
    -Wl,--no-whole-archive)

add_executable(zidl ${SOURCES})
target_link_libraries(zidl
    ${LINK_LIBS})