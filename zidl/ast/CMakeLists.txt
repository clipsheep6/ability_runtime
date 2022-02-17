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

include_directories(
    ${ZIDL_DIR})

set(SOURCES
    ast_array_type.cpp
    ast_boolean_type.cpp
    ast_byte_type.cpp
    ast_char_type.cpp
    ast_double_type.cpp
    ast_float_type.cpp
    ast_integer_type.cpp
    ast_interface_type.cpp
    ast_list_type.cpp
    ast_long_type.cpp
    ast_map_type.cpp
    ast_method.cpp
    ast_module.cpp
    ast_namespace.cpp
    ast_node.cpp
    ast_parameter.cpp
    ast_sequenceable_type.cpp
    ast_short_type.cpp
    ast_string_type.cpp
    ast_type.cpp
    ast_void_type.cpp)

add_library(ast STATIC
    ${SOURCES})