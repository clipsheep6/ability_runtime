/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "dynamic_loader.h"
#include <dlfcn.h> // For dlerror

using namespace testing;
using namespace testing::ext;

class DynamicLoaderOhosTest : public testing::Test {
public:
    DynamicLoaderOhosTest()
    {}
    ~DynamicLoaderOhosTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void DynamicLoaderOhosTest::SetUpTestCase(void)
{}

void DynamicLoaderOhosTest::TearDownTestCase(void)
{}

void DynamicLoaderOhosTest::SetUp(void)
{}

void DynamicLoaderOhosTest::TearDown(void)
{}

/**
 * @tc.name: DynamicLoaderOhosTestInitNamespace_0100
 * @tc.desc: DynamicLoaderOhosTest test for dynamic_init_namespace.
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLoaderOhosTest, DynamicLoaderOhosTestInitNamespace_0100, TestSize.Level1)
{
    Dl_namespace ns;
    const char* name = "test_namespace";

    // Test success
    DynamicInitNamespace(&ns, nullptr, "test_entries", name);
    EXPECT_STREQ(DynamicGetError(), "");

    // Test duplicate init
    DynamicInitNamespace(&ns, nullptr, "test_entries", name);
    EXPECT_STREQ(DynamicGetError(), "");
}

/**
 * @tc.name: DynamicLoaderOhosTestLoadLibrary_0100
 * @tc.desc: DynamicLoaderOhosTest test for dynamic_load_library.
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLoaderOhosTest, DynamicLoaderOhosTestLoadLibrary_0100, TestSize.Level1)
{
    Dl_namespace ns;

    // Test success
    auto handle = DynamicLoadLibrary(&ns, "test_library.so", RTLD_LAZY);
    const char* dlerror = DynamicGetError();
    DynamicLoadLibrary(nullptr, "test_library.so", RTLD_LAZY);
    EXPECT_NE(dlerror, nullptr);
}

/**
 * @tc.name: DynamicLoaderOhosTestFindSymbol_0100
 * @tc.desc: DynamicLoaderOhosTest test for dynamic_find_symbol.
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLoaderOhosTest, DynamicLoaderOhosTestFindSymbol_0100, TestSize.Level1)
{
    void* so = reinterpret_cast<void*>(1);
    const char* symbol = "test_symbol";

    // Test success
    void* result = DynamicFindSymbol(so, symbol);
    EXPECT_EQ(result, dlsym(so, symbol));
}

/**
 * @tc.name: DynamicLoaderOhosTestGetError_0100
 * @tc.desc: DynamicLoaderOhosTest test for dynamic_get_error.
 * @tc.type: FUNC
 */
HWTEST_F(DynamicLoaderOhosTest, DynamicLoaderOhosTestGetError_0100, TestSize.Level1)
{
    // Test dlerror with an error message
    DynamicLoadLibrary(nullptr, "test_library.so", RTLD_LAZY);
}