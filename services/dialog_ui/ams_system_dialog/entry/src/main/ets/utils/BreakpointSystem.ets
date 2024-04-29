/*
 *
 *  * Copyright (c) 2024 Huawei Device Co., Ltd.
 *  * Licensed under the Apache License, Version 2.0 (the "License");
 *  * you may not use this file except in compliance with the License.
 *  * You may obtain a copy of the License at
 *  *
 *  *     http://www.apache.org/licenses/LICENSE-2.0
 *  *
 *  * Unless required by applicable law or agreed to in writing, software
 *  * distributed under the License is distributed on an "AS IS" BASIS,
 *  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  * See the License for the specific language governing permissions and
 *  * limitations under the License.
 *
 */
import mediaQuery from '@ohos.mediaquery';

const TAG = 'BreakpointSystem';
type triggerFunc = (columnType: ColumnType) => void;

export enum ColumnType {
    BREAKPOINT_4_COLUMN = "sm",
    BREAKPOINT_8_COLUMN = "md",
    BREAKPOINT_12_COLUMN = "lg"
}

export const BREAKPOINT_VALUE = [0, 520, 840]

export default class BreakpointSystem {
    private currentBreakpoint: string = '';
    private smListener: mediaQuery.MediaQueryListener | null = null;
    private mdListener: mediaQuery.MediaQueryListener | null = null;
    private lgListener: mediaQuery.MediaQueryListener | null = null;
    private callback: triggerFunc | null = null;

    constructor() {
        console.info(TAG, 'BreakpointSystem construct.');
    }

    private updateCurrentBreakpoint(breakpoint: ColumnType) {
        if (this.currentBreakpoint !== breakpoint) {
            this.currentBreakpoint = breakpoint;
            if (this.callback) {
                this.callback(breakpoint);
            }
        }
    }

    private isBreakpointSM = (mediaQueryResult: mediaQuery.MediaQueryResult) => {
        if (mediaQueryResult.matches) {
            this.updateCurrentBreakpoint(ColumnType.BREAKPOINT_4_COLUMN);
        }
    }
    private isBreakpointMD = (mediaQueryResult: mediaQuery.MediaQueryResult) => {
        if (mediaQueryResult.matches) {
            this.updateCurrentBreakpoint(ColumnType.BREAKPOINT_8_COLUMN);
        }
    }
    private isBreakpointLG = (mediaQueryResult: mediaQuery.MediaQueryResult) => {
        if (mediaQueryResult.matches) {
            this.updateCurrentBreakpoint(ColumnType.BREAKPOINT_12_COLUMN);
        }
    }

    public register(callback: triggerFunc) {
        this.callback = callback;
        console.info(TAG, `register.currentBreakpoint: ${this.currentBreakpoint}`)
        if (BREAKPOINT_VALUE === null || BREAKPOINT_VALUE.length !== 3) {
            console.info(TAG, 'breakpoint system register error');
        }
        this.smListener = mediaQuery.matchMediaSync('(width<' + BREAKPOINT_VALUE[1] + 'vp)');
        this.smListener.on('change', this.isBreakpointSM);
        this.mdListener = mediaQuery.matchMediaSync('(' + BREAKPOINT_VALUE[1] + 'vp<=width<' + BREAKPOINT_VALUE[2] + 'vp)');
        this.mdListener.on('change', this.isBreakpointMD);
        this.lgListener = mediaQuery.matchMediaSync('(' + BREAKPOINT_VALUE[1] + 'vp<=width)');
        this.lgListener.on('change', this.isBreakpointLG);
    }

    public unregister() {
        if (this.smListener !== null) {
            this.smListener.off('change', this.isBreakpointSM)
        }
        if (this.mdListener !== null) {
            this.mdListener.off('change', this.isBreakpointMD)
        }
        if (this.lgListener !== null) {
            this.lgListener.off('change', this.isBreakpointLG)
        }
    }
}
