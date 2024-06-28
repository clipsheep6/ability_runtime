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

import deviceInfo from '@ohos.deviceInfo';
import display from '@ohos.display';

const TAG = '[PositionUtils]';

const UI_SELECTOR_DIALOG_WIDTH = 656;
const UI_SELECTOR_DIALOG_HEIGHT = 700;

const UI_SELECTOR_DIALOG_PC_H0 = 1;
const UI_SELECTOR_DIALOG_PC_H2 = 800;
const UI_SELECTOR_DIALOG_PC_H3 = 928;
const UI_SELECTOR_DIALOG_PC_H4 = 1056;
const UI_SELECTOR_DIALOG_PC_H5 = 1172;

const UI_SELECTOR_DIALOG_PHONE_H1 = 280;
const UI_SELECTOR_DIALOG_PHONE_H2 = 400;
const UI_SELECTOR_DIALOG_PHONE_H3 = 410;
const UI_SELECTOR_LANDSCAPE_SIGNAL_BAR = 24;
const UI_SELECTOR_LANDSCAPE_HEIGHT = 350;
const UI_SELECTOR_LANDSCAPE_COUNT_THREE = 3;
const UI_SELECTOR_LANDSCAPE_COUNT_FOUR = 4;
const UI_SELECTOR_LANDSCAPE_GRILLE_LARGE = 0.107692;
const UI_SELECTOR_LANDSCAPE_GRILLE_SAMLL = 0.015385;
const UI_SELECTOR_LANDSCAPE_MAX_RATIO = 0.9;
const UI_SELECTOR_PORTRAIT_WIDTH_RATIO = 0.8;
const UI_SELECTOR_PORTRAIT_WIDTH_EDGE_RATIO = 0.1;
const UI_SELECTOR_PORTRAIT_HEIGHT_RATIO = 0.98;

const UI_TIPS_DIALOG_WIDTH = 656;
const UI_TIPS_DIALOG_HEIGHT = 270;
const UI_HALF = 2;
const UI_DEFAULT_BUTTOM_CLIP = 100;
const UI_DEFAULT_WIDTH = 2560;
const UI_DEFAULT_HEIGHT = 1600;
const STR_PHONE = 'phone';
const STR_DEFAULT = 'default';
const LINE_NUMS_ZERO = 0;
const LINE_NUMS_TWO = 2;
const LINE_NUMS_THREE = 3;
const LINE_NUMS_FOUR = 4;
const LINE_NUMS_EIGHT = 8;
const WIDTH_MULTIPLE = 0.8;
const HEIGHT_MULTIPLE = 0.3;
const SETX_WIDTH_MULTIPLE = 0.1;

export interface Position {
  width: number;
  height: number;
  offsetX: number;
  offsetY: number;
  oversizeHeight: boolean;
}

export default class PositionUtils {
  public static getTipsDialogPosition(): Position {
    let position = {
      width: UI_TIPS_DIALOG_WIDTH,
      height: UI_TIPS_DIALOG_HEIGHT,
      offsetX: 0,
      offsetY: 0,
      oversizeHeight: false
    };
    let displayClass: display.Display | null = this.getDefaultDisplay();
    if (displayClass) {
      let isPhone = (deviceInfo.deviceType === STR_PHONE) || (deviceInfo.deviceType === STR_DEFAULT);
      if (isPhone) {
        // Bottom
        position.width = displayClass.width * WIDTH_MULTIPLE;
        position.height = displayClass.height * HEIGHT_MULTIPLE;
        position.offsetX = displayClass.width * SETX_WIDTH_MULTIPLE;
        position.offsetY = displayClass.height - position.height - UI_DEFAULT_BUTTOM_CLIP;
      } else {
        // Center
        position.offsetX = (displayClass.width - position.width) / UI_HALF;
        position.offsetY = (displayClass.height - position.height) / UI_HALF;
      }
    } else {
      position.offsetX = (UI_DEFAULT_WIDTH - position.width) / UI_HALF;
      position.offsetY = UI_DEFAULT_HEIGHT - position.height - UI_DEFAULT_BUTTOM_CLIP;
    }
    return position;
  }

  public static getSelectorDialogPosition(lineNums): Position {
    let position = {
      width: 0,
      height: 0,
      offsetX: 0,
      offsetY: 0,
      oversizeHeight: false
    };
    if (deviceInfo.deviceType === STR_PHONE || deviceInfo.deviceType === STR_DEFAULT) {
      this.getPhoneSelectorDialogPosition(position, lineNums);
    } else {
      this.getPcSelectorDialogPosition(position, lineNums);
    }
    return position;
  }

  private static getPhoneSelectorDialogPosition(position, lineNums): void {
    let displayClass: display.Display | null = this.getDefaultDisplay();
    if (!displayClass) {
      return;
    }
    let densityPixels = displayClass.densityPixels;
    let width = displayClass.width;
    let height = displayClass.height;
    if (displayClass.orientation === display.Orientation.PORTRAIT ||
      displayClass.orientation === display.Orientation.PORTRAIT_INVERTED) {
      this.getPhoneSelectorPortraitPosition(position, width, height, lineNums, densityPixels);
    } else {
      this.getPhoneSelectorLandscapePosition(position, width, height, lineNums, densityPixels);
    }
  }

