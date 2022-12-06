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

#include <gtest/gtest.h>
#include <memory>
#include <regex>
#define private public
#define protected public
#include "pac_map.h"
#undef private
#undef protected
#include "user_object_base.h"
namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

#define PAC_MPA_TEST_INT 1000
#define PAC_MAP_TEST_LONG (-1000)
#define PAC_MAP_TEST_FLOAT 1.0f
#define PAC_MAP_TEST_DOUBLE 3.1415926
namespace {
const std::regex INTEGER_REGEX("^[-+]?([0-9]+)([.]([0-9]+))?$");
};
class TUserObjectTest : public UserObjectBase {
public:
    TUserObjectTest() : UserObjectBase("TUserObjectTest"), str_data_("用户自定义对象"), int_data_(0)
    {}
    ~TUserObjectTest()
    {}

    std::string ToString() const override
    {
        std::string tostring = str_data_;
        tostring += "#" + std::to_string(int_data_);
        return tostring;
    }

    void Parse(const std::string &str) override
    {
        std::vector<std::string> elems;

        std::size_t splitPos = str.find("#");
        if (splitPos == std::string::npos) {
            return;
        }
        std::string strData = str.substr(0, splitPos);
        std::string intdata = str.substr(strData.length() + 1, str.length() - 1);
        if (strData.length() + 1 + intdata.length() != str.length()) {
            return;
        }
        bool isNumber = std::regex_match(intdata, INTEGER_REGEX);
        if (isNumber) {
            str_data_ = strData;
            int_data_ = std::stoi(intdata);
        }
    }

    bool Equals(std::shared_ptr<UserObjectBase> &other) override
    {
        if (other->GetClassName() != GetClassName()) {
            return false;
        }

        TUserObjectTest *pobject = static_cast<TUserObjectTest *>(other.get());
        if (pobject == nullptr) {
            return false;
        }
        return ((str_data_ == pobject->str_data_) && (int_data_ == pobject->int_data_));
    }

    void DeepCopy(std::shared_ptr<UserObjectBase> &other) override
    {
        if (other->GetClassName() != GetClassName()) {
            return;
        }

        TUserObjectTest *pobject = static_cast<TUserObjectTest *>(other.get());
        if (pobject != nullptr) {
            str_data_ = pobject->str_data_;
            int_data_ = pobject->int_data_;
        }
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }

    bool Unmarshalling(Parcel &parcel) override
    {
        return true;
    }

private:
    std::string str_data_ = "";
    int int_data_ = 0;
};
REGISTER_USER_OBJECT_BASE(TUserObjectTest);

/*
 * Description：Test for data type of base: like int, short, long std::string etc.
 */
class PacMapTest : public testing::Test {
public:
    PacMapTest() : pacmap_(nullptr)
    {}
    ~PacMapTest()
    {}

