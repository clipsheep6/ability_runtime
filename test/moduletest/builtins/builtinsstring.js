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

/*
 * @tc.name:builtinsstring
 * @tc.desc:test builtins
 * @tc.type: FUNC
 * @tc.require: issueI6OVYD
 */
print("builtins string start");
print(String.fromCharCode(65));
print(String.fromCharCode(65, 66, 66, 65));
print(String.fromCharCode(0x2014));
print(String.fromCharCode(0x12014));
class NewString extends String {
}
print(new NewString('') instanceof NewString);
print("builtins string end");