  private static getPcSelectorDialogPosition(position, lineNums): void {
    position.width = UI_SELECTOR_DIALOG_WIDTH;
    position.height = UI_SELECTOR_DIALOG_HEIGHT;
    let displayClass: display.Display | null = this.getDefaultDisplay();
    this.pcSelectorPositionAdaptive(position, lineNums);
    if (displayClass) {
      position.offsetX = (displayClass.width - position.width) / UI_HALF;
      position.offsetY = (displayClass.height - position.height) / UI_HALF;
    } else {
      position.offsetX = (UI_DEFAULT_WIDTH - position.width) / UI_HALF;
      position.offsetY = UI_DEFAULT_HEIGHT - position.height - UI_DEFAULT_BUTTOM_CLIP;
    }
  }

  private static getPhoneSelectorPortraitPosition(position, width, height, lineNums, densityPixels): void {
    position.width = Math.floor(width * UI_SELECTOR_PORTRAIT_WIDTH_RATIO);
    position.height = Math.floor(UI_SELECTOR_DIALOG_HEIGHT * densityPixels);

    this.phoneSelectorPositionAdaptive(position, densityPixels, lineNums);

    let portraitMax = Math.floor(height * UI_SELECTOR_PORTRAIT_HEIGHT_RATIO);
    if (portraitMax < position.height) {
      position.oversizeHeight = true;
      position.height = Math.floor(UI_SELECTOR_DIALOG_PHONE_H1 * densityPixels);
    }

    position.offsetX = Math.floor(width * UI_SELECTOR_PORTRAIT_WIDTH_EDGE_RATIO);
    position.offsetY = Math.floor((height * UI_SELECTOR_PORTRAIT_HEIGHT_RATIO - position.height));
  }

  private static getPhoneSelectorLandscapePosition(position, width, height, lineNums, densityPixels): void {
    position.width = Math.floor(width * (UI_SELECTOR_LANDSCAPE_GRILLE_LARGE * UI_SELECTOR_LANDSCAPE_COUNT_FOUR +
      UI_SELECTOR_LANDSCAPE_GRILLE_SAMLL * UI_SELECTOR_LANDSCAPE_COUNT_THREE));
    position.height = Math.floor((UI_SELECTOR_LANDSCAPE_HEIGHT) * densityPixels);
    this.phoneSelectorPositionAdaptive(position, densityPixels, lineNums);

    let landscapeMax = Math.floor((height - UI_SELECTOR_LANDSCAPE_SIGNAL_BAR * densityPixels) *
      UI_SELECTOR_LANDSCAPE_MAX_RATIO);
    if (position.height > landscapeMax) {
      position.oversizeHeight = true;
      position.height = Math.floor(UI_SELECTOR_DIALOG_PHONE_H1 * densityPixels);
    }

    position.offsetX = Math.floor((width - position.width) / UI_HALF);
    position.offsetY = Math.floor((height * UI_SELECTOR_PORTRAIT_HEIGHT_RATIO - position.height));
  }

  private static phoneSelectorPositionAdaptive(position, densityPixels, lineNums): void {
    if (lineNums > LINE_NUMS_EIGHT) {
      position.height = Math.floor(UI_SELECTOR_DIALOG_PHONE_H3 * densityPixels);
      return;
    } else if (lineNums > LINE_NUMS_FOUR) {
      position.height = Math.floor(UI_SELECTOR_DIALOG_PHONE_H2 * densityPixels);
      return;
    } else if (lineNums > LINE_NUMS_ZERO) {
      position.height = Math.floor(UI_SELECTOR_DIALOG_PHONE_H1 * densityPixels);
      return;
    }
  }

  private static pcSelectorPositionAdaptive(position, lineNums): void {
    if (lineNums <= LINE_NUMS_TWO) {
      position.height = UI_SELECTOR_DIALOG_PC_H2;
    } else if (lineNums === LINE_NUMS_THREE) {
      position.height = UI_SELECTOR_DIALOG_PC_H3;
    } else if (lineNums === LINE_NUMS_FOUR) {
      position.height = UI_SELECTOR_DIALOG_PC_H4;
    } else if (lineNums > LINE_NUMS_FOUR) {
      position.height = UI_SELECTOR_DIALOG_PC_H5;
    } else {
      position.height = UI_SELECTOR_DIALOG_PC_H0;
    }
  }

  private static getDefaultDisplay(): display.Display | null {
    let displayClass: display.Display | null = null;
    try {
      displayClass = display.getDefaultDisplaySync();
    } catch (err) {
      console.error(TAG, 'getDefaultDisplaySync failed.');
    }
    return displayClass;
  }
};