    std::shared_ptr<PacMap> pacmap_ = nullptr;
    std::shared_ptr<PacMap> pacmap2_ = nullptr;
    static void FillData(PacMap &pacmap);
    static void FillData2(PacMap &pacmap, const PacMap &param_map);

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PacMapTest::SetUpTestCase(void)
{}

void PacMapTest::TearDownTestCase(void)
{}

void PacMapTest::SetUp()
{
    pacmap_ = std::make_shared<PacMap>();
    pacmap2_ = std::make_shared<PacMap>();
}

void PacMapTest::TearDown()
{}

void PacMapTest::FillData(PacMap &pacmap)
{
    std::vector<short> arrayShort;
    std::vector<int> arrayInt;
    std::vector<long> arrayLong;
    std::vector<AAFwk::byte> arrayByte;
    std::vector<bool> arrayBool;
    std::vector<float> arrayFloat;
    std::vector<double> arrayDouble;
    std::vector<std::string> arrayString;

    arrayShort.push_back(PAC_MPA_TEST_INT);
    arrayInt.push_back(PAC_MPA_TEST_INT);
    arrayLong.push_back(PAC_MAP_TEST_LONG);
    arrayByte.push_back('a');
    arrayBool.push_back(true);
    arrayFloat.push_back(PAC_MAP_TEST_FLOAT);
    arrayDouble.push_back(PAC_MAP_TEST_DOUBLE);
    arrayString.push_back("<~!@#$%^&*()_+>特殊字符");

    pacmap.PutShortValue("key_short", PAC_MPA_TEST_INT);
    pacmap.PutIntValue("key_int", PAC_MPA_TEST_INT);
    pacmap.PutLongValue("key_long", PAC_MAP_TEST_LONG);
    pacmap.PutByteValue("key_byte", 'A');
    pacmap.PutBooleanValue("key_boolean", true);
    pacmap.PutFloatValue("key_float", PAC_MAP_TEST_FLOAT);
    pacmap.PutDoubleValue("key_double", PAC_MAP_TEST_DOUBLE);
    pacmap.PutStringValue("key_string", "test clone");

    std::shared_ptr<TUserObjectTest> pubObject = std::make_shared<TUserObjectTest>();
    pacmap.PutObject("key_object", pubObject);

    pacmap.PutShortValueArray("key_short_array", arrayShort);
    pacmap.PutIntValueArray("key_int_array", arrayInt);
    pacmap.PutLongValueArray("key_long_array", arrayLong);
    pacmap.PutByteValueArray("key_byte_array", arrayByte);
    pacmap.PutFloatValueArray("key_float_array", arrayFloat);
    pacmap.PutBooleanValueArray("key_boolean_array", arrayBool);
    pacmap.PutDoubleValueArray("key_double_array", arrayDouble);
    pacmap.PutStringValueArray("key_string_array", arrayString);
}

void PacMapTest::FillData2(PacMap &pacmap, const PacMap &param_map)
{
    std::vector<short> arrayShort;
    std::vector<int> arrayInt;
    std::vector<long> arrayLong;
    std::vector<AAFwk::byte> arrayByte;
    std::vector<bool> arrayBool;
    std::vector<float> arrayFloat;
    std::vector<double> arrayDouble;
    std::vector<std::string> arrayString;

    arrayShort.push_back(PAC_MPA_TEST_INT);
    arrayInt.push_back(PAC_MPA_TEST_INT);
    arrayLong.push_back(PAC_MAP_TEST_LONG);
    arrayByte.push_back('a');
    arrayBool.push_back(true);
    arrayFloat.push_back(PAC_MAP_TEST_FLOAT);
    arrayDouble.push_back(PAC_MAP_TEST_DOUBLE);
    arrayString.push_back("<~!@#$%^&*()_+>特殊字符");

    pacmap.PutShortValue("key_short", PAC_MPA_TEST_INT);
    pacmap.PutIntValue("key_int", PAC_MPA_TEST_INT);
    pacmap.PutLongValue("key_long", PAC_MAP_TEST_LONG);
    pacmap.PutByteValue("key_byte", 'A');
    pacmap.PutBooleanValue("key_boolean", true);
    pacmap.PutFloatValue("key_float", PAC_MAP_TEST_FLOAT);
    pacmap.PutDoubleValue("key_double", PAC_MAP_TEST_DOUBLE);
    pacmap.PutStringValue("key_string", "test clone");

    pacmap.PutPacMap("key_map", param_map);

    pacmap.PutShortValueArray("key_short_array", arrayShort);
    pacmap.PutIntValueArray("key_int_array", arrayInt);
    pacmap.PutLongValueArray("key_long_array", arrayLong);
    pacmap.PutByteValueArray("key_byte_array", arrayByte);
    pacmap.PutFloatValueArray("key_float_array", arrayFloat);
    pacmap.PutBooleanValueArray("key_boolean_array", arrayBool);
    pacmap.PutDoubleValueArray("key_double_array", arrayDouble);
    pacmap.PutStringValueArray("key_string_array", arrayString);
}
/**
 * @tc.number: AppExecFwk_PacMap_PutShortValue_0100
 * @tc.name: PutShortValue
 * @tc.desc: Verify PutShortValue() and GetShortValue().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutShortValue_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutShortValue_0100 start";
    short value = 1000;
    pacmap_->PutShortValue("key_short", value);
    EXPECT_EQ(value, pacmap_->GetShortValue("key_short"));
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutShortValue_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutIntValue_0100
 * @tc.name: PutIntValue and GetIntValue
 * @tc.desc: Verify PutIntValue() and GetIntValue().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutIntValue_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutIntValue_0100 start";
    int value = 1000;
    pacmap_->PutIntValue("key_int", value);
    EXPECT_EQ(value, pacmap_->GetIntValue("key_int"));
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutIntValue_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutLongValue_0100
 * @tc.name: PutLongValue and GetLongValue
 * @tc.desc: Verify PutLongValue() and GetLongValue().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutLongValue_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutLongValue_0100 start";
    long value = -1000;
    pacmap_->PutLongValue("key_long", value);
    EXPECT_EQ(value, pacmap_->GetLongValue("key_long"));
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutLongValue_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutByteValue_0100
 * @tc.name: PutByteValue and GetByteValue
 * @tc.desc: Verify PutByteValue() and GetByteValue().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutByteValue_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutByteValue_0100 start";
    AAFwk::byte value = 'A';
    pacmap_->PutByteValue("key_byte", value);
    EXPECT_EQ(value, pacmap_->GetByteValue("key_byte"));
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutByteValue_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutBooleanValue_0100
 * @tc.name: PutBooleanValue and GetBooleanValue
 * @tc.desc: Verify PutBooleanValue() and GetBooleanValue().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutBooleanValue_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutBooleanValue_0100 start";
    bool value = true;
    pacmap_->PutBooleanValue("key_boolean_true", value);
    EXPECT_EQ(value, pacmap_->GetBooleanValue("key_boolean_true"));

    value = false;
    pacmap_->PutBooleanValue("key_boolean_false", value);
    EXPECT_EQ(value, pacmap_->GetBooleanValue("key_boolean_false"));

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutBooleanValue_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutFloatValue_0100
 * @tc.name: PutFloatValue and GetFloatValue
 * @tc.desc: Verify PutFloatValue() and GetFloatValue().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutFloatValue_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutFloatValue_0100 start";
    float value = 3.14f;
    pacmap_->PutFloatValue("key_float", value);
    EXPECT_EQ(value, pacmap_->GetFloatValue("key_float"));
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutFloatValue_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutDoubleValue_0100
 * @tc.name: PutDoubleValue and GetDoubleValue
 * @tc.desc: Verify PutDoubleValue() and GetDoubleValue().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutDoubleValue_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutDoubleValue_0100 start";
    double value = 3.1415926;
    pacmap_->PutDoubleValue("key_double", value);
    EXPECT_EQ(value, pacmap_->GetDoubleValue("key_double"));
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutDoubleValue_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutStringValue_0100
 * @tc.name: PutStringValue and GetStringValue
 * @tc.desc: Verify PutStringValue() and GetStringValue().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutStringValue_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutStringValue_0100 start";
    std::string value("AppExecFwk_PacMap_PutStringValue_0100  PACMAP测试");
    pacmap_->PutStringValue("key_string", value);
    std::string getStr = pacmap_->GetStringValue("key_string");
    EXPECT_STREQ(value.c_str(), getStr.c_str());
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutStringValue_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutShortValueArray_0100
 * @tc.name: PutShortValueArray and GetShortValueArray
 * @tc.desc: Verify PutShortValueArray() and GetShortValueArray().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutShortValueArray_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutShortValueArray_0100 start";

    std::vector<short> putValue;
    std::vector<short> getValue;
    for (int i = 0; i < 100; i++) {
        putValue.emplace_back(i + 1);
    }
    pacmap_->PutShortValueArray("key_short_array", putValue);
    pacmap_->GetShortValueArray("key_short_array", getValue);

    bool isEqual = (putValue == getValue);
    EXPECT_EQ(true, isEqual);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutShortValueArray_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutIntValueArray_0100
 * @tc.name: PutIntValueArray and GetIntValueArray
 * @tc.desc: Verify PutIntValueArray() and GetIntValueArray().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutIntValueArray_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutIntValueArray_0100 start";

    std::vector<int> putValue;
    std::vector<int> getValue;
    for (int i = 0; i < 100; i++) {
        putValue.emplace_back(i + 1);
    }
    pacmap_->PutIntValueArray("key_int_array", putValue);
    pacmap_->GetIntValueArray("key_int_array", getValue);

    bool isEqual = (putValue == getValue);
    EXPECT_EQ(true, isEqual);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutIntValueArray_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutLongArray_0100
 * @tc.name: PutLongValueArray and GetLongValueArray
 * @tc.desc: Verify PutLongValueArray() and GetLongValueArray().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutLongArray_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutLongArray_0100 start";

    std::vector<long> putValue;
    std::vector<long> getValue;
    for (int i = 0; i < 100; i++) {
        putValue.emplace_back(i + 1);
    }
    pacmap_->PutLongValueArray("key_long_array", putValue);
    pacmap_->GetLongValueArray("key_long_array", getValue);

    bool isEqual = (putValue == getValue);
    EXPECT_EQ(true, isEqual);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutLongArray_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutByteArray_0100
 * @tc.name: PutByteValueArray and GetByteValueArray
 * @tc.desc: Verify PutByteValueArray() and GetByteValueArray().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutByteArray_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutByteArray_0100 start";

    std::vector<AAFwk::byte> putValue;
    std::vector<AAFwk::byte> getValue;
    for (int i = 0; i < 26; i++) {
        putValue.emplace_back('A' + i);
    }
    pacmap_->PutByteValueArray("key_byte_array", putValue);
    pacmap_->GetByteValueArray("key_byte_array", getValue);

    bool isEqual = (putValue == getValue);
    EXPECT_EQ(true, isEqual);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutByteArray_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutFloatArray_0100
 * @tc.name: PutLongValueArray and GetLongValueArray
 * @tc.desc: Verify PutLongValueArray() and GetLongValueArray().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutFloatArray_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutFloatArray_0100 start";

    std::vector<float> putValue;
    std::vector<float> getValue;
    for (int i = 0; i < 100; i++) {
        putValue.emplace_back((i + 1) * 1.0f);
    }
    pacmap_->PutFloatValueArray("key_long_array", putValue);
    pacmap_->GetFloatValueArray("key_long_array", getValue);

    bool isEqual = (putValue == getValue);
    EXPECT_EQ(true, isEqual);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutFloatArray_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutDoubleArray_0100
 * @tc.name: PutDoubleValueArray and GetDoubleValueArray
 * @tc.desc: Verify PutDoubleValueArray() and GetDoubleValueArray().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutDoubleArray_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutDoubleArray_0100 start";

    std::vector<double> putValue;
    std::vector<double> getValue;
    for (int i = 0; i < 100; i++) {
        putValue.emplace_back((i + 1) * 1.0);
    }
    pacmap_->PutDoubleValueArray("key_double_array", putValue);
    pacmap_->GetDoubleValueArray("key_double_array", getValue);

    bool isEqual = (putValue == getValue);
    EXPECT_EQ(true, isEqual);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutDoubleArray_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutStringArray_0100
 * @tc.name: PutStringValueArray and GetStringValueArray
 * @tc.desc: Verify PutStringValueArray() and GetStringValueArray().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutStringArray_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutStringArray_0100 start";

    std::vector<std::string> tempValue;
    std::vector<std::string> putValue;
    std::vector<std::string> getValue;

    tempValue.emplace_back("Adds a String value matching a specified key.");
    tempValue.emplace_back("添加字符串");
    tempValue.emplace_back("<~!@#$%^&*()_+>特殊字符");

    for (int i = 0; i < 100; i++) {
        putValue.emplace_back(tempValue[i % 3]);
    }
    pacmap_->PutStringValueArray("key_string_array", putValue);
    pacmap_->GetStringValueArray("key_string_array", getValue);

    bool isEqual = (putValue == getValue);
    EXPECT_EQ(true, isEqual);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutStringArray_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutObject_0100
 * @tc.name: PutObject and GetObject
 * @tc.desc: Verify PutObject() and GetObject().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutObject_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutObject_0100 start";

    std::shared_ptr<TUserObjectTest> putObject = std::make_shared<TUserObjectTest>();
    pacmap_->PutObject("key_object", putObject);

    std::shared_ptr<UserObjectBase> getObject = pacmap_->GetObject("key_object");
    bool isEqual = false;
    if (getObject.get() != nullptr) {
        isEqual = getObject->Equals(getObject);
    }
    EXPECT_EQ(true, isEqual);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutObject_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_Clone_0100
 * @tc.name: Clone and Equals
 * @tc.desc: Verify Clone() and Equals().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_Clone_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_Clone_0100 start";

    PacMap otherMap;
    FillData(*pacmap_.get());
    otherMap = pacmap_->Clone();
    EXPECT_EQ(true, pacmap_->Equals(otherMap));

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_Clone_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_DeepCopy_0100
 * @tc.name: DeepCopy
 * @tc.desc: Verify DeepCopy().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_DeepCopy_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_DeepCopy_0100 start";

    PacMap otherMap;
    FillData(*pacmap_.get());
    otherMap = pacmap_->DeepCopy();
    EXPECT_EQ(true, pacmap_->Equals(otherMap));

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_DeepCopy_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_Clear_0100
 * @tc.name: Clear and GetSize
 * @tc.desc: Verify Clear() and GetSize().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_Clear_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_Clear_0100 start";

    FillData(*pacmap_.get());
    pacmap_->Clear();
    EXPECT_EQ(0, pacmap_->GetSize());

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_Clear_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutAll_0100
 * @tc.name: PutAll
 * @tc.desc: Verify PutAll().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutAll_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutAll_0100 start";

    PacMap otherMap;
    FillData(otherMap);
    pacmap_->PutAll(otherMap);
    EXPECT_EQ(true, pacmap_->Equals(otherMap));

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutAll_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_GetAll_0100
 * @tc.name: GetAll
 * @tc.desc: Verify GetAll().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_GetAll_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetAll_0100 start";

    FillData(*pacmap_.get());
    std::map<std::string, PacMapObject::INTERFACE> data = pacmap_->GetAll();

    EXPECT_EQ(data.size(), (std::size_t)pacmap_->GetSize());

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetAll_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_HasKey_0100
 * @tc.name: HasKey
 * @tc.desc: Verify HasKey().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_HasKey_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_HasKey_0100 start";

    FillData(*pacmap_.get());
    EXPECT_EQ(true, pacmap_->HasKey("key_short_array"));

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_HasKey_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_GetKeys_0100
 * @tc.name: GetKeys
 * @tc.desc: Verify GetKeys().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_GetKeys_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetKeys_0100 start";

    FillData(*pacmap_.get());
    const std::set<std::string> keys = pacmap_->GetKeys();
    EXPECT_EQ((int)keys.size(), pacmap_->GetSize());

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetKeys_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_Remove_0100
 * @tc.name: Remove
 * @tc.desc: Verify Remove().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_Remove_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_Remove_0100 start";

    FillData(*pacmap_.get());
    EXPECT_EQ(true, pacmap_->HasKey("key_short_array"));
    pacmap_->Remove("key_short_array");
    EXPECT_EQ(false, pacmap_->HasKey("key_short_array"));

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_Remove_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_IsEmpty_0100
 * @tc.name: IsEmpty
 * @tc.desc: Verify IsEmpty().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_IsEmpty_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_IsEmpty_0100 start";

    FillData(*pacmap_.get());
    EXPECT_EQ(false, pacmap_->IsEmpty());
    pacmap_->Clear();
    EXPECT_EQ(true, pacmap_->IsEmpty());

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_IsEmpty_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_Marshalling_0100
 * @tc.name: Marshalling and Unmarshalling
 * @tc.desc: Verify Marshalling() and Unmarshalling().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_Marshalling_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_Marshalling_0100 start";

    Parcel parcel;
    FillData(*pacmap_.get());
    EXPECT_EQ(true, pacmap_->Marshalling(parcel));

    PacMap *unmarshingMap = PacMap::Unmarshalling(parcel);
    EXPECT_EQ(true, unmarshingMap != nullptr);
    if (unmarshingMap != nullptr) {
        EXPECT_EQ(true, pacmap_->Equals(unmarshingMap));
        delete unmarshingMap;
        unmarshingMap = nullptr;
    }
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_Marshalling_0100 end";
}
/**
 * @tc.number: AppExecFwk_PacMap_Marshalling_0200
 * @tc.name: Marshalling and Unmarshalling
 * @tc.desc: Verify Marshalling() and Unmarshalling().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_Marshalling_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_Marshalling_0200 start";

    Parcel parcel;
    FillData(*pacmap_.get());
    FillData2(*pacmap2_.get(), *pacmap_.get());

    EXPECT_EQ(true, pacmap2_->Marshalling(parcel));
    PacMap *unmarshingMap = PacMap::Unmarshalling(parcel);

    EXPECT_EQ(true, unmarshingMap != nullptr);
    if (unmarshingMap != nullptr) {
        EXPECT_EQ(true, pacmap2_->Equals(unmarshingMap));
        delete unmarshingMap;
        unmarshingMap = nullptr;
    }
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_Marshalling_0200 end";
}
/**
 * @tc.number: AppExecFwk_PacMap_Marshalling_0300
 * @tc.name: Marshalling and Unmarshalling
 * @tc.desc: Verify Marshalling() and Unmarshalling().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_Marshalling_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_Marshalling_0300 start";

    Parcel parcel;
    EXPECT_EQ(true, pacmap2_->Marshalling(parcel));
    PacMap *unmarshingMap = PacMap::Unmarshalling(parcel);

    EXPECT_EQ(true, unmarshingMap != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_Marshalling_0300 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutCharValue_0100
 * @tc.name: PutCharValue and GetCharValue
 * @tc.desc: Verify PutCharValue() and GetCharValue().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutCharValue_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutCharValue_0100 start";
    char value = 'a';
    pacmap_->PutCharValue("key_char", value);
    EXPECT_EQ(value, pacmap_->GetCharValue("key_char"));
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutCharValue_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutCharValueArray_0100
 * @tc.name: PutCharValueArray and GetCharValueArray
 * @tc.desc: Verify PutCharValueArray() and GetCharValueArray().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutCharValueArray_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutCharValueArray_0100 start";

    std::vector<char> putValue;
    std::vector<char> getValue;
    for (int i = 0; i < 26; i++) {
        putValue.emplace_back('a' + i);
    }
    pacmap_->PutCharValueArray("key_char_array", putValue);
    pacmap_->GetCharValueArray("key_char_array", getValue);

    bool isEqual = (putValue == getValue);
    EXPECT_EQ(true, isEqual);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutCharValueArray_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_FromString_0100
 * @tc.name: Marshalling and Unmarshalling
 * @tc.desc: Verify Marshalling() and Unmarshalling().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_FromString_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_FromString_0100 start";
    std::string str;
    auto result = pacmap_->FromString(str);
    EXPECT_TRUE(!result);
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_FromString_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_FromString_0200
 * @tc.name: Marshalling and Unmarshalling
 * @tc.desc: Verify Marshalling() and Unmarshalling().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_FromString_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_FromString_0200 start";
    std::string str = "abc";
    auto result = pacmap_->FromString(str);
    EXPECT_TRUE(!result);
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_FromString_0200 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_Parse_0100
 * @tc.name: Marshalling and Unmarshalling
 * @tc.desc: Verify Marshalling() and Unmarshalling().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_Parse_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_Parse_0100 start";
    std::string str = "abc";
    auto result = pacmap_->Parse(str);
    EXPECT_TRUE(result != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_Parse_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_GetCharValueArray_0100
 * @tc.name: PutCharValueArray and GetCharValueArray
 * @tc.desc: Verify PutCharValueArray() and GetCharValueArray().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_GetCharValueArray_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetCharValueArray_0100 start";

    std::vector<char> putValue;
    std::vector<char> getValue;
    for (int i = 0; i < 26; i++) {
        putValue.emplace_back('a' + i);
    }
    pacmap_->PutCharValueArray("key_char_array", putValue);
    pacmap_->GetCharValueArray("key_char_array", getValue);

    bool isEqual = (putValue == getValue);
    EXPECT_EQ(true, isEqual);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetCharValueArray_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_GetBooleanValueArray_0100
 * @tc.name: PutBooleanValue and GetBooleanValue
 * @tc.desc: Verify PutBooleanValue() and GetBooleanValue().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_GetBooleanValueArray_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetBooleanValueArray_0100 start";
    std::vector<bool> putValue;
    std::vector<bool> getValue;
    putValue.emplace_back(true);
    pacmap_->PutBooleanValueArray("key_boolean_true", putValue);
    pacmap_->GetBooleanValueArray("key_boolean_true", getValue);
    bool isEqual = (putValue == getValue);
    EXPECT_EQ(true, isEqual);
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetBooleanValueArray_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_GetCharValue_0100
 * @tc.name: PutCharValue and GetCharValue
 * @tc.desc: Verify PutCharValue() and GetCharValue().
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_GetCharValue_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetCharValue_0100 start";
    char value = 'a';
    pacmap_->PutCharValue("key_char", value);
    EXPECT_EQ(value, pacmap_->GetCharValue("key_char"));
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetCharValue_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_DeepCopy_0200
 * @tc.name: DeepCopy
 * @tc.desc: Verify DeepCopy.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_DeepCopy_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_DeepCopy_0200 start";
    PacMap otherMap;
    FillData(otherMap);
    pacmap_->DeepCopy(otherMap);
    EXPECT_EQ(true, pacmap_->Equals(otherMap));
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_DeepCopy_0200 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_InnerPutObject_0100
 * @tc.name: InnerPutObject
 * @tc.desc: Verify InnerPutObject.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_InnerPutObject_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_InnerPutObject_0100 start";

    std::shared_ptr<UserObjectBase> value = nullptr;
    PacMapList mapList;
    std::string key = "this is key";
    pacmap_->InnerPutObject(mapList, key, value);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_InnerPutObject_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_PutAll_0200
 * @tc.name: PutAll
 * @tc.desc: Verify PutAll().
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_PutAll_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutAll_0200 start";
    FillData(*pacmap_.get());
    std::map<std::string, PacMapObject::INTERFACE> data = pacmap_->GetAll();
    pacmap_->PutAll(data);
    std::string key = "this is key";
    int defaultValue = 10;
    int result = pacmap_->GetIntValue(key, defaultValue);
    EXPECT_EQ(result, defaultValue);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_PutAll_0200 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_GetShortValue_0200
 * @tc.name: GetShortValue
 * @tc.desc: Verify GetShortValue.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_GetShortValue_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetShortValue_0200 start";
    FillData(*pacmap_.get());
    std::string key = "this is key";
    short defaultValue = 10;
    short result = pacmap_->GetShortValue(key, defaultValue);
    EXPECT_EQ(result, defaultValue);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetShortValue_0200 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_GetBooleanValue_0200
 * @tc.name: GetBooleanValue
 * @tc.desc: Verify GetBooleanValue.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_GetBooleanValue_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetBooleanValue_0200 start";
    FillData(*pacmap_.get());
    std::string key = "this is key";
    bool defaultValue = true;
    bool result = pacmap_->GetBooleanValue(key, defaultValue);
    EXPECT_EQ(result, defaultValue);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetBooleanValue_0200 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_GetLongValue_0200
 * @tc.name: GetLongValue
 * @tc.desc: Verify GetLongValue.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_GetLongValue_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetLongValue_0200 start";
    FillData(*pacmap_.get());
    std::string key = "this is key";
    long defaultValue = 100;
    long result = pacmap_->GetLongValue(key, defaultValue);
    EXPECT_EQ(result, defaultValue);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetLongValue_0200 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_GetCharValue_0200
 * @tc.name: GetCharValue
 * @tc.desc: Verify GetCharValue.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_GetCharValue_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetCharValue_0200 start";
    FillData(*pacmap_.get());
    std::string key = "this is key";
    char defaultValue = 'a';
    char result = pacmap_->GetCharValue(key, defaultValue);
    EXPECT_EQ(result, defaultValue);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetCharValue_0200 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_GetByteValue_0200
 * @tc.name: GetByteValue
 * @tc.desc: Verify GetByteValue.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_GetByteValue_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetByteValue_0200 start";
    FillData(*pacmap_.get());
    std::string key = "this is key";
    AAFwk::byte defaultValue = 'A';
    AAFwk::byte result = pacmap_->GetByteValue(key, defaultValue);
    EXPECT_EQ(result, defaultValue);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetByteValue_0200 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_GetFloatValue_0200
 * @tc.name: GetFloatValue
 * @tc.desc: Verify GetFloatValue.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_GetFloatValue_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetFloatValue_0200 start";
    FillData(*pacmap_.get());
    std::string key = "this is key";
    float defaultValue = 3.1;
    float result = pacmap_->GetFloatValue(key, defaultValue);
    EXPECT_EQ(result, defaultValue);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetFloatValue_0200 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_GetDoubleValue_0200
 * @tc.name: GetDoubleValue
 * @tc.desc: Verify GetDoubleValue.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_GetDoubleValue_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetDoubleValue_0200 start";
    FillData(*pacmap_.get());
    std::string key = "this is key";
    double defaultValue = 3.11111;
    double result = pacmap_->GetDoubleValue(key, defaultValue);
    EXPECT_EQ(result, defaultValue);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetDoubleValue_0200 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_GetStringValue_0200
 * @tc.name: GetStringValue
 * @tc.desc: Verify GetDoubleValue.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_GetStringValue_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetStringValue_0200 start";
    FillData(*pacmap_.get());
    std::string key = "this is key";
    std::string defaultValue = "this is defaultValue";
    std::string result = pacmap_->GetStringValue(key, defaultValue);
    EXPECT_EQ(result, defaultValue);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetStringValue_0200 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_GetObject_0100
 * @tc.name: GetObject
 * @tc.desc: Verify GetObject().
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_GetObject_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetObject_0100 start";

    std::string key = "";
    std::shared_ptr<UserObjectBase> getObject = pacmap_->GetObject(key);
    EXPECT_EQ(getObject, nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetObject_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_GetPacMap_0100
 * @tc.name: GetPacMap
 * @tc.desc: Verify GetPacMap.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_GetPacMap_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetPacMap_0100 start";

    std::string key = "this is key";
    pacmap_->GetPacMap(key);
    std::string key1 = "";
    pacmap_->GetPacMap(key1);
    PacMapList desPacMap;
    PacMapList srcPacMap;
    pacmap_->ShallowCopyData(desPacMap, srcPacMap);
    PacMapList pacMapList;
    pacmap_->RemoveData(pacMapList, key1);

    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_GetPacMap_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_EqualPacMapData_0100
 * @tc.name: EqualPacMapData
 * @tc.desc: Verify EqualPacMapData.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_EqualPacMapData_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_EqualPacMapData_0100 start";

    FillData(*pacmap_.get());
    std::map<std::string, PacMapObject::INTERFACE> data = pacmap_->GetAll();
    PacMapList rightPacMapList;
    bool result = pacmap_->EqualPacMapData(data, rightPacMapList);
    EXPECT_EQ(result, false);
    
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_EqualPacMapData_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_CompareArrayData_0100
 * @tc.name: CompareArrayData
 * @tc.desc: Verify CompareArrayData.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_CompareArrayData_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_CompareArrayData_0100 start";

    FillData(*pacmap_.get());
    AAFwk::IInterface *one_interface = nullptr;
    AAFwk::IInterface *two_interface = nullptr;
    bool result = pacmap_->CompareArrayData(one_interface, two_interface);
    EXPECT_EQ(result, true);
    
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_CompareArrayData_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_Equals_0200
 * @tc.name: Equals
 * @tc.desc: Verify Equals.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_Equals_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_Equals_0200 start";

    bool result = pacmap_->Equals(nullptr);
    EXPECT_EQ(result, false);
    
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_Equals_0200 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_ToString_0100
 * @tc.name: ToString
 * @tc.desc: Verify ToString.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_ToString_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToString_0100 start";

    std::string result = pacmap_->ToString();
    std::string ret = "{\"pacmap\":null}";
    EXPECT_EQ(result, ret);
    
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToString_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_ToJsonArrayShort_0100
 * @tc.name: ToJsonArrayShort
 * @tc.desc: Verify ToJsonArrayShort.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_ToJsonArrayShort_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToJsonArrayShort_0100 start";

    std::vector<short> array;
    Json::Value item;
    int type = 1;
    bool result = pacmap_->ToJsonArrayShort(array, item, type);
    EXPECT_EQ(result, false);
    
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToJsonArrayShort_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_ToJsonArrayInt_0100
 * @tc.name: ToJsonArrayInt
 * @tc.desc: Verify ToJsonArrayInt.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_ToJsonArrayInt_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToJsonArrayInt_0100 start";

    std::vector<int> array;
    Json::Value item;
    int type = 1;
    bool result = pacmap_->ToJsonArrayInt(array, item, type);
    EXPECT_EQ(result, false);
    
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToJsonArrayInt_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_ToJsonArrayLong_0100
 * @tc.name: ToJsonArrayLong
 * @tc.desc: Verify ToJsonArrayLong.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_ToJsonArrayLong_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToJsonArrayLong_0100 start";

    std::vector<long> array;
    Json::Value item;
    int type = 1;
    bool result = pacmap_->ToJsonArrayLong(array, item, type);
    EXPECT_EQ(result, false);
    
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToJsonArrayLong_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_ToJsonArrayByte_0100
 * @tc.name: ToJsonArrayByte
 * @tc.desc: Verify ToJsonArrayByte.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_ToJsonArrayByte_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToJsonArrayByte_0100 start";

    std::vector<byte> array;
    Json::Value item;
    int type = 1;
    bool result = pacmap_->ToJsonArrayByte(array, item, type);
    EXPECT_EQ(result, false);
    
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToJsonArrayByte_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_ToJsonArrayBoolean_0100
 * @tc.name: ToJsonArrayBoolean
 * @tc.desc: Verify ToJsonArrayBoolean.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_ToJsonArrayBoolean_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToJsonArrayBoolean_0100 start";

    std::vector<bool> array;
    Json::Value item;
    int type = 1;
    bool result = pacmap_->ToJsonArrayBoolean(array, item, type);
    EXPECT_EQ(result, false);
    
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToJsonArrayBoolean_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_ToJsonArrayFloat_0100
 * @tc.name: ToJsonArrayFloat
 * @tc.desc: Verify ToJsonArrayBoolean.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_ToJsonArrayFloat_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToJsonArrayFloat_0100 start";

    std::vector<float> array;
    Json::Value item;
    int type = 1;
    bool result = pacmap_->ToJsonArrayFloat(array, item, type);
    EXPECT_EQ(result, false);
    
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToJsonArrayFloat_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_ToJsonArrayDouble_0100
 * @tc.name: ToJsonArrayDouble
 * @tc.desc: Verify ToJsonArrayDouble.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_ToJsonArrayDouble_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToJsonArrayDouble_0100 start";

    std::vector<double> array;
    Json::Value item;
    int type = 1;
    bool result = pacmap_->ToJsonArrayDouble(array, item, type);
    EXPECT_EQ(result, false);
    
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToJsonArrayDouble_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_ToJsonArrayString_0100
 * @tc.name: ToJsonArrayString
 * @tc.desc: Verify ToJsonArrayString.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_ToJsonArrayString_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToJsonArrayString_0100 start";

    std::vector<std::string> array;
    Json::Value item;
    int type = 1;
    bool result = pacmap_->ToJsonArrayString(array, item, type);
    EXPECT_EQ(result, false);
    
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ToJsonArrayString_0100 end";
}

/**
 * @tc.number: AppExecFwk_PacMap_ParseJson_0100
 * @tc.name: ParseJson
 * @tc.desc: Verify ParseJson.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_PacMap_ParseJson_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ParseJson_0100 start";

    Json::Value data;
    PacMapList mapList;
    bool result = pacmap_->ParseJson(data, mapList);
    EXPECT_EQ(result, false);
    
    GTEST_LOG_(INFO) << "AppExecFwk_PacMap_ParseJson_0100 end";
}

/**
 * @tc.number: AppExecFwk_ParseJsonItemArrayShort_0100
 * @tc.name: ParseJsonItemArrayShort
 * @tc.desc: Verify ParseJsonItemArrayShort.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_ParseJsonItemArrayShort_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayShort_0100 start";

    PacMapList mapList;
    std::string key = "this is key";
    Json::Value item;
    bool result = pacmap_->ParseJsonItemArrayShort(mapList, key, item);
    EXPECT_EQ(result, true);
    
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayShort_0100 end";
}

/**
 * @tc.number: AppExecFwk_ParseJsonItemArrayInteger_0100
 * @tc.name: ParseJsonItemArrayInteger
 * @tc.desc: Verify ParseJsonItemArrayInteger.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_ParseJsonItemArrayInteger_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayInteger_0100 start";

    PacMapList mapList;
    std::string key = "this is key";
    Json::Value item;
    bool result = pacmap_->ParseJsonItemArrayInteger(mapList, key, item);
    EXPECT_EQ(result, true);
    
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayInteger_0100 end";
}

/**
 * @tc.number: AppExecFwk_ParseJsonItemArrayLong_0100
 * @tc.name: ParseJsonItemArrayLong
 * @tc.desc: Verify ParseJsonItemArrayLong.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_ParseJsonItemArrayLong_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayLong_0100 start";

    PacMapList mapList;
    std::string key = "this is key";
    Json::Value item;
    bool result = pacmap_->ParseJsonItemArrayLong(mapList, key, item);
    EXPECT_EQ(result, true);
    
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayLong_0100 end";
}

/**
 * @tc.number: AppExecFwk_ParseJsonItemArrayChar_0100
 * @tc.name: ParseJsonItemArrayChar
 * @tc.desc: Verify ParseJsonItemArrayChar.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_ParseJsonItemArrayChar_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayChar_0100 start";

    PacMapList mapList;
    std::string key = "this is key";
    Json::Value item;
    bool result = pacmap_->ParseJsonItemArrayChar(mapList, key, item);
    EXPECT_EQ(result, true);
    
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayChar_0100 end";
}

/**
 * @tc.number: AppExecFwk_ParseJsonItemArrayByte_0100
 * @tc.name: ParseJsonItemArrayByte
 * @tc.desc: Verify ParseJsonItemArrayByte.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_ParseJsonItemArrayByte_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayByte_0100 start";

    PacMapList mapList;
    std::string key = "this is key";
    Json::Value item;
    bool result = pacmap_->ParseJsonItemArrayByte(mapList, key, item);
    EXPECT_EQ(result, true);
    
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayByte_0100 end";
}

/**
 * @tc.number: AppExecFwk_ParseJsonItemArrayBoolean_0100
 * @tc.name: ParseJsonItemArrayBoolean
 * @tc.desc: Verify ParseJsonItemArrayBoolean.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_ParseJsonItemArrayBoolean_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayBoolean_0100 start";

    PacMapList mapList;
    std::string key = "this is key";
    Json::Value item;
    bool result = pacmap_->ParseJsonItemArrayBoolean(mapList, key, item);
    EXPECT_EQ(result, true);
    
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayBoolean_0100 end";
}

/**
 * @tc.number: AppExecFwk_ParseJsonItemArrayFloat_0100
 * @tc.name: ParseJsonItemArrayFloat
 * @tc.desc: Verify ParseJsonItemArrayFloat.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_ParseJsonItemArrayFloat_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayFloat_0100 start";

    PacMapList mapList;
    std::string key = "this is key";
    Json::Value item;
    bool result = pacmap_->ParseJsonItemArrayFloat(mapList, key, item);
    EXPECT_EQ(result, true);
    
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayFloat_0100 end";
}

/**
 * @tc.number: AppExecFwk_ParseJsonItemArrayDouble_0100
 * @tc.name: ParseJsonItemArrayDouble
 * @tc.desc: Verify ParseJsonItemArrayDouble.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_ParseJsonItemArrayDouble_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayDouble_0100 start";

    PacMapList mapList;
    std::string key = "this is key";
    Json::Value item;
    bool result = pacmap_->ParseJsonItemArrayDouble(mapList, key, item);
    EXPECT_EQ(result, true);
    
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayDouble_0100 end";
}

/**
 * @tc.number: AppExecFwk_ParseJsonItemArrayString_0100
 * @tc.name: ParseJsonItemArrayString
 * @tc.desc: Verify ParseJsonItemArrayString.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_ParseJsonItemArrayString_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayString_0100 start";

    PacMapList mapList;
    std::string key = "this is key";
    Json::Value item;
    bool result = pacmap_->ParseJsonItemArrayString(mapList, key, item);
    EXPECT_EQ(result, true);
    
    GTEST_LOG_(INFO) << "AppExecFwk_ParseJsonItemArrayString_0100 end";
}

/**
 * @tc.number: AppExecFwk_InnerPutObjectValue_0100
 * @tc.name: InnerPutObjectValue
 * @tc.desc: Verify InnerPutObjectValue.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_InnerPutObjectValue_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_InnerPutObjectValue_0100 start";

    PacMapList mapList;
    std::string key = "this is key";
    Json::Value item;
    bool result = pacmap_->InnerPutObjectValue(mapList, key, item);
    EXPECT_EQ(result, false);
    
    GTEST_LOG_(INFO) << "AppExecFwk_InnerPutObjectValue_0100 end";
}

/**
 * @tc.number: AppExecFwk_InnerPutPacMapValue_0100
 * @tc.name: InnerPutPacMapValue
 * @tc.desc: Verify InnerPutPacMapValue.
 * @tc.require: issueI64N5S
 */
HWTEST_F(PacMapTest, AppExecFwk_InnerPutPacMapValue_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_InnerPutPacMapValue_0100 start";

    PacMapList mapList;
    std::string key = "this is key";
    Json::Value item;
    bool result = pacmap_->InnerPutPacMapValue(mapList, key, item);
    EXPECT_EQ(result, false);
    
    GTEST_LOG_(INFO) << "AppExecFwk_InnerPutPacMapValue_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
