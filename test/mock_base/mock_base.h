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

#ifndef MOCK_BASE_H
#define MOCK_BASE_H

#define GEN_MOCK_STATE_CON(methodName, number) methodName##number

#define MOCK_STATIC_METHOD1_WITH_RETURN_VAL(methodName, methodReturnToken, methodTokens) \
    static methodReturnToken methodName##methodTokens \
    { \
        return GEN_MOCK_STATE_CON(methodName, 1);
    } \
    inline static methodReturnToken GEN_MOCK_STATE_CON(methodName, 1);

#define MOCK_STATIC_METHOD2_WITH_RETURN_VAL(methodName, methodReturnToken, methodTokens) \
    static methodReturnToken methodName##methodTokens \
    { \
        return GEN_MOCK_STATE_CON(methodName, 2);
    } \
    inline static methodReturnToken GEN_MOCK_STATE_CON(methodName, 2);

#define MOCK_STATIC_METHOD3_WITH_RETURN_VAL(methodName, methodReturnToken, methodTokens) \
    static methodReturnToken methodName##methodTokens \
    { \
        return GEN_MOCK_STATE_CON(methodName, 3);
    } \
    inline static methodReturnToken GEN_MOCK_STATE_CON(methodName, 3);

#define MOCK_STATIC_METHOD4_WITH_RETURN_VAL(methodName, methodReturnToken, methodTokens) \
    static methodReturnToken methodName##methodTokens \
    { \
        return GEN_MOCK_STATE_CON(methodName, 4);
    } \
    inline static methodReturnToken GEN_MOCK_STATE_CON(methodName, 4);

#define EXPECT_CALL_RET(className, methodName, number, expectRet) \
    className::GEN_MOCK_STATE_CON = expectRet;
#endif // MOCK_BASE_H
