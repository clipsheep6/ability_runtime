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

#include <gtest/gtest.h>
#include "image_info.h"

using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class ImageInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ImageInfoTest::SetUpTestCase(void)
{}
void ImageInfoTest::TearDownTestCase(void)
{}
void ImageInfoTest::SetUp(void)
{}
void ImageInfoTest::TearDown(void)
{}

/*
 * Feature: Image Info
 * Function: Marshalling and Unmarshalling
 * SubFunction: NA
 * FunctionPoints: Image Marshalling
 * EnvConditions: NA
 * CaseDescription: Verify Marshalling
 */
HWTEST_F(ImageInfoTest, image_info_marshalling_001, TestSize.Level1)
{
    Parcel parcel;
    ImageInfo* parcelable1 = new ImageInfo();
    parcelable1->width = 1;
    parcelable1->height = 2;
    parcelable1->format = 3;
    parcelable1->size = 4;
    parcelable1->shmKey = 5;
    EXPECT_EQ(true, parcelable1->Marshalling(parcel));
    ImageInfo* parcelable2 = parcelable1->Unmarshalling(parcel);
    EXPECT_EQ(parcelable2->width, static_cast<uint32_t>(1));
    EXPECT_EQ(parcelable2->height, static_cast<uint32_t>(2));
    EXPECT_EQ(parcelable2->format, static_cast<uint32_t>(3));
    EXPECT_EQ(parcelable2->size, static_cast<uint32_t>(4));
    EXPECT_EQ(parcelable2->shmKey, 5);
}
}
}