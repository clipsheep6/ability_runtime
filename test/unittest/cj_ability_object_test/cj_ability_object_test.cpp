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

#include "ability.h"
#include "cj_ability_object.h"
#include "cj_runtime.h"
#include "configuration.h"
#include "window_stage_impl.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;

class CjAbilityObjectTest : public testing::Test {
};

void ProxyCall()
{
    CJAbilityObject::LoadModule("0");
    CJAbilityObject::LoadModule("1");
    Want want;
    auto proxy = CJAbilityObject(0);
    proxy.Init(nullptr);
    proxy.OnStart(want, AAFwk::LaunchParam());
    auto win = std::make_shared<Rosen::WindowScene>();
    auto winStage = new Rosen::CJWindowStageImpl(win);
    proxy.OnSceneCreated(winStage);
    proxy.OnSceneRestored(winStage);
    proxy.OnForeground(want);
    proxy.OnBackground();
    auto config = std::make_shared<AppExecFwk::Configuration>();
    proxy.OnConfigurationUpdated(config);
    proxy.OnNewWant(want, AAFwk::LaunchParam());
    std::vector<std::string> params = {"123"};
    std::vector<std::string> infos = {"123"};
    AAFwk::WantParams wantParams = AAFwk::WantParams();
    proxy.OnContinue(wantParams);
    proxy.Dump(params, infos);
    proxy.OnSceneDestroyed();
    proxy.OnStop();
}

HWTEST_F(CjAbilityObjectTest, CJAbilityObject001, TestSize.Level1)
{
    ProxyCall();
}

HWTEST_F(CjAbilityObjectTest, CJAbilityObject002, TestSize.Level1)
{
    auto registerFunc = [](CJAbilityFuncs* funcs) {
        funcs->cjAbilityCreate = [](const char* name) -> int64_t { return name[0] == '0' ? 0 : 1; };
        funcs->cjAbilityRelease = [](int64_t id) {};
        funcs->cjAbilityOnStart = [](int64_t id, WantHandle want, CJLaunchParam launchParam) {};
        funcs->cjAbilityOnStop = [](int64_t id) {};
        funcs->cjAbilityOnSceneCreated = [](int64_t id, WindowStagePtr cjWindowStage) {};
        funcs->cjAbilityOnSceneRestored = [](int64_t id, WindowStagePtr cjWindowStage) {};
        funcs->cjAbilityOnSceneDestroyed = [](int64_t id) {};
        funcs->cjAbilityOnForeground = [](int64_t id, WantHandle want) {};
        funcs->cjAbilityOnBackground = [](int64_t id) {};
        funcs->cjAbilityOnConfigurationUpdated = [](int64_t id, CJConfiguration configuration) {};
        funcs->cjAbilityOnNewWant = [](int64_t id, WantHandle want, CJLaunchParam launchParam) {};
        funcs->cjAbilityDump = [](int64_t id, VectorStringHandle params) { return VectorStringHandle(); };
        funcs->cjAbilityOnContinue = [](int64_t id, const char* params) { return 0; };
        funcs->cjAbilityInit = [](int64_t id, void* ability) {};
    };
    RegisterCJAbilityFuncs(registerFunc);
    ProxyCall();
    RegisterCJAbilityFuncs(nullptr);
}
