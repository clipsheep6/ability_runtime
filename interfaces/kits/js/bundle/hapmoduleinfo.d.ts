/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
import {AbilityInfo} from "./abilityinfo";

/**
 * @name HapModuleInfo
 * @since 3
 * @SysCap BMS
 * @import NA
 * @permission NA
 * @devices phone, tablet
 */
export interface HapModuleInfo {
  readonly name: string;
  readonly description: string;
  readonly descriptionId: number;
  readonly icon: string;
  readonly label: string;
  readonly labelId: number;
  readonly iconId: number;
  readonly backgroundImg: string;
  readonly supportedModes: number;
  readonly reqCapabilities: Array<string>;
  readonly deviceTypes: Array<string>;
  readonly abilityInfos: Array<AbilityInfo>;
  readonly moduleName: string;
  readonly mainAbilityName: string;
  readonly installationFree: boolean;
}