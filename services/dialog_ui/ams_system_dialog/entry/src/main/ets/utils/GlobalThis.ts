import common from '@ohos.app.ability.common';
import want from '@ohos.app.ability.Want';
import extension from '@ohos.app.ability.ServiceExtensionAbility';
import PositionUtils from '../utils/PositionUtils';
import image from '@ohos.multimedia.image';
// 构造单例对象

export interface Position {
  width: number;
  height: number;
  offsetX: number;
  offsetY: number;
  oversizeHeight: boolean;
}

export class GlobalThis {
  private constructor() {}
  private static instance: GlobalThis;
  private _serviceExtensionContexts = new Map<string, common.ServiceExtensionContext>();
  private _records = new Map<string, Record<string, Object>>();
  private _wants = new Map<string, want>();
  private _properties = new Map<string, string | number | Boolean | string[]>();
  private _objects = new Map<string, Object>();
  private _positionUtils = new Map<string, PositionUtils>();
  private _maps = new Map<string, Map<string, image.PixelMap>>();
  private _positions = new Map<string, Position>();

  public static getInstance(): GlobalThis {
    if (!GlobalThis.instance) {
      GlobalThis.instance = new GlobalThis();
    }
    return GlobalThis.instance;
  }

  getContext(key: string): common.ServiceExtensionContext | undefined {
    return this._serviceExtensionContexts.get(key);
  }

  setContext(key: string, value: common.ServiceExtensionContext): void {
    this._serviceExtensionContexts.set(key, value);
  }

  getWant(key: string): want | undefined {
    return this._wants.get(key);
  }

  setWant(key: string, value: want): void {
    this._wants.set(key, value);
  }

  getRecord(key: string): Record<string, Object> | undefined {
    return this._records.get(key);
  }

  setRecord(key: string, value: Record<string, Object>): void {
    this._records.set(key, value);
  }

  getProperty(key: string): string | number | Boolean| string[] | undefined {
    return this._properties.get(key);
  }

  setProperty(key: string, value: string | number | Boolean | string[]): void {
    this._properties.set(key, value);
  }

  getType(key: string): Object | undefined {
    return this._objects.get(key);
  }

  setType(key: string, value: Object): void {
    this._objects.set(key, value);
  }

  getObject(key: string): Object | undefined {
    return this._objects.get(key);
  }

  setObject(key: string, value: Object): void {
    this._objects.set(key, value);
  }

  getPositionUtils(key: string): PositionUtils {
    return this._positionUtils.get(key);
  }

  setPositionUtils(key: string, value: PositionUtils): void {
    this._positionUtils.set(key, value);
  }

  getMap(key: string): Map<string, image.PixelMap> | undefined {
    return this._maps.get(key);
  }

  setMap(key: string, value: Map<string, image.PixelMap>): void {
    this._maps.set(key, value);
  }

  getPosition(key: string): Position | undefined {
    return this._positions.get(key);
  }
  setPosition(key: string, value: Position): void {
    this._positions.set(key, value);
  }
}
